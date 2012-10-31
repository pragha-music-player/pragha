/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

const gchar *mime_flac[] = {"audio/x-flac", NULL};
const gchar *mime_mpeg[] = {"audio/mpeg", NULL};
const gchar *mime_ogg[] = {"audio/x-vorbis+ogg", "audio/ogg", "application/ogg", NULL};
const gchar *mime_wav[] = {"audio/x-wav", NULL};

#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
const gchar *mime_asf[] = {"video/x-ms-asf", "audio/x-ms-wma", NULL};
#endif
#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
const gchar *mime_mp4 [] = {"audio/x-m4a", NULL};
#endif
#ifdef HAVE_TAGLIB_1_7
const gchar *mime_ape [] = {"application/x-ape", "audio/ape", "audio/x-ape", NULL};
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

/* Function to save debug on file. */

void
pragha_log_to_file (const gchar* log_domain,
		    GLogLevelFlags log_level,
		    const gchar* message,
		    gpointer user_data)
{
	FILE* logfile = fopen ((const char*)user_data, "a");
	gchar* level_name = "";

	switch (log_level)
	{
	/* skip irrelevant flags */
	case G_LOG_LEVEL_MASK:
	case G_LOG_FLAG_FATAL:
	case G_LOG_FLAG_RECURSION:
	case G_LOG_LEVEL_ERROR:
		level_name = "ERROR";
		break;
	case G_LOG_LEVEL_CRITICAL:
		level_name = "CRITICAL";
		break;
	case G_LOG_LEVEL_WARNING:
		level_name = "WARNING";
		break;
	case G_LOG_LEVEL_MESSAGE:
		level_name = "MESSAGE";
		break;
	case G_LOG_LEVEL_INFO:
		level_name = "INFO";
		break;
	case G_LOG_LEVEL_DEBUG:
		level_name = "DEBUG";
		break;
	}

	fprintf (logfile, "%s %s: %s\n",
	log_domain ? log_domain : "Pragha", level_name, message);
	fclose (logfile);
}

/**
@brief duplicate utf8 string, truncated after @a num characters if the string is longer than that
@param str the string to be duplicated
@param num maximum no. of characters in @a str to be processed
@return the duplicated string
* Based on emelfm2 code.
*/
gchar *e2_utf8_ndup (const gchar *str, glong num)
{
	glong size = g_utf8_strlen (str, -1);
	if (num > size)
		num = size;
	gchar *end = g_utf8_offset_to_pointer (str, num);
	glong byte_size = end - str + 1;
	gchar *utf8 = g_malloc (byte_size);
	return g_utf8_strncpy (utf8, str, num);
}

/* Compare two strings and returns the levenshtein distance.
 * Based on glyr code. Thanks to cpahl. */

gsize levenshtein_strcmp(const gchar * s, const gchar * t)
{
    int n = (s) ? g_utf8_strlen(s,-1)+1 : 0;
    int m = (t) ? g_utf8_strlen(t,-1)+1 : 0;

    // NOTE: Be sure to call g_utf8_validate(), might fail otherwise
    // It's advisable to call g_utf8_normalize() too.

    // Nothing to compute really..
    if (n < 2) return m;
    if (m < 2) return n;

    // String matrix
    int d[n][m];
    int i,j;

    // Init first row|column to 0...n|m
    for (i=0; i<n; i++) d[i][0] = i;
    for (j=0; j<m; j++) d[0][j] = j;

    for (i=1; i<n; i++)
    {
        // Current char in string s
        gunichar cats = g_utf8_get_char(g_utf8_offset_to_pointer(s,i-1));

        for (j=1; j<m; j++)
        {
            // Do -1 only once
            int jm1 = j-1,
                im1 = i-1;

            gunichar tats = g_utf8_get_char(g_utf8_offset_to_pointer(t,jm1));

            // a = above cell, b = left cell, c = left above celli
            int a = d[im1][j] + 1,
                b = d[i][jm1] + 1,
                c = d[im1][jm1] + (tats != cats);
    
            // Now compute the minimum of a,b,c and set MIN(a,b,c) to cell d[i][j]
            d[i][j] = (a < b) ? MIN(a,c) : MIN(b,c);
        }
    }

    // The result is stored in the very right down cell
    return d[n-1][m-1];
}

