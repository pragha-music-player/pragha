dnl Version information
m4_define([pragha_version_major], [0])
m4_define([pragha_version_minor], [6])
m4_define([pragha_version_micro], [3])
m4_define([pragha_version], [pragha_version_major().pragha_version_minor().pragha_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2009
        Matias De lellis <mati86dl@gmail.com>])
AC_INIT([pragha], [pragha_version()], [mati86dl@gmail.com])

dnl Initialize automake
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE()

dnl check for basic programs
AC_PROG_CC()
AC_PROG_INSTALL()
AC_PROG_LIBTOOL()

dnl Check for i18n support
XDT_I18N([cs de es ja pt ru uk])

dnl Check for required packages
XDT_CHECK_PACKAGE([ALSA], [alsa], [1.0.15])
XDT_CHECK_PACKAGE([AO], [ao], [0.8.8])
XDT_CHECK_PACKAGE([FLAC], [flac], [1.2.1])
XDT_CHECK_PACKAGE([GTHREAD], [gthread-2.0], [2.14.0])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.16.3])
XDT_CHECK_PACKAGE([GTK], [gtk+-2.0], [2.10.14])
XDT_CHECK_PACKAGE([TAGLIB], [taglib], [1.4])
XDT_CHECK_PACKAGE([MAD], [mad], [0.15])
XDT_CHECK_PACKAGE([SQLITE3], [sqlite3], [3.4])
XDT_CHECK_PACKAGE([DBUS], [dbus-1], [0.74])
XDT_CHECK_PACKAGE([LIBNOTIFY], [libnotify], [0.4.4])
XDT_CHECK_PACKAGE([SNDFILE], [sndfile >= 1.0.17], [1.0.17])
XDT_CHECK_PACKAGE([VORBISFILE], [vorbisfile], [1.2.0])
XDT_CHECK_PACKAGE([LIBCURL], [libcurl], [7.18])
XDT_CHECK_PACKAGE([LIBCDIO], [libcdio], [0.78])
XDT_CHECK_PACKAGE([LIBCDIO_PARANOIA], [libcdio_paranoia], [0.78])
XDT_CHECK_PACKAGE([LIBCDDB], [libcddb], [1.2.1])
XDT_CHECK_PACKAGE([LIBMODPLUG], [libmodplug], [0.7.0])

dnl ***********************************
dnl *** Check for taglib_c ***
dnl ***********************************
TAGLIBC_LIBS=""
TAGLIBC_CFLAGS=""
AC_CHECK_LIB([tag_c], [taglib_file_new],
[
 AC_CHECK_HEADER([taglib/tag_c.h],
 [
  TAGLIBC_LIBS="-ltag_c"
  AC_DEFINE([HAVE_TAGLIBC], [1], [Define to 1 if tag_c is found])
  ])
], [-lm])
AC_SUBST([TAGLIBC_CFLAGS])
AC_SUBST([TAGLIBC_LIBS]) 

AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
])

