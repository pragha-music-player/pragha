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

#include "pragha-scanner.h"

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
		gtk_label_set_text(GTK_LABEL(scanner->label), data);
		g_free(data);
	}
	else {
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(scanner->progress_bar));
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

gboolean
pragha_scanner_worker_finished (gpointer data)
{
	GtkWidget *msg_dialog;
	gchar *last_scan_time = NULL;

	PraghaScanner *scanner = data;

	/* Stop updates */
	g_source_remove(scanner->update_timeout);

	/* Destroy the dialog */
	gtk_widget_destroy(scanner->dialog);

	/* Ensure that the other thread has finished */
	g_thread_join (scanner->no_files_thread);

	/* If was not canceled, shows a dialog */
	if (!g_cancellable_is_cancelled (scanner->cancellable)) {
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(scanner->parent),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));
		gtk_dialog_run(GTK_DIALOG(msg_dialog));

		gtk_widget_destroy(msg_dialog);
	}

	/* If not cancelled, remove last scanned folders */
	if(!g_cancellable_is_cancelled (scanner->cancellable)) {
		pragha_preferences_set_filename_list(scanner->preferences,
			                             GROUP_LIBRARY,
			                             KEY_LIBRARY_SCANNED,
			                             scanner->folder_list);
	}

	/* Update the library view */
	pragha_database_change_playlists_done(scanner->cdbase);

	/* Clean memory */
	g_object_unref(scanner->cdbase);
	g_object_unref(scanner->preferences);
	free_str_list(scanner->folder_list);
	free_str_list(scanner->folder_scanned);
	pragha_mutex_free(scanner->no_files_mutex);
	pragha_mutex_free(scanner->files_scanned_mutex);
	g_object_unref(scanner->cancellable);

	/* Save last time update and folders */

	g_get_current_time(&scanner->last_update);
	last_scan_time = g_time_val_to_iso8601(&scanner->last_update);
	pragha_preferences_set_string(scanner->preferences,
	                             GROUP_LIBRARY,
	                             KEY_LIBRARY_LAST_SCANNED,
	                             last_scan_time);
	g_free(last_scan_time);

	g_slice_free (PraghaScanner, data);

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
			pragha_database_add_new_file(scanner->cdbase, ab_file);

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

	db_begin_transaction(scanner->cdbase);

	for(list = scanner->folder_list ; list != NULL; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		pragha_scanner_scan_handler(scanner, list->data);
	}

	db_commit_transaction(scanner->cdbase);

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
			s_ab_file = sanitize_string_to_sqlite3(ab_file);
			if (!find_location_db(s_ab_file, scanner->cdbase)) {
				pragha_database_add_new_file(scanner->cdbase, ab_file);
			}
			else {
				g_stat(ab_file, &sbuf);
				if(sbuf.st_mtime > scanner->last_update.tv_sec) {
					pragha_database_forget_track(scanner->cdbase, ab_file);
					pragha_database_add_new_file(scanner->cdbase, ab_file);
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
	gchar *query;
	PraghaDbResponse result;
	gint i = 0;

	PraghaScanner *scanner = data;

	db_begin_transaction(scanner->cdbase);

	/* First clean removed folders in library. */
	for(list = scanner->folder_scanned ; list != NULL; list = list->next) {
		if(g_cancellable_is_cancelled (scanner->cancellable))
			break;

		if(!is_present_str_list(list->data, scanner->folder_list))
			pragha_database_delete_folder(scanner->cdbase, list->data);
	}

	/* Also clean removed files */
	query = g_strdup_printf("SELECT name FROM LOCATION;");
	if (pragha_database_exec_sqlite_query(scanner->cdbase, query, &result)) {
		for_each_result_row(result, i) {
			if(g_cancellable_is_cancelled (scanner->cancellable))
				break;
			if(!g_file_test(result.resultp[i], G_FILE_TEST_EXISTS))
				pragha_database_forget_track(scanner->cdbase, result.resultp[i]);
		}
		sqlite3_free_table(result.resultp);
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

	db_commit_transaction(scanner->cdbase);

	return scanner;
}


/* Signal handler for deleting the scanner dialog */

gboolean
scanner_dialog_delete_event(GtkWidget *dialog,
                            GdkEvent *event,
                            gpointer user_data)
{
	PraghaScanner *scanner = user_data;

	g_cancellable_cancel (scanner->cancellable);

	return TRUE;
}

/* Signal handler for cancelling the scanner dialog */

void
scanner_dialog_response_event(GtkDialog *dialog,
                              gint response_id,
                              gpointer user_data)
{
	PraghaScanner *scanner = user_data;

	if (response_id == GTK_RESPONSE_CANCEL)
		g_cancellable_cancel (scanner->cancellable);
}

/* Create the dialog and init all */

PraghaScanner *
pragha_scanner_dialog_new(GtkWidget *parent, const gchar *title)
{
	PraghaScanner *scanner;
	GtkWidget *dialog, *table, *label, *progress_bar;
	gchar *last_scan_time = NULL;
	guint row = 0;

	scanner = g_slice_new0(PraghaScanner);

	scanner->cdbase = pragha_database_get();
	scanner->preferences = pragha_preferences_get();
	scanner->parent = parent;

	/* Get last time that update the library and folders to analyze */

	last_scan_time = pragha_preferences_get_string(scanner->preferences,
	                                                GROUP_LIBRARY,
	                                                KEY_LIBRARY_LAST_SCANNED);
	if (last_scan_time) {
		if (!g_time_val_from_iso8601(last_scan_time, &scanner->last_update))
			g_warning("Unable to convert last rescan time");
		g_free(last_scan_time);
	}

	scanner->folder_list =
		pragha_preferences_get_filename_list(scanner->preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_DIR);
	scanner->folder_scanned =
		pragha_preferences_get_filename_list(scanner->preferences,
		                                     GROUP_LIBRARY,
		                                     KEY_LIBRARY_SCANNED);

	/* Create the scanner dialog */

	dialog = gtk_dialog_new_with_buttons(title,
	                                     GTK_WINDOW(parent),
	                                     GTK_DIALOG_MODAL,
	                                     GTK_STOCK_CANCEL,
	                                     GTK_RESPONSE_CANCEL,
	                                     NULL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))),
				  GTK_BUTTONBOX_SPREAD);

	/* Create the layout */

	table = pragha_hig_workarea_table_new();

	label = pragha_hig_workarea_table_add_section_title(table, &row, _("Searching files to analyze"));

	progress_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress_bar,
				    PROGRESS_BAR_WIDTH,
				    -1);

	pragha_hig_workarea_table_add_wide_control(table, &row, progress_bar);
	pragha_hig_workarea_table_finish(table, &row);

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(GTK_WINDOW(dialog)), "delete_event",
			 G_CALLBACK(scanner_dialog_delete_event), scanner);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(scanner_dialog_response_event), scanner);

	/* Add the layout to the dialog and show everything */

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	/* Save references */

	scanner->dialog = dialog;
	scanner->label = label;
	scanner->progress_bar = progress_bar;

	/* Init threads */

	scanner->files_scanned = 0;
	pragha_mutex_create(scanner->files_scanned_mutex);

	scanner->no_files = 0;
	pragha_mutex_create(scanner->no_files_mutex);

	scanner->cancellable = g_cancellable_new ();

	scanner->update_timeout = g_timeout_add_seconds(1, (GSourceFunc)pragha_scanner_update_progress, scanner);

	gtk_widget_show_all(dialog);

	#if GLIB_CHECK_VERSION(2,31,0)
	scanner->no_files_thread = g_thread_new("Count no files", pragha_scanner_count_no_files_worker, scanner);
	#else
	scanner->no_files_thread = g_thread_create(pragha_scanner_count_no_files_worker, scanner, FALSE, NULL);
	#endif

	return scanner;
}

void
pragha_scanner_update_library(GtkWidget *parent)
{
	PraghaScanner *scanner;

	scanner = pragha_scanner_dialog_new(parent, _("Update Library"));

	pragha_async_launch(pragha_scanner_update_worker,
			    pragha_scanner_worker_finished,
			    scanner);
}

void
pragha_scanner_scan_library(GtkWidget *parent)
{
	PraghaScanner *scanner;

	scanner = pragha_scanner_dialog_new(parent, _("Rescan Library"));

	flush_db(scanner->cdbase);

	pragha_async_launch(pragha_scanner_scan_worker,
			    pragha_scanner_worker_finished,
			    scanner);
}