gsize levenshtein_safe_strcmp(const gchar * s, const gchar * t)
{
	gsize rc = 100;

	if(g_utf8_validate(s,-1,NULL) == FALSE ||
	   g_utf8_validate(t,-1,NULL) == FALSE)
		return rc;

	gchar * s_norm = g_utf8_normalize(s, -1 ,G_NORMALIZE_ALL_COMPOSE);
	gchar * t_norm = g_utf8_normalize(t, -1, G_NORMALIZE_ALL_COMPOSE);

	rc = levenshtein_strcmp(s_norm, t_norm);

	g_free(s_norm);
	g_free(t_norm);

	return rc;
}

/* Searches the string haystack for the first occurrence of the string needle
 * considering a maximum levenshtein distance. */

gchar *
g_strstr_lv (gchar *haystack, gchar *needle, gsize lv_distance)
{
	gint needle_len = 0, haystack_len = 0, count = 0;
	gchar *needle_buf = NULL, *rv = NULL;
 
 	haystack_len = g_utf8_strlen(haystack, -1);
	needle_len = g_utf8_strlen(needle, -1);

	/* UTF-8 bytes are 4 bytes length in the worst case */
	needle_buf = g_malloc0(needle_len * 4 + 1);

	do {
		g_utf8_strncpy(needle_buf, haystack, needle_len);

		if (needle_len > 3 && lv_distance != 0) {
			if(levenshtein_safe_strcmp(needle_buf, needle) <= lv_distance) {
				rv = haystack;
				break;
			}
		}
		else {
			if(g_ascii_strcasecmp(needle_buf, needle) == 0) {
				rv = haystack;
				break;
			}
		}
		haystack = g_utf8_next_char(haystack);

	} while(needle_len + count++ < haystack_len);

	g_free(needle_buf);

	return rv;
}

/* Searches the string haystack for the first occurrence of the string needle,
 * considering the aproximate_search option. */

gchar *
pragha_strstr_lv(gchar *haystack, gchar *needle, struct con_win *cwin)
{
	return g_strstr_lv(haystack, needle,
			   cwin->cpref->aproximate_search ? 1 : 0);
}

#if !GLIB_CHECK_VERSION(2,32,0)
/* Functions to check the network manager status. */

static NMState
dbus_check_nm_status (GDBusProxy *proxy)
{
	GVariant *tuple = g_dbus_proxy_call_sync(proxy,
						  "state",
						  NULL,
						  G_DBUS_CALL_FLAGS_NONE,
						  -1,
						  NULL,
						  NULL);

	if (!tuple)
		return NM_STATE_UNKNOWN;

	guint32 state;
	g_variant_get(tuple, "(u)", &state);

	g_variant_unref(tuple);

	return state;
}

gboolean
nm_is_online ()
{
	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
							  G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
							  NULL,
							  NM_DBUS_SERVICE,
							  NM_DBUS_PATH,
							  NM_DBUS_INTERFACE,
							  NULL,
							  NULL);

	if (!proxy)
		return FALSE;

	NMState state = dbus_check_nm_status (proxy);

	g_object_unref(proxy);

	if (state == NM_STATE_CONNECTED_LOCAL ||
	    state == NM_STATE_CONNECTED_SITE ||
	    state == NM_STATE_CONNECTED_GLOBAL)
		return TRUE;

	return FALSE;
}
#endif

/* Test if the song is already in the playlist.*/

gboolean
already_in_current_playlist(struct musicobject *mobj, struct con_win *cwin)
{
	GtkTreeModel *playlist_model;
	GtkTreeIter playlist_iter;
	struct musicobject *omobj = NULL;
	gboolean ret;

	playlist_model = gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->current_playlist));

	ret = gtk_tree_model_get_iter_first (playlist_model, &playlist_iter);
	while (ret) {
		gtk_tree_model_get (playlist_model, &playlist_iter, P_MOBJ_PTR, &omobj, -1);

		if(0 == g_strcmp0(mobj->file, omobj->file))
		   	return TRUE;

		ret = gtk_tree_model_iter_next(playlist_model, &playlist_iter);
	}

	return FALSE;
}

gboolean
already_has_title_of_artist_in_current_playlist(const gchar *title,
						const gchar *artist,
						struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	gboolean ret;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->current_playlist));
	ret = gtk_tree_model_get_iter_first (model, &iter);
	while (ret) {
		gtk_tree_model_get (model, &iter, P_MOBJ_PTR, &mobj, -1);

		if((0 == g_ascii_strcasecmp(mobj->tags->title, title)) &&
		   (0 == g_ascii_strcasecmp(mobj->tags->artist, artist)))
		   	return TRUE;

		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return FALSE;
}

/* Find a song with the artist and title independently of the album and adds it to the playlist */

