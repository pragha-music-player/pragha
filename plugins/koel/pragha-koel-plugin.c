/*************************************************************************/
/* Copyright (C) 2015-2018 matias <mati86dl@gmail.com>                   */
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
#include <json-glib/json-glib.h>

#include "pragha-koel-plugin.h"

#include "src/pragha.h"
#include "src/pragha-favorites.h"
#include "src/pragha-utils.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-playlist.h"
#include "src/pragha-playlists-mgmt.h"
#include "src/pragha-menubar.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-window.h"
#include "src/pragha-hig.h"
#include "src/pragha-database-provider.h"
#include "src/pragha-background-task-bar.h"
#include "src/pragha-background-task-widget.h"
#include "src/pragha-song-cache.h"
#include "plugins/pragha-plugin-macros.h"

typedef struct _PraghaKoelPluginPrivate PraghaKoelPluginPrivate;

struct _PraghaKoelPluginPrivate {
	PraghaApplication          *pragha;
	PraghaSongCache            *cache;
	PraghaFavorites            *favorites;

	GCancellable               *cancellable;

	gchar                      *server;

	gchar                      *token;
	gboolean                    upgrade;

	GHashTable                 *tracks_table;

	PraghaBackgroundTaskWidget *task_widget;

	GtkWidget                  *setting_widget;
	GtkWidget                  *server_entry;
	GtkWidget                  *user_entry;
	GtkWidget                  *pass_entry;

	GtkActionGroup             *action_group_main_menu;
	guint                       merge_id_main_menu;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_KOEL_PLUGIN,
                        PraghaKoelPlugin,
                        pragha_koel_plugin)

/*
 * Definitions
 */

#define GROUP_KEY_KOEL  "koel"
#define KEY_KOEL_SERVER "server"
#define KEY_KOEL_USER   "username"
#define KEY_KOEL_PASS   "password"

/*
 * Propotypes
 */

static void
pragha_koel_plugin_authenticate (PraghaKoelPlugin *plugin);
static void
pragha_koel_plugin_deauthenticate (PraghaKoelPlugin *plugin);

static void
pragha_koel_plugin_set_need_upgrade (PraghaKoelPlugin *plugin, gboolean upgrade);
static gboolean
pragha_koel_plugin_need_upgrade (PraghaKoelPlugin *plugin);

static gboolean
pragha_musicobject_is_koel_file (PraghaMusicobject *mobj);

/*
 * Menu actions
 */

static void
pragha_koel_plugin_upgrade_database (PraghaKoelPlugin *plugin)
{
	pragha_koel_plugin_deauthenticate (plugin);
	pragha_koel_plugin_set_need_upgrade (plugin, TRUE);
	pragha_koel_plugin_authenticate (plugin);
}

static void
pragha_koel_plugin_upgrade_database_action (GtkAction *action, PraghaKoelPlugin *plugin)
{
	pragha_koel_plugin_upgrade_database (plugin);
}

static void
pragha_koel_plugin_upgrade_database_gmenu_action (GSimpleAction *action,
                                                     GVariant      *parameter,
                                                     gpointer       user_data)
{
	PraghaKoelPlugin *plugin = user_data;
	pragha_koel_plugin_upgrade_database (plugin);
}

static const GtkActionEntry main_menu_actions [] = {
	{"Refresh the Koel library", NULL, N_("Refresh the Koel library"),
	 "", "Refresh the Koel library", G_CALLBACK(pragha_koel_plugin_upgrade_database_action)}};

static const gchar *main_menu_xml = "<ui>								\
	<menubar name=\"Menubar\">											\
		<menu action=\"ToolsMenu\">										\
			<placeholder name=\"pragha-plugins-placeholder\">			\
				<menuitem action=\"Refresh the Koel library\"/>		\
				<separator/>											\
			</placeholder>												\
		</menu>															\
	</menubar>															\
</ui>";

/* Helpers */

static gchar *
pragha_koel_plugin_get_song_id (const gchar *server, const gchar *raw_uri)
{
	GRegex *regex = NULL;
	gchar *song_id = NULL, *regex_text = NULL;

	regex_text = g_strdup_printf ("%s/api/", server);
	regex = g_regex_new (regex_text,
	                     G_REGEX_MULTILINE | G_REGEX_RAW, 0, NULL);
	song_id = g_regex_replace_literal (regex, raw_uri, -1, 0, "", 0, NULL);
	g_regex_unref(regex);
	g_free (regex_text);

	return song_id;
}

