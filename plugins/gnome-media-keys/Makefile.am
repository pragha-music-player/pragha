plugindir = $(libdir)/pragha/plugins/gnome-media-keys

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libgnome-media-keys.la

libgnome_media_keys_la_SOURCES =       \
	pragha-gnome-media-keys-plugin.c \
	pragha-gnome-media-keys-plugin.h

libgnome_media_keys_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libgnome_media_keys_la_CFLAGS =						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libgnome_media_keys_la_LIBADD =						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

plugin_DATA = gnome-media-keys.plugin

EXTRA_DIST = $(plugin_DATA)
