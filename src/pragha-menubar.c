/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <gdk/gdkkeysyms.h>

#include "pragha-menubar.h"
#include "pragha-cdda.h"
#include "pragha-playback.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-filter-dialog.h"
#include "pragha-preferences-dialog.h"
#include "pragha-equalizer-dialog.h"
#include "pragha.h"

/*
 * Proptotypes of functions.
 *
 * TODO: move these functions where tcorrespond!.
 */

GtkBuilder *builder;

/*
 * GMenuModel definitions.
 */

static void
pragha_gmenu_prev (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_playback_prev_track (pragha);
}

static void
pragha_gmenu_playpause (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_playback_play_pause_resume (pragha);
}

static void
pragha_gmenu_stop (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_playback_stop (pragha);
}

static void
pragha_gmenu_next (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_playback_next_track (pragha);
}

static void
pragha_gmenu_edit (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_playback_edit_current_track (pragha);
}

static void
pragha_gmenu_about (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_about_dialog (pragha);
}

static void
pragha_gmenu_quit (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;

	pragha_application_quit (pragha);
}

/*
 * Playback submenu.
 */
static void
pragha_gmenu_open (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_open_files (pragha);
}

static void
pragha_gmenu_audio_cd (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_append_audio_cd (pragha);
}

static void
pragha_gmenu_location (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_add_location (pragha);
}

static void
pragha_gmenu_library (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_append_entery_libary (pragha);
}

static void
pragha_gmenu_remove (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_remove_selection (playlist);
}

static void
pragha_gmenu_crop (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_crop_selection (playlist);
}

static void
pragha_gmenu_playlist_export (GSimpleAction *action,
                              GVariant      *parameter,
                              gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);

	export_current_playlist (NULL, playlist);
}

static void
pragha_gmenu_playlist_save (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);

	save_current_playlist (NULL, playlist);
}

static void
pragha_gmenu_selection_export (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);
	export_selected_playlist (NULL, playlist);
}

static void
pragha_gmenu_selection_save (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);
	save_selected_playlist (NULL, playlist);
}

static void
pragha_gmenu_clear (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
	PraghaPlaylist *playlist;
	PraghaApplication *pragha = user_data;

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_remove_all (playlist);
}

static void
pragha_gmenu_search (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_filter_dialog (pragha);
}

/*
 * Useful functions.
 */

static void
activate_toggle (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  GVariant *state;

  state = g_action_get_state (G_ACTION (action));
  g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
  g_variant_unref (state);
}

static void
pragha_set_enable_action (GtkWindow  *window,
                          const char *action_name,
                          gboolean    enabled)
{
	GAction *action;
	action = g_action_map_lookup_action (G_ACTION_MAP (window), action_name);
	g_object_set (action, "enabled", enabled, NULL);
}

GMenu *
pragha_menubar_get_menu_section (PraghaApplication *pragha,
                                 const char        *id)
{
	GObject *object;
	object = gtk_builder_get_object (builder, id);

	if (object == NULL || !G_IS_MENU (object))
		return NULL;

	return G_MENU (object);
}

void
pragha_menubar_emthy_menu_section (PraghaApplication *pragha,
                                   const char        *id)
{
	GMenu *menu;

	menu = G_MENU (gtk_builder_get_object (builder, id));

	while (g_menu_model_get_n_items (G_MENU_MODEL (menu)) > 0) {
		const char *action;
		g_menu_model_get_item_attribute (G_MENU_MODEL (menu), 0, G_MENU_ATTRIBUTE_ACTION, "s", &action);
		if (g_str_has_prefix (action, "win.")) {
			GVariant *target;

			target = g_menu_model_get_item_attribute_value (G_MENU_MODEL (menu), 0, G_MENU_ATTRIBUTE_TARGET, NULL);

			/* Don't remove actions that have a specific target */
			if (target == NULL) {
				GtkWindow *window;
				window = GTK_WINDOW(pragha_application_get_window(pragha));
				g_action_map_remove_action (G_ACTION_MAP (window), action + strlen ("win."));
			}
			else
				g_variant_unref (target);
		}
		g_menu_remove (G_MENU (menu), 0);
	}
}

