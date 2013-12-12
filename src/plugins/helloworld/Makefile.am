plugindir = $(libdir)/pragha/plugins/helloworld

INCLUDES = \
	-I$(top_srcdir) 	\
	$(LIBPEAS_CFLAGS)		\
	$(LIBPEAS_GTK_CFLAGS)

plugin_LTLIBRARIES = libhelloworld.la

libhelloworld_la_SOURCES = \
	peasdemo-hello-world-plugin.h		\
	peasdemo-hello-world-plugin.c		\
	peasdemo-hello-world-configurable.h	\
	peasdemo-hello-world-configurable.c

libhelloworld_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libhelloworld_la_LIBADD  = $(PIBPEAS_LIBS) $(LIBPEAS_GTK_LIBS)

plugin_DATA = helloworld.plugin

EXTRA_DIST = $(plugin_DATA)
