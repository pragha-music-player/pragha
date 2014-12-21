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
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "pragha-android-plugin.h"
#include "praghathreadedsocketservice.h"

#include "src/pragha.h"
#include "src/pragha-playback.h"
#include "src/pragha-utils.h"
#include "src/pragha-window.h"

#include "plugins/pragha-plugin-macros.h"

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_ANDROID_CLIENT_PLUGIN,
                        PraghaAndroidClientPlugin,
                        pragha_android_client_plugin)

typedef struct
{
	PraghaAndroidClientPlugin *plugin;
	gchar                     *message;
} MessageIdleData;

static void
message_idle_data_free (gpointer user_data)
{
	MessageIdleData *data = user_data;
	g_object_unref (data->plugin);
	g_free (data->message);
	g_free (data);
}

static MessageIdleData *
message_idle_data_new (PraghaAndroidClientPlugin *plugin,
                       const gchar               *message)
{
	MessageIdleData *data;
	data = g_new0 (MessageIdleData, 1);
	data->plugin = g_object_ref (plugin);
	data->message = g_strdup (message);
	return data;
}


static gboolean
android_client_message_on_idle (gpointer user_data)
{
	PraghaBackend *backend;
	MessageIdleData *data = user_data;

	PraghaAndroidClientPlugin *plugin = data->plugin;
	PraghaAndroidClientPluginPrivate *priv = plugin->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_emitted_error (backend))
		return FALSE;

	if (!g_strcmp0 (data->message, "prev")) {
		pragha_playback_prev_track (priv->pragha);
	}
	else if (!g_strcmp0 (data->message, "play")) {
		pragha_playback_play_pause_resume (priv->pragha);
	}
	else if (!g_strcmp0 (data->message, "stop")) {
		pragha_playback_stop (priv->pragha);
	}
	else if (!g_strcmp0 (data->message, "next")) {
		pragha_playback_next_track (priv->pragha);
	}
	else {
		g_print ("Unknown command: %s\n", data->message);
	}

	return FALSE;
}

gboolean
socket_thread_on_run (GThreadedSocketService *service,
                      GSocketConnection      *connection,
                      GCancellable           *cancel,
                      gpointer                user_data)
{
	GInputStream *istream = NULL;
	GError *error = NULL;
	gssize res;
	gchar buffer[2048];

	PraghaAndroidClientPlugin *plugin = user_data;
	PraghaAndroidClientPluginPrivate *priv = plugin->priv;

	istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));

	while (TRUE) {
		if (g_cancellable_is_cancelled(cancel))
			break;

		res = g_input_stream_read (istream,
		                           buffer, sizeof(buffer),
		                           cancel,
		                           &error);
		buffer[res] = '\0';

		if (res == 0) {
			g_usleep (G_USEC_PER_SEC*0.2);
			continue;
		}

		if (error != NULL) {
			g_printerr ("g_input_stream_read failed: %s\n", error->message);
			break;
		}

		GSource *idle_source = g_idle_source_new ();
		g_source_set_priority (idle_source, G_PRIORITY_DEFAULT);
		g_source_set_callback (idle_source,
		                       android_client_message_on_idle,
		                       message_idle_data_new (plugin, buffer),
		                       message_idle_data_free);
		g_source_attach (idle_source, priv->main_context);
		g_source_unref (idle_source);
	}

	return FALSE;
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GError *error = NULL;

	PraghaAndroidClientPlugin *plugin = PRAGHA_ANDROID_CLIENT_PLUGIN (activatable);
	PraghaAndroidClientPluginPrivate *priv = plugin->priv;

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");
	priv->main_context = g_main_context_get_thread_default ();

	CDEBUG(DBG_PLUGIN, "AndroidClient plugin %s", G_STRFUNC);

	priv->service = pragha_threaded_socket_service_new ();

	g_socket_listener_add_inet_port (G_SOCKET_LISTENER(priv->service),
	                                 5500,
	                                 NULL,
	                                 &error);

	if (error != NULL) {
		g_error (error->message);
	}

 	g_signal_connect (priv->service,
 	                  "run",
 	                  G_CALLBACK (socket_thread_on_run),
 	                  plugin);

	g_socket_service_start (G_SOCKET_SERVICE(priv->service));
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaAndroidClientPlugin *plugin = PRAGHA_ANDROID_CLIENT_PLUGIN (activatable);
	PraghaAndroidClientPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "AndroidClient plugin %s", G_STRFUNC);

	g_signal_handlers_disconnect_by_func (priv->service,
	                                      socket_thread_on_run,
	                                      plugin);

	pragha_threaded_socket_service_cancel (priv->service);

	g_socket_service_stop (priv->service);
	g_socket_listener_close (G_SOCKET_LISTENER(priv->service));
	g_object_unref (priv->service);

	priv->pragha = NULL;
}