static GActionEntry win_entries[] = {
	/* Playback submenu. */
	{ "prev",             pragha_gmenu_prev,             NULL, NULL,    NULL },
	{ "play",             pragha_gmenu_playpause,        NULL, NULL,    NULL },
	{ "stop",             pragha_gmenu_stop,             NULL, NULL,    NULL },
	{ "next",             pragha_gmenu_next,             NULL, NULL,    NULL },
	{ "shuffle",          activate_toggle,               NULL, "false", NULL },
	{ "repeat",           activate_toggle,               NULL, "false", NULL },
	{ "edit",             pragha_gmenu_edit,             NULL, NULL,    NULL },
	{ "quit",             pragha_gmenu_quit,             NULL, NULL,    NULL },
	{ "about",            pragha_gmenu_about,            NULL, NULL,    NULL },
	/* Playlist submenu. */
	{ "open",             pragha_gmenu_open,             NULL, NULL,    NULL },
	{ "cd",               pragha_gmenu_audio_cd,         NULL, NULL,    NULL },
	{ "location",         pragha_gmenu_location,         NULL, NULL,    NULL },
	{ "libary",           pragha_gmenu_library,          NULL, NULL,    NULL },
	{ "remove",           pragha_gmenu_remove,           NULL, NULL,    NULL },
	{ "crop",             pragha_gmenu_crop,             NULL, NULL,    NULL },
	{ "clear",            pragha_gmenu_clear,            NULL, NULL,    NULL },
	{ "export_playlist",  pragha_gmenu_playlist_export,  NULL, NULL,    NULL },
	{ "new_playlist",     pragha_gmenu_playlist_save,    NULL, NULL,    NULL },
	{ "export_selection", pragha_gmenu_selection_export, NULL, NULL,    NULL },
	{ "new_selection",    pragha_gmenu_selection_save,   NULL, NULL,    NULL },
	{ "search",           pragha_gmenu_search,           NULL, NULL,    NULL }
};

static const gchar *menu_ui = \
	NEW_MENU("menubar") \
		NEW_SUBMENU("_Playback") \
			NEW_ICON_ACCEL_ITEM("Prev track",                     "media-skip-backward",  "&lt;Alt&gt;Left",      "win", "prev") \
			NEW_ICON_ACCEL_ITEM("Play / Pause",                   "media-playback-start", "&lt;Control&gt;space", "win", "play") \
			NEW_ICON_ITEM      ("Stop",                           "media-playback-stop",                          "win", "stop") \
			NEW_ICON_ACCEL_ITEM("Next track",                     "media-skip-forward",   "&lt;Alt&gt;Right",     "win", "next") \
			SEPARATOR \
			NEW_ACCEL_ITEM     ("_Shuffle",                                               "&lt;Control&gt;U",     "win", "shuffle") \
			NEW_ACCEL_ITEM     ("_Repeat",                                                "&lt;Control&gt;R",     "win", "repeat") \
			SEPARATOR \
			NEW_ACCEL_ITEM     ("Edit track information",                                 "&lt;Control&gt;E",     "win", "edit") \
			SEPARATOR \
			NEW_ICON_ACCEL_ITEM("_Quit",                          "application-exit",     "&lt;Control&gt;Q",     "win", "quit") \
		CLOSE_SUBMENU \
		NEW_SUBMENU("Play_list") \
			NEW_ICON_ACCEL_ITEM("_Add files",                     "document-open",        "&lt;Control&gt;O",     "win", "open") \
			NEW_ICON_ITEM      ("Add Audio _CD",                  "media-optical",                                "win", "cd") \
			NEW_ICON_ITEM      ("Add _location",                  "network-workgroup",                            "win", "location") \
			SEPARATOR \
			NEW_ICON_ITEM      ("_Add the library",               "list-add",                                     "win", "library") \
			SEPARATOR \
			NEW_ICON_ITEM      ("Remove selection from playlist", "list-remove",                                  "win", "remove") \
			NEW_ICON_ACCEL_ITEM("Crop playlist",                  "list-remove",          "&lt;Control&gt;C",     "win", "crop") \
			NEW_ICON_ACCEL_ITEM("Clear playlist",                 "edit-clear",           "&lt;Control&gt;L",     "win", "clear") \
			SEPARATOR \
			NEW_SUBMENU("Save playlist") \
				NEW_ICON_ACCEL_ITEM("New playlist",               "document-new",         "&lt;Control&gt;S",     "win", "new_playlist") \
				NEW_ICON_ITEM      ("Export",                     "media-floppy",                                 "win", "export_playlist") \
				SEPARATOR \
				NEW_PLACEHOLDER("playlist-submenu") \
			CLOSE_SUBMENU \
			NEW_SUBMENU("Save selection") \
				NEW_ICON_ACCEL_ITEM("New playlist",               "document-new", "&lt;Shift&gt;&lt;Control&gt;S","win", "new_selection") \
				NEW_ICON_ITEM      ("Export",                     "media-floppy",                                 "win", "export_selection") \
				SEPARATOR \
				NEW_PLACEHOLDER("selection-submenu") \
			CLOSE_SUBMENU \
			SEPARATOR \
			NEW_ICON_ACCEL_ITEM("_Search in playlist",            "edit-find",            "&lt;Control&gt;F",     "win", "search") \
		CLOSE_SUBMENU \
		NEW_SUBMENU("_Help") \
			NEW_ICON_ITEM      ("About",                          "help-about",                                   "win", "about") \
		CLOSE_SUBMENU \
	CLOSE_MENU;

