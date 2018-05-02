/*************************************************************************/
/* Copyright (C) 2011-2017 matias <mati86dl@gmail.com>                   */
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

#include <libpeas/peas.h>

#include "src/pragha.h"
#include "src/pragha-hig.h"
#include "src/pragha-utils.h"
#include "src/pragha-menubar.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-plugins-engine.h"
#include "src/pragha-statusicon.h"
#include "src/pragha-tagger.h"
#include "src/pragha-simple-async.h"
#include "src/pragha-utils.h"
#include "src/pragha-tags-dialog.h"
#include "src/pragha-tags-mgmt.h"
#include "src/pragha-window.h"
#include "src/xml_helper.h"

#include "pragha-lastfm-menu-ui.h"

#include "plugins/pragha-plugin-macros.h"

#include <clastfm.h>

#define PRAGHA_TYPE_LASTFM_PLUGIN         (pragha_lastfm_plugin_get_type ())
#define PRAGHA_LASTFM_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_LASTFM_PLUGIN, PraghaLastfmPlugin))
#define PRAGHA_LASTFM_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_LASTFM_PLUGIN, PraghaLastfmPlugin))
#define PRAGHA_IS_LASTFM_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_LASTFM_PLUGIN))
#define PRAGHA_IS_LASTFM_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_LASTFM_PLUGIN))
#define PRAGHA_LASTFM_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_LASTFM_PLUGIN, PraghaLastfmPluginClass))

struct _PraghaLastfmPluginPrivate {
	PraghaApplication        *pragha;

	/* Last session status. */
	LASTFM_SESSION           *session_id;
	enum LASTFM_STATUS_CODES  status;
	gboolean                  has_user;
	gboolean                  has_pass;

	/* Settings widgets */
	GtkWidget                *setting_widget;
	GtkWidget                *enable_w;
	GtkWidget                *lastfm_uname_w;
	GtkWidget                *lastfm_pass_w;

	GtkWidget                *ntag_lastfm_button;

	/* Song status */
	GMutex                    data_mutex;
	time_t                    playback_started;
	PraghaMusicobject        *current_mobj;
	PraghaMusicobject        *updated_mobj;

	/* Menu options */
	GtkActionGroup           *action_group_main_menu;
	guint                     merge_id_main_menu;

	GtkActionGroup           *action_group_playlist;
	guint                     merge_id_playlist;

	guint                     update_timeout_id;
	guint                     scrobble_timeout_id;
};
typedef struct _PraghaLastfmPluginPrivate PraghaLastfmPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_LASTFM_PLUGIN,
                        PraghaLastfmPlugin,
                        pragha_lastfm_plugin)

/*
 * Some useful definitions
 */
#define LASTFM_API_KEY "ecdc2d21dbfe1139b1f0da35daca9309"
#define LASTFM_SECRET  "f3498ce387f30eeae8ea1b1023afb32b"

#define KEY_LASTFM_SCROBBLE "scrobble"
#define KEY_LASTFM_USER     "lastfm_user"
#define KEY_LASTFM_PASS     "lastfm_pass"

#define WAIT_UPDATE 5

typedef enum {
	LASTFM_NONE = 0,
	LASTFM_GET_SIMILAR,
	LASTFM_GET_LOVED
} LastfmQueryType;

/*
 * Some structs to handle threads.
 */

typedef struct {
	GList              *list;
	LastfmQueryType     query_type;
	guint               query_count;
	PraghaLastfmPlugin *plugin;
} AddMusicObjectListData;

typedef struct {
	PraghaLastfmPlugin *plugin;
	PraghaMusicobject  *mobj;
} PraghaLastfmAsyncData;

static PraghaLastfmAsyncData *
pragha_lastfm_async_data_new (PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaLastfmAsyncData *data;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	backend = pragha_application_get_backend (priv->pragha);

	data = g_slice_new (PraghaLastfmAsyncData);
	data->plugin = plugin;
	data->mobj = pragha_musicobject_dup (pragha_backend_get_musicobject (backend));

	return data;
}

static void
pragha_lastfm_async_data_free (PraghaLastfmAsyncData *data)
{
	g_object_unref (data->mobj);
	g_slice_free (PraghaLastfmAsyncData, data);
}

/*
 * Menubar Prototypes
 */

static void lastfm_add_favorites_action                 (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_get_similar_action                   (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_import_xspf_action                   (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_track_love_action                    (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_track_unlove_action                  (GtkAction *action, PraghaLastfmPlugin *plugin);

static const GtkActionEntry main_menu_actions [] = {
	{"Lastfm", NULL, N_("_Lastfm")},
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", G_CALLBACK(lastfm_track_love_action)},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", G_CALLBACK(lastfm_track_unlove_action)},
	{"Import a XSPF playlist", NULL, N_("Import a XSPF playlist"),
	 "", "Import a XSPF playlist", G_CALLBACK(lastfm_import_xspf_action)},
	{"Add favorites", NULL, N_("Add favorites"),
	 "", "Add favorites", G_CALLBACK(lastfm_add_favorites_action)},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", G_CALLBACK(lastfm_get_similar_action)},
};

static const gchar *main_menu_xml = "<ui>						\
	<menubar name=\"Menubar\">							\
		<menu action=\"ToolsMenu\">						\
			<placeholder name=\"pragha-plugins-placeholder\">		\
				<menu action=\"Lastfm\">				\
					<menuitem action=\"Love track\"/>		\
					<menuitem action=\"Unlove track\"/>		\
					<separator/>					\
					<menuitem action=\"Import a XSPF playlist\"/>	\
					<menuitem action=\"Add favorites\"/>		\
					<menuitem action=\"Add similar\"/>		\
				</menu>							\
				<separator/>						\
			</placeholder>							\
		</menu>									\
	</menubar>									\
</ui>";


/*
 * Playlist Prototypes.
 */

static void lastfm_get_similar_current_playlist_action  (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_track_current_playlist_love_action   (GtkAction *action, PraghaLastfmPlugin *plugin);
static void lastfm_track_current_playlist_unlove_action (GtkAction *action, PraghaLastfmPlugin *plugin);

static const GtkActionEntry playlist_actions [] = {
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", G_CALLBACK(lastfm_track_current_playlist_love_action)},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", G_CALLBACK(lastfm_track_current_playlist_unlove_action)},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", G_CALLBACK(lastfm_get_similar_current_playlist_action)},
};

static const gchar *playlist_xml = "<ui>						\
	<popup name=\"SelectionPopup\">		   					\
	<menu action=\"ToolsMenu\">							\
		<placeholder name=\"pragha-plugins-placeholder\">			\
			<menuitem action=\"Love track\"/>				\
			<menuitem action=\"Unlove track\"/>				\
			<separator/>							\
			<menuitem action=\"Add similar\"/>				\
			<separator/>							\
		</placeholder>								\
	</menu>										\
	</popup>				    					\
</ui>";

/*
 * Gear Menu Prototypes
 */
static void pragha_gmenu_lastfm_add_favorites_action    (GSimpleAction *action,
                                                         GVariant      *parameter,
                                                         gpointer       user_data);
static void pragha_gmenu_lastfm_get_similar_action      (GSimpleAction *action,
                                                         GVariant      *parameter,
                                                         gpointer       user_data);
static void pragha_gmenu_lastfm_import_xspf_action      (GSimpleAction *action,
                                                         GVariant      *parameter,
                                                         gpointer       user_data);
static void pragha_gmenu_lastfm_track_love_action       (GSimpleAction *action,
                                                         GVariant      *parameter,
                                                         gpointer       user_data);
static void pragha_gmenu_lastfm_track_unlove_action     (GSimpleAction *action,
                                                         GVariant      *parameter,
                                                         gpointer       user_data);

static GActionEntry lastfm_entries[] = {
	{ "lastfm-love",       pragha_gmenu_lastfm_track_love_action,    NULL, NULL, NULL },
	{ "lastfm-unlove",     pragha_gmenu_lastfm_track_unlove_action,  NULL, NULL, NULL },
	{ "lastfm-import",     pragha_gmenu_lastfm_import_xspf_action,   NULL, NULL, NULL },
	{ "lastfm-favorities", pragha_gmenu_lastfm_add_favorites_action, NULL, NULL, NULL },
	{ "lastfm-similar",    pragha_gmenu_lastfm_get_similar_action,   NULL, NULL, NULL }
};

/* Save a get the lastfm password.
 * TODO: Implement any basic crypto.
 */

static void
pragha_lastfm_plugin_set_password (PraghaPreferences *preferences, const gchar *pass)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	if (string_is_not_empty(pass))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_LASTFM_PASS,
		                               pass);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_LASTFM_PASS);

	g_free (plugin_group);
}

