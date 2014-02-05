/*************************************************************************/
/* Copyright (C) 2013-2014 matias <mati86dl@gmail.com>                   */
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

#include <glib.h>
#include <glib/gstdio.h>

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-scanner.h"
#include "pragha-scanner-worker.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-simple-async.h"
#include "pragha-statusbar.h"

struct _PraghaScanner {
	/* List of scanner workers*/
	GSList            *workers_list;
	guint             *workers_running;
	guint             *workers_cancelled;

	/* Widgets */
	GtkWidget         *hbox;
	GtkWidget         *progress_bar;
	GSList            *playlists;

	GTimeVal          last_update;

	/* Timeout of update progress */
	guint              update_timeout;
};

/* Update scaning progress in status bar */

static gboolean
pragha_scanner_update_progress (gpointer user_data)
{
	PraghaScannerWorker *worker = NULL;
	GSList *workers_list;
	gdouble fraction = 0.0;
	gint files_scanned = 0;
	gint no_files = 0;
	gchar *data = NULL;

	PraghaScanner *scanner = user_data;

	for (workers_list = scanner->workers_list; workers_list != NULL; workers_list = workers_list->next) {
		worker = workers_list->data;

		no_files += pragha_scanner_worker_get_no_files_found (worker);
		files_scanned += pragha_scanner_worker_get_no_files_scanned (worker);
	}

	if (no_files > 0 && files_scanned > 0) {
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

static void
pragha_scanner_finished_dialog_response_cb (GtkDialog *dialog, gint response, gpointer data)
{
	PraghaScanner *scanner = data;

	if (!scanner->update_timeout)
		gtk_widget_destroy(GTK_WIDGET(dialog));
}

static gboolean
pragha_scanner_finished_dialog_delete (GtkDialog *dialog, GdkEvent  *event, gpointer data)
{
	PraghaScanner *scanner = data;

	if (scanner->update_timeout)
		return TRUE;

	return FALSE;
}

void
pragha_scanner_worker_finished_dialog (PraghaScanner *scanner)
{
	GtkWidget *dialog, *parent;

	parent = gtk_widget_get_toplevel(GTK_WIDGET(scanner->hbox));
	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 "%s", _("Library scan complete"));

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(pragha_scanner_finished_dialog_response_cb), scanner);

	g_signal_connect (G_OBJECT(dialog), "delete-event",
	                  G_CALLBACK(pragha_scanner_finished_dialog_delete), scanner);

	gtk_widget_show_all (dialog);

	set_watch_cursor (dialog);
}

static void
pragha_scanner_worker_finished (PraghaScannerWorker *worker, PraghaScanner *scanner)
{
	GSList *list;

	if (pragha_scanner_worker_is_cancelled (worker))
		scanner->workers_cancelled++;

	scanner->workers_running--;

	if (scanner->workers_running > 0)
		return;

	/* Stop and hide progress*/
	g_source_remove (scanner->update_timeout);
	gtk_widget_hide (scanner->hbox);

	if (scanner->workers_cancelled > 0) {
		g_slist_free_full (scanner->workers_list, g_object_unref);
		scanner->workers_list = NULL;
	}
	else {
		pragha_scanner_worker_finished_dialog (scanner);

		for (list = scanner->workers_list; list != NULL; list = list->next) {
			worker = list->data;
			pragha_scanner_worker_save_tracks (worker);
		}
	}

	scanner->workers_running = 0;
	scanner->workers_cancelled = 0;

	scanner->update_timeout = 0;
}

void
pragha_scanner_add_worker (PraghaScanner *scanner, PraghaScannerWorker *worker)
{
	PraghaPreferences *preferences;

	scanner->workers_list =
		g_slist_prepend (scanner->workers_list, worker);

	scanner->workers_running++;
	g_signal_connect (G_OBJECT (worker), "scan-finalized",
	                  G_CALLBACK(pragha_scanner_worker_finished), scanner);

	if (scanner->update_timeout != 0)
		return;

	scanner->update_timeout =
		g_timeout_add_seconds (1, (GSourceFunc)pragha_scanner_update_progress, scanner);

	preferences = pragha_preferences_get ();
	pragha_preferences_set_show_status_bar (preferences, TRUE);
	g_object_unref (G_OBJECT(preferences));

	gtk_widget_show_all (scanner->hbox);
}

static void
pragha_scanner_wait_workers (PraghaScanner *scanner)
{
	PraghaScannerWorker *worker = NULL;
	GSList *list = NULL;

	for (list = scanner->workers_list; list != NULL; list = list->next) {
		worker = list->data;
		pragha_scanner_worker_join (worker);
	}
}

static void
pragha_scanner_cancel_workers (PraghaScanner *scanner)
{
	PraghaScannerWorker *worker = NULL;
	GSList *list = NULL;

	for (list = scanner->workers_list; list != NULL; list = list->next) {
		worker = list->data;
		pragha_scanner_worker_cancel (worker);
	}
}

static void
pragha_scanner_free_workers (PraghaScanner *scanner)
{
	g_slist_free_full (scanner->workers_list, g_object_unref);
}

void
pragha_scanner_free (PraghaScanner *scanner)
{
	if (scanner->update_timeout) {
		pragha_scanner_cancel_workers (scanner);
	}

	pragha_scanner_wait_workers (scanner);
	pragha_scanner_free_workers (scanner);

	g_slice_free (PraghaScanner, scanner);
}

PraghaScanner *
pragha_scanner_new (void)
{
	PraghaScanner *scanner;
	PraghaStatusbar *statusbar;
	GtkWidget *hbox, *progress_bar, *button, *image;

	scanner = g_slice_new0(PraghaScanner);

	/* Create widgets */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	progress_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress_bar, PROGRESS_BAR_WIDTH, -1);

	gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR(progress_bar), TRUE);

	button = gtk_button_new ();
	image = gtk_image_new_from_icon_name ("process-stop", GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_container_add (GTK_CONTAINER (button), image);

	g_signal_connect_swapped (G_OBJECT (button), "clicked",
	                          G_CALLBACK(pragha_scanner_cancel_workers), scanner);

	gtk_box_pack_start (GTK_BOX (hbox), progress_bar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	/* Init the rest and save references */

	scanner->progress_bar = progress_bar;
	scanner->hbox = hbox;

	/* Append the widget */

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_add_widget(statusbar, hbox);
	g_object_unref(G_OBJECT(statusbar));

	return scanner;
}
