#AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = xtrlock-cairo

xtrlock_cairo_SOURCES = main.c
xtrlock_cairo_CFLAGS = @XTRLOCK_CAIRO_CFLAGS@ -DDATADIR=\""$(datadir)/xtrlock-cairo"\"
xtrlock_cairo_LDADD = @XTRLOCK_CAIRO_LIBS@ @CRYPT_LIBS@

install-exec-hook:
	chmod 755 $(bindir)/xtrlock-cairo
	chmod u+s $(bindir)/xtrlock-cairo

noinst_PROGRAMS = test_dummy	\
				  test_notify_send	\
				  test_cairo_xlib_simple	\
				  test_cairo_xlib

test_dummy_SOURCES = test_dummy.c

test_notify_send_SOURCES = test_notify_send.c

test_cairo_xlib_simple_SOURCES = test_cairo_xlib_simple.c
test_cairo_xlib_simple_CFLAGS = @XTRLOCK_CAIRO_CFLAGS@
test_cairo_xlib_simple_LDADD = @XTRLOCK_CAIRO_LIBS@

test_cairo_xlib_SOURCES = test_cairo_xlib.c                          
test_cairo_xlib_CFLAGS = @XTRLOCK_CAIRO_CFLAGS@                         
test_cairo_xlib_LDADD = @XTRLOCK_CAIRO_LIBS@
