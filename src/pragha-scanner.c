/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>			 */
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

/* Update the dialog. */

gboolean
pragha_scanner_update_progress(gpointer user_data)
{
	gdouble fraction = 0.0;
	gint files_scanned = 0;
	gint no_files;
	gchar *data = NULL;

	PraghaScanner *scanner = user_data;

	if(g_cancellable_is_cancelled (scanner->cancellable))
		return FALSE;

	pragha_mutex_lock (scanner->no_files_mutex);
	no_files = scanner->no_files;
	pragha_mutex_unlock (scanner->no_files_mutex);

	pragha_mutex_lock (scanner->files_scanned_mutex);
	files_scanned = scanner->files_scanned;
	pragha_mutex_unlock (scanner->files_scanned_mutex);

	if(no_files > 0) {
		fraction = (gdouble)files_scanned / (gdouble)no_files;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(scanner->progress_bar), fraction);

		data = g_strdup_printf(_("%i files analized of %i detected"), files_scanned, no_files);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(scanner->progress_bar), data);
		g_free(data);
	}
	else {
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(scanner->progress_bar));
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(scanner->progress_bar), _("Searching files to analyze"));
	}

	return TRUE;
}

/* Thread that counts all files in the library */

gpointer
pragha_scanner_count_no_files_worker(gpointer data)
{
	GSList *list;
	gint no_files = 0;

	PraghaScanner *scanner = data;

	for(list = scanner->folder_list ; list != NULL ; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		no_files += pragha_get_dir_count(list->data, scanner->cancellable);

		pragha_mutex_lock (scanner->no_files_mutex);
		scanner->no_files = no_files;
		pragha_mutex_unlock (scanner->no_files_mutex);
	}

	g_thread_exit(NULL);

	return NULL;
}

/* Function that is executed at the end of analyze the files,
 * or if the analysis was canceled.
 * This runs on the main thread. So, can show a dialog.
 * Finally, frees all memory. */

static void
pragha_scanner_add_track_db(gpointer key,
                            gpointer value,
                            gpointer user_data)
{
	PraghaMusicobject *mobj = value;
	PraghaDatabase *database = user_data;

	add_new_musicobject_db(database, mobj);

	pragha_process_gtk_events ();
}

static void
pragha_scanner_finished_dialog_response_cb (GtkDialog *dialog, gint response, gpointer data)
{
	PraghaScanner *scanner = data;

	if(!scanner->update_timeout)
		gtk_widget_destroy(GTK_WIDGET(dialog));

}

static gboolean
pragha_scanner_finished_dialog_delete (GtkDialog *dialog, GdkEvent  *event, gpointer data)
{
	PraghaScanner *scanner = data;

	if(scanner->update_timeout)
		return TRUE;

	return FALSE;
}

