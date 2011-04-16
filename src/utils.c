/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
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

#include <string.h>
#include <fcntl.h>
#include "consonance.h"

const gchar *mime_mpeg[] = {"audio/mpeg", NULL};
const gchar *mime_wav[] = {"audio/x-wav", NULL};
const gchar *mime_flac[] = {"audio/x-flac", NULL};
const gchar *mime_ogg[] = {"audio/x-vorbis+ogg", "audio/ogg",
			   "application/ogg", NULL};
const gchar *mime_modplug[] = {"audio/x-mod", "audio/x-xm", NULL};
const gchar *mime_image[] = {"image/jpeg", "image/png", NULL};

/* Accepts only absolute filename */

gboolean is_hidden_file(const gchar *file)
{
	gchar *base_name;
	gint ret;

	if (!file)
		return FALSE;

	base_name = g_path_get_basename(file);
	if (base_name[0] == '.')
		ret = TRUE;
	else
		ret = FALSE;

	g_free(base_name);
	return ret;
}

/* Accepts only absolute filename */

gboolean is_playable_file(const gchar *file)
{
	if (!file)
		return FALSE;

	if (g_file_test(file, G_FILE_TEST_IS_REGULAR) &&
	    (get_file_type((gchar*)file) != -1))
		return TRUE;
	else
		return FALSE;
}

/* Accepts only basename */

gboolean is_base_dir_and_accessible(gchar *file, struct con_win *cwin)
{
	gint ret;
	gchar *absolute_name;

	if (!file)
		return FALSE;

	absolute_name = g_strdup_printf("%s/%s", cwin->cstate->file_tree_pwd,
					file);
	if (g_file_test(absolute_name,
			G_FILE_TEST_IS_DIR) &&
	    !g_access(absolute_name, R_OK | X_OK))
		ret = TRUE;
	else
		ret = FALSE;

	g_free(absolute_name);
	return ret;
}

/* Accepts only absolute path */

gboolean is_dir_and_accessible(gchar *dir, struct con_win *cwin)
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

gint dir_file_count(gchar *dir_name, gint call_recur)
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

static gint no_single_quote(gchar *str)
{
	gchar *tmp = str;
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

gchar* sanitize_string_sqlite3(gchar *str)
{
	gint cn, i=0;
	gchar *ch;
	gchar *tmp;

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

static gboolean is_valid_mime(gchar *mime, const gchar **mlist)
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

enum file_type get_file_type(gchar *file)
{
	gint ret = -1;
	gboolean uncertain;
	gchar *result = NULL;

	if (!file)
		return -1;

	result = g_content_type_guess((const gchar *)file, NULL, 0, &uncertain);

	if (result) {
		if (is_valid_mime(result, mime_wav))
			ret = FILE_WAV;
		else if(is_valid_mime(result, mime_mpeg))
			ret = FILE_MP3;
		else if(is_valid_mime(result, mime_flac))
			ret = FILE_FLAC;
		else if(is_valid_mime(result, mime_ogg))
			ret = FILE_OGGVORBIS;
		else if(is_valid_mime(result, mime_modplug))
			ret = FILE_MODPLUG;
		else ret = -1;
	}

	g_free(result);
	return ret;
}

/* Return true if given file is an image */

gboolean is_image_file(gchar *file)
{
	gboolean uncertain = FALSE, ret = FALSE;
	gchar *result = NULL;

	if (!file)
		return FALSE;

	/* Type: JPG, PNG */

	result = g_content_type_guess((const gchar*)file, NULL, 0, &uncertain);

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
/*        gchar *res;*/
/*        gchar *temp1, *temp2;*/
/*        gint temp_time=0;*/
/*        gint sec=0, min=0, hours=0, days=0;*/

/*        if (length < 0)*/
/*                return g_strdup_printf ("n/a");*/

/*        days = (length / 86400);*/
/*        temp_time = (time % 86400);*/

/*        hours = (temp_time / 3600);*/
/*        temp_time = (temp_time % 3600);*/

/*        min = (temp_time / 60);*/
/*        sec = (temp_time % 60);*/

/*        res = g_strdup_printf ("%d %s", sec, "seconds");*/

/*        if (min != 0) {*/
/*                temp1 = g_strdup_printf ("%d %s, ", min, "minutes");*/
/*                temp2 = g_strconcat (temp1, res, NULL);*/
/*                g_free (temp1);*/
/*                g_free (res);*/
/*                res = temp2;*/
/*        }*/

/*        if (hours != 0) {*/
/*                temp1 = g_strdup_printf ("%d %s, ", hours, "hours");*/
/*                temp2 = g_strconcat (temp1, res, NULL);*/
/*                g_free (temp1);*/
/*                g_free (res);*/
/*                res = temp2;*/
/*        }*/

/*        if (days != 0) {*/
/*                temp1 = g_strdup_printf ("%d %s, ", days, "days");*/
/*                temp2 = g_strconcat (temp1, res, NULL);*/
/*                g_free (temp1);*/
/*                g_free (res);*/
/*                res = temp2;*/
/*        }*/

/*        return res;*/
	static gchar *str, tmp[24];
	gint days = 0, hours = 0, minutes = 0, seconds = 0;

	str = g_new0(char, 128);
	memset(tmp, '\0', 24);

	if (length > 86400) {
		days = length/86400;
		length = length%86400;
		g_sprintf(tmp, "%d %s, ", days, (days>1)?"days":"day");
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
	} else {
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

/* Return the containing folder of given path. Returned string must be freed. */

gchar* get_containing_folder(gchar *path)
{
	gchar *dir;
	gchar *base;

	if (!path)
		return NULL;

	dir = g_path_get_dirname(path);
	base = g_path_get_basename(dir);

	g_free(dir);

	return base;
}

/* Free a list of strings */

void free_str_list(GSList *list)
{
	gint cnt = 0, i;
	GSList *l = list;

	cnt = g_slist_length(list);

	for (i=0; i<cnt; i++) {
		g_free(l->data);
		l = l->next;
	}

	g_slist_free(list);
}

/* Compare two UTF-8 strings */

gint compare_utf8_str(gchar *str1, gchar *str2)
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

/* Returns TRUE if the given filename has m3u as extension */

gboolean is_m3u_playlist(gchar *file)
{
	gboolean ret = FALSE;
	gchar **tokens;
	gchar *base;
	gint len = 0;

	base = g_path_get_basename(file);
	if (!base)
		return FALSE;

	tokens = g_strsplit(base, ".", 0);
	if (tokens)
		len = g_strv_length(tokens);

	if (!len)
		goto exit;

	if (!g_ascii_strncasecmp(tokens[len - 1], "m3u", 3))
		ret = TRUE;

exit:
	g_free(base);
	if (tokens)
		g_strfreev(tokens);

	return ret;
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

        gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);

        menu_xpos += widget->allocation.x;
        menu_ypos += widget->allocation.y;

	if (menu_ypos > gdk_screen_get_height (gtk_widget_get_screen (widget)) / 2)
		menu_ypos -= requisition.height + widget->style->ythickness;
	else
		menu_ypos += widget->allocation.height + widget->style->ythickness;

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