static void
pragha_koel_plugin_add_track_db (gpointer key,
                                    gpointer value,
                                    gpointer user_data)
{
	PraghaMusicobject *mobj = value;
	PraghaDatabase *database = user_data;

	pragha_database_add_new_musicobject (database, mobj);

	pragha_process_gtk_events ();
}

/*
 * Basic Cache.
 */

static void
pragha_koel_cache_clear (PraghaKoelPlugin *plugin)
{
	PraghaKoelPluginPrivate *priv = plugin->priv;

	g_hash_table_remove_all (priv->tracks_table);
}

static void
pragha_koel_save_cache (PraghaKoelPlugin *plugin)
{
	PraghaDatabase *database;
	PraghaKoelPluginPrivate *priv = plugin->priv;

	database = pragha_database_get ();
	g_hash_table_foreach (priv->tracks_table,
	                      pragha_koel_plugin_add_track_db,
	                      database);
	g_object_unref (database);
}

static void
pragha_koel_cache_insert_track (PraghaKoelPlugin *plugin, PraghaMusicobject *mobj)
{
	PraghaKoelPluginPrivate *priv = plugin->priv;

	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	g_hash_table_insert (priv->tracks_table,
	                     g_strdup(file),
	                     mobj);
}

/*
 * Settings.
 */

static gchar *
pragha_koel_plugin_get_server (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_KOEL_SERVER);

	g_free (plugin_group);

	return string;
}

static void
pragha_koel_plugin_set_server (PraghaPreferences *preferences, const gchar *server)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	if (string_is_not_empty(server))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_KOEL_SERVER,
		                               server);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_KOEL_SERVER);

	g_free (plugin_group);
}

static gchar *
pragha_koel_plugin_get_user (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_KOEL_USER);

	g_free (plugin_group);

	return string;
}

static void
pragha_koel_plugin_set_user (PraghaPreferences *preferences, const gchar *user)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	if (string_is_not_empty(user))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_KOEL_USER,
		                               user);
	else
		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_KOEL_USER);

	g_free (plugin_group);
}

static gchar *
pragha_koel_plugin_get_password (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_KOEL_PASS);

	g_free (plugin_group);

	return string;
}

static void
pragha_koel_plugin_set_password (PraghaPreferences *preferences, const gchar *pass)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);

	if (string_is_not_empty(pass))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_KOEL_PASS,
		                               pass);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_KOEL_PASS);

	g_free (plugin_group);
}

/*
 * Koel plugin.
 */

static const gchar *
pragha_koel_plugin_json_array_get_name (JsonArray *jarray,
                                        gint64     jid)
{
	JsonObject *json_object;
	gint64 object_id = 0;
	const gchar *object_name = NULL;
	GList *l_node = NULL;
	GList *l_nodes = json_array_get_elements (jarray);
	for (l_node = l_nodes; l_node != NULL; l_node = g_list_next(l_node))
	{
		json_object = json_node_get_object ((JsonNode *)l_node->data);
		object_id = json_object_get_int_member(json_object, "id");
		if (jid == object_id)
		{
			object_name = json_object_get_string_member(json_object, "name");
			break;
		}
	}
	g_list_free(l_nodes);

	return object_name;
}

static gint64
pragha_koel_plugin_json_array_get_int_member (JsonArray   *jarray,
                                              gint64       jid,
                                              const gchar *tag)
{
	JsonObject *json_object;
	gint64 object_id = 0, integer_tag = 0;
	GList *l_node = NULL;
	GList *l_nodes = json_array_get_elements (jarray);
	for (l_node = l_nodes; l_node != NULL; l_node = g_list_next(l_node))
	{
		json_object = json_node_get_object ((JsonNode *)l_node->data);
		object_id = json_object_get_int_member(json_object, "id");
		if (jid == object_id)
		{
			if (json_object_has_member(json_object, tag))
				integer_tag = json_object_get_int_member(json_object, tag);
			break;
		}
	}
	g_list_free(l_nodes);

	return integer_tag;
}

