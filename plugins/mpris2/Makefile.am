plugindir = $(libdir)/pragha/plugins/mpris2

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libmpris2.la

libmpris2_la_SOURCES =      \
	pragha-mpris2-plugin.h \
	pragha-mpris2-plugin.c

libmpris2_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libmpris2_la_CFLAGS = \
	$(PRAGHA_CFLAGS)

libmpris2_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(top_builddir)/src/libpragha.la


plugin_DATA = mpris2.plugin

EXTRA_DIST = $(plugin_DATA)
