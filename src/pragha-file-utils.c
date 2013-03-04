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

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "pragha.h" //new_musicobject_from_file()
#include "pragha-file-utils.h"
#include "pragha-musicobject.h"
#include "pragha-preferences.h"
#include "pragha-utils.h"
#include "pragha-playlists-mgmt.h"

/*
 * Mimetype handles by pragha
 */

const gchar *mime_flac[] = {"audio/x-flac", NULL};
const gchar *mime_mpeg[] = {"audio/mpeg", NULL};
const gchar *mime_ogg[] = {"audio/x-vorbis+ogg", "audio/ogg", "application/ogg", NULL};
const gchar *mime_wav[] = {"audio/x-wav", NULL};
const gchar *mime_asf[] = {"video/x-ms-asf", "audio/x-ms-wma", NULL};
const gchar *mime_mp4 [] = {"audio/x-m4a", NULL};
const gchar *mime_ape [] = {"application/x-ape", "audio/ape", "audio/x-ape", NULL};

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

enum file_type
get_file_type(const gchar *file)
{
	gint ret = -1;
	gchar *result = NULL;

	if (!file)
		return -1;

	result = get_mime_type(file);

	if (result) {
		if(is_valid_mime(result, mime_flac))
			ret = FILE_FLAC;
		else if(is_valid_mime(result, mime_mpeg))
			ret = FILE_MP3;
		else if(is_valid_mime(result, mime_ogg))
			ret = FILE_OGGVORBIS;
		else if (is_valid_mime(result, mime_wav))
			ret = FILE_WAV;
		else if (is_valid_mime(result, mime_asf))
			ret = FILE_ASF;
		else if (is_valid_mime(result, mime_mp4))
			ret = FILE_MP4;
		else if (is_valid_mime(result, mime_ape))
			ret = FILE_APE;

		else ret = -1;
	}

	g_free(result);
	return ret;
}

enum playlist_type
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

/* Return true if given file is an image */

gboolean is_image_file(const gchar *file)
{
	gboolean uncertain = FALSE, ret = FALSE;
	gchar *result = NULL;

	if (!file)
		return FALSE;

	/* Type: JPG, PNG */

	result = g_content_type_guess(file, NULL, 0, &uncertain);

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

	recent_data.mime_type = get_mime_type(filename);
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
	    (get_file_type(file) != -1))
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

		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
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

gint dir_file_count(const gchar *dir_name, gint call_recur)
{
	static gint file_count = 0;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	/* Reinitialize static variable if called from rescan_library_action */

	if (call_recur)
		file_count = 0;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_warning("Unable to open library : %s", dir_name);
		return file_count;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			dir_file_count(ab_file, 0);
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
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);

		if (is_dir_and_accessible(ab_file)) {
			preferences = pragha_preferences_get();
			if(pragha_preferences_get_add_recursively(preferences))
				list = append_mobj_list_from_folder(list, ab_file);
			g_object_unref(G_OBJECT(preferences));
		}
		else {
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (G_LIKELY(mobj))
					list = g_list_append(list, mobj);
			}
		}

		/* Have to give control to GTK periodically ... */
		if (pragha_process_gtk_events ())
			return NULL;

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

	if (is_dir_and_accessible(filename)) {
		list = append_mobj_list_from_folder(list, filename);
	}
	else if (pragha_pl_parser_guess_format_from_extension(filename) != PL_FORMAT_UNKNOWN) {
		list = pragha_pl_parser_append_mobj_list_by_extension(list, filename);
	}
	else {
		if (is_playable_file(filename)) {
			mobj = new_musicobject_from_file(filename);
			if (G_LIKELY(mobj)) {
				list = g_list_append(list, mobj);
				add_recent_file(filename);
			}
		}
	}

	/* Have to give control to GTK periodically ... */
	pragha_process_gtk_events ();

	return list;
}