static void
pragha_koel_plugin_cache_provider_done (SoupSession *session,
                                        SoupMessage *msg,
                                        gpointer     user_data)
{
	PraghaDatabase *database;
	PraghaDatabaseProvider *provider;
	PraghaToolbar *toolbar;
	GtkWidget *tasks_button;
	PraghaMusicobject *mobj = NULL;

	JsonParser *parser = NULL;
	JsonNode *root, *albums_node, *artists_node, *songs_node, *interactions_node;
	JsonObject *root_object;
	JsonArray *albums_array, *artists_array, *songs_array, *interactions_array;
	GList *l_songs, *l_song, *liked = NULL;

	PraghaKoelPlugin *plugin = user_data;
	PraghaKoelPluginPrivate *priv = plugin->priv;

	if (g_cancellable_is_cancelled (priv->cancellable)) {
		toolbar = pragha_application_get_toolbar (priv->pragha);
		tasks_button = pragha_toolbar_get_task_progress_button (toolbar);
		pragha_background_task_bar_remove_widget (PRAGHA_BACKGROUND_TASK_BAR(tasks_button), GTK_WIDGET(priv->task_widget));
		g_cancellable_reset (priv->cancellable);
		return;
	}

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		toolbar = pragha_application_get_toolbar (priv->pragha);
		tasks_button = pragha_toolbar_get_task_progress_button (toolbar);
		pragha_background_task_bar_remove_widget (PRAGHA_BACKGROUND_TASK_BAR(tasks_button), GTK_WIDGET(priv->task_widget));
		g_critical("KOEL ERROR Response: %s", msg->response_body->data);
		return;
	}

	parser = json_parser_new ();
	json_parser_load_from_data (parser, msg->response_body->data, -1, NULL);
	root = json_parser_get_root (parser);
	root_object = json_node_get_object (root);

	albums_node = json_object_get_member(root_object, "albums");
	albums_array = json_node_get_array (albums_node);

	artists_node = json_object_get_member(root_object, "artists");
	artists_array = json_node_get_array (artists_node);

	songs_node = json_object_get_member(root_object, "songs");
	songs_array = json_node_get_array (songs_node);

	l_songs = json_array_get_elements (songs_array);
	for (l_song = l_songs; l_song != NULL; l_song = g_list_next(l_song))
	{
		JsonObject *song_object = json_node_get_object ((JsonNode *)l_song->data);

		const gchar *song_id = json_object_get_string_member(song_object, "id");
		const gchar *title = json_object_get_string_member(song_object, "title");
		gint64 album_id = json_object_get_int_member(song_object, "album_id");
		gint64 artist_id = json_object_get_int_member(song_object, "artist_id");
		gint64 track = json_object_get_int_member(song_object, "track");
		gint64 length = json_object_get_double_member(song_object, "length");

		const gchar *artist = pragha_koel_plugin_json_array_get_name (artists_array, artist_id);
		const gchar *album = pragha_koel_plugin_json_array_get_name (albums_array, album_id);
		guint64 album_year = pragha_koel_plugin_json_array_get_int_member (albums_array, album_id, "year");
		gchar *url = g_strdup_printf("%s/api/%s", priv->server, song_id);

		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", url,
		                     "source", FILE_HTTP,
		                     "provider",  priv->server,
		                     "track-no", track,
		                     "title", title != NULL ? title : "",
		                     "artist", artist != NULL ? artist : "",
		                     "album", album != NULL ? album : "",
		                     "year", (gint)album_year,
		                     "length", (gint)length,
		                     NULL);

		if (G_LIKELY(mobj))
			pragha_koel_cache_insert_track (plugin, mobj);

		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();

		g_free (url);
	}

	interactions_node = json_object_get_member(root_object, "interactions");
	interactions_array = json_node_get_array (interactions_node);

	l_songs = json_array_get_elements (interactions_array);
	for (l_song = l_songs; l_song != NULL; l_song = g_list_next(l_song))
	{
		JsonObject *song_object = json_node_get_object ((JsonNode *)l_song->data);

		if (!json_object_get_boolean_member (song_object, "liked"))
			continue;

		const gchar *song_id = json_object_get_string_member(song_object, "song_id");
		gchar *url = g_strdup_printf("%s/api/%s", priv->server, song_id);

		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", url,
		                     "source", FILE_HTTP,
		                     "provider",  priv->server,
		                     NULL);

		if (G_LIKELY(mobj))
			liked = g_list_prepend (liked, mobj);

		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();

		g_free (url);
	}

	g_object_unref(parser);

	/* Upgrade. */

	pragha_koel_plugin_set_need_upgrade (plugin, FALSE);

	database = pragha_database_get ();
	provider = pragha_database_provider_get ();
	if (pragha_database_find_provider (database, priv->server))
	{
		pragha_provider_forget_songs (provider, priv->server);
	}
	else
	{
		pragha_provider_add_new (provider,
		                         priv->server,
		                         "KOEL",
		                         priv->server,
		                         "folder-remote");
		pragha_provider_set_visible (provider, priv->server, TRUE);
	}

	if (G_LIKELY(liked)) {
		pragha_playlist_database_update_playlist (database, _("Favorites on Koel"), liked);
		pragha_playlist_database_insert_playlist (database, _("Favorites"), liked);
		g_list_free_full (liked, g_object_unref);
	}

	pragha_koel_save_cache (plugin);

	pragha_koel_cache_clear (plugin);
	g_object_unref (provider);
	g_object_unref (database);

	/* Report that finish */

	toolbar = pragha_application_get_toolbar (priv->pragha);
	tasks_button =  pragha_toolbar_get_task_progress_button (toolbar);
	pragha_background_task_bar_remove_widget (PRAGHA_BACKGROUND_TASK_BAR(tasks_button), GTK_WIDGET(priv->task_widget));

	pragha_provider_update_done (provider);
}

