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

#ifndef PRAGHA_SIMPLE_ASYNC_H
#define PRAGHA_SIMPLE_ASYNC_H

#include <glib.h>

typedef struct {
	gpointer userdata;
	gpointer finished_data;
	GThreadFunc func_w;
	GSourceFunc func_f;
} AsyncSimple;

#if GLIB_CHECK_VERSION (2, 32, 0)
#define PRAGHA_MUTEX(mtx) GMutex mtx
#define pragha_mutex_free(mtx) g_mutex_clear (&(mtx))
#define pragha_mutex_lock(mtx) g_mutex_lock (&(mtx))
#define pragha_mutex_unlock(mtx) g_mutex_unlock (&(mtx))
#define pragha_mutex_create(mtx) g_mutex_init (&(mtx))
#else
#define PRAGHA_MUTEX(mtx) GMutex *mtx
#define pragha_mutex_free(mtx) g_mutex_free (mtx)
#define pragha_mutex_lock(mtx) g_mutex_lock (mtx)
#define pragha_mutex_unlock(mtx) g_mutex_unlock (mtx)
#define pragha_mutex_create(mtx) (mtx) = g_mutex_new ()
#endif

gboolean pragha_async_set_idle_message (gpointer user_data);

void     pragha_async_launch           (GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata);

GThread *pragha_async_launch_full      (GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata);

#endif /* PRAGHA_SIMPLE_ASYNC_H */