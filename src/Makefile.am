AM_CPPFLAGS =								\
	-DG_LOG_DOMAIN=\"pragha\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DPIXMAPDIR=\""$(datadir)/pixmaps/pragha"\"			\
	-DDESKTOPENTRY=\""$(datadir)/applications/pragha.desktop"\"	\
	-DLIBPLUGINDIR=\""$(libdir)/pragha/plugins/"\"	\
	-DUSRPLUGINDIR=\""$(prefix)/share/pragha/plugins"\"	\
	-DUSRSTYLEDIR=\""$(datadir)/pragha/style"\"	\
	-I$(top_srcdir)

EXTRA_DIST =
DISTCLEANFILES =

#
# Pragha library
#
pkglib_LTLIBRARIES = libpragha.la

libpraghadir = $(includedir)/libpragha
libpragha_HEADERS = \
	gtkcellrendererbubble.h \
	pragha-album-art.h \
	pragha-app-notification.h \
	pragha-app-notification-container.h \
	pragha-art-cache.h \
	pragha-backend.h \
	pragha-background-task-bar.h \
	pragha-background-task-widget.h \
	pragha-database-provider.h \
	pragha-database.h \
	pragha-debug.h \
	pragha-dnd.h \
	pragha-equalizer-dialog.h \
	pragha-favorites.h \
	pragha-file-utils.h \
	pragha-filter-dialog.h \
	pragha-library-pane.h \
	pragha-hig.h \
	pragha-menubar.h \
	pragha-music-enum.h \
	pragha-musicobject.h \
	pragha-musicobject-mgmt.h \
	pragha-playback.h \
	pragha-playlist.h \
	pragha-playlists-mgmt.h \
	pragha-preferences.h \
	pragha-preferences-dialog.h \
	pragha-prepared-statement.h \
	pragha-prepared-statement-private.h \
	pragha-provider.h \
	pragha-scanner.h \
	pragha-search-entry.h \
	pragha-session.h \
	pragha-sidebar.h \
	pragha-simple-async.h \
	pragha-simple-widgets.h \
	pragha-song-cache.h \
	pragha-statusbar.h \
	pragha-statusicon.h \
	pragha-tagger.h \
	pragha-tags-dialog.h \
	pragha-tags-mgmt.h \
	pragha-temp-provider.h \
	pragha-toolbar.h \
	pragha-utils.h \
	pragha-window.h \
	pragha.h \
	xml_helper.h

libpragha_la_SOURCES = \
	$(libpragha_HEADERS) \
	$(BUILT_SOURCES) \
	gtkcellrendererbubble.c \
	info-bar-import-music.c \
	pragha-album-art.c \
	pragha-app-notification.c \
	pragha-app-notification-container.c \
	pragha-art-cache.c \
	pragha-backend.c \
	pragha-background-task-bar.c \
	pragha-background-task-widget.c \
	pragha-cmdline.c \
	pragha-database-provider.c \
	pragha-database.c \
	pragha-debug.c \
	pragha-dnd.c \
	pragha-equalizer-dialog.c \
	pragha-favorites.c \
	pragha-file-utils.c \
	pragha-filter-dialog.c \
	pragha-hig.c \
	pragha-library-pane.c \
	pragha-menubar.c \
	pragha-music-enum.c \
	pragha-musicobject.c \
	pragha-musicobject-mgmt.c \
	pragha-playback.c \
	pragha-playlist.c \
	pragha-playlists-mgmt.c \
	pragha-preferences.c \
	pragha-preferences-dialog.c \
	pragha-prepared-statement.c \
	pragha-provider.c \
	pragha-scanner.c \
	pragha-search-entry.c \
	pragha-session.c \
	pragha-sidebar.c \
	pragha-simple-async.c \
	pragha-simple-widgets.c \
	pragha-song-cache.c \
	pragha-statusbar.c \
	pragha-statusicon.c \
	pragha-tagger.c \
	pragha-tags-dialog.c \
	pragha-tags-mgmt.c \
	pragha-temp-provider.c \
	pragha-toolbar.c \
	pragha-utils.c \
	pragha-window.c \
	pragha.c \
	xml_helper.c

libpragha_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	-I$(top_srcdir)/src/include \
	-Wall -lm

libpragha_la_LIBADD = \
	$(PRAGHA_LIBS)

if HAVE_LIBPEAS
libpragha_la_SOURCES += \
	pragha-plugins-engine.c
libpragha_HEADERS += \
	pragha-plugins-engine.h
endif

if HAVE_GUDEV
libpragha_la_SOURCES += \
	pragha-device-client.c
libpragha_HEADERS += \
	pragha-device-client.h
endif


#
# pkgconfig
#
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_in_files = libpragha.pc.in
pkgconfig_DATA = $(pkgconfig_in_files:.pc.in=.pc)

%.pc: %.pc.in
	sed "s,\@VERSION\@,$(VERSION),g; s,\@prefix\@,$(prefix),g; s,\@libdir\@,$(libdir),g; s,\@includedir\@,$(includedir),g" < $< > $@

EXTRA_DIST += \
	$(pkgconfig_in_files)

DISTCLEANFILES += \
	$(pkgconfig_DATA)


#
# WIN32
#
if WIN32
libpragha_la_SOURCES += \
	win32/win32dep.c
libpragha_HEADERS += \
	win32/win32dep.h

AM_CFLAGS = \
	-mms-bitfields

AM_LDFLAGS = \
	-Wl,-subsystem,windows
endif

libpragha_la_LDFLAGS = \
	-avoid-version \
	-export-dynamic \
	-no-undefined \
	-export-symbols-regex "^[^_].*"
# endif WIN32


#
# Build sources
#
pragha-window-ui.h: pragha-window.ui Makefile
	$(AM_V_GEN) xdt-csource --strip-comments --strip-content --static --name=pragha_window_ui $< > $@

BUILT_SOURCES = \
	pragha-window-ui.h

DISTCLEANFILES += \
	pragha-window-ui.h


#
# Pragha binary
#
bin_PROGRAMS = pragha

pragha_SOURCES = \
	main.c

pragha_CFLAGS = \
	$(libpragha_la_CFLAGS)

pragha_LDADD = \
	libpragha.la

if WIN32
pragha-res.o: win32/pragha.rc
	$(WINDRES) -i win32/pragha.rc --input-format=rc -o pragha-res.o -O coff
pragha_LDADD += pragha-res.o
endif

EXTRA_DIST += \
	pragha-window.ui
