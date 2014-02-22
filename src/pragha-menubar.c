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
static void statistics_action(GtkAction *action, PraghaApplication *pragha);

/*
 * Playback submenu.
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
pragha_gmenu_quit (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
	PraghaApplication *pragha = user_data;

	pragha_application_quit (pragha);
}

/*
 * Playlist submenu.
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

/* View submenu */

static void
pragha_gmenu_fullscreen (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
	GdkWindowState window_state;
	gboolean fullscreen;
	GVariant *state;

	PraghaApplication *pragha = user_data;

	state = g_action_get_state (G_ACTION (action));

	fullscreen = !g_variant_get_boolean (state);

	g_action_change_state (G_ACTION (action), g_variant_new_boolean (fullscreen));
	g_variant_unref (state);

	if (fullscreen) {
		gtk_window_fullscreen (GTK_WINDOW(pragha_application_get_window(pragha)));
		gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(pragha_application_get_window(pragha)), FALSE);
	}
	else {
		window_state = gdk_window_get_state (gtk_widget_get_window (pragha_application_get_window(pragha)));
		if (window_state & GDK_WINDOW_STATE_FULLSCREEN)
			gtk_window_unfullscreen (GTK_WINDOW(pragha_application_get_window(pragha)));
		gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(pragha_application_get_window(pragha)), TRUE);
	}
}

static void
pragha_gmenu_controls_below (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	PraghaPreferences *preferences;
	PraghaToolbar *toolbar;
	GtkWidget *parent;
	GVariant *state;
	gboolean controls_below;

	state = g_action_get_state (G_ACTION (action));
	controls_below = !g_variant_get_boolean (state);
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (controls_below));
	g_variant_unref (state);

	preferences = pragha_application_get_preferences (pragha);
	pragha_preferences_set_controls_below (preferences, controls_below);

	toolbar = pragha_application_get_toolbar (pragha);
	parent  = gtk_widget_get_parent (GTK_WIDGET(toolbar));

	gint position = pragha_preferences_get_controls_below (preferences) ? 3 : 1;

	gtk_box_reorder_child(GTK_BOX(parent), GTK_WIDGET(toolbar), position);
}

static void
pragha_gmenu_jump_to_song (GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer       user_data)
{
	PraghaPlaylist *playlist;

	PraghaApplication *pragha = user_data;
	playlist = pragha_application_get_playlist (pragha);

	pragha_playlist_show_current_track (playlist);
}

/* Tools Submenu */

static void
pragha_gmenu_equalizer (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_equalizer_dialog_show(pragha);
}

static void
pragha_gmenu_rescan_library (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
	PraghaScanner *scanner;
	PraghaApplication *pragha = user_data;

	scanner = pragha_application_get_scanner (pragha);
	pragha_scanner_scan_library (scanner);
}

static void
pragha_gmenu_update_library (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
	PraghaScanner *scanner;
	PraghaApplication *pragha = user_data;

	scanner = pragha_application_get_scanner (pragha);
	pragha_scanner_update_library (scanner);
}

static void
pragha_gmenu_show_statistic (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	statistics_action (NULL, pragha);
}

static void
pragha_gmenu_show_preferences (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_preferences_dialog_show (pragha);
}

/* Help Submenu */

static void
pragha_gmenu_show_homepage (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	const gchar *uri = "http://pragha.wikispaces.com/";

	open_url (uri, pragha_application_get_window(pragha));
}

static void
pragha_gmenu_show_community (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	const gchar *uri = "http://bbs.archlinux.org/viewtopic.php?id=46171";

	open_url (uri, pragha_application_get_window(pragha));
}

static void
pragha_gmenu_show_wiki (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	const gchar *uri = "http://pragha.wikispaces.com/";

	open_url (uri, pragha_application_get_window(pragha));
}

static void
pragha_gmenu_translate (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	const gchar *uri = "http://www.transifex.net/projects/p/Pragha/";
	open_url (uri, pragha_application_get_window(pragha));
}

