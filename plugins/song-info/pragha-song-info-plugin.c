/*************************************************************************/
/* Copyright (C) 2011-2019 matias <mati86dl@gmail.com>                   */
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

#include <glyr/glyr.h>
#include <glyr/cache.h>

#include <glib/gstdio.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "plugins/pragha-plugin-macros.h"

#include "pragha-song-info-plugin.h"
#include "pragha-song-info-dialog.h"
#include "pragha-song-info-pane.h"
#include "pragha-song-info-thread-albumart.h"
#include "pragha-song-info-thread-dialog.h"
#include "pragha-song-info-thread-pane.h"

#include "src/pragha.h"
#include "src/pragha-hig.h"
#include "src/pragha-playlist.h"
#include "src/pragha-playback.h"
#include "src/pragha-sidebar.h"
#include "src/pragha-simple-async.h"
#include "src/pragha-simple-widgets.h"
#include "src/pragha-preferences-dialog.h"
#include "src/pragha-utils.h"

struct _PraghaSongInfoPluginPrivate {
	PraghaApplication  *pragha;
	GtkWidget          *setting_widget;

	PraghaSonginfoPane *pane;

	PraghaInfoCache    *cache_info;

	gboolean            download_album_art;
	GtkWidget          *download_album_art_w;
	GtkWidget          *proxy_w;

	GtkActionGroup     *action_group_playlist;
	guint               merge_id_playlist;

	GCancellable       *pane_search;
};

PRAGHA_PLUGIN_REGISTER_PRIVATE_CODE (PRAGHA_TYPE_SONG_INFO_PLUGIN,
                                     PraghaSongInfoPlugin,
                                     pragha_song_info_plugin)

/*
 * Popups
 */

static void get_lyric_current_playlist_action       (GtkAction *action, PraghaSongInfoPlugin *plugin);
static void get_artist_info_current_playlist_action (GtkAction *action, PraghaSongInfoPlugin *plugin);

static const GtkActionEntry playlist_actions [] = {
	{"Search lyric", NULL, N_("Search _lyric"),
	 "", "Search lyric", G_CALLBACK(get_lyric_current_playlist_action)},
	{"Search artist info", NULL, N_("Search _artist info"),
	 "", "Search artist info", G_CALLBACK(get_artist_info_current_playlist_action)},
};

static const gchar *playlist_xml = "<ui>						\
	<popup name=\"SelectionPopup\">		   				\
	<menu action=\"ToolsMenu\">							\
		<placeholder name=\"pragha-glyr-placeholder\">			\
			<menuitem action=\"Search lyric\"/>				\
			<menuitem action=\"Search artist info\"/>			\
			<separator/>							\
		</placeholder>								\
	</menu>										\
	</popup>				    						\
</ui>";

/*
 * Action on playlist that show a dialog
 */

static void
get_artist_info_current_playlist_action (GtkAction *action, PraghaSongInfoPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj;
	const gchar *artist = NULL;

	PraghaApplication *pragha = NULL;

	pragha = plugin->priv->pragha;
	playlist = pragha_application_get_playlist (pragha);

	mobj = pragha_playlist_get_selected_musicobject (playlist);

	artist = pragha_musicobject_get_artist (mobj);

	CDEBUG(DBG_INFO, "Get Artist info Action of current playlist selection");

	if (string_is_empty(artist))
		return;

	pragha_songinfo_plugin_get_info_to_dialog (plugin, GLYR_GET_ARTISTBIO, artist, NULL);
}

static void
get_lyric_current_playlist_action (GtkAction *action, PraghaSongInfoPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj;
	const gchar *artist = NULL;
	const gchar *title = NULL;

	PraghaApplication *pragha = NULL;
	pragha = plugin->priv->pragha;

	playlist = pragha_application_get_playlist (pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	artist = pragha_musicobject_get_artist (mobj);
	title = pragha_musicobject_get_title (mobj);

	CDEBUG(DBG_INFO, "Get lyrics Action of current playlist selection.");

	if (string_is_empty(artist) || string_is_empty(title))
		return;

	pragha_songinfo_plugin_get_info_to_dialog (plugin, GLYR_GET_LYRICS, artist, title);
}

/*
 * Handlers depending on backend status
 */

static void
related_get_album_art_handler (PraghaSongInfoPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaArtCache *art_cache;
	PraghaMusicobject *mobj;
	const gchar *artist = NULL;
	const gchar *album = NULL;
	gchar *album_art_path = NULL;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_INFO, "Get album art handler");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	mobj = pragha_backend_get_musicobject (backend);
	artist = pragha_musicobject_get_artist (mobj);
	album = pragha_musicobject_get_album (mobj);

	if (string_is_empty(artist) || string_is_empty(album))
		return;

	art_cache = pragha_application_get_art_cache (priv->pragha);
	album_art_path = pragha_art_cache_get_album_uri (art_cache, artist, album);
	if (album_art_path)
		goto exists;

	pragha_songinfo_plugin_get_album_art (plugin, artist, album);

exists:
	g_free (album_art_path);
}