static void
pragha_koel_plugin_msg_cancelled (GCancellable *cancellable, gpointer user_data)
{
	SoupSession *session = user_data;
	soup_session_abort (session);
}

static void
pragha_koel_plugin_cache_provider (PraghaKoelPlugin *plugin)
{
	PraghaToolbar *toolbar;
	GtkWidget *tasks_button;
	SoupSession *session;
	SoupMessage *msg;
	gchar *query = NULL, *request = NULL;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Koel server plugin %s", G_STRFUNC);

	if (!priv->token)
		return;

	/* Add the taks manager */

	toolbar = pragha_application_get_toolbar (priv->pragha);
	tasks_button =  pragha_toolbar_get_task_progress_button (toolbar);
	pragha_background_task_bar_prepend_widget (PRAGHA_BACKGROUND_TASK_BAR(tasks_button), GTK_WIDGET(priv->task_widget));

	/* Launch thread to get music */

	query = g_strdup_printf("%s/api/data?jwt-token=%s", priv->server, priv->token);
	session = soup_session_new ();
	msg = soup_message_new (SOUP_METHOD_GET, query);
	soup_session_queue_message (session, msg,
	                            pragha_koel_plugin_cache_provider_done, plugin);

	g_cancellable_connect (priv->cancellable,
	                       G_CALLBACK (pragha_koel_plugin_msg_cancelled),
	                       session,
	                       NULL);

	g_free (query);
	g_free (request);
}

/*
 * Playcount.
 */
static void
pragha_koel_plugin_increase_playcount_done (SoupSession *session,
                                            SoupMessage *msg,
                                            gpointer     user_data)
{
	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		g_critical("KOEL ERROR Response: %s", msg->response_body->data);
}

static void
pragha_koel_plugin_increase_playcount (PraghaKoelPlugin *plugin, const gchar *filename)
{
	JsonBuilder *builder;
	JsonGenerator *generator;
	JsonNode *node;
	SoupSession *session;
	SoupMessage *msg;
	gchar *query = NULL, *request = NULL, *song_id = NULL, *data = NULL;
	gsize length;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Koel server plugin %s", G_STRFUNC);

	if (!priv->token)
		return;

	song_id = pragha_koel_plugin_get_song_id (priv->server, filename);

	builder = json_builder_new ();
	json_builder_begin_object (builder);
	json_builder_set_member_name (builder, "song");
	json_builder_add_string_value (builder, song_id);
	json_builder_end_object (builder);

	generator = json_generator_new ();
	node = json_builder_get_root (builder);
	json_generator_set_root (generator, node);
	data = json_generator_to_data (generator, &length);

	query = g_strdup_printf("%s/api/interaction/play?jwt-token=%s", priv->server, priv->token);
	session = soup_session_new ();
	msg = soup_message_new (SOUP_METHOD_POST, query);
	soup_message_headers_append (msg->request_headers, "Accept", "application/json");
	soup_message_set_request (msg, "application/json", SOUP_MEMORY_COPY, data, length);
	soup_session_queue_message (session, msg,
	                            pragha_koel_plugin_increase_playcount_done, plugin);

	g_object_unref (generator);
	g_object_unref (builder);
	json_node_free (node);
	g_free (query);
	g_free (request);
	g_free (song_id);
	g_free (data);
}

/*
 * Interactions.
 */
