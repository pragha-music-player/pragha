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

static const gchar *cp_context_menu_xml = "<ui>					\
	<popup>					    				\
	<menuitem action=\"Queue track\"/>					\
	<menuitem action=\"Dequeue track\"/>					\
	<separator/>				    				\
	<menuitem action=\"Remove from playlist\"/>				\
	<menuitem action=\"Crop playlist\"/>					\
	<menuitem action=\"Clear playlist\"/>					\
	<separator/>				    				\
	<menuitem action=\"Add to another playlist\"/>				\
	<menuitem action=\"Save playlist\"/>					\
	<separator/>				    				\
	<menu action=\"ToolsMenu\">						\
		<menuitem action=\"Search lyric\"/>				\
		<menuitem action=\"Search artist info\"/>			\
		<separator/>							\
		<menuitem action=\"Love track\"/>				\
		<menuitem action=\"Unlove track\"/>				\
		<separator/>							\
		<menuitem action=\"Add similar\"/>				\
	</menu>									\
	<separator/>				    				\
	<menuitem action=\"Copy tag to selection\"/>				\
	<separator/>				    				\
	<menuitem action=\"Edit tags\"/>					\
	</popup>				    				\
	</ui>";

static const gchar *cp_null_context_menu_xml = "<ui>				\
	<popup>					    				\
	<menuitem action=\"Add files\"/>					\
	<menuitem action=\"Add Audio CD\"/>					\
	<menuitem action=\"Add location\"/>					\
	<separator/>				    				\
	<menuitem action=\"Add the library\"/>					\
	<separator/>				    				\
	<menuitem action=\"Lateral panel\"/>					\
	<separator/>				    				\
	<menuitem action=\"Quit\"/>						\
	</popup>				    				\
	</ui>";

static GtkActionEntry cp_context_aentries[] = {
	{"Queue track", GTK_STOCK_ADD, N_("Add to playback queue"),
	 "", "Add to playback queue", G_CALLBACK(queue_current_playlist)},
	{"Dequeue track", GTK_STOCK_REMOVE, N_("Remove to playback queue"),
	 "", "Remove to playback queue", G_CALLBACK(dequeue_current_playlist)},
	{"Remove from playlist", GTK_STOCK_REMOVE, N_("Remove from playlist"),
	 "", "Remove selection from playlist", G_CALLBACK(remove_from_playlist)},
	{"Crop playlist", GTK_STOCK_REMOVE, N_("Crop playlist"),
	 "", "Remove no telected tracks of playlist", G_CALLBACK(crop_current_playlist)},
	{"Clear playlist", GTK_STOCK_CLEAR, N_("Clear playlist"),
	 "", "Clear the current playlist", G_CALLBACK(current_playlist_clear_action)},
	{"Add to another playlist", GTK_STOCK_SAVE_AS, N_("Add to another playlist")},
	{"Save playlist", GTK_STOCK_SAVE, N_("Save playlist")},
	{"ToolsMenu", NULL, N_("_Tools")},
	#ifdef HAVE_LIBGLYR
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "", "Search lyric", G_CALLBACK(related_get_lyric_current_playlist_action)},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", G_CALLBACK(related_get_artist_info_current_playlist_action)},
	#else
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "", "Search lyric", NULL},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", NULL},
	#endif
	{"Lastfm", NULL, N_("_Lastfm")},
	#ifdef HAVE_LIBCLASTFM
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", G_CALLBACK(lastfm_track_current_playlist_love_action)},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", G_CALLBACK(lastfm_track_current_playlist_unlove_action)},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", G_CALLBACK(lastfm_get_similar_current_playlist_action)},
	#else
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", NULL},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", NULL},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", NULL},
	#endif
	{"Copy tag to selection", GTK_STOCK_COPY, NULL,
	 "", "Copy tag to selection", G_CALLBACK(copy_tags_to_selection_action)},
	{"Edit tags", GTK_STOCK_EDIT, N_("Edit track information"),
	 "", "Edit information for this track", G_CALLBACK(edit_tags_current_playlist)}
};

static GtkActionEntry cp_null_context_aentries[] = {
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 "", N_("Open a media file"), G_CALLBACK(open_file_action)},
	{"Add Audio CD", GTK_STOCK_CDROM, N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(add_audio_cd_action)},
	{"Add location", GTK_STOCK_NETWORK, N_("Add _location"),
	 "", "Add a no local stream", G_CALLBACK(add_location_action)},
	{"Add the library", GTK_STOCK_ADD, N_("_Add the library"),
	"", "Add all the library", G_CALLBACK(add_libary_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 "<Control>Q", "Quit pragha", G_CALLBACK(quit_action)}
};

static GtkToggleActionEntry cp_null_toggles_entries[] = {
	{"Lateral panel", NULL, N_("Lateral _panel"),
	 "", "Lateral panel", G_CALLBACK(library_pane_action),
	TRUE}
};

/* Update playback state pixbuf */

void pragha_playlist_update_pixbuf_path(PraghaPlaylist* cplaylist, GtkTreePath *path, struct con_win *cwin)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf = NULL;
	GtkIconTheme *icon_theme;
	GError *error = NULL;

	if(pragha_playlist_is_changing(cplaylist))
		return;

	if(pragha_backend_emitted_error(cwin->backend)) {
		icon_theme = gtk_icon_theme_get_default ();
		error = pragha_backend_get_error(cwin->backend);

		if(error->code == GST_RESOURCE_ERROR_NOT_FOUND)
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "list-remove", 16, 0, NULL);
		else
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "dialog-warning", 16, 0, NULL);
	}
	else {
		switch (pragha_backend_get_state (cwin->backend))
		{
			case ST_PLAYING:
				pixbuf = cplaylist->playing_pixbuf;
				break;
			case ST_PAUSED:
				pixbuf = cplaylist->paused_pixbuf;
				break;
			default:
				break;
		}
	}

	if (path != NULL) {
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(cplaylist->view));
		if (gtk_tree_model_get_iter (model, &iter, path)) {
			gtk_list_store_set (GTK_LIST_STORE(model), &iter, P_STATUS_PIXBUF, pixbuf, -1);
		}
	}

	if (error)
		g_object_unref (pixbuf);
}

static gchar* get_display_name(PraghaMusicobject *mobj)
{
	gchar *name = NULL;
	
	if (pragha_musicobject_get_file_type(mobj) == FILE_CDDA) {
		name = g_strdup(pragha_musicobject_get_file(mobj));
	} else {
		name = get_display_filename(pragha_musicobject_get_file(mobj), FALSE);
	}
	return name;
}

static gint get_total_playtime(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint total_playtime = 0;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	if(cplaylist->changing)
		return 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj)
			total_playtime += pragha_musicobject_get_length(mobj);
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return total_playtime;
}

/* Update status bar */

void update_status_bar_playtime(struct con_win *cwin)
{
	gint total_playtime = 0, no_tracks = 0;
	gchar *str, *tot_str;

	if(pragha_playlist_is_changing(cwin->cplaylist))
		return;

	total_playtime = get_total_playtime(cwin->cplaylist);
	no_tracks = pragha_playlist_get_no_tracks(cwin->cplaylist);

	tot_str = convert_length_str(total_playtime);
	str = g_strdup_printf("%i %s - %s",
			      no_tracks,
			      ngettext("Track", "Tracks", no_tracks),
			      tot_str);

	CDEBUG(DBG_VERBOSE, "Updating status bar with new playtime: %s", tot_str);

	gtk_label_set_text(GTK_LABEL(cwin->status_bar), str);

	g_free(tot_str);
	g_free(str);
}

/* Clear current seq ref */

static void clear_curr_seq_ref(PraghaPlaylist *cplaylist)
{
	if (!cplaylist->curr_seq_ref)
		return;

	gtk_tree_row_reference_free(cplaylist->curr_seq_ref);
	cplaylist->curr_seq_ref = NULL;
}

/* Clear cstate->curr_seq_ref if it happens to contain the given path */

static void test_clear_curr_seq_ref(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GtkTreePath *lpath;

	if (!cplaylist->curr_seq_ref)
		return;

	lpath = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);
	if (!gtk_tree_path_compare(path, lpath)) {
		gtk_tree_row_reference_free(cplaylist->curr_seq_ref);
		cplaylist->curr_seq_ref = NULL;
	}
	gtk_tree_path_free(lpath);
}

/* Check if given ref is the current rand reference */

static gboolean is_current_rand_ref(GtkTreeRowReference *ref, PraghaPlaylist *cplaylist)
{
	if (ref == cplaylist->curr_rand_ref)
		return TRUE;
	else
		return FALSE;
}

static void requeue_track_refs (PraghaPlaylist *cplaylist)
{
	GSList *list = NULL;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GtkTreePath *lpath;
	gchar *ch_queue_no=NULL;
	GtkTreeIter iter;
	gint i=0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	list = cplaylist->queue_track_refs;

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

static void delete_queue_track_refs(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GSList *list = NULL;
	GtkTreeRowReference *dref = NULL, *ref;
	GtkTreeModel *model;
	GtkTreePath *lpath;
	GtkTreeIter iter;

	if (cplaylist->queue_track_refs) {
		list = cplaylist->queue_track_refs;
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);

			if (!gtk_tree_path_compare(path, lpath))
				dref = ref;

			if (gtk_tree_model_get_iter(model, &iter, lpath)){
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_QUEUE, NULL, -1);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_BUBBLE, FALSE, -1);
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
		if (dref) {
			cplaylist->queue_track_refs = g_slist_remove(cplaylist->queue_track_refs, dref);
			gtk_tree_row_reference_free(dref);
		}
	}
}

/* Delete the ref corresponding to the given path */

static void delete_rand_track_refs(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (cplaylist->rand_track_refs) {
		list = cplaylist->rand_track_refs;
		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);
			if (!gtk_tree_path_compare(path, lpath)) {
				if (is_current_rand_ref(ref, cplaylist))
					cplaylist->curr_rand_ref = NULL;
				cplaylist->rand_track_refs =
					g_list_remove(cplaylist->rand_track_refs,
						      ref);
				gtk_tree_row_reference_free(ref);
				gtk_tree_path_free(lpath);
				break;
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
	}
}

/* Mark a track in current playlist as dirty */

static void
current_playlist_set_dirty_track(PraghaPlaylist *cplaylist, GtkTreePath *path)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, TRUE, -1);

	if (cplaylist->unplayed_tracks)
		cplaylist->unplayed_tracks--;
}

/* Mark a track in current playlist as dirty */

