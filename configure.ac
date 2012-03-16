dnl configure.ac
dnl
dnl Pragha - A Lightweight Music Player for GNU/Linux
dnl

dnl Version information
m4_define([pragha_version_major], [1])
m4_define([pragha_version_minor], [0])
m4_define([pragha_version_micro], [2.git])
m4_define([pragha_version], [pragha_version_major().pragha_version_minor().pragha_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2009-2012
        Matias De lellis <mati86dl@gmail.com>])
AC_INIT([pragha], [pragha_version()], [mati86dl@gmail.com])


dnl  Initialize automake

AM_INIT_AUTOMAKE([no-dist-gzip dist-bzip2])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE()

dnl *********************
dnl * Debugging support *
dnl *********************
AC_ARG_ENABLE(debug,[  --enable-debug    Turn on debugging],[
	case "${enableval}" in
	   yes)	debug=true ;
		CFLAGS="$CFLAGS -DDEBUG -D_DEBUG -g -ggdb -O0 -fno-inline -fno-inline-functions" ;;
	    no)	debug=false ;
		CFLAGS="$CFLAGS -DNDEBUG" ;;
	     *)	AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	esac
],[	debug=false
	CFLAGS="$CFLAGS -DNDEBUG"
])

dnl experimental support for gtk3
AC_ARG_WITH([gtk3],
            [AS_HELP_STRING([--with-gtk3],
              [enable experimental support for gtk3])],
            [CFLAGS="$CFLAGS -Wno-deprecated-declarations"],
            [with_gtk3=no])

AC_SUBST(CFLAGS)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Check for basic programs
AC_PROG_CC()
AC_PROG_INSTALL()
AC_PROG_LIBTOOL()

dnl Check for i18n support
XDT_I18N([cs de es el fr hu it ja nl no pl pt pt_BR ru sv tr uk zh_CN])

dnl Check for required packages
XDT_CHECK_PACKAGE([GSTREAMER], [gstreamer-0.10], [0.10])
XDT_CHECK_PACKAGE([GSTREAMER_BASE], [gstreamer-base-0.10], [0.10])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.24])
AS_IF([test "x$with_gtk3" != xno],
	[XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.0.0])],
	[XDT_CHECK_PACKAGE([GTK], [gtk+-2.0], [2.20.0])])
XDT_CHECK_PACKAGE([SQLITE3], [sqlite3], [3.4])
XDT_CHECK_PACKAGE([DBUS], [dbus-1], [1.1])
XDT_CHECK_PACKAGE([DBUS_GLIB], [dbus-glib-1], [0.84])
XDT_CHECK_PACKAGE([LIBNOTIFY], [libnotify], [0.4.4])
XDT_CHECK_PACKAGE([LIBCDIO], [libcdio], [0.80])
XDT_CHECK_PACKAGE([LIBCDIO_PARANOIA], [libcdio_paranoia], [0.80])
XDT_CHECK_PACKAGE([LIBCDDB], [libcddb], [1.3.0])
XDT_CHECK_PACKAGE([LIBCURL], [libcurl], [7.19.2])
XDT_CHECK_PACKAGE([LIBX11], [x11], 1.0.0)
XDT_CHECK_PACKAGE([TAGLIB], [taglib], [1.4])

dnl Check for taglib_c headers.
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

dnl Check min taglib to add monkeyaudio support
TAGLIB_1_7_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([TAGLIB_1_7],
                       [taglib], [1.7.0],
                       [taglib],
                       [taglib library to APE], [yes])

dnl Check gstreamer-plugins-base
GSTREAMER_PLUGINS_BASE_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([GSTREAMER_PLUGINS_BASE],
                       [gstreamer-plugins-base-0.10], [0.10],
                       [gstreamer-plugins-base-0.10],
                       [gstreamer-plugins-base-0.10], [yes])

dnl Check global keyboard shortcuts support
LIBKEYBINDER_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBKEYBINDER],
                       [keybinder], [0.2.0],
                       [keybinder],
                       [keybinder library], [yes])

dnl Check libclastfm
LIBCLASTFM_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBCLASTFM],
                       [libclastfm], [0.5],
                       [libclastfm],
                       [libclastfm library], [yes])

dnl Check libglyr
LIBGLYR_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBGLYR],
                       [libglyr], [0.9.4],
                       [libglyr],
                       [libglyr library], [yes])

dnl Check exo
EXO_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([EXO],
                       [exo-1], [0.6.0],
                       [exo-1],
                       [exo-1 library], [yes])

AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
])

case "$debug" in
     "true" )
	AC_MSG_RESULT([Debugging enabled, optimizations disabled.]) ;;
esac
 
AC_MSG_RESULT([CFLAGS = $CFLAGS]) 

echo ""
echo "Build Configuration for $PACKAGE version $VERSION"
echo ""
echo "  Prefix.................................................................: ${prefix}"
echo "  Cubic volume support...............(Need gstreamer-plugins-base-devel).: $GSTREAMER_PLUGINS_BASE_FOUND"
echo "  Global keyboard shortcuts support............(Need keybinder >= 0.2.0).: $LIBKEYBINDER_FOUND"
echo "  Lastfm scroble, and add similar songs support (Need libclastfm >= 0.5).: $LIBCLASTFM_FOUND"
echo "  Better session management support................(Need exo-1 >= 0.6.0).: $EXO_FOUND"
echo "  Search lyrics, artists info and albums art.....(Need libglyr >= 0.9.4).: $LIBGLYR_FOUND"
echo ""

