plugindir = $(libdir)/pragha/plugins/mpris2

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libmpris2.la

libmpris2_la_SOURCES =      \
	pragha-mpris2-plugin.h \
	pragha-mpris2-plugin.c

libmpris2_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libmpris2_la_CFLAGS =						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBNOTIFY_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libmpris2_la_LIBADD =						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBNOTIFY_LIBS) 						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS)

if HAVE_GSTREAMER_AUDIO
libmpris2_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libmpris2_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libmpris2_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libmpris2_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = mpris2.plugin

EXTRA_DIST = $(plugin_DATA)
