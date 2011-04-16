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
	data/consonance.png

desktopdir = $(datadir)/applications
desktop_DATA = data/consonance.desktop

EXTRA_DIST = 		\
	$(text_DATA)	\
	$(doc_DATA)	\
	$(image_DATA)	\
	$(desktop_DATA)	\
	$(man_MANS)
