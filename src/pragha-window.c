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

#include "pragha-window.h"
#include "pragha-menubar.h"
#include "pragha-playback.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-session.h"
#include "pragha-utils.h"
#include "pragha-debug.h"
#include "pragha.h"

/********************************/
/* Externally visible functions */
/********************************/

gboolean
pragha_close_window(GtkWidget *widget, GdkEvent *event, struct con_win *cwin)
{
	if(pragha_preferences_get_hide_instead_close(cwin->preferences)) {
		if(pragha_preferences_get_show_status_icon(cwin->preferences) &&
		   gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon)))
			pragha_window_toggle_state(cwin, FALSE);
		else
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
	}
	else {
		pragha_application_quit ();
	}
	return TRUE;
}

void
pragha_window_toggle_state (struct con_win *cwin, gboolean ignoreActivity)
{
	gint x = 0, y = 0;

	if (gtk_widget_get_visible (cwin->mainwindow)) {
		if (ignoreActivity || gtk_window_is_active (GTK_WINDOW(cwin->mainwindow))){
			gtk_window_get_position (GTK_WINDOW(cwin->mainwindow), &x, &y);
			gtk_widget_hide (GTK_WIDGET(cwin->mainwindow));
			gtk_window_move (GTK_WINDOW(cwin->mainwindow), x ,y);
		}
		else gtk_window_present (GTK_WINDOW(cwin->mainwindow));
	}
	else {
		gtk_widget_show (GTK_WIDGET(cwin->mainwindow));
	}
}

static void
backend_error_dialog_response_cb (GtkDialog *dialog, gint response, struct con_win *cwin)
{
	switch (response) {
		case GTK_RESPONSE_APPLY: {
			pragha_advance_playback (cwin);
			break;
		}
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_DELETE_EVENT:
		default: {
			pragha_backend_stop (pragha_application_get_backend (cwin));
			break;
		}
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
gui_backend_error_show_dialog_cb (PraghaBackend *backend, const GError *error, gpointer user_data)
{
	GtkWidget *dialog;

	struct con_win *cwin = user_data;

	const gchar *file = pragha_musicobject_get_file (pragha_backend_get_musicobject (backend));

	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (cwin->mainwindow),
	                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                             GTK_MESSAGE_QUESTION,
	                                             GTK_BUTTONS_NONE,
	                                             _("<b>Error playing current track.</b>\n(%s)\n<b>Reason:</b> %s"),
	                                             file, error->message);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_STOP, GTK_RESPONSE_ACCEPT);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_NEXT, GTK_RESPONSE_APPLY);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY);

	g_signal_connect(G_OBJECT(dialog), "response",
	                 G_CALLBACK(backend_error_dialog_response_cb), cwin);

	gtk_widget_show_all(dialog);
}

void
gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin)
{
	PraghaPlaylist *playlist;
	playlist = pragha_application_get_playlist (cwin);

	pragha_playlist_set_track_error (playlist, pragha_backend_get_error (backend));
}

static gboolean
pragha_window_state_event (GtkWidget *widget, GdkEventWindowState *event, struct con_win *cwin)
{
	GtkAction *action_fullscreen;

 	if (event->type == GDK_WINDOW_STATE && (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
		action_fullscreen = pragha_application_get_menu_action (cwin, "/Menubar/ViewMenu/Fullscreen");

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_fullscreen),
		                              (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0);
	}

	return FALSE;
}

void
pragha_window_unfullscreen (GObject *object, struct con_win *cwin)
{
	GtkAction *action_fullscreen;

	action_fullscreen = pragha_application_get_menu_action (cwin, "/Menubar/ViewMenu/Fullscreen");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_fullscreen), FALSE);
}

/*
 * Public api.
 */

