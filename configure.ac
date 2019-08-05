dnl configure.ac
dnl
dnl Pragha - A Lightweight Music Player for GNU/Linux
dnl

dnl Version information
m4_define([pragha_version_major], [1])
m4_define([pragha_version_minor], [3])
m4_define([pragha_version_micro], [99.1])

m4_define([pragha_version], [pragha_version_major().pragha_version_minor().pragha_version_micro()])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2009-2019
        Matias De lellis <mati86dl@gmail.com>])
AC_INIT([pragha], [pragha_version()], [mati86dl@gmail.com])
AC_CONFIG_HEADERS([config.h])

dnl  Initialize automake
AM_INIT_AUTOMAKE([no-dist-gzip dist-bzip2])
AM_MAINTAINER_MODE()

dnl Set required and max glib/gdk versions
AC_DEFINE(GDK_VERSION_MIN_REQUIRED, GDK_VERSION_3_14, [Ignore post 3.14 deprecations])
AC_DEFINE(GDK_VERSION_MAX_ALLOWED, GDK_VERSION_3_14, [Ignore post 3.14 deprecations])

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

dnl ***********************************************************************
dnl Check for win32 platform
dnl ***********************************************************************
AC_CANONICAL_HOST
AC_MSG_CHECKING([whether to build with mingw32 for a Windows host])
case "${host}" in
        *-*-mingw*|*-*-cygwin*) WIN32="yes" ;;
        *) WIN32="no" ;;
esac

AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

AS_IF([test "x$WIN32" = "xyes"],
      [AC_CHECK_TOOL([WINDRES], [windres])
       AS_IF([test -z "$WINDRES"],
             [AC_MSG_ERROR([windres is required when building for a Windows host])])])

AC_SUBST(CFLAGS)

PLUGIN_LIBTOOL_FLAGS="-avoid-version -module -shared -export-dynamic -no-undefined"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

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
XDT_I18N([ar bg ca_ES cs da de es el fr hu id it ja ko_KR lt nl no pl pt pt_BR ru sk sv tr uk vi zh_CN])

dnl Check tools required
AC_CHECK_PROG([HAVE_XDT_CSOURCE], [xdt-csource], [yes])
if test x"$HAVE_XDT_CSOURCE" != xyes; then
  AC_MSG_FAILURE([xdt-csource not installed])
fi

GSTREAMER_REQUIRED=0.11.90

dnl Check for required packages
XDT_CHECK_PACKAGE([GSTREAMER], [gstreamer-1.0], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GSTREAMER_BASE], [gstreamer-base-1.0], [$GSTREAMER_REQUIRED])
XDT_CHECK_PACKAGE([GIO], [gio-2.0], [2.42])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.14.0])
XDT_CHECK_PACKAGE([SQLITE3], [sqlite3], [3.4])
XDT_CHECK_PACKAGE([TAGLIB_C], [taglib_c], [1.8])

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
                           [libnotify], [0.7.5],
                           [libnotify],
                           [libnotify library], [yes])
else
AM_CONDITIONAL([HAVE_LIBNOTIFY], 0)
fi

dnl Check global keyboard shortcuts support
LIBKEYBINDER_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBKEYBINDER],
                           [keybinder-3.0], [0.2.0],
                           [keybinder],
                           [keybinder library], [yes])
else
AM_CONDITIONAL([HAVE_LIBKEYBINDER], 0)
fi

dnl Check libglyr
LIBGLYR_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBGLYR],
                           [libglyr], [1.0.1],
                           [libglyr],
                           [libglyr library], [yes])
else
AM_CONDITIONAL([HAVE_LIBGLYR], 0)
fi

GUDEV_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([GUDEV],
                           [gudev-1.0], [145],
                           [gudev-1.0],
                           [gudev-1.0 library], [yes])
else
AM_CONDITIONAL([HAVE_GUDEV], 0)
fi

LIBMTP_FOUND="no"
if test x"$GUDEV_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBMTP],
                           [libmtp], [1.1.0],
                           [libmtp],
                           [libmtp library], [yes])
else
AM_CONDITIONAL([HAVE_LIBMTP], 0)
fi

dnl Check libpsuop
LIBSOUP_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBSOUP],
                           [libsoup-2.4], [2.38],
                           [libsoup-2.4],
                           [libsoup-2.4 library], [yes])
else
AM_CONDITIONAL([HAVE_LIBSOUP], 0)
fi

dnl Check libpsuop
JSON_GLIB_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([JSON_GLIB],
                           [json-glib-1.0], [1.0.0],
                           [json-glib-1.0],
                           [json-glib-1.0 library], [yes])
else
AM_CONDITIONAL([HAVE_JSON_GLIB], 0)
fi

