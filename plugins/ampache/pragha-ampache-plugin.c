/*************************************************************************/
/* Copyright (C) 2015 matias <mati86dl@gmail.com>                        */
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

#include <libsoup/soup.h>

#include "pragha-ampache-plugin.h"

#include "src/pragha.h"
#include "src/pragha-utils.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-playlist.h"
#include "src/pragha-menubar.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-window.h"
#include "src/pragha-hig.h"
#include "src/xml_helper.h"

#include "plugins/pragha-plugin-macros.h"

typedef struct _PraghaAmpachePluginPrivate PraghaAmpachePluginPrivate;

struct _PraghaAmpachePluginPrivate {
	PraghaApplication    *pragha;

	gchar                *server;
	gchar                *username;
	gchar                *password;

	gchar                *auth;
	gint                  songs_count;

	GtkWidget            *setting_widget;
	GtkWidget            *server_entry;
	GtkWidget            *user_entry;
	GtkWidget            *pass_entry;

	GtkActionGroup       *action_group_main_menu;
	guint                 merge_id_main_menu;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_AMPACHE_PLUGIN,
                        PraghaAmpachePlugin,
                        pragha_ampache_plugin)

/*
 * Definitions
 */

#define GROUP_KEY_AMPACHE  "ampache"
#define KEY_AMPACHE_SERVER "server"
#define KEY_AMPACHE_USER   "username"
#define KEY_AMPACHE_PASS   "password"

/*
 *
 */
static void
pragha_ampache_plugin_search_music (PraghaAmpachePlugin *plugin);

static void
pragha_ampache_plugin_authenticate (PraghaAmpachePlugin *plugin);
static void
pragha_ampache_plugin_deauthenticate (PraghaAmpachePlugin *plugin);

/*
 * Menu actions
 */
static void
pragha_ampache_plugin_search_music_action (GtkAction *action, PraghaAmpachePlugin *plugin)
{
	pragha_ampache_plugin_search_music (plugin);
}

static void
pragha_gmenu_ampcahe_plugin_search_music_action (GSimpleAction *action,
                                                 GVariant      *parameter,
                                                 gpointer       user_data)
{
	pragha_ampache_plugin_search_music (PRAGHA_AMPACHE_PLUGIN(user_data));
}

static const GtkActionEntry main_menu_actions [] = {
	{"Append music on Ampache Server", NULL, N_("Search music on Ampache server"),
	 "", "Append music on Ampache Server", G_CALLBACK(pragha_ampache_plugin_search_music_action)}
};

static const gchar *main_menu_xml = "<ui>								\
	<menubar name=\"Menubar\">											\
		<menu action=\"ToolsMenu\">										\
			<placeholder name=\"pragha-plugins-placeholder\">			\
				<menuitem action=\"Append music on Ampache Server\"/>	\
				<separator/>											\
			</placeholder>												\
		</menu>															\
	</menubar>															\
</ui>";

/* Helpers */

static gchar *
pragha_ampache_plugin_unescape_response (const gchar *response)
{
	const gchar *cpointer = NULL;
	gchar *rest = NULL;

	if (g_ascii_strncasecmp(response, "<![CDATA[", 9) == 0) {
		cpointer = response;
		cpointer += 9;

		rest = g_strndup(cpointer, (strlen(cpointer) - 3));
	}
	else {
		rest = g_strdup(response);
	}

	return rest;
}

/*
 * Settings.
 */

static gchar *
pragha_ampache_plugin_get_server (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_AMPACHE_SERVER);

	g_free (plugin_group);

	return string;
}

static void
pragha_ampache_plugin_set_server (PraghaPreferences *preferences, const gchar *server)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	if (string_is_not_empty(server))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_SERVER,
		                               server);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_SERVER);

	g_free (plugin_group);
}

static gchar *
pragha_ampache_plugin_get_user (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_AMPACHE_USER);

	g_free (plugin_group);

	return string;
}

static void
pragha_ampache_plugin_set_user (PraghaPreferences *preferences, const gchar *user)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	if (string_is_not_empty(user))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_USER,
		                               user);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_USER);

	g_free (plugin_group);
}

static gchar *
pragha_ampache_plugin_get_password (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_AMPACHE_PASS);

	g_free (plugin_group);

	return string;
}

