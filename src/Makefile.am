bin_PROGRAMS = pragha

AM_CPPFLAGS =								\
	-DG_LOG_DOMAIN=\"pragha\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/pragha"\"			\
	-DDESKTOPENTRY=\""$(datadir)/applications/pragha.desktop"\"	\
	-DLIBPLUGINDIR=\""$(libdir)/pragha/plugins/"\"	\
	-DUSRPLUGINDIR=\""$(prefix)/share/pragha/plugins"\"	\
	-I$(top_srcdir)

pragha_SOURCES = \
	gtkcellrendererbubble.c \
	gtkcellrendererbubble.h \
	info-bar-import-music.c \
	pragha-album-art.c \
	pragha-album-art.h \
	pragha-art-cache.c \
	pragha-art-cache.h \
	pragha-backend.c \
	pragha-backend.h \
	pragha-cmdline.c \
	pragha-database.c \
	pragha-database.h \
	pragha-debug.c \
	pragha-debug.h \
	pragha-dnd.c \
	pragha-dnd.h \
	pragha-equalizer-dialog.c \
	pragha-equalizer-dialog.h \
	pragha-file-utils.c \
	pragha-file-utils.h \
	pragha-filter-dialog.c \
	pragha-filter-dialog.h \
	pragha-hig.c \
	pragha-hig.h \
	pragha-library-pane.c \
	pragha-library-pane.h \
	pragha-menubar.c \
	pragha-menubar.h \
	pragha-music-enum.c \
	pragha-music-enum.h \
	pragha-musicobject.c \
	pragha-musicobject.h \
	pragha-musicobject-mgmt.c \
	pragha-musicobject-mgmt.h \
	pragha-playback.c \
	pragha-playback.h \
	pragha-playlist.c \
	pragha-playlist.h \
	pragha-playlists-mgmt.c \
	pragha-playlists-mgmt.h \
	pragha-preferences.c \
	pragha-preferences.h \
	pragha-preferences-dialog.c \
	pragha-preferences-dialog.h \
	pragha-prepared-statement.c \
	pragha-prepared-statement.h \
	pragha-prepared-statement-private.h \
	pragha-scanner.c \
	pragha-scanner.h \
	pragha-search-entry.c \
	pragha-search-entry.h \
	pragha-session.c \
	pragha-session.h \
	pragha-sidebar.c \
	pragha-sidebar.h \
	pragha-simple-async.c \
	pragha-simple-async.h \
	pragha-simple-widgets.c \
	pragha-simple-widgets.h \
	pragha-statusbar.c \
	pragha-statusbar.h \
	pragha-statusicon.c \
	pragha-statusicon.h \
	pragha-tagger.c \
	pragha-tagger.h \
	pragha-tags-dialog.c \
	pragha-tags-dialog.h \
	pragha-tags-mgmt.c \
	pragha-tags-mgmt.h \
	pragha-toolbar.c \
	pragha-toolbar.h \
	pragha-utils.c \
	pragha-utils.h \
	pragha-window.c \
	pragha-window.h \
	pragha.c \
	pragha.h \
	xml_helper.c \
	xml_helper.h

pragha_CFLAGS =	$(GSTREAMER_CFLAGS)					\
	$(GSTREAMER_BASE_CFLAGS)					\
	$(GLIB_CFLAGS) 							\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_C_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall -lm

pragha_LDADD = $(GSTREAMER_LIBS)					\
	$(GLIB_LIBS) 							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_C_LIBS)

if HAVE_GSTREAMER_AUDIO
pragha_CFLAGS += $(GSTREAMER_AUDIO_CFLAGS)
pragha_LDADD += $(GSTREAMER_AUDIO_LIBS)
endif

if HAVE_LIBXFCE4UI
pragha_CFLAGS += $(LIBXFCE4UI_CFLAGS)
pragha_LDADD += $(LIBXFCE4UI_LIBS)
endif

if HAVE_PLPARSER
pragha_CFLAGS += $(PLPARSER_CFLAGS)
pragha_LDADD += $(PLPARSER_LIBS)
endif

if HAVE_LIBPEAS
pragha_SOURCES += \
	pragha-plugins-engine.c \
	pragha-plugins-engine.h

pragha_CFLAGS += $(LIBPEAS_CFLAGS)
pragha_LDADD += $(LIBPEAS_LIBS)

pragha_CFLAGS += $(LIBPEAS_GTK_CFLAGS)
pragha_LDADD += $(LIBPEAS_GTK_LIBS)
endif

if WIN32
pragha_SOURCES += \
	$(top_srcdir)/win32/win32dep.c \
	$(top_srcdir)/win32/win32dep.h

AM_CFLAGS = \
	-mms-bitfields

AM_LDFLAGS = \
	-Wl,-subsystem,windows

pragha-res.o: $(top_srcdir)/win32/pragha.rc
	$(WINDRES) -i $(top_srcdir)/win32/pragha.rc --input-format=rc -o pragha-res.o -O coff

pragha_LDADD += pragha-res.o
endif