dnl Check rygel
RYGEL_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([RYGEL],
                           [rygel-server-2.6], [0.26.0],
                           [rygel-server-2.6],
                           [rygel-server-2.6 library], [yes])
else
AM_CONDITIONAL([HAVE_RYGEL], 0)
fi

dnl Check grilo
GRILO_FOUND="no"
GRILO3_FOUND="no"
GRILO2_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([GRILO3],
                           [grilo-0.3], [0.3.0],
                           [grilo-0.3],
                           [grilo-0.3 library], [yes])
if test x"$GRILO3_FOUND" = x"no"; then
XDT_CHECK_OPTIONAL_PACKAGE([GRILO2],
                           [grilo-0.2], [0.2.11],
                           [grilo-0.2],
                           [grilo-0.2 library], [yes])
else
AM_CONDITIONAL([HAVE_GRILO2], 0)
fi
else
AM_CONDITIONAL([HAVE_GRILO3], 0)
AM_CONDITIONAL([HAVE_GRILO2], 0)
fi

if test x"$GRILO3_FOUND" = x"yes"; then
GRILO_FOUND="yes"
fi
if test x"$GRILO2_FOUND" = x"yes"; then
GRILO_FOUND="yes"
fi

dnl Check grilo-net
GRILO_NET_FOUND="no"
GRILO_NET3_FOUND="no"
GRILO_NET2_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([GRILO_NET3],
                           [grilo-net-0.3], [0.3.0],
                           [grilo-net-0.3],
                           [grilo-net-0.3 library], [yes])
if test x"$GRILO_NET3_FOUND" = x"no"; then
XDT_CHECK_OPTIONAL_PACKAGE([GRILO_NET2],
                           [grilo-net-0.2], [0.2.5],
                           [grilo-net-0.2],
                           [grilo-net-0.2 library], [yes])
else
AM_CONDITIONAL([HAVE_GRILO_NET2], 0)
fi
else
AM_CONDITIONAL([HAVE_GRILO_NET3], 0)
AM_CONDITIONAL([HAVE_GRILO_NET2], 0)
fi

if test x"$GRILO_NET3_FOUND" = x"yes"; then
GRILO_NET_FOUND="yes"
fi
if test x"$GRILO_NET2_FOUND" = x"yes"; then
GRILO_NET_FOUND="yes"
fi

dnl Check libclastfm
LIBCLASTFM_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBCLASTFM],
                           [libclastfm], [0.5],
                           [libclastfm],
                           [libclastfm library], [yes])
else
AM_CONDITIONAL([HAVE_LIBCLASTFM], 0)
fi

dnl Check libcdio, libcdio_paranoia and libcddb
LIBCDIO_FOUND="no"
LIBCDIO_PARANOIA_FOUND="no"
LIBCDDB_FOUND="no"
if test x"$LIBPEAS_FOUND" = x"yes"; then
XDT_CHECK_OPTIONAL_PACKAGE([LIBCDIO],
                           [libcdio], [0.80],
                           [libcdio],
                           [libcdio library], [yes])

XDT_CHECK_OPTIONAL_PACKAGE([LIBCDIO_PARANOIA],
                           [libcdio_paranoia], [0.90],
                           [libcdio_paranoia],
                           [libcdio_paranoia library], [yes])

XDT_CHECK_OPTIONAL_PACKAGE([LIBCDDB],
                           [libcddb], [1.3.0],
                           [libcddb],
                           [libcddb library], [yes])
else
AM_CONDITIONAL([HAVE_LIBCDIO], 0)
AM_CONDITIONAL([HAVE_LIBCDIO_PARANOIA], 0)
AM_CONDITIONAL([HAVE_LIBCDDB], 0)
fi

CDROM_SUPPORT="no"
if test x"$LIBCDIO_FOUND" = x"yes"; then
if test x"$LIBCDIO_PARANOIA_FOUND" = x"yes"; then
if test x"$LIBCDDB_FOUND" = x"yes"; then
CDROM_SUPPORT="yes"
fi
fi
fi

KOEL_SUPPORT="no"
if test x"$LIBSOUP_FOUND" = x"yes"; then
if test x"$JSON_GLIB_FOUND" = x"yes"; then
KOEL_SUPPORT="yes"
fi
fi


dnl Check gstreamer-audio
GSTREAMER_AUDIO_FOUND="no"
XDT_CHECK_OPTIONAL_PACKAGE([GSTREAMER_AUDIO],
                       [gstreamer-audio-1.0], [$GSTREAMER_REQUIRED],
                       [gstreamer-audio],
                       [gstreamer-audio], [yes])

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
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([data/icons/Makefile])
AC_CONFIG_FILES([data/icons/16x16/Makefile])
AC_CONFIG_FILES([data/icons/24x24/Makefile])
AC_CONFIG_FILES([data/icons/32x32/Makefile])
AC_CONFIG_FILES([data/icons/48x48/Makefile])
AC_CONFIG_FILES([data/icons/64x64/Makefile])
AC_CONFIG_FILES([data/icons/128x128/Makefile])
AC_CONFIG_FILES([data/icons/symbolic/Makefile])
AC_CONFIG_FILES([data/ui/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/win32/Makefile])

