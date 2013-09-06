/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/**************************************************************************/

#ifndef PRAGHA_FILE_UTILS_H
#define PRAGHA_FILE_UTILS_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gio/gio.h>
#include "pragha-preferences.h"

/* File music types */

enum file_type {
	FILE_WAV,
	FILE_MP3,
	FILE_FLAC,
	FILE_OGGVORBIS,
	FILE_ASF,
	FILE_MP4,
	FILE_APE,
	FILE_CDDA,
	FILE_HTTP,
#if HAVE_LIBMTP
	FILE_MTP,
#endif
	LAST_FILE_TYPE
};

/* Playlist type formats */

enum playlist_type {
	PL_FORMAT_UNKNOWN,
	PL_FORMAT_M3U,
	PL_FORMAT_PLS,
	PL_FORMAT_ASX,
	PL_FORMAT_XSPF
};

enum generic_type {
	MEDIA_TYPE_UNKNOWN,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_PLAYLIST,
	MEDIA_TYPE_IMAGE
};

extern const gchar *mime_mpeg[];
extern const gchar *mime_wav[];
extern const gchar *mime_flac[];
extern const gchar *mime_ogg[];
extern const gchar *mime_asf[];
extern const gchar *mime_mp4[];
extern const gchar *mime_ape[];

extern const gchar *mime_image[];

#ifdef HAVE_PLPARSER
extern const gchar *mime_playlist[];
extern const gchar *mime_dual[];
#endif

gboolean is_playable_file(const gchar *file);
enum generic_type pragha_file_get_generic_type (const gchar *filename);
enum file_type get_file_type(const gchar *file);
gboolean is_dir_and_accessible(const gchar *dir);

gchar    *get_image_path_from_dir (const gchar *path);
gchar    *get_pref_image_path_dir (PraghaPreferences *preferences, const gchar *path);

gint pragha_get_dir_count(const gchar *dir_name, GCancellable *cancellable);

GList *append_mobj_list_from_folder(GList *list, gchar *dir_name);
GList *append_mobj_list_from_unknown_filename(GList *list, gchar *filename);

#endif /* PRAGHA_FILE_UTILS_H */