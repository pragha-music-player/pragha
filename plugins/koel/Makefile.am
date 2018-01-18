plugindir = $(libdir)/pragha/plugins/koel

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpkoel.la

libpkoel_la_SOURCES =		\
	pragha-koel-plugin.h 	\
	pragha-koel-plugin.c

libpkoel_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpkoel_la_CFLAGS =						\
	$(LIBSOUP_CFLAGS)						\
	$(JSON_GLIB_CFLAGS)						\
	$(GSTREAMER_CFLAGS)						\
	$(GSTREAMER_BASE_CFLAGS)				\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 					\
	-I$(top_srcdir)/src/include 			\
	-Wall

libpkoel_la_LIBADD =						\
	$(LIBSOUP_LIBS)							\
	$(JSON_GLIB_LIBS)						\
	$(GSTREAMER_LIBS)						\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)

if HAVE_GSTREAMER_AUDIO
libpkoel_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libpkoel_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libpkoel_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libpkoel_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = koel.plugin

EXTRA_DIST = $(plugin_DATA)