static void
pragha_song_info_cancel_pane_search (PraghaSongInfoPlugin *plugin)
{
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	if (priv->pane_search) {
		g_cancellable_cancel (priv->pane_search);
		g_object_unref (priv->pane_search);
		priv->pane_search = NULL;
	}
}

static void
related_get_song_info_pane_handler (PraghaSongInfoPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaMusicobject *mobj;
	GList *list = NULL, *l = NULL;
	const gchar *artist = NULL, *title = NULL, *filename = NULL;
	gchar *artist_bio = NULL, *lyrics = NULL, *provider = NULL;
	GLYR_GET_TYPE view_type = GLYR_GET_UNKNOWN;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	CDEBUG (DBG_INFO, "Get song info handler");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED) {
		pragha_songinfo_pane_clear_text (priv->pane);
		pragha_songinfo_pane_clear_list (priv->pane);
		return;
	}

	mobj = pragha_backend_get_musicobject (backend);
	artist = pragha_musicobject_get_artist (mobj);
	title = pragha_musicobject_get_title (mobj);
	filename = pragha_musicobject_get_file (mobj);

	if (string_is_empty(artist) || string_is_empty(title))
		return;

	pragha_song_info_cancel_pane_search (plugin);
	pragha_songinfo_pane_clear_list (priv->pane);

	view_type = pragha_songinfo_pane_get_default_view (priv->pane);
	switch (view_type) {
		case GLYR_GET_ARTIST_BIO:
			if (pragha_info_cache_contains_artist_bio (priv->cache_info, artist))
			{
				artist_bio = pragha_info_cache_get_artist_bio (priv->cache_info,
				                                               artist, &provider);

				pragha_songinfo_pane_set_title (priv->pane, artist);
				pragha_songinfo_pane_set_text (priv->pane, artist_bio, provider);
				g_free (artist_bio);
				g_free (provider);
				return;
			}
			break;
		case GLYR_GET_LYRICS:
			if (pragha_info_cache_contains_song_lyrics (priv->cache_info, title, artist))
			{
				lyrics = pragha_info_cache_get_song_lyrics (priv->cache_info,
				                                            title, artist,
				                                            &provider);
				pragha_songinfo_pane_set_title (priv->pane, title);
				pragha_songinfo_pane_set_text (priv->pane, lyrics, provider);
				g_free (lyrics);
				g_free (provider);
				return;
			}
			break;
		case GLYR_GET_SIMILAR_SONGS:
			if (pragha_info_cache_contains_similar_songs (priv->cache_info, title, artist))
			{
				list = pragha_info_cache_get_similar_songs (priv->cache_info,
				                                            title, artist,
				                                            &provider);
				for (l = list ; l != NULL ; l = l->next) {
					pragha_songinfo_pane_append_song_row (priv->pane,
						pragha_songinfo_pane_row_new ((PraghaMusicobject *)l->data));
				}
				pragha_songinfo_pane_set_title (priv->pane, title);
				pragha_songinfo_pane_set_text (priv->pane, "", provider);
				g_list_free (list);
				g_free (provider);
				return;
			}
			break;
		default:
			break;
	}
	priv->pane_search = pragha_songinfo_plugin_get_info_to_pane (plugin, view_type, artist, title, filename);
}

static void
pragha_song_info_get_info (gpointer data)
{
	PraghaSongInfoPlugin *plugin = data;
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	if (priv->download_album_art)
		related_get_album_art_handler (plugin);

	if (!gtk_widget_is_visible(GTK_WIDGET(priv->pane)))
		return;

	related_get_song_info_pane_handler (plugin);
}