static void
pragha_gmenu_about (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
	PraghaApplication *pragha = user_data;
	pragha_application_about_dialog (pragha);
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
	GtkBuilder *builder;

	builder = pragha_application_get_menu_ui (pragha);
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
	GtkBuilder *builder;

	builder = pragha_application_get_menu_ui (pragha);
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

void
pragha_menubar_append_action (PraghaApplication *pragha,
                              const gchar       *placeholder,
                              GSimpleAction     *action,
                              GMenuItem         *item)
{
	GActionMap *map;
	GMenu *place;

	place = pragha_menubar_get_menu_section (pragha, placeholder);

	map = G_ACTION_MAP (pragha_application_get_window(pragha));

	g_action_map_add_action (map, G_ACTION (action));
	g_menu_append_item (G_MENU (place), item);
}

void
pragha_menubar_remove_action (PraghaApplication *pragha,
                              const gchar       *placeholder,
                              const gchar       *action_name)
{
	GtkBuilder *builder;
	GActionMap *map;
	GMenu *menu;
	const char *action;
	gint i;

	builder = pragha_application_get_menu_ui (pragha);
	menu = G_MENU (gtk_builder_get_object (builder, placeholder));

	for (i = 0; i < g_menu_model_get_n_items (G_MENU_MODEL(menu)); i++) {
		if (g_menu_model_get_item_attribute (G_MENU_MODEL(menu), i, G_MENU_ATTRIBUTE_ACTION, "s", &action)) {
			if (g_strcmp0 (action + strlen ("win."), action_name) == 0) {
				g_menu_remove (G_MENU (menu), i);

				map = G_ACTION_MAP (pragha_application_get_window(pragha));
				g_action_map_remove_action (map, action_name);
				break;
			}
		}
	}
}

void
pragha_menubar_append_submenu (PraghaApplication  *pragha,
                               const gchar        *placeholder,
                               const gchar        *xml_ui,
                               const gchar        *menu_id,
                               const gchar        *label,
                               gpointer            user_data)
{
	GtkBuilder *builder;
	GError *error = NULL;
	GMenuModel *menu;
	GMenu *section;
	GMenuItem *menu_item;

	builder = gtk_builder_new ();
	gtk_builder_add_from_string (builder, xml_ui, -1, &error);

	if (error) {
		g_print ("GtkBuilder error: %s", error->message);
		g_error_free (error);
		error = NULL;
	}

	section = pragha_menubar_get_menu_section (pragha, placeholder);
	menu = G_MENU_MODEL (gtk_builder_get_object (builder, menu_id));

	menu_item = g_menu_item_new_submenu (label, menu);
	g_menu_item_set_attribute (menu_item, "pragha-merge-id", "s", menu_id);
	g_menu_insert_item (section, -1, menu_item);

	g_object_unref (menu_item);
}

void
pragha_menubar_remove_by_id (PraghaApplication *pragha,
                             const gchar       *placeholder,
                             const gchar       *item_id)
{
	GtkBuilder *builder;
	GMenu *menu;
	const char *id;
	gint i;

	builder = pragha_application_get_menu_ui (pragha);
	menu = G_MENU (gtk_builder_get_object (builder, placeholder));

	for (i = 0; i < g_menu_model_get_n_items (G_MENU_MODEL(menu)); i++) {
		if (g_menu_model_get_item_attribute (G_MENU_MODEL(menu), i, "pragha-merge-id", "s", &id)) {
			if (g_strcmp0 (id, item_id) == 0)
				g_menu_remove (G_MENU (menu), i);
		}
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
	{ "search",           pragha_gmenu_search,           NULL, NULL,    NULL },
	/* View Submenu */
	{ "fullscreen",       pragha_gmenu_fullscreen,       NULL, "false", NULL },
	{ "sidebar1",         activate_toggle,               NULL, "false", NULL },
	{ "sidebar2",         activate_toggle,               NULL, "false", NULL },
	{ "controls-below",   pragha_gmenu_controls_below,   NULL, "false", NULL },
	{ "status-bar",       activate_toggle,               NULL, "false", NULL },
	{ "jump-song",        pragha_gmenu_jump_to_song,     NULL, NULL,    NULL },
	/* Tools submenu */
	{ "equalizer",        pragha_gmenu_equalizer,        NULL, NULL,    NULL },
	{ "lib-rescan",       pragha_gmenu_rescan_library,   NULL, NULL,    NULL },
	{ "lib-update",       pragha_gmenu_update_library,   NULL, NULL,    NULL },
	{ "statistics",       pragha_gmenu_show_statistic,   NULL, NULL,    NULL },
	{ "preferences",      pragha_gmenu_show_preferences, NULL, NULL,    NULL },
	/* Help submenu */
	{ "homepage",         pragha_gmenu_show_homepage,    NULL, NULL,    NULL },
	{ "community",        pragha_gmenu_show_community,   NULL, NULL,    NULL },
	{ "wiki",             pragha_gmenu_show_wiki,        NULL, NULL,    NULL },
	{ "translate",        pragha_gmenu_translate,        NULL, NULL,    NULL },
	{ "about",            pragha_gmenu_about,            NULL, NULL,    NULL }
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
		NEW_SUBMENU("_View") \
			NEW_ACCEL_ITEM     ("_Fullscreen",                                            "F11",                  "win", "fullscreen") \
			SEPARATOR \
			NEW_ACCEL_ITEM     ("Lateral _panel",                                         "F9",                   "win", "sidebar1") \
			NEW_ACCEL_ITEM     ("Secondary lateral panel",                                "&lt;Control&gt;F9",    "win", "sidebar2") \
			NEW_ITEM           ("Playback controls below",                                                        "win", "controls-below") \
			NEW_ITEM           ("Status bar",                                                                     "win", "status-bar") \
			SEPARATOR \
			NEW_ICON_ACCEL_ITEM("Jump to playing song",           "go-jump",              "&lt;Control&gt;J",     "win", "jump-song") \
		CLOSE_SUBMENU \
		NEW_SUBMENU("_Tools") \
			NEW_ITEM           ("E_qualizer",                                                                     "win", "equalizer") \
			SEPARATOR \
			NEW_PLACEHOLDER("pragha-plugins-placeholder") \
			SEPARATOR \
			NEW_ICON_ITEM      ("_Rescan library",                "system-run",                                   "win", "lib-rescan") \
			NEW_ICON_ITEM      ("_Update library",                "system-run",                                   "win", "lib-update") \
			SEPARATOR \
			NEW_ICON_ITEM      ("_Statistics",                   "dialog-information",                            "win", "statistics") \
			SEPARATOR \
			NEW_ICON_ACCEL_ITEM("_Preferences",                   "preferences-system",   "&lt;Control&gt;P",     "win", "preferences") \
		CLOSE_SUBMENU \
		NEW_SUBMENU("_Help") \
			NEW_ICON_ITEM      ("Homepage",                       "go-home",                                      "win", "homepage") \
			NEW_ICON_ITEM      ("Community",                      "dialog-information",                           "win", "community") \
			NEW_ITEM           ("Wiki",                                                                           "win", "wiki") \
			SEPARATOR \
			NEW_ICON_ITEM      ("Translate Pragha",               "preferences-desktop-locale",                   "win", "translate") \
			SEPARATOR \
			NEW_ICON_ITEM      ("About",                          "help-about",                                   "win", "about") \
		CLOSE_SUBMENU \
	CLOSE_MENU;

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

GtkBuilder *
pragha_application_set_menubar (PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	GtkBuilder *builder;
	GActionMap *map;
	GAction *action;
	GError *error = NULL;
	gchar *pragha_accels_path = NULL;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	builder = gtk_builder_new ();
	gtk_builder_add_from_string (builder, menu_ui, -1, &error);
	if (error) {
		g_print ("GtkBuilder error: %s", error->message);
		g_error_free (error);
		error = NULL;
	}

	/* Get the action map*/

	map = G_ACTION_MAP (pragha_application_get_window(pragha));

	/* Add the menu */

	g_action_map_add_action_entries (G_ACTION_MAP (map),
	                                 win_entries, G_N_ELEMENTS (win_entries), pragha);
	gtk_application_set_menubar (GTK_APPLICATION (pragha),
	                             G_MENU_MODEL (gtk_builder_get_object (builder, "menubar")));

	/* Insensitive second sidebar */

	action = g_action_map_lookup_action (map, "sidebar2");
	g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

	/* Binding properties to Actions. */

	preferences = pragha_application_get_preferences (pragha);

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

	action = g_action_map_lookup_action (map, "sidebar1");
	g_object_bind_property_full (preferences, "lateral-panel",
	                             action, "state",
	                             binding_flags,
	                             binding_gboolean_to_variant,
	                             binding_variant_to_gboolean,
	                             NULL,
	                             NULL);
	action = g_action_map_lookup_action (map, "sidebar2");
	g_object_bind_property_full (preferences, "secondary-lateral-panel",
	                             action, "state",
	                             binding_flags,
	                             binding_gboolean_to_variant,
	                             binding_variant_to_gboolean,
	                             NULL,
	                             NULL);

	action = g_action_map_lookup_action (map, "status-bar");
	g_object_bind_property_full (preferences, "show-status-bar",
	                             action, "state",
	                             binding_flags,
	                             binding_gboolean_to_variant,
	                             binding_variant_to_gboolean,
	                             NULL,
	                             NULL);

	g_signal_connect (pragha_application_get_backend (pragha), "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), pragha);

	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, g_get_user_config_dir(), "/pragha/accels.scm", NULL);
	gtk_accel_map_load (pragha_accels_path);
	g_free (pragha_accels_path);

	return builder;
}
