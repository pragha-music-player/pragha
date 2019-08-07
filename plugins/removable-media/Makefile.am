plugindir = $(libdir)/pragha/plugins/removable

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libremovable.la

libremovable_la_SOURCES = \
	pragha-devices-removable.c

libremovable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libremovable_la_CFLAGS = \
	$(PRAGHA_CFLAGS)

libremovable_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = removable.plugin

EXTRA_DIST = $(plugin_DATA)