GList *
prepend_song_with_artist_and_title_to_mobj_list(const gchar *artist,
						const gchar *title,
						GList *list,
						struct con_win *cwin)
{
	gchar *query = NULL;
	struct db_result result;
	struct musicobject *mobj = NULL;
	gint location_id = 0, i;
	gchar *sartist, *stitle;

	if(already_has_title_of_artist_in_current_playlist(title, artist, cwin))
		return list;

	sartist = sanitize_string_sqlite3(artist);
	stitle = sanitize_string_sqlite3(title);

	query = g_strdup_printf("SELECT TRACK.title, ARTIST.name, LOCATION.id "
				"FROM TRACK, ARTIST, LOCATION "
				"WHERE ARTIST.id = TRACK.artist AND LOCATION.id = TRACK.location "
				"AND TRACK.title = '%s' COLLATE NOCASE "
				"AND ARTIST.name = '%s' COLLATE NOCASE "
				"ORDER BY RANDOM() LIMIT 1;",
				stitle, sartist);

	if(exec_sqlite_query(query, cwin->cdbase, &result)) {
		for_each_result_row(result, i) {
			location_id = atoi(result.resultp[i+2]);

			mobj = new_musicobject_from_db(location_id, cwin);
			list = g_list_prepend(list, mobj);
		}
		sqlite3_free_table(result.resultp);
	}
	g_free(sartist);
	g_free(stitle);

	return list;
}

gint
append_track_with_artist_and_title(const gchar *artist, const gchar *title, struct con_win *cwin)
{
	gchar *query = NULL;
	struct db_result result;
	struct musicobject *mobj = NULL;
	gint location_id = 0, i;
	gchar *sartist, *stitle;

	sartist = sanitize_string_sqlite3(artist);
	stitle = sanitize_string_sqlite3(title);

	query = g_strdup_printf("SELECT TRACK.title, ARTIST.name, LOCATION.id "
				"FROM TRACK, ARTIST, LOCATION "
				"WHERE ARTIST.id = TRACK.artist AND LOCATION.id = TRACK.location "
				"AND TRACK.title = '%s' COLLATE NOCASE "
				"AND ARTIST.name = '%s' COLLATE NOCASE;",
				stitle, sartist);

	if(exec_sqlite_query(query, cwin->cdbase, &result)) {
		for_each_result_row(result, i) {
			location_id = atoi(result.resultp[i+2]);

			mobj = new_musicobject_from_db(location_id, cwin);

			if(already_in_current_playlist(mobj, cwin) == FALSE) {
				append_current_playlist(NULL, mobj, cwin);
			}
			else {
				delete_musicobject(mobj);
				location_id = 0;
			}
			break;
		}
		sqlite3_free_table(result.resultp);
	}
	g_free(sartist);
	g_free(stitle);

	return location_id;
}

/* Get the musicobject of seleceted track on current playlist */

struct musicobject *
get_selected_musicobject(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list != NULL) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (!mobj)
				g_warning("Invalid mobj pointer");
		}
		gtk_tree_path_free(path);
		g_list_free(list);
	}
	return mobj;
}

/* Set and remove the watch cursor to suggest background work.*/

void
set_watch_cursor (GtkWidget *window)
{
	GdkCursor *cursor = gdk_cursor_new (GDK_WATCH);
	gdk_window_set_cursor (gtk_widget_get_window (window), cursor);
	gdk_cursor_unref (cursor);
}

void
remove_watch_cursor (GtkWidget *window)
{
	gdk_window_set_cursor (gtk_widget_get_window (window), NULL);
}

/* Set a message on status bar, and restore it at 5 seconds */

gboolean restore_status_bar(gpointer data)
{
	struct con_win *cwin = data;

	update_status_bar(cwin);

	return FALSE;
}

void set_status_message (const gchar *message, struct con_win *cwin)
{
	g_timeout_add_seconds(5, restore_status_bar, cwin);

	gtk_label_set_text(GTK_LABEL(cwin->status_bar), message);
}

/* Obtain Pixbuf of lastfm. Based on Amatory code. */