if test x"$LIBPEAS_FOUND" = x"yes"; then
AC_CONFIG_FILES([plugins/Makefile])
AC_CONFIG_FILES([plugins/acoustid/Makefile])
AC_CONFIG_FILES([plugins/ampache/Makefile])
AC_CONFIG_FILES([plugins/mpris2/Makefile])
AC_CONFIG_FILES([plugins/keybinder/Makefile])
AC_CONFIG_FILES([plugins/koel/Makefile])
AC_CONFIG_FILES([plugins/gnome-media-keys/Makefile])
AC_CONFIG_FILES([plugins/song-info/Makefile])
AC_CONFIG_FILES([plugins/notify/Makefile])
AC_CONFIG_FILES([plugins/mtp/Makefile])
AC_CONFIG_FILES([plugins/removable-media/Makefile])
AC_CONFIG_FILES([plugins/tunein/Makefile])
AC_CONFIG_FILES([plugins/dlna/Makefile])
AC_CONFIG_FILES([plugins/dlna-renderer/Makefile])
AC_CONFIG_FILES([plugins/cdrom/Makefile])
AC_CONFIG_FILES([plugins/lastfm/Makefile])
AC_CONFIG_FILES([plugins/visualizer/Makefile])
fi
AC_OUTPUT

echo ""
echo "Build Configuration for $PACKAGE version $VERSION"
echo ""
echo "  Gtk version used......: $GTK_VERSION"
echo "  Gstreamer version used: $GSTREAMER_VERSION"
echo ""
echo "  Prefix................: ${prefix}"
echo "  Host..................: ${host}"
echo ""
echo "  Debug enabled.........: $debug"
echo ""
echo "  Plugins enabled.......: $LIBPEAS_FOUND"
echo "   * Show notification when change songs..........(Need libnotify >= 0.7.5).: $LIBNOTIFY_FOUND"
echo "   * Controls Pragha using Mpris2 interface.................................: $LIBPEAS_FOUND"
echo "   * Controls Pragha using gnome-media-keys interface.......................: $LIBPEAS_FOUND"
echo "   * Play Audio cds...............(Need libcdio, libcdio_paranoia, libcddb).: $CDROM_SUPPORT"
echo "   * Controls Pragha using multimedia keys....(Need keybinder-3.0 >= 0.2.0).: $LIBKEYBINDER_FOUND"
echo "   * Search lyrics, artists info and albums art.....(Need libglyr >= 1.0.1).: $LIBGLYR_FOUND"
echo "   * Search metadata on AcoustID.....................(Need libsoup >= 2.38).: $LIBSOUP_FOUND"
echo "   * Search radios on TuneIn.........................(Need libsoup >= 2.38).: $LIBSOUP_FOUND"
echo "   * Share music library on DLNA............(Need rygel-server-2.2 >= 0.26).: $RYGEL_FOUND"
echo "   * Play songs on DLNA Servers......................(Need grilo >= 0.2.11).: $GRILO_FOUND"
echo "   * Append music on Ampache server...............(Need grilo-net >= 0.2.5).: $GRILO_NET_FOUND"
echo "   * Append music on Koel server...............(Need libsoup and json-glib).: $KOEL_SUPPORT"
echo "   * Lastfm scroble, and add similar songs support (Need libclastfm >= 0.5).: $LIBCLASTFM_FOUND"
echo "   * EXPERIMENTAL management of external devices...........(Need gudev-1.0).: $GUDEV_FOUND"
echo "     * USB flash drive/Memory cards........................(Need gudev-1.0).: $GUDEV_FOUND"
echo "     * Optical disc Hot-plug...............................(Need gudev-1.0).: $GUDEV_FOUND"
echo "     * MTP compatible devices..................:(Need gudev-1.0 and libmtp).: $LIBMTP_FOUND"
echo ""
echo "  Others build options..:"
echo "   * Cubic volume support.....................(Need gstreamer-plugins-base).: $GSTREAMER_AUDIO_FOUND"
echo "   * Better session management support........(Need libxfce4ui-2 >= 4.11.0).: $LIBXFCE4UI_FOUND"
echo "   * Use totem-pl-parser to open playlists....(Need totem-plparser >= 2.26).: $PLPARSER_FOUND"
echo ""
