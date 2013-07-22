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

#include "pragha-playback.h"
#include "pragha-window.h"
#include "pragha.h"

/*****************/
/* Playlist Tree */
/*****************/

static GtkWidget*
create_playlist_pane_view(struct con_win *cwin)
{
	GtkWidget *vbox;
	PraghaPlaylist *cplaylist;
	PraghaStatusbar *statusbar;

	vbox = gtk_vbox_new(FALSE, 2);

	statusbar = pragha_statusbar_get();
	cplaylist = pragha_playlist_new(cwin);

	/* Pack everything */

	gtk_box_pack_start(GTK_BOX(vbox),
			   pragha_playlist_get_widget(cplaylist),
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(statusbar),
			   FALSE,
			   FALSE,
			   0);

	/* Store references*/

	cwin->cplaylist = cplaylist;
	cwin->statusbar = statusbar;

	return vbox;
}

/********************************/
/* Externally visible functions */
/********************************/

GtkWidget* create_main_region(struct con_win *cwin)
{
	GtkWidget *hpane;
	GtkWidget *browse_mode, *playlist_pane;

	/* A two paned container */

	hpane = gtk_hpaned_new();

	/* Left pane contains a notebook widget holding the various views */

	browse_mode = pragha_sidebar_get_widget(cwin->sidebar);

	/* Set initial sizes */

	gtk_paned_set_position (GTK_PANED (hpane),
		pragha_preferences_get_sidebar_size(cwin->preferences));

	/* Right pane contains the current playlist */

	playlist_pane = create_playlist_pane_view(cwin);

	/* Pack everything into the hpane */

	gtk_paned_pack1 (GTK_PANED (hpane), browse_mode, FALSE, TRUE);
	gtk_paned_pack2 (GTK_PANED (hpane), playlist_pane, TRUE, FALSE);

	/* Store references*/

	cwin->paned = hpane;

	gtk_widget_show_all(hpane);

	return hpane;
}

GtkWidget* create_info_box(struct con_win *cwin)
{
	GtkWidget *info_box;

	info_box = gtk_vbox_new(FALSE, 0);

	cwin->info_box = info_box;

	return info_box;
}

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
		pragha_quit(cwin);
	}
	return TRUE;
}

void
pragha_destroy_window(GtkWidget *widget, struct con_win *cwin)
{
	pragha_quit(cwin);
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

void mainwindow_add_widget_to_info_box(struct con_win *cwin, GtkWidget *widget)
{
	GList *list;
	GtkWidget *children;

	list = gtk_container_get_children (GTK_CONTAINER(cwin->info_box));

	if(list) {
		children = list->data;
		gtk_container_remove(GTK_CONTAINER(cwin->info_box), children);
		gtk_widget_destroy(GTK_WIDGET(children));
		g_list_free(list);
	}
		
	gtk_container_add(GTK_CONTAINER(cwin->info_box), widget);
}

void gui_free (struct con_win *cwin)
{
	gint *window_size, *window_position;
	gint win_width, win_height, win_x, win_y;
	GdkWindowState state;
	const gchar *user_config_dir;
	gchar *pragha_accels_path = NULL;

	/* Save last window state */

	if (pragha_preferences_get_remember_state(cwin->preferences)) {
		state = gdk_window_get_state (gtk_widget_get_window (cwin->mainwindow));
		if (state & GDK_WINDOW_STATE_FULLSCREEN)
			pragha_preferences_set_start_mode(cwin->preferences, FULLSCREEN_STATE);
		else if(state & GDK_WINDOW_STATE_WITHDRAWN)
			pragha_preferences_set_start_mode(cwin->preferences, ICONIFIED_STATE);
		else
			pragha_preferences_set_start_mode(cwin->preferences, NORMAL_STATE);
	}

	/* Save geometry only if window is not maximized or fullscreened */

	if (!(state & GDK_WINDOW_STATE_MAXIMIZED) || !(state & GDK_WINDOW_STATE_FULLSCREEN)) {
		window_size = g_new0(gint, 2);
		gtk_window_get_size(GTK_WINDOW(cwin->mainwindow),
				    &win_width,
				    &win_height);
		window_size[0] = win_width;
		window_size[1] = win_height;

		window_position = g_new0(gint, 2);
		gtk_window_get_position(GTK_WINDOW(cwin->mainwindow),
					&win_x,
					&win_y);
		window_position[0] = win_x;
		window_position[1] = win_y;

		pragha_preferences_set_integer_list (cwin->preferences,
		                                     GROUP_WINDOW,
		                                     KEY_WINDOW_SIZE,
		                                     window_size,
		                                     2);

		pragha_preferences_set_integer_list (cwin->preferences,
		                                     GROUP_WINDOW,
		                                     KEY_WINDOW_POSITION,
		                                     window_position,
		                                     2);

		g_free(window_size);
		g_free(window_position);
	}

	/* Save sidebar size */

	pragha_preferences_set_sidebar_size(cwin->preferences,
		gtk_paned_get_position(GTK_PANED(cwin->paned)));


	/* Save menu accelerators edited */

	user_config_dir = g_get_user_config_dir();
	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha/accels.scm", NULL);
	gtk_accel_map_save (pragha_accels_path);

	/* Free memory */

	if (cwin->pixbuf_app)
		g_object_unref(cwin->pixbuf_app);
	g_free(pragha_accels_path);
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
			pragha_backend_stop (cwin->backend);
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

	const gchar *file = pragha_musicobject_get_file (pragha_backend_get_musicobject (cwin->backend));

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
			 G_CALLBACK(backend_error_dialog_response_cb),
			 cwin);

	gtk_widget_show_all(dialog);
}

void
gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin)
{
	update_current_playlist_view_new_track (cwin->cplaylist, backend);
}