static gchar *
pragha_lastfm_plugin_get_password (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_LASTFM_PASS);

	g_free (plugin_group);

	return string;
}

static void
pragha_lastfm_plugin_set_user (PraghaPreferences *preferences, const gchar *user)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	if (string_is_not_empty(user))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_LASTFM_USER,
		                               user);
	else
 		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_LASTFM_USER);

	g_free (plugin_group);
}

static gchar *
pragha_lastfm_plugin_get_user (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *string = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	string = pragha_preferences_get_string (preferences,
	                                        plugin_group,
	                                        KEY_LASTFM_USER);

	g_free (plugin_group);

	return string;
}

static void
pragha_lastfm_plugin_set_scrobble_support (PraghaPreferences *preferences, gboolean supported)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	pragha_preferences_set_boolean (preferences,
		                            plugin_group,
		                            KEY_LASTFM_SCROBBLE,
		                            supported);

	g_free (plugin_group);
}

static gboolean
pragha_lastfm_plugin_get_scrobble_support (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL;
	gboolean scrobble = FALSE;

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");

	scrobble = pragha_preferences_get_boolean (preferences,
	                                           plugin_group,
	                                           KEY_LASTFM_SCROBBLE);

	g_free (plugin_group);

	return scrobble;
}


/* Upadate lastfm menubar acording lastfm state */

static void
pragha_action_group_set_sensitive (GtkActionGroup *group, const gchar *name, gboolean sensitive)
{
	GtkAction *action;
	action = gtk_action_group_get_action (group, name);
	gtk_action_set_sensitive (action, sensitive);
}

static void
pragha_lastfm_update_menu_actions (PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaBackendState state = ST_STOPPED;
	GtkWindow *window;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	backend = pragha_application_get_backend (priv->pragha);
	state = pragha_backend_get_state (backend);

	gboolean playing    = (state != ST_STOPPED);
	gboolean logged     = (priv->status == LASTFM_STATUS_OK);
	gboolean lfm_inited = (priv->session_id != NULL);
	gboolean has_user   = (lfm_inited && priv->has_user);

	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Love track", playing && logged);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Unlove track", playing && logged);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Add favorites", has_user);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Add similar", playing && lfm_inited);

	pragha_action_group_set_sensitive (priv->action_group_playlist, "Love track", logged);
	pragha_action_group_set_sensitive (priv->action_group_playlist, "Unlove track", logged);
	pragha_action_group_set_sensitive (priv->action_group_playlist, "Add similar", lfm_inited);

	window = GTK_WINDOW(pragha_application_get_window(priv->pragha));
	pragha_menubar_set_enable_action (window, "lastfm-love", playing && logged);
	pragha_menubar_set_enable_action (window, "lastfm-unlove", playing && logged);
	pragha_menubar_set_enable_action (window, "lastfm-favorities", has_user);
	pragha_menubar_set_enable_action (window, "lastfm-similar", playing && lfm_inited);
}

/*
 * Advise not connect with lastfm.
 */
static void pragha_lastfm_no_connection_advice (void)
{
	PraghaStatusbar *statusbar = pragha_statusbar_get ();
	pragha_statusbar_set_misc_text (statusbar, _("Unable to establish conection with Last.fm"));
	g_object_unref (statusbar);
}

/* Find a song with the artist and title independently of the album and adds it to the playlist */

static GList *
prepend_song_with_artist_and_title_to_mobj_list (PraghaLastfmPlugin *plugin,
                                                 const gchar *artist,
                                                 const gchar *title,
                                                 GList *list)
{
	PraghaPlaylist *playlist;
	PraghaDatabase *cdbase;
	PraghaMusicobject *mobj = NULL;
	gint location_id = 0;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);

	if (pragha_mobj_list_already_has_title_of_artist (list, title, artist) ||
	    pragha_playlist_already_has_title_of_artist (playlist, title, artist))
		return list;

	cdbase = pragha_application_get_database (priv->pragha);

	const gchar *sql =
		"SELECT LOCATION.id "
		"FROM TRACK, ARTIST, PROVIDER, LOCATION "
		"WHERE ARTIST.id = TRACK.artist "
		"AND LOCATION.id = TRACK.location "
		"AND TRACK.provider = PROVIDER.id AND PROVIDER.visible <> 0 "
		"AND TRACK.title = ? COLLATE NOCASE "
		"AND ARTIST.name = ? COLLATE NOCASE "
		"ORDER BY RANDOM() LIMIT 1;";

	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, title);
	pragha_prepared_statement_bind_string (statement, 2, artist);

	if (pragha_prepared_statement_step (statement)) {
		location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db (cdbase, location_id);
		list = g_list_prepend (list, mobj);
	}

	pragha_prepared_statement_free (statement);

	return list;
}


