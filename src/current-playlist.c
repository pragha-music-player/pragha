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

#include <string.h>
#include "pragha.h"

/*********************/
/* General functions */
/*********************/

/* Update playback state pixbuf */

void update_pixbuf_state_on_path (GtkTreePath *path, GError *error, struct con_win *cwin)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf = NULL;
	GtkIconTheme *icon_theme;

	if(cwin->cstate->playlist_change)
		return;

	if (error) {
		icon_theme = gtk_icon_theme_get_default ();
		if(error->code == GST_RESOURCE_ERROR_NOT_FOUND)
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "gtk-remove",16, 0, NULL);
		else
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "gtk-dialog-warning",16, 0, NULL);
	}
	else {
		switch (cwin->cstate->state)
		{
			case ST_PLAYING:
				pixbuf = cwin->pixbuf->pixbuf_playing;
				break;
			case ST_PAUSED:
				pixbuf = cwin->pixbuf->pixbuf_paused;
				break;
			default:
				break;
		}
	}
	if (path != NULL) {
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->current_playlist));
		if (gtk_tree_model_get_iter (model, &iter, path)) {
			gtk_list_store_set (GTK_LIST_STORE(model), &iter, P_STATUS_PIXBUF, pixbuf, -1);
		}
	}

	if (error)
		g_object_unref (pixbuf);
}

static gchar* get_display_name(struct musicobject *mobj)
{
	gchar *name = NULL;
	
	if (mobj->file_type == FILE_CDDA) {
		name = g_strdup(mobj->file);
	} else {
		name = get_display_filename(mobj->file, FALSE);
	}
	return name;
}

static gint get_total_playtime(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint total_playtime = 0;
	struct musicobject *mobj = NULL;
	gboolean ret;

	if(cwin->cstate->playlist_change)
		return 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj)
			total_playtime += mobj->tags->length;
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return total_playtime;
}

/* Update status bar */

void update_status_bar(struct con_win *cwin)
{
	gint total_playtime = 0;
	gchar *str, *tot_str;

	if(cwin->cstate->playlist_change)
		return;

	total_playtime = get_total_playtime(cwin);
	tot_str = convert_length_str(total_playtime);
	str = g_strdup_printf("%i %s - %s",
				cwin->cstate->tracks_curr_playlist,
				(cwin->cstate->tracks_curr_playlist>1)?_("Tracks"):_("Track"),
				tot_str);

	CDEBUG(DBG_VERBOSE, "Updating status bar with new playtime: %s", tot_str);

	gtk_label_set_text(GTK_LABEL(cwin->status_bar), str);

	g_free(tot_str);
	g_free(str);
}

/* Clear current seq ref */

static void clear_curr_seq_ref(struct con_win *cwin)
{
	if (!cwin->cstate->curr_seq_ref)
		return;

	gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
	cwin->cstate->curr_seq_ref = NULL;
}

/* Clear cstate->curr_seq_ref if it happens to contain the given path */

static void test_clear_curr_seq_ref(GtkTreePath *path, struct con_win *cwin)
{
	GtkTreePath *lpath;

	if (!cwin->cstate->curr_seq_ref)
		return;

	lpath = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	if (!gtk_tree_path_compare(path, lpath)) {
		gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
		cwin->cstate->curr_seq_ref = NULL;
	}
	gtk_tree_path_free(lpath);
}

/* Check if given ref is the current rand reference */

static gboolean is_current_rand_ref(GtkTreeRowReference *ref, struct con_win *cwin)
{
	if (ref == cwin->cstate->curr_rand_ref)
		return TRUE;
	else
		return FALSE;
}

void requeue_track_refs (struct con_win *cwin)
{
	GSList *list = NULL;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GtkTreePath *lpath;
	gchar *ch_queue_no=NULL;
	GtkTreeIter iter;
	gint i=0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	list = cwin->cstate->queue_track_refs;

	while (list) {
		ref = list->data;
		lpath = gtk_tree_row_reference_get_path(ref);
		if (gtk_tree_model_get_iter(model, &iter, lpath)){
			ch_queue_no = g_strdup_printf("%d", ++i);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_QUEUE, ch_queue_no, -1);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_BUBBLE, TRUE, -1);
			g_free(ch_queue_no);
		}
		gtk_tree_path_free(lpath);
		list = list->next;
	}
}

/* Delete the ref corresponding to the given path */

void delete_queue_track_refs(GtkTreePath *path, struct con_win *cwin)
{
	GSList *list = NULL;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GtkTreePath *lpath;
	GtkTreeIter iter;

	if (cwin->cstate->queue_track_refs) {
		list = cwin->cstate->queue_track_refs;
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);
			if (!gtk_tree_path_compare(path, lpath)) {
				gtk_tree_row_reference_free(ref);
				cwin->cstate->queue_track_refs = g_slist_remove_all(cwin->cstate->queue_track_refs, ref);
			}
			if (gtk_tree_model_get_iter(model, &iter, lpath)){
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_QUEUE, NULL, -1);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_BUBBLE, FALSE, -1);
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
	}
}

/* Delete the ref corresponding to the given path */

void delete_rand_track_refs(GtkTreePath *path, struct con_win *cwin)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (cwin->cstate->rand_track_refs) {
		list = cwin->cstate->rand_track_refs;
		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);
			if (!gtk_tree_path_compare(path, lpath)) {
				if (is_current_rand_ref(ref, cwin))
					cwin->cstate->curr_rand_ref = NULL;
				gtk_tree_row_reference_free(ref);
				cwin->cstate->rand_track_refs =
					g_list_remove(cwin->cstate->rand_track_refs,
						      ref);
				gtk_tree_path_free(lpath);
				break;
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
	}
}

/* Return the next node after the given ref */

static GtkTreeRowReference* get_rand_ref_next(GtkTreeRowReference *ref,
					      struct con_win *cwin)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	list = cwin->cstate->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->next->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return the prev node of the given ref */

static GtkTreeRowReference* get_rand_ref_prev(GtkTreeRowReference *ref,
					      struct con_win *cwin)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	list = cwin->cstate->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->prev->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return path of track at nth position in current playlist */

static GtkTreePath* current_playlist_nth_track(gint n, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint pos = 0;

	if(cwin->cstate->playlist_change)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	do {
		if (pos == n) {
			path = gtk_tree_model_get_path(model, &iter);
			break;
		}
		pos++;
	}
	while (gtk_tree_model_iter_next(model, &iter));

	return path;
}

/* Return path of the next queue track */

GtkTreePath* get_next_queue_track(struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	path = gtk_tree_row_reference_get_path(cwin->cstate->queue_track_refs->data);

	delete_queue_track_refs(path, cwin);
	requeue_track_refs (cwin);

	return path;
}

/* Return path of a first random track */

GtkTreePath* get_first_random_track(struct con_win *cwin)
{
	gint rnd;
	GtkTreePath *path = NULL;

	do {
		rnd = g_rand_int_range(cwin->cstate->rand,
				       0,
				       cwin->cstate->tracks_curr_playlist);
		path = current_playlist_nth_track(rnd, cwin);

	} while (cwin->cstate->tracks_curr_playlist > 1 && (path == NULL));

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next unique random track */

static GtkTreePath* get_next_unplayed_random_track(struct con_win *cwin)
{
	gint rnd;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gboolean played = TRUE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	while (played && cwin->cstate->unplayed_tracks) {
		rnd = g_rand_int_range(cwin->cstate->rand,
				       0,
				       cwin->cstate->tracks_curr_playlist);
		path = current_playlist_nth_track(rnd, cwin);
		if (!path) {
			g_printerr("No track at position : %d\n", rnd);
			return NULL;
		}

		if (gtk_tree_model_get_iter(model, &iter, path))
			gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);

		if (played) {
			gtk_tree_path_free(path);
			path = NULL;
		}
	}
	return path;
}

/* Return path of next random track,
   this is called after exhausting all unique tracks */

