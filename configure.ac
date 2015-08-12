AC_INIT([xtrlock-cairo], [1.0], [xiang.zhai@i-soft.com.cn])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AM_MAINTAINER_MODE
IT_PROG_INTLTOOL([0.35.0])                                                      
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

CFLAGS='-Wall -std=gnu99 -Werror'

X11_REQUIRED=1.6.2
XTST_REQUIRED=1.2.2
CAIRO_XLIB_REQUIRED=1.12.16

PKG_CHECK_MODULES(XTRLOCK_CAIRO, x11 >= $X11_REQUIRED xtst >= $XTST_REQUIRED 
                  cairo-xlib >= $CAIRO_XLIB_REQUIRED)
AC_SUBST(XTRLOCK_CAIRO_CFLAGS)
AC_SUBST(XTRLOCK_CAIRO_LIBS)

AC_CHECK_HEADERS([shadow.h])

AC_CHECK_LIB(crypt, crypt, HAVE_CRYPT=yes, HAVE_CRYPT=no)           
if test "$HAVE_CRYPT" = "no"; then                                               
 AC_MSG_ERROR([crypt is required])                                               
fi                                                                                 
CRYPT_LIBS=-lcrypt
AC_SUBST(CRYPT_LIBS)

AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug=@<:@no/yes@:>@],
                              [turn on debugging [default=no]])],,
              [enable_debug=no])

if test "x$enable_debug" = "xyes"; then
  test "$cflags_set" = set || CFLAGS="$CFLAGS -g -DXTRLOCK_CAIRO_DEBUG"
fi

dnl ================================================================            
dnl  Language Support                                                           
dnl ================================================================            
GETTEXT_PACKAGE=xtrlock-cairo 
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define the gettext package to be used])
AC_SUBST(GETTEXT_PACKAGE)                                                       
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(XTRLOCK_CAIRO_LOCALEDIR)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 po/Makefile.in
 data/Makefile
 src/Makefile
])
AC_OUTPUT
