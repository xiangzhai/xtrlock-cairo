/* Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn> */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <string.h>
#include <crypt.h>
#include <libintl.h>
#include <locale.h>

#define _(x) gettext(x)
#define TEXT_SIZE 137
#define M_PI 3.1415926
#define TEXT_HEIGHT 16
#define TEXT_MARGIN 6
#define PWD_CHAR "*"

static struct passwd *pw = NULL;
static Display *display = NULL;
static int window_width = 0;
static int window_height = 0;
static Window window;
static cairo_surface_t *x11_cs = NULL;
static cairo_surface_t *image_cs = NULL;
static cairo_surface_t *text_cs = NULL;
static cairo_t *x11_cr = NULL;
static cairo_t *text_cr = NULL;
static char text[TEXT_SIZE] = {'\0'};
static int quit = 0;

static void cleanup();
static int check_password(const char *s);
static void clear_text();
static void set_font(cairo_t *cr);
static void draw_image();
static void draw_text(char *text);

static void cleanup() 
{
    if (x11_cr) cairo_destroy(x11_cr); x11_cr = NULL;
    if (text_cr) cairo_destroy(text_cr); text_cr = NULL;
    if (image_cs) cairo_surface_destroy(image_cs); image_cs = NULL;
    if (text_cs) cairo_surface_destroy(text_cs); text_cs = NULL;
    if (x11_cs) cairo_surface_destroy(x11_cs); x11_cs = NULL;
    if (display) XCloseDisplay(display); display = NULL;
}

static int check_password(const char *s) 
{
    return !strcmp(crypt(s, pw->pw_passwd), pw->pw_passwd);
}

static void clear_text() 
{
    memset(text, 0, TEXT_SIZE);
    snprintf(text, TEXT_SIZE, _("Password "));
}

static void set_font(cairo_t *cr) 
{
    cairo_select_font_face(cr, "Serif",                                        
                           CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);       
    cairo_set_font_size(cr, TEXT_HEIGHT);
}

static void draw_image() 
{
    int w, h;
    
    if (!x11_cr) 
        return;

    w = cairo_image_surface_get_width(image_cs);
    h = cairo_image_surface_get_height(image_cs);

    cairo_arc(x11_cr, 
              window_width / 2, (window_height - h) / 2, w / 2, 0, 2 * M_PI);
    cairo_clip(x11_cr);
    cairo_set_source_surface(x11_cr, image_cs, 
                             (window_width - w) / 2, window_height / 2 - h);
    cairo_paint(x11_cr);
}

static void draw_text(char *text) 
{
    int x = 0, y = TEXT_HEIGHT;

    if (!text_cr) 
        return;

    cairo_set_source_rgb(text_cr, 0, 0, 0);
    cairo_paint(text_cr);

    cairo_set_source_rgb(text_cr, 1, 1, 0);
    cairo_move_to(text_cr, x, y);
    y += TEXT_HEIGHT + TEXT_MARGIN;
    cairo_move_to(text_cr, x, y); 
    cairo_show_text(text_cr, text);                                     
}