static GtkTreePath* get_next_random_track(struct con_win *cwin)
{
	gint rnd;
	GtkTreePath *path = NULL, *rpath;

	rpath = gtk_tree_row_reference_get_path(cwin->cstate->curr_rand_ref);
	do {
		rnd = g_rand_int_range(cwin->cstate->rand,
				       0,
				       cwin->cstate->tracks_curr_playlist);
		path = current_playlist_nth_track(rnd, cwin);
	} while (!gtk_tree_path_compare(rpath, path) &&
		 (cwin->cstate->tracks_curr_playlist > 1));

	gtk_tree_path_free(rpath);

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next sequential track */

static GtkTreePath* get_next_sequential_track(struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	gboolean ret = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	/* If no tracks, return NULL.
	   If current track has been removed from the playlist,
	   return the first track. */

	if (!cwin->cstate->curr_seq_ref && !ret)
		return NULL;
	else if (!cwin->cstate->curr_seq_ref && ret) {
		path = gtk_tree_model_get_path(model, &iter);
		return path;
	}

	path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	gtk_tree_model_get_iter(model, &iter, path);

	if (!gtk_tree_model_iter_next(model, &iter)) {
		gtk_tree_path_free(path);
		path = NULL;
	}
	else {
		gtk_tree_path_free(path);
		path = gtk_tree_model_get_path(model, &iter);
	}

	return path;
}

/* Return path of next track in the list cstate->rand_track_refs */
/* This is called when the user clicks 'next' after one/more 'prev(s)' */

static GtkTreePath* get_next_random_ref_track(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	i = g_list_find(cwin->cstate->rand_track_refs, cwin->cstate->curr_rand_ref);
	if (i) {
		j = g_list_next(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}
	return path;
}

/* Return path of the node previous to the current track from
   cstate->rand_track_refs */

static GtkTreePath* get_prev_random_track(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	if (!cwin->cstate->rand_track_refs)
		return NULL;

	i = g_list_find(cwin->cstate->rand_track_refs, cwin->cstate->curr_rand_ref);
	if (i) {
		j = g_list_previous(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}

	return path;
}

/* Return path of the previous sequential track */

static GtkTreePath* get_prev_sequential_track(struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;

	if (!cwin->cstate->curr_seq_ref)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);
	gtk_tree_model_get_iter(model, &iter, path);

	if (!gtk_tree_path_prev(path)) {
		gtk_tree_path_free(path);
		path = NULL;
	}

	return path;
}

/* Add a new ref to cstate->rand_track_refs */

static void append_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin)
{
	cwin->cstate->rand_track_refs = g_list_append(cwin->cstate->rand_track_refs,
						      ref);
}

/* Remove all nodes and free the list */

static void clear_rand_track_refs(struct con_win *cwin)
{
	GList *list;

	if (cwin->cstate->rand_track_refs) {
		list = cwin->cstate->rand_track_refs;
		while (list) {
			gtk_tree_row_reference_free(list->data);
			list = list->next;
		}
		g_list_free(cwin->cstate->rand_track_refs);
		cwin->cstate->rand_track_refs = NULL;
	}

	cwin->cstate->curr_rand_ref = NULL;
}

/* Remove all nodes and free the list */

static void clear_queue_track_refs(struct con_win *cwin)
{
	GSList *list = NULL;

	if (cwin->cstate->queue_track_refs) {
		list = cwin->cstate->queue_track_refs;
		while (list) {
			gtk_tree_row_reference_free(list->data);
			list = list->next;
		}
		g_slist_free(cwin->cstate->queue_track_refs);
		cwin->cstate->queue_track_refs = NULL;
	}

	cwin->cstate->queue_track_refs = NULL;
}

/* Mark a track in current playlist as dirty */

static void current_playlist_set_dirty_track(GtkTreePath *path, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, TRUE, -1);
}

/* Comparison function for column names */

static gint compare_playlist_column_name(gconstpointer a, gconstpointer b)
{
	const gchar *e1 = a;
	const gchar *e2 = b;

	return g_ascii_strcasecmp(e1, e2);
}

/* Function to add/delete columns from preferences */

static void modify_current_playlist_columns(struct con_win *cwin,
					    const gchar *col_name,
					    gboolean state)
{
	gboolean pref_present;
	GSList *element;

	if (!col_name) {
		g_warning("Invalid column name");
		return;
	}

	pref_present = is_present_str_list(col_name, cwin->cpref->playlist_columns);

	/* Already in preferences */

	if (pref_present && state) {
		return;
	}

	/* Remove from preferences */

	else if (pref_present && !state) {
		element = g_slist_find_custom(cwin->cpref->playlist_columns,
				      col_name, compare_playlist_column_name);
		if (element) {
			g_free(element->data);
			cwin->cpref->playlist_columns =
				g_slist_delete_link(cwin->cpref->playlist_columns,
						    element);
		}
		else
			g_warning("Column : %s not found in preferences",
				  col_name);
	}

	/* Add to preferences */
 
	else if (!pref_present && state) { 
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(col_name));
	}
}

/* Build a dialog to get a new playlist name */

static gchar* get_playlist_dialog(enum playlist_mgmt *choice,
				  enum playlist_mgmt type,
				  struct con_win *cwin)
{
	gchar **playlists, *playlist = NULL;
	gint result, i=0;
	GtkWidget *dialog, *radio_new, *radio_add;
	GtkWidget *hbox, *vbox1, *vbox2;
	GtkWidget *entry;
	GtkWidget *combo_add;

	/* Retrieve list of playlist names from DB */

	playlists = get_playlist_names_db(cwin);

	/* Create dialog window */

	hbox = gtk_hbox_new(TRUE, 2);
	vbox1 = gtk_vbox_new(TRUE, 2);
	vbox2 = gtk_vbox_new(TRUE, 2);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	combo_add = gtk_combo_box_text_new();
	radio_new = gtk_radio_button_new_with_label(NULL, _("Save as a new playlist"));
	radio_add = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(radio_new), _("Append to an existing playlist"));

	if (playlists) {
		while (playlists[i]) {
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_add),
						  playlists[i]);
			i++;
		}
		g_strfreev(playlists);
	}
	else {
		gtk_widget_set_sensitive(combo_add, FALSE);
		gtk_widget_set_sensitive(radio_add, FALSE);
	}

	dialog = gtk_dialog_new_with_buttons(NULL,
			     GTK_WINDOW(cwin->mainwindow),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
			     NULL);

	if(type == SAVE_COMPLETE)
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save playlist"));
	else
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save selection"));

	gtk_box_pack_start(GTK_BOX(vbox1), radio_new, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), radio_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), combo_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 2);

	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(entry));

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Export"), GTK_RESPONSE_HELP);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		/* Get playlist name */
		/* Store a copy because the dialog box is destroyed on return */

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_new))) {
			playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			*choice = NEW_PLAYLIST;
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_add))) {
			playlist = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_add));
			*choice = APPEND_PLAYLIST;
		}
		break;
	case GTK_RESPONSE_HELP:
		playlist = g_strdup(_("New playlist"));
		*choice = EXPORT_PLAYLIST;
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return playlist;
}

/* Function to jump to track on current playlist */

void jump_to_path_on_current_playlist (GtkTreePath *path, struct con_win *cwin)
{
	GtkTreeSelection *selection;
	gint cx, cy, cnt_selected;

	GdkRectangle vrect;
	GdkRectangle crect;

	if (path) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
		cnt_selected = gtk_tree_selection_count_selected_rows(selection);

		if (cnt_selected > 1)
			return;

		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(GTK_TREE_SELECTION (selection), path);

		gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(cwin->current_playlist), &vrect);
		gtk_tree_view_get_cell_area(GTK_TREE_VIEW(cwin->current_playlist), path, NULL, &crect);

		gtk_tree_view_convert_widget_to_tree_coords(GTK_TREE_VIEW(cwin->current_playlist), crect.x, crect.y, &cx, &cy);

		if (cwin->cpref->shuffle) {
			if ((cy < vrect.y) || (cy + crect.height > vrect.y + vrect.height)) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cwin->current_playlist),
							     path, NULL, TRUE, 0.5, 0.0);
			}
		}
		else {
			if (cy < vrect.y) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cwin->current_playlist),
							     path, NULL, TRUE, 0.0, 0.0);
			}
			else if (cy + crect.height > vrect.y + vrect.height) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cwin->current_playlist),
							     path, NULL, TRUE, 1.0, 0.0);
			}
		}
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(cwin->current_playlist),
					 path, NULL, FALSE);
	}
}

/* Select last path. useful when change the model. */

void select_last_path_of_current_playlist(struct con_win *cwin)
{
	gchar *ref = NULL;
	GtkTreePath *path = NULL;

	ref = g_strdup_printf("%d", cwin->cstate->tracks_curr_playlist - 1);

	path = gtk_tree_path_new_from_string(ref);

	jump_to_path_on_current_playlist (path, cwin);

	gtk_tree_path_free(path);
	g_free(ref);
}

/* Get a new playlist name that is not reserved */

