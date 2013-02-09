/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

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
	cplaylist = cplaylist_new(cwin);

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

/*****************/
/* DnD functions */
/*****************/
/* These two functions are only callbacks that must be passed to
gtk_tree_selection_set_select_function() to chose if GTK is allowed
to change selection itself or if we handle it ourselves */

gboolean tree_selection_func_true(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data)
{
	return TRUE;
}

gboolean tree_selection_func_false(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data)
{
	return FALSE;
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

	/* Right pane contains the current playlist */

	playlist_pane = create_playlist_pane_view(cwin);

	/* Set initial sizes */

	gtk_paned_set_position (GTK_PANED (hpane), cwin->cpref->sidebar_size);

	/* Pack everything into the hpane */

	gtk_paned_pack1 (GTK_PANED (hpane), browse_mode, FALSE, TRUE);
	gtk_paned_pack2 (GTK_PANED (hpane), playlist_pane, TRUE, FALSE);

	/* Store references*/

	cwin->paned = hpane;

	return hpane;
}

static void
buffering_cb (PraghaBackend *backend, gint percent, gpointer user_data)
{
	GtkProgressBar *track_progress_bar = user_data;
	gtk_progress_bar_set_fraction(track_progress_bar, (gdouble)percent/100);
}

GtkWidget* create_playing_box(struct con_win *cwin)
{
	GtkWidget *now_playing_label,*track_length_label,*track_time_label;
	GtkWidget *track_progress_bar;
	GtkWidget *track_length_align, *track_time_align, *track_progress_align, *vbox_align;
	GtkWidget *pack_vbox, *playing_hbox, *time_hbox;
	GtkWidget *track_length_event_box, *track_playing_event_box;

	#if GTK_CHECK_VERSION (3, 0, 0)
	GtkWidget *track_progress_event_box;
	#endif
 
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *ntag_lastfm_button;
	#endif

	playing_hbox = gtk_hbox_new(FALSE, 2);

	track_playing_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_playing_event_box), FALSE);

	g_signal_connect (G_OBJECT(track_playing_event_box), "button-press-event",
			G_CALLBACK(edit_tags_playing_event), cwin);

	now_playing_label = gtk_label_new(NULL);
	gtk_label_set_ellipsize (GTK_LABEL(now_playing_label), PANGO_ELLIPSIZE_END);
	gtk_label_set_markup(GTK_LABEL(now_playing_label),_("<b>Not playing</b>"));
	gtk_misc_set_alignment (GTK_MISC(now_playing_label), 0, 1);

	gtk_container_add(GTK_CONTAINER(track_playing_event_box), now_playing_label);

	#ifdef HAVE_LIBCLASTFM
	ntag_lastfm_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(ntag_lastfm_button), GTK_RELIEF_NONE);
	gtk_button_set_image(GTK_BUTTON(ntag_lastfm_button),
			     gtk_image_new_from_stock(GTK_STOCK_SPELL_CHECK,
						      GTK_ICON_SIZE_MENU));
	gtk_widget_set_tooltip_text(GTK_WIDGET(ntag_lastfm_button), _("Last.fm suggested a tag correction"));
	g_signal_connect(G_OBJECT(ntag_lastfm_button), "clicked",
	                 G_CALLBACK(edit_tags_corrected_by_lastfm), cwin);
	#endif

	gtk_box_pack_start(GTK_BOX(playing_hbox),
			   GTK_WIDGET(track_playing_event_box),
			   TRUE, TRUE, 0);
	#ifdef HAVE_LIBCLASTFM
	gtk_box_pack_start(GTK_BOX(playing_hbox),
			   GTK_WIDGET(ntag_lastfm_button),
			   FALSE, FALSE, 0);
	#endif

	time_hbox = gtk_hbox_new(FALSE, 2);

	/* Setup track progress */

	track_progress_align = gtk_alignment_new(0, 0.5, 1, 0);

	#if GTK_CHECK_VERSION (3, 0, 0)
	track_progress_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_progress_event_box), FALSE);
	#endif

	track_progress_bar = gtk_progress_bar_new();

	gtk_widget_set_size_request(GTK_WIDGET(track_progress_bar), -1, 12);

	#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_event_box);
	gtk_container_add(GTK_CONTAINER(track_progress_event_box), track_progress_bar);

	g_signal_connect(G_OBJECT(track_progress_event_box), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);
	#else
	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_bar);
	gtk_widget_set_events(track_progress_bar, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(G_OBJECT(track_progress_bar), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);
	#endif
	g_signal_connect (cwin->backend, "buffering", G_CALLBACK(buffering_cb), track_progress_bar);

	track_time_label = gtk_label_new(NULL);
	track_length_label = gtk_label_new(NULL);

	track_time_align = gtk_alignment_new(1, 0.5, 0, 0);
	track_length_align = gtk_alignment_new(0, 0.5, 0, 0);

	gtk_container_add(GTK_CONTAINER(track_time_align), track_time_label);
	gtk_container_add(GTK_CONTAINER(track_length_align), track_length_label);

	gtk_label_set_markup(GTK_LABEL(track_length_label),"<small>--:--</small>");
	gtk_label_set_markup(GTK_LABEL(track_time_label),"<small>00:00</small>");

	track_length_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_length_event_box), FALSE);

	g_signal_connect (G_OBJECT(track_length_event_box), "button-press-event",
			G_CALLBACK(timer_remaining_mode_change), cwin);
	gtk_container_add(GTK_CONTAINER(track_length_event_box), track_length_align);

	cwin->track_progress_bar = 	track_progress_bar;
	cwin->now_playing_label = 	now_playing_label;
	#ifdef HAVE_LIBCLASTFM
	cwin->ntag_lastfm_button =	ntag_lastfm_button;
	#endif
	cwin->track_time_label =	track_time_label;
	cwin->track_length_label = 	track_length_label;

	gtk_box_pack_start(GTK_BOX(time_hbox),
			   GTK_WIDGET(track_time_align),
			   FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(time_hbox),
 			   GTK_WIDGET(track_progress_align),
 			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(time_hbox),
			   GTK_WIDGET(track_length_event_box),
			   FALSE, FALSE, 3);

	pack_vbox = gtk_vbox_new(FALSE, 1);

	gtk_box_pack_start(GTK_BOX(pack_vbox),
			   GTK_WIDGET(playing_hbox),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pack_vbox),
			   GTK_WIDGET(time_hbox),
			   FALSE, FALSE, 0);

	vbox_align = gtk_alignment_new(0.5, 0.5, 1, 0);
	gtk_container_add(GTK_CONTAINER(vbox_align), pack_vbox);

	return vbox_align;
}

