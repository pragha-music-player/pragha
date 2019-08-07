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

libgnome_media_keys_la_CFLAGS = \
	$(PRAGHA_CFLAGS)

libgnome_media_keys_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = gnome-media-keys.plugin

EXTRA_DIST = $(plugin_DATA)
