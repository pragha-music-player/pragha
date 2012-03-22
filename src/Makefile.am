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
		glyr-related.c \
		init.c \
		jumptotrack.c \
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
		xml_helper.c \
		xml_helper.h \
		equalizer.c

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
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
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
	$(LIBCDDB_LIBS)

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

if HAVE_LIBGLYR
pragha_CFLAGS += $(LIBGLYR_CFLAGS)
pragha_LDFLAGS += $(LIBGLYR_LIBS)
endif

if HAVE_EXO
pragha_CFLAGS += $(EXO_CFLAGS)
pragha_LDFLAGS += $(EXO_LIBS)
endif
