DEFINES = -DSHAREDIR=\""$(pkgdatadir)"\"

bin_PROGRAMS = consonance

consonance_CFLAGS = $(DEFINES)

consonance_LDADD = $(glib_LIBS) \
	$(gthread_LIBS) \
	$(gio_LIBS) \
	$(gtk_LIBS) \
	$(libxfcegui4_LIBS) \
	$(libxfceutil4_LIBS) \
	$(libexo_LIBS) \
	$(mad_LIBS) \
	$(flac_LIBS) \
	$(sqlite3_LIBS) \
	$(ao_LIBS) \
	$(alsa_LIBS) \
	$(dbus_LIBS) \
	$(libnotify_LIBS) \
	$(sndfile_LIBS) \
	$(vorbisfile_LIBS) \
	$(libmodplug_LIBS) \
	$(libcurl_LIBS) \
	$(libcdio_LIBS) \
	$(libcdio_paranoia_LIBS) \
	$(libcddb_LIBS) \
	-ltag_c

AM_CPPFLAGS = $(glib_CFLAGS) \
	$(gio_CFLAGS) \
	$(gtk_CFLAGS) \
	$(libxfcegui4_CFLAGS) \
	$(libxfceutil4_CFLAGS) \
	$(libexo_CFLAGS) \
	$(mad_CFLAGS) \
	$(flac_CFLAGS) \
	$(sqlite3_CFLAGS) \
	$(taglib_CFLAGS) \
	$(ao_CFLAGS) \
	$(alsa_CFLAGS) \
	$(dbus_CFLAGS) \
	$(libnotify_CFLAGS) \
	$(sndfile_CFLAGS) \
	$(vorbisfile_CFLAGS) \
	$(libcurl_CFLAGS) \
	$(libcdio_CFLAGS) \
	$(libcddb_CFLAGS) \
	-I$(top_srcdir)/src/include \
	-Wall

consonance_SOURCES = consonance.c \
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
		eggtrayicon.c \
		eggtrayicon.h \
		sexy-icon-entry.c \
		sexy-icon-entry.h \
		sexy-marshal.c \
		sexy-marshal.h \
		consonance.h \
		md5.h \
		wav.h \
		oggvorbis.h \
		mp3.h \
		mod.h \
		flac.h \
		cdda.h
