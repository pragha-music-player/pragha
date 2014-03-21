plugindir = $(libdir)/pragha/plugins/cdrom

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libcdrom.la

libcdrom_la_SOURCES =      \
	pragha-cdrom-plugin.c  \
	pragha-cdrom-plugin.h

libcdrom_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libcdrom_la_CFLAGS =						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libcdrom_la_LIBADD =						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS)

if HAVE_GSTREAMER_AUDIO
libcdrom_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libcdrom_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libcdrom_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libcdrom_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = cdrom.plugin

EXTRA_DIST = $(plugin_DATA)
