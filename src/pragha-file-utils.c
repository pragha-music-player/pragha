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
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-file-utils.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "pragha-utils.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-musicobject-mgmt.h"

/*
 * Mimetype handles by pragha
 */

#ifdef G_OS_WIN32
const gchar *mime_flac[] = {"audio/x-flac", "application/x-ext-flac", NULL};
const gchar *mime_mpeg[] = {"audio/mpeg", "application/x-ext-mp3", NULL};
const gchar *mime_ogg[] = {"audio/x-vorbis+ogg", "audio/ogg", "application/ogg", "application/x-ext-ogg", NULL};
const gchar *mime_wav[] = {"audio/x-wav", "audio/wav", "application/x-ext-wav", NULL};
const gchar *mime_asf[] = {"video/x-ms-asf", "audio/x-ms-wma", "application/x-ext-wma", NULL};
const gchar *mime_mp4 [] = {"audio/x-m4a", "application/x-ext-m4a", NULL};
const gchar *mime_ape [] = {"application/x-ape", "audio/ape", "audio/x-ape", NULL};
const gchar *mime_tracker[] = {"audio/x-mod", "audio/x-xm", "application/x-ext-mod", NULL};
#else
const gchar *mime_flac[] = {"audio/x-flac", NULL};
const gchar *mime_mpeg[] = {"audio/mpeg", NULL};
const gchar *mime_ogg[] = {"audio/x-vorbis+ogg", "audio/ogg", "application/ogg", NULL};
const gchar *mime_wav[] = {"audio/x-wav", NULL};
const gchar *mime_asf[] = {"video/x-ms-asf", "audio/x-ms-wma", NULL};
const gchar *mime_mp4 [] = {"audio/x-m4a", NULL};
const gchar *mime_ape [] = {"application/x-ape", "audio/ape", "audio/x-ape", NULL};
const gchar *mime_tracker[] = {"audio/x-mod", "audio/x-xm", NULL};
#endif

const gchar *mime_image[] = {"image/jpeg", "image/png", NULL};

/* Next based on http://git.gnome.org/browse/totem-pl-parser/tree/plparse/totem-pl-parser.c */
#ifdef HAVE_PLPARSER
const gchar *mime_playlist[] = {"audio/x-mpegurl",
					  "video/vnd.mpegurl",
					  "audio/playlist",
					  "audio/x-scpls",
					  "application/x-smil",
					  "application/smil",
					  "application/vnd.ms-wpl",
					  "video/x-ms-wvx",
					  "audio/x-ms-wax",
					  "application/xspf+xml",
					  "text/uri-list",
					  "text/x-google-video-pointer",
					  "text/google-video-pointer",
					  "audio/x-iriver-pla",
					  "application/atom+xml",
					  "application/rss+xml",
					  "text/x-opml+xml",
					  "audio/x-amzxml",
					  NULL};

/* These ones are "dual" types, might be a video, might be a parser. */
const gchar *mime_dual[] = {"audio/x-real-audio",
				    "audio/x-pn-realaudio",
				    "application/ram",
				    "application/vnd.rn-realmedia",
				    "audio/x-pn-realaudio-plugin",
				    "audio/vnd.rn-realaudio",
				    "audio/x-realaudio",
				    "audio/x-ms-asx",
				    "video/x-ms-asf",
				    "video/x-ms-wmv",
				    "audio/x-ms-wma",
				    "video/quicktime",
				    "video/mp4",
				    "application/x-quicktime-media-link",
				    "application/x-quicktimeplayer",
				    "application/xml",
				    NULL};
#endif

/*
 * Mimetype management.
 */

static gboolean
is_valid_mime(const gchar *mime, const gchar **mlist)
{
	gint i = 0;
	while (mlist[i]) {
		if (g_content_type_equals(mime, mlist[i]))
			return TRUE;
		i++;
	}
	return FALSE;
}

#ifdef G_OS_WIN32
/*
 * Next based on evince code.
 * See https://git.gnome.org/browse/evince/tree/libdocument/ev-file-helpers.c
 */
static gchar *
get_mime_type_from_uri (const gchar *uri, GError **error)
{
	GFile *file;
	GFileInfo *file_info;
	const gchar *content_type;
	gchar *mime_type = NULL;

	file = g_file_new_for_uri (uri);
	file_info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
	                               0, NULL, error);
	g_object_unref (file);

	if (file_info == NULL)
		return NULL;

	content_type = g_file_info_get_content_type (file_info);
	if (content_type != NULL) {
		mime_type = g_content_type_get_mime_type (content_type);
	}
	if (mime_type == NULL) {
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Unknown MIME Type");
	}

	g_object_unref (file_info);

	return mime_type;
}
#else
/* Accepts only absolute filename */
/* NB: Disregarding 'uncertain' flag for now. */
static gchar*
get_mime_type (const gchar *file)
{
	gboolean uncertain;
	gchar *result = NULL;

	result = g_content_type_guess(file, NULL, 0, &uncertain);

	return result;
}
#endif

