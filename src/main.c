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
#define TEXT_HEIGHT 12
#define TEXT_MARGIN 6
#define PWD_CHAR "*"

static struct passwd *pw = NULL;
static Display *display = NULL;
static int window_width = 0;
static int window_height = 0;
static Window window;
static cairo_surface_t *cs = NULL;
static cairo_t *cr = NULL;
static char text[TEXT_SIZE] = {'\0'};
static int quit = 0;

static void cleanup();
static int check_password(const char *s);
static void clear_text();
static void draw(char *text);

static void cleanup() 
{
    if (cr) cairo_destroy(cr); cr = NULL;                                          
    if (cs) cairo_surface_destroy(cs); cs = NULL;                                  
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

static void draw(char *text) 
{
    int x, y = 0;
    
    if (!cr) 
        return;

    cairo_push_group(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);                                             
    cairo_paint(cr);

    /* text */
    cairo_set_source_rgb(cr, 1, 1, 0);
    x = window_width / 2;
    y = (window_height - TEXT_HEIGHT) / 2;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, pw->pw_name);
    y += TEXT_HEIGHT + TEXT_MARGIN;
    cairo_move_to(cr, x, y); 
    cairo_show_text(cr, text);                                     
    
    cairo_pop_group_to_source(cr);                                 
    cairo_paint(cr);

    cairo_surface_flush(cs);
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

    /* cairo surface */
    cs = cairo_xlib_surface_create(display, window, 
                                   DefaultVisual(display, screen), 
                                   0, 0);
    if (!cs) {
        printf("ERROR: fail to create surface\n");
        cleanup();
        return 1;
    }
    cairo_xlib_surface_set_size(cs, DisplayWidth(display, screen), 
                                DisplayHeight(display, screen));

    /* cairo context */
    cr = cairo_create(cs);
    if (!cr) {
        printf("ERROR: fail to create context\n");
        cleanup();
        return 1;
    }

    /* font */
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_select_font_face(cr, "Serif", 
                           CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, TEXT_HEIGHT); 

    /* grab keyboard */
    XGrabKeyboard(display, window, False, GrabModeAsync, 
                  GrabModeAsync, CurrentTime);

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
        draw(text);
        usleep(100);
    }

    cleanup();

    return 0;
}