static gchar* get_playlist_name(struct con_win *cwin, enum playlist_mgmt type, enum playlist_mgmt *choice)
{
	gchar *playlist = NULL;
	enum playlist_mgmt sel = 0;

	do {
		playlist = get_playlist_dialog(&sel, type, cwin);
		if (playlist && !g_ascii_strcasecmp(playlist, SAVE_PLAYLIST_STATE)) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				_("<b>con_playlist</b> is a reserved playlist name"));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_free(playlist);
			continue;
		} else {
			break;
		}
	} while (1);

	*choice = sel;
	return playlist;
}

/*************************/
/* General playlist mgmt */
/*************************/

/* Update the state on 'Next', 'Prev' or selecting a new track
   from current playlist */
	
void update_current_state(GtkTreePath *path,
			  enum playlist_action action, struct con_win *cwin)
{
	GtkTreeRowReference *rand_ref;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	CDEBUG(DBG_VERBOSE, "Update the state from current playlist");

	/* Update view */
	update_pixbuf_state_on_path (path, NULL, cwin);
	jump_to_path_on_current_playlist (path, cwin);

	/* Update current song info */

	__update_current_song_info(cwin);
	__update_progress_song_info(cwin, 0);

	/* Update album art */

	update_album_art(cwin->cstate->curr_mobj, cwin);

	/* Show OSD */

	show_osd(cwin);

	/* Emit new state on mpris and pragha dbus. */
	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);

	/* Store reference */

	if (!cwin->cpref->shuffle) {
		gtk_tree_row_reference_free(cwin->cstate->curr_seq_ref);
		cwin->cstate->curr_seq_ref = gtk_tree_row_reference_new(model, path);
	}

	/* Append the new reference to the list of played track references
	   to retrace the sequence */

	if (cwin->cpref->shuffle) {
		switch (action) {
			/* If 'Prev', get the previous node from the track references */
			case PLAYLIST_PREV:
				if (cwin->cstate->curr_rand_ref) {
					cwin->cstate->curr_rand_ref =
						get_rand_ref_prev(cwin->cstate->curr_rand_ref,
								  cwin);
				}
			break;

			/* If 'Next', get the next node from the track references */
			/* Do this only if the current track and the
			   last node don't match */
			case PLAYLIST_NEXT:
				if (cwin->cstate->curr_rand_ref) {
					if (cwin->cstate->curr_rand_ref !=
					    (g_list_last(cwin->cstate->rand_track_refs)->data)) {
						cwin->cstate->curr_rand_ref =
							get_rand_ref_next(cwin->cstate->curr_rand_ref,
									  cwin);
						break;
					}
				}

			/* Append a new ref of the track to the track references */
			case PLAYLIST_CURR:
				rand_ref = gtk_tree_row_reference_new(model, path);
				cwin->cstate->rand_track_refs =
					g_list_append(cwin->cstate->rand_track_refs,
						      rand_ref);
				cwin->cstate->curr_rand_ref = rand_ref;
				break;
			default:
				break;
		}
	}

	/* Mark the track as dirty */

	current_playlist_set_dirty_track(path, cwin);
	if (cwin->cstate->unplayed_tracks)
		cwin->cstate->unplayed_tracks--;
}

/* Return musicobject of the given path */

struct musicobject* current_playlist_mobj_at_path(GtkTreePath *path,
						  struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	return mobj;
}

/* Return path of musicobject already in tree */

GtkTreePath* current_playlist_path_at_mobj(struct musicobject *mobj,
						struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *ptr = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	
	gtk_tree_model_get_iter_first(model, &iter);
	do {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &ptr, -1);
		if (ptr == mobj) {
			return gtk_tree_model_get_path(model, &iter);
		}
	} while(gtk_tree_model_iter_next(model, &iter));
	
	return NULL;
}

/* Reset random rand_refs and appends given ref */

void reset_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin)
{
	GtkTreePath *path;

	current_playlist_clear_dirty_all(cwin);
	clear_rand_track_refs(cwin);
	append_rand_track_refs(ref, cwin);
	cwin->cstate->curr_rand_ref = ref;
	path = gtk_tree_row_reference_get_path(ref);
	current_playlist_set_dirty_track(path, cwin);
	gtk_tree_path_free(path);
}

/* Mark all tracks in current playlist as clean */

void current_playlist_clear_dirty_all(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	ret = gtk_tree_model_get_iter_first(model, &iter);
	if (!ret)
		return;

	while(ret) {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, FALSE, -1);
		ret = gtk_tree_model_iter_next(model, &iter);
	}
}

/* Return the path of the selected track */

GtkTreePath* current_playlist_get_selection(struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreePath *path = NULL;
	gint cnt_selected = 0;
	GList *list;

	if(cwin->cstate->playlist_change)
		return NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	cnt_selected = gtk_tree_selection_count_selected_rows(selection);

	if (!cnt_selected)
		return NULL;

	else if (cnt_selected == 1) {
		list = gtk_tree_selection_get_selected_rows(selection, NULL);
		if (list) {
			path = list->data;
			g_list_free(list);
		}
	}
	else if (cnt_selected > 1)
		g_message("Selected multiple");

	return path;
}

/* Return the path of the next track to be played */

GtkTreePath* current_playlist_get_next(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GList *last;
	GtkTreeIter iter;
	gboolean rand_unplayed = FALSE, seq_last = FALSE;
	GtkTreeRowReference *ref;

	if(cwin->cstate->playlist_change)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	/* Check if tree is empty */

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	if(cwin->cstate->queue_track_refs){
		path = get_next_queue_track(cwin);
		ref = gtk_tree_row_reference_new(model, path);
		reset_rand_track_refs(ref, cwin);
		cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
	}
	else{
		switch (cwin->cpref->shuffle) {
			case TRUE:
				last = g_list_last(cwin->cstate->rand_track_refs);
				if ((!cwin->cstate->curr_rand_ref) || (last && (cwin->cstate->curr_rand_ref == last->data))){
					path = get_next_unplayed_random_track(cwin);
					if (!path)
						rand_unplayed = TRUE;
				}
				else path = get_next_random_ref_track(cwin);
				break;
			case FALSE:
				path = get_next_sequential_track(cwin);
				if (!path) seq_last = TRUE;
				break;
			default:
				break;
		}
	}
	if (rand_unplayed && cwin->cpref->repeat)
		path = get_next_random_track(cwin);

	if (seq_last && cwin->cpref->repeat)
		path = current_playlist_nth_track(0, cwin);

	return path;
}

/* Return the path of the next(prev) track to be played */

GtkTreePath* current_playlist_get_prev(struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	gboolean seq_first = FALSE;

	if(cwin->cstate->playlist_change)
		return NULL;

	switch (cwin->cpref->shuffle) {
	case TRUE:
		path = get_prev_random_track(cwin);
		break;
	case FALSE:
		path = get_prev_sequential_track(cwin);
		if (!path) seq_first = TRUE;
		break;
	default:
		break;
	}

	if (seq_first && cwin->cpref->repeat)
		path = current_playlist_nth_track((cwin->cstate->tracks_curr_playlist-1),
						  cwin);

	return path;
}

/* Return the path of the Actual track playing */

GtkTreePath* current_playlist_get_actual(struct con_win *cwin)
{
	GtkTreePath *path=NULL;

	if (cwin->cpref->shuffle && cwin->cstate->curr_rand_ref)
		path = gtk_tree_row_reference_get_path(cwin->cstate->curr_rand_ref);
	else if (!cwin->cpref->shuffle && cwin->cstate->curr_seq_ref)
		path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);

	return path;
}

void jump_to_playing_song_handler(GtkButton *button, struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	if(cwin->cstate->playlist_change)
		return;

	path = current_playlist_get_actual(cwin);

	jump_to_path_on_current_playlist (path, cwin);

	gtk_tree_path_free(path);
}

/* Dequeue selected rows from current playlist */

void dequeue_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *l;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	l = list;
	while (l) {
		path = l->data;
		delete_queue_track_refs(path, cwin);
		gtk_tree_path_free(path);
		l = l->next;
	}
	requeue_track_refs(cwin);
	g_list_free (list);
}

/* Queue selected rows from current playlist */

void queue_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeRowReference *ref;
	GList *list, *l;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	l= list;
	while (l) {
		path = l->data;
		ref = gtk_tree_row_reference_new(model, path);
		cwin->cstate->queue_track_refs = g_slist_append(cwin->cstate->queue_track_refs, ref);
		gtk_tree_path_free(path);
		l = l->next;
	}
	requeue_track_refs(cwin);
	g_list_free (list);
}

/* Toglle queue state of selection on current playlist. */

