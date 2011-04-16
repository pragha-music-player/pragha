AC_INIT([consonance], [0.5.91], [m.sujith@gmail.com])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR([src/consonance.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC

# Check user options.
AC_ARG_ENABLE(modplug,
	[AS_HELP_STRING([--enable_modplug], [support modplug (default: no)])],
	[enable_modplug=$enableval],
	[enable_modplug=no])

# Checks for libraries.
PKG_CHECK_MODULES([alsa],
		  [alsa >= 1.0.15],
		  [],
		  [AC_MSG_ERROR(alsa-1.0.15 is required)])

PKG_CHECK_MODULES([ao],
                  [ao >= 0.8.8],
		  [],
                  [AC_MSG_ERROR(ao-0.8.8 is required)])

PKG_CHECK_MODULES([flac],
                  [flac >= 1.2.1],
		  [],
                  [AC_MSG_ERROR(libflac-1.2.1 is required)])

PKG_CHECK_MODULES([gthread],
                  [gthread-2.0 >= 2.14.0],
		  [],
                  [AC_MSG_ERROR(gthread-2.14.0 is required)])

PKG_CHECK_MODULES([gio],
                  [gio-2.0 >= 2.16.3],
		  [],
                  [AC_MSG_ERROR(gio-2.16.3 is required)])

PKG_CHECK_MODULES([gtk],
                  [gtk+-2.0 >= 2.10.14],
		  [],
                  [AC_MSG_ERROR(gtk-2.10.14 is required)])

PKG_CHECK_MODULES([taglib],
                  [taglib >= 1.4],
		  [],
                  [AC_MSG_ERROR(taglib-1.4-2 is required)])

PKG_CHECK_MODULES([mad],
                  [mad >= 0.15],
		  [],
                  [AC_MSG_ERROR(libmad-0.15 is required)])

PKG_CHECK_MODULES([sqlite3],
                  [sqlite3 >= 3.4],
		  [],
                  [AC_MSG_ERROR(sqlite-3.4 is required)])

PKG_CHECK_MODULES([dbus],
                  [dbus-glib-1 >= 0.74],
		  [],
                  [AC_MSG_ERROR(dbus-glib-1 is required)])

PKG_CHECK_MODULES([libnotify],
                  [libnotify >= 0.4.4],
		  [],
                  [AC_MSG_ERROR(libnotify-0.4.4 is required)])

PKG_CHECK_MODULES([sndfile],
                  [sndfile >= 1.0.17],
		  [],
                  [AC_MSG_ERROR(libsndfile-1.0.17 is required)])

PKG_CHECK_MODULES([vorbisfile],
                  [vorbisfile >= 1.2.0],
		  [],
                  [AC_MSG_ERROR(vorbisfile-1.1.3 is required)])

PKG_CHECK_MODULES([libcurl],
                  [libcurl >= 7.18],
		  [],
                  [AC_MSG_ERROR(curl-7.18 is required)])

PKG_CHECK_MODULES([libcdio],
                  [libcdio >= 0.78],
		  [],
                  [AC_MSG_ERROR(libcdio-0.78 is required)])

PKG_CHECK_MODULES([libcdio_paranoia],
                  [libcdio_paranoia >= 0.78],
		  [],
                  [AC_MSG_ERROR(libcdio_paranoia-0.78 is required)])

PKG_CHECK_MODULES([libcddb],
                  [libcddb >= 1.2.1],
		  [],
                  [AC_MSG_ERROR(libcddb-1.3.0 is required)])

PKG_CHECK_MODULES([libmodplug],
                  [libmodplug >= 0.7.0],
		  [],
                  [AC_MSG_ERROR(libmodplug-0.8.0 is required)])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