/* Set correction basedm on lastfm now playing segestion.. */

static void
pragha_corrected_by_lastfm_dialog_response (GtkWidget    *dialog,
                                            gint          response_id,
                                            PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *nmobj, *current_mobj;
	PraghaTagger *tagger;
	gint changed = 0;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, pragha_application_get_window(priv->pragha));
		return;
	}

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if (changed) {
			backend = pragha_application_get_backend (priv->pragha);

			nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

			if (pragha_backend_get_state (backend) != ST_STOPPED) {
				current_mobj = pragha_backend_get_musicobject (backend);
				if (pragha_musicobject_compare(nmobj, current_mobj) == 0) {
					toolbar = pragha_application_get_toolbar (priv->pragha);

					/* Update public current song */
					pragha_update_musicobject_change_tag(current_mobj, changed, nmobj);

					/* Update current song on playlist */
					playlist = pragha_application_get_playlist (priv->pragha);
					pragha_playlist_update_current_track (playlist, changed, nmobj);

					pragha_toolbar_set_title(toolbar, current_mobj);
				}
			}

			if (G_LIKELY(pragha_musicobject_is_local_file (nmobj))) {
				tagger = pragha_tagger_new();
				pragha_tagger_add_file (tagger, pragha_musicobject_get_file(nmobj));
				pragha_tagger_set_changes(tagger, nmobj, changed);
				pragha_tagger_apply_changes (tagger);
				g_object_unref(tagger);
			}
		}
	}

	gtk_widget_hide (priv->ntag_lastfm_button);
	gtk_widget_destroy (dialog);
}

static void
pragha_lastfm_tags_corrected_dialog (GtkButton *button, PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaMusicobject *tmobj, *nmobj;
	gchar *otitle = NULL, *oartist = NULL, *oalbum = NULL;
	gchar *ntitle = NULL, *nartist = NULL, *nalbum = NULL;
	gint prechanged = 0;
	GtkWidget *dialog;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	backend = pragha_application_get_backend (priv->pragha);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	/* Get all info of current track */

	tmobj = pragha_musicobject_dup (pragha_backend_get_musicobject (backend));

	g_object_get(tmobj,
	             "title", &otitle,
	             "artist", &oartist,
	             "album", &oalbum,
	             NULL);

	/* Get all info of suggestions
	 * Temp Musicobject to not block tag edit dialog */
	g_mutex_lock (&priv->data_mutex);
	nmobj = pragha_musicobject_dup(priv->updated_mobj);
	g_mutex_unlock (&priv->data_mutex);

	g_object_get(nmobj,
	             "title", &ntitle,
	             "artist", &nartist,
	             "album", &nalbum,
	             NULL);

	/* Compare original mobj and suggested from lastfm */
	if (g_ascii_strcasecmp(otitle, ntitle)) {
		pragha_musicobject_set_title(tmobj, ntitle);
		prechanged |= TAG_TITLE_CHANGED;
	}
	if (g_ascii_strcasecmp(oartist, nartist)) {
		pragha_musicobject_set_artist(tmobj, nartist);
		prechanged |= TAG_ARTIST_CHANGED;
	}
	if (g_ascii_strcasecmp(oalbum, nalbum)) {
		pragha_musicobject_set_album(tmobj, nalbum);
		prechanged |= TAG_ALBUM_CHANGED;
	}

	dialog = pragha_tags_dialog_new();
	gtk_window_set_transient_for (GTK_WINDOW(dialog),
		GTK_WINDOW(pragha_application_get_window (priv->pragha)));

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_corrected_by_lastfm_dialog_response), plugin);

	pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), tmobj);
	pragha_tags_dialog_set_changed(PRAGHA_TAGS_DIALOG(dialog), prechanged);

	gtk_widget_show (dialog);
}

void
pragha_lastfm_set_tiny_button (GtkWidget *button)
{
	GtkCssProvider *provider;
	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider,
	                                 "#tiny-button {\n"
#if GTK_CHECK_VERSION (3, 14, 0)
	                                 " margin : 0px;\n"
	                                 " min-width: 10px; \n"
	                                 " min-height: 10px; \n"
#else
	                                 " -GtkButton-default-border : 0px;\n"
	                                 " -GtkButton-default-outside-border : 0px;\n"
	                                 " -GtkButton-inner-border: 0px;\n"
	                                 " -GtkWidget-focus-line-width: 0px;\n"
	                                 " -GtkWidget-focus-padding: 0px;\n"
#endif
	                                 " padding: 1px;}",
	                                 -1, NULL);
	gtk_style_context_add_provider (gtk_widget_get_style_context (button),
	                                GTK_STYLE_PROVIDER (provider),
	                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_widget_set_name (button, "tiny-button");
	g_object_unref (provider);
}

static GtkWidget*
pragha_lastfm_tag_suggestion_button_new (PraghaLastfmPlugin *plugin)
{
	GtkWidget* ntag_lastfm_button, *image;
	ntag_lastfm_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(ntag_lastfm_button), GTK_RELIEF_NONE);

	image = gtk_image_new_from_icon_name ("tools-check-spelling", GTK_ICON_SIZE_MENU);
	gtk_button_set_image(GTK_BUTTON(ntag_lastfm_button), image);

	gtk_widget_set_tooltip_text(GTK_WIDGET(ntag_lastfm_button),
	                            _("Last.fm suggested a tag correction"));

	g_signal_connect(G_OBJECT(ntag_lastfm_button), "clicked",
	                 G_CALLBACK(pragha_lastfm_tags_corrected_dialog), plugin);

	pragha_lastfm_set_tiny_button (ntag_lastfm_button);
	gtk_image_set_pixel_size (GTK_IMAGE(image), 12);

	return ntag_lastfm_button;
}

/* Love and unlove music object */

gpointer
do_lastfm_love_mobj (PraghaLastfmPlugin *plugin, const gchar *title, const gchar *artist)
{
	gint rv;

	CDEBUG(DBG_PLUGIN, "Love mobj on thread");

	PraghaLastfmPluginPrivate *priv = plugin->priv;
	rv = LASTFM_track_love (priv->session_id,
	                        title,
	                        artist);

	if (rv != LASTFM_STATUS_OK)
		return _("Love song on Last.fm failed.");
	else
		return NULL;
}

gpointer
do_lastfm_unlove_mobj (PraghaLastfmPlugin *plugin, const gchar *title, const gchar *artist)
{
	gint rv;

	CDEBUG(DBG_PLUGIN, "Unlove mobj on thread");

	PraghaLastfmPluginPrivate *priv = plugin->priv;
	rv = LASTFM_track_unlove (priv->session_id,
	                          title,
	                          artist);

	if (rv != LASTFM_STATUS_OK)
		return _("Unlove song on Last.fm failed.");
	else
		return NULL;
}


