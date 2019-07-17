plugindir = $(libdir)/pragha/plugins/removable

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libremovable.la

libremovable_la_SOURCES = \
	pragha-devices-removable.c

libremovable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libremovable_la_CFLAGS =						\
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

libremovable_la_LIBADD =							\
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
libremovable_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libremovable_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libremovable_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libremovable_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = removable.plugin

EXTRA_DIST = $(plugin_DATA)
