/*************************************************************************/
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#include "pragha-song-info-dialog.h"

#include "src/pragha.h"
#include "src/pragha-hig.h"
#include "src/pragha-playback.h"
#include "src/pragha-simple-async.h"
#include "src/pragha-simple-widgets.h"
#include "src/pragha-utils.h"

#define PRAGHA_TYPE_SONG_INFO_PLUGIN         (pragha_song_info_plugin_get_type ())
#define PRAGHA_SONG_INFO_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_SONG_INFO_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_IS_SONG_INFO_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_IS_SONG_INFO_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_SONG_INFO_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPluginClass))

typedef struct {
	PraghaApplication *pragha;

	GlyrDatabase      *cache_db;

	gboolean           download_album_art;

	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;

	GtkActionGroup    *action_group_playlist;
	guint              merge_id_playlist;
} PraghaSongInfoPluginPrivate;

PRAGHA_PLUGIN_REGISTER_CONFIGURABLE (PRAGHA_TYPE_SONG_INFO_PLUGIN,
                                     PraghaSongInfoPlugin,
                                     pragha_song_info_plugin)

typedef struct
{
	PraghaApplication *pragha;
	GlyrQuery          query;
	GlyrMemCache      *head;
}
glyr_struct;

/*
 * Popups
 */

static void get_lyric_action       (GtkAction *action, PraghaSongInfoPlugin *plugin);
static void get_artist_info_action (GtkAction *action, PraghaSongInfoPlugin *plugin);

static const GtkActionEntry main_menu_actions [] = {
	{"Search lyric", NULL, N_("Search _lyric"),
	 "<Control>Y", "Search lyric", G_CALLBACK(get_lyric_action)},
	{"Search artist info", "dialog-information", N_("Search _artist info"),
	 "", "Search artist info", G_CALLBACK(get_artist_info_action)},
};

static const gchar *main_menu_xml = "<ui>						\
	<menubar name=\"Menubar\">							\
		<menu action=\"ToolsMenu\">						\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menuitem action=\"Search lyric\"/>			\
				<menuitem action=\"Search artist info\"/>		\
				<separator/>						\
			</placeholder>							\
		</menu>									\
	</menubar>										\
</ui>";

static void get_lyric_current_playlist_action       (GtkAction *action, PraghaSongInfoPlugin *plugin);
static void get_artist_info_current_playlist_action (GtkAction *action, PraghaSongInfoPlugin *plugin);

static const GtkActionEntry playlist_actions [] = {
	{"Search lyric", NULL, N_("Search _lyric"),
	 "", "Search lyric", G_CALLBACK(get_lyric_current_playlist_action)},
	{"Search artist info", "dialog-information", N_("Search _artist info"),
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

/* Save the downloaded album art in cache, and updates the gui.*/

static void
pragha_update_downloaded_album_art (glyr_struct *glyr_info)
{
	PraghaArtCache *art_cache;
	const gchar *artist = NULL, *album = NULL;

	if(glyr_info->head == NULL)
		return;

	PraghaApplication *pragha = glyr_info->pragha;

	artist = glyr_info->query.artist;
	album = glyr_info->query.album;

	art_cache = pragha_application_get_art_cache (pragha);

	if (glyr_info->head->data)
		pragha_art_cache_put (art_cache, artist, album, glyr_info->head->data, glyr_info->head->size);
}

/* Manages the results of glyr threads. */

static void
glyr_finished_successfully(glyr_struct *glyr_info)
{
	GtkWidget *window;
	gchar *title_header = NULL, *subtitle_header = NULL;

	PraghaApplication *pragha = glyr_info->pragha;

	switch (glyr_info->head->type) {
		case GLYR_TYPE_COVERART:
			pragha_update_downloaded_album_art(glyr_info);
			break;
		case GLYR_TYPE_LYRICS:
			title_header =  g_strdup_printf(_("Lyrics thanks to %s"), glyr_info->head->prov);
			subtitle_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), glyr_info->query.title, glyr_info->query.artist);
			window = pragha_application_get_window (pragha);

			pragha_show_related_text_info_dialog (window, title_header, subtitle_header, glyr_info->head->data);
			break;
		case GLYR_TYPE_ARTIST_BIO:
			title_header =  g_strdup_printf(_("Artist info"));
			subtitle_header = g_strdup_printf(_("%s <small><span weight=\"light\">thanks to</span></small> %s"), glyr_info->query.artist, glyr_info->head->prov);
			window = pragha_application_get_window (pragha);

			pragha_show_related_text_info_dialog (window, title_header, subtitle_header, glyr_info->head->data);
			break;
		default:
			break;
	}

	g_free(title_header);
	g_free(subtitle_header);

	glyr_free_list(glyr_info->head);
}

