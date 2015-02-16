plugindir = $(libdir)/pragha/plugins/notify

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpnotify.la

libpnotify_la_SOURCES =						\
	pragha-notify-plugin.c

libpnotify_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpnotify_la_CFLAGS =						\
	$(LIBGLYR_CFLAGS)							\
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
	-I$(top_srcdir)/src/include 					\
	-Wall

libpnotify_la_LIBADD =						\
	$(LIBGLYR_LIBS) 							\
	$(GSTREAMER_LIBS)							\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)							\
	$(LIBNOTIFY_LIBS) 						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpnotify_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpnotify_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpnotify_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpnotify_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = notify.plugin

EXTRA_DIST = $(plugin_DATA)