/* Functions related to current playlist. */

gpointer
do_lastfm_current_playlist_love (gpointer data)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	return do_lastfm_love_mobj (plugin, title, artist);
}

static void
lastfm_track_current_playlist_love_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	CDEBUG(DBG_PLUGIN, "Love handler to current playlist");

	PraghaLastfmPluginPrivate *priv = plugin->priv;
	if (priv->status != LASTFM_STATUS_OK) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	pragha_async_launch (do_lastfm_current_playlist_love,
	                     pragha_async_set_idle_message,
	                     plugin);
}

gpointer
do_lastfm_current_playlist_unlove (gpointer data)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	return do_lastfm_unlove_mobj (plugin, title, artist);
}

static void
lastfm_track_current_playlist_unlove_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	CDEBUG(DBG_PLUGIN, "Unlove Handler to current playlist");

	PraghaLastfmPluginPrivate *priv = plugin->priv;
	if (priv->status != LASTFM_STATUS_OK) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	pragha_async_launch (do_lastfm_current_playlist_unlove,
	                     pragha_async_set_idle_message,
	                     plugin);
}

static gboolean
append_mobj_list_current_playlist_idle(gpointer user_data)
{
	PraghaPlaylist *playlist;
	PraghaStatusbar *statusbar;
	gchar *summary = NULL;
	guint songs_added = 0;

	AddMusicObjectListData *data = user_data;

	GList *list = data->list;
	PraghaLastfmPlugin *plugin = data->plugin;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (list != NULL) {
		playlist = pragha_application_get_playlist (priv->pragha);
		pragha_playlist_append_mobj_list (playlist, list);

		songs_added = g_list_length(list);
		g_list_free(list);
	}
	else {
		remove_watch_cursor (pragha_application_get_window(priv->pragha));
	}

	switch(data->query_type) {
		case LASTFM_GET_SIMILAR:
			if (data->query_count > 0)
				summary = g_strdup_printf(_("Added %d tracks of %d suggested from Last.fm"),
				                          songs_added, data->query_count);
			else
				summary = g_strdup_printf(_("Last.fm doesn't suggest any similar track"));
			break;
		case LASTFM_GET_LOVED:
			if (data->query_count > 0)
				summary = g_strdup_printf(_("Added %d songs of the last %d loved on Last.fm."),
							  songs_added, data->query_count);
			else
				summary = g_strdup_printf(_("You don't have favorite tracks on Last.fm"));
			break;
		case LASTFM_NONE:
		default:
			break;
	}

	if (summary != NULL) {
		statusbar = pragha_statusbar_get ();
		pragha_statusbar_set_misc_text (statusbar, summary);
		g_object_unref (statusbar);
		g_free(summary);
	}

	g_slice_free (AddMusicObjectListData, data);

	return FALSE;
}

gpointer
do_lastfm_get_similar (PraghaLastfmPlugin *plugin, const gchar *title, const gchar *artist)
{
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track = NULL;
	guint query_count = 0;
	GList *list = NULL;
	gint rv;

	AddMusicObjectListData *data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (string_is_not_empty(title) && string_is_not_empty(artist)) {
		rv = LASTFM_track_get_similar (priv->session_id,
		                               title,
		                               artist,
		                               50, &results);

		for (li=results; li && rv == LASTFM_STATUS_OK; li=li->next) {
			track = li->data;
			list = prepend_song_with_artist_and_title_to_mobj_list (plugin, track->artist, track->name, list);
			query_count += 1;
		}
	}

	data = g_slice_new (AddMusicObjectListData);
	data->list = list;
	data->query_type = LASTFM_GET_SIMILAR;
	data->query_count = query_count;
	data->plugin = plugin;

	LASTFM_free_track_info_list (results);

	return data;
}

gpointer
do_lastfm_get_similar_current_playlist_action (gpointer user_data)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	AddMusicObjectListData *data;

	PraghaLastfmPlugin *plugin = user_data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	data = do_lastfm_get_similar (plugin, title, artist);

	return data;
}

static void
lastfm_get_similar_current_playlist_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Get similar action to current playlist");

	if (priv->session_id == NULL) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	set_watch_cursor (pragha_application_get_window(priv->pragha));
	pragha_async_launch (do_lastfm_get_similar_current_playlist_action,
	                     append_mobj_list_current_playlist_idle,
	                     plugin);
}

/* Functions that respond to menu options. */

static void
lastfm_import_xspf_response (GtkDialog          *dialog,
                             gint                response,
                             PraghaLastfmPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaStatusbar *statusbar;
	XMLNode *xml = NULL, *xi, *xc, *xt;
	gchar *contents, *summary;
	gint try = 0, added = 0;
	GList *list = NULL;

	GFile *file;
	gsize size;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (response != GTK_RESPONSE_ACCEPT)
		goto cancel;

	file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

	if (!g_file_load_contents (file, NULL, &contents, &size, NULL, NULL)) {
		goto out;
    	}

	if (g_utf8_validate (contents, -1, NULL) == FALSE) {
		gchar *fixed;
		fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
		if (fixed != NULL) {
			g_free (contents);
			contents = fixed;
		}
	}

	xml = tinycxml_parse(contents);

	xi = xmlnode_get(xml,CCA { "playlist","trackList","track",NULL},NULL,NULL);
	for(;xi;xi= xi->next) {
		try++;
		xt = xmlnode_get(xi,CCA {"track","title",NULL},NULL,NULL);
		xc = xmlnode_get(xi,CCA {"track","creator",NULL},NULL,NULL);

		if (xt && xc)
			list = prepend_song_with_artist_and_title_to_mobj_list (plugin, xc->content, xt->content, list);
	}

	if (list) {
		playlist = pragha_application_get_playlist (priv->pragha);
		pragha_playlist_append_mobj_list (playlist, list);
		g_list_free (list);
	}

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_set_misc_text (statusbar, summary);
	g_object_unref (statusbar);

	g_free(summary);

	xmlnode_free(xml);
	g_free (contents);
out:
	g_object_unref (file);
cancel:
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static void
lastfm_import_xspf_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	GtkWidget *dialog;
	GtkFileFilter *media_filter;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	dialog = gtk_file_chooser_dialog_new (_("Import a XSPF playlist"),
	                                      GTK_WINDOW(pragha_application_get_window(priv->pragha)),
	                                      GTK_FILE_CHOOSER_ACTION_OPEN,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Open"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	media_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(media_filter), _("Supported media"));
	gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter), "application/xspf+xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(media_filter));

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(lastfm_import_xspf_response), plugin);

	gtk_widget_show_all (dialog);
}