void toggle_queue_selected_current_playlist (struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean is_queue = FALSE;
	GList *list;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	while (list) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
			if(is_queue)
				delete_queue_track_refs(path, cwin);
			else {
				ref = gtk_tree_row_reference_new(model, path);
				cwin->cstate->queue_track_refs = g_slist_append(cwin->cstate->queue_track_refs, ref);
			}
		}
		gtk_tree_path_free(path);
		list = list->next;
	}
	requeue_track_refs(cwin);
	g_list_free (list);
}

/* Based on Totem Code */
int current_playlist_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeIter iter;
	GList *list;
	gint n_select = 0;
	gboolean is_queue = FALSE;

	/* Special case some shortcuts 
	if (event->state != 0) {
		if ((event->state & GDK_CONTROL_MASK)
		    && event->keyval == GDK_KEY_a) {
			gtk_tree_selection_select_all
				(playlist->priv->selection);
			return TRUE;
		}
	}*/
	/* If we have modifiers, and either Ctrl, Mod1 (Alt), or any
	 * of Mod3 to Mod5 (Mod2 is num-lock...) are pressed, we
	 * let Gtk+ handle the key */
	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK)))
		return FALSE;
	if (event->keyval == GDK_KEY_Delete){
		remove_from_playlist(NULL, cwin);
		return TRUE;
	}
	else if(event->keyval == GDK_KEY_q || event->keyval == GDK_KEY_Q){
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
		n_select = gtk_tree_selection_count_selected_rows(selection);

		if(n_select==1){
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			if (gtk_tree_model_get_iter(model, &iter, list->data)){
				gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
				if(is_queue)
					delete_queue_track_refs(list->data, cwin);
				else{
					ref = gtk_tree_row_reference_new(model, list->data);
					cwin->cstate->queue_track_refs = g_slist_append(cwin->cstate->queue_track_refs, ref);
				}
				requeue_track_refs(cwin);
			}
			gtk_tree_path_free(list->data);
			g_list_free (list);
		}
		return TRUE;
	}
	return FALSE;
}

/* Idle function to free musicobject when clear and crop current playlist */

void delete_mobj_list_foreach (gpointer data, gpointer user_data)
{
	struct musicobject *mobj = data;

	delete_musicobject(mobj);
}

/* Remove selected rows from current playlist */

void remove_from_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path, *next;
	GtkTreeIter iter;
	GList *list = NULL, *i = NULL;
	GSList *mobj_to_delete = NULL;
	struct musicobject *mobj = NULL;
	gboolean played = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		/* Select the next row to the last selected */

		gtk_tree_view_get_cursor(GTK_TREE_VIEW(cwin->current_playlist), &next, NULL);
		do {
			if(gtk_tree_selection_path_is_selected(selection, next) == FALSE)
				break;

			gtk_tree_path_next(next);
		}
		while(next != NULL);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW(cwin->current_playlist), next, NULL, FALSE);
		gtk_tree_path_free (next);

		/* Get references from the paths and store them in the 'data'
		   portion of the list elements.
		   This idea was inspired by code from 'claws-mail' */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			ref = gtk_tree_row_reference_new(model, path);
			i->data = ref;
			gtk_tree_path_free(path);
		}
		
		/* Now build iterators from the references and delete
		   them from the store */

		for (i=list; i != NULL; i = i->next) {
			ref = i->data;
			path = gtk_tree_row_reference_get_path(ref);
			delete_rand_track_refs(path, cwin);
			delete_queue_track_refs(path, cwin);
			test_clear_curr_seq_ref(path, cwin);

			if (gtk_tree_model_get_iter(model, &iter, path)) {
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

				if (G_UNLIKELY(mobj == cwin->cstate->curr_mobj))
					cwin->cstate->curr_mobj_clear = TRUE;
				else
					mobj_to_delete = g_slist_prepend(mobj_to_delete, mobj);

				gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				cwin->cstate->tracks_curr_playlist--;
				if (!played)
					cwin->cstate->unplayed_tracks--;
			}
			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
		}

		g_list_free(list);
	}

	g_slist_foreach(mobj_to_delete, delete_mobj_list_foreach, NULL);
	g_slist_free(mobj_to_delete);

	requeue_track_refs (cwin);
	update_status_bar(cwin);
}

/* Crop selected rows from current playlist */

void crop_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	gboolean ret, played = FALSE;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GSList *to_delete = NULL, *mobj_to_delete = NULL, *i = NULL;
	GdkCursor *cursor;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));

	/* At least one row must be selected */

	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), cursor);
	gdk_cursor_unref(cursor);

	/* Get a reference to all the nodes that are _not_ selected */

	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		if (gtk_tree_selection_iter_is_selected(selection, &iter) == FALSE) {
			path = gtk_tree_model_get_path(model, &iter);
			ref = gtk_tree_row_reference_new(model, path);
			to_delete = g_slist_prepend(to_delete, ref);
			gtk_tree_path_free(path);
		}
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	/* Delete the referenced nodes */

	g_object_ref(model);
	cwin->cstate->playlist_change = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), NULL);

	for (i=to_delete; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);
		delete_rand_track_refs(path, cwin);
		delete_queue_track_refs(path, cwin);
		test_clear_curr_seq_ref(path, cwin);

		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

			if (G_UNLIKELY(mobj == cwin->cstate->curr_mobj))
				cwin->cstate->curr_mobj_clear = TRUE;
			else
				mobj_to_delete = g_slist_prepend(mobj_to_delete, mobj);

			gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			cwin->cstate->tracks_curr_playlist--;
			if (!played)
				cwin->cstate->unplayed_tracks--;

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE))
					return;
			}
		}
		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), model);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), TRUE);
	cwin->cstate->playlist_change = FALSE;
	g_object_unref(model);

	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), NULL);

	g_slist_foreach(to_delete, delete_mobj_list_foreach, NULL);
	g_slist_free(to_delete);

	requeue_track_refs (cwin);
	update_status_bar(cwin);
}

/* Show track properties dialog
   This function is a fscking eyesore. */

void track_properties(struct musicobject *mobj, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *properties_table;
	GtkWidget *label_length, *label_bitrate, *label_channels, *label_samplerate, *label_folder, *label_filename;
	GtkWidget *info_length, *info_bitrate, *info_channels, *info_samplerate, *info_folder, *info_filename;

	gchar *length = NULL, *bitrate = NULL, *channels = NULL, *samplerate = NULL, *folder = NULL, *filename = NULL;

	if(!mobj)
		return;

	length = convert_length_str(mobj->tags->length);
	bitrate = g_strdup_printf("%d kbps", mobj->tags->bitrate);
	channels = g_strdup_printf("%d %s", mobj->tags->channels, _("Channels"));
	samplerate = g_strdup_printf("%d Hz", mobj->tags->samplerate);
	folder = get_display_filename(mobj->file, TRUE);
	filename = get_display_name(mobj);

	/* Create table */

	properties_table = gtk_table_new(6, 2, FALSE);

	gtk_table_set_col_spacings(GTK_TABLE(properties_table), 5);
	gtk_table_set_row_spacings(GTK_TABLE(properties_table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(properties_table), 5);

	/* Create labels */

	label_length = gtk_label_new(_("Length"));
	label_bitrate = gtk_label_new(_("Bitrate"));
	label_channels = gtk_label_new(_("Channels"));
	label_samplerate = gtk_label_new(_("Samplerate"));
	label_folder = gtk_label_new(_("Folder"));
	label_filename = gtk_label_new(_("Filename"));

	gtk_misc_set_alignment(GTK_MISC (label_length), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_bitrate), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_channels), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_samplerate), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_folder), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_filename), 1, 0);

	gtk_label_set_attribute_bold(GTK_LABEL(label_length));
	gtk_label_set_attribute_bold(GTK_LABEL(label_bitrate));
	gtk_label_set_attribute_bold(GTK_LABEL(label_channels));
	gtk_label_set_attribute_bold(GTK_LABEL(label_samplerate));
	gtk_label_set_attribute_bold(GTK_LABEL(label_folder));
	gtk_label_set_attribute_bold(GTK_LABEL(label_filename));

	/* Create info labels */

	info_length = gtk_label_new(length);
	info_bitrate = gtk_label_new(bitrate);
	info_channels = gtk_label_new(channels);
	info_samplerate = gtk_label_new(samplerate);
	info_folder = gtk_label_new(folder);
	info_filename = gtk_label_new(filename);

	gtk_misc_set_alignment(GTK_MISC (info_length), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_bitrate), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_channels), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_samplerate), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_folder), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_filename), 0, 0);

	gtk_label_set_selectable(GTK_LABEL(info_length), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_bitrate), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_channels), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_samplerate), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_folder), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_filename), TRUE);

	/* Attach labels */

	gtk_table_attach(GTK_TABLE (properties_table), label_length,
			0, 1, 0, 1,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_length,
			1, 2, 0, 1,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_bitrate,
			0, 1, 1, 2,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_bitrate,
			1, 2, 1, 2,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_channels,
			0, 1, 2, 3,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_channels,
			1, 2, 2, 3,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_samplerate,
			0, 1, 3, 4,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_samplerate,
			1, 2, 3, 4,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_folder,
			0, 1, 4, 5,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_folder,
			1, 2, 4, 5,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_filename,
			0, 1, 5, 6,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_filename,
			1, 2, 5, 6,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	/* The main edit dialog */

	dialog = gtk_dialog_new_with_buttons(_("Details"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), properties_table);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	g_free(length);
	g_free(bitrate);
	g_free(channels);
	g_free(samplerate);
	g_free(folder);
	g_free(filename);
}

