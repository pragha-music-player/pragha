SUBDIRS = icons

appdatadir = $(datadir)/metainfo
dist_appdata_DATA = io.github.pragha-music-player.metainfo.xml

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
	genre.png

man_MANS = pragha.1

EXTRA_DIST =            \
	$(desktop_in_files) \
	$(image_DATA)       \
	$(man_MANS)

CLEANFILES = \
	$(desktop_DATA)