static void
pragha_ampache_plugin_set_password (PraghaPreferences *preferences, const gchar *pass)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);

	if (string_is_not_empty(pass))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_PASS,
		                               pass);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_AMPACHE_PASS);

	g_free (plugin_group);
}

/*
 * Ampache plugin.
 */

PraghaMusicobject *
pragha_ampache_xml_get_media (XMLNode *xml)
{
	PraghaMusicobject *mobj;
	XMLNode *xi = NULL;
	gchar *url = NULL, *title =  NULL, *artist = NULL, *album = NULL;

	xi = xmlnode_get (xml, CCA{"song", "url", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content))
		url = pragha_ampache_plugin_unescape_response (xi->content);

	xi = xmlnode_get (xml, CCA{"song", "title", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content))
		title = pragha_ampache_plugin_unescape_response (xi->content);

	xi = xmlnode_get (xml, CCA{"song", "artist", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content))
		artist = pragha_ampache_plugin_unescape_response (xi->content);

	xi = xmlnode_get (xml, CCA{"song", "album", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content))
		album = pragha_ampache_plugin_unescape_response (xi->content);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", url,
	                     "source", FILE_HTTP,
	                     "title", title,
	                     "artist", artist,
	                     "album", album,
	                     NULL);

	g_free (url);
	g_free (title);
	g_free (artist);
	g_free (album);

	return mobj;
}

static void
pragha_ampache_get_songs_done (SoupSession *session,
                               SoupMessage *msg,
                               gpointer     user_data)
{
	PraghaPlaylist *playlist = NULL;
	PraghaMusicobject *mobj;
	XMLNode *xml = NULL, *xi;
	GList *list = NULL;

	PraghaAmpachePlugin *plugin = user_data;
	PraghaAmpachePluginPrivate *priv = plugin->priv;

	remove_watch_cursor (pragha_application_get_window(priv->pragha));

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		return;

	xml = tinycxml_parse ((gchar *)msg->response_body->data);

	xi = xmlnode_get (xml, CCA{"root", "song", NULL }, NULL, NULL);
	for (;xi;xi=xi->next) {
		mobj = pragha_ampache_xml_get_media (xi);
		if (G_LIKELY(mobj))
			list = g_list_prepend (list, mobj);
	}

	if (list) {
		playlist = pragha_application_get_playlist (priv->pragha);
		pragha_playlist_append_mobj_list (playlist, list);
		g_list_free (list);
	}

	xmlnode_free (xml);
}


static void
pragha_ampache_plugin_search_music (PraghaAmpachePlugin *plugin)
{
	SoupSession *session;
	SoupMessage *msg;
	gchar *url = NULL;
	guint i = 0;

	PraghaAmpachePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Amache server plugin %s", G_STRFUNC);

	if (!priv->auth)
		return;

	set_watch_cursor (pragha_application_get_window(priv->pragha));

	/* Get auth */
	session = soup_session_sync_new ();

	for (i=0 ; i <= priv->songs_count/500; i++) {
		url = g_strdup_printf("%s/server/xml.server.php?action=songs&offset=%i&limit=500&auth=%s",
		                      priv->server, i*500, priv->auth);

		msg = soup_message_new ("GET", url);
		soup_session_queue_message (session, msg,
		                            pragha_ampache_get_songs_done, plugin);

		g_free (url);
	}
}

/*
 * Ampache Settings
 */
