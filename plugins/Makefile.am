AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(LIBPEAS_CFLAGS)							\
	$(LIB_PEAS_GTK_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)

SUBDIRS =              \
	mpris2             \
	gnome-media-keys

if HAVE_LIBGLYR
SUBDIRS += song-info
endif

if HAVE_LIBKEYBINDER
SUBDIRS += keybinder
endif