static void
pragha_koel_plugin_interaction_like_done (SoupSession *session,
                                          SoupMessage *msg,
                                          gpointer     user_data)
{
	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		g_critical("KOEL ERROR Response: %s", msg->response_body->data);
}

static void
pragha_koel_plugin_interaction_like_launch (PraghaKoelPlugin *plugin,
                                            const gchar      *filename,
                                            gboolean          like)
{
	JsonBuilder *builder;
	JsonGenerator *generator;
	JsonNode *node;
	SoupSession *session;
	SoupMessage *msg;
	gchar *query = NULL, *request = NULL, *song_id = NULL, *data = NULL;
	gsize length;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Koel server plugin %s", G_STRFUNC);

	if (!priv->token)
		return;

	song_id = pragha_koel_plugin_get_song_id (priv->server, filename);

	builder = json_builder_new ();
	json_builder_begin_object (builder);
	json_builder_set_member_name (builder, "songs");
	json_builder_begin_array(builder);
	json_builder_add_string_value (builder, song_id);
	json_builder_end_array(builder);
	json_builder_end_object (builder);

	generator = json_generator_new ();
	node = json_builder_get_root (builder);
	json_generator_set_root (generator, node);
	data = json_generator_to_data (generator, &length);

	if (like)
		query = g_strdup_printf("%s/api/interaction/batch/like?jwt-token=%s", priv->server, priv->token);
	else
		query = g_strdup_printf("%s/api/interaction/batch/unlike?jwt-token=%s", priv->server, priv->token);

	session = soup_session_new ();
	msg = soup_message_new (SOUP_METHOD_POST, query);
	soup_message_headers_append (msg->request_headers, "Accept", "application/json");
	soup_message_set_request (msg, "application/json", SOUP_MEMORY_COPY, data, length);
	soup_session_queue_message (session, msg,
	                            pragha_koel_plugin_interaction_like_done, plugin);

	g_object_unref (generator);
	g_object_unref (builder);
	json_node_free (node);
	g_free (query);
	g_free (request);
	g_free (song_id);
	g_free (data);
}

static void
pragha_koel_plugin_favorites_song_added (PraghaFavorites   *favorites,
                                         PraghaMusicobject *mobj,
                                         PraghaKoelPlugin  *plugin)
{
	PraghaDatabase *database;
	const gchar *file = NULL;
	gint playlist_id = 0;

	if (!pragha_musicobject_is_koel_file (mobj))
		return;
	file  = pragha_musicobject_get_file (mobj);

	pragha_koel_plugin_interaction_like_launch (plugin, file, TRUE);

	database = pragha_database_get ();
	playlist_id = pragha_database_find_playlist (database, _("Favorites on Koel"));
	pragha_database_add_playlist_track (database, playlist_id, file);
	g_object_unref (database);
}

static void
pragha_koel_plugin_favorites_song_removed (PraghaFavorites   *favorites,
                                           PraghaMusicobject *mobj,
                                           PraghaKoelPlugin  *plugin)
{
	PraghaDatabase *database;
	const gchar *file = NULL;
	gint playlist_id = 0;

	if (!pragha_musicobject_is_koel_file (mobj))
		return;
	file  = pragha_musicobject_get_file (mobj);
	pragha_koel_plugin_interaction_like_launch (plugin, file, FALSE);

	database = pragha_database_get ();
	playlist_id = pragha_database_find_playlist (database, _("Favorites on Koel"));
	pragha_database_delete_playlist_track (database, playlist_id, file);
	g_object_unref (database);
}

/*
 * Koel Settings
 */