static void
pragha_ampache_preferences_dialog_response (GtkDialog           *dialog,
                                            gint                 response_id,
                                            PraghaAmpachePlugin *plugin)
{
	PraghaPreferences *preferences;
	const gchar *entry_server = NULL, *entry_user = NULL, *entry_pass = NULL;
	gchar *test_server = NULL, *test_user = NULL, *test_pass = NULL;
	gboolean changed = FALSE;

	PraghaAmpachePluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();

	test_server = pragha_ampache_plugin_get_server (preferences);
	test_user = pragha_ampache_plugin_get_user (preferences);
	test_pass = pragha_ampache_plugin_get_password (preferences);

	switch(response_id) {
		case GTK_RESPONSE_CANCEL:
			pragha_gtk_entry_set_text (GTK_ENTRY(priv->server_entry), test_server);
			pragha_gtk_entry_set_text (GTK_ENTRY(priv->user_entry), test_user);
			pragha_gtk_entry_set_text (GTK_ENTRY(priv->pass_entry), test_pass);
			break;
		case GTK_RESPONSE_OK:
			entry_server = gtk_entry_get_text (GTK_ENTRY(priv->server_entry));
			entry_user = gtk_entry_get_text (GTK_ENTRY(priv->user_entry));
			entry_pass = gtk_entry_get_text (GTK_ENTRY(priv->pass_entry));

			if (g_strcmp0 (test_server, entry_server)) {
				pragha_ampache_plugin_set_server (preferences, entry_server);
				changed = TRUE;
			}
			if (g_strcmp0 (test_user, entry_user)) {
				pragha_ampache_plugin_set_user (preferences, entry_user);
				changed = TRUE;
			}
			if (g_strcmp0 (test_pass, entry_pass)) {
				pragha_ampache_plugin_set_password (preferences, entry_pass);
				changed = TRUE;
			}

			if (changed) {
				pragha_ampache_plugin_deauthenticate (plugin);
				pragha_ampache_plugin_authenticate (plugin);
			}
			break;
		default:
			break;
	}

	g_object_unref (preferences);
	g_free (test_server);
	g_free (test_user);
	g_free (test_pass);
}

static void
pragha_ampache_plugin_append_setting (PraghaAmpachePlugin *plugin)
{
	PreferencesDialog *dialog;
	GtkWidget *table, *label, *ampache_server, *ampache_uname, *ampache_pass;
	guint row = 0;

	PraghaAmpachePluginPrivate *priv = plugin->priv;

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title (table, &row, "Ampache");

	label = gtk_label_new (_("Server"));
	ampache_server = gtk_entry_new ();
	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(ampache_server), GTK_ENTRY_ICON_PRIMARY, "network-server");
	gtk_entry_set_activates_default (GTK_ENTRY(ampache_server), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, ampache_server);


	label = gtk_label_new (_("Username"));
	ampache_uname = gtk_entry_new ();
	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(ampache_uname), GTK_ENTRY_ICON_PRIMARY, "system-users");
	gtk_entry_set_max_length (GTK_ENTRY(ampache_uname), LASTFM_UNAME_LEN);
	gtk_entry_set_activates_default (GTK_ENTRY(ampache_uname), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, ampache_uname);

	label = gtk_label_new (_("Password"));
	ampache_pass = gtk_entry_new ();
	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(ampache_pass), GTK_ENTRY_ICON_PRIMARY, "changes-prevent");
	gtk_entry_set_max_length (GTK_ENTRY(ampache_pass), LASTFM_PASS_LEN);
	gtk_entry_set_visibility (GTK_ENTRY(ampache_pass), FALSE);
	//gtk_entry_set_invisible_char (GTK_ENTRY(ampache_pass), '*');
	gtk_entry_set_activates_default (GTK_ENTRY(ampache_pass), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, ampache_pass);

	/* Append panes */

	priv->setting_widget = table;
	priv->server_entry = ampache_server;
	priv->user_entry = ampache_uname;
	priv->pass_entry = ampache_pass;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_append_services_setting (dialog,
	                                            priv->setting_widget, FALSE);

	/* Configure handler and settings */
	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_ampache_preferences_dialog_response),
	                                           plugin);
}

static void
pragha_ampache_plugin_remove_setting (PraghaAmpachePlugin *plugin)
{
	PreferencesDialog *dialog;
	PraghaAmpachePluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_remove_services_setting (dialog,
	                                            priv->setting_widget);

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_ampache_preferences_dialog_response),
	                                              plugin);
}
/*
 * Authentication.
 */

static void
pragha_ampache_get_auth_done (SoupSession *session,
                              SoupMessage *msg,
                              gpointer     user_data)
{
	XMLNode *xml = NULL, *xi;
	gchar *songs_count = NULL;

	PraghaAmpachePlugin *plugin = user_data;
	PraghaAmpachePluginPrivate *priv = plugin->priv;

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		return;

	xml = tinycxml_parse ((gchar *)msg->response_body->data);

	xi = xmlnode_get (xml, CCA{"root", "auth", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content)) {
		priv->auth = pragha_ampache_plugin_unescape_response (xi->content);
	}

	xi = xmlnode_get (xml, CCA{"root", "songs", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content)) {
		songs_count = pragha_ampache_plugin_unescape_response (xi->content);
		priv->songs_count = atoi (songs_count);
		g_free (songs_count);
	}

	xmlnode_free (xml);
}