static void
glyr_finished_incorrectly(glyr_struct *glyr_info)
{
	PraghaStatusbar *statusbar = pragha_statusbar_get ();

	switch (glyr_info->query.type) {
		case GLYR_GET_LYRICS:
			pragha_statusbar_set_misc_text (statusbar, _("Lyrics not found."));
			break;
		case GLYR_GET_ARTIST_BIO:
			pragha_statusbar_set_misc_text (statusbar, _("Artist information not found."));
			break;
		case GLYR_GET_COVERART:
		default:
			break;
	}
	g_object_unref (statusbar);
}

static gboolean
glyr_finished_thread_update (gpointer data)
{
	GtkWidget *window;
	glyr_struct *glyr_info = data;

	window = pragha_application_get_window (glyr_info->pragha);
	remove_watch_cursor (window);

	if(glyr_info->head != NULL)
		glyr_finished_successfully (glyr_info);
	else
		glyr_finished_incorrectly (glyr_info);

	glyr_query_destroy (&glyr_info->query);
	g_slice_free (glyr_struct, glyr_info);

	return FALSE;
}

/* Get artist bio or lyric on a thread. */

static gpointer
get_related_info_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	head = glyr_get (&glyr_info->query, &error, NULL);

	glyr_info->head = head;

	return glyr_info;
}

/* Configure the thrad to get the artist bio or lyric. */

static void
configure_and_launch_get_text_info_dialog (GLYR_GET_TYPE        type,
                                           const gchar          *artist,
                                           const gchar          *title,
                                           PraghaSongInfoPlugin *plugin)
{
	GtkWidget *window;
	glyr_struct *glyr_info;

	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init (&glyr_info->query);
	glyr_opt_type (&glyr_info->query, type);

	switch (type) {
		case GLYR_GET_ARTIST_BIO:
			glyr_opt_artist(&glyr_info->query, artist);

			glyr_opt_lang (&glyr_info->query, "auto");
			glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
			break;
		case GLYR_GET_LYRICS:
			glyr_opt_artist(&glyr_info->query, artist);
			glyr_opt_title(&glyr_info->query, title);
			break;
		default:
			break;
	}

	glyr_opt_lookup_db (&glyr_info->query, priv->cache_db);
	glyr_opt_db_autowrite (&glyr_info->query, TRUE);

	glyr_info->pragha = plugin->priv->pragha;

	window = pragha_application_get_window (glyr_info->pragha);
	set_watch_cursor (window);

	pragha_async_launch (get_related_info_idle_func,
	                     glyr_finished_thread_update,
	                     glyr_info);
}

static void
get_artist_info_action (GtkAction *action, PraghaSongInfoPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaMusicobject *mobj = NULL;
	const gchar *artist = NULL;
	PraghaApplication *pragha = NULL;

	pragha = plugin->priv->pragha;
	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	CDEBUG(DBG_INFO, "Get Artist info Action");

	mobj = pragha_backend_get_musicobject (backend);
	artist = pragha_musicobject_get_artist (mobj);

	if (string_is_empty(artist))
		return;

	configure_and_launch_get_text_info_dialog (GLYR_GET_ARTISTBIO, artist, NULL, plugin);
}

