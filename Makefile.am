SUBDIRS = \
	data  \
	po    \
	src

if HAVE_LIBPEAS
SUBDIRS += plugins
endif

AUTOMAKE_OPTIONS = \
	1.8            \
	dist-bzip2

doc_DATA = ChangeLog FAQ NEWS README

EXTRA_DIST =     \
	$(text_DATA) \
	$(doc_DATA)

transifex-setup:
	tx init
	tx set --auto-remote https://www.transifex.com/projects/p/Pragha/
	tx set --auto-local -r Pragha.Pragha 'po/<lang>.po' --source-lang en --source-file po/pragha.pot --execute

transifex-pull:
	tx pull -a -f
	@echo "You can now git commit -a -m 'Transfix pull, *.po update'"

transifex-push:
	tx push -s
	@echo "You can now git commit -a -m 'Transfix push, pragha.pot update'"

distclean-local:
	rm -rf *.cache *~
