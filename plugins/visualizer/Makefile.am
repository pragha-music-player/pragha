plugindir = $(libdir)/pragha/plugins/visualizer

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libvisualizer.la

libvisualizer_la_SOURCES =      \
	pragha-visualizer-plugin.h \
	pragha-visualizer-plugin.c \
	pragha-visualizer.h \
	pragha-visualizer.c \
	pragha-visualizer-particle.h \
	pragha-visualizer-particle.c

libvisualizer_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libvisualizer_la_CFLAGS = \
	$(PRAGHA_CFLAGS)

libvisualizer_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = visualizer.plugin

EXTRA_DIST = \
	$(plugin_DATA)