static void
current_playlist_unset_dirty_track(PraghaPlaylist *cplaylist, GtkTreePath *path)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, FALSE, -1);

	cplaylist->unplayed_tracks++;
}

/* Mark all tracks in current playlist as clean */

static void
current_playlist_clear_dirty_all(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	ret = gtk_tree_model_get_iter_first(model, &iter);
	while(ret) {
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, FALSE, -1);
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	cplaylist->unplayed_tracks = cplaylist->no_tracks;
}

/* Remove all ref of song next current song */

static void trim_down_rand_track_refs(PraghaPlaylist *cplaylist)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (cplaylist->rand_track_refs) {
		list = g_list_find(cplaylist->rand_track_refs, cplaylist->curr_rand_ref);

		if (list) {
			list = g_list_next(list);
			while (list) {
				ref = list->data;
				lpath = gtk_tree_row_reference_get_path(ref);

				current_playlist_unset_dirty_track(cplaylist, lpath);

				cplaylist->rand_track_refs =
					g_list_remove(cplaylist->rand_track_refs,
						      ref);
				gtk_tree_row_reference_free(ref);
				gtk_tree_path_free(lpath);
				list = list->next;
			}
		}
	}
}

/* Return the next node after the given ref */

static GtkTreeRowReference* get_rand_ref_next(GtkTreeRowReference *ref,
					      PraghaPlaylist *cplaylist)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cplaylist->rand_track_refs)
		return NULL;

	list = cplaylist->rand_track_refs;
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
					      PraghaPlaylist *cplaylist)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!cplaylist->rand_track_refs)
		return NULL;

	list = cplaylist->rand_track_refs;
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

GtkTreePath* current_playlist_nth_track(gint n, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;

	if(cplaylist->changing)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if(gtk_tree_model_iter_nth_child(model, &iter, NULL, n))
		path = gtk_tree_model_get_path(model, &iter);

	return path;
}

/* Return path of the next queue track */

GtkTreePath* get_next_queue_track(PraghaPlaylist *cplaylist)
{
	GtkTreePath *path = NULL;

	path = gtk_tree_row_reference_get_path(cplaylist->queue_track_refs->data);

	/* Remove old next song. */
	trim_down_rand_track_refs(cplaylist);

	/*Remove the queue reference and update gui. */
	delete_queue_track_refs(path, cplaylist);
	requeue_track_refs (cplaylist);

	return path;
}

/* Return path of a first random track */

GtkTreePath* get_first_random_track(PraghaPlaylist *cplaylist)
{
	gint rnd;
	GtkTreePath *path = NULL;

	do {
		rnd = g_rand_int_range(cplaylist->rand,
				       0,
				       cplaylist->no_tracks);
		path = current_playlist_nth_track(rnd, cplaylist);

	} while (cplaylist->no_tracks > 1 && (path == NULL));

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next unique random track */

static GtkTreePath* get_next_unplayed_random_track(PraghaPlaylist *cplaylist)
{
	gint rnd;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gboolean played = TRUE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	while (played && cplaylist->unplayed_tracks) {
		rnd = g_rand_int_range(cplaylist->rand,
				       0,
				       cplaylist->no_tracks);
		path = current_playlist_nth_track(rnd, cplaylist);
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

static GtkTreePath* get_next_random_track(PraghaPlaylist *cplaylist)
{
	gint rnd;
	GtkTreePath *path = NULL, *rpath;

	rpath = gtk_tree_row_reference_get_path(cplaylist->curr_rand_ref);
	do {
		rnd = g_rand_int_range(cplaylist->rand,
				       0,
				       cplaylist->no_tracks);
		path = current_playlist_nth_track(rnd, cplaylist);
	} while (!gtk_tree_path_compare(rpath, path) &&
		 (cplaylist->no_tracks > 1));

	gtk_tree_path_free(rpath);

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next sequential track */

static GtkTreePath* get_next_sequential_track(PraghaPlaylist *cplaylist)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	gboolean ret = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	ret = gtk_tree_model_get_iter_first(model, &iter);

	/* If no tracks, return NULL.
	   If current track has been removed from the playlist,
	   return the first track. */

	if (!cplaylist->curr_seq_ref && !ret)
		return NULL;
	else if (!cplaylist->curr_seq_ref && ret) {
		path = gtk_tree_model_get_path(model, &iter);
		return path;
	}

	path = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);
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

static GtkTreePath* get_next_random_ref_track(PraghaPlaylist *cplaylist)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	i = g_list_find(cplaylist->rand_track_refs, cplaylist->curr_rand_ref);
	if (i) {
		j = g_list_next(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}
	return path;
}

/* Return path of the node previous to the current track from
   cstate->rand_track_refs */

static GtkTreePath* get_prev_random_track(PraghaPlaylist *cplaylist)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	if (!cplaylist->rand_track_refs)
		return NULL;

	i = g_list_find(cplaylist->rand_track_refs, cplaylist->curr_rand_ref);
	if (i) {
		j = g_list_previous(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}

	return path;
}

/* Return path of the previous sequential track */

static GtkTreePath* get_prev_sequential_track(PraghaPlaylist *cplaylist)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;

	if (!cplaylist->curr_seq_ref)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	path = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);
	gtk_tree_model_get_iter(model, &iter, path);

	if (!gtk_tree_path_prev(path)) {
		gtk_tree_path_free(path);
		path = NULL;
	}

	return path;
}

/* Add a new ref to cstate->rand_track_refs */

static void append_rand_track_refs(GtkTreeRowReference *ref, PraghaPlaylist *cplaylist)
{
	cplaylist->rand_track_refs = g_list_append(cplaylist->rand_track_refs,
						   ref);
}

/* Remove all nodes and free the list */

static void clear_rand_track_refs(PraghaPlaylist *cplaylist)
{
	if (cplaylist->rand_track_refs) {
		g_list_free_full(cplaylist->rand_track_refs, (GDestroyNotify) gtk_tree_row_reference_free);
		cplaylist->rand_track_refs = NULL;
	}

	cplaylist->curr_rand_ref = NULL;
}

/* Remove all nodes and free the list */

static void clear_queue_track_refs(PraghaPlaylist *cplaylist)
{
	if (cplaylist->queue_track_refs) {
		g_slist_free_full(cplaylist->queue_track_refs, (GDestroyNotify) gtk_tree_row_reference_free);
		cplaylist->queue_track_refs = NULL;
	}
}

/* Comparison function for column names */

static gint compare_playlist_column_name(gconstpointer a, gconstpointer b)
{
	const gchar *e1 = a;
	const gchar *e2 = b;

	return g_ascii_strcasecmp(e1, e2);
}

/* Function to add/delete columns from preferences */

static void
modify_current_playlist_columns(PraghaPlaylist* cplaylist,
				const gchar *col_name,
				gboolean state)
{
	gboolean pref_present;
	GSList *element;

	if (!col_name) {
		g_warning("Invalid column name");
		return;
	}

	pref_present = is_present_str_list(col_name, cplaylist->columns);

	/* Already in preferences */

	if (pref_present && state) {
		return;
	}

	/* Remove from preferences */

	else if (pref_present && !state) {
		element = g_slist_find_custom(cplaylist->columns,
				      col_name, compare_playlist_column_name);
		if (element) {
			g_free(element->data);
			cplaylist->columns =
				g_slist_delete_link(cplaylist->columns,
						    element);
		}
		else
			g_warning("Column : %s not found in preferences",
				  col_name);
	}

	/* Add to preferences */
 
	else if (!pref_present && state) {
		 cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(col_name));
	}
}

/* Function to jump to track on current playlist */

void
jump_to_path_on_current_playlist (PraghaPlaylist *cplaylist, GtkTreePath *path, gboolean center)
{
	GtkTreeSelection *selection;
	gint cx, cy, cnt_selected;

	GdkRectangle vrect;
	GdkRectangle crect;

	if (path) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
		cnt_selected = gtk_tree_selection_count_selected_rows(selection);

		if (cnt_selected > 1)
			return;

		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(GTK_TREE_SELECTION (selection), path);

		gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(cplaylist->view), &vrect);
		gtk_tree_view_get_cell_area(GTK_TREE_VIEW(cplaylist->view), path, NULL, &crect);

		gtk_tree_view_convert_widget_to_tree_coords(GTK_TREE_VIEW(cplaylist->view), crect.x, crect.y, &cx, &cy);

		if (cplaylist->shuffle) {
			if ((cy < vrect.y) || (cy + crect.height > vrect.y + vrect.height)) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cplaylist->view),
							     path, NULL, TRUE, 0.5, 0.0);
			}
		}
		else {
			if (cy < vrect.y) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cplaylist->view),
							     path, NULL, TRUE, 0.0, 0.0);
			}
			else if (cy + crect.height > vrect.y + vrect.height) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(cplaylist->view),
							     path, NULL, TRUE, 1.0, 0.0);
			}
		}
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(cplaylist->view),
					 path, NULL, FALSE);
	}
}

/* Select the song numbered according to the position in the playlist */

void select_numered_path_of_current_playlist(PraghaPlaylist *cplaylist, gint path_number, gboolean center)
{
	GtkTreePath *path = NULL;

	path = current_playlist_nth_track(path_number, cplaylist);

	jump_to_path_on_current_playlist (cplaylist, path, center);

	gtk_tree_path_free(path);
}

/*************************/
/* General playlist mgmt */
/*************************/

/* Update the state on 'Next', 'Prev' or selecting a new track
   from current playlist */

enum playlist_action
pragha_playlist_get_current_update_action(PraghaPlaylist* cplaylist)
{
	return cplaylist->current_update_action;
}

void
pragha_playlist_report_finished_action(PraghaPlaylist* cplaylist)
{
	cplaylist->current_update_action = PLAYLIST_NONE;
}

void
pragha_playlist_set_current_update_action(PraghaPlaylist* cplaylist, enum playlist_action action)
{
	cplaylist->current_update_action = action;
}

