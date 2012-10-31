/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
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

/* Generic function to set a message when finished the async operation. */

gboolean
set_async_finished_message (gpointer user_data)
{
	AsycMessageData *data = user_data;

	if (data == NULL)
		return FALSE;

	set_status_message (data->message, data->cwin);

	g_slice_free (AsycMessageData, data);

	return FALSE;
}

AsycMessageData *
async_finished_message_new(const gchar *message, struct con_win *cwin)
{
	AsycMessageData *data;
	data = g_slice_new (AsycMessageData);

	data->message = message;
	data->cwin = cwin;

	return data;
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

	g_idle_add(pragha_async_finished, as);
	g_thread_exit(NULL);

	return NULL;
}

void
pragha_async_launch(GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata)
{
	AsyncSimple *as;

	as = g_slice_new0(AsyncSimple);
	as->func_w = worker_func;
	as->func_f = finish_func;
	as->userdata = userdata;
	as->finished_data = NULL;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Launch async", pragha_async_worker, as);
	#else
	g_thread_create(pragha_async_worker, as, FALSE, NULL);
	#endif
}