/*
 * Menubar callbacks.
 */

/* View */

static void fullscreen_action (GtkAction *action, PraghaApplication *pragha);
static void show_controls_below_action (GtkAction *action, PraghaApplication *pragha);
static void jump_to_playing_song_action (GtkAction *action, PraghaApplication *pragha);

/* Tools */

static void show_equalizer_action(GtkAction *action, PraghaApplication *pragha);
static void rescan_library_action(GtkAction *action, PraghaApplication *pragha);
static void update_library_action(GtkAction *action, PraghaApplication *pragha);
static void statistics_action(GtkAction *action, PraghaApplication *pragha);
static void pref_action(GtkAction *action, PraghaApplication *pragha);

/* Help */

static void home_action(GtkAction *action, PraghaApplication *pragha);
static void community_action(GtkAction *action, PraghaApplication *pragha);
static void wiki_action(GtkAction *action, PraghaApplication *pragha);
static void translate_action(GtkAction *action, PraghaApplication *pragha);

/*
 * Menu bar ui definition.
 */

static const gchar *main_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">						\
		<menu action=\"ViewMenu\">					\
			<menuitem action=\"Fullscreen\"/>			\
			<separator/>						\
			<menuitem action=\"Lateral panel1\"/>		\
			<menuitem action=\"Lateral panel2\"/>		\
			<menuitem action=\"Playback controls below\"/>	\
			<menuitem action=\"Status bar\"/>			\
			<separator/>						\
			<menuitem action=\"Jump to playing song\"/>	\
		</menu>								\
		<menu action=\"ToolsMenu\">					\
			<separator/>						\
			<menuitem action=\"Equalizer\"/>			\
			<separator/>						\
			<placeholder name=\"pragha-plugins-placeholder\"/>		\
			<separator/>						\
			<menuitem action=\"Rescan library\"/>			\
			<menuitem action=\"Update library\"/>			\
			<separator/>						\
			<menuitem action=\"Statistics\"/>			\
			<separator/>						\
			<menuitem action=\"Preferences\"/>			\
		</menu>								\
		<menu action=\"HelpMenu\">					\
			<menuitem action=\"Home\"/>				\
			<menuitem action=\"Community\"/>			\
			<menuitem action=\"Wiki\"/>				\
			<separator/>						\
			<menuitem action=\"Translate Pragha\"/>			\
			<separator/>						\
			<menuitem action=\"About\"/>				\
		</menu>								\
	</menubar>								\
</ui>";