void
pragha_playlist_update_current_playlist_state(PraghaPlaylist* cplaylist, GtkTreePath *path)
{
	GtkTreeRowReference *rand_ref;
	GtkTreeModel *model;

	CDEBUG(DBG_VERBOSE, "Update the state from current playlist");

	/* Append the new reference to the list of played track references
	   to retrace the sequence */

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (!cplaylist->shuffle) {
		gtk_tree_row_reference_free(cplaylist->curr_seq_ref);
		cplaylist->curr_seq_ref = gtk_tree_row_reference_new(model, path);
	}

	if (cplaylist->shuffle) {
		switch (cplaylist->current_update_action) {
			/* If 'Prev', get the previous node from the track references */
			case PLAYLIST_PREV:
				if (cplaylist->curr_rand_ref) {
					cplaylist->curr_rand_ref =
						get_rand_ref_prev(cplaylist->curr_rand_ref,
								  cplaylist);
				}
			break;

			/* If 'Next', get the next node from the track references */
			/* Do this only if the current track and the
			   last node don't match */
			case PLAYLIST_NEXT:
				if (cplaylist->curr_rand_ref) {
					if (cplaylist->curr_rand_ref !=
					    (g_list_last(cplaylist->rand_track_refs)->data)) {
						cplaylist->curr_rand_ref =
							get_rand_ref_next(cplaylist->curr_rand_ref,
									  cplaylist);
						break;
					}
				}

			/* Append a new ref of the track to the track references */
			case PLAYLIST_CURR:
				rand_ref = gtk_tree_row_reference_new(model, path);
				cplaylist->rand_track_refs =
					g_list_append(cplaylist->rand_track_refs,
						      rand_ref);
				cplaylist->curr_rand_ref = rand_ref;
				break;
			default:
				break;
		}
	}

	/* Mark the track as dirty */

	current_playlist_set_dirty_track(cplaylist, path);
}

void update_current_playlist_view_new_track(struct con_win *cwin)
{
	GtkTreePath *path;
	path = current_playlist_get_actual(cwin->cplaylist);

	if(path) {
		pragha_playlist_update_pixbuf_path (cwin->cplaylist, path, cwin);
		jump_to_path_on_current_playlist (cwin->cplaylist, path, cwin->cplaylist->shuffle);
		gtk_tree_path_free(path);
	}
}

void update_current_playlist_view_track(struct con_win *cwin)
{
	GtkTreePath *path;
	path = current_playlist_get_actual(cwin->cplaylist);

	if(path) {
		pragha_playlist_update_pixbuf_path (cwin->cplaylist, path, cwin);
		gtk_tree_path_free(path);
	}
}

/* Return musicobject of the given path */

PraghaMusicobject *
current_playlist_mobj_at_path(GtkTreePath *path,
                              PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	return mobj;
}

/* Return path of musicobject already in tree */

GtkTreePath* current_playlist_path_at_mobj(PraghaMusicobject *mobj,
					   PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *ptr = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	
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

static void
reset_rand_track_refs(PraghaPlaylist *cplaylist, GtkTreeRowReference *ref)
{
	GtkTreePath *path;

	/* All songs can be played. */
	current_playlist_clear_dirty_all(cplaylist);
	clear_rand_track_refs(cplaylist);

	/* Set the current song as played. */
	append_rand_track_refs(ref, cplaylist);
	cplaylist->curr_rand_ref = ref;

	path = gtk_tree_row_reference_get_path(ref);
	current_playlist_set_dirty_track(cplaylist, path);

	gtk_tree_path_free(path);
}

void
pragha_playlist_set_first_rand_ref(PraghaPlaylist *cplaylist, GtkTreePath *path)
{
	GtkTreeModel *model;
	GtkTreeRowReference *ref;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	ref = gtk_tree_row_reference_new(model, path);
	reset_rand_track_refs(cplaylist, ref);
}

/* Return the path of the selected track */

GtkTreePath* current_playlist_get_selection(PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GtkTreePath *path = NULL;
	gint cnt_selected = 0;
	GList *list;

	if(cplaylist->changing)
		return NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
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

GtkTreePath* current_playlist_get_next(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GList *last;
	GtkTreeIter iter;
	gboolean rand_unplayed = FALSE, seq_last = FALSE;

	if(cplaylist->changing)
		return NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	/* Check if tree is empty */

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	if(cplaylist->queue_track_refs) {
		path = get_next_queue_track(cplaylist);
	}
	else {
		switch (cplaylist->shuffle) {
			case TRUE:
				last = g_list_last(cplaylist->rand_track_refs);
				if ((!cplaylist->curr_rand_ref) ||
				    (last && (cplaylist->curr_rand_ref == last->data))){
					path = get_next_unplayed_random_track(cplaylist);
					if (!path)
						rand_unplayed = TRUE;
				}
				else path = get_next_random_ref_track(cplaylist);
				break;
			case FALSE:
				path = get_next_sequential_track(cplaylist);
				if (!path) seq_last = TRUE;
				break;
			default:
				break;
		}
	}
	if (rand_unplayed && cplaylist->repeat)
		path = get_next_random_track(cplaylist);

	if (seq_last && cplaylist->repeat)
		path = current_playlist_nth_track(0, cplaylist);

	return path;
}

/* Return the path of the next(prev) track to be played */

GtkTreePath* current_playlist_get_prev(PraghaPlaylist *cplaylist)
{
	GtkTreePath *path = NULL;
	gboolean seq_first = FALSE;

	if(cplaylist->changing)
		return NULL;

	switch (cplaylist->shuffle) {
	case TRUE:
		path = get_prev_random_track(cplaylist);
		break;
	case FALSE:
		path = get_prev_sequential_track(cplaylist);
		if (!path) seq_first = TRUE;
		break;
	default:
		break;
	}

	if (seq_first && cplaylist->repeat)
		path = current_playlist_nth_track((cplaylist->no_tracks-1),
						  cplaylist);

	return path;
}

/* Return the path of the Actual track playing */

GtkTreePath* current_playlist_get_actual(PraghaPlaylist *cplaylist)
{
	GtkTreePath *path=NULL;

	if (cplaylist->shuffle && cplaylist->curr_rand_ref)
		path = gtk_tree_row_reference_get_path(cplaylist->curr_rand_ref);
	else if (!cplaylist->shuffle && cplaylist->curr_seq_ref)
		path = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);

	return path;
}

/* Dequeue selected rows from current playlist */

void pragha_playlist_dequeue_handler(PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GList *list;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	g_list_foreach (list, (GFunc) delete_queue_track_refs, cplaylist);
	requeue_track_refs(cplaylist);
	g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
}


/* Dequeue selected rows from current playlist */

void dequeue_current_playlist(GtkAction *action, struct con_win *cwin)
{
	pragha_playlist_dequeue_handler(cwin->cplaylist);
}

/* Queue selected rows from current playlist */

void pragha_playlist_queue_handler(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeRowReference *ref;
	GList *list, *l;
	gboolean is_queue = FALSE;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	l= list;
	while (l) {
		path = l->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
			if(!is_queue) {
				ref = gtk_tree_row_reference_new(model, path);
				cplaylist->queue_track_refs = g_slist_append(cplaylist->queue_track_refs, ref);
			}
		}
		gtk_tree_path_free(path);
		l = l->next;
	}
	requeue_track_refs(cplaylist);
	g_list_free (list);
}

/* Queue selected rows from current playlist */

void queue_current_playlist(GtkAction *action, struct con_win *cwin)
{
	pragha_playlist_queue_handler(cwin->cplaylist);
}

/* Toglle queue state of selection on current playlist. */

void toggle_queue_selected_current_playlist (PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean is_queue = FALSE;
	GList *list;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	while (list) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
			if(is_queue)
				delete_queue_track_refs(path, cplaylist);
			else {
				ref = gtk_tree_row_reference_new(model, path);
				cplaylist->queue_track_refs = g_slist_append(cplaylist->queue_track_refs, ref);
			}
		}
		gtk_tree_path_free(path);
		list = list->next;
	}
	requeue_track_refs(cplaylist);
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
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->cplaylist->view));
		n_select = gtk_tree_selection_count_selected_rows(selection);

		if(n_select==1){
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			if (gtk_tree_model_get_iter(model, &iter, list->data)){
				gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
				if(is_queue)
					delete_queue_track_refs(list->data, cwin->cplaylist);
				else{
					ref = gtk_tree_row_reference_new(model, list->data);
					cwin->cplaylist->queue_track_refs = g_slist_append(cwin->cplaylist->queue_track_refs, ref);
				}
				requeue_track_refs(cwin->cplaylist);
			}
			gtk_tree_path_free(list->data);
			g_list_free (list);
		}
		return TRUE;
	}
	return FALSE;
}

/* Remove selected rows from current playlist */

void
pragha_playlist_remove_selection(PraghaPlaylist *cplaylist, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path, *next;
	GtkTreeIter iter;
	GList *list = NULL, *i = NULL;
	PraghaMusicobject *mobj = NULL;
	gboolean played = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		/* Select the next row to the last selected */

		gtk_tree_view_get_cursor(GTK_TREE_VIEW(cplaylist->view), &next, NULL);
		do {
			if(gtk_tree_selection_path_is_selected(selection, next) == FALSE)
				break;

			gtk_tree_path_next(next);
		}
		while(next != NULL);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW(cplaylist->view), next, NULL, FALSE);
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
			delete_rand_track_refs(path, cplaylist);
			delete_queue_track_refs(path, cplaylist);
			test_clear_curr_seq_ref(path, cplaylist);

			if (gtk_tree_model_get_iter(model, &iter, path)) {
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
				g_object_unref(mobj);
				gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				cplaylist->no_tracks--;
				if (!played)
					cplaylist->unplayed_tracks--;
			}
			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
		}

		g_list_free(list);
	}

	requeue_track_refs (cplaylist);
}

void remove_from_playlist(GtkAction *action, struct con_win *cwin)
{
	set_watch_cursor (cwin->mainwindow);
	pragha_playlist_remove_selection(cwin->cplaylist, cwin);
	remove_watch_cursor (cwin->mainwindow);

	update_status_bar_playtime(cwin);
}

/* Crop selected rows from current playlist */

void
pragha_playlist_crop_selection(PraghaPlaylist *cplaylist, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret, played = FALSE;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GSList *to_delete = NULL, *i = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));

	/* At least one row must be selected */

	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

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
	pragha_playlist_set_changing(cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), NULL);

	for (i=to_delete; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);
		delete_rand_track_refs(path, cplaylist);
		delete_queue_track_refs(path, cplaylist);
		test_clear_curr_seq_ref(path, cplaylist);

		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			g_object_unref(mobj);
			gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
			cplaylist->no_tracks--;
			if (!played)
				cplaylist->unplayed_tracks--;

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}
		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), model);
	pragha_playlist_set_changing(cplaylist, FALSE);
	g_object_unref(model);

	requeue_track_refs (cwin->cplaylist);
	g_slist_free(to_delete);

}

void crop_current_playlist(GtkAction *action, struct con_win *cwin)
{
	set_watch_cursor (cwin->mainwindow);
	pragha_playlist_crop_selection(cwin->cplaylist, cwin);
	remove_watch_cursor (cwin->mainwindow);

	update_status_bar_playtime(cwin);
}