void
pragha_window_add_widget_to_infobox (struct con_win *cwin, GtkWidget *widget)
{
	GList *list;
	GtkWidget *children;

	list = gtk_container_get_children (GTK_CONTAINER(cwin->infobox));

	if(list) {
		children = list->data;
		gtk_container_remove(GTK_CONTAINER(cwin->infobox), children);
		gtk_widget_destroy(GTK_WIDGET(children));
		g_list_free(list);
	}
		
	gtk_container_add(GTK_CONTAINER(cwin->infobox), widget);
}

gint
pragha_menubar_append_plugin_action (struct con_win *cwin,
                                     GtkActionGroup *action_group,
                                     const gchar *menu_xml)
{
	GtkUIManager *ui_manager;
	GError *error = NULL;
	gint merge_id;

	ui_manager = pragha_application_get_menu_ui_manager (cwin);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, -1);

	merge_id = gtk_ui_manager_add_ui_from_string (ui_manager,
	                                              menu_xml,
	                                              -1,
	                                              &error);

	if (error) {
		g_warning ("Adding plugin to menubar: %s", error->message);
		g_error_free (error);
	}

	return merge_id;
}

void
pragha_menubar_remove_plugin_action (struct con_win *cwin,
                                     GtkActionGroup *action_group,
                                     gint merge_id)
{
	GtkUIManager * ui_manager = pragha_application_get_menu_ui_manager (cwin);

	gtk_ui_manager_remove_ui (ui_manager, merge_id);
	gtk_ui_manager_remove_action_group (ui_manager, action_group);
	g_object_unref (action_group);
}

/*
 * Create and destroy the main window.
 */

void
pragha_window_free (struct con_win *cwin)
{
	PraghaPreferences *preferences;
	gint *window_size, *window_position;
	gint win_width, win_height, win_x, win_y;
	GdkWindowState state;
	const gchar *user_config_dir;
	gchar *pragha_accels_path = NULL;

	preferences = pragha_preferences_get();

	/* Save last window state */

	state = gdk_window_get_state (gtk_widget_get_window (cwin->mainwindow));

	if (pragha_preferences_get_remember_state(preferences)) {
		if (state & GDK_WINDOW_STATE_FULLSCREEN)
			pragha_preferences_set_start_mode(preferences, FULLSCREEN_STATE);
		else if(state & GDK_WINDOW_STATE_WITHDRAWN)
			pragha_preferences_set_start_mode(preferences, ICONIFIED_STATE);
		else
			pragha_preferences_set_start_mode(preferences, NORMAL_STATE);
	}

	/* Save geometry only if window is not maximized or fullscreened */

	if (!(state & GDK_WINDOW_STATE_MAXIMIZED) || !(state & GDK_WINDOW_STATE_FULLSCREEN)) {
		window_size = g_new0(gint, 2);
		gtk_window_get_size(GTK_WINDOW(cwin->mainwindow),
		                    &win_width, &win_height);
		window_size[0] = win_width;
		window_size[1] = win_height;

		window_position = g_new0(gint, 2);
		gtk_window_get_position(GTK_WINDOW(cwin->mainwindow),
		                        &win_x, &win_y);
		window_position[0] = win_x;
		window_position[1] = win_y;

		pragha_preferences_set_integer_list (preferences,
		                                     GROUP_WINDOW,
		                                     KEY_WINDOW_SIZE,
		                                     window_size,
		                                     2);

		pragha_preferences_set_integer_list (preferences,
		                                     GROUP_WINDOW,
		                                     KEY_WINDOW_POSITION,
		                                     window_position,
		                                     2);

		g_free(window_size);
		g_free(window_position);
	}

	/* Save sidebar size */

	pragha_preferences_set_sidebar_size(preferences,
		gtk_paned_get_position(GTK_PANED(cwin->pane)));


	/* Save menu accelerators edited */

	user_config_dir = g_get_user_config_dir();
	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha/accels.scm", NULL);
	gtk_accel_map_save (pragha_accels_path);

	/* Free memory */

	if (cwin->pixbuf_app)
		g_object_unref(cwin->pixbuf_app);

	g_object_unref (cwin->menu_ui_manager);

	g_object_unref(preferences);
	g_free(pragha_accels_path);
}