static GtkActionEntry main_aentries[] = {
	{"ViewMenu", NULL, N_("_View")},
	{"ToolsMenu", NULL, N_("_Tools")},
	{"HelpMenu", NULL, N_("_Help")},
	{"Preferences", "preferences-system", N_("_Preferences"),
	 "<Control>P", "Set preferences", G_CALLBACK(pref_action)},
	{"Jump to playing song", "go-jump", N_("Jump to playing song"),
	 "<Control>J", "Jump to playing song", G_CALLBACK(jump_to_playing_song_action)},
	{"Equalizer", NULL, N_("E_qualizer"),
	 "", "Equalizer", G_CALLBACK(show_equalizer_action)},
	{"Rescan library", "system-run", N_("_Rescan library"),
	 "", "Rescan library", G_CALLBACK(rescan_library_action)},
	{"Update library", "system-run", N_("_Update library"),
	 "", "Update library", G_CALLBACK(update_library_action)},
	{"Statistics", "dialog-information", N_("_Statistics"),
	 "", "Statistics", G_CALLBACK(statistics_action)},
	{"Home", "go-home", N_("Homepage"),
	 "", "Homepage", G_CALLBACK(home_action)},
	{"Community", "dialog-information", N_("Community"),
	 "", "Forum of pragha", G_CALLBACK(community_action)},
	{"Wiki", NULL, N_("Wiki"),
	 "", "Wiki of pragha", G_CALLBACK(wiki_action)},
	{"Translate Pragha", "preferences-desktop-locale", N_("Translate Pragha"),
	 "", "Translate Pragha", G_CALLBACK(translate_action)}
};

static GtkToggleActionEntry toggles_entries[] = {
	{"Fullscreen", NULL, N_("_Fullscreen"),
	 "F11", "Switch between full screen and windowed mode", G_CALLBACK(fullscreen_action),
	FALSE},
	{"Lateral panel1", NULL, N_("Lateral _panel"),
	 "F9", "Lateral panel", NULL,
	TRUE},
	{"Lateral panel2", NULL, N_("Secondary lateral panel"),
	 "", "Secondary lateral panel", NULL,
	FALSE},
	{"Playback controls below", NULL, N_("Playback controls below"),
	 NULL, "Show playback controls below", G_CALLBACK(show_controls_below_action),
	FALSE},
	{"Status bar", NULL, N_("Status bar"),
	 "", "Status bar", NULL,
	TRUE}
};

/* Sentitive menubar actions depending on the playback status. */

void
pragha_menubar_update_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	GtkWindow *window;
	gboolean playing = FALSE;

	PraghaApplication *pragha = user_data;
	playing = (pragha_backend_get_state (backend) != ST_STOPPED);

	window = GTK_WINDOW(pragha_application_get_window(pragha));
	pragha_set_enable_action (window, "prev", playing);
	pragha_set_enable_action (window, "stop", playing);
	pragha_set_enable_action (window, "next", playing);
	pragha_set_enable_action (window, "edit", playing);
}

/* Handler for the 'Add Audio CD' item in the pragha menu */

void add_audio_cd_action(GtkAction *action, PraghaApplication *pragha)
{
	pragha_application_append_audio_cd (pragha);
}

/* Handler for the 'Preferences' item in the Edit menu */

static void pref_action(GtkAction *action, PraghaApplication *pragha)
{
	pragha_preferences_dialog_show (pragha);
}

/* Handler for the 'Full screen' item in the Edit menu */

static void
fullscreen_action (GtkAction *action, PraghaApplication *pragha)
{
	GtkWidget *menu_bar;
	gboolean fullscreen;
	GdkWindowState state;

	menu_bar = pragha_application_get_menubar (pragha);

	fullscreen = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	if(fullscreen){
		gtk_window_fullscreen(GTK_WINDOW(pragha_application_get_window(pragha)));
		gtk_widget_hide(GTK_WIDGET(menu_bar));
	}
	else {
		state = gdk_window_get_state (gtk_widget_get_window (pragha_application_get_window(pragha)));
		if (state & GDK_WINDOW_STATE_FULLSCREEN)
			gtk_window_unfullscreen(GTK_WINDOW(pragha_application_get_window(pragha)));
		gtk_widget_show(GTK_WIDGET(menu_bar));
	}
}

