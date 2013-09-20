/*
 * Copyright (C) 2007 Jan Arne Petersen <jap@gnome.org>
 * Copyright (C) 2012 Pavel Vasin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

//based on code from Totem movie player

#include "pragha-playback.h"
#include "gnome-media-keys.h"
#include "pragha.h"

#define PLAYER_NAME "Pragha"

struct _con_gnome_media_keys {
    struct con_win *cwin;
    guint watch_id;
    guint handler_id;
    GDBusProxy *proxy;
};

static void on_media_player_key_pressed(con_gnome_media_keys *gmk,
                                        const gchar *key)
{
	PraghaBackend *backend;
    struct con_win *cwin = gmk->cwin;

	backend = pragha_application_get_backend (cwin);

    if (pragha_backend_emitted_error (backend))
        return;

    if (g_strcmp0("Play", key) == 0)
        pragha_playback_play_pause_resume(cwin);
    else if (g_strcmp0("Pause", key) == 0)
        pragha_backend_pause (backend);
    else if (g_strcmp0("Stop", key) == 0)
        pragha_playback_stop(cwin);
    else if (g_strcmp0("Previous", key) == 0)
        pragha_playback_prev_track(cwin);
    else if (g_strcmp0("Next", key) == 0)
        pragha_playback_next_track(cwin);
    else if (g_strcmp0("Repeat", key) == 0)
    {
        gboolean repeat = pragha_preferences_get_repeat(cwin->preferences);
        pragha_preferences_set_repeat(cwin->preferences, !repeat);
    }
    else if (g_strcmp0("Shuffle", key) == 0)
    {
        gboolean shuffle = pragha_preferences_get_shuffle(cwin->preferences);
        pragha_preferences_set_shuffle(cwin->preferences, !shuffle);
    }

    //XXX missed buttons: "Rewind" and "FastForward"
}

static void grab_media_player_keys_cb(GDBusProxy *proxy,
                                      GAsyncResult *res,
                                      con_gnome_media_keys *gmk)
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

static void grab_media_player_keys(con_gnome_media_keys *gmk)
{
    if (gmk->proxy == NULL)
        return;

    g_dbus_proxy_call(gmk->proxy,
                      "GrabMediaPlayerKeys",
                      g_variant_new("(su)", PLAYER_NAME, 0),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1, NULL,
                      (GAsyncReadyCallback) grab_media_player_keys_cb,
                      gmk);
}

static gboolean on_window_focus_in_event(GtkWidget *window,
                                         GdkEventFocus *event,
                                         con_gnome_media_keys *gmk)
{
    grab_media_player_keys(gmk);

    return FALSE;
}

static void key_pressed(GDBusProxy *proxy,
                        gchar *sender_name,
                        gchar *signal_name,
                        GVariant *parameters,
                        con_gnome_media_keys *gmk)
{
    char *app, *cmd;

    if (g_strcmp0(signal_name, "MediaPlayerKeyPressed") != 0)
        return;

    g_variant_get(parameters, "(ss)", &app, &cmd);

    if (g_strcmp0(app, PLAYER_NAME) == 0)
        on_media_player_key_pressed(gmk, cmd);

    g_free(app);
    g_free(cmd);
}

static void got_proxy_cb(GObject *source_object,
                         GAsyncResult *res,
                         con_gnome_media_keys *gmk)
{
    GError *error = NULL;

    gmk->proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

    if (gmk->proxy == NULL)
    {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            g_warning("Failed to contact settings daemon: %s", error->message);
        g_error_free(error);
        return;
    }

    grab_media_player_keys(gmk);

    g_signal_connect(G_OBJECT(gmk->proxy), "g-signal", G_CALLBACK(key_pressed), gmk);
}

static void name_appeared_cb(GDBusConnection *connection,
                             const gchar *name,
                             const gchar *name_owner,
                             con_gnome_media_keys *gmk)
{
    g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                             G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                             G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                             NULL,
                             "org.gnome.SettingsDaemon",
                             "/org/gnome/SettingsDaemon/MediaKeys",
                             "org.gnome.SettingsDaemon.MediaKeys",
                             NULL,
                             (GAsyncReadyCallback) got_proxy_cb,
                             gmk);
}

static void name_vanished_cb(GDBusConnection *connection,
                             const gchar *name,
                             con_gnome_media_keys *gmk)
{
    if (gmk->proxy != NULL)
    {
        g_object_unref(gmk->proxy);
        gmk->proxy = NULL;
    }
}

gboolean gnome_media_keys_will_be_useful()
{
    GDBusConnection *bus = NULL;
    GVariant *response = NULL;
    GDBusNodeInfo *node_info = NULL;
    GDBusInterfaceInfo *interface_info;
    GDBusMethodInfo *method_info;
    const gchar *xml_data;
    gboolean result = TRUE;

    bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

    if (!bus)
    {
        result = FALSE;
        goto out;
    }

    response = g_dbus_connection_call_sync(bus,
                                           "org.gnome.SettingsDaemon",
                                           "/org/gnome/SettingsDaemon/MediaKeys",
                                           "org.freedesktop.DBus.Introspectable",
                                           "Introspect",
                                           NULL,
                                           G_VARIANT_TYPE ("(s)"),
                                           G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                           -1,
                                           NULL,
                                           NULL);

    if (!response)
    {
        result = FALSE;
        goto out;
    }

    g_variant_get(response, "(&s)", &xml_data);

    node_info = g_dbus_node_info_new_for_xml(xml_data, NULL);

    if (!node_info)
    {
        result = FALSE;
        goto out;
    }

    interface_info = g_dbus_node_info_lookup_interface(node_info, "org.gnome.SettingsDaemon.MediaKeys");

    if (!interface_info)
    {
        result = FALSE;
        goto out;
    }

    method_info = g_dbus_interface_info_lookup_method(interface_info, "GrabMediaPlayerKeys");

    if (!method_info)
    {
        result = FALSE;
        goto out;
    }

out:
    if (bus)
        g_object_unref(bus);
    if (response)
        g_variant_unref(response);
    if (node_info)
        g_dbus_node_info_unref(node_info);

    return result;
}

gint init_gnome_media_keys(struct con_win *cwin)
{
    con_gnome_media_keys *gmk = g_slice_new0(con_gnome_media_keys);

    gmk->cwin = cwin;

    gmk->watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                     "org.gnome.SettingsDaemon",
                                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                                     (GBusNameAppearedCallback) name_appeared_cb,
                                     (GBusNameVanishedCallback) name_vanished_cb,
                                     gmk,
                                     NULL);

    gmk->handler_id = g_signal_connect(G_OBJECT(pragha_window_get_mainwindow(cwin)), "focus-in-event",
                                       G_CALLBACK(on_window_focus_in_event), gmk);

    cwin->cgnome_media_keys = gmk;

    return 0;
}

void gnome_media_keys_free(con_gnome_media_keys *gmk)
{
    struct con_win *cwin = gmk->cwin;

    g_bus_unwatch_name(gmk->watch_id);

    if (gmk->handler_id != 0)
        g_signal_handler_disconnect(G_OBJECT(pragha_window_get_mainwindow(cwin)), gmk->handler_id);

    if (gmk->proxy != NULL)
        g_object_unref(gmk->proxy);

    g_slice_free(con_gnome_media_keys, gmk);
}
