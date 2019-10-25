/*************************************************************************/
/* Copyright (C) 2019 matias <mati86dl@gmail.com>                        */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <gio/gio.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "pragha-subsonic-api.h"
#include "pragha-subsonic-preferences.h"

#include "pragha-subsonic-plugin.h"

#include "src/pragha.h"
#include "src/pragha-app-notification.h"
#include "src/pragha-backend.h"
#include "src/pragha-background-task-bar.h"
#include "src/pragha-background-task-widget.h"
#include "src/pragha-database-provider.h"
#include "src/pragha-menubar.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-song-cache.h"
#include "src/pragha-temp-provider.h"
#include "src/pragha-utils.h"
#include "src/pragha-window.h"

#include "plugins/pragha-plugin-macros.h"


typedef struct _PraghaSubsonicPluginPrivate PraghaSubsonicPluginPrivate;

struct _PraghaSubsonicPluginPrivate {
	PraghaApplication          *pragha;

	PraghaSubsonicApi          *subsonic;

	PraghaSubsonicPreferences  *preferences;

	gchar                      *server;

	PraghaBackgroundTaskWidget *task_widget;

	GtkActionGroup             *action_group_main_menu;
	guint                       merge_id_main_menu;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_SUBSONIC_PLUGIN,
                        PraghaSubsonicPlugin,
                        pragha_subsonic_plugin)

/*
 * Propotypes
 */

static void
pragha_subsonic_plugin_authenticate (PraghaSubsonicPlugin *plugin);

/*
 * Menu actions
 */

static void
pragha_subsonic_plugin_upgrade_database_action (GtkAction            *action,
                                                PraghaSubsonicPlugin *plugin)
{
	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	if (pragha_subsonic_api_is_scanning(priv->subsonic))
		return;

	pragha_subsonic_api_scan_server (priv->subsonic);
}

static void
pragha_subsonic_plugin_upgrade_database_gmenu_action (GSimpleAction *action,
                                                      GVariant      *parameter,
                                                      gpointer       user_data)
{
	PraghaSubsonicPlugin *plugin = user_data;
	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	if (pragha_subsonic_api_is_scanning(priv->subsonic))
		return;

	pragha_subsonic_api_scan_server (priv->subsonic);
}

static const GtkActionEntry main_menu_actions [] = {
	{"Refresh the Subsonic library", NULL, N_("Refresh the Subsonic library"),
	 "", "Refresh the Subsonic library", G_CALLBACK(pragha_subsonic_plugin_upgrade_database_action)}};

static const gchar *main_menu_xml = "<ui>								\
	<menubar name=\"Menubar\">											\
		<menu action=\"ToolsMenu\">										\
			<placeholder name=\"pragha-plugins-placeholder\">			\
				<menuitem action=\"Refresh the Subsonic library\"/>		\
				<separator/>											\
			</placeholder>												\
		</menu>															\
	</menubar>															\
</ui>";


/*
 * Api responces
 */

static void
pragha_subsonic_plugin_authenticated (PraghaSubsonicApi    *subsonic,
                                      SubsonicStatusCode    code,
                                      PraghaSubsonicPlugin *plugin)
{
	PraghaBackgroundTaskBar *taskbar;
	PraghaAppNotification *notification;
	PraghaDatabaseProvider *provider;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	if (code != S_GENERIC_OK) {
		notification = pragha_app_notification_new (string_is_empty(priv->server) ? priv->server : _("Subsonic"),
			_("There was an error authenticating with the server"));
		pragha_app_notification_set_timeout (notification, 5);
		pragha_app_notification_show (notification);
	}
	else {
		provider = pragha_database_provider_get ();
		if (!pragha_provider_exist (provider, priv->server)) {
			taskbar = pragha_background_task_bar_get ();
			pragha_background_task_bar_prepend_widget (taskbar, GTK_WIDGET(priv->task_widget));
			pragha_background_task_widget_set_description (priv->task_widget, _("Getting albums from the server"));
			g_object_unref(G_OBJECT(taskbar));

			pragha_subsonic_api_scan_server (priv->subsonic);
		}
		g_object_unref (provider);
	}
}

static void
pragha_subsonic_plugin_pinged (PraghaSubsonicApi    *subsonic,
                               SubsonicStatusCode    code,
                               PraghaSubsonicPlugin *plugin)
{
	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	if (code == S_GENERIC_OK) {
		g_warning ("Has connection with Subsonic server...");
	}
	else {
		g_warning ("Wrom connection with Subsonic server. Errro code: %i", code);
	}
}

