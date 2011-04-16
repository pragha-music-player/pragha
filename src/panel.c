/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
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

#include "consonance.h"

/* Get the first image file from the directory and create a pixbuf of that file */

static GdkPixbuf* get_image_from_dir(gchar *path, struct con_win *cwin)
{
	GdkPixbuf *image = NULL;
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL;

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR) &&
		    is_image_file(ab_file)) {
			CDEBUG(DBG_INFO, "Image file: %s", ab_file);
			image = gdk_pixbuf_new_from_file_at_scale(ab_file,
								  ALBUM_ART_SIZE,
								  ALBUM_ART_SIZE,
								  FALSE,
								  &error);
			if (!image) {
				g_critical("Unable to open image file: %s",
					   ab_file);
				g_error_free(error);
			}
			g_free(ab_file);
			break;
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	return image;
}

/* Find out if any of the preferred album art files are present in the given dir.
   Runs through the patterns in sequence */

static GdkPixbuf* get_pref_image_dir(gchar *path, struct con_win *cwin)
{
	GdkPixbuf *image = NULL;
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL, **pattern;
	GSList *file_list = NULL, *l;
	gint i = 0;

	/* Form a list of all files in the given dir */

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR)) {
			file_list = g_slist_append(file_list, g_strdup(next_file));

		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	/* Now, run the preferred patterns through them */

	pattern = g_strsplit(cwin->cpref->album_art_pattern, ";",
			     ALBUM_ART_NO_PATTERNS);
	while (pattern[i]) {
		if (is_present_str_list(pattern[i], file_list)) {
			ab_file = g_strconcat(path, "/", pattern[i], NULL);
			if (is_image_file(ab_file)) {
				CDEBUG(DBG_INFO, "Image file: %s", ab_file);
				image = gdk_pixbuf_new_from_file_at_scale(ab_file,
							  ALBUM_ART_SIZE,
							  ALBUM_ART_SIZE,
							  FALSE,
							  &error);
				if (!image) {
					g_critical("Unable to open image file: %s\n",
						   ab_file);
					g_error_free(error);
					g_free(ab_file);
					i++;
					continue;
				}
				g_free(ab_file);
				break;
			}
			g_free(ab_file);
		}
		i++;
	}

	/* Cleanup */

	l = file_list;
	while (l) {
		g_free(l->data);
		l = l->next;
	}
	g_slist_free(file_list);
	g_strfreev(pattern);

	return image;
}

void __update_current_song_info(struct con_win *cwin, gint length)
{
	gchar *str = NULL;
	gchar *tot_length = NULL, *cur_pos = NULL;

	if (!cwin->cstate->curr_mobj) {
		g_critical("Curr mobj is invalid");
		return;
	}

	cur_pos = convert_length_str(length);
	tot_length = convert_length_str(cwin->cstate->curr_mobj->tags->length);
	str = g_strdup_printf("%s [ %s / %s ]",
			      cwin->cstate->curr_mobj->tags->title,
			      cur_pos,
			      tot_length);

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cwin->track_progress_bar),
				  (const gchar*)str);

	g_free(str);
	g_free(tot_length);
	g_free(cur_pos);
}

void unset_current_song_info(struct con_win *cwin)
{
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cwin->track_progress_bar),
				  TRACK_PROGRESS_BAR_STOPPED);
}

void __update_track_progress_bar(struct con_win *cwin, gint length)
{
	gdouble fraction = 0;

	fraction = (gdouble)length / (gdouble)cwin->cstate->curr_mobj->tags->length;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar),
				      fraction);
}

void unset_track_progress_bar(struct con_win *cwin)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), 0);
}

void track_progress_change_cb(GtkWidget *widget,
			      GdkEventButton *event,
			      struct con_win *cwin)
{
	gint seek = 0;
	gdouble fraction = 0;

	if (event->button != 1)
		return;

	if (cwin->cstate->state != ST_PLAYING)
		return;

	if (!cwin->cstate->curr_mobj)
		return;

	fraction = event->x / widget->allocation.width;
	seek = (cwin->cstate->curr_mobj->tags->length * event->x) / widget->allocation.width;
	if (seek >= cwin->cstate->curr_mobj->tags->length)
		seek = cwin->cstate->curr_mobj->tags->length;

	seek_playback(cwin, seek, fraction);
}

void update_album_art(struct musicobject *mobj, struct con_win *cwin)
{
	GdkPixbuf *album_art;
	gchar *dir;

	if (cwin->cpref->show_album_art) {

		/* Destroy previous album art image */

		if (cwin->album_art) {
			gtk_widget_destroy(cwin->album_art);
			cwin->album_art = NULL;
		}
		if (mobj && mobj->file_type != FILE_CDDA) {
			dir = g_path_get_dirname(mobj->file);
			if (cwin->cpref->album_art_pattern) {
				album_art = get_pref_image_dir(dir, cwin);
				if (!album_art) {
					album_art = get_image_from_dir(dir, cwin);
				}
			} else {
				album_art = get_image_from_dir(dir, cwin);
			}

			if (album_art) {
				cwin->album_art = gtk_image_new_from_pixbuf(album_art);
				g_object_unref(G_OBJECT(album_art));
			}
			else
				cwin->album_art = gtk_image_new_from_stock(
					GTK_STOCK_INFO,
					GTK_ICON_SIZE_LARGE_TOOLBAR);

			gtk_container_add(GTK_CONTAINER(cwin->album_art_frame),
					  GTK_WIDGET(cwin->album_art));
			gtk_widget_show_all(cwin->album_art_frame);

			g_free(dir);
		}
	}
}

