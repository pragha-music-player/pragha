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

void __non_recur_add(const gchar *dir_name, gboolean init, struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (!g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (mobj)
					append_current_playlist(cwin->cplaylist, mobj);
				CDEBUG(DBG_VERBOSE, "Play file from file_tree: %s",
				       ab_file);
			}

		/* Have to give control to GTK periodically ... */

		if (!init) {
			if (pragha_process_gtk_events ())
				return;
		}

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

void __recur_add(const gchar *dir_name, struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);

		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			__recur_add(ab_file, cwin);
		else {
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (mobj) {
					append_current_playlist(cwin->cplaylist, mobj);
					CDEBUG(DBG_VERBOSE,
					       "Play file from file_tree: %s",
					       ab_file);
				}
			}
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}

	g_dir_close(dir);
}

GList *
append_mobj_list_from_folder(GList *list, gchar *dir_name, struct con_win *cwin)
{
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

		if (pragha_preferences_get_add_recursively(cwin->preferences) &&
		    g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			list = append_mobj_list_from_folder(list, ab_file, cwin);
		else {
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (mobj) {
					list = g_list_append(list, mobj);
					CDEBUG(DBG_VERBOSE,
					       "Play file from file_tree: %s",
					       ab_file);
				}
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
