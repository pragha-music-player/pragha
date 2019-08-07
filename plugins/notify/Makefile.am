plugindir = $(libdir)/pragha/plugins/notify

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libnotify.la

libnotify_la_SOURCES =						\
	pragha-notify-plugin.c

libnotify_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libnotify_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBNOTIFY_CFLAGS)

libnotify_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = notify.plugin

EXTRA_DIST = $(plugin_DATA)