/* Show track properties dialog
   This function is a fscking eyesore. */

static void
track_properties_response(GtkDialog *dialog,
			gint response,
			gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void track_properties(PraghaMusicobject *mobj, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *properties_table;
	GtkWidget *label_length, *label_bitrate, *label_channels, *label_samplerate, *label_folder, *label_filename;
	GtkWidget *info_length, *info_bitrate, *info_channels, *info_samplerate, *info_folder, *info_filename;

	gchar *length = NULL, *bitrate = NULL, *channels = NULL, *samplerate = NULL, *folder = NULL, *filename = NULL;

	if(!mobj)
		return;

	length = convert_length_str(pragha_musicobject_get_length(mobj));
	bitrate = g_strdup_printf("%d kbps", pragha_musicobject_get_bitrate(mobj));
	channels = g_strdup_printf("%d %s", pragha_musicobject_get_channels(mobj), _("Channels"));
	samplerate = g_strdup_printf("%d Hz", pragha_musicobject_get_samplerate(mobj));
	folder = get_display_filename(pragha_musicobject_get_file(mobj), TRUE);
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

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(track_properties_response), NULL);

	gtk_widget_show_all(dialog);

	g_free(length);
	g_free(bitrate);
	g_free(channels);
	g_free(samplerate);
	g_free(folder);
	g_free(filename);
}

void
pragha_playlist_remove_all (PraghaPlaylist *cplaylist, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	clear_rand_track_refs(cplaylist);
	clear_queue_track_refs(cplaylist);
	clear_curr_seq_ref(cplaylist);

	ret = gtk_tree_model_get_iter_first(model, &iter);

	while (ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		g_object_unref(mobj);
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	gtk_list_store_clear(GTK_LIST_STORE(model));

	cplaylist->no_tracks = 0;
	cplaylist->unplayed_tracks = 0;
}

void
current_playlist_clear_action (GtkAction *action, struct con_win *cwin)
{
	set_watch_cursor (cwin->mainwindow);
	pragha_playlist_remove_all (cwin->cplaylist, cwin);
	remove_watch_cursor (cwin->mainwindow);

	update_status_bar_playtime(cwin);
}

/* Update a track to the current playlist */

void pragha_playlist_update_change_tag(PraghaPlaylist *cplaylist, GtkTreeIter *iter, gint changed, PraghaMusicobject *mobj)
{
	GtkTreeModel *model;
	gchar *ch_track_no = NULL, *ch_year = NULL, *ch_title = NULL;

	if (!changed)
		return;

	CDEBUG(DBG_VERBOSE, "Track Updates: 0x%x", changed);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (changed & TAG_TNO_CHANGED) {
		ch_track_no = g_strdup_printf("%d", pragha_musicobject_get_track_no(mobj));
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_TRACK_NO, ch_track_no, -1);
		g_free(ch_track_no);
	}
	if (changed & TAG_TITLE_CHANGED) {
		if(pragha_musicobject_get_title(mobj) && strlen(pragha_musicobject_get_title(mobj)))
			ch_title = g_strdup(pragha_musicobject_get_title(mobj));
		else
			ch_title = get_display_name(mobj);

		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_TITLE, ch_title, -1);
		g_free(ch_title);
	}
	if (changed & TAG_ARTIST_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_ARTIST, pragha_musicobject_get_artist(mobj),-1);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_ALBUM, pragha_musicobject_get_album(mobj),-1);
	}
	if (changed & TAG_GENRE_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_GENRE, pragha_musicobject_get_genre(mobj),-1);
	}
	if (changed & TAG_YEAR_CHANGED) {
		ch_year = g_strdup_printf("%d", pragha_musicobject_get_year(mobj));
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_YEAR, ch_year, -1);
		g_free(ch_year);
	}
	if (changed & TAG_COMMENT_CHANGED) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, P_COMMENT, pragha_musicobject_get_comment(mobj),-1);
	}
}

/* Get all music objects of references list and update tags */

void
pragha_playlist_update_ref_list_change_tag(PraghaPlaylist *cplaylist, GList *list, gint changed)
{
	PraghaMusicobject *mobj = NULL;
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	GList *i;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	for (i = list; i != NULL; i = i->next) {
		mobj = NULL;

		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);

		if (G_LIKELY(gtk_tree_model_get_iter(model, &iter, path)))
			gtk_tree_path_free(path);
		else
			continue;

		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

		if (G_UNLIKELY(mobj == NULL)) {
			g_warning("Invalid mobj pointer");
			continue;
		}
		pragha_playlist_update_change_tag(cplaylist, &iter, changed, mobj);
	}
}

void
pragha_playlist_update_current_track(PraghaPlaylist *cplaylist, gint changed, PraghaMusicobject *mobj)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;

	path = current_playlist_get_actual(cplaylist);

	if(!path)
		return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (gtk_tree_model_get_iter(model, &iter, path))
		pragha_playlist_update_change_tag(cplaylist, &iter, changed, mobj);

	gtk_tree_path_free(path);
}

/* Insert a track to the current playlist */

static void
insert_current_playlist(PraghaPlaylist *cplaylist,
			GtkTreeModel *model,
			PraghaMusicobject *mobj,
			GtkTreeViewDropPosition droppos,
			GtkTreeIter *pos)
{
	GtkTreeIter iter;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;

	if(model == NULL)
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}

	if(pragha_musicobject_get_track_no(mobj) > 0)
		ch_track_no = g_strdup_printf("%d", pragha_musicobject_get_track_no(mobj));
	if(pragha_musicobject_get_year(mobj) > 0)
		ch_year = g_strdup_printf("%d", pragha_musicobject_get_year(mobj));
	if(pragha_musicobject_get_length(mobj))
		ch_length = convert_length_str(pragha_musicobject_get_length(mobj));
	if(pragha_musicobject_get_bitrate(mobj))
		ch_bitrate = g_strdup_printf("%d", pragha_musicobject_get_bitrate(mobj));

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
			   P_TITLE, (pragha_musicobject_get_title(mobj) && strlen(pragha_musicobject_get_title(mobj))) ?
					pragha_musicobject_get_title(mobj) : ch_filename,
			   P_ARTIST, pragha_musicobject_get_artist(mobj),
			   P_ALBUM, pragha_musicobject_get_album(mobj),
			   P_GENRE, pragha_musicobject_get_genre(mobj),
			   P_BITRATE, ch_bitrate,
			   P_YEAR, ch_year,
			   P_COMMENT, pragha_musicobject_get_comment(mobj),
			   P_LENGTH, ch_length,
			   P_FILENAME, ch_filename,
			   P_PLAYED, FALSE,
			   -1);

	/* Increment global count of tracks */

	cplaylist->no_tracks++;
	cplaylist->unplayed_tracks++;

	/* Have to give control to GTK periodically ... */
	if (pragha_process_gtk_events ())
		return;

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

/* Append a track to the current playlist */

void append_current_playlist(PraghaPlaylist *cplaylist, GtkTreeModel *model, PraghaMusicobject *mobj)
{
	append_current_playlist_ex(cplaylist, model, mobj, NULL);
}

void append_current_playlist_ex(PraghaPlaylist *cplaylist, GtkTreeModel *model, PraghaMusicobject *mobj, GtkTreePath **path)
{
	GtkTreeIter iter;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;

	if (model == NULL)
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}

	if(pragha_musicobject_get_track_no(mobj) > 0)
		ch_track_no = g_strdup_printf("%d", pragha_musicobject_get_track_no(mobj));
	if(pragha_musicobject_get_year(mobj) > 0)
		ch_year = g_strdup_printf("%d", pragha_musicobject_get_year(mobj));
	if(pragha_musicobject_get_length(mobj))
		ch_length = convert_length_str(pragha_musicobject_get_length(mobj));
	if(pragha_musicobject_get_bitrate(mobj))
		ch_bitrate = g_strdup_printf("%d", pragha_musicobject_get_bitrate(mobj));

	ch_filename = get_display_name(mobj);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   P_MOBJ_PTR, mobj,
			   P_QUEUE, NULL,
			   P_BUBBLE, FALSE,
			   P_STATUS_PIXBUF, NULL,
			   P_TRACK_NO, ch_track_no,
			   P_TITLE, (pragha_musicobject_get_title(mobj) &&
			             strlen(pragha_musicobject_get_title(mobj))) ?
	                             pragha_musicobject_get_title(mobj) :
	                             ch_filename,
			   P_ARTIST, pragha_musicobject_get_artist(mobj),
			   P_ALBUM, pragha_musicobject_get_album(mobj),
			   P_GENRE, pragha_musicobject_get_genre(mobj),
			   P_BITRATE, ch_bitrate,
			   P_YEAR, ch_year,
			   P_COMMENT, pragha_musicobject_get_comment(mobj),
			   P_LENGTH, ch_length,
			   P_FILENAME, ch_filename,
			   P_PLAYED, FALSE,
			   -1);

	/* Increment global count of tracks */

	cplaylist->no_tracks++;
	cplaylist->unplayed_tracks++;

	if(path)
		*path = gtk_tree_model_get_path(model, &iter);

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

void
pragha_playlist_append_mobj_and_play(PraghaPlaylist *cplaylist, PraghaMusicobject *mobj)
{
	GtkTreePath *path;

	append_current_playlist_ex(cplaylist, NULL, mobj, &path);
	pragha_playlist_activate_path(cplaylist, path);
	gtk_tree_path_free(path);
}

/* Insert a list of mobj to the current playlist. */

void
pragha_playlist_insert_mobj_list(PraghaPlaylist *cplaylist,
				 GList *list,
				 GtkTreeViewDropPosition droppos,
				 GtkTreeIter *pos)
{
	GtkTreeModel *model;
	PraghaMusicobject *mobj;
	GList *l;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	g_object_ref(model);
	pragha_playlist_set_changing(cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), NULL);

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		insert_current_playlist(cplaylist, model, mobj, droppos, pos);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), model);
	pragha_playlist_set_changing(cplaylist, FALSE);
	g_object_unref(model);

}

/* Append a list of mobj to the current playlist */

void
pragha_playlist_append_mobj_list(PraghaPlaylist *cplaylist, GList *list)
{
	GtkTreeModel *model;
	PraghaMusicobject *mobj;
	GList *l;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	g_object_ref(model);
	pragha_playlist_set_changing(cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), NULL);

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		append_current_playlist(cplaylist, model, mobj);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), model);
	pragha_playlist_set_changing(cplaylist, FALSE);
	g_object_unref(model);
}

