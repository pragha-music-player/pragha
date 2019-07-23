plugindir = $(libdir)/pragha/plugins/mtp

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpmtp.la

libpmtp_la_SOURCES = \
	pragha-devices-mtp.c \
	pragha-mtp-musicobject.c \
	pragha-mtp-musicobject.h \
	pragha-mtp-thread.c \
	pragha-mtp-thread.h \
	pragha-mtp-thread-data.c \
	pragha-mtp-thread-data.h

libpmtp_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpmtp_la_CFLAGS =							\
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
	$(LIBMTP_CFLAGS)						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libpmtp_la_LIBADD =							\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)						\
	$(GUDEV_LIBS) 							\
	$(LIBMTP_LIBS)

if HAVE_GSTREAMER_AUDIO
libpmtp_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpmtp_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpmtp_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpmtp_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = mtp.plugin

EXTRA_DIST = $(plugin_DATA)
