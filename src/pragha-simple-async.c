/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>                   */
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

#include "pragha-simple-async.h"
#include "pragha-statusbar.h"

#define ASYNC_FINALIZE (gpointer)0x01

struct _AsyncSimple {
	gpointer userdata;
	gpointer finished_data;
	GThreadFunc func_w;
	GSourceFunc func_f;
};


struct _PraghaAsyncManager
{
	GThread     *thread;
	GAsyncQueue *queue;
	gint         counter;
};

/* Generic function to set a message when finished the async operation.
 * You need set 'pragha_async_set_idle_message' as finish_func
 * and then return a 'const gchar *' on worker_func. */

gboolean
pragha_async_set_idle_message (gpointer user_data)
{
	PraghaStatusbar *statusbar;

	const gchar *message = user_data;

	if (message == NULL)
		return FALSE;

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_set_misc_text(statusbar, message);
	g_object_unref(G_OBJECT(statusbar));

	return FALSE;
}

/* Launch a asynchronous operation (worker_func), and when finished use another
 * function (finish_func) in the main loop using the information returned by
 * the asynchronous operation. */

static gboolean
pragha_async_finished(gpointer data)
{
	AsyncSimple *as = data;

	/* TODO: Check if finalize pragha.. */

	as->func_f(as->finished_data);
	g_slice_free(AsyncSimple, as);

	return FALSE;
}

static gpointer
pragha_async_worker(gpointer data)
{
	AsyncSimple *as = data;

	as->finished_data = as->func_w(as->userdata);

	g_idle_add_full(G_PRIORITY_HIGH_IDLE, pragha_async_finished, as, NULL);

	return NULL;
}

void
pragha_async_launch (GThreadFunc worker_func, GSourceFunc finish_func, gpointer user_data)
{
	AsyncSimple *as;

	as = g_slice_new0(AsyncSimple);
	as->func_w = worker_func;
	as->func_f = finish_func;
	as->userdata = user_data;
	as->finished_data = NULL;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_unref(g_thread_new("Launch async", pragha_async_worker, as));
	#else
	g_thread_create(pragha_async_worker, as, FALSE, NULL);
	#endif
}

GThread *
pragha_async_launch_full (GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata)
{
	AsyncSimple *as;

	as = g_slice_new0(AsyncSimple);
	as->func_w = worker_func;
	as->func_f = finish_func;
	as->userdata = userdata;
	as->finished_data = NULL;

	#if GLIB_CHECK_VERSION(2,31,0)
	return g_thread_new("Launch async", pragha_async_worker, as);
	#else
	return g_thread_create(pragha_async_worker, as, TRUE, NULL);
	#endif
}

/*
 * A simple Async manager with queue
 */

gint sort_async_queue_jobs (gconstpointer a, gconstpointer b, gpointer data)
{
    return (a == ASYNC_FINALIZE) ? -1 : (b == ASYNC_FINALIZE) ? 1 : 0;
}

static gpointer
pragha_async_manager_thread (gpointer data)
{
	gpointer thread_data;
	PraghaAsyncManager *asyn_manager = data;

	g_async_queue_ref (asyn_manager->queue);

	while (TRUE) {
		thread_data = g_async_queue_pop (asyn_manager->queue);

		if (thread_data == ASYNC_FINALIZE) {
            break;
		}
		else {
			AsyncSimple *as = thread_data;

			/* Execute slow task. */
			as->finished_data = as->func_w(as->userdata);

			/*  Launch finished task on main thread. */
			g_idle_add_full(G_PRIORITY_HIGH_IDLE, pragha_async_finished, as, NULL);
		}
	}
	g_async_queue_unref (asyn_manager->queue);

	return NULL;
}

void
pragha_async_manager_add_task (PraghaAsyncManager *async_manager, GThreadFunc worker_func, GSourceFunc finish_func, gpointer user_data)
{
	AsyncSimple *as;
	as = g_slice_new0(AsyncSimple);

	as->func_w        = worker_func;
	as->func_f        = finish_func;
	as->userdata      = user_data;
	as->finished_data = NULL;

	g_async_queue_push (async_manager->queue, as);
}

void
pragha_async_manager_free (PraghaAsyncManager *async_manager)
{
	g_async_queue_push (async_manager->queue, ASYNC_FINALIZE);
	g_async_queue_sort (async_manager->queue, sort_async_queue_jobs, NULL);

	g_thread_join       (async_manager->thread);
	g_async_queue_unref (async_manager->queue);

	g_slice_free (PraghaAsyncManager, async_manager);
}

PraghaAsyncManager *
pragha_async_manager_new (void)
{
	PraghaAsyncManager *async_manager;

	async_manager = g_slice_new0(PraghaAsyncManager);

	async_manager->queue  = g_async_queue_new();
	async_manager->thread = g_thread_new ("Asyn Manager", pragha_async_manager_thread, async_manager);

	return async_manager;
}
