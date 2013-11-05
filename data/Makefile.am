appdatadir = $(datadir)/appdata
dist_appdata_DATA = pragha.appdata.xml

desktopdir=$(datadir)/applications
desktop_in_files = pragha.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

imagedir = $(datadir)/pixmaps/pragha
image_DATA =   \
	album.png  \
	artist.png \
	track.png  \
	cover.png  \
	genre.png  \
	pragha.png

iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = pragha.png

man_MANS = pragha.1

EXTRA_DIST =            \
	$(desktop_in_files) \
	$(icons_DATA)       \
	$(image_DATA)       \
	$(man_MANS)

CLEANFILES = \
	$(desktop_DATA)

distuninstallcheck_listfiles = \
	find . -type f -print | grep -v ./share/icons/hicolor/icon-theme.cache

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then					\
		echo "Updating Gtk icon cache.";			\
		$(gtk_update_icon_cache);				\
	else								\
		echo "*** Icon cache not updated. Remember to run:";	\
		echo "***";						\
		echo "***   $(gtk_update_icon_cache)";			\
		echo "***";						\
	fi

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