static void
backend_changed_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	PraghaMusicSource file_source = FILE_NONE;
	PraghaBackendState state = ST_STOPPED;

	PraghaSongInfoPlugin *plugin = user_data;
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	pragha_song_info_cancel_pane_search (plugin);

	state = pragha_backend_get_state (backend);

	CDEBUG(DBG_INFO, "Configuring thread to get the cover art");

	if (state == ST_STOPPED) {
		pragha_songinfo_pane_clear_text (priv->pane);
		pragha_songinfo_pane_clear_list (priv->pane);
	}

	if (state != ST_PLAYING)
		return;

	file_source = pragha_musicobject_get_source (pragha_backend_get_musicobject (backend));

	if (file_source == FILE_NONE) {
		pragha_songinfo_pane_clear_text (priv->pane);
		pragha_songinfo_pane_clear_list (priv->pane);
		return;
	}

	pragha_song_info_get_info (plugin);
}

static void
pragha_songinfo_pane_append (PraghaSonginfoPane *pane,
                             PraghaMusicobject *mobj,
                             PraghaSongInfoPlugin *plugin)
{
	PraghaPlaylist *playlist;
	const gchar *provider = NULL, *title = NULL, *artist = NULL;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	provider = pragha_musicobject_get_provider (mobj);
	if (string_is_empty(provider))
	{
		open_url (pragha_musicobject_get_file(mobj), NULL);
	}
	else
	{
		title = pragha_musicobject_get_title (mobj);
		artist = pragha_musicobject_get_artist (mobj);
		playlist = pragha_application_get_playlist (priv->pragha);
		if (!pragha_playlist_already_has_title_of_artist (playlist, title, artist))
			pragha_playlist_append_single_song (playlist, g_object_ref(mobj));
		pragha_playlist_select_title_of_artist (playlist, title, artist);
	}
}

static void
pragha_songinfo_pane_queue (PraghaSonginfoPane *pane,
                            PraghaMusicobject *mobj,
                            PraghaSongInfoPlugin *plugin)
{
	PraghaPlaylist *playlist;
	const gchar *provider = NULL, *title = NULL, *artist = NULL;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	provider = pragha_musicobject_get_provider (mobj);
	if (string_is_empty(provider))
		return;

	title = pragha_musicobject_get_title (mobj);
	artist = pragha_musicobject_get_artist (mobj);
	playlist = pragha_application_get_playlist (priv->pragha);
	if (!pragha_playlist_already_has_title_of_artist (playlist, title, artist))
		pragha_playlist_append_single_song (playlist, g_object_ref(mobj));
	pragha_playlist_select_title_of_artist (playlist, title, artist);
	pragha_playlist_toggle_queue_selected (playlist);
}

static void
pragha_songinfo_pane_append_all (PraghaSonginfoPane   *pane,
                                 PraghaSongInfoPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj;
	const gchar *title = NULL, *artist = NULL;
	GList *mlist, *list;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	mlist = pragha_songinfo_get_mobj_list (priv->pane);
	list = mlist;
	while (list != NULL) {
		mobj = PRAGHA_MUSICOBJECT(list->data);
		title = pragha_musicobject_get_title (mobj);
		artist = pragha_musicobject_get_artist (mobj);
		playlist = pragha_application_get_playlist (priv->pragha);
		if (!pragha_playlist_already_has_title_of_artist (playlist, title, artist))
			pragha_playlist_append_single_song (playlist, g_object_ref(mobj));
		list = g_list_next(list);
	}
	g_list_free (mlist);
}

static gchar *
pragha_songinfo_plugin_get_proxy (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        "Proxy");

	g_free (plugin_group);

	return string;
}

/*
 * Update handlers
 */

static void
pragha_songinfo_pane_type_changed (PraghaSonginfoPane *pane, PraghaSongInfoPlugin *plugin)
{
	related_get_song_info_pane_handler (plugin);
}

static void
pragha_songinfo_pane_visibility_changed (PraghaPreferences *preferences, GParamSpec *pspec, PraghaSongInfoPlugin *plugin)
{
	if (pragha_preferences_get_secondary_lateral_panel (preferences))
		related_get_song_info_pane_handler (plugin);
}

/*
 * Public api
 */

PraghaInfoCache *
pragha_songinfo_plugin_get_cache_info (PraghaSongInfoPlugin *plugin)
{
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	return priv->cache_info;
}

