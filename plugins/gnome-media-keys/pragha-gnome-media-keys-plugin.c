/*************************************************************************/
/* Copyright (C) 2007 Jan Arne Petersen <jap@gnome.org>                  */
/* Copyright (C) 2012-2013 Pavel Vasin                                   */
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

#include "pragha-gnome-media-keys-plugin.h"

#include "src/pragha.h"
#include "src/pragha-playback.h"

#define PLAYER_NAME "Pragha"

#include "plugins/pragha-plugin-macros.h"

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_GNOME_MEDIA_KEYS_PLUGIN,
                        PraghaGnomeMediaKeysPlugin,
                        pragha_gnome_media_keys_plugin)

static void
on_media_player_key_pressed (PraghaGnomeMediaKeysPlugin *plugin,
                             const gchar                *key)
{
	PraghaBackend *backend;
	PraghaPreferences *preferences;

	PraghaApplication *pragha = plugin->priv->pragha;

	backend = pragha_application_get_backend (pragha);
	preferences = pragha_application_get_preferences (pragha);

	if (pragha_backend_emitted_error (backend))
		return;

	if (g_strcmp0("Play", key) == 0)
		pragha_playback_play_pause_resume(pragha);
	else if (g_strcmp0("Pause", key) == 0)
		pragha_backend_pause (backend);
	else if (g_strcmp0("Stop", key) == 0)
		pragha_playback_stop(pragha);
	else if (g_strcmp0("Previous", key) == 0)
		pragha_playback_prev_track(pragha);
	else if (g_strcmp0("Next", key) == 0)
		pragha_playback_next_track(pragha);
	else if (g_strcmp0("Repeat", key) == 0) {
		gboolean repeat = pragha_preferences_get_repeat (preferences);
		pragha_preferences_set_repeat (preferences, !repeat);
	}
	else if (g_strcmp0("Shuffle", key) == 0) {
		gboolean shuffle = pragha_preferences_get_shuffle (preferences);
		pragha_preferences_set_shuffle (preferences, !shuffle);
	}

	//XXX missed buttons: "Rewind" and "FastForward"
}

static void
grab_media_player_keys_cb (GDBusProxy                 *proxy,
                           GAsyncResult               *res,
                           PraghaGnomeMediaKeysPlugin *plugin)
{
    GVariant *variant;
    GError *error = NULL;

    variant = g_dbus_proxy_call_finish(proxy, res, &error);

    if (variant == NULL)
    {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            g_warning("Failed to call \"GrabMediaPlayerKeys\": %s", error->message);
        g_error_free(error);
        return;
    }

    g_variant_unref(variant);
}

static void
grab_media_player_keys (PraghaGnomeMediaKeysPlugin *plugin)
{
	if (plugin->priv->proxy == NULL)
		return;

	g_dbus_proxy_call (plugin->priv->proxy,
	                   "GrabMediaPlayerKeys",
	                   g_variant_new("(su)", PLAYER_NAME, 0),
	                   G_DBUS_CALL_FLAGS_NONE,
	                   -1, NULL,
	                   (GAsyncReadyCallback) grab_media_player_keys_cb,
	                   plugin);
}

static gboolean
on_window_focus_in_event (GtkWidget                  *window,
                          GdkEventFocus              *event,
                          PraghaGnomeMediaKeysPlugin *plugin)
{
	grab_media_player_keys (plugin);

	return FALSE;
}

static void
key_pressed (GDBusProxy                 *proxy,
             gchar                      *sender_name,
             gchar                      *signal_name,
             GVariant                   *parameters,
             PraghaGnomeMediaKeysPlugin *plugin)
{
	char *app, *cmd;

	if (g_strcmp0(signal_name, "MediaPlayerKeyPressed") != 0)
		return;

	g_variant_get(parameters, "(ss)", &app, &cmd);

	if (g_strcmp0(app, PLAYER_NAME) == 0)
		on_media_player_key_pressed (plugin, cmd);

	g_free(app);
	g_free(cmd);
}

static void
got_proxy_cb (GObject                    *source_object,
              GAsyncResult               *res,
              PraghaGnomeMediaKeysPlugin *plugin)
{
	GError *error = NULL;

	plugin->priv->proxy = g_dbus_proxy_new_for_bus_finish (res, &error);

	if (plugin->priv->proxy == NULL) {
		if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning("Failed to contact settings daemon: %s", error->message);
		g_error_free(error);
		return;
	}

	grab_media_player_keys (plugin);

	g_signal_connect (G_OBJECT(plugin->priv->proxy), "g-signal",
	                  G_CALLBACK(key_pressed), plugin);
}

static void
name_appeared_cb (GDBusConnection            *connection,
                  const gchar                *name,
                  const gchar                *name_owner,
                  PraghaGnomeMediaKeysPlugin *plugin)
{
	g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
	                          G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
	                          G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
	                          NULL,
	                          "org.gnome.SettingsDaemon",
	                          "/org/gnome/SettingsDaemon/MediaKeys",
	                          "org.gnome.SettingsDaemon.MediaKeys",
	                          NULL,
	                          (GAsyncReadyCallback) got_proxy_cb,
	                          plugin);
}

static void
name_vanished_cb (GDBusConnection            *connection,
                  const gchar                *name,
                  PraghaGnomeMediaKeysPlugin *plugin)
{
	if (plugin->priv->proxy != NULL) {
		g_object_unref(plugin->priv->proxy);
		plugin->priv->proxy = NULL;
	}
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GtkWidget *window;
	PraghaGnomeMediaKeysPlugin *plugin = PRAGHA_GNOME_MEDIA_KEYS_PLUGIN (activatable);

	g_debug ("%s", G_STRFUNC);

	PraghaGnomeMediaKeysPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	plugin->priv->watch_id =
		g_bus_watch_name (G_BUS_TYPE_SESSION,
		                  "org.gnome.SettingsDaemon",
		                  G_BUS_NAME_WATCHER_FLAGS_NONE,
		                  (GBusNameAppearedCallback) name_appeared_cb,
		                  (GBusNameVanishedCallback) name_vanished_cb,
		                  plugin,
		                  NULL);

	window = pragha_application_get_window (plugin->priv->pragha);

	plugin->priv->handler_id =
		g_signal_connect (G_OBJECT(window), "focus-in-event",
		                  G_CALLBACK(on_window_focus_in_event), plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	GtkWidget *window;
	PraghaGnomeMediaKeysPlugin *plugin = PRAGHA_GNOME_MEDIA_KEYS_PLUGIN (activatable);

	g_debug ("%s", G_STRFUNC);

	g_bus_unwatch_name (plugin->priv->watch_id);

	window = pragha_application_get_window (plugin->priv->pragha);

	if (plugin->priv->handler_id != 0)
		g_signal_handler_disconnect (G_OBJECT(window), plugin->priv->handler_id);

	if (plugin->priv->proxy != NULL)
		g_object_unref(plugin->priv->proxy);
}
