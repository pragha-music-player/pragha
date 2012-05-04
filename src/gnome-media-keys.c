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

#include "pragha.h"

#define PLAYER_NAME "Pragha"

static void on_media_player_key_pressed(struct con_win *cwin,
                                        const gchar *key)
{
    if (cwin->cgst->emitted_error)
        return;

    if (strcmp("Play", key) == 0)
        play_pause_resume(cwin);
    else if (strcmp("Pause", key) == 0)
        backend_pause(cwin);
    else if (strcmp("Stop", key) == 0)
        backend_stop(NULL, cwin);
    else if (strcmp("Previous", key) == 0)
        play_prev_track(cwin);
    else if (strcmp("Next", key) == 0)
        play_next_track(cwin);
    else if (strcmp("Repeat", key) == 0)
        repeat_button_handler(GTK_TOGGLE_BUTTON(cwin->repeat_button), cwin);
    else if (strcmp("Shuffle", key) == 0)
        shuffle_button_handler(GTK_TOGGLE_BUTTON(cwin->shuffle_button), cwin);

    //XXX missed buttons: "Rewind" and "FastForward"
}

static void grab_media_player_keys_cb(GDBusProxy *proxy,
                                      GAsyncResult *res,
                                      struct con_win *cwin)
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

static void grab_media_player_keys(struct con_win *cwin)
{
    struct con_gnome_media_keys *gmk = cwin->cgnome_media_keys;

    if (gmk->proxy == NULL)
        return;

    g_dbus_proxy_call(gmk->proxy,
                      "GrabMediaPlayerKeys",
                      g_variant_new("(su)", PLAYER_NAME, 0),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1, NULL,
                      (GAsyncReadyCallback) grab_media_player_keys_cb,
                      cwin);
}

static gboolean on_window_focus_in_event(GtkWidget *window,
                                         GdkEventFocus *event,
                                         struct con_win *cwin)
{
    grab_media_player_keys(cwin);

    return FALSE;
}

static void key_pressed(GDBusProxy *proxy,
                        gchar *sender_name,
                        gchar *signal_name,
                        GVariant *parameters,
                        struct con_win *cwin)
{
    char *app, *cmd;

    if (g_strcmp0(signal_name, "MediaPlayerKeyPressed") != 0)
        return;

    g_variant_get(parameters, "(ss)", &app, &cmd);

    if (g_strcmp0(app, PLAYER_NAME) == 0)
        on_media_player_key_pressed(cwin, cmd);

    g_free(app);
    g_free(cmd);
}

static void got_proxy_cb(GObject *source_object,
                         GAsyncResult *res,
                         struct con_win *cwin)
{
    struct con_gnome_media_keys *gmk = cwin->cgnome_media_keys;
    GError *error = NULL;

    gmk->proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

    if (gmk->proxy == NULL)
    {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            g_warning("Failed to contact settings daemon: %s", error->message);
        g_error_free(error);
        return;
    }

    grab_media_player_keys(cwin);

    g_signal_connect(G_OBJECT(gmk->proxy), "g-signal", G_CALLBACK(key_pressed), cwin);
}

static void name_appeared_cb(GDBusConnection *connection,
                             const gchar *name,
                             const gchar *name_owner,
                             struct con_win *cwin)
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
                             cwin);
}

static void name_vanished_cb(GDBusConnection *connection,
                             const gchar *name,
                             struct con_win *cwin)
{
    struct con_gnome_media_keys *gmk = cwin->cgnome_media_keys;

    if (gmk->proxy != NULL)
    {
        g_object_unref(gmk->proxy);
        gmk->proxy = NULL;
    }
}

gint init_gnome_media_keys(struct con_win *cwin)
{
    cwin->cgnome_media_keys = g_slice_new0(struct con_gnome_media_keys);
    struct con_gnome_media_keys *gmk = cwin->cgnome_media_keys;

    gmk->watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                     "org.gnome.SettingsDaemon",
                                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                                     (GBusNameAppearedCallback) name_appeared_cb,
                                     (GBusNameVanishedCallback) name_vanished_cb,
                                     cwin,
                                     NULL);

    gmk->handler_id = g_signal_connect(G_OBJECT(cwin->mainwindow), "focus-in-event",
                                       G_CALLBACK(on_window_focus_in_event), cwin);

    return 0;
}

void cleanup_gnome_media_keys(struct con_win *cwin)
{
    struct con_gnome_media_keys *gmk = cwin->cgnome_media_keys;

    g_bus_unwatch_name(gmk->watch_id);

    if (gmk->handler_id != 0)
        g_signal_handler_disconnect(G_OBJECT(cwin->mainwindow), gmk->handler_id);

    if (gmk->proxy != NULL)
        g_object_unref(gmk->proxy);

    g_slice_free(struct con_gnome_media_keys, gmk);
}