gpointer
do_lastfm_add_favorites_action (gpointer user_data)
{
	PraghaPreferences *preferences;
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track;
	gint rpages = 0, cpage = 0;
	AddMusicObjectListData *data;
	guint query_count = 0;
	GList *list = NULL;
	gchar *user = NULL;

	PraghaLastfmPlugin *plugin = user_data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	preferences = pragha_application_get_preferences (priv->pragha);
	user = pragha_lastfm_plugin_get_user (preferences);

	do {
		rpages = LASTFM_user_get_loved_tracks (priv->session_id,
		                                       user,
		                                       cpage,
		                                       &results);

		for (li=results; li; li=li->next) {
			track = li->data;
			list = prepend_song_with_artist_and_title_to_mobj_list (plugin, track->artist, track->name, list);
			query_count += 1;
		}
		LASTFM_free_track_info_list (results);
		cpage++;
	} while (rpages != 0);

	data = g_slice_new (AddMusicObjectListData);
	data->list = list;
	data->query_type = LASTFM_GET_LOVED;
	data->query_count = query_count;
	data->plugin = plugin;

	g_free (user);

	return data;
}

static void
lastfm_add_favorites_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Add Favorites action");

	if ((priv->session_id == NULL) || !priv->has_user) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	set_watch_cursor (pragha_application_get_window(priv->pragha));
	pragha_async_launch (do_lastfm_add_favorites_action,
	                     append_mobj_list_current_playlist_idle,
	                     plugin);
}

static gpointer
do_lastfm_get_similar_action (gpointer user_data)
{
	AddMusicObjectListData *data;

	PraghaLastfmAsyncData *lastfm_async_data = user_data;

	PraghaLastfmPlugin *plugin   = lastfm_async_data->plugin;
	PraghaMusicobject *mobj = lastfm_async_data->mobj;

	const gchar *title = pragha_musicobject_get_title (mobj);
	const gchar *artist = pragha_musicobject_get_artist (mobj);

	data = do_lastfm_get_similar (plugin, title, artist);

	pragha_lastfm_async_data_free(lastfm_async_data);

	return data;
}

static void
lastfm_get_similar_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Get similar action");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	if (priv->session_id == NULL) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	set_watch_cursor (pragha_application_get_window(priv->pragha));
	pragha_async_launch (do_lastfm_get_similar_action,
	                     append_mobj_list_current_playlist_idle,
	                     pragha_lastfm_async_data_new (plugin));
}

static gpointer
do_lastfm_current_song_love (gpointer data)
{
	gpointer msg_data = NULL;

	PraghaLastfmAsyncData *lastfm_async_data = data;

	PraghaLastfmPlugin *plugin = lastfm_async_data->plugin;
	PraghaMusicobject *mobj = lastfm_async_data->mobj;

	const gchar *title = pragha_musicobject_get_title (mobj);
	const gchar *artist = pragha_musicobject_get_artist (mobj);

	msg_data = do_lastfm_love_mobj (plugin, title, artist);

	pragha_lastfm_async_data_free(lastfm_async_data);

	return msg_data;
}

static void
lastfm_track_love_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Love Handler");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	if (priv->status != LASTFM_STATUS_OK) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	pragha_async_launch (do_lastfm_current_song_love,
	                     pragha_async_set_idle_message,
	                     pragha_lastfm_async_data_new (plugin));
}

static gpointer
do_lastfm_current_song_unlove (gpointer data)
{
	gpointer msg_data = NULL;

	PraghaLastfmAsyncData *lastfm_async_data = data;

	PraghaLastfmPlugin *plugin = lastfm_async_data->plugin;
	PraghaMusicobject *mobj = lastfm_async_data->mobj;

	const gchar *title = pragha_musicobject_get_title (mobj);
	const gchar *artist = pragha_musicobject_get_artist (mobj);

	msg_data = do_lastfm_unlove_mobj (plugin, title, artist);

	pragha_lastfm_async_data_free(lastfm_async_data);

	return msg_data;
}

static void
lastfm_track_unlove_action (GtkAction *action, PraghaLastfmPlugin *plugin)
{
	PraghaBackend *backend;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Unlove Handler");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	if (priv->status != LASTFM_STATUS_OK) {
		pragha_lastfm_no_connection_advice ();
		return;
	}

	pragha_async_launch (do_lastfm_current_song_unlove,
	                     pragha_async_set_idle_message,
	                     pragha_lastfm_async_data_new (plugin));
}

/*
 * Gear menu actions.
 */

static void
pragha_gmenu_lastfm_add_favorites_action (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
	lastfm_add_favorites_action (NULL, PRAGHA_LASTFM_PLUGIN(user_data));
}

static void
pragha_gmenu_lastfm_get_similar_action (GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       user_data)
{
	lastfm_get_similar_action (NULL, PRAGHA_LASTFM_PLUGIN(user_data));
}

static void
pragha_gmenu_lastfm_import_xspf_action (GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       user_data)
{
	lastfm_import_xspf_action (NULL, PRAGHA_LASTFM_PLUGIN(user_data));
}

static void
pragha_gmenu_lastfm_track_love_action (GSimpleAction *action,
                                       GVariant      *parameter,
                                       gpointer       user_data)
{
	lastfm_track_love_action (NULL, PRAGHA_LASTFM_PLUGIN(user_data));
}

static void
pragha_gmenu_lastfm_track_unlove_action (GSimpleAction *action,
                                         GVariant      *parameter,
                                         gpointer       user_data)
{
	lastfm_track_unlove_action (NULL, PRAGHA_LASTFM_PLUGIN(user_data));
}

/*
 * Handlers
 */

static gpointer
pragha_lastfm_scrobble_thread (gpointer data)
{
	gchar *title = NULL, *artist = NULL, *album = NULL;
	gint track_no, length, rv;
	time_t last_time;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Scrobbler thread");

	g_mutex_lock (&priv->data_mutex);
	g_object_get (priv->current_mobj,
	              "title",    &title,
	              "artist",   &artist,
	              "album",    &album,
	              "track-no", &track_no,
	              "length",   &length,
	              NULL);
	last_time = priv->playback_started;
	g_mutex_unlock (&priv->data_mutex);

	rv = LASTFM_track_scrobble (priv->session_id,
	                            title,
	                            album,
	                            artist,
	                            last_time,
	                            length,
	                            track_no,
	                            0, NULL);

	g_free(title);
	g_free(artist);
	g_free(album);

	if (rv != LASTFM_STATUS_OK)
		return _("Last.fm submission failed");
	else
		return _("Track scrobbled on Last.fm");
}