static void
pragha_koel_preferences_dialog_response (GtkDialog        *dialog,
                                         gint              response_id,
                                         PraghaKoelPlugin *plugin)
{
	PraghaDatabase *database;
	PraghaDatabaseProvider *provider;
	PraghaPreferences *preferences;
	const gchar *entry_server = NULL, *entry_user = NULL, *entry_pass = NULL;
	gchar *test_server = NULL, *test_user = NULL, *test_pass = NULL;
	gboolean changed = FALSE, changed_server = FALSE;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();

	test_server = pragha_koel_plugin_get_server (preferences);
	test_user = pragha_koel_plugin_get_user (preferences);
	test_pass = pragha_koel_plugin_get_password (preferences);

	switch(response_id)
	{
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
				pragha_koel_plugin_set_server (preferences, entry_server);
				changed = TRUE;
				changed_server = TRUE;
			}
			if (g_strcmp0 (test_user, entry_user)) {
				pragha_koel_plugin_set_user (preferences, entry_user);
				changed = TRUE;
			}
			if (g_strcmp0 (test_pass, entry_pass)) {
				pragha_koel_plugin_set_password (preferences, entry_pass);
				changed = TRUE;
			}

			if (changed)
			{
				/* Deauthenticate connection */

				pragha_koel_plugin_deauthenticate (plugin);

				/* Remove old provider if exist */

				if (changed_server)
				{
					database = pragha_database_get ();
					if (pragha_database_find_provider (database, test_server)) {
						provider = pragha_database_provider_get ();
						pragha_provider_remove (provider, test_server);
						pragha_provider_update_done (provider);
						g_object_unref (provider);
					}
					g_object_unref (database);
				}

				/* With all mandatory fields updates the collection.*/

				if (string_is_not_empty(entry_server) &&
				    string_is_not_empty(entry_user) &&
				    string_is_not_empty(entry_pass))
				{
					pragha_koel_plugin_set_need_upgrade (plugin, TRUE);
					pragha_koel_plugin_authenticate (plugin);
				}
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
pragha_koel_plugin_append_setting (PraghaKoelPlugin *plugin)
{
	PraghaPreferences *preferences;
	PreferencesDialog *dialog;
	GtkWidget *table, *label, *koel_server, *koel_uname, *koel_pass;
	gchar *server = NULL, *user = NULL, *pass = NULL;
	guint row = 0;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title (table, &row, "Koel");

	label = gtk_label_new (_("Server"));
	koel_server = gtk_entry_new ();

	server = pragha_koel_plugin_get_server (preferences);
	pragha_gtk_entry_set_text (GTK_ENTRY(koel_server), server);

	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(koel_server), GTK_ENTRY_ICON_PRIMARY, "network-server");
	gtk_entry_set_activates_default (GTK_ENTRY(koel_server), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, koel_server);

	label = gtk_label_new (_("Username"));
	koel_uname = gtk_entry_new ();

	user = pragha_koel_plugin_get_user (preferences);
	pragha_gtk_entry_set_text (GTK_ENTRY(koel_uname), user);

	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(koel_uname), GTK_ENTRY_ICON_PRIMARY, "system-users");
	gtk_entry_set_max_length (GTK_ENTRY(koel_uname), LASTFM_UNAME_LEN);
	gtk_entry_set_activates_default (GTK_ENTRY(koel_uname), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, koel_uname);

	label = gtk_label_new (_("Password"));
	koel_pass = gtk_entry_new ();

	pass = pragha_koel_plugin_get_password (preferences);
	pragha_gtk_entry_set_text (GTK_ENTRY(koel_pass), pass);

	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(koel_pass), GTK_ENTRY_ICON_PRIMARY, "changes-prevent");
	gtk_entry_set_max_length (GTK_ENTRY(koel_pass), LASTFM_PASS_LEN);
	gtk_entry_set_visibility (GTK_ENTRY(koel_pass), FALSE);
	gtk_entry_set_activates_default (GTK_ENTRY(koel_pass), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, label, koel_pass);

	/* Append panes */

	priv->setting_widget = table;
	priv->server_entry = koel_server;
	priv->user_entry = koel_uname;
	priv->pass_entry = koel_pass;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_append_services_setting (dialog,
	                                            priv->setting_widget, FALSE);

	/* Configure handler and settings */

	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_koel_preferences_dialog_response),
	                                           plugin);

	g_object_unref (preferences);

	g_free (server);
	g_free (user);
	g_free (pass);
}

static void
pragha_koel_plugin_remove_setting (PraghaKoelPlugin *plugin)
{
	PreferencesDialog *dialog;
	PraghaKoelPluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_remove_services_setting (dialog,
	                                            priv->setting_widget);

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_koel_preferences_dialog_response),
	                                              plugin);
}

/*
 * Authentication.
 */

static void
pragha_koel_get_auth_done (SoupSession *session,
                           SoupMessage *msg,
                           gpointer     user_data)
{
	JsonParser *parser;
	JsonNode *root;
	JsonObject *object;

	PraghaKoelPlugin *plugin = user_data;
	PraghaKoelPluginPrivate *priv = plugin->priv;

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		g_critical("KOEL ERROR Response: %s", msg->response_body->data);
		return;
	}

	parser = json_parser_new ();
	json_parser_load_from_data (parser, msg->response_body->data, -1, NULL);
	root = json_parser_get_root (parser);
	object = json_node_get_object (root);

	if (json_object_has_member(object, "token")) {
		priv->token = g_strdup (json_object_get_string_member (object, "token"));
	}
	else {
		g_critical("KOEL AUTH ERROR: %s", json_object_get_string_member (object, "error"));
	}
	g_object_unref(parser);

	if (string_is_not_empty(priv->token))
	{
		if (pragha_koel_plugin_need_upgrade (plugin))
			pragha_koel_plugin_cache_provider (plugin);
	}
}