#if GTK_CHECK_VERSION (3, 0, 0)
static void init_gui_state(struct con_win *cwin)
{
	PraghaPlaylist *playlist;
	pragha_library_pane_init_view (cwin->clibrary);

	if (pragha_preferences_get_restore_playlist(cwin->preferences)) {
		playlist = pragha_application_get_playlist (cwin);
		init_current_playlist_view (playlist);
	}

	cwin->scanner = pragha_scanner_new();
	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		pragha_window_add_widget_to_infobox(cwin, info_bar);
	}
}
#else
static gboolean _init_gui_state(gpointer data)
{
	PraghaPlaylist *playlist;
	struct con_win *cwin = data;

	if (pragha_process_gtk_events ())
		return TRUE;
	pragha_library_pane_init_view (cwin->clibrary);

	if (pragha_process_gtk_events ())
		return TRUE;
	if (pragha_preferences_get_restore_playlist(cwin->preferences)) {
		playlist = pragha_application_get_playlist (cwin);
		init_current_playlist_view (playlist);
	}

	cwin->scanner = pragha_scanner_new();
	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		pragha_window_add_widget_to_infobox(cwin, info_bar);
	}

	return TRUE;
}
#endif

static void
pragha_window_init_menu_actions (struct con_win *cwin)
{
	GtkAction *action = NULL;
	const gchar *start_mode;

	action = pragha_application_get_menu_action (cwin, "/Menubar/ViewMenu/Fullscreen");

	start_mode = pragha_preferences_get_start_mode(cwin->preferences);
	if(!g_ascii_strcasecmp(start_mode, FULLSCREEN_STATE))
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), TRUE);
	else
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);

	action = pragha_application_get_menu_action (cwin, "/Menubar/ViewMenu/Playback controls below");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), pragha_preferences_get_controls_below (cwin->preferences));
}

static void
pragha_window_init (struct con_win *cwin)
{
	const gchar *start_mode;

	/* Init window state */

	start_mode = pragha_preferences_get_start_mode(cwin->preferences);
	if(!g_ascii_strcasecmp(start_mode, FULLSCREEN_STATE)) {
		gtk_widget_show(cwin->mainwindow);
	}
	else if(!g_ascii_strcasecmp(start_mode, ICONIFIED_STATE)) {
		if(gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))) {
			gtk_widget_hide(GTK_WIDGET(cwin->mainwindow));
		}
		else {
			g_warning("(%s): No embedded status_icon.", __func__);
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
			gtk_widget_show(cwin->mainwindow);
		}
	}
	else {
		gtk_widget_show(cwin->mainwindow);
	}

	pragha_window_init_menu_actions(cwin);
	update_playlist_changes_on_menu(cwin);

	pragha_init_session_support(cwin);

	#if GTK_CHECK_VERSION (3, 0, 0)
	init_gui_state(cwin);
	#else
	gtk_init_add(_init_gui_state, cwin);
	#endif
}

