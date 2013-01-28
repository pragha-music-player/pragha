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

typedef struct {
	PraghaDatabase *cdbase;
	GSList         *folder_list;
	GtkWidget      *dialog;
	GtkWidget      *progress_bar;
	guint           no_files;
	PRAGHA_MUTEX   (no_files_mutex);
	GThread        *no_files_thread;
	guint           files_scanned;
	PRAGHA_MUTEX   (files_scanned_mutex);
	GCancellable   *cancellable;
	guint           update_timeout;
} PraghaScanner;

/* Update the dialog. */

gboolean
update_scanned_progress(gpointer user_data);

gpointer
pragha_count_no_files_worker(gpointer data);

void
pragha_rescan_handler(PraghaScanner *scanner, const gchar *dir_name);

gpointer
pragha_scanner_worker(gpointer data);

gboolean
rescan_dialog_delete_event(GtkWidget *widget,
                           GdkEvent *event,
                           gpointer user_data);

void
rescan_dialog_response_event(GtkDialog *dialog,
                             gint response_id,
                             gpointer user_data);

void
pragha_rescan_library (GSList *folder_list, struct con_win *cwin);