/* Test if the song is already in the mobj list */

gboolean
pragha_mobj_list_already_has_title_of_artist(GList *list,
					     const gchar *title,
					     const gchar *artist)
{
	PraghaMusicobject *mobj = NULL;
	GList *i;

	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;
		if((0 == g_ascii_strcasecmp(pragha_musicobject_get_title(mobj), title)) &&
		   (0 == g_ascii_strcasecmp(pragha_musicobject_get_artist(mobj), artist)))
		   	return TRUE;
	}
	return FALSE;
}

/* Test if the song is already in the playlist.*/

gboolean
pragha_playlist_already_has_title_of_artist(PraghaPlaylist *cplaylist,
					    const gchar *title,
					    const gchar *artist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(cplaylist->view));
	ret = gtk_tree_model_get_iter_first (model, &iter);
	while (ret) {
		gtk_tree_model_get (model, &iter, P_MOBJ_PTR, &mobj, -1);

		if((0 == g_ascii_strcasecmp(pragha_musicobject_get_title(mobj), title)) &&
		   (0 == g_ascii_strcasecmp(pragha_musicobject_get_artist(mobj), artist)))
		   	return TRUE;

		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return FALSE;
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

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

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->view));
	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	/* If no tracks have been selected in the current playlist,
	   return immediately. */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						cwin->cplaylist->view));
	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	playlist = get_playlist_name(cwin, SAVE_SELECTED, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist(playlist, SAVE_SELECTED, cwin);
			update_menu_playlist_changes(cwin);
			init_library_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist(playlist, SAVE_SELECTED, cwin);
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

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->view));

	if(pragha_playlist_is_changing(cwin->cplaylist))
		return;

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	playlist = get_playlist_name(cwin, SAVE_COMPLETE, &choice);
	if (playlist) {
		switch(choice) {
		case NEW_PLAYLIST:
			new_playlist(playlist, SAVE_COMPLETE, cwin);
			update_menu_playlist_changes(cwin);
			init_library_view(cwin);
			break;
		case APPEND_PLAYLIST:
			append_playlist(playlist, SAVE_COMPLETE, cwin);
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

/*******************/
/* Event Callbacks */
/*******************/

/* Handler for row double click / kboard enter */

static void
current_playlist_row_activated_cb(GtkTreeView *current_playlist,
				  GtkTreePath *path,
				  GtkTreeViewColumn *column,
				  struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	PraghaMusicobject *mobj;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->view));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	if (pragha_playlist_is_shuffle(cwin->cplaylist)) {
		if (pragha_backend_get_state (cwin->backend) == ST_STOPPED) {
			clear_rand_track_refs(cwin->cplaylist);
			current_playlist_clear_dirty_all(cwin->cplaylist);
		}
		else {
			trim_down_rand_track_refs(cwin->cplaylist);
		}
	}

	/* Stop to set ready and clear all info */
	pragha_backend_stop(cwin->backend);

	/* Start playing new track */
	cwin->cplaylist->current_update_action = PLAYLIST_CURR;
	pragha_playlist_update_current_playlist_state(cwin->cplaylist, path);

	pragha_backend_start(cwin->backend, mobj);
}

void
copy_tags_to_selection_action(GtkAction *action, struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	gint changed = 0;

	mobj = g_object_get_data (G_OBJECT(action), "mobj");
	changed = GPOINTER_TO_INT(g_object_get_data (G_OBJECT(action), "change"));

	/* Check if user is trying to set the same track no for multiple tracks */
	if (changed & TAG_TNO_CHANGED) {
		if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(mobj), cwin))
			return;
	}

	/* Check if user is trying to set the same title/track no for
	   multiple tracks */
	if (changed & TAG_TITLE_CHANGED) {
		if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(mobj), cwin))
			return;
	}

	copy_tags_selection_current_playlist(mobj, changed, cwin);
}

static void
personalize_copy_tag_to_seleccion(GtkWidget *item_widget,
				  GtkTreeViewColumn *column,
				  GtkTreeIter *iter,
				  PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GList *list = NULL;
	gint icolumn = 0;
	GtkAction *action = NULL;
	gchar *label = NULL;
	PraghaMusicobject *mobj = NULL;
	gint change = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	gtk_tree_model_get(model, iter, P_MOBJ_PTR, &mobj, -1);

	/* Get the column clicked and set menu. */

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(cplaylist->view));
	icolumn = g_list_index(list, column);

	switch (icolumn) {
		case 1: {
			change = TAG_TNO_CHANGED;
			label = g_strdup_printf(_("Copy \"%i\" to selected track numbers"),
						pragha_musicobject_get_track_no(mobj));
			break;
			}
		case 2: {
			change = TAG_TITLE_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected titles"),
						pragha_musicobject_get_title(mobj));
			break;
			}
		case 3: {
			change = TAG_ARTIST_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected artists"),
						pragha_musicobject_get_artist(mobj));
			break;
			}
		case 4: {
			change = TAG_ALBUM_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected albums"),
						pragha_musicobject_get_album(mobj));
			break;
		}
		case 5: {
			change = TAG_GENRE_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected genres"),
						pragha_musicobject_get_genre(mobj));
			break;
		}
		case 7: {
			change = TAG_YEAR_CHANGED;
			label = g_strdup_printf(_("Copy \"%i\" to selected years"),
						pragha_musicobject_get_year(mobj));
			break;
		}
		case 8: {
			change = TAG_COMMENT_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected comments"),
						pragha_musicobject_get_comment(mobj));
			break;
		}
		default: {
			break;
		}
	}

	if (change) {
		action = gtk_ui_manager_get_action(cplaylist->cp_context_menu,
						   "/popup/Copy tag to selection");

		g_object_set_data(G_OBJECT(action), "mobj", mobj);
		g_object_set_data(G_OBJECT(action), "change", GINT_TO_POINTER(change));

		gtk_action_set_label(GTK_ACTION(action), label);
		gtk_widget_show (GTK_WIDGET(item_widget));
	}
	else {
		gtk_widget_hide (GTK_WIDGET(item_widget));
	}

	g_list_free(list);
	g_free(label);
}

/* Handler for current playlist click */

static gboolean
current_playlist_button_press_cb(GtkWidget *widget,
				 GdkEventButton *event,
				 PraghaPlaylist *cplaylist)
{
	GtkWidget *popup_menu, *item_widget;
	GtkTreeSelection *selection;
	gint n_select = 0;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	gboolean ret = FALSE, is_queue = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));

	switch(event->button){
	case 1:
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL)){
			if (gtk_tree_selection_path_is_selected(selection, path)
			    && !(event->state & GDK_CONTROL_MASK)
			    && !(event->state & GDK_SHIFT_MASK)) {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_false, cplaylist, NULL);
			}
			else {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cplaylist, NULL);
			}
			gtk_tree_path_free(path);
		}
		break;
	case 3:
		if (cplaylist->no_tracks == 0) {
			popup_menu = gtk_ui_manager_get_widget(cplaylist->cp_null_context_menu, "/popup");
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL, event->button, event->time);
			ret = FALSE;
			break;
		}
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, &column, NULL, NULL)) {
			if (!(gtk_tree_selection_path_is_selected(selection, path))){
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}

			n_select = gtk_tree_selection_count_selected_rows(selection);

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/ToolsMenu");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), (n_select == 1));

			if (gtk_tree_model_get_iter(model, &iter, path)){
				gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);

				if(is_queue){
					item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Queue track");
					gtk_widget_hide(GTK_WIDGET(item_widget));

					item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Dequeue track");
					gtk_widget_show(GTK_WIDGET(item_widget));
				}
				else{
					item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Queue track");
					gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);
					gtk_widget_show(GTK_WIDGET(item_widget));

					item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Dequeue track");
					gtk_widget_hide(GTK_WIDGET(item_widget));
				}
				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Remove from playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Crop playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Add to another playlist");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);

				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Copy tag to selection");

				if(n_select > 1)
					personalize_copy_tag_to_seleccion(item_widget, column, &iter, cplaylist);
				else
					gtk_widget_hide (GTK_WIDGET(item_widget));

				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Edit tags");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), TRUE);
			}
			else{
				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Queue track");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);
				gtk_widget_show(GTK_WIDGET(item_widget));

				item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Dequeue track");
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
			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Queue track");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);
			gtk_widget_show(GTK_WIDGET(item_widget));

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Dequeue track");
			gtk_widget_hide(GTK_WIDGET(item_widget));

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Remove from playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Crop playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Add to another playlist");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/ToolsMenu");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Copy tag to selection");
			gtk_widget_hide (GTK_WIDGET(item_widget));

			item_widget = gtk_ui_manager_get_widget(cplaylist->cp_context_menu, "/popup/Edit tags");
			gtk_widget_set_sensitive (GTK_WIDGET(item_widget), FALSE);

			gtk_tree_selection_unselect_all(selection);
		}

		popup_menu = gtk_ui_manager_get_widget(cplaylist->cp_context_menu,
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

static gboolean
current_playlist_button_release_cb(GtkWidget *widget,
				   GdkEventButton *event,
				   PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));

	if((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK) || (cplaylist->dragging == TRUE) || (event->button!=1)){
		gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cplaylist, NULL);
		cplaylist->dragging = FALSE;
		return FALSE;
	}

	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL);

	if (path){
		gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cplaylist, NULL);
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, path);
		gtk_tree_path_free(path);
	}
	return FALSE;
}

/* Handler for column header right click popup menu */

gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       PraghaPlaylist *cplaylist)
{
	gboolean ret = FALSE;

	switch(event->button) {
	case 3: {
		gtk_menu_popup(GTK_MENU(cplaylist->header_context_menu),
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

static gboolean
dnd_current_playlist_begin(GtkWidget *widget,
			   GdkDragContext *context,
			   PraghaPlaylist *cplaylist)
{
	cplaylist->dragging = TRUE;
	return FALSE;
}

/* Callback for DnD signal 'drag-data-get' */

static void
drag_current_playlist_get_data (GtkWidget *widget,
				GdkDragContext *context,
				GtkSelectionData *selection_data,
				guint target_type,
				guint time,
				PraghaPlaylist *cplaylist)
{
	g_assert (selection_data != NULL);

	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list, *i;
	GtkTreePath *path;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	guint uri_i = 0;
	gchar **uri_list;

        switch (target_type){
		case TARGET_URI_LIST:
			CDEBUG(DBG_VERBOSE, "DnD: TARGET_URI_LIST");

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			uri_list = g_new(gchar* , gtk_tree_selection_count_selected_rows(selection) + 1);

			for (i=list; i != NULL; i = i->next){
				path = i->data;
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

				if (G_LIKELY(mobj &&
				    pragha_musicobject_get_file_type(mobj) != FILE_CDDA &&
				    pragha_musicobject_get_file_type(mobj) != FILE_HTTP))
					uri_list[uri_i++] = g_filename_to_uri(pragha_musicobject_get_file(mobj), NULL, NULL);

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

static gboolean
dnd_current_playlist_drop(GtkWidget *widget,
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
			     PraghaPlaylist *cplaylist)
{
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GList *list = NULL, *l;

	CDEBUG(DBG_VERBOSE, "Dnd: Reorder");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	/* Clear sort */
	clear_sort_current_playlist(NULL, cplaylist);

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
	gchar *name = NULL;
	enum node_type node_type;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;

	CDEBUG(DBG_VERBOSE, "Dnd: Library");

	cwin->cstate->view_change = TRUE;

	ref_arr = *(GArray **)gtk_selection_data_get_data(data);
	if (!ref_arr)
		g_warning("No selections to process in DnD");

	library_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));

	/* Dnd from the library, so will read everything from database. */

	db_begin_transaction(cwin->cdbase);

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
					list = add_playlist_to_mobj_list(name, list, TRUE, cwin);
					g_free(name);
					break;
				case NODE_RADIO:
					gtk_tree_model_get(library_model, &iter, L_NODE_DATA, &name, -1);
					list = add_radio_to_mobj_list(name, list, TRUE, cwin);
					g_free(name);
					break;
				default:
					break;
			}

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return NULL;

			gtk_tree_path_free(path);
		}
		i++;
	} while (path != NULL);

	db_commit_transaction(cwin->cdbase);

	cwin->cstate->view_change = FALSE;

	g_array_free(ref_arr, FALSE);

	return list;
}

GList *
dnd_current_playlist_received_uri_list(GtkSelectionData *data,
						   struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
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
			if (pragha_process_gtk_events ())
				return NULL;

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
	PraghaMusicobject *mobj = NULL;
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
		if (pragha_process_gtk_events ())
			return NULL;
	}
	g_free(filename);

	return list;
}

/* Callback for DnD signal 'drag-data-received' */

static void
dnd_current_playlist_received(GtkWidget *playlist_view,
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
	GList *list = NULL;
	gint prev_tracks = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(playlist_view));

	is_row = gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(playlist_view),
						x, y,
						&dest_path,
						&pos);

	gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(playlist_view), &vrect);
	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(playlist_view), dest_path, NULL, &crect);
	
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

	if (gtk_drag_get_source_widget(context) == playlist_view) {
		dnd_current_playlist_reorder(model, &dest_iter, pos, cwin->cplaylist);
		goto exit;
	}

	/* Show busy mouse icon */

	set_watch_cursor (cwin->mainwindow);

	/* Save the last position to select the songs added */

	prev_tracks = cwin->cplaylist->no_tracks;

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

	if (is_row)
		pragha_playlist_insert_mobj_list(cwin->cplaylist, list, pos, &dest_iter);
	else
		pragha_playlist_append_mobj_list(cwin->cplaylist, list);

	update_status_bar_playtime(cwin);

	if (is_row)
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(playlist_view), dest_path, NULL, TRUE, row_align, 0.0);
	else
		select_numered_path_of_current_playlist(cwin->cplaylist, prev_tracks, TRUE);

	/* Remove busy mouse icon */

	remove_watch_cursor (cwin->mainwindow);

	g_list_free(list);

exit:
	gtk_tree_path_free(dest_path);
	gtk_drag_finish(context, TRUE, FALSE, time);
}

/* Get a list of all music objects on current playlist */

GList *
pragha_playlist_get_mobj_list(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;
	gboolean valid;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get (model, &iter, P_MOBJ_PTR, &mobj, -1);

		if (G_LIKELY(mobj))
			list = g_list_prepend(list, mobj);

		valid = gtk_tree_model_iter_next(model, &iter);
	}
	return list;
}

/* Get a list of selected music objects on current playlist */

GList *
pragha_playlist_get_selection_mobj_list(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL, *mlist = NULL, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

			if (G_LIKELY(mobj))
				mlist = g_list_prepend(mlist, mobj);

			gtk_tree_path_free(path);
		}
		g_list_free (list);
	}
	return mlist;
}

GList *
pragha_playlist_get_selection_ref_list(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GList *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	/* Get references from the paths and store them in the 'data'
	   portion of the list elements.
	   This idea was inspired by code from 'claws-mail' */

	for (i = list; i != NULL; i = i->next) {
		path = i->data;
		ref = gtk_tree_row_reference_new(model, path);
		i->data = ref;
		gtk_tree_path_free(path);
	}

	return list;
}

/* Get the musicobject of seleceted track on current playlist */

PraghaMusicobject *
pragha_playlist_get_selected_musicobject(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list != NULL) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (!mobj)
				g_warning("Invalid mobj pointer");
		}

		g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);
	}

	return mobj;
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

	playlist_id = find_playlist_db(SAVE_PLAYLIST_STATE, cwin->cdbase);
	if (!playlist_id)
		playlist_id = add_new_playlist_db(SAVE_PLAYLIST_STATE,
						  cwin->cdbase);
	else
		flush_playlist_db(playlist_id, cwin->cdbase);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->view));
	if (!gtk_tree_model_get_iter_first(model, &iter))
		return;

	save_playlist(playlist_id, SAVE_COMPLETE, cwin);

	/* Save reference to current song. */

	path = current_playlist_get_actual(cwin->cplaylist);
	if(path) {
		ref_char = gtk_tree_path_to_string (path);
		gtk_tree_path_free(path);

		pragha_preferences_set_string(cwin->preferences,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF,
					      ref_char);
		g_free (ref_char);
	}
	else {
		pragha_preferences_remove_key(cwin->preferences,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF);
	}
}

/* Init current playlist on application bringup,
   restore stored playlist */

static void init_playlist_current_playlist(struct con_win *cwin)
{
	gchar *s_playlist, *query, *file;
	gint playlist_id, location_id, i = 0;
	struct db_result result;
	PraghaMusicobject *mobj;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->view));

	set_watch_cursor (cwin->mainwindow);

	g_object_ref(model);
	pragha_playlist_set_changing(cwin->cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->cplaylist->view), NULL);

	s_playlist = sanitize_string_sqlite3(SAVE_PLAYLIST_STATE);
	playlist_id = find_playlist_db(s_playlist, cwin->cdbase);
	query = g_strdup_printf("SELECT FILE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d",
				playlist_id);
	exec_sqlite_query(query, cwin->cdbase, &result);

	for_each_result_row(result, i) {
		file = sanitize_string_sqlite3(result.resultp[i]);
		/* TODO: Fix this negradaaa!. */
		if(g_str_has_prefix(file, "Radio:") == FALSE) {
			if ((location_id = find_location_db(file, cwin->cdbase)))
				mobj = new_musicobject_from_db(location_id, cwin);
			else
				mobj = new_musicobject_from_file(result.resultp[i]);
		}
		else {
			mobj = new_musicobject_from_location(file + strlen("Radio:"), file + strlen("Radio:"), cwin);
		}
		append_current_playlist(cwin->cplaylist, model, mobj);
		g_free(file);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->cplaylist->view), model);
	pragha_playlist_set_changing(cwin->cplaylist, FALSE);
	g_object_unref(model);

	update_status_bar_playtime(cwin);
	remove_watch_cursor (cwin->mainwindow);

	sqlite3_free_table(result.resultp);
	g_free(s_playlist);
}

void init_current_playlist_view(struct con_win *cwin)
{
	gchar *ref = NULL;
	GtkTreePath *path = NULL;

	init_playlist_current_playlist(cwin);

	ref = pragha_preferences_get_string(cwin->preferences,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF);

	if (!ref)
		return;

	path = gtk_tree_path_new_from_string(ref);
	jump_to_path_on_current_playlist (cwin->cplaylist, path, cwin->cplaylist->shuffle);

	gtk_tree_path_free(path);
	g_free(ref);
}

/* Initialize columns of current playlist */

void init_current_playlist_columns(PraghaPlaylist* cplaylist, struct con_win *cwin)
{
	gchar **columns;
	const gchar *col_name;
	GtkTreeViewColumn *col;
	GList *list = NULL, *i;
	GSList *j;
	gint k = 0;
	gint *col_widths;
	gsize cnt = 0, isize;

	columns = pragha_preferences_get_string_list(cwin->preferences,
						     GROUP_PLAYLIST,
						     KEY_PLAYLIST_COLUMNS,
						     &cnt);
	if (columns) {
		for (isize=0; isize < cnt; isize++) {
			cplaylist->columns =
				g_slist_append(cplaylist->columns,
					       g_strdup(columns[isize]));
		}
		g_strfreev(columns);
	}
	else {
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_TITLE_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_ARTIST_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_ALBUM_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_LENGTH_STR));
	}

	col_widths = pragha_preferences_get_integer_list(cwin->preferences,
							 GROUP_PLAYLIST,
							 KEY_PLAYLIST_COLUMN_WIDTHS,
							 &cnt);
	if (col_widths) {
		for (isize = 0; isize < cnt; isize++) {
			cplaylist->column_widths =
				g_slist_append(cplaylist->column_widths,
					       GINT_TO_POINTER(col_widths[isize]));
		}
		g_free(col_widths);
	}
	else {
		for (isize=0; isize < 4; isize++) {
			cplaylist->column_widths =
				g_slist_append(cplaylist->column_widths,
				       GINT_TO_POINTER(DEFAULT_PLAYLIST_COL_WIDTH));
		}
	}

	/* Mark only the columns that are present in
	   cplaylist->columns as visible.
	   And set their sizes */

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(cplaylist->view));

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			col = i->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cplaylist->columns)) {
				j = g_slist_nth(cplaylist->column_widths,
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
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view), 0);
	gtk_tree_view_column_set_visible(col, TRUE);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(col, 32);
}

