/*************************************************************************/
/* Copyright (C) 2007-2009 sujtih <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

/* Generate and add the recently-used data */

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
			file_count += dir_file_count(ab_file, 0);
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
