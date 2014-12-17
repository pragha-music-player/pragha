plugindir = $(libdir)/pragha/plugins/keybinder

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)	\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libandroid-client.la

libandroid_client_la_SOURCES =  \
	pragha-android-plugin.h \
	pragha-android-plugin.c

libandroid_client_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libandroid_client_la_CFLAGS =						\
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

libandroid_client_la_LIBADD =						\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libandroid_client_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libandroid_client_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libandroid_client_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libandroid_client_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = android-client.plugin

EXTRA_DIST = $(plugin_DATA)
