bin_PROGRAMS = pragha

INCLUDES =								\
	-DDBUS_API_SUBJECT_TO_CHANGE					\
	-DG_LOG_DOMAIN=\"pragha\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/pragha"\"			\
	-DDESKTOPENTRY=\""$(datadir)/applications/pragha.desktop"\"	\
	-I$(top_srcdir)

pragha_SOURCES =cdda.c \
		cdda.h \
		cmdline.c \
		current-playlist.c \
		db.c \
		dbus.c \
		filetree.c \
		gui.c \
		init.c \
		librarytree.c \
		menu.c \
		mpris.c \
		musicobject.c \
		panel.c \
		playlist.c \
		pragha.c \
		pragha.h \
		pragha-backend.c \
		pref.c \
		statusicon.c \
		tags.c \
		utils.c \
		gtkcellrendererbubble.c \
		gtkcellrendererbubble.h \
		pragha-lastfm.c \
		pragha-chartlyrics.c \
		chartlyrics_helper.c \
		chartlyrics_helper.h

pragha_CFLAGS =	$(GSTREAMER_CFLAGS)					\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_CFLAGS) 						\
	$(TAGLIBC_CFLAGS) 						\
	$(DBUS_CFLAGS) 							\
	$(DBUS_GLIB_CFLAGS) 						\
	$(LIBNOTIFY_CFLAGS) 						\
	$(LIBX11_CFLAGS)						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	$(LIBCURL_CFLAGS)						\
	-I$(top_srcdir)/src/include 					\
	-Wall

pragha_LDFLAGS = $(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_LIBS)							\
	$(TAGLIBC_LIBS)							\
	$(DBUS_LIBS) 							\
	$(DBUS_GLIB_LIBS)						\
	$(LIBNOTIFY_LIBS) 						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS) 						\
	$(LIBCURL_LIBS)							\
	$(LIBX11_LIBS)

if HAVE_GSTREAMER_PLUGINS_BASE
pragha_CFLAGS += $(GSTREAMER_BASE_CFLAGS)
pragha_LDFLAGS += $(GSTREAMER_BASE_LIBS)				\
	$(GSTREAMER_PLUGINS_BASE_LIBS) -lgstinterfaces-0.10
endif

if HAVE_LIBKEYBINDER
pragha_CFLAGS += $(LIBKEYBINDER_CFLAGS)
pragha_LDFLAGS += $(LIBKEYBINDER_LIBS)
endif

if HAVE_LIBCLASTFM
pragha_CFLAGS += $(LIBCLASTFM_CFLAGS)
pragha_LDFLAGS += $(LIBCLASTFM_LIBS)
endif



