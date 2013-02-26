bin_PROGRAMS = pragha

AM_CPPFLAGS =								\
	-DDBUS_API_SUBJECT_TO_CHANGE					\
	-DG_LOG_DOMAIN=\"pragha\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/pragha"\"			\
	-DDESKTOPENTRY=\""$(datadir)/applications/pragha.desktop"\"	\
	-I$(top_srcdir)

pragha_SOURCES =cdda.c \
		cdda.h \
		cmdline.c \
		dbus.c \
		file-utils.c \
		gui.c \
		glyr-related.c \
		init.c \
		info-bar-import-music.c \
		menu.c \
		mpris.c \
		gnome-media-keys.c \
		musicobject.c \
		notify.c \
		panel.c \
		playlist.c \
		pragha.c \
		pragha.h \
		pragha-album-art.c \
		pragha-album-art.h \
		pragha-backend.c \
		pragha-backend.h \
		pragha-database.c \
		pragha-database.h \
		pragha-filter-dialog.c \
		pragha-gst-metadata.c \
		pragha-gst-metadata.h \
		pragha-hig.c \
		pragha-library-pane.c \
		pragha-musicobject.c \
		pragha-musicobject.h \
		pragha-playback.c \
		pragha-playlist.c \
		pragha-preferences.c \
		pragha-preferences.h \
		pragha-prepared-statement.c \
		pragha-prepared-statement.h \
		pragha-prepared-statement-private.h \
		pragha-scanner.c \
		pragha-scanner.h \
		pragha-sidebar.c \
		pragha-simple-async.c \
		pragha-statusbar.c \
		pragha-statusbar.h \
		pref.c \
		statusicon.c \
		tags.c \
		utils.c \
		gtkcellrendererbubble.c \
		gtkcellrendererbubble.h \
		pragha-lastfm.c \
		xml_helper.c \
		xml_helper.h \
		equalizer.c \
		widgets.c

pragha_CFLAGS =	$(GSTREAMER_CFLAGS)					\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	$(DBUS_CFLAGS) 							\
	$(DBUS_GLIB_CFLAGS) 						\
	$(LIBNOTIFY_CFLAGS) 						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

pragha_LDADD = $(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)						\
	$(DBUS_LIBS) 							\
	$(DBUS_GLIB_LIBS)						\
	$(LIBNOTIFY_LIBS) 						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS)

## for gstreamer 0.10
if HAVE_GSTREAMER_INTERFACES
pragha_CFLAGS += $(GSTREAMER_INTERFACES_CFLAGS)
pragha_LDADD += $(GSTREAMER_INTERFACES_LIBS)
endif

## for gstreamer 1.0
if HAVE_GSTREAMER_AUDIO
pragha_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
pragha_LDADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBKEYBINDER
pragha_SOURCES += keybinder.c
pragha_CFLAGS += $(LIBKEYBINDER_CFLAGS)
pragha_LDADD += $(LIBKEYBINDER_LIBS)
endif

if HAVE_LIBCLASTFM
pragha_CFLAGS += $(LIBCLASTFM_CFLAGS)
pragha_LDADD += $(LIBCLASTFM_LIBS)
endif

if HAVE_LIBGLYR
pragha_CFLAGS += $(LIBGLYR_CFLAGS)
pragha_LDADD += $(LIBGLYR_LIBS)
endif

if HAVE_LIBXFCE4UI
pragha_CFLAGS += $(LIBXFCE4UI_CFLAGS)
pragha_LDADD += $(LIBXFCE4UI_LIBS)
endif

if HAVE_PLPARSER
pragha_CFLAGS += $(PLPARSER_CFLAGS)
pragha_LDADD += $(PLPARSER_LIBS)
endif

