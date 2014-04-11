[Setup]
AppName= Pragha
AppVerName=Pragha V.1.3.0
AppVersion=1.3.0
AppPublisher=Matias De lellis
AppPublisherURL=https://github.com/matiasdelellis
AppCopyright=Copyright (C) 2009-2014 Matias De lellis
AppSupportURL=https://github.com/matiasdelellis/pragha
AppUpdatesURL=https://github.com/matiasdelellis/pragha
DefaultDirName={pf}\Pragha
DefaultGroupName=Pragha
DisableStartupPrompt=yes
WindowShowCaption=yes
WindowVisible=no
LicenseFile=..\COPYING
;BackColor=$FF8200
;BackColor=clPurple
;BackColor2=clBlack
Compression=bzip/9
SourceDir=.
OutputDir=.
OutputBaseFilename=Pragha 1.3.0
ChangesAssociations=no
SetupIconFile="pragha.ico"
AppId={{1A58C548-142C-4016-9943-6A39EB25BB51}

#define MINGW  "Z:\usr\i686-w64-mingw32\sys-root\mingw"
#define ICON_PATH "Z:\usr\share\icons\oxygen"
#define GST_CODECS "c:\gstreamer\1.0\x86"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Run]
;Filename: "{app}\bin\gtk-update-icon-cache.exe {app}\share\icons\oxygen\"
Filename: "{app}\Readme.txt"; Description: "View the README file"; Flags: postinstall shellexec skipifsilent unchecked
Filename: "{app}\bin\pragha.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent

[Registry]
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\pragha.exe"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\pragha.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\pragha.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\pragha.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}\lib\pango\1.8.0\modules"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\pragha.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\pragha.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app}\lib\gstreamer-1.0\"; Flags: uninsdeletevalue

[Components]
Name: "main"; Description: "Main Files"; Types: full compact custom; Flags: fixed
Name: "codecs"; Description: "Privative Codecs"; Types: full