/* Handler for the 'Show_controls_below_action' item in the view menu */

static void
show_controls_below_action (GtkAction *action, PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	PraghaToolbar *toolbar;
	GtkWidget *parent;

	preferences = pragha_application_get_preferences (pragha);

	pragha_preferences_set_controls_below (preferences,
		gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));

	toolbar = pragha_application_get_toolbar (pragha);
	parent  = gtk_widget_get_parent (GTK_WIDGET(toolbar));

	gint position = pragha_preferences_get_controls_below (preferences) ? 3 : 1;

	gtk_box_reorder_child(GTK_BOX(parent), GTK_WIDGET(toolbar), position);
}

static void
jump_to_playing_song_action (GtkAction *action, PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	playlist = pragha_application_get_playlist (pragha);

	pragha_playlist_show_current_track (playlist);
}

/* Handler for the 'Equalizer' item in the Tools menu */

static void
show_equalizer_action(GtkAction *action, PraghaApplication *pragha)
{
	pragha_equalizer_dialog_show(pragha);
}


/* Handler for the 'Rescan Library' item in the Tools menu */

static void rescan_library_action(GtkAction *action, PraghaApplication *pragha)
{
	PraghaScanner *scanner;
	scanner = pragha_application_get_scanner (pragha);

	pragha_scanner_scan_library (scanner);
}

/* Handler for the 'Update Library' item in the Tools menu */

static void update_library_action(GtkAction *action, PraghaApplication *pragha)
{
	PraghaScanner *scanner;
	scanner = pragha_application_get_scanner (pragha);

	pragha_scanner_update_library (scanner);
}

/* Handler for 'Statistics' action in the Tools menu */

static void statistics_action(GtkAction *action, PraghaApplication *pragha)
{
	PraghaDatabase *cdbase;
	gint n_artists, n_albums, n_tracks;
	GtkWidget *dialog;

	cdbase = pragha_application_get_database (pragha);

	n_artists = pragha_database_get_artist_count (cdbase);
	n_albums = pragha_database_get_album_count (cdbase);
	n_tracks = pragha_database_get_track_count (cdbase);

	dialog = gtk_message_dialog_new(GTK_WINDOW(pragha_application_get_window(pragha)),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s %d\n%s %d\n%s %d",
					_("Total Tracks:"),
					n_tracks,
					_("Total Artists:"),
					n_artists,
					_("Total Albums:"),
					n_albums);

	gtk_window_set_title(GTK_WINDOW(dialog), _("Statistics"));

	g_signal_connect (dialog, "response",
	                  G_CALLBACK (gtk_widget_destroy), NULL);

	gtk_widget_show_all(dialog);
}

static void home_action(GtkAction *action, PraghaApplication *pragha)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(uri, pragha_application_get_window(pragha));
}

static void community_action(GtkAction *action, PraghaApplication *pragha)
{
	const gchar *uri = "http://bbs.archlinux.org/viewtopic.php?id=46171";
	open_url(uri, pragha_application_get_window(pragha));
}

static void wiki_action(GtkAction *action, PraghaApplication *pragha)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(uri, pragha_application_get_window(pragha));
}

static void translate_action(GtkAction *action, PraghaApplication *pragha)
{
	const gchar *uri = "http://www.transifex.net/projects/p/Pragha/";
	open_url(uri, pragha_application_get_window(pragha));
}

static gboolean
binding_gboolean_to_variant (GBinding *binding,
                             const GValue *from_value,
                             GValue *to_value,
                             gpointer user_data)
{
	GVariant *vvalue = g_variant_new_boolean (g_value_get_boolean (from_value));
	g_value_set_variant (to_value, vvalue);
	return TRUE;
}

