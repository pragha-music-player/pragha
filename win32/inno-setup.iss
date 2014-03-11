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
AppId={{1A58C548-142C-4016-9943-6A39EB25BB51}

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Registry]
Root: HKCR; SubKey: ".*.mp3"; ValueType: string; ValueData: "Mp3 audio"; Flags: uninsdeletekey
Root: HKCR; SubKey: "Mp3 audio"; ValueType: string; ValueData: "Play mp3 with Pragha"; Flags: uninsdeletekey
Root: HKCR; SubKey: "Mp3 audio\Shell\Open\Command"; ValueType: string; ValueData: """{app}/bin\pragha.exe"" ""%1"""; Flags: uninsdeletekey

[Run]
Filename: "{app}\bin\pragha.exe"; Parameters: "/x"
Filename: "{app}\README"; Description: "View the README file"; Flags: postinstall shellexec skipifsilent
Filename: "{app}\bin\pragha.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent unchecked

#define MINGW  "Z:\usr\i686-w64-mingw32\sys-root\mingw"

[Files]
;pragha files
Source: "..\src\pragha.exe"; DestDir: "{app}/bin"; DestName: "pragha.exe"
Source: "pragha.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\data\pragha.png"; DestDir: "{app}\share\icons\hicolor\128x128\apps"
Source: "..\po\cs.gmo"; DestDir: "{app}\share\locale\cs\LC_MESSAGES\"; DestName: "pragha.mo"
Source: "..\po\de.gmo"; DestDir: "{app}\share\locale\de\LC_MESSAGES\"; DestName: "pragha.mo"
Source: "..\po\fr.gmo"; DestDir: "{app}\share\locale\fr\LC_MESSAGES\"; DestName: "pragha.mo"
Source: "..\po\pt.gmo"; DestDir: "{app}\share\locale\pt\LC_MESSAGES\"; DestName: "pragha.mo"
Source: "..\po\ru.gmo"; DestDir: "{app}\share\locale\ru\LC_MESSAGES\"; DestName: "pragha.mo"
Source: "..\README"; DestDir: "{app}"
;icons
Source: "..\data\pragha.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "..\data\album.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "..\data\artist.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "..\data\track.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "..\data\cover.png"; DestDir: "{app}\share\pixmaps\pragha"
Source: "..\data\genre.png"; DestDir: "{app}\share\pixmaps\pragha"
; Deps
Source: "{#MINGW}\bin\iconv.dll"; DestDir: "{app}/bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icudata50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icui18n50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icuio50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icule50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\iculx50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icutest50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icutu50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\icuuc50.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libasprintf-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libatk-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libbz2-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libcairo-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libcairo-gobject-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libcairo-script-interpreter-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libexpat-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libffi-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libfontconfig-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libfreetype-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgailutil-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgcc_s_sjlj-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgdk_pixbuf-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgdk-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextlib-0-18-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextpo-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgettextsrc-0-18-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgio-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libglib-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgmodule-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgmp-10.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgmpxx-4.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgnutls-28.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgnutls-xssl-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgnutlsxx-28.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgobject-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstallocators-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstapp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstaudio-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstbase-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstcontroller-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstfft-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstnet-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstpbutils-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstreamer-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstriff-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstrtp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstrtsp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstsdp-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgsttag-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgstvideo-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgthread-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libgtk-3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libharfbuzz-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libharfbuzz-icu-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libhogweed-2-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libintl-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libjasper-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libjpeg-62.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libnettle-4-7.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libogg-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libp11-kit-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpango-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangocairo-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangoft2-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpangowin32-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpixman-1-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libpng16-16.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libsqlite3-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libssp-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libtag_c.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libtag.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libtasn1-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libtermcap-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libturbojpeg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libvorbis-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libvorbisenc-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libvorbisfile-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\bin\zlib1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#MINGW}\etc\gtk-3.0\gtk.immodules"; DestDir: "{app}\etc\gtk-3.0"; Flags: ignoreversion
Source: "{#MINGW}\etc\gtk-3.0\im-multipress.conf"; DestDir: "{app}\etc\gtk-3.0"; Flags: ignoreversion
Source: "{#MINGW}\etc\pango\pango.modules"; DestDir: "{app}\etc\pango"; Flags: ignoreversion
Source: "{#MINGW}\etc\pkcs11\pkcs11.conf.example"; DestDir: "{app}\etc\pkcs11"; Flags: ignoreversion
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
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-tga.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-qtif.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-jasper.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-ras.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-xpm.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-wbmp.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-ani.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-pcx.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-pnm.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-icns.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\gdk-pixbuf-2.0\2.10.0\loaders\libpixbufloader-xbm.dll"; DestDir: "{app}\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-basic-fc.dll"; DestDir: "{app}\pango\1.8.0\modules"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-indic-lang.dll"; DestDir: "{app}\pango\1.8.0\modules"; Flags: ignoreversion
Source: "{#MINGW}\lib\pango\1.8.0\modules\pango-arabic-lang.dll"; DestDir: "{app}\pango\1.8.0\modules"; Flags: ignoreversion
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
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.Settings.FileChooser.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.exampleapp.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.Settings.ColorChooser.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\gschemas.compiled"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\org.gtk.Demo.gschema.xml"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
Source: "{#MINGW}\share\glib-2.0\schemas\gschema.dtd"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion
; Gstreamer1
Source: "{#MINGW}\lib\gstreamer-1.0\libgstasf.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstcoreelements.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsttcp.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsta52dec.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstdvdsub.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvideotestsrc.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudiotestsrc.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstrmdemux.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstsubparse.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstdvdlpcmdec.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvorbis.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstx264.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstmpeg2dec.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudioresample.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvolume.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstplayback.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudiorate.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstaudioconvert.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstmad.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstgio.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstxingmux.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstadder.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstamrwbdec.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvideorate.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstdvdread.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstapp.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstpango.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsttypefindfunctions.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvideoscale.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstamrnb.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstcdio.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgsttwolame.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstvideoconvert.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstogg.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstlame.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "{#MINGW}\lib\gstreamer-1.0\libgstencodebin.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion

[Icons]
Name: "{group}\Pragha"; Filename: "{app}\pragha.exe"; Comment: "Yeah!. Music..";
Name: "{group}\Pragha Website"; Filename: "https://github.com/matiasdelellis/pragha";
Name: "{group}\Uninstall Pragha"; Filename: "{uninstallexe}";
Name: "{userdesktop}\Pragha"; Filename: "{app}\pragha.exe"; Tasks: desktopicon; Comment: "Yeah!. Music..";

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
Name: "{app}\pango"
Name: "{app}\pango\1.8.0"
Name: "{app}\pango\1.8.0\modules"
Name: "{app}\gtk-3.0"
Name: "{app}\lib\gtk-3.0\3.0.0"
Name: "{app}\lib\gtk-3.0\3.0.0\immodules"
Name: "{app}\lib\gstreamer-1.0"
Name: "{app}\share\glib-2.0\schemas"
