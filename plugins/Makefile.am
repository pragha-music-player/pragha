AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(LIBPEAS_CFLAGS)							\
	$(LIB_PEAS_GTK_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)

pluginincludedir = $(includedir)/pragha/plugins
plugininclude_HEADERS = pragha-plugin-macros.h

SUBDIRS =              \
	mpris2             \
	gnome-media-keys

if HAVE_LIBGLYR
SUBDIRS += song-info
endif

if HAVE_LIBKEYBINDER
SUBDIRS += keybinder
endif

if HAVE_LIBNOTIFY
SUBDIRS += notify
endif

if HAVE_LIBMTP
SUBDIRS += devices
endif