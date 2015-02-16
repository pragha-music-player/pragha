plugindir = $(libdir)/pragha/plugins/dlna

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpdlna.la

libpdlna_la_SOURCES =      \
	pragha-dlna-plugin.h \
	pragha-dlna-plugin.c

libpdlna_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpdlna_la_CFLAGS =						\
	$(RYGEL_CFLAGS)						\
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

libpdlna_la_LIBADD =						\
	$(RYGEL_LIBS) 						\
	$(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpdlna_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpdlna_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpdlna_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpdlna_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = dlna.plugin

EXTRA_DIST = $(plugin_DATA)