static void
pragha_subsonic_plugin_scan_finished (PraghaSubsonicApi    *subsonic,
                                      SubsonicStatusCode    code,
                                      PraghaSubsonicPlugin *plugin)
{
	PraghaTempProvider *provider;
	PraghaAppNotification *notification;
	PraghaBackgroundTaskBar *taskbar;
	PraghaPlaylist *playlist;
	GSList *songs_list, *l;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	taskbar = pragha_background_task_bar_get ();
	pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(taskbar));

	if (code == S_GENERIC_OK) {
		provider = pragha_temp_provider_new (priv->server,
		                                     "SUBSONIC",
		                                     priv->server,
		                                     "folder-remote");

		songs_list = pragha_subsonic_api_get_songs_list (priv->subsonic);
		for (l = songs_list ; l != NULL ; l = l->next) {
			pragha_temp_provider_insert_track (provider, PRAGHA_MUSICOBJECT(l->data));
		}
		pragha_temp_provider_merge_database (provider);
		pragha_temp_provider_commit_database (provider);
		pragha_temp_provider_set_visible (provider, TRUE);
		g_object_unref (provider);
	}
	else if (code != S_USER_CANCELLED) {
		notification = pragha_app_notification_new (priv->server,
			_("There was an error searching the collection."));
		pragha_app_notification_show (notification);
	}

}


/*
 * Authentication.
 */

static void
pragha_subsonic_plugin_authenticate (PraghaSubsonicPlugin *plugin)
{
	const gchar *server = NULL, *username = NULL, *password = NULL;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	/* Get settings */

	server = pragha_subsonic_preferences_get_server_text(priv->preferences);
	username = pragha_subsonic_preferences_get_username_text(priv->preferences);
	password = pragha_subsonic_preferences_get_password_text(priv->preferences);

	if (string_is_empty (server))
		return;
	if (string_is_empty (username))
		return;
	if (string_is_empty (password))
		return;

	priv->server = g_strdup (server);

	pragha_subsonic_api_authentication (priv->subsonic,
	                                    server, username, password);
}


/*
 * PraghaSubsonicPreferences signals
 */

static void
pragha_subsonic_preferences_server_changed (PraghaSubsonicPreferences *preferences,
                                            PraghaSubsonicPlugin      *plugin)
{
	PraghaDatabaseProvider *provider;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	pragha_subsonic_api_deauthentication(priv->subsonic);

	if (string_is_not_empty(priv->server)) {
		provider = pragha_database_provider_get ();
		pragha_provider_remove (provider,
		                        priv->server);
		pragha_provider_update_done (provider);
		g_object_unref (provider);

		g_free (priv->server);
		priv->server = NULL;
	}
}

static void
pragha_subsonic_preferences_credentials_changed (PraghaSubsonicPreferences *preferences,
                                                 PraghaSubsonicPlugin      *plugin)
{
	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	pragha_subsonic_api_deauthentication (priv->subsonic);

	pragha_subsonic_plugin_authenticate (plugin);
}


/*
 * Gstreamer.source.
 */

static gboolean
pragha_musicobject_is_subsonic_file (PraghaMusicobject *mobj)
{
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicSource file_source = FILE_NONE;

	enum_map = pragha_music_enum_get ();
	file_source = pragha_music_enum_map_get(enum_map, "SUBSONIC");
	g_object_unref (enum_map);

	return (file_source == pragha_musicobject_get_source (mobj));
}

static void
pragha_subsonic_plugin_prepare_source (PraghaBackend        *backend,
                                       PraghaSubsonicPlugin *plugin)
{
	PraghaSongCache *cache;
	PraghaMusicobject *mobj;
	const gchar *location = NULL;
	gchar *filename = NULL, *song_id = NULL, *uri = NULL;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_subsonic_file (mobj))
		return;

	location =  pragha_musicobject_get_file (mobj);

	cache = pragha_song_cache_get ();
	filename = pragha_song_cache_get_from_location (cache, location);
	g_object_unref(cache);

	if (filename != NULL) {
		uri = g_filename_to_uri (filename, NULL, NULL);
		g_free (filename);
	}
	else {
		uri = pragha_subsonic_api_get_playback_url (priv->subsonic, location);
	}

	pragha_backend_set_playback_uri (backend, uri);
	g_free (uri);
}

