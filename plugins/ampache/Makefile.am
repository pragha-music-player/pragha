plugindir = $(libdir)/pragha/plugins/ampache

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpampache.la

libpampache_la_SOURCES =		\
	pragha-ampache-plugin.h 	\
	pragha-ampache-plugin.c

libpampache_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpampache_la_CFLAGS =						\
	$(GRILO_NET_CFLAGS)						\
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

libpampache_la_LIBADD =						\
	$(GRILO_NET_LIBS) 						\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpampache_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpampache_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpampache_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpampache_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = ampache.plugin

EXTRA_DIST = $(plugin_DATA)