GtkWidget* create_info_box(struct con_win *cwin)
{
	GtkWidget *info_box;

	info_box = gtk_vbox_new(FALSE, 0);

	cwin->info_box = info_box;

	return info_box;
}

gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin)
{
	if(cwin->cpref->close_to_tray) {
		if(cwin->cpref->show_icon_tray &&
		   gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon)))
			toogle_main_window(cwin, FALSE);
		else
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
	}
	else {
		exit_pragha(widget, cwin);
	}
	return TRUE;
}

void toogle_main_window (struct con_win *cwin, gboolean ignoreActivity)
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
	gtk_container_add(GTK_CONTAINER(cwin->info_box), widget);
}

void gui_free (struct con_win *cwin)
{
	const gchar *user_config_dir;
	gchar *pragha_accels_path = NULL;

	/* Save menu accelerators edited */

	user_config_dir = g_get_user_config_dir();
	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha/accels.scm", NULL);
	gtk_accel_map_save (pragha_accels_path);

	/* Free memory */

	if (cwin->pixbuf_app)
		g_object_unref(cwin->pixbuf_app);
	gtk_widget_destroy(GTK_WIDGET(cwin->albumart));
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
	gchar *file = NULL;

	struct con_win *cwin = user_data;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "file", &file,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

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

	g_free(file);
}

void
gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin)
{
	update_current_playlist_view_new_track (cwin->cplaylist, backend);
}
