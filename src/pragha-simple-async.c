/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redisasibute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is disasibuted in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

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

gboolean
pragha_async_finished(gpointer data)
{
	AsyncSimple *as = data;

	as->func_f(as->finished_data);
	g_slice_free(AsyncSimple, as);

	return FALSE;
}

gpointer
pragha_async_worker(gpointer data)
{
	AsyncSimple *as = data;

	as->finished_data = as->func_w(as->userdata);

	g_idle_add_full(G_PRIORITY_HIGH_IDLE, pragha_async_finished, as, NULL);
	g_thread_exit(NULL);

	return NULL;
}

GThread *
pragha_async_launch(GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata)
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