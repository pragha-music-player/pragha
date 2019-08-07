plugindir = $(libdir)/pragha/plugins/dlna

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libdlna.la

libdlna_la_SOURCES =      \
	pragha-dlna-plugin.h \
	pragha-dlna-plugin.c

libdlna_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libdlna_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(RYGEL_CFLAGS)

libdlna_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(RYGEL_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = dlna.plugin

EXTRA_DIST = $(plugin_DATA)