static void
pragha_koel_plugin_authenticate (PraghaKoelPlugin *plugin)
{
	JsonBuilder *builder;
	JsonGenerator *generator;
	JsonNode *node;
	SoupSession *session;
	SoupMessage *msg;
	const gchar *server = NULL, *username = NULL, *password = NULL;
	gchar *query = NULL, *request = NULL, *data = NULL;
	gsize length;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	/* Get settings */

	server = gtk_entry_get_text (GTK_ENTRY(priv->server_entry));
	username = gtk_entry_get_text (GTK_ENTRY(priv->user_entry));
	password = gtk_entry_get_text (GTK_ENTRY(priv->pass_entry));

	if (string_is_empty (server))
		return;
	if (string_is_empty (username))
		return;
	if (string_is_empty (password))
		return;

	/* Set new server as alias of provider */

	priv->server = g_strdup (server);

	/* Authenticate */

	builder = json_builder_new ();
	json_builder_begin_object (builder);
	json_builder_set_member_name (builder, "email");
	json_builder_add_string_value (builder, username);
	json_builder_set_member_name (builder, "password");
	json_builder_add_string_value (builder, password);
	json_builder_end_object (builder);

	generator = json_generator_new ();
	node = json_builder_get_root (builder);
	json_generator_set_root (generator, node);
	data = json_generator_to_data (generator, &length);

	query = g_strdup_printf("%s/api/me", server);

	session = soup_session_new ();
	msg = soup_message_new (SOUP_METHOD_POST, query);
	soup_message_headers_append (msg->request_headers, "Accept", "application/json");
	soup_message_set_request (msg, "application/json", SOUP_MEMORY_COPY, data, length);
	soup_session_queue_message (session, msg,
	                            pragha_koel_get_auth_done, plugin);

	g_object_unref (generator);
	g_object_unref (builder);
	json_node_free (node);
	g_free (query);
	g_free (request);
	g_free (data);
}

static void
pragha_koel_plugin_deauthenticate (PraghaKoelPlugin *plugin)
{
	PraghaKoelPluginPrivate *priv = plugin->priv;

	if (priv->server) {
		g_free (priv->server);
		priv->server = NULL;
	}
	if (priv->token) {
		g_free (priv->token);
		priv->token = NULL;
	}

	priv->upgrade = FALSE;
}

static void
pragha_koel_plugin_set_need_upgrade (PraghaKoelPlugin *plugin, gboolean upgrade)
{
	PraghaKoelPluginPrivate *priv = plugin->priv;

	priv->upgrade = upgrade;
}

static gboolean
pragha_koel_plugin_need_upgrade (PraghaKoelPlugin *plugin)
{
	PraghaKoelPluginPrivate *priv = plugin->priv;

	return priv->upgrade;
}


/*
 * Gstreamer source
 */

static gboolean
pragha_musicobject_is_koel_file (PraghaMusicobject *mobj)
{
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicSource file_source = FILE_NONE;

	enum_map = pragha_music_enum_get ();
	file_source = pragha_music_enum_map_get(enum_map, "KOEL");
	g_object_unref (enum_map);

	return (file_source == pragha_musicobject_get_source (mobj));
}

static void
pragha_koel_plugin_prepare_source (PraghaBackend       *backend,
                                      PraghaKoelPlugin *plugin)
{
	PraghaMusicobject *mobj;
	const gchar *location = NULL;
	gchar *filename = NULL, *uri = NULL;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_koel_file (mobj))
		return;

	location =  pragha_musicobject_get_file (mobj);
	filename = pragha_song_cache_get_from_location (priv->cache, location);
	if (filename != NULL) {
		uri = g_filename_to_uri (filename, NULL, NULL);
		g_free (filename);
	}
	else {
		uri = g_strdup_printf ("%s/play?jwt-token=%s", location, priv->token);
	}
	pragha_backend_set_playback_uri (backend, uri);
	g_free (uri);
}