PraghaSonginfoPane *
pragha_songinfo_plugin_get_pane (PraghaSongInfoPlugin *plugin)
{
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	return priv->pane;
}

void
pragha_songinfo_plugin_init_glyr_query (gpointer data)
{
	PraghaPreferences *preferences;
	gchar *proxy = NULL;
	GlyrQuery *query = data;

	preferences = pragha_preferences_get ();
	proxy = pragha_songinfo_plugin_get_proxy (preferences);

	glyr_query_init (query);
	glyr_opt_proxy (query, proxy);

	g_object_unref (preferences);
	g_free (proxy);
}

PraghaApplication *
pragha_songinfo_plugin_get_application (PraghaSongInfoPlugin *plugin)
{
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	return priv->pragha;
}

/*
 * Preferences plugin
 */

static void
pragha_songinfo_preferences_dialog_response (GtkDialog            *dialog,
                                             gint                  response_id,
                                             PraghaSongInfoPlugin *plugin)
{
	PraghaPreferences *preferences;
	const gchar *entry_proxy = NULL;
	gchar *plugin_group = NULL, *test_proxy = NULL;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");

	test_proxy = pragha_songinfo_plugin_get_proxy (preferences);

	switch(response_id) {
		case GTK_RESPONSE_CANCEL:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->download_album_art_w),
			                              priv->download_album_art);
			pragha_gtk_entry_set_text (GTK_ENTRY(priv->proxy_w), test_proxy);
			break;
		case GTK_RESPONSE_OK:
			priv->download_album_art =
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->download_album_art_w));

			pragha_preferences_set_boolean (preferences,
			                                plugin_group, "DownloadAlbumArt",
			                                priv->download_album_art);

			entry_proxy = gtk_entry_get_text (GTK_ENTRY(priv->proxy_w));

			if (g_strcmp0 (test_proxy, entry_proxy))
				pragha_preferences_set_string (preferences, plugin_group, "Proxy", entry_proxy);

			break;
		default:
			break;
	}

	g_object_unref (preferences);
	g_free (plugin_group);
	g_free (test_proxy);
}

static void
pragha_songinfo_plugin_append_setting (PraghaSongInfoPlugin *plugin)
{
	PraghaPreferencesDialog *dialog;
	PraghaPreferences *preferences = NULL;
	gchar *plugin_group = NULL, *proxy = NULL;
	GtkWidget *table, *download_album_art_w, *proxy_label, *proxy_w;
	guint row = 0;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Song Information"));

	download_album_art_w = gtk_check_button_new_with_label (_("Download the album art while playing their songs."));
	pragha_hig_workarea_table_add_wide_control (table, &row, download_album_art_w);

	preferences = pragha_preferences_get ();
	proxy_label = gtk_label_new (_("Proxy"));
	proxy_w = gtk_entry_new ();
	proxy = pragha_songinfo_plugin_get_proxy (preferences);
	if (proxy)
		gtk_entry_set_text (GTK_ENTRY(proxy_w), proxy);
	gtk_entry_set_placeholder_text (GTK_ENTRY(proxy_w), "[protocol://][user:pass@]yourproxy.domain[:port]");
	gtk_entry_set_activates_default (GTK_ENTRY(proxy_w), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, proxy_label, proxy_w);

	plugin_group = pragha_preferences_get_plugin_group_name(preferences, "song-info");

	priv->download_album_art =
		pragha_preferences_get_boolean (preferences, plugin_group, "DownloadAlbumArt");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(download_album_art_w),
	                              priv->download_album_art);

	priv->setting_widget = table;
	priv->download_album_art_w = download_album_art_w;
	priv->proxy_w = proxy_w;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_append_services_setting (dialog, table, FALSE);

	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_songinfo_preferences_dialog_response),
	                                           plugin);

	g_object_unref (G_OBJECT (preferences));
	g_free (plugin_group);
	g_free (proxy);
}

static void
pragha_songinfo_plugin_remove_setting (PraghaSongInfoPlugin *plugin)
{
	PraghaPreferencesDialog *dialog;
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_songinfo_preferences_dialog_response),
	                                              plugin);

	pragha_preferences_remove_services_setting (dialog, priv->setting_widget);
}

