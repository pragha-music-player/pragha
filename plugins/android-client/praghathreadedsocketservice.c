/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include "praghathreadedsocketservice.h"

struct _PraghaThreadedSocketServicePrivate
{
	GThread      *socket_thread;
	GCancellable *canceller;
};

static guint pragha_threaded_socket_service_run_signal;

typedef struct _PraghaThreadedSocketService PraghaThreadedSocketService;

G_DEFINE_TYPE_WITH_PRIVATE (PraghaThreadedSocketService,
                            pragha_threaded_socket_service,
                            G_TYPE_SOCKET_SERVICE)

typedef struct
{
	PraghaThreadedSocketService *threaded;
	GSocketConnection           *connection;
	GCancellable                *canceller;
} PraghaThreadedSocketServiceData;

static gpointer
pragha_threaded_socket_service_func (gpointer user_data)
{
	PraghaThreadedSocketServiceData *data = user_data;
	PraghaThreadedSocketService *threaded = data->threaded;
	gboolean result;

	g_signal_emit (threaded, pragha_threaded_socket_service_run_signal,
	               0, data->connection, data->canceller, &result);

	g_object_unref (threaded);
	g_object_unref (data->connection);
	g_object_unref (data->canceller);
	g_slice_free (PraghaThreadedSocketServiceData, data);

	threaded->priv->socket_thread = NULL;

	return NULL;
}

static gboolean
pragha_threaded_socket_service_incoming (GSocketService    *service,
                                         GSocketConnection *connection,
                                         GObject           *source_object)
{
	PraghaThreadedSocketService *threaded;
	PraghaThreadedSocketServiceData *data;

	threaded = PRAGHA_THREADED_SOCKET_SERVICE (service);

	if (threaded->priv->socket_thread) {
		g_print ("Alrady have a incoming socket runing\n");
		return TRUE;
	}

	data = g_slice_new (PraghaThreadedSocketServiceData);

	/* Ref the socket service for the thread */
	data->threaded = g_object_ref (threaded);
	data->connection = g_object_ref (connection);
	data->canceller = g_object_ref (threaded->priv->canceller);

	threaded->priv->socket_thread =
		g_thread_new("Incoming Socket", pragha_threaded_socket_service_func, data);

	return TRUE;
}

static void
pragha_threaded_socket_service_init (PraghaThreadedSocketService *service)
{
	service->priv = pragha_threaded_socket_service_get_instance_private (service);
}

static void
pragha_threaded_socket_service_constructed (GObject *object)
{
	PraghaThreadedSocketService *service = PRAGHA_THREADED_SOCKET_SERVICE (object);

	service->priv->canceller = g_cancellable_new ();
	service->priv->socket_thread = NULL;
}

static void
pragha_threaded_socket_service_finalize (GObject *object)
{
	PraghaThreadedSocketService *service = PRAGHA_THREADED_SOCKET_SERVICE (object);

	if (service->priv->socket_thread) {
		g_cancellable_cancel (service->priv->canceller);
		g_thread_join (service->priv->socket_thread);
		service->priv->socket_thread = NULL;
	}
	if (service->priv->canceller) {
		g_object_unref (service->priv->canceller);
		service->priv->canceller = NULL;
	}

	G_OBJECT_CLASS (pragha_threaded_socket_service_parent_class)->finalize (object);
}

static void
pragha_threaded_socket_service_class_init (PraghaThreadedSocketServiceClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	GSocketServiceClass *ss_class = &class->parent_class;

	gobject_class->constructed = pragha_threaded_socket_service_constructed;
	gobject_class->finalize = pragha_threaded_socket_service_finalize;

	ss_class->incoming = pragha_threaded_socket_service_incoming;

	pragha_threaded_socket_service_run_signal =
		g_signal_new ("run", G_TYPE_FROM_CLASS (class), G_SIGNAL_RUN_LAST,
		               G_STRUCT_OFFSET (PraghaThreadedSocketServiceClass, run),
		               g_signal_accumulator_true_handled, NULL,
		               NULL, G_TYPE_BOOLEAN,
		               2, G_TYPE_SOCKET_CONNECTION, G_TYPE_CANCELLABLE);
}

void
pragha_threaded_socket_service_cancel (GSocketService *service)
{
	PraghaThreadedSocketService *threaded = PRAGHA_THREADED_SOCKET_SERVICE (service);

	if (threaded->priv->socket_thread) {
		g_cancellable_cancel (threaded->priv->canceller);
		g_thread_join (threaded->priv->socket_thread);
		threaded->priv->socket_thread = NULL;
	}
}

GSocketService *
pragha_threaded_socket_service_new (void)
{
	return g_object_new (PRAGHA_TYPE_THREADED_SOCKET_SERVICE, NULL);
}