static void
get_lyric_action (GtkAction *action, PraghaSongInfoPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaMusicobject *mobj = NULL;
	const gchar *artist = NULL;
	const gchar *title = NULL;

	PraghaApplication *pragha = NULL;
	pragha = plugin->priv->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	mobj = pragha_backend_get_musicobject (backend);
	artist = pragha_musicobject_get_artist (mobj);
	title = pragha_musicobject_get_title (mobj);

	if (string_is_empty(artist) || string_is_empty(title))
		return;

	configure_and_launch_get_text_info_dialog (GLYR_GET_LYRICS, artist, title, plugin);
}

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

	configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, plugin);
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

	configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, plugin);
}

static void
related_get_album_art_handler (PraghaSongInfoPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaArtCache *art_cache;
	glyr_struct *glyr_info;
	PraghaMusicobject *mobj;
	const gchar *artist = NULL;
	const gchar *album = NULL;
	gchar *album_art_path;

	CDEBUG(DBG_INFO, "Get album art handler");

	PraghaApplication *pragha = NULL;
	pragha = plugin->priv->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	mobj = pragha_backend_get_musicobject (backend);
	artist = pragha_musicobject_get_artist (mobj);
	album = pragha_musicobject_get_album (mobj);

	if (string_is_empty(artist) || string_is_empty(album))
		return;

	art_cache = pragha_application_get_art_cache (pragha);
	album_art_path = pragha_art_cache_get_uri (art_cache, artist, album);

	if (album_art_path)
		goto exists;

	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init(&glyr_info->query);

	glyr_opt_type (&glyr_info->query, GLYR_GET_COVERART);
	glyr_opt_from (&glyr_info->query, "lastfm;musicbrainz");

	glyr_opt_artist (&glyr_info->query, artist);
	glyr_opt_album (&glyr_info->query, album);

	glyr_info->pragha = pragha;

	pragha_async_launch (get_related_info_idle_func, glyr_finished_thread_update, glyr_info);

exists:
	g_free(album_art_path);
}

static void
backend_changed_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	PraghaMusicType file_type = FILE_NONE;
	PraghaBackendState state = 0;
	gboolean playing = FALSE;
	GtkAction *action;

	PraghaSongInfoPlugin *plugin = user_data;
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	state = pragha_backend_get_state (backend);
	playing = (state != ST_STOPPED);

	action = gtk_action_group_get_action (priv->action_group_main_menu, "Search lyric");
	gtk_action_set_sensitive (action, playing);

	action = gtk_action_group_get_action (priv->action_group_main_menu, "Search artist info");
	gtk_action_set_sensitive (action, playing);

	CDEBUG(DBG_INFO, "Configuring thread to get the cover art");

	if (state != ST_PLAYING)
		return;

	file_type = pragha_musicobject_get_file_type (pragha_backend_get_musicobject (backend));

	if (file_type == FILE_NONE || file_type == FILE_HTTP)
		return;

	if (priv->download_album_art)
		related_get_album_art_handler (plugin);
}