void
pragha_window_new (struct con_win *cwin)
{
	PraghaPreferences *preferences;
	GtkWidget *window;
	PraghaPlaylist *playlist;
	PraghaLibraryPane *library;
	PraghaSidebar *sidebar;
	PraghaStatusbar *statusbar;
	PraghaToolbar *toolbar;
	GtkWidget *menubar, *pane, *infobox;
	GtkWidget *playlist_statusbar_vbox, *vbox_main;
	gint *win_size, *win_position;
	gsize cnt = 0;

	CDEBUG(DBG_INFO, "Packaging widgets, and initiating the window");

	preferences = pragha_application_get_preferences (cwin);

	/* Collect widgets. */

	window    = pragha_application_get_window (cwin);
	playlist  = pragha_application_get_playlist (cwin);
	library   = pragha_application_get_library (cwin);
	sidebar   = pragha_application_get_sidebar (cwin);
	statusbar = pragha_application_get_statusbar (cwin);
	toolbar   = pragha_application_get_toolbar (cwin);
	menubar   = pragha_application_get_menubar (cwin);

	pane      = cwin->pane;
	infobox   = cwin->infobox;

	/* Main window */

#if !GTK_CHECK_VERSION (3, 0, 0)
	GdkScreen *screen = gtk_widget_get_screen (window);
	GdkColormap *colormap = gdk_screen_get_rgba_colormap (screen);
	if (colormap && gdk_screen_is_composited (screen))
		gtk_widget_set_default_colormap(colormap);
#endif

	g_signal_connect (G_OBJECT(window), "window-state-event",
	                  G_CALLBACK(pragha_window_state_event), cwin);
	g_signal_connect (G_OBJECT(window), "delete_event",
	                  G_CALLBACK(pragha_close_window), cwin);

	/* Set Default Size */

	win_size = pragha_preferences_get_integer_list (preferences,
	                                                GROUP_WINDOW,
	                                                KEY_WINDOW_SIZE,
	                                                &cnt);
	if (win_size) {
		gtk_window_set_default_size(GTK_WINDOW(window),
		                            win_size[0], win_size[1]);
		g_free(win_size);
	}
	else {
		gtk_window_set_default_size(GTK_WINDOW(window),
		                            MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	}

	/* Set Position */

	win_position = pragha_preferences_get_integer_list (preferences,
	                                                    GROUP_WINDOW,
	                                                    KEY_WINDOW_POSITION,
	                                                    &cnt);

	if (win_position) {
		gtk_window_move(GTK_WINDOW(window),
		                win_position[0], win_position[1]);
		g_free(win_position);
	}
	else {
		gtk_window_set_position(GTK_WINDOW(window),
		                        GTK_WIN_POS_CENTER);
	}

	/* Pack widgets: [ Playlist ]
	 *               [Status Bar]
	 */

	playlist_statusbar_vbox = gtk_vbox_new(FALSE, 2);

	gtk_box_pack_start (GTK_BOX(playlist_statusbar_vbox), GTK_WIDGET(playlist),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(playlist_statusbar_vbox), GTK_WIDGET(statusbar),
	                    FALSE, FALSE, 0);

	/* Pack widgets: [Sidebar][ Playlist ]
	 *               [       ][Status Bar]
	 */

	gtk_paned_pack1 (GTK_PANED (pane), pragha_sidebar_get_widget(sidebar), FALSE, TRUE);
	gtk_paned_pack2 (GTK_PANED (pane), playlist_statusbar_vbox, TRUE, FALSE);

	gtk_paned_set_position (GTK_PANED (pane),
		pragha_preferences_get_sidebar_size (preferences));


	/* Pack widgets: [    Menubar        ]
	 *               [    Toolbar        ]
	 *               [    Infobox        ]
	 *               [Sidebar][ Playlist ]
	 *               [Sidebar][Status Bar]
	 */

	vbox_main = gtk_vbox_new(FALSE, 2);

	gtk_box_pack_start (GTK_BOX(vbox_main), menubar,
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox_main), GTK_WIDGET(toolbar),
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox_main), infobox,
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox_main), pane,
	                    TRUE, TRUE, 0);

	/* Add library pane to sidebar. */

	pragha_sidebar_attach_plugin (sidebar,
		                          pragha_library_pane_get_widget (library),
		                          pragha_library_pane_get_pane_title (library),
		                          pragha_library_pane_get_popup_menu (library));

	/* Show the widgets individually.
	 *  NOTE: the rest of the widgets, depends on the preferences.
	 */

	gtk_widget_show(vbox_main);

	gtk_widget_show (menubar);
	gtk_widget_show (GTK_WIDGET(toolbar));
	gtk_widget_show (infobox);
	gtk_widget_show (pane);

	gtk_widget_show(playlist_statusbar_vbox);
	gtk_widget_show_all (GTK_WIDGET(playlist));

	/* Pack everyting on the main window. */

	gtk_container_add(GTK_CONTAINER(window), vbox_main);

	pragha_window_init (cwin);
}
