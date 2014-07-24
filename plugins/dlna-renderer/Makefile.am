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

libpdlnarenderer_la_CFLAGS =				\
	$(GRILO_CFLAGS)							\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)				\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 					\
	-I$(top_srcdir)/src/include 			\
	-Wall

libpdlnarenderer_la_LIBADD =				\
	$(GRILO_LIBS) 							\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpdlnarenderer_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpdlnarenderer_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpdlnarenderer_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpdlnarenderer_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = dlna-renderer.plugin

EXTRA_DIST = $(plugin_DATA)