static void
pragha_koel_plugin_download_done (PraghaBackend       *backend,
                                     gchar               *filename,
                                     PraghaKoelPlugin *plugin)
{
	PraghaMusicobject *mobj;
	const gchar *location = NULL;

	PraghaKoelPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_koel_file (mobj))
		return;

	location = pragha_musicobject_get_file (mobj);
	pragha_song_cache_put_location (priv->cache, location, filename);
}

static void
pragha_koel_plugin_half_played (PraghaBackend    *backend,
                                PraghaKoelPlugin *plugin)
{
	PraghaMusicobject *mobj;
	const gchar *filename = NULL;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_koel_file (mobj))
		return;

	filename = pragha_musicobject_get_file (mobj);
	pragha_koel_plugin_increase_playcount (plugin, filename);
}

/*
 * Plugin.
 */

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	GMenuItem *item;
	GSimpleAction *action;

	PraghaKoelPlugin *plugin = PRAGHA_KOEL_PLUGIN (activatable);

	PraghaKoelPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "Koel Server plugin %s", G_STRFUNC);

	priv->cancellable = g_cancellable_new ();

	/* New cache */

	priv->cache = pragha_song_cache_get ();

	/* Favororites */

	priv->favorites = pragha_favorites_get ();

	/* Temp tables.*/

	priv->tracks_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	/* New Task widget */

	priv->task_widget = pragha_background_task_widget_new (_("Searching files to analyze"),
	                                                       "network-server",
	                                                       0,
	                                                       priv->cancellable);
	g_object_ref (G_OBJECT(priv->task_widget));

	/* Attach main menu */

	priv->action_group_main_menu = gtk_action_group_new ("PraghaKoelPlugin");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	/* Gear Menu */

	action = g_simple_action_new ("refresh-koel", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_koel_plugin_upgrade_database_gmenu_action), plugin);

	item = g_menu_item_new (_("Refresh the Koel library"), "win.refresh-koel");
	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);
	g_object_unref (item);

	/* Backend signals */

	backend = pragha_application_get_backend (priv->pragha);
	pragha_backend_set_local_storage (backend, TRUE);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_koel_plugin_prepare_source), plugin);
	g_signal_connect (backend, "download-done",
	                  G_CALLBACK(pragha_koel_plugin_download_done), plugin);
	g_signal_connect (backend, "half-played",
	                  G_CALLBACK(pragha_koel_plugin_half_played), plugin);

	/* Favorites handler */

	g_signal_connect (priv->favorites, "song-added",
	                  G_CALLBACK(pragha_koel_plugin_favorites_song_added), plugin);
	g_signal_connect (priv->favorites, "song-removed",
	                  G_CALLBACK(pragha_koel_plugin_favorites_song_removed), plugin);

	/* Append setting */

	pragha_koel_plugin_append_setting (plugin);

	/* Authenticate */

	pragha_koel_plugin_authenticate (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	PraghaDatabaseProvider *provider;
	PraghaPreferences *preferences;
	gchar *plugin_group = NULL;

	PraghaKoelPlugin *plugin = PRAGHA_KOEL_PLUGIN (activatable);
	PraghaKoelPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Koel Server plugin %s", G_STRFUNC);

	/* Cache */

	g_hash_table_destroy (priv->tracks_table);
	g_object_unref (priv->cache);

	/* Favorites */

	g_object_unref (priv->favorites);

	/* If user disable the plugin (Pragha not shutdown) */

	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha)))
	{
		/* Remove provider from gui. */

		if (priv->server) {
			provider = pragha_database_provider_get ();
			pragha_provider_remove (provider,
			                        priv->server);
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}

		/* Remove settings */

		preferences = pragha_application_get_preferences (priv->pragha);
		plugin_group = pragha_preferences_get_plugin_group_name (preferences, GROUP_KEY_KOEL);
		pragha_preferences_remove_group (preferences, plugin_group);
		g_free (plugin_group);
	}

	backend = pragha_application_get_backend (priv->pragha);
	pragha_backend_set_local_storage (backend, FALSE);
	g_signal_handlers_disconnect_by_func (backend, pragha_koel_plugin_prepare_source, plugin);

	/* Menu Action */

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	pragha_menubar_remove_action (priv->pragha, "pragha-plugins-placeholder", "refresh-koel");

	/* Close */

	pragha_koel_plugin_deauthenticate (plugin);

	/* Setting dialog widget */

	pragha_koel_plugin_remove_setting (plugin);
}
