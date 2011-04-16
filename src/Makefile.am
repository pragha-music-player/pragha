INCLUDES =								\
	-DDBUS_API_SUBJECT_TO_CHANGE					\
	-DG_LOG_DOMAIN=\"pragha\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DSHAREDIR=\""$(pkgdatadir)"\"					\
	-I$(top_srcdir)

bin_PROGRAMS = pragha

pragha_CFLAGS = $(DEFINES)

pragha_SOURCES =pragha.c \
		init.c \
		gui.c \
		menu.c \
		utils.c \
		musicobject.c \
		db.c \
		librarytree.c \
		panel.c \
		filetree.c \
		current_playlist.c \
		tags.c \
		playback_mp3.c \
		playback_wav.c \
		playback_flac.c \
		playback_oggvorbis.c \
		playback_modplug.c \
		playback_cdda.c \
		pref.c \
		threads.c \
		audio.c \
		statusicon.c \
		cmdline.c \
		dbus.c \
		playlist.c \
		md5.c \
		lastfm.c \
		pragha.h \
		md5.h \
		wav.h \
		oggvorbis.h \
		mp3.h \
		mod.h \
		flac.h \
		cdda.h

pragha_CFLAGS =	$(GLIB_CFLAGS) 						\
	$(GTHREAD_CFLAGS)						\
	$(GIO_CFLAGS) 							\
	$(GTK_CFLAGS) 							\
	$(MAD_CFLAGS) 							\
	$(FLAC_CFLAGS) 							\
	$(SQLITE3_CFLAGS) 						\
	$(TAGLIB_CFLAGS) 						\
	$(TAGLIBC_CFLAGS) 						\
	$(AO_CFLAGS) 							\
	$(ALSA_CFLAGS) 							\
	$(DBUS_CFLAGS) 							\
	$(LIBNOTIFY_CFLAGS) 						\
	$(SNDFILE_CFLAGS) 						\
	$(VORBISFILE_CFLAGS) 						\
	$(LIBCURL_CFLAGS) 						\
	$(LIBCDIO_CFLAGS) 						\
	$(LIBCDDB_CFLAGS) 						\
	-I$(top_srcdir)/src/include 					\
	-Wall

pragha_LDFLAGS = $(GLIB_LIBS) 						\
	$(GTHREAD_LIBS)							\
	$(GIO_LIBS) 							\
	$(GTK_LIBS) 							\
	$(MAD_LIBS) 							\
	$(FLAC_LIBS) 							\
	$(SQLITE3_LIBS)							\
	$(TAGLIB_LIBS)							\
	$(TAGLIBC_LIBS)							\
	$(AO_LIBS) 							\
	$(ALSA_LIBS) 							\
	$(DBUS_LIBS) 							\
	$(LIBNOTIFY_LIBS) 						\
	$(SNDFILE_LIBS) 						\
	$(VORBISFILE_LIBS) 						\
	$(LIBMODPLUG_LIBS) 						\
	$(LIBCURL_LIBS) 						\
	$(LIBCDIO_LIBS) 						\
	$(LIBCDIO_PARANOIA_LIBS) 					\
	$(LIBCDDB_LIBS)