GdkPixbuf *vgdk_pixbuf_new_from_memory(const char *data, size_t size)
{
	GInputStream *buffer_stream=NULL;
	GdkPixbuf *buffer_pix=NULL;
	GError *err = NULL;

	buffer_stream = g_memory_input_stream_new_from_data (data, size, NULL);
	
	buffer_pix = gdk_pixbuf_new_from_stream(buffer_stream, NULL, &err);
	g_input_stream_close(buffer_stream, NULL, NULL);
	g_object_unref(buffer_stream);

	if(buffer_pix == NULL){
		g_warning("vgdk_pixbuf_new_from_memory: %s\n",err->message);
		g_error_free (err);	
	}
	return buffer_pix;
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

static gint no_single_quote(const gchar *str)
{
	const gchar *tmp = str;
	gint i = 0;

	if (!str)
		return 0;

	while (*tmp) {
		if (*tmp == '\'') {
			i++;
		}
		tmp++;
	}
	return i;
}

/* Replace ' by '' */

gchar* sanitize_string_sqlite3(const gchar *str)
{
	gint cn, i=0;
	gchar *ch;
	const gchar *tmp;

	if (!str)
		return NULL;

	cn = no_single_quote(str);
	ch = g_malloc0(strlen(str) + cn + 1);
	tmp = str;

	while (*tmp) {
		if (*tmp == '\'') {
			ch[i++] = '\'';
			ch[i++] = '\'';
			tmp++;
			continue;
		}
		ch[i++] = *tmp++;
	}
	return ch;
}

static gboolean is_valid_mime(const gchar *mime, const gchar **mlist)
{
	gint i=0;

	while (mlist[i]) {
		if (g_content_type_equals(mime, mlist[i]))
			return TRUE;
		i++;
	}

	return FALSE;
}

/* Accepts only absolute filename */
/* NB: Disregarding 'uncertain' flag for now. */

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
		#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
		else if (is_valid_mime(result, mime_asf))
			ret = FILE_ASF;
		#endif
		#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
		else if (is_valid_mime(result, mime_mp4))
			ret = FILE_MP4;
		#endif
		#ifdef HAVE_TAGLIB_1_7
		else if (is_valid_mime(result, mime_ape))
			ret = FILE_APE;
		#endif

		else ret = -1;
	}

	g_free(result);
	return ret;
}

gchar* get_mime_type (const gchar *file)
{
	gboolean uncertain;
	gchar *result = NULL;

	result = g_content_type_guess(file, NULL, 0, &uncertain);

	return result;
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

/* NB: Have to take care of longer lengths .. */

gchar* convert_length_str(gint length)
{
	static gchar *str, tmp[24];
	gint days = 0, hours = 0, minutes = 0, seconds = 0;

	str = g_new0(char, 128);
	memset(tmp, '\0', 24);

	if (length > 86400) {
		days = length/86400;
		length = length%86400;
		g_sprintf(tmp, "%d %s, ", days, ngettext("day", "days", days));
		g_strlcat(str, tmp, 24);
	}

	if (length > 3600) {
		hours = length/3600;
		length = length%3600;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%d:", hours);
		g_strlcat(str, tmp, 24);
	}

	if (length > 60) {
		minutes = length/60;
		length = length%60;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%02d:", minutes);
		g_strlcat(str, tmp, 24);
	}
	else
		g_strlcat(str, "00:", 4);

	seconds = length;
	memset(tmp, '\0', 24);
	g_sprintf(tmp, "%02d", seconds);
	g_strlcat(str, tmp, 24);

	return str;
}

/* Check if str is present in list ( containing gchar* elements in 'data' ) */

gboolean is_present_str_list(const gchar *str, GSList *list)
{
	GSList *i;
	gchar *lstr;
	gboolean ret = FALSE;

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			lstr = i->data;
			if (!g_ascii_strcasecmp(str, lstr)) {
				ret = TRUE;
				break;
			}
		}
	}
	else {
		ret = FALSE;
	}

	return ret;
}

/* Delete str from list */

GSList* delete_from_str_list(const gchar *str, GSList *list)
{
	GSList *i = NULL;
	gchar *lstr;

	if (!list)
		return NULL;

	for (i = list; i != NULL; i = i->next) {
		lstr = i->data;
		if (!g_ascii_strcasecmp(str, lstr)) {
			g_free(i->data);
			return g_slist_delete_link(list, i);
		}
	}

	return list;
}

gchar *
path_get_dir_as_uri (const gchar *path)
{
	gchar *dir = g_path_get_dirname (path);
	gchar *uri = g_filename_to_uri (dir, NULL, NULL);
	g_free (dir);
	return uri;
}

/* Returns either the basename of the given filename, or (if the parameter 
 * get_folder is set) the basename of the container folder of filename. In both
 * cases the returned string is encoded in utf-8 format. If GLib can not make
 * sense of the encoding of filename, as a last resort it replaces unknown
 * characters with U+FFFD, the Unicode replacement character */

