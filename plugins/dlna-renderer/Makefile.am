plugindir = $(libdir)/pragha/plugins/dlna-renderer

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpdlnarenderer.la

libpdlnarenderer_la_SOURCES =		\
	pragha-dlna-renderer-plugin.h 	\
	pragha-dlna-renderer-plugin.c

libpdlnarenderer_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpdlnarenderer_la_CFLAGS = \
	$(PRAGHA_CFLAGS)

libpdlnarenderer_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(top_builddir)/src/libpragha.la

if HAVE_GRILO3
libpdlnarenderer_la_CFLAGS += $(GRILO3_CFLAGS)
libpdlnarenderer_la_LIBADD += $(GRILO3_LIBS)
endif
if HAVE_GRILO2
libpdlnarenderer_la_CFLAGS += $(GRILO2_CFLAGS)
libpdlnarenderer_la_LIBADD += $(GRILO2_LIBS)
endif

plugin_DATA = dlna-renderer.plugin

EXTRA_DIST = $(plugin_DATA)