gboolean
pragha_scanner_worker_finished (gpointer data)
{
	GtkWidget *msg_dialog;
	gchar *last_scan_time = NULL;
	PraghaPreferences *preferences;
	PraghaDatabase *database;

	PraghaScanner *scanner = data;

	/* Stop updates */

	g_source_remove(scanner->update_timeout);

	/* Ensure that the other thread has finished */
	g_thread_join (scanner->no_files_thread);

	/* If not cancelled, update database and show a dialog */
	if(!g_cancellable_is_cancelled (scanner->cancellable)) {
		/* Hide the scanner and show the dialog */

		gtk_widget_hide(scanner->hbox);
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(scanner->hbox))),
		                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		                                    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));

		g_signal_connect(G_OBJECT(msg_dialog), "response",
		                 G_CALLBACK(pragha_scanner_finished_dialog_response_cb),
		                 scanner);

		g_signal_connect(G_OBJECT(msg_dialog), "delete-event",
		                 G_CALLBACK(pragha_scanner_finished_dialog_delete),
		                 scanner);

		gtk_widget_show_all(msg_dialog);

		/* Save new database and update the library view */

		set_watch_cursor(msg_dialog);
		set_watch_cursor(scanner->hbox);

		database = pragha_database_get();

		db_begin_transaction(database);

		flush_db(database);
		g_hash_table_foreach (scanner->tracks_table,
		                      pragha_scanner_add_track_db,
		                      database);

		db_commit_transaction(database);

		pragha_database_change_playlists_done(database);
		g_object_unref(database);

		remove_watch_cursor(scanner->hbox);
		remove_watch_cursor(msg_dialog);

		/* Save finished time and folders scanned. */

		g_get_current_time(&scanner->last_update);
		last_scan_time = g_time_val_to_iso8601(&scanner->last_update);
		preferences = pragha_preferences_get();
		pragha_preferences_set_string(preferences,
			                     GROUP_LIBRARY,
			                     KEY_LIBRARY_LAST_SCANNED,
			                     last_scan_time);
		g_free(last_scan_time);

		pragha_preferences_set_filename_list(preferences,
			                             GROUP_LIBRARY,
			                             KEY_LIBRARY_SCANNED,
			                             scanner->folder_list);
		g_object_unref(G_OBJECT(preferences));
	}
	else {
		gtk_widget_hide(scanner->hbox);
	}

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(scanner->progress_bar), NULL);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(scanner->progress_bar), 0.0);

	/* Clean memory */

	g_hash_table_remove_all(scanner->tracks_table);
	free_str_list(scanner->folder_list);
	scanner->folder_list = NULL;
	free_str_list(scanner->folder_scanned);
	scanner->folder_scanned = NULL;
	scanner->no_files = 0;
	scanner->files_scanned = 0;

	g_cancellable_reset (scanner->cancellable);
	scanner->update_timeout = 0;

	return FALSE;
}

/* Function that analyzes all files of library recursively */

void
pragha_scanner_scan_handler(PraghaScanner *scanner, const gchar *dir_name)
{
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;
	PraghaMusicobject *mobj = NULL;

	if(g_cancellable_is_cancelled (scanner->cancellable))
		return;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if(g_cancellable_is_cancelled (scanner->cancellable)) {
			g_dir_close(dir);
			return;
		}

		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			pragha_scanner_scan_handler(scanner, ab_file);
		else {
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (G_LIKELY(mobj))
					 g_hash_table_insert(scanner->tracks_table,
					                     g_strdup(pragha_musicobject_get_file(mobj)),
					                     mobj);
			}

			pragha_mutex_lock (scanner->files_scanned_mutex);
			scanner->files_scanned++;
			pragha_mutex_unlock (scanner->files_scanned_mutex);
		}

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

/* Thread that analyze all files in the library */

gpointer
pragha_scanner_scan_worker(gpointer data)
{
	GSList *list;

	PraghaScanner *scanner = data;

	for(list = scanner->folder_list ; list != NULL; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		pragha_scanner_scan_handler(scanner, list->data);
	}

	return scanner;
}

/* Function that analyzes all files of library recursively */

