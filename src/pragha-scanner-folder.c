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

/* Thread that counts all files in the library */

static gpointer
pragha_scanner_count_no_files_thread (gpointer data)
{
	gint no_files = 0;
	PraghaScannerWorker *worker = data;

	if (pragha_scanner_worker_is_cancelled (worker))
		return NULL;

	no_files = pragha_get_dir_count (pragha_scanner_worker_get_container(worker),
	                                 pragha_scanner_worker_get_cancellable(worker));

	pragha_scanner_worker_set_no_files_found (worker, no_files);

	return NULL;
}

/* Thread that analyze all files in the library */

static void
pragha_scanner_scan_handler (PraghaScannerWorker *worker, const gchar *dir_name)
{
	PraghaMusicobject *mobj = NULL;
	PraghaMediaType file_type;
	const gchar *next_file = NULL;
	GDir *dir;
	gchar *ab_file = NULL;
	GError *error = NULL;

	if (pragha_scanner_worker_is_cancelled (worker))
		return;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if (pragha_scanner_worker_is_cancelled (worker)) {
			g_dir_close(dir);
			return;
		}

		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			pragha_scanner_scan_handler (worker, ab_file);
		else {
			file_type = pragha_file_get_media_type (ab_file);
			switch (file_type) {
				case MEDIA_TYPE_AUDIO:
					mobj = new_musicobject_from_file(ab_file);
					if (G_LIKELY(mobj))
						 pragha_scanner_worker_append_track (worker, mobj);
					break;
				case MEDIA_TYPE_PLAYLIST:
					//pragha_scanner_worker_append_playlist (worker, ab_file);
					break;
				case MEDIA_TYPE_IMAGE:
				case MEDIA_TYPE_UNKNOWN:
				default:
					break;
			}
			pragha_scanner_worker_plus_no_files_scanned (worker);
		}

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

static gpointer
pragha_scanner_scan_thread (gpointer data)
{
	PraghaScannerWorker *worker = data;

	/* Scan reucurcibly */
	pragha_scanner_scan_handler (worker, pragha_scanner_worker_get_container(worker));

	return worker;
}

void
pragha_scanner_scan_library (PraghaScanner *scanner)
{
	PraghaScannerWorker *worker;
	PraghaPreferences *preferences;
	GSList *folder_list, *list;

	preferences = pragha_preferences_get ();

	folder_list =
		pragha_preferences_get_filename_list (preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR);

	for (list = folder_list; list != NULL; list = list->next) {
		worker = pragha_scanner_worker_new (list->data);

		pragha_scanner_worker_configure_threads (worker,
		                                         pragha_scanner_scan_thread,
		                                         pragha_scanner_count_no_files_thread,
		                                         worker);

		pragha_scanner_add_worker (scanner, worker);
	}

	g_object_unref (G_OBJECT(preferences));
}

