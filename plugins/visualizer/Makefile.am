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

libvisualizer_la_CFLAGS =						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libvisualizer_la_LIBADD =						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libvisualizer_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libvisualizer_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libvisualizer_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libvisualizer_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = visualizer.plugin

EXTRA_DIST = \
	$(plugin_DATA)