static void
pragha_subsonic_plugin_download_done (PraghaBackend        *backend,
                                      gchar                *filename,
                                      PraghaSubsonicPlugin *plugin)
{
	PraghaSongCache *cache;
	PraghaMusicobject *mobj;
	const gchar *location = NULL;

	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_subsonic_file (mobj))
		return;

	location = pragha_musicobject_get_file (mobj);

	cache = pragha_song_cache_get ();
	pragha_song_cache_put_location (cache, location, filename);
	g_object_unref(cache);
}


/*
 * Plugin.
 */

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PreferencesDialog *dialog;
	PraghaBackend *backend;
	GtkWidget *settings_widget;
	GMenuItem *item;
	GSimpleAction *action;

	PraghaSubsonicPlugin *plugin = PRAGHA_SUBSONIC_PLUGIN (activatable);

	PraghaSubsonicPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "Subsonic Server plugin %s", G_STRFUNC);

	/* New subsonic client api */

	priv->subsonic = pragha_subsonic_api_new();
	g_signal_connect (priv->subsonic, "authenticated",
	                  G_CALLBACK (pragha_subsonic_plugin_authenticated), plugin);
	g_signal_connect (priv->subsonic, "pong",
	                  G_CALLBACK (pragha_subsonic_plugin_pinged), plugin);
	g_signal_connect (priv->subsonic, "scan-finished",
	                  G_CALLBACK (pragha_subsonic_plugin_scan_finished), plugin);

	/* Settings */

	dialog = pragha_application_get_preferences_dialog (priv->pragha);

	priv->preferences = pragha_subsonic_preferences_new (dialog);
	g_signal_connect (priv->preferences, "server-changed",
	                  G_CALLBACK (pragha_subsonic_preferences_server_changed), plugin);
	g_signal_connect (priv->preferences, "credentials-changed",
	                  G_CALLBACK (pragha_subsonic_preferences_credentials_changed), plugin);

	/* Attach main menu */

	priv->action_group_main_menu = gtk_action_group_new ("PraghaSubsonicPlugin");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	/* Attach gear menu */

	action = g_simple_action_new ("refresh-subsonic", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_subsonic_plugin_upgrade_database_gmenu_action), plugin);

	item = g_menu_item_new (_("Refresh the Subsonic library"), "win.refresh-subsonic");
	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);
	g_object_unref (item);

	/* Backend signals */

	backend = pragha_application_get_backend (priv->pragha);
	pragha_backend_set_local_storage (backend, TRUE);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_subsonic_plugin_prepare_source), plugin);
	g_signal_connect (backend, "download-done",
	                  G_CALLBACK(pragha_subsonic_plugin_download_done), plugin);

	/* Task progress widget */

	priv->task_widget = pragha_background_task_widget_new (_("Searching files to analyze"),
	                                                       "network-server",
	                                                       0,
	                                                       pragha_subsonic_get_cancellable(priv->subsonic));
	g_object_ref (G_OBJECT(priv->task_widget));

	/* Authenticate */

	pragha_subsonic_plugin_authenticate (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	PraghaBackgroundTaskBar *taskbar;
	PraghaDatabaseProvider *provider;
	PraghaPreferences *preferences;
	PreferencesDialog *dialog;
	GtkWidget *settings_widget;

	PraghaSubsonicPlugin *plugin = PRAGHA_SUBSONIC_PLUGIN (activatable);
	PraghaSubsonicPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Subsonic Server plugin %s", G_STRFUNC);

	/* Subsonic client */

	g_object_unref (priv->subsonic);

	/* Remove background task widget. */

	taskbar = pragha_background_task_bar_get ();
	pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(taskbar));

	/* Remove settings */

	g_object_unref (priv->preferences);

	/* Remove menu actions */

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	pragha_menubar_remove_action (priv->pragha,
	                              "pragha-plugins-placeholder",
	                              "refresh-subsonic");

	/* If user disable the plugin (Pragha not shutdown) */

	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha)))
	{
		pragha_subsonic_preferences_forget_settings (priv->preferences);
		if (string_is_not_empty(priv->server)) {
			provider = pragha_database_provider_get ();
			pragha_provider_remove (provider,
			                        priv->server);
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}
	}

	/* Clean memory */

	if (priv->server) {
		g_free (priv->server);
		priv->server = NULL;
	}
}