static GtkWidget*
create_header_context_menu(PraghaPlaylist* cplaylist)
{
	GtkWidget *menu;
	GtkWidget *toggle_track,
		*toggle_title,
		*toggle_artist,
		*toggle_album,
		*toggle_genre,
		*toggle_bitrate,
		*toggle_year,
		*toggle_comment,
		*toggle_length,
		*toggle_filename,
		*separator,
		*action_clear_sort;

	menu = gtk_menu_new();

	/* Create the checkmenu items */

	toggle_track = gtk_check_menu_item_new_with_label(_("Track"));
	toggle_title = gtk_check_menu_item_new_with_label(_("Title"));
	toggle_artist = gtk_check_menu_item_new_with_label(_("Artist"));
	toggle_album = gtk_check_menu_item_new_with_label(_("Album"));
	toggle_genre = gtk_check_menu_item_new_with_label(_("Genre"));
	toggle_bitrate = gtk_check_menu_item_new_with_label(_("Bitrate"));
	toggle_year = gtk_check_menu_item_new_with_label(_("Year"));
	toggle_comment = gtk_check_menu_item_new_with_label(_("Comment"));
	toggle_length = gtk_check_menu_item_new_with_label(_("Length"));
	toggle_filename = gtk_check_menu_item_new_with_label(_("Filename"));
	separator = gtk_separator_menu_item_new ();

	action_clear_sort = gtk_image_menu_item_new_with_label(_("Clear sort"));
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(action_clear_sort),
                gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));

	/* Add the items to the menu */

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_track);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_title);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_artist);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_album);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_genre);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_bitrate);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_year);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_comment);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_length);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_filename);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), action_clear_sort);

	/* Initialize the state of the items */

	if (is_present_str_list(P_TRACK_NO_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_track), TRUE);
	if (is_present_str_list(P_TITLE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_title), TRUE);
	if (is_present_str_list(P_ARTIST_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_artist), TRUE);
	if (is_present_str_list(P_ALBUM_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_album), TRUE);
	if (is_present_str_list(P_GENRE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_genre), TRUE);
	if (is_present_str_list(P_BITRATE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_bitrate), TRUE);
	if (is_present_str_list(P_YEAR_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_year), TRUE);
	if (is_present_str_list(P_COMMENT_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_comment), TRUE);
	if (is_present_str_list(P_LENGTH_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_length), TRUE);
	if (is_present_str_list(P_FILENAME_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_filename), TRUE);

	/* Setup the individual signal handlers */

	g_signal_connect(G_OBJECT(toggle_track), "toggled",
			 G_CALLBACK(playlist_track_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_title), "toggled",
			 G_CALLBACK(playlist_title_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_artist), "toggled",
			 G_CALLBACK(playlist_artist_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_album), "toggled",
			 G_CALLBACK(playlist_album_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_genre), "toggled",
			 G_CALLBACK(playlist_genre_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_bitrate), "toggled",
			 G_CALLBACK(playlist_bitrate_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_year), "toggled",
			 G_CALLBACK(playlist_year_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_comment), "toggled",
			 G_CALLBACK(playlist_comment_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_length), "toggled",
			 G_CALLBACK(playlist_length_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_filename), "toggled",
			 G_CALLBACK(playlist_filename_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(action_clear_sort), "activate",
			 G_CALLBACK(clear_sort_current_playlist_cb), cplaylist);

	gtk_widget_show_all(menu);

	return menu;
}

static GtkUIManager*
create_cp_context_menu(GtkWidget *current_playlist,
		       struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("CP Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       cp_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create current playlist context menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     cp_context_aentries,
				     G_N_ELEMENTS(cp_context_aentries),
				     (gpointer)cwin);
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager*
create_cp_null_context_menu(GtkWidget *current_playlist,
			    struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("CP Null Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       cp_null_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create current playlist null context menu, err : %s",
			   error->message);
	}
	gtk_action_group_add_actions(context_actions,
				     cp_null_context_aentries,
				     G_N_ELEMENTS(cp_null_context_aentries),
				     (gpointer)cwin);
	gtk_action_group_add_toggle_actions (context_actions,
					cp_null_toggles_entries,
					G_N_ELEMENTS(cp_null_toggles_entries),
					cwin);
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}
static const GtkTargetEntry pentries[] = {
	{"REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY},
	{"text/uri-list", GTK_TARGET_OTHER_APP, TARGET_URI_LIST},
	{"text/plain", GTK_TARGET_OTHER_APP, TARGET_PLAIN_TEXT}
};

static void init_playlist_dnd(PraghaPlaylist *cplaylist, struct con_win *cwin)
{
	/* Source/Dest: Current Playlist */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cplaylist->view),
					       GDK_BUTTON1_MASK,
					       pentries,
					       G_N_ELEMENTS(pentries),
					       GDK_ACTION_COPY | GDK_ACTION_MOVE);

	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(cplaylist->view),
					     pentries,
					     G_N_ELEMENTS(pentries),
					     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cplaylist->view)),
			 "drag-begin",
			 G_CALLBACK(dnd_current_playlist_begin),
			 cplaylist);
	 g_signal_connect (G_OBJECT(cplaylist->view),
			 "drag-data-get",
			 G_CALLBACK (drag_current_playlist_get_data),
			 cplaylist);
	g_signal_connect(G_OBJECT(cplaylist->view),
			 "drag-drop",
			 G_CALLBACK(dnd_current_playlist_drop),
			 cwin);
	g_signal_connect(G_OBJECT(cplaylist->view),
			 "drag-data-received",
			 G_CALLBACK(dnd_current_playlist_received),
			 cwin);
}

static void
create_current_playlist_columns(PraghaPlaylist *cplaylist, GtkTreeView *view)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *state_pixbuf,
		*label_track,
		*label_title,
		*label_artist,
		*label_album,
		*label_genre,
		*label_bitrate,
		*label_year,
		*label_comment,
		*label_length,
		*label_filename;
	GtkWidget *col_button;

	label_track = gtk_label_new(_("Track"));
	label_title = gtk_label_new(_("Title"));
	label_artist = gtk_label_new(_("Artist"));
	label_album = gtk_label_new(_("Album"));
	label_genre = gtk_label_new(_("Genre"));
	label_bitrate = gtk_label_new(_("Bitrate"));
	label_year = gtk_label_new(_("Year"));
	label_comment = gtk_label_new(_("Comment"));
	label_length = gtk_label_new(_("Length"));
	label_filename = gtk_label_new(_("Filename"));

	state_pixbuf = gtk_image_new_from_icon_name ("audio-volume-high", GTK_ICON_SIZE_MENU);

	/* Column : Queue Bubble and Status Pixbuf */

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_bubble_new ();
	gtk_cell_renderer_set_fixed_size (renderer, 12, -1);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
	gtk_tree_view_column_set_attributes(column, renderer, "markup", P_QUEUE, "show-bubble", P_BUBBLE, NULL);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", P_STATUS_PIXBUF, NULL);

	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_column_set_widget (column, state_pixbuf);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_widget_show (state_pixbuf);

	/* Column : Track No */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TRACK_NO_STR,
							  renderer,
							  "text",
							  P_TRACK_NO,
							  NULL);

	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TRACK_NO);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_track);
	gtk_widget_show(label_track);
	col_button = gtk_widget_get_ancestor(label_track, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Title */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TITLE_STR,
							  renderer,
							  "text",
							  P_TITLE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TITLE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_title);
	gtk_widget_show(label_title);
	col_button = gtk_widget_get_ancestor(label_title, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Artist */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_ARTIST_STR,
							  renderer,
							  "text",
							  P_ARTIST,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ARTIST);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_artist);
	gtk_widget_show(label_artist);
	col_button = gtk_widget_get_ancestor(label_artist, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Album */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_ALBUM_STR,
							  renderer,
							  "text",
							  P_ALBUM,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ALBUM);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_album);
	gtk_widget_show(label_album);
	col_button = gtk_widget_get_ancestor(label_album, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Genre */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_GENRE_STR,
							  renderer,
							  "text",
							  P_GENRE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_GENRE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_genre);
	gtk_widget_show(label_genre);
	col_button = gtk_widget_get_ancestor(label_genre, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Bitrate */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_BITRATE_STR,
							  renderer,
							  "text",
							  P_BITRATE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_BITRATE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_bitrate);
	gtk_widget_show(label_bitrate);
	col_button = gtk_widget_get_ancestor(label_bitrate, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Year */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_YEAR_STR,
							  renderer,
							  "text",
							  P_YEAR,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_YEAR);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_year);
	gtk_widget_show(label_year);
	col_button = gtk_widget_get_ancestor(label_year, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Comment */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_COMMENT_STR,
							  renderer,
							  "text",
							  P_COMMENT,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_COMMENT);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_comment);
	gtk_widget_show(label_comment);
	col_button = gtk_widget_get_ancestor(label_comment, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Length */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_LENGTH_STR,
							  renderer,
							  "text",
							  P_LENGTH,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_LENGTH);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_length);
	gtk_widget_show(label_length);
	col_button = gtk_widget_get_ancestor(label_length, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Filename */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_FILENAME_STR,
							  renderer,
							  "text",
							  P_FILENAME,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_FILENAME);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_filename);
	gtk_widget_show(label_filename);
	col_button = gtk_widget_get_ancestor(label_filename, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);
}

static void
update_current_playlist_view_playback_state_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;

	if (cwin->cplaylist->current_update_action == PLAYLIST_NONE)
		update_current_playlist_view_track(cwin);
}

GtkWidget *
create_current_playlist_container(struct con_win *cwin)
{
	GtkWidget *scroll_window;

	scroll_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);

	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_window),
					    GTK_SHADOW_IN);

	return scroll_window;
}

GtkWidget* create_current_playlist_view(PraghaPlaylist* cplaylist, struct con_win *cwin)
{
	GtkWidget *current_playlist;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;

	/* Create the tree store */

	store = gtk_list_store_new(N_P_COLUMNS,
				   G_TYPE_POINTER,	/* Pointer to musicobject */
				   G_TYPE_STRING,	/* Queue No String */
				   G_TYPE_BOOLEAN,	/* Show Bublle Queue */
				   GDK_TYPE_PIXBUF,	/* Playback status pixbuf */
				   G_TYPE_STRING,	/* Tag : Track No */
				   G_TYPE_STRING,	/* Tag : Title */
				   G_TYPE_STRING,	/* Tag : Artist */
				   G_TYPE_STRING,	/* Tag : Album */
				   G_TYPE_STRING,	/* Tag : Genre */
				   G_TYPE_STRING,	/* Tag : Bitrate */
				   G_TYPE_STRING,	/* Tag : Year */
				   G_TYPE_STRING,	/* Tag : Comment */
				   G_TYPE_STRING,	/* Tag : Length */
				   G_TYPE_STRING,	/* Filename */
				   G_TYPE_BOOLEAN);	/* Played flag */

	/* Create the tree view */

	current_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(current_playlist));
	sortable = GTK_TREE_SORTABLE(model);

	/* Disable interactive search */

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(current_playlist), FALSE);

	/* Set the sort functions */

	gtk_tree_sortable_set_sort_func(sortable,
					P_TRACK_NO,
					compare_track_no,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_BITRATE,
					compare_bitrate,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_YEAR,
					compare_year,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_LENGTH,
					compare_length,
					NULL,
					NULL);

	/* Set selection properties */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(current_playlist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create the columns and cell renderers */

	create_current_playlist_columns(cplaylist, GTK_TREE_VIEW(current_playlist));

	/* Signal handler for double-clicking on a row */

	g_signal_connect(G_OBJECT(current_playlist), "row-activated",
			 G_CALLBACK(current_playlist_row_activated_cb), cwin);

	g_signal_connect (G_OBJECT (current_playlist), "key-press-event",
			  G_CALLBACK (current_playlist_key_press), cwin);

	/* Store the treeview in the scrollbar widget */

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(current_playlist),
				      pragha_preferences_get_use_hint(cwin->preferences));

	/* Update menu depending playback state */

	g_signal_connect (cwin->backend, "notify::state", G_CALLBACK (update_current_playlist_view_playback_state_cb), cwin);

	g_object_unref(store);

	return current_playlist;
}

