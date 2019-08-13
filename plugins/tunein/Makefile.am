plugindir = $(libdir)/pragha/plugins/tunein

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libtunein.la

libtunein_la_SOURCES =      \
	pragha-tunein-plugin.c

libtunein_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libtunein_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBSOUP_CFLAGS)


libtunein_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBSOUP_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = tunein.plugin

EXTRA_DIST = $(plugin_DATA)
