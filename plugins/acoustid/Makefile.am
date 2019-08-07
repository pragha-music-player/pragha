plugindir = $(libdir)/pragha/plugins/acoustid

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libacoustid.la

libacoustid_la_SOURCES =      \
	pragha-acoustid-plugin.c

libacoustid_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libacoustid_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBSOUP_CFLAGS)

libacoustid_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBSOUP_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = acoustid.plugin

EXTRA_DIST = $(plugin_DATA)