static void
pragha_song_info_prefrenceces_event (PraghaPreferences *preferences, const gchar *key, PraghaSongInfoPlugin *plugin)
{
	PraghaSongInfoPluginPrivate *priv = NULL;
	gchar *plugin_group = NULL;

	if (g_strcmp0(key, "DownloadAlbumArt") == 0) {
		priv = plugin->priv;

		plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");
		priv->download_album_art = pragha_preferences_get_boolean (preferences, plugin_group, "DownloadAlbumArt");
		g_free (plugin_group);
	}
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaPreferences *preferences;
	PraghaPlaylist *playlist;
	gchar *cache_folder = NULL, *plugin_group = NULL;
	GtkAction *action;

	PraghaSongInfoPlugin *plugin = PRAGHA_SONG_INFO_PLUGIN (activatable);
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	glyr_init ();

	cache_folder = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (), "pragha", NULL);

	g_mkdir_with_parents (cache_folder, S_IRWXU);
	priv->cache_db = glyr_db_init (cache_folder);
	g_free (cache_folder);

	/* Attach main menu */
	priv->action_group_main_menu = gtk_action_group_new ("PraghaGlyrMainMenuActions");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	action = gtk_action_group_get_action (priv->action_group_main_menu, "Search lyric");
	gtk_action_set_sensitive (action, FALSE);

	action = gtk_action_group_get_action (priv->action_group_main_menu, "Search artist info");
	gtk_action_set_sensitive (action, FALSE);

	/* Attach Playlist popup menu*/
	priv->action_group_playlist = gtk_action_group_new ("PraghaGlyrPlaylistActions");
	gtk_action_group_set_translation_domain (priv->action_group_playlist, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_playlist,
	                              playlist_actions,
	                              G_N_ELEMENTS (playlist_actions),
	                              plugin);

	playlist = pragha_application_get_playlist (priv->pragha);
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                priv->action_group_playlist,
	                                                                playlist_xml);

	g_signal_connect (pragha_application_get_backend (priv->pragha), "notify::state",
	                  G_CALLBACK (backend_changed_state_cb), plugin);

	preferences = pragha_application_get_preferences (priv->pragha);

	g_signal_connect (G_OBJECT(preferences), "PluginsChanged",
	                  G_CALLBACK(pragha_song_info_prefrenceces_event), plugin);

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");
	priv->download_album_art = pragha_preferences_get_boolean (preferences, plugin_group, "DownloadAlbumArt");
	g_free (plugin_group);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaApplication *pragha = NULL;
	PraghaPreferences *preferences;
	PraghaPlaylist *playlist;
	gchar *plugin_group = NULL;

	PraghaSongInfoPlugin *plugin = PRAGHA_SONG_INFO_PLUGIN (activatable);
	PraghaSongInfoPluginPrivate *priv = plugin->priv;

	pragha = plugin->priv->pragha;

	g_debug ("%s", G_STRFUNC);

	g_signal_handlers_disconnect_by_func (pragha_application_get_backend (pragha),
	                                      backend_changed_state_cb, plugin);

	pragha_menubar_remove_plugin_action (pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);

	priv->merge_id_playlist = 0;

	preferences = pragha_application_get_preferences (pragha);

	g_signal_handlers_disconnect_by_func (preferences,
	                                      pragha_song_info_prefrenceces_event,
	                                      plugin);

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "song-info");
	pragha_preferences_set_boolean (preferences, plugin_group, "DownloadAlbumArt", priv->download_album_art);
	g_free (plugin_group);

	glyr_db_destroy (priv->cache_db);

	glyr_cleanup ();

	priv->pragha= NULL;
}

static void
toggle_download_album_art (GtkToggleButton *button)
{
	PraghaPreferences *preferences = NULL;
	gchar *plugin_group = NULL;

	preferences = pragha_preferences_get ();

	plugin_group = pragha_preferences_get_plugin_group_name(preferences, "song-info");
	pragha_preferences_set_boolean (preferences,
	                                plugin_group, "DownloadAlbumArt",
	                                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
	pragha_preferences_plugin_changed (preferences, "DownloadAlbumArt");

	g_object_unref (G_OBJECT (preferences));
	g_free (plugin_group);
}

static GtkWidget *
pragha_plugin_create_configure_widget (PeasGtkConfigurable *configurable)
{
	PraghaPreferences *preferences = NULL;
	gchar *plugin_group = NULL;
	GtkWidget *table, *download_album_art;
	guint row = 0;

	table = pragha_hig_workarea_table_new ();

	download_album_art = gtk_check_button_new_with_label (_("Download the album art while playing their songs."));
	pragha_hig_workarea_table_add_wide_control (table, &row, download_album_art);

	preferences = pragha_preferences_get ();

	plugin_group = pragha_preferences_get_plugin_group_name(preferences, "song-info");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(download_album_art),
		pragha_preferences_get_boolean (preferences, plugin_group, "DownloadAlbumArt"));

	g_object_unref (G_OBJECT (preferences));
	g_free (plugin_group);

	g_signal_connect (G_OBJECT(download_album_art), "toggled",
	                  G_CALLBACK(toggle_download_album_art), NULL);

	return table;
}
