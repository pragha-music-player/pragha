plugindir = $(libdir)/pragha/plugins/tunein

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libtunein.la

libtunein_la_SOURCES =      \
	pragha-tunein-plugin.c

libtunein_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libtunein_la_CFLAGS =						\
	$(LIBSOUP_CFLAGS)						\
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

libtunein_la_LIBADD =						\
	$(LIBSOUP_LIBS) 						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libtunein_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libtunein_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libtunein_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libtunein_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = tunein.plugin

EXTRA_DIST = $(plugin_DATA)