static void
playlist_column_set_visible(PraghaPlaylist* cplaylist, gint column, gboolean visible)
{
	const gchar *col_name;
	GtkTreeViewColumn *col;

	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view),
				       column - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, visible);
	modify_current_playlist_columns(cplaylist, col_name, visible);
}

/* Callback for adding/deleting track_no column */

void playlist_track_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_TRACK_NO, state);
}

/* Callback for adding/deleting title column */

void playlist_title_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_TITLE, state);
}

/* Callback for adding/deleting artist column */

void playlist_artist_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_ARTIST, state);
}

/* Callback for adding/deleting album column */

void playlist_album_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_ALBUM, state);
}

/* Callback for adding/deleting genre column */

void playlist_genre_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_GENRE, state);
}

/* Callback for adding/deleting bitrate column */

void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_BITRATE, state);
}

/* Callback for adding/deleting year column */

void playlist_year_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_YEAR, state);
}

/* Callback for adding/deleting comment column */

void playlist_comment_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_COMMENT, state);
}

/* Callback for adding/deleting length column */

void playlist_length_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_LENGTH, state);
}

/* Callback for adding/deleting filename column */

void playlist_filename_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_FILENAME, state);
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist_cb(GtkMenuItem *item, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cplaylist->view));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Comparison function for track numbers */

gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_track_no(mobj_a) < pragha_musicobject_get_track_no(mobj_b))
		return -1;
	else if (pragha_musicobject_get_track_no(mobj_a) > pragha_musicobject_get_track_no(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for bitrates */

gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_bitrate(mobj_a) < pragha_musicobject_get_bitrate(mobj_b))
		return -1;
	else if (pragha_musicobject_get_bitrate(mobj_a) > pragha_musicobject_get_bitrate(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for years */

gint compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_year(mobj_a) < pragha_musicobject_get_year(mobj_b))
		return -1;
	else if (pragha_musicobject_get_year(mobj_a) > pragha_musicobject_get_year(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for lengths */

gint compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_length(mobj_a) < pragha_musicobject_get_length(mobj_b))
		return -1;
	else if (pragha_musicobject_get_length(mobj_a) > pragha_musicobject_get_length(mobj_b))
		return 1;
	else
		return 0;
}

gboolean
pragha_playlist_propagate_event(PraghaPlaylist* cplaylist, GdkEventKey *event)
{
	GdkEvent *new_event;
	gboolean ret;

	gtk_widget_grab_focus(cplaylist->view);

	new_event = gdk_event_copy ((GdkEvent *) event);
	ret = gtk_widget_event (GTK_WIDGET (cplaylist->view), new_event);
	gdk_event_free (new_event);

	return ret;
}

void
pragha_playlist_activate_path(PraghaPlaylist* cplaylist, GtkTreePath *path)
{
	GtkTreeViewColumn *col;
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view), 1);
	gtk_tree_view_row_activated(GTK_TREE_VIEW(cplaylist->view),
				   path,
				   col);
}

void
pragha_playlist_activate_unique_mobj(PraghaPlaylist* cplaylist, PraghaMusicobject *mobj)
{
	GtkTreePath *path = NULL;

	path = current_playlist_path_at_mobj(mobj, cplaylist);

	if(path) {
		pragha_playlist_activate_path(cplaylist, path);
		gtk_tree_path_free (path);
	}
}

void
pragha_playlist_set_rules_hint(PraghaPlaylist* cplaylist, gboolean use_hint)
{
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(cplaylist->view), use_hint);
}

gint
pragha_playlist_get_no_tracks(PraghaPlaylist* cplaylist)
{
	return cplaylist->no_tracks;
}

gboolean
pragha_playlist_has_queue(PraghaPlaylist* cplaylist)
{
	if(cplaylist->queue_track_refs)
		return TRUE;
	else
		return FALSE;
}

gboolean
pragha_playlist_is_changing(PraghaPlaylist* cplaylist)
{
	return cplaylist->changing;
}

void
pragha_playlist_set_changing(PraghaPlaylist* cplaylist, gboolean changing)
{
	cplaylist->changing = changing;

	gtk_widget_set_sensitive(GTK_WIDGET(cplaylist->widget), !changing);
}

gboolean
pragha_playlist_is_shuffle(PraghaPlaylist* cplaylist)
{
	return cplaylist->shuffle;
}

void
pragha_playlist_set_shuffle(PraghaPlaylist* cplaylist, gboolean shuffle)
{
	GtkTreeRowReference *ref;

	cplaylist->shuffle = shuffle;

	if(!cplaylist->no_tracks)
		return;

	if (shuffle) {
		CDEBUG(DBG_INFO, "Turning shuffle on");
		if (cplaylist->curr_seq_ref) {
			ref = gtk_tree_row_reference_copy(cplaylist->curr_seq_ref);
			reset_rand_track_refs(cplaylist, ref);
		}
	}
	else {
		CDEBUG(DBG_INFO, "Turning shuffle off");
		current_playlist_clear_dirty_all(cplaylist);
		if (cplaylist->curr_rand_ref)
			cplaylist->curr_seq_ref =
				gtk_tree_row_reference_copy(cplaylist->curr_rand_ref);
		else
			cplaylist->curr_seq_ref = NULL;
	}
}

void
current_playlist_set_repeat(PraghaPlaylist* cplaylist, gboolean repeat)
{
	cplaylist->repeat = repeat;
}

GtkWidget *
pragha_playlist_get_view(PraghaPlaylist* cplaylist)
{
	return cplaylist->view;
}

GtkWidget *
pragha_playlist_get_widget(PraghaPlaylist* cplaylist)
{
	return cplaylist->widget;
}

void
current_playlist_save_preferences(PraghaPlaylist* cplaylist)
{
	GtkTreeViewColumn *col;
	const gchar *col_name;
	gchar **columns;
	gint cnt = 0, i = 0, *col_widths;
	GSList *list;
	GList *cols, *j;
	PraghaPreferences *preferences;

	preferences = pragha_preferences_get();

	/* Save list of columns visible in current playlist */

	if (cplaylist->columns) {
		list = cplaylist->columns;
		cnt = g_slist_length(cplaylist->columns);
		columns = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			columns[i] = list->data;
			list = list->next;
		}

		pragha_preferences_set_string_list(preferences,
						   GROUP_PLAYLIST,
						   KEY_PLAYLIST_COLUMNS,
						   (const gchar **)columns,
						   cnt);
		g_free(columns);
	}

	/* Save column widths */

	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(cplaylist->view));
	cnt = g_list_length(cols);
	if (cols) {
		col_widths = g_new0(gint, cnt);
		for (j=cols, i=0; j != NULL; j = j->next) {
			col = j->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cplaylist->columns))
				col_widths[i++] =
					gtk_tree_view_column_get_width(col);
		}
		pragha_preferences_set_integer_list(preferences,
						    GROUP_PLAYLIST,
						    KEY_PLAYLIST_COLUMN_WIDTHS,
						    col_widths,
						    i);
		g_list_free(cols);
		g_free(col_widths);
	}

	g_object_unref(G_OBJECT(preferences));
}

void
cplatlist_init_pixbuf(PraghaPlaylist* cplaylist)
{
	GtkIconTheme *icontheme = gtk_icon_theme_get_default();

	cplaylist->playing_pixbuf =
		gtk_icon_theme_load_icon (icontheme,
					  "media-playback-start",
					  16, 0, NULL);
	cplaylist->paused_pixbuf =
		gtk_icon_theme_load_icon (icontheme,
					  "media-playback-pause",
					  16, 0, NULL);

}

void
cplaylist_free(PraghaPlaylist* cplaylist)
{
	free_str_list(cplaylist->columns);
	g_slist_free(cplaylist->column_widths);
	g_rand_free(cplaylist->rand);

	g_object_unref(cplaylist->playing_pixbuf);
	g_object_unref(cplaylist->paused_pixbuf);

	g_slice_free(PraghaPlaylist, cplaylist);
}

PraghaPlaylist*
cplaylist_new(struct con_win *cwin)
{
	PraghaPlaylist *cplaylist;

	cplaylist = g_slice_new0(PraghaPlaylist);

	cplaylist->view = create_current_playlist_view(cplaylist, cwin);
	cplaylist->widget = create_current_playlist_container(cwin);

	init_current_playlist_columns(cplaylist, cwin);

	cplaylist->header_context_menu = create_header_context_menu(cplaylist);
	cplaylist->cp_context_menu = create_cp_context_menu(cplaylist->view, cwin);
	cplaylist->cp_null_context_menu = create_cp_null_context_menu(cplaylist->view, cwin);

	g_signal_connect(G_OBJECT(cplaylist->view), "button-press-event",
			 G_CALLBACK(current_playlist_button_press_cb), cplaylist);

	g_signal_connect(G_OBJECT(cplaylist->view), "button-release-event",
			 G_CALLBACK(current_playlist_button_release_cb), cplaylist);

	cplatlist_init_pixbuf(cplaylist);

	gtk_container_add (GTK_CONTAINER(cplaylist->widget), cplaylist->view);

	init_playlist_dnd(cplaylist, cwin);

	cplaylist->rand = g_rand_new();
	cplaylist->changing = TRUE;
	cplaylist->dragging = FALSE;
	cplaylist->rand_track_refs = NULL;
	cplaylist->queue_track_refs = NULL;
	cplaylist->current_update_action = PLAYLIST_NONE;

	return cplaylist;
}