/* Clear all rows from current playlist */

void clear_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	gboolean ret;
	GSList *to_delete = NULL;
	GdkCursor *cursor;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), cursor);
	gdk_cursor_unref(cursor);

	clear_rand_track_refs(cwin);
	clear_queue_track_refs(cwin);
	clear_curr_seq_ref(cwin);

	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

		if (G_UNLIKELY(mobj == cwin->cstate->curr_mobj))
			cwin->cstate->curr_mobj_clear = TRUE;
		else
			to_delete = g_slist_prepend(to_delete, mobj);

		ret = gtk_tree_model_iter_next(model, &iter);
	}

	gtk_list_store_clear(GTK_LIST_STORE(model));

	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), NULL);

	g_slist_foreach(to_delete, delete_mobj_list_foreach, NULL);
	g_slist_free(to_delete);

	cwin->cstate->tracks_curr_playlist = 0;
	cwin->cstate->unplayed_tracks = 0;

	update_status_bar(cwin);
}

/* Update a track to the current playlist */

void update_track_current_playlist(GtkTreeIter *iter, gint changed, struct musicobject *mobj, struct con_win *cwin)
{
	GtkTreeModel *model;
	gchar *ch_track_no = NULL, *ch_year = NULL, *ch_filename = NULL;

	if (!changed)
		return;

	CDEBUG(DBG_VERBOSE, "Track Updates: 0x%x", changed);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	ch_filename = get_display_name(mobj);

	if(mobj->tags->track_no > 0)
		ch_track_no = g_strdup_printf("%d", mobj->tags->track_no);
	if(mobj->tags->year > 0)
		ch_year = g_strdup_printf("%d", mobj->tags->year);

	if (changed & TAG_TNO_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_TRACK_NO, ch_track_no, -1);
	}
	if (changed & TAG_TITLE_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_TITLE,
					(mobj->tags->title && strlen(mobj->tags->title)) ? mobj->tags->title : ch_filename, -1);
	}
	if (changed & TAG_ARTIST_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_ARTIST, mobj->tags->artist,-1);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_ALBUM, mobj->tags->album,-1);
	}
	if (changed & TAG_GENRE_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_GENRE, mobj->tags->genre,-1);
	}
	if (changed & TAG_YEAR_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_YEAR, ch_year, -1);
	}
	if (changed & TAG_COMMENT_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_COMMENT, mobj->tags->comment,-1);
	}

	/* inform mpris2 */
	mpris_update_mobj_changed(cwin, mobj, changed);

	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_filename);
}

/* Insert a list of mobj to current playlist. */

void insert_mobj_list_current_playlist(GtkTreeModel *model, GList *list,
						   GtkTreeViewDropPosition droppos, GtkTreeIter *pos,
						   struct con_win *cwin)
{
	struct musicobject *mobj;
	GList *l;

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		insert_current_playlist(model, mobj, droppos, pos, cwin);
	}
}

/* Insert a track to the current playlist */

void insert_current_playlist(GtkTreeModel *model, struct musicobject *mobj,
				     GtkTreeViewDropPosition droppos, GtkTreeIter *pos,
				     struct con_win *cwin)
{
	GtkTreeIter iter;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;

	if(model == NULL)
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}
	if (!mobj->tags) {
		g_warning("Corrupt music object, no tags found");
		return;
	}

	if(mobj->tags->track_no > 0)
		ch_track_no = g_strdup_printf("%d", mobj->tags->track_no);
	if(mobj->tags->year > 0)
		ch_year = g_strdup_printf("%d", mobj->tags->year);
	if(mobj->tags->length)
		ch_length = convert_length_str(mobj->tags->length);
	if(mobj->tags->bitrate)
		ch_bitrate = g_strdup_printf("%d", mobj->tags->bitrate);

	ch_filename = get_display_name(mobj);

	if (droppos == GTK_TREE_VIEW_DROP_AFTER)
		gtk_list_store_insert_after(GTK_LIST_STORE(model), &iter, pos);
	else
		gtk_list_store_insert_before(GTK_LIST_STORE(model), &iter, pos);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   P_MOBJ_PTR, mobj,
			   P_QUEUE, NULL,
			   P_BUBBLE, FALSE,
			   P_STATUS_PIXBUF, NULL,
			   P_TRACK_NO, ch_track_no,
			   P_TITLE, (mobj->tags->title && strlen(mobj->tags->title)) ?
					mobj->tags->title : ch_filename,
			   P_ARTIST, mobj->tags->artist,
			   P_ALBUM, mobj->tags->album,
			   P_GENRE, mobj->tags->genre,
			   P_BITRATE, ch_bitrate,
			   P_YEAR, ch_year,
			   P_COMMENT, mobj->tags->comment,
			   P_LENGTH, ch_length,
			   P_FILENAME, ch_filename,
			   P_PLAYED, FALSE,
			   -1);

	/* Increment global count of tracks */

	cwin->cstate->tracks_curr_playlist++;
	cwin->cstate->unplayed_tracks++;

	/* Have to give control to GTK periodically ... */
	/* If gtk_main_quit has been called, return -
	   since main loop is no more. */
	while(gtk_events_pending()) {
		if (gtk_main_iteration_do(FALSE))
			return;
	}

	mpris_update_mobj_added(cwin, mobj, &iter);

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

/* Append a track to the current playlist */

void append_current_playlist(GtkTreeModel *model, struct musicobject *mobj, struct con_win *cwin)
{
	append_current_playlist_ex(model, mobj, cwin, NULL);
}

void append_current_playlist_ex(GtkTreeModel *model, struct musicobject *mobj, struct con_win *cwin, GtkTreePath **path)
{
	GtkTreeIter iter;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;

	if (model == NULL)
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}
	if (!mobj->tags) {
		g_warning("Corrupt music object, no tags found");
		return;
	}

	if(mobj->tags->track_no > 0)
		ch_track_no = g_strdup_printf("%d", mobj->tags->track_no);
	if(mobj->tags->year > 0)
		ch_year = g_strdup_printf("%d", mobj->tags->year);
	if(mobj->tags->length)
		ch_length = convert_length_str(mobj->tags->length);
	if(mobj->tags->bitrate)
		ch_bitrate = g_strdup_printf("%d", mobj->tags->bitrate);

	ch_filename = get_display_name(mobj);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   P_MOBJ_PTR, mobj,
			   P_QUEUE, NULL,
			   P_BUBBLE, FALSE,
			   P_STATUS_PIXBUF, NULL,
			   P_TRACK_NO, ch_track_no,
			   P_TITLE, (mobj->tags->title && strlen(mobj->tags->title)) ?
					mobj->tags->title : ch_filename,
			   P_ARTIST, mobj->tags->artist,
			   P_ALBUM, mobj->tags->album,
			   P_GENRE, mobj->tags->genre,
			   P_BITRATE, ch_bitrate,
			   P_YEAR, ch_year,
			   P_COMMENT, mobj->tags->comment,
			   P_LENGTH, ch_length,
			   P_FILENAME, ch_filename,
			   P_PLAYED, FALSE,
			   -1);

	/* Increment global count of tracks */

	cwin->cstate->tracks_curr_playlist++;
	cwin->cstate->unplayed_tracks++;

	/* Have to give control to GTK periodically ... */
	while(gtk_events_pending()) {
		gtk_main_iteration_do(FALSE);
	}

	/* inform mpris2 */
	mpris_update_mobj_added(cwin, mobj, &iter);

	if(path)
		*path = gtk_tree_model_get_path(model, &iter);

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

/* Append a list of mobj to the current playlist */

void append_mobj_list_current_playlist(GtkTreeModel *model, GList *list, struct con_win *cwin)
{
	struct musicobject *mobj;
	GList *l;

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		append_current_playlist(model, mobj, cwin);
	}
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Save selected tracks as a playlist */