gchar *
pragha_file_get_music_type(const gchar *filename)
{
	gchar *result = NULL;

	if (!filename)
		return NULL;

#ifdef G_OS_WIN32
	result = get_mime_type_from_uri (filename, NULL);
#else
	result = get_mime_type (filename);
#endif

	return result;
}

PraghaPlaylistType
pragha_pl_parser_guess_format_from_extension (const gchar *filename)
{
	if ( g_str_has_suffix (filename, ".m3u") || g_str_has_suffix (filename, ".M3U") )
		return PL_FORMAT_M3U;

	if ( g_str_has_suffix (filename, ".pls") || g_str_has_suffix (filename, ".PLS") )
		return PL_FORMAT_PLS;

	if ( g_str_has_suffix (filename, ".xspf") || g_str_has_suffix (filename, ".XSPF") )
		return PL_FORMAT_XSPF;

	if ( g_str_has_suffix (filename, ".asx") || g_str_has_suffix (filename, ".ASX") )
		return PL_FORMAT_ASX;

	if ( g_str_has_suffix (filename, ".wax") || g_str_has_suffix (filename, ".WAX") )
		return PL_FORMAT_XSPF;

	return PL_FORMAT_UNKNOWN;
}

/* Determine if the any file is useful to pragha. */

PraghaMediaType
pragha_file_get_media_type (const gchar *filename)
{
	PraghaMediaType ret = MEDIA_TYPE_UNKNOWN;
	gchar *result = NULL;

	if (!filename)
		return ret;

#ifdef G_OS_WIN32
	result = get_mime_type_from_uri (filename, NULL);
#else
	result = get_mime_type (filename);
#endif

	if (result) {
		if (is_valid_mime(result, mime_flac) ||
		    is_valid_mime(result, mime_mpeg) ||
		    is_valid_mime(result, mime_ogg) ||
		    is_valid_mime(result, mime_wav) ||
		    is_valid_mime(result, mime_asf) ||
		    is_valid_mime(result, mime_mp4) ||
		    is_valid_mime(result, mime_ape) ||
		    is_valid_mime(result, mime_tracker))
			ret = MEDIA_TYPE_AUDIO;
		#ifdef HAVE_PLPARSER
		else if (is_valid_mime(result, mime_playlist))
		#else
		else if (g_str_has_suffix (filename, ".m3u") || g_str_has_suffix (filename, ".M3U") ||
		         g_str_has_suffix (filename, ".pls") || g_str_has_suffix (filename, ".PLS") ||
		         g_str_has_suffix (filename, ".xspf") || g_str_has_suffix (filename, ".XSPF") ||
		         g_str_has_suffix (filename, ".asx") || g_str_has_suffix (filename, ".ASX") ||
		         g_str_has_suffix (filename, ".wax") || g_str_has_suffix (filename, ".WAX"))
		#endif
			ret = MEDIA_TYPE_PLAYLIST;
		else if (is_valid_mime(result, mime_image))
			ret = MEDIA_TYPE_IMAGE;
		else if (g_str_has_prefix(result, "audio/") || g_str_has_prefix(result, "video/"))
			ret = MEDIA_TYPE_AUDIO;
	}

	g_free(result);

	return ret;
}

/* Return true if given file is an image */

static gboolean
is_image_file(const gchar *file)
{
	gboolean ret = FALSE;
	gchar *result = NULL;

	if (!file)
		return FALSE;

	/* Type: JPG, PNG */

#ifdef G_OS_WIN32
	result = get_mime_type_from_uri (file, NULL);
#else
	result = get_mime_type (file);
#endif
	if (!result)
		return FALSE;
	else {
		ret = is_valid_mime(result, mime_image);
		g_free(result);
		return ret;
	}
}

/*
 *Generate and add the recently-used data
 */

static void
add_recent_file (const gchar *filename)
{
	GtkRecentData recent_data;
	gchar *uri = NULL;

#ifdef G_OS_WIN32
	recent_data.mime_type = get_mime_type_from_uri (filename, NULL);
#else
	recent_data.mime_type = get_mime_type (filename);
#endif

	if (recent_data.mime_type == NULL)
		return;

	recent_data.display_name = g_filename_display_basename (filename);
	recent_data.app_name = g_strdup (g_get_application_name ());
	recent_data.app_exec =  g_strjoin (" ", g_get_prgname (), "%u", NULL);
	recent_data.description = NULL;
	recent_data.groups = NULL;
	recent_data.is_private = FALSE;

	uri = g_filename_to_uri(filename, NULL, NULL);
	gtk_recent_manager_add_full(gtk_recent_manager_get_default(), uri, &recent_data);

	g_free (recent_data.display_name);
	g_free (recent_data.mime_type);
	g_free (recent_data.app_name);
	g_free (recent_data.app_exec);
	g_free (uri);
}

/* Accepts only absolute filename */