/*
 * Plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaPreferences *preferences;
	PraghaPlaylist *playlist;
	PraghaSidebar *sidebar;
	GLYR_GET_TYPE view_type = GLYR_GET_LYRICS;
	gchar *plugin_group = NULL;

	PraghaSongInfoPlugin *plugin = PRAGHA_SONG_INFO_PLUGIN (activatable);
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Song-info plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	glyr_init ();

	priv->cache_info = pragha_info_cache_get();

	/* Attach Playlist popup menu*/

	priv->action_group_playlist = pragha_menubar_plugin_action_new ("PraghaGlyrPlaylistActions",
	                                                                playlist_actions,
	                                                                G_N_ELEMENTS (playlist_actions),
	                                                                NULL,
	                                                                0,
	                                                                plugin);

	playlist = pragha_application_get_playlist (priv->pragha);
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                priv->action_group_playlist,
	                                                                playlist_xml);

	/* Create the pane and attach it */
	priv->pane = pragha_songinfo_pane_new ();
	sidebar = pragha_application_get_second_sidebar (priv->pragha);
	pragha_sidebar_attach_plugin (sidebar,
		                          GTK_WIDGET (priv->pane),
		                          pragha_songinfo_pane_get_pane_title (priv->pane),
		                          pragha_songinfo_pane_get_popover (priv->pane));

	/* Connect signals */

	g_signal_connect (pragha_application_get_backend (priv->pragha), "notify::state",
	                  G_CALLBACK (backend_changed_state_cb), plugin);
	backend_changed_state_cb (pragha_application_get_backend (priv->pragha), NULL, plugin);

	preferences = pragha_application_get_preferences (priv->pragha);

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");
	view_type = pragha_preferences_get_integer (preferences, plugin_group, "default-view");
	pragha_songinfo_pane_set_default_view (priv->pane, view_type);
	g_free (plugin_group);

	g_signal_connect (G_OBJECT(preferences), "notify::secondary-lateral-panel",
	                  G_CALLBACK(pragha_songinfo_pane_visibility_changed), plugin);

	g_signal_connect (G_OBJECT(priv->pane), "type-changed",
	                  G_CALLBACK(pragha_songinfo_pane_type_changed), plugin);
	g_signal_connect (G_OBJECT(priv->pane), "append",
	                  G_CALLBACK(pragha_songinfo_pane_append), plugin);
	g_signal_connect (G_OBJECT(priv->pane), "append-all",
	                  G_CALLBACK(pragha_songinfo_pane_append_all), plugin);
	g_signal_connect (G_OBJECT(priv->pane), "queue",
	                  G_CALLBACK(pragha_songinfo_pane_queue), plugin);

	/* Default values */

	pragha_songinfo_plugin_append_setting (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaApplication *pragha = NULL;
	PraghaPreferences *preferences;
	PraghaPlaylist *playlist;
	PraghaSidebar *sidebar;
	gchar *plugin_group = NULL;

	PraghaSongInfoPlugin *plugin = PRAGHA_SONG_INFO_PLUGIN (activatable);
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	pragha = plugin->priv->pragha;

	CDEBUG(DBG_PLUGIN, "SongInfo plugin %s", G_STRFUNC);

	g_signal_handlers_disconnect_by_func (pragha_application_get_backend (pragha),
	                                      backend_changed_state_cb, plugin);

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);

	priv->merge_id_playlist = 0;

	preferences = pragha_application_get_preferences (pragha);

	g_signal_handlers_disconnect_by_func (G_OBJECT(preferences),
	                                      pragha_songinfo_pane_visibility_changed,
	                                      plugin);

	g_signal_handlers_disconnect_by_func (G_OBJECT(preferences),
	                                      pragha_songinfo_pane_type_changed,
	                                      plugin);

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");
	pragha_preferences_set_integer (preferences, plugin_group, "default-view",
			pragha_songinfo_pane_get_default_view (priv->pane));

	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha))) {
		pragha_preferences_remove_group (preferences, plugin_group);
	}
	g_free (plugin_group);

	sidebar = pragha_application_get_second_sidebar (priv->pragha);
	pragha_sidebar_remove_plugin (sidebar, GTK_WIDGET(priv->pane));

	pragha_songinfo_plugin_remove_setting (plugin);

	g_object_unref (priv->cache_info);

	glyr_cleanup ();

	priv->pragha = NULL;
}
