# Device library.

deviceclientdir = $(libdir)/pragha/plugins/devices

deviceclient_LTLIBRARIES = libdeviceclient.la

libdeviceclient_la_SOURCES = \
	pragha-device-client.c \
	pragha-device-client.h

#libdeviceclient_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libdeviceclient_la_CFLAGS =						\
	$(GTK_CFLAGS) 							\
	$(GUDEV_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libdeviceclient_la_LIBADD =						\
	$(GTK_LIBS) 							\
	$(GUDEV_LIBS)

# Fake plugin

plugindir = $(libdir)/pragha/plugins/devices

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libdevices.la

libdevices_la_SOURCES = \
	pragha-devices-plugin.c \
	pragha-devices-plugin.h

libdevices_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libdevices_la_CFLAGS =						\
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

libdevices_la_LIBADD =							\
	libdeviceclient.la						\
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
libdevices_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libdevices_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libdevices_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libdevices_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = devices.plugin

EXTRA_DIST = $(plugin_DATA)
