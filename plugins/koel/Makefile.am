plugindir = $(libdir)/pragha/plugins/koel

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpkoel.la

libpkoel_la_SOURCES =		\
	pragha-koel-plugin.h 	\
	pragha-koel-plugin.c

libpkoel_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpkoel_la_CFLAGS = \
	$(LIBSOUP_CFLAGS) \
	$(JSON_GLIB_CFLAGS) \
	$(PRAGHA_CFLAGS)

libpkoel_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBSOUP_LIBS) \
	$(JSON_GLIB_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = koel.plugin

EXTRA_DIST = $(plugin_DATA)
