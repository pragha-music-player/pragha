plugindir = $(libdir)/pragha/plugins/keybinder

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libkeybinder.la

libkeybinder_la_SOURCES =      \
	pragha-keybinder-plugin.h \
	pragha-keybinder-plugin.c

libkeybinder_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libkeybinder_la_CFLAGS =						\
	$(LIBKEYBINDER_CFLAGS)						\
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

libkeybinder_la_LIBADD =						\
	$(LIBKEYBINDER_LIBS) 						\
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
libkeybinder_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libkeybinder_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
libkeybinder_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libkeybinder_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

plugin_DATA = keybinder.plugin

EXTRA_DIST = $(plugin_DATA)
