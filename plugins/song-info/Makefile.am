plugindir = $(libdir)/pragha/plugins/song-info

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpsong-info.la

libpsong_info_la_SOURCES =						\
	pragha-song-info-plugin.c					\
	pragha-song-info-plugin.h					\
	pragha-song-info-dialog.c					\
	pragha-song-info-dialog.h					\
	pragha-song-info-pane.c						\
	pragha-song-info-pane.h						\
	pragha-song-info-thread-albumart.c			\
	pragha-song-info-thread-albumart.h			\
	pragha-song-info-thread-dialog.c			\
	pragha-song-info-thread-dialog.h			\
	pragha-song-info-thread-pane.c				\
	pragha-song-info-thread-pane.h

libpsong_info_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpsong_info_la_CFLAGS =						\
	$(LIBGLYR_CFLAGS)							\
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

libpsong_info_la_LIBADD =						\
	$(LIBGLYR_LIBS) 							\
	$(GSTREAMER_LIBS)							\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)							\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpsong_info_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpsong_info_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpsong_info_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpsong_info_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = song-info.plugin

EXTRA_DIST = $(plugin_DATA)