static gboolean
pragha_lastfm_show_corrrection_button (gpointer user_data)
{
	PraghaToolbar *toolbar;
	PraghaBackend *backend;
	gchar *cfile = NULL, *nfile = NULL;

	PraghaLastfmPlugin *plugin = user_data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	/* Hack to safe!.*/
	if (!priv->ntag_lastfm_button) {
		toolbar = pragha_application_get_toolbar (priv->pragha);

		priv->ntag_lastfm_button =
			pragha_lastfm_tag_suggestion_button_new (plugin);

		pragha_toolbar_add_extention_widget (toolbar, priv->ntag_lastfm_button);
	}

	backend = pragha_application_get_backend (priv->pragha);
	g_object_get(pragha_backend_get_musicobject (backend),
	             "file", &cfile,
	             NULL);

	g_mutex_lock (&priv->data_mutex);
	g_object_get (priv->updated_mobj,
	              "file", &nfile,
	              NULL);
	g_mutex_unlock (&priv->data_mutex);

	if (g_ascii_strcasecmp(cfile, nfile) == 0)
		gtk_widget_show (priv->ntag_lastfm_button);

	g_free(cfile);
	g_free(nfile);

	return FALSE;
}

static gpointer
pragha_lastfm_now_playing_thread (gpointer data)
{
	LFMList *list = NULL;
	LASTFM_TRACK_INFO *ntrack = NULL;
	gchar *title = NULL, *artist = NULL, *album = NULL;
	gint track_no, length, changed = 0, rv;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Update now playing thread");

	g_mutex_lock (&priv->data_mutex);
	g_object_get (priv->current_mobj,
	              "title",    &title,
	              "artist",   &artist,
	              "album",    &album,
	              "track-no", &track_no,
	              "length",   &length,
	              NULL);
	g_mutex_unlock (&priv->data_mutex);

	rv = LASTFM_track_update_now_playing (priv->session_id,
	                                      title,
	                                      album,
	                                      artist,
	                                      length,
	                                      track_no,
	                                      0,
	                                      &list);

	if (rv == LASTFM_STATUS_OK) {
		/* Fist check lastfm response, and compare tags. */
		if (list != NULL) {
			ntrack = list->data;
			if (ntrack->name && !g_strrstr(ntrack->name, ";&#") && g_ascii_strcasecmp(title, ntrack->name))
				changed |= TAG_TITLE_CHANGED;
			if (ntrack->artist && !g_strrstr(ntrack->artist, ";&#") && g_ascii_strcasecmp(artist, ntrack->artist))
				changed |= TAG_ARTIST_CHANGED;
			if (ntrack->album && !g_strrstr(ntrack->album, ";&#") && g_ascii_strcasecmp(album, ntrack->album))
				changed |= TAG_ALBUM_CHANGED;
		}

		if (changed) {
			g_mutex_lock (&priv->data_mutex);
			if (priv->updated_mobj)
				g_object_unref (priv->updated_mobj);
			priv->updated_mobj = pragha_musicobject_dup (priv->current_mobj);
			if (changed & TAG_TITLE_CHANGED)
				pragha_musicobject_set_title (priv->updated_mobj, ntrack->name);
			if (changed & TAG_ARTIST_CHANGED)
				pragha_musicobject_set_artist (priv->updated_mobj, ntrack->artist);
			if (changed & TAG_ALBUM_CHANGED)
				pragha_musicobject_set_album (priv->updated_mobj, ntrack->album);
			g_mutex_unlock (&priv->data_mutex);

			g_idle_add (pragha_lastfm_show_corrrection_button, plugin);
		}
	}
	LASTFM_free_track_info_list(list);

	g_free(title);
	g_free(artist);
	g_free(album);

	if (rv != LASTFM_STATUS_OK)
		return _("Update current song on Last.fm failed.");
	else
		return NULL;
}

static gboolean
pragha_lastfm_now_playing_handler (gpointer data)
{
	PraghaBackend *backend;
	PraghaMusicobject *mobj = NULL;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	priv->update_timeout_id = 0;

	CDEBUG(DBG_PLUGIN, "Update now playing Handler");

	/* Set current song info */
	backend = pragha_application_get_backend (priv->pragha);
	mobj = pragha_backend_get_musicobject (backend);

	g_mutex_lock (&priv->data_mutex);
	if (priv->current_mobj)
		g_object_unref (priv->current_mobj);
	priv->current_mobj = pragha_musicobject_dup (mobj);
	if (priv->updated_mobj)
		g_object_unref (priv->updated_mobj);
	priv->updated_mobj = NULL;
	time(&priv->playback_started);
	g_mutex_unlock (&priv->data_mutex);

	/* Launch tread */
	pragha_async_launch (pragha_lastfm_now_playing_thread,
	                     pragha_async_set_idle_message,
	                     plugin);

	return FALSE;
}

static gboolean
pragha_lastfm_scrobble_handler (gpointer data)
{
	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Scrobbler Handler");

	priv->scrobble_timeout_id = 0;

	if (priv->status != LASTFM_STATUS_OK) {
		pragha_lastfm_no_connection_advice ();
		return FALSE;
	}

	pragha_async_launch (pragha_lastfm_scrobble_thread,
	                     pragha_async_set_idle_message,
	                     plugin);

	return FALSE;
}

static void
backend_changed_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	PraghaPreferences *preferences;
	PraghaMusicobject *mobj = NULL;
	PraghaMusicSource file_source = FILE_NONE;
	PraghaBackendState state = 0;
	gint length = 0, dalay_time = 0;

	PraghaLastfmPlugin *plugin = user_data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	state = pragha_backend_get_state (backend);

	CDEBUG(DBG_PLUGIN, "Configuring thread to update Lastfm");

	/* Update gui. */

	pragha_lastfm_update_menu_actions (plugin);

	/* Update thread. */

	if (priv->update_timeout_id) {
		g_source_remove (priv->update_timeout_id);
		priv->update_timeout_id = 0;
	}
	if (priv->scrobble_timeout_id) {
		g_source_remove (priv->scrobble_timeout_id);
		priv->scrobble_timeout_id = 0;
	}

	if (state != ST_PLAYING) {
		if (priv->ntag_lastfm_button)
			gtk_widget_hide (priv->ntag_lastfm_button);
		return;
	}

	/*
	 * Check settings, status, file-type, title, artist and length before update.
	 */

	preferences = pragha_application_get_preferences (priv->pragha);
	if (!pragha_lastfm_plugin_get_scrobble_support (preferences))
		return;

	if (!priv->has_user || !priv->has_pass)
		return;

	if (priv->status != LASTFM_STATUS_OK)
		return;

	mobj = pragha_backend_get_musicobject (backend);

	file_source = pragha_musicobject_get_source (mobj);
	if (file_source == FILE_NONE || file_source == FILE_HTTP)
		return;

	length = pragha_musicobject_get_length (mobj);
	if (length < 30)
		return;

	if (string_is_empty(pragha_musicobject_get_title(mobj)))
		return;
	if (string_is_empty(pragha_musicobject_get_artist(mobj)))
		return;

	/* Now playing delayed handler */
	priv->update_timeout_id =
		g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, WAIT_UPDATE,
	                                pragha_lastfm_now_playing_handler, plugin, NULL);

	/* Scrobble delayed handler */
	dalay_time = ((length / 2) > 240) ? 240 : (length / 2);
	priv->scrobble_timeout_id =
		g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, dalay_time,
		                            pragha_lastfm_scrobble_handler, plugin, NULL);
}

