AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(LIBPEAS_CFLAGS)							\
	$(LIB_PEAS_GTK_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)

SUBDIRS =              \
	gnome-media-keys \
	helloworld

if HAVE_LIBKEYBINDER
SUBDIRS += keybinder
endif

