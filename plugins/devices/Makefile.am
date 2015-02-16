plugindir = $(libdir)/pragha/plugins/devices

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpdevices.la

libpdevices_la_SOURCES = \
	pragha-devices-plugin.c \
	pragha-devices-plugin.h \
	pragha-device-client.c \
	pragha-device-client.h

libpdevices_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpdevices_la_CFLAGS =						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	$(GUDEV_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libpdevices_la_LIBADD =							\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)						\
	$(GUDEV_LIBS)

if HAVE_GSTREAMER_AUDIO
libpdevices_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpdevices_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpdevices_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpdevices_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = devices.plugin

EXTRA_DIST = $(plugin_DATA)
