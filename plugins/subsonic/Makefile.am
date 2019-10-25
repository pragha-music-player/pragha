plugindir = $(libdir)/pragha/plugins/subsonic

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpsubsonic.la

libpsubsonic_la_SOURCES = \
	pragha-subsonic-api.c \
	pragha-subsonic-api.h \
	pragha-subsonic-preferences.c \
	pragha-subsonic-preferences.h \
	pragha-subsonic-plugin.c \
	pragha-subsonic-plugin.h

libpsubsonic_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpsubsonic_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(GRILO_NET_CFLAGS)

libpsubsonic_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(GRILO_NET_LIBS) \
	$(top_builddir)/src/libpragha.la

if HAVE_GRILO3
libpsubsonic_la_CFLAGS += $(GRILO_NET3_CFLAGS)
libpsubsonic_la_LIBADD += $(GRILO_NET3_LIBS)
endif
if HAVE_GRILO2
libpsubsonic_la_CFLAGS += $(GRILO_NET2_CFLAGS)
libpsubsonic_la_LIBADD += $(GRILO_NET2_LIBS)
endif

plugin_DATA = subsonic.plugin

EXTRA_DIST = $(plugin_DATA)