void save_selected_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gchar *playlist;
	enum playlist_mgmt choice = 0;

	/* If current playlist is empty, return immediately. */

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	/* If no tracks have been selected in the current playlist,
	   return immediately. */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						cwin->current_playlist));
	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	playlist = get_playlist_name(cwin, SAVE_SELECTED, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist((const gchar *)playlist, SAVE_SELECTED, cwin);
			init_library_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist((const gchar *)playlist, SAVE_SELECTED, cwin);
			break;
		case EXPORT_PLAYLIST:
			export_playlist (SAVE_SELECTED, cwin);
			break;
		default:
			break;

		}
		g_free(playlist);
	}
}

/* Save current playlist */

void save_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *playlist = NULL;
	enum playlist_mgmt choice = 0;

	/* If current playlist is empty, return immediately. */

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	if(cwin->cstate->playlist_change)
		return;

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	playlist = get_playlist_name(cwin, SAVE_COMPLETE, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist((const gchar *)playlist, SAVE_COMPLETE, cwin);
			init_library_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist((const gchar *)playlist, SAVE_COMPLETE, cwin);
			break;
		case EXPORT_PLAYLIST:
			export_playlist (SAVE_COMPLETE, cwin);
			break;
		default:
			break;

		}
		g_free(playlist);
	}
}

/**********************/
/* Playback functions */
/**********************/

/* Play first track in current playlist */

void play_first_current_playlist(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	struct musicobject *mobj;

	if(cwin->cstate->playlist_change)
		return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	/* Check if tree is empty, if not play first track */

	if (gtk_tree_model_get_iter_first(model, &iter)) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

		backend_start(mobj, cwin);

		clear_rand_track_refs(cwin);
		path = gtk_tree_model_get_path(model, &iter);
		update_current_state( path, PLAYLIST_CURR, cwin);
		gtk_tree_path_free(path);
	}
}

/* Play prev track in current playlist */

void play_prev_track(struct con_win *cwin)
{
	GtkTreePath *path;
	struct musicobject *mobj = NULL;

	/* Get the next (prev) track to be played */
	path = current_playlist_get_prev(cwin);

	/* No more tracks */
	if (!path)
		return;

	/* Stop currently playing track */
	backend_stop(NULL, cwin);

	/* Start playing new track */
	mobj = current_playlist_mobj_at_path(path, cwin);
	backend_start(mobj, cwin);

	update_current_state(path, PLAYLIST_PREV, cwin);
	gtk_tree_path_free(path);
}

/* Play next track in current_playlist */

void play_next_track(struct con_win *cwin)
{
	GtkTreePath *path;
	struct musicobject *mobj = NULL;

	/* Are we playing right now ? */

	if (cwin->cstate->state == ST_STOPPED)
		return;

	/* Stop currently playing track */
	backend_stop(NULL, cwin);

	/* Get the next track to be played */
	path = current_playlist_get_next(cwin);

	/* No more tracks */
	if (!path)
		return;

	/* Start playing new track */
	mobj = current_playlist_mobj_at_path(path, cwin);
	backend_start(mobj, cwin);

	update_current_state(path, PLAYLIST_NEXT, cwin);
	gtk_tree_path_free(path);
}

/* Start playback of a new track, or resume playback of current track */

void play_track(struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	GtkTreePath *path = NULL;

	/* New action is based on the current state */

	/************************************/
        /* State     Action		    */
	/* 				    */
	/* Playing   Restart playback	    */
	/* Paused    Resume playback	    */
	/* Stopped   Start playback	    */
        /************************************/

	switch (cwin->cstate->state) {
	case ST_PLAYING:
		if (cwin->cpref->shuffle && cwin->cstate->curr_rand_ref)
			path = gtk_tree_row_reference_get_path(cwin->cstate->curr_rand_ref);
		else if (!cwin->cpref->shuffle && cwin->cstate->curr_seq_ref)
			path = gtk_tree_row_reference_get_path(cwin->cstate->curr_seq_ref);

		if (path) {
			mobj = new_musicobject_from_file(cwin->cstate->curr_mobj->file);
			backend_start(mobj, cwin);

			update_current_state(path, PLAYLIST_CURR, cwin);
			gtk_tree_path_free(path);
		}
		break;
	case ST_PAUSED:
		backend_resume(cwin);
		break;
	case ST_STOPPED:
		if(cwin->cstate->playlist_change)
			break;
		if(cwin->cstate->queue_track_refs)
			path = get_next_queue_track(cwin);
		if(!path)
			path = current_playlist_get_selection(cwin);
		if(!path && cwin->cpref->shuffle)
			path = get_first_random_track(cwin);
		if(!path) {
			play_first_current_playlist(cwin);
			break;
		}

		mobj = current_playlist_mobj_at_path(path, cwin);
		backend_start(mobj, cwin);

		clear_rand_track_refs(cwin);
		current_playlist_clear_dirty_all(cwin);
		cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
		update_current_state(path, PLAYLIST_CURR, cwin);
		gtk_tree_path_free(path);
		break;
	default:
		break;
	}
}

/* Toggle pause/resume */

void pause_resume_track(struct con_win *cwin)
{
	switch(cwin->cstate->state) {
	case ST_PAUSED:
		backend_resume(cwin);
		break;
	case ST_PLAYING:
		backend_pause(cwin);
		break;
	default:
		break;
	}
}

/*******************/
/* Event Callbacks */
/*******************/

/* Handler for row double click / kboard enter */

void current_playlist_row_activated_cb(GtkTreeView *current_playlist,
				       GtkTreePath *path,
				       GtkTreeViewColumn *column,
				       struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	struct musicobject *mobj;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	backend_start(mobj, cwin);

	clear_rand_track_refs(cwin);
	current_playlist_clear_dirty_all(cwin);
	cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
	update_current_state(path, PLAYLIST_CURR, cwin);
}

/* Handler for current playlist click */

gboolean current_playlist_button_press_cb(GtkWidget *widget,
					 GdkEventButton *event,
					 struct con_win *cwin)
{
	GtkWidget *popup_menu, *item_widget;
	GtkTreeSelection *selection;
	gint n_select = 0;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean ret = FALSE, is_queue = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));

	switch(event->button){
	case 1:
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL)){
			if (gtk_tree_selection_path_is_selected(selection, path)
			    && !(event->state & GDK_CONTROL_MASK)
			    && !(event->state & GDK_SHIFT_MASK)) {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_false, cwin, NULL);
			}
			else {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cwin, NULL);
			}
			gtk_tree_path_free(path);
		}
		break;
	case 3:
		if ( cwin->cstate->tracks_curr_playlist == 0) {
			popup_menu = gtk_ui_manager_get_widget(cwin->cp_null_context_menu, "/popup");
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL, event->button, event->time);
			ret = FALSE;
			break;
		}
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL)){
			if (!(gtk_tree_selection_path_is_selected(selection, path))){
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}

			n_select = gtk_tree_selection_count_selected_rows(selection);

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/ToolsMenu");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), (n_select == 1));

			if (gtk_tree_model_get_iter(model, &iter, path)){
				gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);

				if(is_queue){
					item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Queue track");
					gtk_widget_hide(GTK_WIDGET(item_widget));

					item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Dequeue track");
					gtk_widget_show(GTK_WIDGET(item_widget));
				}
				else{
					item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Queue track");
					gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);
					gtk_widget_show(GTK_WIDGET(item_widget));

					item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Dequeue track");
					gtk_widget_hide(GTK_WIDGET(item_widget));
				}
				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Remove from playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Crop playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Add to another playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Edit tags");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);
			}
			else{
				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Queue track");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);
				gtk_widget_show(GTK_WIDGET(item_widget));

				item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Dequeue track");
				gtk_widget_hide(GTK_WIDGET(item_widget));
			}

			/* If more than one track is selected, don't propagate event */

			if (n_select > 1)
				ret = TRUE;
			else
				ret = FALSE;

			gtk_tree_path_free(path);
		}
		else {
			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Queue track");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);
			gtk_widget_show(GTK_WIDGET(item_widget));

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Dequeue track");
			gtk_widget_hide(GTK_WIDGET(item_widget));

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Remove from playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Crop playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Add to another playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/ToolsMenu");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cwin->cp_context_menu, "/popup/Edit tags");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			gtk_tree_selection_unselect_all(selection);
		}

		popup_menu = gtk_ui_manager_get_widget(cwin->cp_context_menu,
						       "/popup");
		gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
			       event->button, event->time);
		break;
	default:
		ret = FALSE;
		break;
	}

	return ret;
}

