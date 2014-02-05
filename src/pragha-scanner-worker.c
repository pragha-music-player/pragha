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
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-simple-async.h"

struct _PraghaScannerWorker {
	GObject _parent;

	/* Name to container in database */
	gchar             *container;

	/* Last update time to handle updates */
	GTimeVal          last_update;

	/* Cache */
	GHashTable        *tracks_table;

	/* Counter files thread */
	GThread           *no_files_thread;
	GMutex             files_found_mutex;
	guint              no_files_found;

	/* Scanner worker thread */
	GThread           *worker_thread;
	GMutex             files_scanned_mutex;
	guint              no_files_scanned;

	/* Cancellation safe */
	GCancellable      *cancellable;
};

enum
{
   PROP_0,
   PROP_CONTAINER,
   LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { 0 };

enum {
	SIGNAL_SCAN_FINALIZED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaScannerWorker, pragha_scanner_worker, G_TYPE_OBJECT)

void
pragha_scanner_worker_save_tracks (PraghaScannerWorker *worker)
{
	PraghaDatabase *database;
	PraghaMusicobject *mobj;
	const gchar *container;
	GHashTableIter iter;
	gpointer value, key;

	database = pragha_database_get();
	pragha_database_begin_transaction (database);

	container = pragha_scanner_worker_get_container (worker);

	pragha_database_delete_dir (database, container);

	g_hash_table_iter_init (&iter, worker->tracks_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		mobj = PRAGHA_MUSICOBJECT (value);
		pragha_database_add_new_musicobject (database, container, mobj);

		pragha_process_gtk_events ();
	}

	pragha_database_commit_transaction (database);
	g_object_unref (database);
}

void
pragha_scanner_worker_append_track (PraghaScannerWorker *worker,
                                    PraghaMusicobject   *mobj)
{
	 g_hash_table_insert (worker->tracks_table,
		                  g_strdup (pragha_musicobject_get_file(mobj)),
		                  mobj);
}

/*
 * Number of files found
 */
gint
pragha_scanner_worker_get_no_files_found (PraghaScannerWorker *worker)
{
	gint no_files = 0;

	g_mutex_lock (&worker->files_found_mutex);
	no_files = worker->no_files_found;
	g_mutex_unlock (&worker->files_found_mutex);

	return no_files;
}

void
pragha_scanner_worker_set_no_files_found (PraghaScannerWorker *worker, gint no_files_found)
{
	g_mutex_lock (&worker->files_found_mutex);
	worker->no_files_found = no_files_found;
	g_mutex_unlock (&worker->files_found_mutex);
}

void
pragha_scanner_worker_plus_no_files_found (PraghaScannerWorker *worker)
{
	g_mutex_lock (&worker->files_found_mutex);
	worker->no_files_found++;
	g_mutex_unlock (&worker->files_found_mutex);
}

/*
 * Number of files scanned
 */
gint
pragha_scanner_worker_get_no_files_scanned (PraghaScannerWorker *worker)
{
	gint no_files = 0;

	g_mutex_lock (&worker->files_scanned_mutex);
	no_files = worker->no_files_scanned;
	g_mutex_unlock (&worker->files_scanned_mutex);

	return no_files;
}

void
pragha_scanner_worker_plus_no_files_scanned (PraghaScannerWorker *worker)
{
	g_mutex_lock (&worker->files_scanned_mutex);
	worker->no_files_scanned++;
	g_mutex_unlock (&worker->files_scanned_mutex);
}

void
pragha_scanner_worker_set_no_files_scanned (PraghaScannerWorker *worker, gint no_files_scanned)
{
	g_mutex_lock (&worker->files_scanned_mutex);
	worker->no_files_scanned = no_files_scanned;
	g_mutex_unlock (&worker->files_scanned_mutex);
}

void
pragha_scanner_worker_cancel (PraghaScannerWorker *worker)
{
	g_cancellable_cancel (worker->cancellable);
}

gboolean
pragha_scanner_worker_is_cancelled (PraghaScannerWorker *worker)
{
	return g_cancellable_is_cancelled (worker->cancellable);
}

GCancellable *
pragha_scanner_worker_get_cancellable (PraghaScannerWorker *worker)
{
	return worker->cancellable;
}

void
pragha_scanner_worker_join (PraghaScannerWorker *worker)
{
	g_thread_join (worker->no_files_thread);
	g_thread_join (worker->worker_thread);
}

/**/

gboolean
pragha_scanner_worker_scan_finalize (gpointer userdata)
{
	PraghaScannerWorker *worker = userdata;

	g_signal_emit (worker, signals[SIGNAL_SCAN_FINALIZED], 0);

	return FALSE;
}


static void
pragha_scanner_worker_finalize (GObject *object)
{
	PraghaScannerWorker *worker = PRAGHA_SCANNER_WORKER (object);

	if (worker->no_files_thread ||
	    worker->worker_thread) {
		/* Cancel */
		g_cancellable_cancel (worker->cancellable);

		/* Wait finished*/
		g_thread_join (worker->no_files_thread);
		g_thread_join (worker->worker_thread);
	}

	/* Free the rest */
	g_hash_table_destroy(worker->tracks_table);

	g_mutex_clear (&worker->files_found_mutex);
	g_mutex_clear (&worker->files_scanned_mutex);

	g_object_unref (worker->cancellable);
	g_free (worker->container);

	G_OBJECT_CLASS(pragha_scanner_worker_parent_class)->finalize(object);
}

void
pragha_scanner_worker_set_container (PraghaScannerWorker *worker, const gchar *container)
{
	worker->container = g_strdup (container);
}

const gchar *
pragha_scanner_worker_get_container (PraghaScannerWorker *worker)
{
	return worker->container;
}

static void
pragha_scanner_worker_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	PraghaScannerWorker *worker = PRAGHA_SCANNER_WORKER (object);