static void
pragha_menubar_append_lastfm (PraghaLastfmPlugin *plugin)
{
	PraghaPlaylist *playlist;
	GtkWindow *window;
	GActionMap *map;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	/*
	 * Menubar
	 */
	priv->action_group_main_menu = gtk_action_group_new ("PraghaLastfmMainMenuActions");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Love track", FALSE);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Unlove track", FALSE);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Add favorites", FALSE);
	pragha_action_group_set_sensitive (priv->action_group_main_menu, "Add similar", FALSE);

	/*
	 * Playlist
	 */
	priv->action_group_playlist = gtk_action_group_new ("PraghaLastfmPlaylistActions");
	gtk_action_group_set_translation_domain (priv->action_group_playlist, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_playlist,
	                              playlist_actions,
	                              G_N_ELEMENTS (playlist_actions),
	                              plugin);

	playlist = pragha_application_get_playlist (priv->pragha);
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                priv->action_group_playlist,
	                                                                playlist_xml);

	/*
	 * Gear Menu
	 */
	pragha_menubar_append_submenu (priv->pragha, "pragha-plugins-placeholder",
	                               lastfm_menu_ui,
	                               "lastfm-sudmenu",
	                               _("_Lastfm"),
	                               plugin);

	map = G_ACTION_MAP (pragha_application_get_window(priv->pragha));
	g_action_map_add_action_entries (G_ACTION_MAP (map),
	                                 lastfm_entries, G_N_ELEMENTS(lastfm_entries),
	                                 plugin);

	window = GTK_WINDOW(pragha_application_get_window(priv->pragha));
	pragha_menubar_set_enable_action (window, "lastfm-love", FALSE);
	pragha_menubar_set_enable_action (window, "lastfm-unlove", FALSE);
	pragha_menubar_set_enable_action (window, "lastfm-favorities", FALSE);
	pragha_menubar_set_enable_action (window, "lastfm-similar", FALSE);
}

static void
pragha_menubar_remove_lastfm (PraghaLastfmPlugin *plugin)
{
	PraghaPlaylist *playlist;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (!priv->merge_id_main_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);

	priv->merge_id_main_menu = 0;

	if (!priv->merge_id_playlist)
		return;

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);

	priv->merge_id_playlist = 0;

	pragha_menubar_remove_by_id (priv->pragha,
	                             "pragha-plugins-placeholder",
	                             "lastfm-sudmenu");
}

static gboolean
pragha_lastfm_connect_idle(gpointer data)
{
	PraghaPreferences *preferences;
	gchar *user = NULL, *pass = NULL;
	gboolean scrobble = FALSE;

	PraghaLastfmPlugin *plugin = data;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Connecting LASTFM");

	priv->session_id = LASTFM_init (LASTFM_API_KEY, LASTFM_SECRET);

	preferences = pragha_application_get_preferences (priv->pragha);

	scrobble = pragha_lastfm_plugin_get_scrobble_support (preferences);
	user = pragha_lastfm_plugin_get_user (preferences);
	pass = pragha_lastfm_plugin_get_password (preferences);

	priv->has_user = string_is_not_empty(user);
	priv->has_pass = string_is_not_empty(pass);

	if (scrobble && priv->has_user && priv->has_pass) {
		priv->status = LASTFM_login (priv->session_id, user, pass);

		if (priv->status != LASTFM_STATUS_OK) {
			pragha_lastfm_no_connection_advice ();
			CDEBUG(DBG_PLUGIN, "Failure to login on lastfm");
		}
	}

	pragha_lastfm_update_menu_actions (plugin);

	g_free(user);
	g_free(pass);

	return FALSE;
}

/* Init lastfm with a simple thread when change preferences and show error messages. */

static void
pragha_lastfm_connect (PraghaLastfmPlugin *plugin)
{
	g_idle_add (pragha_lastfm_connect_idle, plugin);
}

static void
pragha_lastfm_disconnect (PraghaLastfmPlugin *plugin)
{
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	if (priv->session_id != NULL) {
		CDEBUG(DBG_PLUGIN, "Disconnecting LASTFM");

		LASTFM_dinit(priv->session_id);

		priv->session_id = NULL;
		priv->status = LASTFM_STATUS_INVALID;
		priv->has_user = FALSE;
		priv->has_pass = FALSE;
	}
	pragha_lastfm_update_menu_actions (plugin);
}

/*
 * Lastfm Settings
 */
static void
pragha_lastfm_preferences_dialog_response (GtkDialog    *dialog,
                                           gint          response_id,
                                           PraghaLastfmPlugin *plugin)
{
	PraghaPreferences *preferences;
	const gchar *entry_user = NULL, *entry_pass = NULL;
	gchar *test_user = NULL, *test_pass = NULL;
	gboolean changed = FALSE, test_scrobble = FALSE, toggle_scrobble = FALSE;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();

	test_user = pragha_lastfm_plugin_get_user (preferences);
	test_pass = pragha_lastfm_plugin_get_password (preferences);

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->enable_w),
			pragha_lastfm_plugin_get_scrobble_support (preferences));

		pragha_gtk_entry_set_text (GTK_ENTRY(priv->lastfm_uname_w), test_user);
		pragha_gtk_entry_set_text (GTK_ENTRY(priv->lastfm_pass_w), test_pass);
		break;
	case GTK_RESPONSE_OK:
		toggle_scrobble = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->enable_w));
		entry_user = gtk_entry_get_text (GTK_ENTRY(priv->lastfm_uname_w));
		entry_pass = gtk_entry_get_text (GTK_ENTRY(priv->lastfm_pass_w));

		test_scrobble = pragha_lastfm_plugin_get_scrobble_support (preferences);

		if (test_scrobble != toggle_scrobble) {
			pragha_lastfm_plugin_set_scrobble_support (preferences, toggle_scrobble);
			changed = TRUE;
		}

		if (g_strcmp0 (test_user, entry_user)) {
			pragha_lastfm_plugin_set_user (preferences, entry_user);
			changed = TRUE;
		}

		if (g_strcmp0 (test_pass, entry_pass)) {
			pragha_lastfm_plugin_set_password (preferences, entry_pass);
			changed = TRUE;
		}

		if (changed) {
			pragha_lastfm_disconnect (plugin);
			pragha_lastfm_connect (plugin);
		}
		break;
	default:
		break;
	}
	g_object_unref (preferences);
	g_free (test_user);
	g_free (test_pass);
}

