; Direct Sound support
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdirectsoundsink.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood

; Equalizer
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstequalizer.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood

; MP3
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstmad.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstaudioparsers.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstid3demux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgoodSource: "{#GST_CODECS}\lib\gstreamer-1.0\libgstapetag.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood

;FLAC
Source: "{#GST_CODECS}\bin\libFLAC-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstflac.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood
;M4ASource: "{#GST_CODECS}\bin\libfaad-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstfaad.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstisomp4.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\bin\libz.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsgood

;Wav
Source: "{#GST_CODECS}\bin\libwavpack-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsgood
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstwavparse.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsgood

; Gstreamer codecs
Source: "{#GST_CODECS}\bin\liba52-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libbz2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libcharset-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libdca-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libfribidi-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgcrypt-20.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgnutls-28.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgnutls-openssl-27.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgnutlsxx-28.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgomp-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgpg-error-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgstcodecparsers-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libgsturidownloader-1.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libhogweed-2-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libiconv-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libjpeg-8.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libmms-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libnettle-4-7.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libopus-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\liborc-0.4-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\librtmp.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libsoup-2.4-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libtasn1-3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libtheora-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libtheoradec-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libtheoraenc-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libtiff-5.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\bin\libxml2-2.dll"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: codecsbad

Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstmatroska.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdeinterlace.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstrmdemux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstasf.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdashdemux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdebug.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstasfmux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstopus.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstfragmented.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdebugutilsbad.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgsttheora.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstmultifile.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstwasapi.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsouphttpsrc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsiren.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsmoothstreaming.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstrawparse.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstinterleave.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstaiff.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstgdkpixbuf.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstreplaygain.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstautodetect.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstjpegformat.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstjpeg.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstwavpack.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgsttaglib.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstliveadder.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstpng.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstfieldanalysis.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstautoconvert.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstrtmp.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstid3tag.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstcoloreffects.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsdpelem.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsegmentclip.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstivtc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstwavenc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstpcapparse.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstlevel.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstpnm.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstcutter.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstauparse.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgsta52dec.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsubenc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstinterlace.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstfreeverb.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstmulaw.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdirectsoundsrc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstmms.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstxingmux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgsticydemux.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstdataurisrc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstadpcmenc.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstadpcmdec.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstaccurip.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstsmooth.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
Source: "{#GST_CODECS}\lib\gstreamer-1.0\libgstremovesilence.dll"; DestDir: "{app}\lib\gstreamer-1.0\"; Flags: ignoreversion; Components: codecsbad