void
pragha_scanner_update_handler(PraghaScanner *scanner, const gchar *dir_name)
{
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL, *s_ab_file = NULL;
	GError *error = NULL;
	struct stat sbuf;
	PraghaMusicobject *mobj = NULL;

	if(g_cancellable_is_cancelled (scanner->cancellable))
		return;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			return;

		ab_file = g_strconcat(dir_name, "/", next_file, NULL);

		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR)) {
			pragha_scanner_update_handler(scanner, ab_file);
		}
		else {
			mobj = g_hash_table_lookup(scanner->tracks_table,
			                           ab_file);
			if(!mobj) {
				mobj = new_musicobject_from_file(ab_file);
				if (G_LIKELY(mobj))
					 g_hash_table_insert(scanner->tracks_table,
					                     g_strdup(pragha_musicobject_get_file(mobj)),
					                     mobj);

			}
			else {
				g_stat(ab_file, &sbuf);
				if(sbuf.st_mtime > scanner->last_update.tv_sec) {
					mobj = new_musicobject_from_file(ab_file);
					if (G_LIKELY(mobj)) {
						g_hash_table_replace(scanner->tracks_table,
						                     g_strdup(pragha_musicobject_get_file(mobj)),
						                     mobj);
					}
				}
			}

			pragha_mutex_lock (scanner->files_scanned_mutex);
			scanner->files_scanned++;
			pragha_mutex_unlock (scanner->files_scanned_mutex);

			g_free(s_ab_file);
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

/* Thread that analyze all files in the library */

gpointer
pragha_scanner_update_worker(gpointer data)
{
	GSList *list;
	GHashTableIter iter;
	gpointer key, value;
	gchar *file;

	PraghaScanner *scanner = data;

	/* Clean removed files */

	g_hash_table_iter_init (&iter, scanner->tracks_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		file = key;
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;
		if(!g_file_test(file, G_FILE_TEST_EXISTS))
			g_hash_table_iter_remove(&iter);
	}

	/* Then update files changed.. */
	for(list = scanner->folder_list ; list != NULL; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		if(is_present_str_list(list->data, scanner->folder_scanned))
			pragha_scanner_update_handler(scanner, list->data);
	}

	/* Then add news folder in library */
	for(list = scanner->folder_list; list != NULL; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		if(!is_present_str_list(list->data, scanner->folder_scanned))
			pragha_scanner_scan_handler(scanner, list->data);
	}

	return scanner;
}

void
pragha_scanner_update_library(PraghaScanner *scanner)
{
	PraghaPreferences *preferences;
	PraghaDatabase *database;
	gchar *last_scan_time = NULL;
	PraghaMusicobject *mobj = NULL;
	guint i = 0, location_id;
	GSList *list;
	gchar *query;
	PraghaDbResponse result;

	if(scanner->update_timeout)
		return;

	preferences = pragha_preferences_get();

	/* Get last time that update the library and folders to analyze */

	last_scan_time = pragha_preferences_get_string(preferences,
	                                               GROUP_LIBRARY,
	                                               KEY_LIBRARY_LAST_SCANNED);
	if (last_scan_time) {
		if (!g_time_val_from_iso8601(last_scan_time, &scanner->last_update))
			g_warning("Unable to convert last rescan time");
		g_free(last_scan_time);
	}

	scanner->folder_list =
		pragha_preferences_get_filename_list(preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_DIR);
	scanner->folder_scanned =
		pragha_preferences_get_filename_list(preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_SCANNED);
	g_object_unref(G_OBJECT(preferences));

	/* Update the gui */

	scanner->update_timeout = g_timeout_add_seconds(1, (GSourceFunc)pragha_scanner_update_progress, scanner);

	gtk_widget_show_all(scanner->hbox);

	/* Append the files from database that no changed. */

	database = pragha_database_get();
	for(list = scanner->folder_scanned ; list != NULL; list = list->next) {
		if(is_present_str_list(list->data, scanner->folder_list)) {
			query = g_strdup_printf("SELECT id FROM LOCATION WHERE name LIKE '%s%%';", (gchar *)list->data);
			if (pragha_database_exec_sqlite_query(database, query, &result)) {
				for_each_result_row(result, i) {
					location_id = atoi(result.resultp[i]);
					mobj = new_musicobject_from_db(database, location_id);
					if (G_LIKELY(mobj)) {
						 g_hash_table_insert(scanner->tracks_table,
						                     g_strdup(pragha_musicobject_get_file(mobj)),
						                     mobj);
					}

					pragha_process_gtk_events ();
				}
				sqlite3_free_table(result.resultp);
			}
		}
	}
	g_object_unref(database);

	/* Launch threads */

	#if GLIB_CHECK_VERSION(2,31,0)
	scanner->no_files_thread = g_thread_new("Count no files", pragha_scanner_count_no_files_worker, scanner);
	#else
	scanner->no_files_thread = g_thread_create(pragha_scanner_count_no_files_worker, scanner, FALSE, NULL);
	#endif

	pragha_async_launch(pragha_scanner_update_worker,
			    pragha_scanner_worker_finished,
			    scanner);
}

void
pragha_scanner_scan_library(PraghaScanner *scanner)
{
	PraghaPreferences *preferences;
	gchar *last_scan_time = NULL;

	if(scanner->update_timeout)
		return;

	preferences = pragha_preferences_get();

	/* Get last time that update the library and folders to analyze */

	last_scan_time = pragha_preferences_get_string(preferences,
	                                               GROUP_LIBRARY,
	                                               KEY_LIBRARY_LAST_SCANNED);
	if (last_scan_time) {
		if (!g_time_val_from_iso8601(last_scan_time, &scanner->last_update))
			g_warning("Unable to convert last rescan time");
		g_free(last_scan_time);
	}

	scanner->folder_list =
		pragha_preferences_get_filename_list(preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_DIR);
	scanner->folder_scanned =
		pragha_preferences_get_filename_list(preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_SCANNED);
	g_object_unref(G_OBJECT(preferences));

	/* Update the gui */

	scanner->update_timeout = g_timeout_add_seconds(1, (GSourceFunc)pragha_scanner_update_progress, scanner);

	gtk_widget_show_all(scanner->hbox);

	/* Launch threads */

	#if GLIB_CHECK_VERSION(2,31,0)
	scanner->no_files_thread = g_thread_new("Count no files", pragha_scanner_count_no_files_worker, scanner);
	#else
	scanner->no_files_thread = g_thread_create(pragha_scanner_count_no_files_worker, scanner, FALSE, NULL);
	#endif

	pragha_async_launch(pragha_scanner_scan_worker,
			    pragha_scanner_worker_finished,
			    scanner);
}

void
pragha_scanner_free(PraghaScanner *scanner)
{
	if(scanner->update_timeout) {
		g_cancellable_cancel (scanner->cancellable);
		g_thread_join (scanner->no_files_thread);
	}

	g_hash_table_destroy(scanner->tracks_table);
	free_str_list(scanner->folder_list);
	free_str_list(scanner->folder_scanned);
	pragha_mutex_free(scanner->no_files_mutex);
	pragha_mutex_free(scanner->files_scanned_mutex);
	g_object_unref(scanner->cancellable);

	g_slice_free (PraghaScanner, scanner);
}

void
scanner_cancel_click_handler(GtkButton *button,
                             PraghaScanner *scanner)
{
	g_cancellable_cancel (scanner->cancellable);
}

PraghaScanner *
pragha_scanner_new()
{
	PraghaScanner *scanner;
	PraghaStatusbar *statusbar;
	GtkWidget *hbox, *progress_bar, *button, *image;

	scanner = g_slice_new0(PraghaScanner);

	/* Create widgets */
	hbox = gtk_hbox_new(FALSE, 0);

	progress_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress_bar, PROGRESS_BAR_WIDTH, -1);
	#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR(progress_bar), TRUE);
	#endif

	button = gtk_button_new ();
	image = gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_container_add (GTK_CONTAINER (button), image);

	g_signal_connect(G_OBJECT (button),
			 "clicked",
			 G_CALLBACK(scanner_cancel_click_handler),
			 scanner);

	gtk_box_pack_start (GTK_BOX (hbox), progress_bar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	/* Init the rest and save references */

	scanner->progress_bar = progress_bar;
	scanner->hbox = hbox;
	scanner->tracks_table = g_hash_table_new_full (g_str_hash,
	                                               g_str_equal,
	                                               g_free,
	                                               g_object_unref);
	scanner->files_scanned = 0;
	pragha_mutex_create(scanner->files_scanned_mutex);
	scanner->no_files = 0;
	pragha_mutex_create(scanner->no_files_mutex);
	scanner->cancellable = g_cancellable_new ();
	scanner->update_timeout = 0;

	/* Append the widget */

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_add_widget(statusbar, hbox);
	g_object_unref(G_OBJECT(statusbar));

	return scanner;
}