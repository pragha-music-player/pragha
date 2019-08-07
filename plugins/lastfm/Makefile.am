plugindir = $(libdir)/pragha/plugins/lastfm

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libplastfm.la

libplastfm_la_SOURCES =      \
	pragha-lastfm-plugin.c

libplastfm_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libplastfm_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBCLASTFM_CFLAGS)

libplastfm_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBCLASTFM_LIBS) \
	$(top_builddir)/src/libpragha.la

pragha-lastfm-menu-ui.h: pragha-lastfm-menu.ui Makefile
	$(AM_V_GEN) xdt-csource --strip-comments --strip-content --static --name=lastfm_menu_ui $< > $@

plugin_DATA = lastfm.plugin

EXTRA_DIST = \
	$(plugin_DATA) \
	pragha-lastfm-menu.ui

DISTCLEANFILES = \
	pragha-lastfm-menu-ui.h

BUILT_SOURCES = \
	pragha-lastfm-menu-ui.h
