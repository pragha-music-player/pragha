plugindir = $(libdir)/pragha/plugins/song-info

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libsong-info.la

libsong_info_la_SOURCES =						\
	pragha-song-info-cache.c					\
	pragha-song-info-cache.h					\
	pragha-song-info-plugin.c					\
	pragha-song-info-plugin.h					\
	pragha-song-info-dialog.c					\
	pragha-song-info-dialog.h					\
	pragha-song-info-pane.c						\
	pragha-song-info-pane.h						\
	pragha-song-info-thread-albumart.c			\
	pragha-song-info-thread-albumart.h			\
	pragha-song-info-thread-artistart.c			\
	pragha-song-info-thread-artistart.h			\
	pragha-song-info-thread-dialog.c			\
	pragha-song-info-thread-dialog.h			\
	pragha-song-info-thread-pane.c				\
	pragha-song-info-thread-pane.h

libsong_info_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libsong_info_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBGLYR_CFLAGS)

libsong_info_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBGLYR_LIBS) \
	$(top_builddir)/src/libpragha.la


pragha-song-info-ui.h: pragha-song-info.ui Makefile
	$(AM_V_GEN) xdt-csource --strip-comments --strip-content --static --name=pragha_song_info_ui $< > $@

plugin_DATA = song-info.plugin

EXTRA_DIST = \
	$(plugin_DATA) \
	pragha-song-info.ui

DISTCLEANFILES = \
	pragha-song-info-ui.h

BUILT_SOURCES = \
	pragha-song-info-ui.h