[Files]
; Pragha files
Source: "{#MINGW}\bin\pragha.exe"; DestDir: "{app}/bin"; DestName: "pragha.exe"
Source: "pragha.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "settings.ini"; DestDir: "{app}\etc\gtk-3.0"; Flags: ignoreversion
Source: "..\data\pragha.png"; DestDir: "{app}\share\icons\oxygen\128x128\apps"
; Translations
Source: "{#MINGW}\share\locale\cs\LC_MESSAGES\pragha.mo"; DestDir: "{app}\share\locale\cs\LC_MESSAGES\"
Source: "{#MINGW}\share\locale\es\LC_MESSAGES\pragha.mo"; DestDir: "{app}\share\locale\es\LC_MESSAGES\"
Source: "{#MINGW}\share\locale\fr\LC_MESSAGES\pragha.mo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES\"
Source: "{#MINGW}\share\locale\pt\LC_MESSAGES\pragha.mo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES\"
Source: "{#MINGW}\share\locale\ru\LC_MESSAGES\pragha.mo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES\"
Source: "{#MINGW}\share\doc\pragha\README"; DestDir: "{app}"; DestName: "Readme.txt"
; Icons
Source: "{#MINGW}\share\pixmaps\pragha\pragha.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "{#MINGW}\share\pixmaps\pragha\album.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "{#MINGW}\share\pixmaps\pragha\artist.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "{#MINGW}\share\pixmaps\pragha\track.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "{#MINGW}\share\pixmaps\pragha\cover.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "{#MINGW}\share\pixmaps\pragha\genre.png"; DestDir: "{app}\share\pixmaps\pragha"

; Deps
; Atk
Source: "{#MINGW}\bin\libatk-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Bzip2
Source: "{#MINGW}\bin\libbz2-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Cairo
Source: "{#MINGW}\bin\libcairo-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libcairo-gobject-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libcairo-script-interpreter-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Expat
Source: "{#MINGW}\bin\libexpat-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Fontconfig
Source: "{#MINGW}\bin\libfontconfig-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Freetype
Source: "{#MINGW}\bin\libfreetype-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Gcc
Source: "{#MINGW}\bin\libgcc_s_sjlj-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libssp-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Gcc-c++
Source: "{#MINGW}\bin\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Gdk-pixbuf
Source: "{#MINGW}\bin\libgdk_pixbuf-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-tga.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-qtif.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-jasper.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-ras.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-xpm.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-wbmp.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-ani.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-pcx.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-pnm.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-icns.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-xbm.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
; Gettext
Source: "{#MINGW}\bin\libasprintf-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextlib-0-18-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextpo-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextsrc-0-18-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libintl-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Glib
Source: "{#MINGW}\bin\libgio-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libglib-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgmodule-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgobject-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgthread-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Gstreamer1
Source: "{#MINGW}\bin\libgstbase-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstcontroller-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstnet-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstreamer-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstcoreelements.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
; Gstreamer1-plugin-base
Source: "{#MINGW}\bin\libgstallocators-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstapp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstaudio-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstpbutils-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstriff-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstsdp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgsttag-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstvideo-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstadder.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstapp.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudioconvert.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudiorate.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudioresample.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudiotestsrc.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstencodebin.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstgio.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstogg.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstpango.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstplayback.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstsubparse.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsttcp.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsttypefindfunctions.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvolume.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvorbis.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
; Gtk3
Source: "{#MINGW}\bin\libgailutil-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgdk-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgtk-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-ti-et.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-cyrillic-translit.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-ti-er.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-thai.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-ipa.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-multipress.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-am-et.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-cedilla.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-ime.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-inuktitut.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
Source: "{#MINGW}\lib\gtk-3.0\3.0.0\immodules\im-viqr.dll"; DestDir: "{app}\lib\gtk-3.0\3.0.0\immodules"; Flags: ignoreversion
; Harfbuzz
Source: "{#MINGW}\bin\libharfbuzz-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libharfbuzz-icu-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Icu
Source: "{#MINGW}\bin\icudata50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icui18n50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icuio50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icule50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\iculx50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icutest50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icutu50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icuuc50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Jasper
Source: "{#MINGW}\bin\libjasper-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Ffi
Source: "{#MINGW}\bin\libffi-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Jpeg-turbo
Source: "{#MINGW}\bin\libjpeg-62.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libturbojpeg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Ogg
Source: "{#MINGW}\bin\libogg-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Png
Source: "{#MINGW}\bin\libpng16-16.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Vorbis
Source: "{#MINGW}\bin\libvorbis-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libvorbisenc-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libvorbisfile-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Pango
Source: "{#MINGW}\bin\libpango-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangocairo-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangoft2-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangowin32-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-basic-fc.dll"; DestDir: "{app}\lib\pango\1.8.0\modules"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-indic-lang.dll"; DestDir: "{app}\lib\pango\1.8.0\modules"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-arabic-lang.dll"; DestDir: "{app}\lib\pango\1.8.0\modules"; Flags: ignoreversion
; Pixman
Source: "{#MINGW}\bin\libpixman-1-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Sqlite
Source: "{#MINGW}\bin\libsqlite3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Taglib
Source: "{#MINGW}\bin\libtag.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libtag_c.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Termcap
Source: "{#MINGW}\bin\libtermcap-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Iconv
Source: "{#MINGW}\bin\iconv.dll"; DestDir: "{app}/bin"; Flags: ignoreversion
; Winpthread
Source: "{#MINGW}\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; Zlib
Source: "{#MINGW}\bin\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

; Settings
Source: "{#MINGW}\etc\gtk-3.0\gtk.immodules"; DestDir: "{app}\etc\gtk-3.0"; Flags: ignoreversion
Source: "{#MINGW}\etc\gtk-3.0\im-multipress.conf"; DestDir: "{app}\etc\gtk-3.0"; Flags: ignoreversion
Source: "{#MINGW}\etc\pango\pango.modules"; DestDir: "{app}\etc\pango"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\fonts.conf"; DestDir: "{app}\etc\fonts"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\30-metric-aliases.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\10-scale-bitmap-fonts.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\30-urw-aliases.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\51-local.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\README"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\20-unhint-small-vera.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\90-synthetic.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\65-nonlatin.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\40-nonlatin.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\80-delicious.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\65-fonts-persian.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\50-user.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\69-unifont.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\60-latin.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\45-latin.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\etc\fonts\conf.d\49-sansserif.conf"; DestDir: "{app}\etc\fonts\conf.d"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.Settings.FileChooser.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.Settings.ColorChooser.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\gschemas.compiled"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\gschema.dtd"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion

; Stock Icons
Source: "{#ICON_PATH}\48x48\actions\media-skip-backward.png"; DestDir: "{app}\share\icons\oxygen\48x48\actions\"
Source: "{#ICON_PATH}\48x48\actions\media-playback-start.png"; DestDir: "{app}\share\icons\oxygen\48x48\actions\"
Source: "{#ICON_PATH}\48x48\actions\media-playback-pause.png"; DestDir: "{app}\share\icons\oxygen\48x48\actions\"
Source: "{#ICON_PATH}\48x48\actions\media-playback-stop.png"; DestDir: "{app}\share\icons\oxygen\48x48\actions\"
Source: "{#ICON_PATH}\48x48\actions\media-skip-forward.png"; DestDir: "{app}\share\icons\oxygen\48x48\actions\"
Source: "{#ICON_PATH}\48x48\status\media-playlist-shuffle.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\media-playlist-repeat.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\dialog-information.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\audio-volume-muted.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\audio-volume-high.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\audio-volume-low.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"
Source: "{#ICON_PATH}\48x48\status\audio-volume-medium.png"; DestDir: "{app}\share\icons\oxygen\48x48\status\"

Source: "{#ICON_PATH}\22x22\actions\view-refresh.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\actions\process-stop.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\actions\window-close.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\actions\go-jump.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\actions\list-add.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"; DestName: "list-add-symbolic"
Source: "{#ICON_PATH}\22x22\actions\list-remove.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\actions\list-remove.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"; DestName: "list-add-symbolic"
Source: "{#ICON_PATH}\22x22\actions\edit-find.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"; DestName: "edit-find-symbolic.png"
Source: "{#ICON_PATH}\22x22\actions\edit-clear.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"; DestName: "edit-clear-symbolic.png"
Source: "{#ICON_PATH}\22x22\actions\edit-clear.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\devices\media-optical.png"; DestDir: "{app}\share\icons\oxygen\22x22\devices\"
Source: "{#ICON_PATH}\22x22\mimetypes\audio-x-generic.png"; DestDir: "{app}\share\icons\oxygen\22x22\mimetypes"
Source: "{#ICON_PATH}\22x22\status\audio-volume-high.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\status\dialog-warning.png"; DestDir: "{app}\share\icons\oxygen\22x22\actions\"
Source: "{#ICON_PATH}\22x22\places\folder.png"; DestDir: "{app}\share\icons\oxygen\22x22\places\"
Source: "{#ICON_PATH}\22x22\places\folder-sound.png"; DestDir: "{app}\share\icons\oxygen\22x22\places\"; DestName: "folder-music.png"
Source: "{#ICON_PATH}\index.theme"; DestDir: "{app}\share\icons\oxygen\"

#include "codecs.iss"

[Icons]
Name: "{group}\Pragha"; Filename: "{app}\bin\pragha.exe"; IconFilename: {app}\pragha.ico; Comment: "Yeah!. Music..";
Name: "{group}\Pragha Website"; Filename: "https://github.com/matiasdelellis/pragha";
Name: "{group}\Uninstall Pragha"; Filename: "{uninstallexe}"; IconFilename: {app}\pragha.ico;
Name: "{userdesktop}\Pragha"; Filename: "{app}\bin\pragha.exe"; IconFilename: {app}\pragha.ico; Tasks: desktopicon; Comment: "Yeah!. Music..";

[Dirs]
Name: "{app}\etc"
Name: "{app}\etc\gtk-3.0"
Name: "{app}\etc\pango"
Name: "{app}\etc\pkcs11"
Name: "{app}\etc\fonts"
Name: "{app}\etc\fonts\conf.d"
Name: "{app}\lib\gdk-pixbuf-2.0"
Name: "{app}\lib\gdk-pixbuf-2.0\2.10.0"
Name: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"
Name: "{app}\lib\pango"
Name: "{app}\lib\pango\1.8.0"
Name: "{app}\lib\pango\1.8.0\modules"
Name: "{app}\lib\gtk-3.0"
Name: "{app}\lib\gtk-3.0\3.0.0"
Name: "{app}\lib\gtk-3.0\3.0.0\immodules"
Name: "{app}\lib\gstreamer-1.0"
Name: "{app}\share\glib-2.0\schemas"