gboolean current_playlist_button_release_cb(GtkWidget *widget,
					    GdkEventButton *event,
					    struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));

	if((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK) || (cwin->cstate->dragging == TRUE) || (event->button!=1)){
		gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cwin, NULL);
		cwin->cstate->dragging = FALSE;
		return FALSE;
	}

	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL);

	if (path){
		gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cwin, NULL);
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, path);
		gtk_tree_path_free(path);
	}
	return FALSE;
}

/* Handler for column header right click popup menu */

gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       struct con_win *cwin)
{
	gboolean ret = FALSE;

	switch(event->button) {
	case 3: {
		gtk_menu_popup(GTK_MENU(cwin->header_context_menu),
			       NULL, NULL, NULL, NULL, event->button, event->time);
		ret = TRUE;
		break;
	}
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

/*******/
/* DnD */
/*******/

gboolean dnd_current_playlist_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin)
{
	cwin->cstate->dragging = TRUE;
	return FALSE;
}

/* Callback for DnD signal 'drag-data-get' */

void drag_current_playlist_get_data (GtkWidget *widget,
				    GdkDragContext *context,
				    GtkSelectionData *selection_data,
				    guint target_type,
				    guint time,
				    struct con_win *cwin)
{
	g_assert (selection_data != NULL);

	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list, *i;
	GtkTreePath *path;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;
	guint uri_i = 0;
	gchar **uri_list;

        switch (target_type){
		case TARGET_URI_LIST:
			CDEBUG(DBG_VERBOSE, "DnD: TARGET_URI_LIST");

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			uri_list = g_new(gchar* , gtk_tree_selection_count_selected_rows(selection) + 1);

			for (i=list; i != NULL; i = i->next){
				path = i->data;
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

				if (G_LIKELY(mobj &&
				    mobj->file_type != FILE_CDDA &&
				    mobj->file_type != FILE_HTTP))
					uri_list[uri_i++] = g_filename_to_uri(mobj->file, NULL, NULL);

				gtk_tree_path_free(path);
			}
			uri_list[uri_i++] = NULL;

			gtk_selection_data_set_uris(selection_data,  uri_list);
			g_strfreev(uri_list);
			g_list_free(list);
			break;
		default:
			break;
	}

}

gboolean dnd_current_playlist_drop(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   guint time,
				   struct con_win *cwin)
{
	GdkAtom target;

	if (gtk_drag_get_source_widget(context) == cwin->library_tree) {
		CDEBUG(DBG_VERBOSE, "DnD: library_tree");
		target = GDK_POINTER_TO_ATOM(g_list_nth_data(gdk_drag_context_list_targets(context),
							     TARGET_REF_LIBRARY));
		gtk_drag_get_data(widget,
				  context,
				  target,
				  time);
		return TRUE;
	}

	return FALSE;
}

/* Reorder playlist with DnD. */

void
dnd_current_playlist_reorder(GtkTreeModel *model,
					GtkTreeIter *dest_iter,
					GtkTreeViewDropPosition pos,
					struct con_win *cwin)
{
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GList *list = NULL, *l;

	CDEBUG(DBG_VERBOSE, "Dnd: Reorder");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	/* Clear sort */
	clear_sort_current_playlist(NULL, cwin);

	/* No selections */
	if (!list)
		goto exit;

	/* Store references to the selected paths */
	l = list;
	while(l) {
		path = l->data;
		ref = gtk_tree_row_reference_new(model, path);
		l->data = ref;
		gtk_tree_path_free(path);
		l = l->next;
	}

	for (l=list; l != NULL; l = l->next) {
		ref = l->data;
		path = gtk_tree_row_reference_get_path(ref);
		gtk_tree_model_get_iter(model, &iter, path);

		if (pos == GTK_TREE_VIEW_DROP_BEFORE)
			gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, dest_iter);
		else if (pos == GTK_TREE_VIEW_DROP_AFTER)
			gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, dest_iter);

			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
	}

exit:
	g_list_free(list);
}

GList *
dnd_current_playlist_received_from_library(GtkSelectionData *data,
							struct con_win *cwin)
{
	GtkTreeModel *library_model;
	GArray *ref_arr;
	GtkTreePath *path = NULL;
	gint i = 0, location_id = 0;
	GtkTreeIter iter;
	gchar *name = NULL, *query;
	enum node_type node_type;
	struct musicobject *mobj = NULL;
	GList *list = NULL;

	CDEBUG(DBG_VERBOSE, "Dnd: Library");

	ref_arr = *(GArray **)gtk_selection_data_get_data(data);
	if (!ref_arr)
		g_warning("No selections to process in DnD");

	library_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));

	/* Dnd from the library, so will read everything from database. */

	query = g_strdup_printf("BEGIN;");
	exec_sqlite_query(query, cwin, NULL);

	/* Get the mobjs from the path of the library. */

	do {
		path = g_array_index(ref_arr, GtkTreePath *, i);
		if (path) {
			gtk_tree_model_get_iter(library_model, &iter, path);
			gtk_tree_model_get(library_model, &iter, L_NODE_TYPE, &node_type, -1);

			switch (node_type)
			{
				case NODE_BASENAME:
				case NODE_TRACK:
					gtk_tree_model_get(library_model, &iter, L_LOCATION_ID, &location_id, -1);
					mobj = new_musicobject_from_db(location_id, cwin);
					list = g_list_prepend(list, mobj);
					break;
				case NODE_PLAYLIST:
					gtk_tree_model_get(library_model, &iter, L_NODE_DATA, &name, -1);
					list = prepend_playlist_to_mobj_list(name, list, cwin);
					g_free(name);
					break;
				case NODE_RADIO:
					gtk_tree_model_get(library_model, &iter, L_NODE_DATA, &name, -1);
					list = prepend_radio_to_mobj_list(name, list, cwin);
					g_free(name);
					break;
				default:
					break;
			}

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE))
					return NULL;
			}

			gtk_tree_path_free(path);
		}
		i++;
	} while (path != NULL);

	query = g_strdup_printf("END;");
	exec_sqlite_query(query, cwin, NULL);

	g_array_free(ref_arr, FALSE);

	return list;
}

GList *
dnd_current_playlist_received_uri_list(GtkSelectionData *data,
						   struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	gchar **uris = NULL, *filename = NULL;
	GList *list = NULL;
	gint i = 0;

	CDEBUG(DBG_VERBOSE, "Target: URI_LIST");

	uris = gtk_selection_data_get_uris(data);

	if(uris){
		for(i = 0; uris[i] != NULL; i++) {
			filename = g_filename_from_uri(uris[i], NULL, NULL);
			if (g_file_test(filename, G_FILE_TEST_IS_DIR)){
				list = append_mobj_list_from_folder(list, filename, cwin);
			}
			else {
				mobj = new_musicobject_from_file(filename);
				if (!mobj)
					g_critical("Invalid location filename");
				else {
					list = g_list_append(list, mobj);
				}
			}

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE))
					return NULL;
			}

			g_free(filename);
		}
		g_strfreev(uris);
	}

	return list;
}

GList *
dnd_current_playlist_received_plain_text(GtkSelectionData *data,
							struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	gchar *filename = NULL;
	GList *list = NULL;

	CDEBUG(DBG_VERBOSE, "Target: PLAIN_TEXT");

	filename = (gchar*)gtk_selection_data_get_text(data);

	if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
		list = append_mobj_list_from_folder(list, filename, cwin);
	}
	else {
		mobj = new_musicobject_from_file(filename);
		if (!mobj)
			g_critical("Invalid location filename");
		else {
			list = g_list_append(list, mobj);
		}
		/* Have to give control to GTK periodically ... */
		/* If gtk_main_quit has been called, return -
		   since main loop is no more. */
		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE))
				return NULL;
		}
	}
	g_free(filename);

	return list;
}

/* Callback for DnD signal 'drag-data-received' */

