dnl configure.ac
dnl
dnl Pragha - A Lightweight Music Player for GNU/Linux
dnl

dnl Version information
m4_define([pragha_version_major], [1])
m4_define([pragha_version_minor], [2])
m4_define([pragha_version_micro], [rc])
m4_define([pragha_version], [pragha_version_major().pragha_version_minor().pragha_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2009-2013
        Matias De lellis <mati86dl@gmail.com>])
AC_INIT([pragha], [pragha_version()], [mati86dl@gmail.com])
AC_CONFIG_HEADERS([config.h])

dnl  Initialize automake
AM_INIT_AUTOMAKE([no-dist-gzip dist-bzip2])
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

dnl gstreamer version
AC_ARG_WITH([gstreamer],
            [AS_HELP_STRING([--with-gstreamer=0.10|1.0],[which gstreamer version to compile against (default: 1.0)])],
            [case "$with_gstreamer" in
                0.10|1.0) ;;
                *) AC_MSG_ERROR([invalid gstreamer version specified]) ;;
             esac],
            [with_gstreamer=1.0]
)

case "$with_gstreamer" in
  0.10) GSTREAMER_API_VERSION=0.10
       GSTREAMER_REQUIRED=0.10
       ;;
  1.0) GSTREAMER_API_VERSION=1.0
       GSTREAMER_REQUIRED=0.11.90
       ;;
esac
AC_DEFINE_UNQUOTED(GSTREAMER_API_VERSION, $GSTREAMER_API_VERSION, [Define the GStreamer+ API version])

AC_SUBST(CFLAGS)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Check for basic programs
AC_PROG_CC()
AM_PROG_CC_C_O()
AC_PROG_INSTALL()
IT_PROG_INTLTOOL()

dnl Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl Check for i18n support
XDT_I18N([cs de es el fr hu it ja nl no pl pt pt_BR ru sv tr uk zh_CN])

dnl Check for required packages
XDT_CHECK_PACKAGE([GSTREAMER], [gstreamer-${GSTREAMER_API_VERSION}], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GSTREAMER_BASE], [gstreamer-base-${GSTREAMER_API_VERSION}], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.28])
AS_IF([test "x$with_gtk3" != xno],
	[XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.0.0])],
	[XDT_CHECK_PACKAGE([GTK], [gtk+-2.0], [2.24.0])])
XDT_CHECK_PACKAGE([SQLITE3], [sqlite3], [3.4])
XDT_CHECK_PACKAGE([LIBNOTIFY], [libnotify], [0.4.4])
XDT_CHECK_PACKAGE([LIBCDIO], [libcdio], [0.80])
XDT_CHECK_PACKAGE([LIBCDIO_PARANOIA], [libcdio_paranoia], [0.80])
XDT_CHECK_PACKAGE([LIBCDDB], [libcddb], [1.3.0])
XDT_CHECK_PACKAGE([TAGLIB_C], [taglib_c], [1.4])

dnl Test for new paranoia headers location
AC_CHECK_HEADER(cdio/paranoia/cdda.h, [
	AC_DEFINE(HAVE_PARANOIA_NEW_INCLUDES, 1, [Define to 1 if cdio/paranoia/paranoia.h is present]) ])

dnl Check min taglib to add monkeyaudio support
TAGLIB_1_7_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([TAGLIB_1_7],
                       [taglib], [1.7.0],
                       [taglib],
                       [taglib library to APE], [yes])

dnl Check gstreamer-interfaces
dnl need for gstreamer 0.10
GSTREAMER_INTERFACES_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([GSTREAMER_INTERFACES],
                       [gstreamer-interfaces-${GSTREAMER_API_VERSION}], [$GSTREAMER_REQUIRED],
                       [gstreamer-interfaces],
                       [gstreamer-interfaces], [yes])

dnl Check gstreamer-audio
dnl need for gstreamer 1.0
GSTREAMER_AUDIO_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([GSTREAMER_AUDIO],
                       [gstreamer-audio-${GSTREAMER_API_VERSION}], [$GSTREAMER_REQUIRED],
                       [gstreamer-audio],
                       [gstreamer-audio], [yes])

dnl Check global keyboard shortcuts support
LIBKEYBINDER_FOUND="no"
AS_IF([test "x$with_gtk3" != xno],
	[keybinder_version="keybinder-3.0"],
	[keybinder_version="keybinder"])
XDT_CHECK_OPTIONAL_PACKAGE([LIBKEYBINDER],
                       [$keybinder_version], [0.2.0],
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
                       [libglyr], [1.0.1],
                       [libglyr],
                       [libglyr library], [yes])

dnl Check libxfce4ui
AS_IF([test "x$with_gtk3" != xno],
	[libxfce4ui_name="libxfce4ui-2"],
	[libxfce4ui_name="libxfce4ui-1"])
AS_IF([test "x$with_gtk3" != xno],
	[libxfce4ui_version="4.10.0"],
	[libxfce4ui_version="4.8.0"])

LIBXFCE4UI_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBXFCE4UI],
                       [$libxfce4ui_name], [$libxfce4ui_version],
                       [libxfce4ui],
                       [libxfce4ui library], [yes])

PLPARSER_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([PLPARSER],
                       [totem-plparser], [2.26],
                       [totem-plparser],
                       [totem-plparser library], [yes])

AC_OUTPUT([
Makefile
data/Makefile
po/Makefile.in
src/Makefile
])

echo ""
echo "Build Configuration for $PACKAGE version $VERSION"
echo ""

echo "  Gtk version used...: $GTK_VERSION"
echo "  Prefix.............: ${prefix}"
echo "  Debug enabled......: $debug"
echo ""
AS_IF([test $GSTREAMER_API_VERSION = 1.0],
      [echo "  Cubic volume support.....................(Need gstreamer-plugins-base).: $GSTREAMER_AUDIO_FOUND"],
      [echo "  Cubic volume support.....................(Need gstreamer-plugins-base).: $GSTREAMER_INTERFACES_FOUND"])
AS_IF([test "x$with_gtk3" != xno],
	[echo "  Use multimedia key without gnome-media-keys.......(Need keybinder-3.0).: $LIBKEYBINDER_FOUND"],
	[echo "  Use multimedia key without gnome-media-keys..(Need keybinder >= 0.2.0).: $LIBKEYBINDER_FOUND"])
AS_IF([test "x$with_gtk3" != xno],
	[echo "  Better session management support..........(Need libxfce4ui >= 4.11.0).: $LIBXFCE4UI_FOUND"],
	[echo "  Better session management support...........(Need libxfce4ui >= 4.8.0).: $LIBXFCE4UI_FOUND"])
echo "  Lastfm scroble, and add similar songs support (Need libclastfm >= 0.5).: $LIBCLASTFM_FOUND"
echo "  Search lyrics, artists info and albums art.....(Need libglyr >= 1.0.1).: $LIBGLYR_FOUND"
echo "  Use totem-pl-parser to open playlists....(Need totem-plparser >= 2.26).: $PLPARSER_FOUND"
echo ""

