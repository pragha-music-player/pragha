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

#ifndef PRAGHA_SCANNER_WORKER_H
#define PRAGHA_SCANNER_WORKER_H

#include <glib-object.h>
#include <gio/gio.h>

#include "pragha-musicobject.h"

#define PRAGHA_TYPE_SCANNER_WORKER (pragha_scanner_worker_get_type())
#define PRAGHA_SCANNER_WORKER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SCANNER_WORKER, PraghaScannerWorker))
#define PRAGHA_SCANNER_WORKER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SCANNER_WORKER, PraghaScannerWorker const))
#define PRAGHA_SCANNER_WORKER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_SCANNER_WORKER, PraghaScannerWorkerClass))
#define PRAGHA_IS_SCANNER_WORKER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SCANNER_WORKER))
#define PRAGHA_IS_SCANNER_WORKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_SCANNER_WORKER))
#define PRAGHA_SCANNER_WORKER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_SCANNER_WORKER, PraghaScannerWorkerClass))

typedef struct _PraghaScannerWorker PraghaScannerWorker;
typedef struct _PraghaScannerWorkerClass PraghaScannerWorkerClass;

struct _PraghaScannerWorkerClass
{
	GObjectClass parent_class;
	void (*scan_finalized) (PraghaScannerWorker *worker);
};

/*
 * Api
 */
void
pragha_scanner_worker_save_tracks (PraghaScannerWorker *worker);

void
pragha_scanner_worker_append_track (PraghaScannerWorker *worker,
                                    PraghaMusicobject   *mobj);

gint pragha_scanner_worker_get_no_files_found  (PraghaScannerWorker *worker);
void pragha_scanner_worker_set_no_files_found  (PraghaScannerWorker *worker, gint no_files_found);
void pragha_scanner_worker_plus_no_files_found (PraghaScannerWorker *worker);

gint pragha_scanner_worker_get_no_files_scanned  (PraghaScannerWorker *worker);
void pragha_scanner_worker_set_no_files_scanned  (PraghaScannerWorker *worker, gint no_files_scanned);
void pragha_scanner_worker_plus_no_files_scanned (PraghaScannerWorker *worker);

void          pragha_scanner_worker_cancel (PraghaScannerWorker *worker);
gboolean      pragha_scanner_worker_is_cancelled (PraghaScannerWorker *worker);
GCancellable *pragha_scanner_worker_get_cancellable (PraghaScannerWorker *worker);

void pragha_scanner_worker_join (PraghaScannerWorker *worker);

void pragha_scanner_worker_set_container (PraghaScannerWorker *worker, const gchar *container);
const gchar *pragha_scanner_worker_get_container (PraghaScannerWorker *worker);

void
pragha_scanner_worker_configure_threads (PraghaScannerWorker *worker,
                                         GThreadFunc          worker_func,
                                         GThreadFunc          counter_func,
                                         gpointer             userdata);

PraghaScannerWorker * pragha_scanner_worker_new (const gchar *container);


#endif /* PRAGHA_SCANNER_WORKER_H */