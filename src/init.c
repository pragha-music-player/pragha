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
		mainwindow_add_widget_to_info_box(cwin, info_bar);
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
		mainwindow_add_widget_to_info_box(cwin, info_bar);
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

void init_pixbufs(struct con_win *cwin)
{
	cwin->pixbuf_app = gdk_pixbuf_new_from_file(PIXMAPDIR"/pragha.png", NULL);
	if (!cwin->pixbuf_app)
		g_warning("Unable to load pragha png");
}

static gboolean
window_state_event (GtkWidget *widget, GdkEventWindowState *event, struct con_win *cwin)
{
	GtkAction *action_fullscreen;

 	if (event->type == GDK_WINDOW_STATE && (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
		action_fullscreen = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Fullscreen");

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_fullscreen), (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0);
	}

	return FALSE;
}

void init_gui(gint argc, gchar **argv, struct con_win *cwin)
{
	GtkWidget *vbox, *info_box, *hbox_main, *menu_bar;
	gint *win_size, *win_position;
	gsize cnt = 0;
	const gchar *start_mode;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	CDEBUG(DBG_INFO, "Initializing gui");

	gtk_init(&argc, &argv);

	g_set_application_name(_("Pragha Music Player"));
	g_setenv("PULSE_PROP_media.role", "audio", TRUE);

	init_pixbufs(cwin);

	/* Main window */

	cwin->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	if (cwin->pixbuf_app)
		gtk_window_set_icon(GTK_WINDOW(cwin->mainwindow),
		                    cwin->pixbuf_app);

	gtk_window_set_title(GTK_WINDOW(cwin->mainwindow), _("Pragha Music Player"));

#if GTK_CHECK_VERSION (3, 0, 0)
	//XXX remove this?
#else
	GdkScreen *screen = gtk_widget_get_screen(cwin->mainwindow);
	GdkColormap *colormap = gdk_screen_get_rgba_colormap (screen);
	if (colormap && gdk_screen_is_composited (screen)){
		gtk_widget_set_default_colormap(colormap);
	}
#endif

	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "window-state-event",
			 G_CALLBACK(window_state_event), cwin);
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "delete_event",
			 G_CALLBACK(pragha_close_window), cwin);
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "destroy",
			 G_CALLBACK(pragha_destroy_window), cwin);

	/* Set Default Size */

	win_size = pragha_preferences_get_integer_list (cwin->preferences,
	                                                GROUP_WINDOW,
	                                                KEY_WINDOW_SIZE,
	                                                &cnt);
	if (win_size) {
		gtk_window_set_default_size(GTK_WINDOW(cwin->mainwindow),
		                            win_size[0],
		                            win_size[1]);
		g_free(win_size);
	}
	else {
		gtk_window_set_default_size(GTK_WINDOW(cwin->mainwindow),
		                            MIN_WINDOW_WIDTH,
		                            MIN_WINDOW_HEIGHT);
	}

	/* Set Position */

	win_position = pragha_preferences_get_integer_list (cwin->preferences,
	                                                    GROUP_WINDOW,
	                                                    KEY_WINDOW_POSITION,
	                                                    &cnt);

	if (win_position) {
		gtk_window_move(GTK_WINDOW(cwin->mainwindow),
		                win_position[0],
		                win_position[1]);
		g_free(win_position);
	}
	else {
		gtk_window_set_position(GTK_WINDOW(cwin->mainwindow),
		                        GTK_WIN_POS_CENTER);
	}

	/* Systray */

	create_status_icon(cwin);

	/* Main Vbox */

	vbox = gtk_vbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(cwin->mainwindow), vbox);

	/* Create hboxen */

	cwin->bar_context_menu = pragha_menubar_new(cwin);
	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), cwin);

	info_box = create_info_box(cwin);

	cwin->sidebar = pragha_sidebar_new(cwin);
	cwin->clibrary = pragha_library_pane_new(cwin);
	cwin->toolbar = pragha_toolbar_new(cwin);

	pragha_sidebar_add_pane(cwin->sidebar,
	                        pragha_library_pane_get_widget(cwin->clibrary));
	pragha_sidebar_attach_menu(cwin->sidebar,
	                           GTK_MENU(gtk_ui_manager_get_widget(pragha_library_pane_get_pane_context_menu(cwin->clibrary), "/popup")));

	hbox_main = create_main_region(cwin);
	menu_bar = gtk_ui_manager_get_widget(cwin->bar_context_menu, "/Menubar");

	/* Pack all hboxen into vbox */

	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(menu_bar),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(pragha_toolbar_get_widget(cwin->toolbar)),
			   FALSE,FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(info_box),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(hbox_main),
			   TRUE,TRUE, 0);

	gtk_widget_show(vbox);

	/* Send notifications on gui, OSD and mpris of new songs */

	g_signal_connect(cwin->backend,
			 "notify::state",
			 G_CALLBACK(pragha_backend_notificate_new_state), cwin);
	g_signal_connect(cwin->backend,
	                 "finished",
	                 G_CALLBACK(pragha_backend_finished_song), cwin);
	g_signal_connect(cwin->backend,
	                 "tags-changed",
	                 G_CALLBACK(pragha_backend_tags_changed), cwin);

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

	/* TODO: Move it to Widgets construction. */
	g_object_bind_property (cwin->preferences, "lateral-panel",
	                        pragha_sidebar_get_widget(cwin->sidebar), "visible",
	                        binding_flags);
	g_object_bind_property (cwin->preferences, "show-status-bar",
	                        cwin->statusbar, "visible",
	                        binding_flags);

	init_menu_actions(cwin);
	update_playlist_changes_on_menu(cwin);

	g_signal_connect(cwin->backend,
			 "error",
			 G_CALLBACK(gui_backend_error_show_dialog_cb), cwin);

	g_signal_connect(cwin->backend,
			 "error",
			 G_CALLBACK(gui_backend_error_update_current_playlist_cb), cwin);

	pragha_init_session_support(cwin);

	#if GTK_CHECK_VERSION (3, 0, 0)
	init_gui_state(cwin);
	#else
	gtk_init_add(_init_gui_state, cwin);
	#endif
}