static void
pragha_ampache_plugin_authenticate (PraghaAmpachePlugin *plugin)
{
	PraghaPreferences *preferences;
	SoupSession *session = NULL;
	SoupMessage *msg = NULL;
	GChecksum *hash = NULL;
	time_t timestamp = 0;
	gchar *url = NULL, *timestampc = NULL, *key = NULL, *passphraseh = NULL, *passphrasec = NULL;

	PraghaAmpachePluginPrivate *priv = plugin->priv;

	/* Get settings */

	preferences = pragha_preferences_get ();
	priv->server = pragha_ampache_plugin_get_server (preferences);
	priv->username = pragha_ampache_plugin_get_user (preferences);
	priv->password = pragha_ampache_plugin_get_password (preferences);
	g_object_unref (preferences);

	if (string_is_empty (priv->server))
		return;
	if (string_is_empty (priv->username))
		return;
	if (string_is_empty (priv->password))
		return;

	/* Authenticate */

	time(&timestamp);
	timestampc = g_strdup_printf("%i", (gint)timestamp);

	hash = g_checksum_new (G_CHECKSUM_SHA256);
	g_checksum_update (hash, (guchar *)priv->password, strlen(priv->password));
	key = g_strdup(g_checksum_get_string(hash));

	g_checksum_reset (hash);

	passphrasec = g_strdup_printf("%s%s", timestampc, key);
	g_checksum_update(hash, (guchar *)passphrasec, -1);
	passphraseh = g_strdup(g_checksum_get_string(hash));

	url = g_strdup_printf("%s/server/xml.server.php?action=handshake&auth=%s&timestamp=%s&version=350001&user=%s",
	                      priv->server, passphraseh, timestampc, priv->username);

	session = soup_session_sync_new ();
	msg = soup_message_new ("GET", url);
	soup_session_queue_message (session, msg,
	                            pragha_ampache_get_auth_done, plugin);

	g_checksum_free(hash);

	g_free (timestampc);
	g_free (key);
	g_free (passphrasec);
	g_free (passphraseh);
	g_free (url);
}

static void
pragha_ampache_plugin_deauthenticate (PraghaAmpachePlugin *plugin)
{
	PraghaAmpachePluginPrivate *priv = plugin->priv;

	if (priv->server) {
		g_free (priv->server);
		priv->server = NULL;
	}
	if (priv->username) {
		g_free (priv->username);
		priv->username = NULL;
	}
	if (priv->username) {
		g_free (priv->username);
		priv->username = NULL;
	}
	if (priv->auth) {
		g_free (priv->auth);
		priv->auth = NULL;
	}
	if (priv->songs_count > 0)
		priv->songs_count = 0;
}

/*
 * Plugin.
 */

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GMenuItem *item;
	GSimpleAction *action;

	PraghaAmpachePlugin *plugin = PRAGHA_AMPACHE_PLUGIN (activatable);

	PraghaAmpachePluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "Ampache Server plugin %s", G_STRFUNC);

	/* Attach main menu */

	priv->action_group_main_menu = gtk_action_group_new ("PraghaAmpachePlugin");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	/* Gear Menu */

	action = g_simple_action_new ("append-ampache", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_gmenu_ampcahe_plugin_search_music_action), plugin);

	item = g_menu_item_new (_("Append music on Ampache server"), "win.append-ampache");

	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);

	pragha_ampache_plugin_append_setting (plugin);

	/* Authenticate */

	pragha_ampache_plugin_authenticate (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaPreferences *preferences;
	gchar *plugin_group = NULL;

	PraghaAmpachePlugin *plugin = PRAGHA_AMPACHE_PLUGIN (activatable);
	PraghaAmpachePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Ampache Server plugin %s", G_STRFUNC);

	/* Settings */

	preferences = pragha_application_get_preferences (priv->pragha);
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_AMPACHE);
	if (!pragha_plugins_is_shutdown(pragha_application_get_plugins_engine(priv->pragha))) {
		pragha_preferences_remove_group (preferences, plugin_group);
	}
	g_free (plugin_group);

	/* Menu Action */

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	pragha_menubar_remove_action (priv->pragha, "pragha-plugins-placeholder", "append-ampache");

	pragha_ampache_plugin_remove_setting (plugin);
}