int main(int argc, char *argv[]) 
{
    struct spwd *sp = NULL;
    XSetWindowAttributes attrib;
    int screen;
    XEvent ev;
    KeySym ks;
    char cbuf[10] = {'\0'}, rbuf[128] = {'\0'};
    int clen, rlen = 0;

    setlocale(LC_ALL, "");                                                       
    bindtextdomain(GETTEXT_PACKAGE, XTRLOCK_CAIRO_LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);
#if MSS_DEBUG
    printf("DEBUG: %s, line %d %s %s\n", __func__, __LINE__, 
           GETTEXT_PACKAGE, XTRLOCK_CAIRO_LOCALEDIR);
#endif

    pw = getpwuid(getuid());                                                
    if (!pw) { 
        printf("ERROR: password entry for uid not found\n"); 
        return 1; 
    }                
#if MSS_DEBUG
    printf("DEBUG: %s, line %d current login user %s\n", 
           __func__, __LINE__, pw->pw_name);
#endif
    sp = getspnam(pw->pw_name);                                                      
    if (sp) {                                                                        
        pw->pw_passwd = sp->sp_pwdp;
#if MSS_DEBUG
        printf("DEBUG: %s, line %d crypt pwd %s\n", 
               __func__, __LINE__, pw->pw_passwd);
#endif
    }
    endspent();

    if (setgid(getgid())) { 
        printf("ERROR: setgid\n"); 
        return 1; 
    }
    if (setuid(getuid())) { 
        printf("ERROR: setuid\n"); 
        return 1; 
    }

    if (strlen(pw->pw_passwd) < 13) {
        printf("ERROR: password entry has no pwd\n");
        return 1;
    }

    /* display */
    display= XOpenDisplay(NULL);
    if (!display) {
        printf("ERROR: fail to open display\n");
        return 1;
    }

    /* window */
    attrib.override_redirect = True;
    screen = DefaultScreen(display);                                               
    attrib.background_pixel = BlackPixel(display, screen);                         
    window_width = DisplayWidth(display, screen);
    window_height = DisplayHeight(display, screen);
    window= XCreateWindow(display, DefaultRootWindow(display),
                          0, 0, window_width, window_height,
                          0, DefaultDepth(display, screen), CopyFromParent,
                          DefaultVisual(display, screen),
                          CWOverrideRedirect | CWBackPixel, &attrib);
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);

    /* cairo root surface */
    x11_cs = cairo_xlib_surface_create(display, window, 
                                       DefaultVisual(display, screen), 
                                       0, 0);
    if (!x11_cs) {
        printf("ERROR: fail to create x11 surface\n");
        cleanup();
        return 1;
    }
    cairo_xlib_surface_set_size(x11_cs, window_width, window_height);

    /* cairo x11 context */
    x11_cr = cairo_create(x11_cs);
    if (!x11_cr) {
        printf("ERROR: fail to create x11 context\n");
        cleanup();
        return 1;
    }

    /* cairo image */
    image_cs = cairo_image_surface_create_from_png(DATADIR "/doge.png");
    if (!image_cs) {
        printf("ERROR: fail to create image surface\n");
        cleanup();
        return 1;
    }

    /* cairo text surface */
    text_cs = cairo_surface_create_for_rectangle(x11_cs, 
            window_width / 2, window_height / 2, 
            window_width / 2 - TEXT_MARGIN, TEXT_HEIGHT * 3);

    /* cairo text context */
    text_cr = cairo_create(text_cs);
    if (!text_cr) {
        printf("ERROR: fail to create text context\n");
        cleanup();
        return 1;
    }

    /* cairo text font */
    set_font(text_cr);

    /* grab keyboard */
    XGrabKeyboard(display, window, False, GrabModeAsync, 
                  GrabModeAsync, CurrentTime);

    draw_image();
    clear_text();
    while (quit == 0) {
        XNextEvent(display, &ev);
        switch (ev.type) {
        case KeyPress:
            clen = XLookupString(&ev.xkey, cbuf, 9, &ks, 0);
            switch (ks) {
            case XK_Escape: 
            case XK_Clear:
                rlen = 0;
                clear_text();
                break;
            case XK_Delete: 
            case XK_BackSpace:
                if (rlen) 
                    rlen--;
                clear_text();
                for (int i = 0; i < rlen; i++)
                    strcat(text, PWD_CHAR);
                break;
            case XK_Linefeed: 
            case XK_Return:
                if (rlen == 0) 
                    break;
                rbuf[rlen] = 0;
                if (check_password(rbuf)) {
                    quit = 1;
                    break;
                } else 
                    clear_text();
                rlen = 0;
                break;
            default:
                if (clen != 1) 
                    break;
                if (rlen < (sizeof(rbuf) - 1)) {
                    rbuf[rlen] = cbuf[0];
                    strcat(text, PWD_CHAR);
                    rlen++;
                } 
                break;
            }
        default:
            break;
        }
        draw_text(text);
        usleep(100);
    }

    cleanup();

    return 0;
}
