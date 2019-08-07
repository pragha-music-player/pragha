plugindir = $(libdir)/pragha/plugins/mtp

AM_CPPFLAGS = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libpmtp.la

libpmtp_la_SOURCES = \
	pragha-devices-mtp.c \
	pragha-mtp-musicobject.c \
	pragha-mtp-musicobject.h \
	pragha-mtp-thread.c \
	pragha-mtp-thread.h \
	pragha-mtp-thread-data.c \
	pragha-mtp-thread-data.h

libpmtp_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

libpmtp_la_CFLAGS = \
	$(PRAGHA_CFLAGS) \
	$(LIBMTP_CFLAGS)

libpmtp_la_LIBADD = \
	$(PRAGHA_LIBS) \
	$(LIBMTP_LIBS) \
	$(top_builddir)/src/libpragha.la

plugin_DATA = mtp.plugin

EXTRA_DIST = $(plugin_DATA)