gboolean is_playable_file(const gchar *file)
{
	if (!file)
		return FALSE;

	if (g_file_test(file, G_FILE_TEST_IS_REGULAR) &&
	    (pragha_file_get_media_type(file) != MEDIA_TYPE_AUDIO))
		return TRUE;
	else
		return FALSE;
}

/* Accepts only absolute path */

gboolean is_dir_and_accessible(const gchar *dir)
{
	gint ret;

	if (!dir)
		return FALSE;

	if (g_file_test(dir, G_FILE_TEST_IS_DIR) && !g_access(dir, R_OK | X_OK))
		ret = TRUE;
	else
		ret = FALSE;

	return ret;
}

/* Get the first image file from the directory */

gchar*
get_image_path_from_dir (const gchar *path)
{
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL;
	gchar *result = NULL;

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, G_DIR_SEPARATOR_S, next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR) &&
		    is_image_file(ab_file)) {
			result = ab_file;
			goto exit;
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}

exit:
	g_dir_close(dir);
	return result;
}

/* Find out if any of the preferred album art files are present in the given dir.
   Runs through the patterns in sequence */

gchar*
get_pref_image_path_dir (PraghaPreferences *preferences, const gchar *path)
{
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL, **pattern;
	const gchar *patterns = NULL;
	GSList *file_list = NULL;
	gint i = 0;

	patterns = pragha_preferences_get_album_art_pattern(preferences);

	if (string_is_empty(patterns))
		return NULL;

	/* Form a list of all files in the given dir */

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, G_DIR_SEPARATOR_S, next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR))
			file_list = g_slist_append(file_list, g_strdup(next_file));

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	/* Now, run the preferred patterns through them */

	pattern = g_strsplit(patterns, ";", ALBUM_ART_NO_PATTERNS);
	while (pattern[i]) {
		if (is_present_str_list(pattern[i], file_list)) {
			ab_file = g_strconcat(path, G_DIR_SEPARATOR_S, pattern[i], NULL);
			if (is_image_file(ab_file))
				return ab_file;
			g_free(ab_file);
		}
		i++;
	}

	/* Cleanup */

	g_slist_free_full(file_list, g_free);
	g_strfreev(pattern);

	return NULL;
}

/* Count files on a folder. */

gint pragha_get_dir_count(const gchar *dir_name, GCancellable *cancellable)
{
	gint file_count = 0;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_warning("Unable to open library : %s", dir_name);
		return file_count;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if(g_cancellable_is_cancelled (cancellable))
			return 0;

		ab_file = g_strconcat(dir_name, G_DIR_SEPARATOR_S, next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			file_count += pragha_get_dir_count(ab_file, cancellable);
		else {
			file_count++;
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}

	g_dir_close(dir);

	return file_count;
}

GList *
append_mobj_list_from_folder(GList *list, gchar *dir_name)
{
	PraghaPreferences *preferences;
	PraghaMusicobject *mobj = NULL;
	PraghaMediaType file_type;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return list;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, G_DIR_SEPARATOR_S, next_file, NULL);

		if (is_dir_and_accessible(ab_file)) {
			preferences = pragha_preferences_get();
			if(pragha_preferences_get_add_recursively(preferences))
				list = append_mobj_list_from_folder(list, ab_file);
			g_object_unref(G_OBJECT(preferences));
		}
		else {
			file_type = pragha_file_get_media_type (ab_file);
			switch (file_type) {
				case MEDIA_TYPE_AUDIO:
					mobj = new_musicobject_from_file (ab_file, NULL);
					if (G_LIKELY(mobj))
						list = g_list_append(list, mobj);
					break;
				case MEDIA_TYPE_PLAYLIST:
					list = pragha_pl_parser_append_mobj_list_by_extension (list, ab_file);
					break;
				case MEDIA_TYPE_IMAGE:
				case MEDIA_TYPE_UNKNOWN:
				default:
					break;
			}
		}

		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}

	g_dir_close(dir);

	return list;
}

GList *
append_mobj_list_from_unknown_filename(GList *list, gchar *filename)
{
	PraghaMusicobject *mobj;
	PraghaMediaType file_type;

	if (is_dir_and_accessible(filename)) {
		list = append_mobj_list_from_folder (list, filename);
	}
	else {
		file_type = pragha_file_get_media_type (filename);
		switch (file_type) {
			case MEDIA_TYPE_AUDIO:
				mobj = new_musicobject_from_file (filename, NULL);
				if (G_LIKELY(mobj)) {
					list = g_list_append(list, mobj);
					add_recent_file(filename);
				}
				break;
			case MEDIA_TYPE_PLAYLIST:
				list = pragha_pl_parser_append_mobj_list_by_extension (list, filename);
				break;
			case MEDIA_TYPE_IMAGE:
			case MEDIA_TYPE_UNKNOWN:
			default:
				break;
		}
	}

	/* Have to give control to GTK periodically ... */
	pragha_process_gtk_events ();

	return list;
}
