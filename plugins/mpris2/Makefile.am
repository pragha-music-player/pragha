plugindir = $(libdir)/pragha/plugins/mpris2

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpmpris2.la

libpmpris2_la_SOURCES =      \
	pragha-mpris2-plugin.h \
	pragha-mpris2-plugin.c

libpmpris2_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpmpris2_la_CFLAGS =						\
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

libpmpris2_la_LIBADD =						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpmpris2_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpmpris2_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpmpris2_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpmpris2_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = mpris2.plugin

EXTRA_DIST = $(plugin_DATA)