gchar* get_display_filename(const gchar *filename, gboolean get_folder)
{
	gchar *utf8_filename = NULL;
	gchar *dir = NULL;

	/* Get the containing folder of the file or the file itself ? */
	if (get_folder) {
		dir = g_path_get_dirname(filename);
		utf8_filename = g_filename_display_name(dir);
		g_free(dir);
	}
	else {
		utf8_filename = g_filename_display_basename(filename);
	}
	return utf8_filename;
}

/* Free a list of strings */

void free_str_list(GSList *list)
{
	g_slist_free_full(list, g_free);
}

/* Compare two UTF-8 strings */

gint compare_utf8_str(const gchar *str1, const gchar *str2)
{
	gchar *key1, *key2;
	gint ret = 0;

	if (!str1)
		return 1;

	if (!str2)
		return -1;

	key1 = g_utf8_collate_key(str1, -1);
	key2 = g_utf8_collate_key(str2, -1);

	ret = strcmp(key1, key2);

	g_free(key1);
	g_free(key2);

	return ret;
}

gboolean validate_album_art_pattern(const gchar *pattern)
{
	gchar **tokens;
	gint i = 0;
	gboolean ret = FALSE;

	if (!pattern || (pattern && !strlen(pattern)))
		return TRUE;

	if (g_strrstr(pattern, "*")) {
		g_warning("Contains wildcards");
		return FALSE;
	}

	tokens = g_strsplit(pattern, ";", 0);
	while (tokens[i]) i++;

	/* Check if more than six patterns are given */

	if (i > ALBUM_ART_NO_PATTERNS) {
		g_warning("More than six patterns");
		goto exit;
	}

	ret = TRUE;
exit:
	if (tokens)
		g_strfreev(tokens);

	return ret;
}

gboolean
pragha_process_gtk_events ()
{
	while (gtk_events_pending ()) {
		if (gtk_main_iteration_do (FALSE))
			return TRUE;
	}
	return FALSE;
}

/* callback used to open default browser when URLs got clicked */
void open_url(struct con_win *cwin, const gchar *url)
{
	gboolean success = TRUE;
	const gchar *argv[3];
	gchar *methods[] = {"xdg-open","firefox","mozilla","opera","konqueror",NULL};
	int i = 0;

	/* First try gtk_show_uri() (will fail if gvfs is not installed) */
	if (!gtk_show_uri (NULL, url,  gtk_get_current_event_time (), NULL)) {
		success = FALSE;
		argv[1] = url;
		argv[2] = NULL;
		/* Next try all available methods for opening the URL */
		while (methods[i] != NULL) {
			argv[0] = methods[i++];
			if (g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH,
				NULL, NULL, NULL, NULL)) {
				success = TRUE;
				break;
			}
		}
	}
	/* No method was found to open the URL */
	if (!success) {
		GtkWidget *d;
		d = gtk_message_dialog_new (GTK_WINDOW (cwin->mainwindow),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
					"%s", _("Unable to open the browser"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (d),
							 "%s", "No methods supported");
		g_signal_connect (d, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_window_present (GTK_WINDOW (d));
	}
}

/* It gives the position of the menu on the 
   basis of the position of combo_order */

void
menu_position(GtkMenu *menu,
		gint *x, gint *y,
		gboolean *push_in,
		gpointer user_data)
{
	GtkWidget *widget;
	GtkRequisition requisition;
	gint menu_xpos;
	gint menu_ypos;

	widget = GTK_WIDGET (user_data);

	gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

	gdk_window_get_origin (gtk_widget_get_window(widget), &menu_xpos, &menu_ypos);

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	menu_xpos += allocation.x;
	menu_ypos += allocation.y;

	if (menu_ypos > gdk_screen_get_height (gtk_widget_get_screen (widget)) / 2)
		menu_ypos -= requisition.height + gtk_widget_get_style(widget)->ythickness;
	else
		menu_ypos += allocation.height + gtk_widget_get_style(widget)->ythickness;

	*x = menu_xpos;
	*y = menu_ypos - 5;

	*push_in = TRUE;
}

/* Return TRUE if the previous installed version is
   incompatible with the current one */

gboolean is_incompatible_upgrade(struct con_win *cwin)
{
	/* Lesser than 0.2, version string is non-existent */

	if (!cwin->cpref->installed_version)
		return TRUE;

	if (atof(cwin->cpref->installed_version) < atof(PACKAGE_VERSION))
		return TRUE;

	return FALSE;
}
