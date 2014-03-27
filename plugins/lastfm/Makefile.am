plugindir = $(libdir)/pragha/plugins/lastfm

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libplastfm.la

libplastfm_la_SOURCES =      \
	pragha-lastfm-plugin.c

libplastfm_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libplastfm_la_CFLAGS =						\
	$(LIBCLASTFM_CFLAGS)					\
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

libplastfm_la_LIBADD =						\
	$(LIBCLASTFM_LIBS)						\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libplastfm_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libplastfm_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libplastfm_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libplastfm_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = lastfm.plugin

EXTRA_DIST = $(plugin_DATA)
