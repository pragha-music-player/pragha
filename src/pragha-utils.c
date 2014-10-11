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

#include <glib.h>
#include <glib/gprintf.h>

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#include "pragha-utils.h"

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
pragha_strstr_lv(gchar *haystack, gchar *needle, PraghaPreferences *preferences)
{
	gboolean aproximate_search;
	aproximate_search = pragha_preferences_get_approximate_search(preferences);

	return g_strstr_lv(haystack, needle,
			   aproximate_search ? 1 : 0);
}

/* Set and remove the watch cursor to suggest background work.*/

void
set_watch_cursor (GtkWidget *widget)
{
	GdkCursor *cursor;
	GtkWidget  *toplevel;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
	if (G_LIKELY (toplevel != NULL)) {
		cursor = gdk_cursor_new (GDK_WATCH);

		gdk_window_set_cursor (gtk_widget_get_window (toplevel), cursor);
		g_object_unref (cursor);
	}
}

void
remove_watch_cursor (GtkWidget *widget)
{
	GtkWidget  *toplevel;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
	if (G_LIKELY (toplevel != NULL))
		gdk_window_set_cursor (gtk_widget_get_window (toplevel), NULL);
}

GdkPixbuf *
pragha_gdk_pixbuf_new_from_memory (gconstpointer data, gsize size)
{
	GError *error = NULL;

	GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
	gdk_pixbuf_loader_write (loader, data, size, &error);
	GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (pixbuf)
		g_object_ref (pixbuf);
	gdk_pixbuf_loader_close (loader, NULL);
	g_object_unref (loader);

	if (error) {
		g_warning ("pragha_gdk_pixbuf_new_from_memory: %s\n", error->message);
		g_error_free (error);
	}

	return pixbuf;
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

	if (!str)
		return FALSE;

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

	if (!str)
		return list;
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

gchar* get_display_name(PraghaMusicobject *mobj)
{
	gchar *name = NULL;
	
	if (!pragha_musicobject_is_local_file(mobj)) {
		name = g_strdup(pragha_musicobject_get_file(mobj));
	} else {
		name = get_display_filename(pragha_musicobject_get_file(mobj), FALSE);
	}
	return name;
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

gchar *
pragha_escape_slashes (const gchar *str)
{
	gchar *dup = g_strdup (str);
	gchar *i = dup;
	while (*i) {
		if (*i == '/' || *i == '\\')
			*i = '|';
		i = g_utf8_next_char (i);
	}
	return dup;
}

gboolean validate_album_art_pattern(const gchar *pattern)
{
	gchar **tokens;
	gint i = 0;
	gboolean ret = FALSE;

	if (string_is_empty(pattern))
		return TRUE;

	if (g_strrstr(pattern, "*")) {
		g_warning("Contains wildcards");
		return FALSE;
	}

	tokens = g_strsplit(pattern, ";", 0);
	while (tokens[i]) i++;

	/* Check if more than six patterns are given */

	if (i <= ALBUM_ART_NO_PATTERNS) {
		ret = TRUE;
	}
	else {
		g_warning("More than six patterns");
	}

	g_strfreev(tokens);

	return ret;
}

void
pragha_process_gtk_events ()
{
#ifdef DEBUG
	extern GThread *pragha_main_thread;
	if (g_thread_self () != pragha_main_thread)
		g_warning ("THREAD SAFETY ERROR!");
#endif
	while (g_main_context_pending (NULL)) {
		g_main_context_iteration (NULL, FALSE);
	}
}

/* callback used to open default browser when URLs got clicked */
void open_url(const gchar *url, GtkWidget *parent)
{
	#ifdef G_OS_WIN32
	if (g_file_test(url, G_FILE_TEST_IS_DIR))
		ShellExecute (0, "explore", url, NULL, NULL, SW_SHOWNORMAL);
	else
		ShellExecute (0, "open", url, NULL, NULL, SW_SHOWNORMAL);
	#else
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
		d = gtk_message_dialog_new (GTK_WINDOW (parent),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
					"%s", _("Unable to open the browser"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (d),
							 "%s", "No methods supported");
		g_signal_connect (d, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_window_present (GTK_WINDOW (d));
	}
	#endif
}

/* It gives the position of the menu on the 
   basis of the position of combo_order */

void
pragha_utils_set_menu_position (GtkMenu  *menu,
                                gint     *x,
                                gint     *y,
                                gboolean *push_in,
                                gpointer  user_data)
{
	GtkWidget *widget;
	GtkAllocation allocation;
	GtkRequisition requisition;
	gint menu_xpos, menu_ypos;

	widget = GTK_WIDGET (user_data);

	gtk_widget_get_preferred_size (GTK_WIDGET(menu), &requisition, NULL);

	gdk_window_get_origin (gtk_widget_get_window(widget), &menu_xpos, &menu_ypos);

	gtk_widget_get_allocation(widget, &allocation);

	menu_xpos += allocation.x;
	menu_ypos += allocation.y;

	if (menu_ypos > gdk_screen_get_height (gtk_widget_get_screen (widget)) / 2)
		menu_ypos -= requisition.height;
	else
		menu_ypos += allocation.height;

	*x = menu_xpos;
	*y = menu_ypos - 5;

	*push_in = TRUE;
}