static void
toggle_lastfm (GtkToggleButton *button, PraghaLastfmPlugin *plugin)
{
	gboolean is_active;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->enable_w));

	gtk_widget_set_sensitive (priv->lastfm_uname_w, is_active);
	gtk_widget_set_sensitive (priv->lastfm_pass_w, is_active);

	if (!is_active) {
		gtk_entry_set_text (GTK_ENTRY(priv->lastfm_uname_w), "");
		gtk_entry_set_text (GTK_ENTRY(priv->lastfm_pass_w), "");
	}
}

static void
pragha_lastfm_init_settings (PraghaLastfmPlugin *plugin)
{
	PraghaPreferences *preferences;
	gchar *user = NULL, *pass = NULL;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get ();

	if (pragha_lastfm_plugin_get_scrobble_support (preferences)) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->enable_w), TRUE);

		user = pragha_lastfm_plugin_get_user (preferences);
		if (string_is_not_empty(user))
			gtk_entry_set_text (GTK_ENTRY(priv->lastfm_uname_w), user);
		g_free(user);

		pass = pragha_lastfm_plugin_get_password (preferences);
		if (string_is_not_empty(pass))
			gtk_entry_set_text (GTK_ENTRY(priv->lastfm_pass_w), pass);
		g_free(pass);
	}
	else {
		gtk_widget_set_sensitive (priv->lastfm_uname_w, FALSE);
		gtk_widget_set_sensitive (priv->lastfm_pass_w, FALSE);
	}

	g_object_unref (preferences);
}

static void
pragha_lastfm_plugin_append_setting (PraghaLastfmPlugin *plugin)
{
	PreferencesDialog *dialog;
	GtkWidget *table;
	GtkWidget *lastfm_check, *lastfm_uname, *lastfm_pass, *lastfm_ulabel, *lastfm_plabel;
	guint row = 0;

	PraghaLastfmPluginPrivate *priv = plugin->priv;

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title (table, &row, "Last.fm");

	lastfm_check = gtk_check_button_new_with_label (_("Scrobble on Last.fm"));
	pragha_hig_workarea_table_add_wide_control (table, &row, lastfm_check);

	lastfm_ulabel = gtk_label_new (_("Username"));
	lastfm_uname = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(lastfm_uname), LASTFM_UNAME_LEN);
	gtk_entry_set_activates_default (GTK_ENTRY(lastfm_uname), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, lastfm_ulabel, lastfm_uname);

	lastfm_plabel = gtk_label_new (_("Password"));
	lastfm_pass = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(lastfm_pass), LASTFM_PASS_LEN);
	gtk_entry_set_visibility (GTK_ENTRY(lastfm_pass), FALSE);
	gtk_entry_set_activates_default (GTK_ENTRY(lastfm_pass), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, lastfm_plabel, lastfm_pass);

	/* Store references. */

	priv->enable_w = lastfm_check;
	priv->lastfm_uname_w = lastfm_uname;
	priv->lastfm_pass_w = lastfm_pass;
	priv->setting_widget = table;

	/* Append panes */

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_append_services_setting (dialog,
	                                            priv->setting_widget, FALSE);

	/* Configure handler and settings */
	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_lastfm_preferences_dialog_response),
	                                           plugin);

	g_signal_connect (G_OBJECT(lastfm_check), "toggled",
	                  G_CALLBACK(toggle_lastfm), plugin);

	pragha_lastfm_init_settings (plugin);
}

static void
pragha_lastfm_plugin_remove_setting (PraghaLastfmPlugin *plugin)
{
	PreferencesDialog *dialog;
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_remove_services_setting (dialog,
	                                            priv->setting_widget);

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_lastfm_preferences_dialog_response),
	                                              plugin);
}

/*
 * Lastfm plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaLastfmPlugin *plugin = PRAGHA_LASTFM_PLUGIN (activatable);
	PraghaLastfmPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Lastfm plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	/* Init plugin flags */

	priv->session_id = NULL;
	priv->status = LASTFM_STATUS_INVALID;

	g_mutex_init (&priv->data_mutex);
	priv->updated_mobj = NULL;
	priv->current_mobj = NULL;

	priv->ntag_lastfm_button = NULL;

	priv->has_user = FALSE;
	priv->has_pass = FALSE;

	priv->update_timeout_id = 0;
	priv->scrobble_timeout_id = 0;

	/* Append menu and settings */

	pragha_menubar_append_lastfm (plugin);
	pragha_lastfm_plugin_append_setting (plugin);

	/* Test internet and launch threads.*/

	if (g_network_monitor_get_network_available (g_network_monitor_get_default ())) {
		g_idle_add (pragha_lastfm_connect_idle, plugin);
	}
	else {
		g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, 30,
		                            pragha_lastfm_connect_idle, plugin, NULL);
	}

	/* Connect playback signals */

	g_signal_connect (pragha_application_get_backend (priv->pragha), "notify::state",
	                  G_CALLBACK (backend_changed_state_cb), plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaPreferences *preferences;
	PraghaLastfmPlugin *plugin = PRAGHA_LASTFM_PLUGIN (activatable);
	PraghaLastfmPluginPrivate *priv = plugin->priv;
	gchar *plugin_group = NULL;

	CDEBUG(DBG_PLUGIN, "Lastfm plugin %s", G_STRFUNC);

	/* Disconnect playback signals */

	g_signal_handlers_disconnect_by_func (pragha_application_get_backend (priv->pragha),
	                                      backend_changed_state_cb, plugin);

	pragha_lastfm_disconnect (plugin);

	/* Settings */

	preferences = pragha_application_get_preferences (priv->pragha);
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "lastfm");
	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha))) {
		pragha_preferences_remove_group (preferences, plugin_group);
	}
	g_free (plugin_group);

	/* Remove menu and settings */

	pragha_menubar_remove_lastfm (plugin);
	pragha_lastfm_plugin_remove_setting (plugin);

	/* Clean */

	if (priv->updated_mobj)
		g_object_unref (priv->updated_mobj);
	if (priv->current_mobj)
		g_object_unref (priv->current_mobj);
	g_mutex_clear (&priv->data_mutex);
}
