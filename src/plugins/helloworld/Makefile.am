plugindir = $(libdir)/pragha/plugins/helloworld

INCLUDES = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libhelloworld.la

libhelloworld_la_SOURCES = \
	peasdemo-hello-world-plugin.h		\
	peasdemo-hello-world-plugin.c		\
	peasdemo-hello-world-configurable.h	\
	peasdemo-hello-world-configurable.c

libhelloworld_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libhelloworld_la_CFLAGS =	$(GSTREAMER_CFLAGS)					\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(LIBNOTIFY_CFLAGS) 						\
	$(LIBPEAS_CFLAGS) 						\
	$(LIBPEAS_GTK_CFLAGS) 						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

libhelloworld_la_LIBADD = $(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(LIBNOTIFY_LIBS) 						\
	$(LIBPEAS_LIBS)							\
	$(LIBPEAS_GTK_LIBS)						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS)

if HAVE_GSTREAMER_AUDIO
libhelloworld_la_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
libhelloworld_la_LIBADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBKEYBINDER
libhelloworld_la_CFLAGS += $(LIBKEYBINDER_CFLAGS)
libhelloworld_la_LIBADD += $(LIBKEYBINDER_LIBS)
endif

if HAVE_LIBCLASTFM
libhelloworld_la_CFLAGS += $(LIBCLASTFM_CFLAGS)
libhelloworld_la_LIBADD += $(LIBCLASTFM_LIBS)
endif

if HAVE_LIBGLYR
libhelloworld_la_CFLAGS += $(LIBGLYR_CFLAGS)
libhelloworld_la_LIBADD += $(LIBGLYR_LIBS)
endif

if HAVE_LIBXFCE4UI
libhelloworld_la_CFLAGS += $(LIBXFCE4UI_CFLAGS)
libhelloworld_la_LIBADD += $(LIBXFCE4UI_LIBS)
endif

if HAVE_PLPARSER
libhelloworld_la_CFLAGS += $(PLPARSER_CFLAGS)
libhelloworld_la_LIBADD += $(PLPARSER_LIBS)
endif

plugin_DATA = helloworld.plugin

EXTRA_DIST = $(plugin_DATA)
