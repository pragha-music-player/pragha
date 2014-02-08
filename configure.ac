dnl configure.ac
dnl
dnl Pragha - A Lightweight Music Player for GNU/Linux
dnl

dnl Version information
m4_define([pragha_version_major], [1])
m4_define([pragha_version_minor], [3])
m4_define([pragha_version_micro], [0])

m4_define([pragha_version], [pragha_version_major().pragha_version_minor().pragha_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2009-2014
        Matias De lellis <mati86dl@gmail.com>])
AC_INIT([pragha], [pragha_version()], [mati86dl@gmail.com])
AC_CONFIG_HEADERS([config.h])

dnl  Initialize automake
AM_INIT_AUTOMAKE([no-dist-gzip dist-bzip2])
AM_MAINTAINER_MODE()

dnl Set required and max glib/gdk versions
AC_DEFINE(GDK_VERSION_MIN_REQUIRED, GDK_VERSION_3_4, [Ignore post 3.10 deprecations])
AC_DEFINE(GDK_VERSION_MAX_ALLOWED, GDK_VERSION_3_10, [Ignore post 3.10 deprecations])

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
XDT_I18N([cs de es el fr hu it ja ko_KR nl no pl pt pt_BR ru sv tr uk vi zh_CN])

GSTREAMER_REQUIRED=0.11.90

dnl Check for required packages
XDT_CHECK_PACKAGE([GSTREAMER], [gstreamer-1.0], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GSTREAMER_BASE], [gstreamer-base-1.0], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.32])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.4.0])
XDT_CHECK_PACKAGE([SQLITE3], [sqlite3], [3.4])
XDT_CHECK_PACKAGE([LIBCDIO], [libcdio], [0.80])
XDT_CHECK_PACKAGE([LIBCDIO_PARANOIA], [libcdio_paranoia], [0.90])
XDT_CHECK_PACKAGE([LIBCDDB], [libcddb], [1.3.0])
XDT_CHECK_PACKAGE([TAGLIB_C], [taglib_c], [1.4])

dnl Check peas library to plugins support
LIBPEAS_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBPEAS],
                           [libpeas-1.0], [1.0.0],
                           [libpeas-1.0],
                           [libpeas-1.0 library], [yes])

if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_PACKAGE([LIBPEAS_GTK], [libpeas-gtk-1.0], [1.0.0])
fi

dnl Check notify support
LIBNOTIFY_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBNOTIFY],
                           [libnotify], [0.4.4],
                           [libnotify],
                           [libnotify library], [yes])
fi

dnl Check global keyboard shortcuts support
LIBKEYBINDER_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBKEYBINDER],
                           [keybinder-3.0], [0.2.0],
                           [keybinder],
                           [keybinder library], [yes])
fi

dnl Check libglyr
LIBGLYR_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBGLYR],
                           [libglyr], [1.0.1],
                           [libglyr],
                           [libglyr library], [yes])
fi

GUDEV_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([GUDEV],
                           [gudev-1.0], [145],
                           [gudev-1.0],
                           [gudev-1.0 library], [yes])
fi

LIBMTP_FOUND="no"
if test x"$GUDEV_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBMTP],
                           [libmtp], [1.1.0],
                           [libmtp],
                           [libmtp library], [yes])
fi

dnl Check min taglib to add monkeyaudio support
TAGLIB_1_7_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([TAGLIB_1_7],
                       [taglib], [1.7.0],
                       [taglib],
                       [taglib library to APE], [yes])

dnl Check gstreamer-audio
GSTREAMER_AUDIO_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([GSTREAMER_AUDIO],
                       [gstreamer-audio-1.0], [$GSTREAMER_REQUIRED],
                       [gstreamer-audio],
                       [gstreamer-audio], [yes])

dnl Check libclastfm
LIBCLASTFM_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBCLASTFM],
                       [libclastfm], [0.5],
                       [libclastfm],
                       [libclastfm library], [yes])


dnl Check libxfce4ui
LIBXFCE4UI_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([LIBXFCE4UI],
                       [libxfce4ui-2], [4.10.0],
                       [libxfce4ui],
                       [libxfce4ui library], [yes])

dnl Check totem-pl-parser
PLPARSER_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([PLPARSER],
                       [totem-plparser], [2.26],
                       [totem-plparser],
                       [totem-plparser library], [yes])

dnl Output files
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([po/Makefile.in])

if test x"$LIBPEAS_FOUND" = x"yes"; then
AC_CONFIG_FILES([plugins/Makefile])
AC_CONFIG_FILES([plugins/mpris2/Makefile])
AC_CONFIG_FILES([plugins/keybinder/Makefile])
AC_CONFIG_FILES([plugins/gnome-media-keys/Makefile])
AC_CONFIG_FILES([plugins/song-info/Makefile])
AC_CONFIG_FILES([plugins/notify/Makefile])
AC_CONFIG_FILES([plugins/devices/Makefile])
fi
AC_OUTPUT

echo ""
echo "Build Configuration for $PACKAGE version $VERSION"
echo ""
echo "  Gtk version used......: $GTK_VERSION"
echo "  Prefix................: ${prefix}"
echo "  Debug enabled.........: $debug"
echo ""
echo "  Plugins enabled.......: $LIBPEAS_FOUND"
echo "   * Show notification when change songs....................................: $LIBNOTIFY_FOUND"
echo "   * Controls Pragha using Mpris2 interface.................................: $LIBPEAS_FOUND"
echo "   * Controls Pragha using gnome-media-keys interface.......................: $LIBPEAS_FOUND"
echo "   * Controls Pragha using multimedia keys.............(Need keybinder-3.0).: $LIBKEYBINDER_FOUND"
echo "   * Search lyrics, artists info and albums art.....(Need libglyr >= 1.0.1).: $LIBGLYR_FOUND"
echo "   * EXPERIMENTAL management removable devices.:(Need gudev-1.0 and libmtp).: $LIBMTP_FOUND"
echo ""
echo "  Others build options..:"
AS_IF([test $GSTREAMER_API_VERSION = 1.0],
      [echo "   * Cubic volume support.....................(Need gstreamer-plugins-base).: $GSTREAMER_AUDIO_FOUND"],
      [echo "   * Cubic volume support.....................(Need gstreamer-plugins-base).: $GSTREAMER_INTERFACES_FOUND"])
echo "   * Better session management support........(Need libxfce4ui-2 >= 4.11.0).: $LIBXFCE4UI_FOUND"
echo "   * Lastfm scroble, and add similar songs support (Need libclastfm >= 0.5).: $LIBCLASTFM_FOUND"
echo "   * Use totem-pl-parser to open playlists....(Need totem-plparser >= 2.26).: $PLPARSER_FOUND"
echo ""

