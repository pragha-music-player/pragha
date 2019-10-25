AM_CPPFLAGS =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(LIBPEAS_CFLAGS)							\
	$(LIB_PEAS_GTK_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)

pluginincludedir = $(includedir)/libpragha/plugins
plugininclude_HEADERS = pragha-plugin-macros.h

SUBDIRS = \
	visualizer

if DBUS_PLUGINS_SUPPORT
SUBDIRS += \
	mpris2 \
	gnome-media-keys
endif

if HAVE_LIBGLYR
SUBDIRS += song-info
endif

if HAVE_LIBKEYBINDER
SUBDIRS += keybinder
endif

if HAVE_LIBNOTIFY
SUBDIRS += notify
endif

if HAVE_GUDEV
SUBDIRS += removable-media
if HAVE_LIBMTP
SUBDIRS += mtp
endif
endif

if HAVE_LIBSOUP
SUBDIRS += acoustid
SUBDIRS += tunein
endif

if HAVE_RYGEL
SUBDIRS += dlna
endif

if HAVE_GRILO3
SUBDIRS += dlna-renderer
endif
if HAVE_GRILO2
SUBDIRS += dlna-renderer
endif

if HAVE_GRILO_NET3
SUBDIRS += ampache
endif
if HAVE_GRILO_NET2
SUBDIRS += ampache
endif

if HAVE_GRILO_NET3
SUBDIRS += subsonic
endif
if HAVE_GRILO_NET2
SUBDIRS += subsonic
endif

if HAVE_JSON_GLIB
if HAVE_LIBSOUP
SUBDIRS += koel
endif
endif

if HAVE_LIBCDIO
if HAVE_LIBCDIO_PARANOIA
if HAVE_LIBCDDB
SUBDIRS += cdrom
endif
endif
endif

if HAVE_LIBCLASTFM
SUBDIRS += lastfm
endif

