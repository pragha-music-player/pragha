plugindir = $(libdir)/pragha/plugins/keybinder

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libkeybinder.la

libkeybinder_la_SOURCES =      \
	pragha-keybinder-plugin.h \
	pragha-keybinder-plugin.c

libkeybinder_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libkeybinder_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBKEYBINDER_CFLAGS)

libkeybinder_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBKEYBINDER_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = keybinder.plugin

EXTRA_DIST = $(plugin_DATA)