static gboolean
binding_variant_to_gboolean (GBinding *binding,
                             const GValue *from_value,
                             GValue *to_value,
                             gpointer user_data)
{
	gboolean vbool = g_variant_get_boolean(g_value_get_variant(from_value));
	g_value_set_boolean (to_value, vbool);
	return TRUE;
}

void
pragha_menubar_connect_signals (GtkUIManager *menu_ui_manager, PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	GtkActionGroup *main_actions;
	GActionMap *map;
	GAction *action;
	GError *error = NULL;
	gsize length = -1;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	builder = gtk_builder_new ();
	gtk_builder_add_from_string (builder, menu_ui, length, &error);
	if (error) {
		g_print ("GtkBuilder error: %s", error->message);
		g_error_free (error);
	}

	g_action_map_add_action_entries (G_ACTION_MAP (pragha_application_get_window(pragha)),
	                                 win_entries, G_N_ELEMENTS (win_entries), pragha);
	gtk_application_set_menubar (GTK_APPLICATION (pragha),
	                             G_MENU_MODEL (gtk_builder_get_object (builder, "menubar")));
	//g_object_unref (builder);
 
	main_actions = gtk_action_group_new("Main Actions");

	gtk_action_group_set_translation_domain (main_actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (main_actions,
	                              main_aentries,
	                              G_N_ELEMENTS(main_aentries),
	                              (gpointer)pragha);
	gtk_action_group_add_toggle_actions (main_actions,
	                                     toggles_entries,
	                                     G_N_ELEMENTS(toggles_entries),
	                                     pragha);

	gtk_window_add_accel_group (GTK_WINDOW(pragha_application_get_window(pragha)),
	                            gtk_ui_manager_get_accel_group(menu_ui_manager));

	gtk_ui_manager_insert_action_group (menu_ui_manager, main_actions, 0);

	/* Hide second sidebar */
	GtkAction *action_sidebar = gtk_ui_manager_get_action(menu_ui_manager, "/Menubar/ViewMenu/Lateral panel2");
	gtk_action_set_visible (action_sidebar, FALSE);

	/* Binding properties to Actions. */

	preferences = pragha_application_get_preferences (pragha);

	map = G_ACTION_MAP (pragha_application_get_window(pragha));
	action = g_action_map_lookup_action (map, "shuffle");
	g_object_bind_property_full (preferences, "shuffle",
	                             action, "state",
	                             binding_flags,
	                             binding_gboolean_to_variant,
	                             binding_variant_to_gboolean,
	                             NULL,
	                             NULL);

	action = g_action_map_lookup_action (map, "repeat");
	g_object_bind_property_full (preferences, "repeat",
	                             action, "state",
	                             binding_flags,
	                             binding_gboolean_to_variant,
	                             binding_variant_to_gboolean,
	                             NULL,
	                             NULL);

	GtkAction *action_lateral1 = gtk_ui_manager_get_action(menu_ui_manager, "/Menubar/ViewMenu/Lateral panel1");
	g_object_bind_property (preferences, "lateral-panel", action_lateral1, "active", binding_flags);

	GtkAction *action_lateral2 = gtk_ui_manager_get_action(menu_ui_manager, "/Menubar/ViewMenu/Lateral panel2");
	g_object_bind_property (preferences, "secondary-lateral-panel", action_lateral2, "active", binding_flags);

	GtkAction *action_status_bar = gtk_ui_manager_get_action(menu_ui_manager, "/Menubar/ViewMenu/Status bar");
	g_object_bind_property (preferences, "show-status-bar", action_status_bar, "active", binding_flags);

	g_object_unref (main_actions);
}

GtkUIManager*
pragha_menubar_new (void)
{
	GtkUIManager *main_menu = NULL;
	gchar *pragha_accels_path = NULL;
	GError *error = NULL;

	main_menu = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_string(main_menu, main_menu_xml, -1, &error)) {
		g_critical("Unable to create main menu, err : %s", error->message);
	}

	/* Load menu accelerators edited */

	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, g_get_user_config_dir(), "/pragha/accels.scm", NULL);
	gtk_accel_map_load (pragha_accels_path);
	g_free (pragha_accels_path);

	return main_menu;
}
