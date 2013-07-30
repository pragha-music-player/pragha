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

#ifdef HAVE_LIBXFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#endif
#include "pragha-playback.h"
#include "pragha-library-pane.h"
#include "pragha-menubar.h"
#include "pragha-statusicon.h"
#include "pragha-utils.h"
#include "pragha-window.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-preferences-dialog.h"
#include "pragha-session.h"
#include "pragha-debug.h"
#include "pragha.h"

#if GTK_CHECK_VERSION (3, 0, 0)
static void init_gui_state(struct con_win *cwin)
{
	pragha_library_pane_init_view(cwin->clibrary, cwin);

	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		init_current_playlist_view(cwin->cplaylist);

	cwin->scanner = pragha_scanner_new();
	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		pragha_window_add_widget_to_infobox(cwin->window, info_bar);
	}
}
#else
static gboolean _init_gui_state(gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_process_gtk_events ())
		return TRUE;
	pragha_library_pane_init_view(cwin->clibrary, cwin);

	if (pragha_process_gtk_events ())
		return TRUE;
	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		init_current_playlist_view(cwin->cplaylist);

	cwin->scanner = pragha_scanner_new();
	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		pragha_window_add_widget_to_infobox(cwin->window, info_bar);
	}

	return TRUE;
}
#endif

void init_menu_actions(struct con_win *cwin)
{
	GtkAction *action = NULL;
	const gchar *start_mode;

	/* Init state of menus */

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Fullscreen");

	start_mode = pragha_preferences_get_start_mode(cwin->preferences);
	if(!g_ascii_strcasecmp(start_mode, FULLSCREEN_STATE))
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), TRUE);
	else
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Playback controls below");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), pragha_preferences_get_controls_below (cwin->preferences));
}

void init_gui(gint argc, gchar **argv, struct con_win *cwin)
{
	const gchar *start_mode;

	CDEBUG(DBG_INFO, "Initializing gui");

	/* HACK:
	 * All menus require the window created:
	 *  * gtk_window_add_accel_group()
	 */
	cwin->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	cwin->bar_context_menu = pragha_menubar_new(cwin);
	cwin->sidebar = pragha_sidebar_new();
	cwin->clibrary = pragha_library_pane_new(cwin);

	/* Toolbar */

	cwin->toolbar = pragha_toolbar_new();
	g_signal_connect (cwin->toolbar, "prev",
	                  G_CALLBACK(pragha_playback_prev_song), cwin);
	g_signal_connect (cwin->toolbar, "play",
	                  G_CALLBACK(pragha_playback_play_song), cwin);
	g_signal_connect (cwin->toolbar, "stop",
	                  G_CALLBACK(pragha_playback_stop_song), cwin);
	g_signal_connect (cwin->toolbar, "next",
	                  G_CALLBACK(pragha_playback_next_song), cwin);
	g_signal_connect (cwin->toolbar, "album-art-activated",
	                  G_CALLBACK(pragha_playback_show_current_album_art), cwin);
	g_signal_connect (cwin->toolbar, "track-info-activated",
	                  G_CALLBACK(pragha_playback_edit_current_track), cwin);
	g_signal_connect (cwin->toolbar, "track-progress-activated",
	                  G_CALLBACK(pragha_playback_seek_fraction), cwin);

	cwin->statusbar = pragha_statusbar_get();
	cwin->cplaylist = pragha_playlist_new(cwin);

	pragha_sidebar_add_pane(cwin->sidebar,
	                        pragha_library_pane_get_widget(cwin->clibrary));
	pragha_sidebar_attach_menu(cwin->sidebar,
	                           GTK_MENU(gtk_ui_manager_get_widget(pragha_library_pane_get_pane_context_menu(cwin->clibrary), "/popup")));

	cwin->window = pragha_window_new(cwin);
	/* HACK:
	 * All menus require the window created:
	 *  * gtk_window_add_accel_group()
	 */
	/*cwin->mainwindow = pragha_window_get_mainwindow (cwin->window);*/

	/* Systray */

	create_status_icon(cwin);


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

	init_menu_actions(cwin);
	update_playlist_changes_on_menu(cwin);

	pragha_init_session_support(cwin);

	#if GTK_CHECK_VERSION (3, 0, 0)
	init_gui_state(cwin);
	#else
	gtk_init_add(_init_gui_state, cwin);
	#endif
}