void dnd_current_playlist_received(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   GtkSelectionData *data,
				   enum dnd_target info,
				   guint time,
				   struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *dest_path = NULL;
	GtkTreeIter dest_iter;
	GtkTreeViewDropPosition pos = 0;
	gboolean is_row;
	GdkRectangle vrect, crect;
	gdouble row_align;
	GdkCursor *cursor;
	GList *list = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	is_row = gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(cwin->current_playlist),
						x, y,
						&dest_path,
						&pos);

	gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(cwin->current_playlist), &vrect);
	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(cwin->current_playlist), dest_path, NULL, &crect);
	
	row_align = (gdouble)crect.y / (gdouble)vrect.height;

	switch(pos) {
		case GTK_TREE_VIEW_DROP_INTO_OR_BEFORE:
			pos = GTK_TREE_VIEW_DROP_BEFORE;
			break;
		case GTK_TREE_VIEW_DROP_INTO_OR_AFTER:
			pos = GTK_TREE_VIEW_DROP_AFTER;
			break;
		default:
			break;
	}

	if (is_row)
		gtk_tree_model_get_iter (model, &dest_iter, dest_path);

	/* Reorder within current playlist */

	if (gtk_drag_get_source_widget(context) == cwin->current_playlist) {
		dnd_current_playlist_reorder(model, &dest_iter, pos, cwin);
		goto exit;
	}

	/* Show busy mouse icon */

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor (gtk_widget_get_window(cwin->mainwindow), cursor);
	gdk_cursor_unref(cursor);

	/* Get new tracks to append on playlist */

	switch(info) {
	case TARGET_REF_LIBRARY:
		list = dnd_current_playlist_received_from_library(data, cwin);
		break;
	case TARGET_URI_LIST:
		list = dnd_current_playlist_received_uri_list(data, cwin);
		break;
	case TARGET_PLAIN_TEXT:
		list = dnd_current_playlist_received_plain_text(data, cwin);
		break;
	default:
		g_warning("Unknown DND type");
		break;
	}

	/* Insert mobj list to current playlist. */

	g_object_ref(model);
	cwin->cstate->playlist_change = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), NULL);

	if (is_row)
		insert_mobj_list_current_playlist(model, list, pos, &dest_iter, cwin);
	else
		append_mobj_list_current_playlist(model, list, cwin);

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), model);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), TRUE);
	cwin->cstate->playlist_change = FALSE;
	g_object_unref(model);

	update_status_bar(cwin);

	if (is_row)
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(cwin->current_playlist), dest_path, NULL, TRUE, row_align, 0.0);
	else
		select_last_path_of_current_playlist(cwin);

	/* Remove busy mouse icon */

	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), NULL);

	g_list_free(list);

exit:
	gtk_tree_path_free(dest_path);
	gtk_drag_finish(context, TRUE, FALSE, time);
}

/* Save current playlist state on exit */

void save_current_playlist_state(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gint playlist_id = 0;
	gchar *ref_char = NULL;

	/* Save last playlist. */

	playlist_id = find_playlist_db(SAVE_PLAYLIST_STATE, cwin);
	if (!playlist_id)
		playlist_id = add_new_playlist_db(SAVE_PLAYLIST_STATE,
						  cwin);
	else
		flush_playlist_db(playlist_id, cwin);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	save_playlist(playlist_id, SAVE_COMPLETE, cwin);

	/* Save reference to current song. */

	path = current_playlist_get_actual(cwin);
	if(path) {
		ref_char = gtk_tree_path_to_string (path);
		gtk_tree_path_free(path);

		g_key_file_set_string(cwin->cpref->configrc_keyfile,
					GROUP_PLAYLIST,
					KEY_CURRENT_REF,
					ref_char);
		g_free (ref_char);
	}
	else {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_PLAYLIST) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_PLAYLIST,
				       KEY_CURRENT_REF,
				       NULL)){
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF,
					      NULL);
		}
	}
}

/* Init current playlist on application bringup,
   restore stored playlist */

void init_playlist_current_playlist(struct con_win *cwin)
{
	gchar *s_playlist, *query, *file;
	gint playlist_id, location_id, i = 0;
	struct db_result result;
	struct musicobject *mobj;
	GtkTreeModel *model;
	GdkCursor *cursor;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor (gtk_widget_get_window(cwin->mainwindow), cursor);
	gdk_cursor_unref(cursor);

	g_object_ref(model);
	cwin->cstate->playlist_change = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), NULL);

	s_playlist = sanitize_string_sqlite3(SAVE_PLAYLIST_STATE);
	playlist_id = find_playlist_db(s_playlist, cwin);
	query = g_strdup_printf("SELECT FILE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d",
				playlist_id);
	exec_sqlite_query(query, cwin, &result);

	for_each_result_row(result, i) {
		file = sanitize_string_sqlite3(result.resultp[i]);
		/* TODO: Fix this negradaaa!. */
		if(g_str_has_prefix((gchar*)file, "Radio:") == FALSE) {
			if ((location_id = find_location_db(file, cwin)))
				mobj = new_musicobject_from_db(location_id, cwin);
			else
				mobj = new_musicobject_from_file(result.resultp[i]);
		}
		else {
			mobj = new_musicobject_from_location(file + strlen("Radio:"), file + strlen("Radio:"), cwin);
		}
		append_current_playlist(model, mobj, cwin);
		g_free(file);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), model);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), TRUE);
	cwin->cstate->playlist_change = FALSE;
	g_object_unref(model);

	update_status_bar(cwin);

	mpris_update_tracklist_replaced(cwin);

	gdk_window_set_cursor(gtk_widget_get_window(cwin->mainwindow), NULL);

	sqlite3_free_table(result.resultp);
	g_free(s_playlist);
}

void init_current_playlist_view(struct con_win *cwin)
{
	gchar *ref = NULL;
	GError *error = NULL;
	GtkTreePath *path=NULL;

	init_playlist_current_playlist(cwin);

	ref = g_key_file_get_string(cwin->cpref->configrc_keyfile,
				    GROUP_PLAYLIST,
				    KEY_CURRENT_REF,
				    &error);
	if (!ref) {
		g_error_free(error);
		error = NULL;
		return;
	}

	path = gtk_tree_path_new_from_string(ref);
	jump_to_path_on_current_playlist (path, cwin);

	gtk_tree_path_free(path);
	g_free(ref);
}

/* Initialize columns of current playlist */

void init_current_playlist_columns(struct con_win *cwin)
{
	const gchar *col_name;
	GtkTreeViewColumn *col;
	GList *list = NULL, *i;
	GSList *j;
	gint k = 0;

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(cwin->current_playlist));

	/* Mark only the columns that are present in
	   cwin->cpref->playlist_columns as visible.
	   And set their sizes */

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			col = i->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cwin->cpref->playlist_columns)) {
				j = g_slist_nth(cwin->cpref->playlist_column_widths,
						k++);
				gtk_tree_view_column_set_visible(col, TRUE);
				gtk_tree_view_column_set_sizing(col,
						GTK_TREE_VIEW_COLUMN_FIXED);
				if (GPOINTER_TO_INT(j->data) > COL_WIDTH_THRESH)
					gtk_tree_view_column_set_fixed_width(col,
						     GPOINTER_TO_INT(j->data));
				else
					gtk_tree_view_column_set_fixed_width(col,
						     DEFAULT_PLAYLIST_COL_WIDTH);
			}
			else
				gtk_tree_view_column_set_visible(col, FALSE);
		}
		g_list_free(list);
	}
	else
		g_warning("(%s): No columns in playlist view", __func__);

	/* Always show queue and status pixbuf colum */
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist), 0);
	gtk_tree_view_column_set_visible(col, TRUE);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(col, 32);
}

/* Callback for adding/deleting track_no column */

void playlist_track_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_TRACK_NO - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting title column */

void playlist_title_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_TITLE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting artist column */

void playlist_artist_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_ARTIST - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting album column */

void playlist_album_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_ALBUM - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting genre column */

void playlist_genre_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_GENRE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting bitrate column */

void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_BITRATE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting year column */

void playlist_year_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_YEAR - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting comment column */

void playlist_comment_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_COMMENT - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting length column */

void playlist_length_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_LENGTH - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting filename column */

void playlist_filename_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cwin->current_playlist),
				       P_FILENAME - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist_cb(GtkMenuItem *item, struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Comparison function for track numbers */

gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->track_no < mobj_b->tags->track_no)
		return -1;
	else if (mobj_a->tags->track_no > mobj_b->tags->track_no)
		return 1;
	else
		return 0;
}

/* Comparison function for bitrates */

gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->bitrate < mobj_b->tags->bitrate)
		return -1;
	else if (mobj_a->tags->bitrate > mobj_b->tags->bitrate)
		return 1;
	else
		return 0;
}

/* Comparison function for years */

gint compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->year < mobj_b->tags->year)
		return -1;
	else if (mobj_a->tags->year > mobj_b->tags->year)
		return 1;
	else
		return 0;
}

/* Comparison function for lengths */

gint compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->length < mobj_b->tags->length)
		return -1;
	else if (mobj_a->tags->length > mobj_b->tags->length)
		return 1;
	else
		return 0;
}