/* Unset album art */

void unset_album_art(struct con_win *cwin)
{
	if (cwin->album_art) {
		gtk_widget_destroy(cwin->album_art);
		cwin->album_art = NULL;
	}
}

void shuffle_button_handler(GtkToggleButton *button, struct con_win *cwin)
{
	GtkTreeRowReference *ref;

	cwin->cpref->shuffle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	current_playlist_clear_dirty_all(cwin);

 	if (!cwin->cpref->shuffle) {
		CDEBUG(DBG_INFO, "Turning shuffle off");
		cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
		if (cwin->cstate->curr_rand_ref)
			cwin->cstate->curr_seq_ref =
				gtk_tree_row_reference_copy(cwin->cstate->curr_rand_ref);
		else
			cwin->cstate->curr_seq_ref = NULL;
	}
	else if (cwin->cpref->shuffle) {
		CDEBUG(DBG_INFO, "Turning shuffle on");
		if (cwin->cstate->curr_seq_ref) {
			ref = gtk_tree_row_reference_copy(cwin->cstate->curr_seq_ref);
			cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist - 1;
			reset_rand_track_refs(ref, cwin);
		}
	}
}

void repeat_button_handler(GtkToggleButton *button, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Repeat handler");
	cwin->cpref->repeat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
}

void play_button_handler(GtkButton *button, struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	GThread *thread;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeRowReference *ref;

	/* New action is based on the current state */

	/************************************/
        /* State     Action		    */
	/* 				    */
	/* Playing   Pause playback	    */
	/* Paused    Resume playback	    */
	/* Stopped   Start playback	    */
        /************************************/

	switch (cwin->cstate->state) {
	case ST_PLAYING:
		pause_playback(cwin);
		break;
	case ST_PAUSED:
		resume_playback(cwin);
		break;
	case ST_STOPPED:
		path = current_playlist_get_selection(cwin);
		if (!path) {
			play_first_current_playlist(cwin);
			break;
		}

		if (cwin->cpref->shuffle) {
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
			ref = gtk_tree_row_reference_new(model, path);
			reset_rand_track_refs(ref, cwin);
			cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
		}

		mobj = current_playlist_mobj_at_path(path, cwin);
		thread = start_playback(mobj, cwin);
		if (!thread)
			g_critical("Unable to create playback thread");
		else
			update_current_state(thread, path, PLAYLIST_CURR, cwin);

		gtk_tree_path_free(path);
		break;
	default:
		break;
	}
}

void stop_button_handler(GtkButton *button, struct con_win *cwin)
{
	stop_playback(cwin);
}

void prev_button_handler(GtkButton *button, struct con_win *cwin)
{
	play_prev_track(cwin);
}

void next_button_handler(GtkButton *button, struct con_win *cwin)
{
	play_next_track(cwin);
}

void vol_button_handler(GtkScaleButton *button, gdouble value, struct con_win *cwin)
{
	if (!cwin->cstate->audio_init)
		return;

	cwin->cmixer->curr_vol = value;
	cwin->cmixer->set_volume(cwin);
}

void play_button_toggle_state(struct con_win *cwin)
{
	if (cwin->cstate->state == ST_PLAYING)
		gtk_button_set_image(GTK_BUTTON(cwin->play_button),
				     cwin->pixbuf->image_pause);
	else if ((cwin->cstate->state == ST_PAUSED) ||
		 (cwin->cstate->state == ST_STOPPED))
		gtk_button_set_image(GTK_BUTTON(cwin->play_button),
				     cwin->pixbuf->image_play);
}	

/* Toggle appearance of album art widget */

void album_art_toggle_state(struct con_win *cwin)
{
	if (cwin->cpref->show_album_art) {
		if (!cwin->album_art_frame) {
			cwin->album_art_frame = gtk_frame_new(NULL);
			gtk_box_pack_start(GTK_BOX(cwin->hbox_panel),
					   GTK_WIDGET(cwin->album_art_frame),
					   FALSE, FALSE, 2);
			gtk_box_reorder_child(GTK_BOX(cwin->hbox_panel),
					      cwin->album_art_frame,
					      0);
		}
		gtk_widget_show_now(cwin->album_art_frame);
		resize_album_art_frame(cwin);
		if (cwin->cstate->state != ST_STOPPED)
			update_album_art(cwin->cstate->curr_mobj, cwin);
	}
	else
		if (cwin->album_art_frame)
			gtk_widget_hide(cwin->album_art_frame);
}

/* Set the initial width of the frame to the default height */

void resize_album_art_frame(struct con_win *cwin)
{
	if (cwin->album_art_frame)
		gtk_widget_set_size_request(GTK_WIDGET(cwin->album_art_frame),
					    ALBUM_ART_SIZE,
					    ALBUM_ART_SIZE);
}
