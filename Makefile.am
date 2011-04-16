SUBDIRS = src

man_MANS = data/consonance.1

docdir= $(pkgdatadir)/doc
doc_DATA = README ChangeLog FAQ

imagedir = $(pkgdatadir)/data
image_DATA = 		\
	data/artist.png	\
	data/album.png	\
	data/track.png  \
	data/cover.png  \
	data/genre.png	\
	data/consonance.png	\
	data/bookmark_starred.png \
	data/bookmark_unstarred.png

iconsdir = $(datadir)/icons/hicolor/scalable/apps
icons_DATA = data/consonance.png

desktopdir = $(datadir)/applications
desktop_DATA = data/consonance.desktop

EXTRA_DIST = 		\
	$(text_DATA)	\
	$(doc_DATA)	\
	$(image_DATA)	\
	$(desktop_DATA)	\
	$(icons_DATA)	\
	$(man_MANS)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@-if test -z "$(DESTDIR)"; then					\
		echo "Updating Gtk icon cache.";			\
		$(gtk_update_icon_cache);				\
	else								\
		echo "*** Icon cache not updated. Remember to run:";	\
		echo "***";						\
		echo "***   $(gtk_update_icon_cache)";			\
		echo "***";						\
	fi