	switch (property_id)
	{
		case PROP_CONTAINER:
			pragha_scanner_worker_set_container (worker, g_value_dup_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_scanner_worker_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	PraghaScannerWorker *worker = PRAGHA_SCANNER_WORKER (object);

	switch (property_id)
	{
		case PROP_CONTAINER:
			g_value_set_string (value, pragha_scanner_worker_get_container (worker));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_scanner_worker_class_init (PraghaScannerWorkerClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->set_property = pragha_scanner_worker_set_property;
	gobject_class->get_property = pragha_scanner_worker_get_property;
	gobject_class->finalize = pragha_scanner_worker_finalize;

	/*
	 * Properties:
	 */
	properties[PROP_CONTAINER] =
		g_param_spec_string ("container", "Container", "The Container",
		                     "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class, LAST_PROP, properties);

	/*
	 * Signals:
	 */
	signals[SIGNAL_SCAN_FINALIZED] =
		g_signal_new ("scan-finalized",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaScannerWorkerClass, scan_finalized),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
}

static void
pragha_scanner_worker_init (PraghaScannerWorker *worker)
{
	worker->tracks_table = g_hash_table_new_full (g_str_hash,
	                                              g_str_equal,
	                                              g_free,
	                                              g_object_unref);
	worker->no_files_scanned = 0;
	g_mutex_init (&worker->files_scanned_mutex);

	worker->no_files_found = 0;
	g_mutex_init (&worker->files_found_mutex);

	worker->cancellable = g_cancellable_new ();
}

void
pragha_scanner_worker_configure_threads (PraghaScannerWorker *worker,
                                         GThreadFunc          worker_func,
                                         GThreadFunc          counter_func,
                                         gpointer             userdata)
{
	worker->no_files_thread = g_thread_new ("Count no files", counter_func, userdata);
	worker->worker_thread =
		pragha_async_launch_full (worker_func,
		                          pragha_scanner_worker_scan_finalize,
		                          userdata);
}

PraghaScannerWorker *
pragha_scanner_worker_new (const gchar *container)
{
	PraghaScannerWorker *worker = NULL;

	worker = g_object_new (PRAGHA_TYPE_SCANNER_WORKER,
	                       "container", container,
	                       NULL);

	return worker;
}

