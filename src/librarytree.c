/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
/*									 */
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

/* Returns TRUE if any of the childs of p_iter matches node_data. iter
 * and p_iter must be created outside this function */

static gboolean find_child_node(const gchar *node_data, GtkTreeIter *iter,
	GtkTreeIter *p_iter, GtkTreeModel *model)
{
	gchar *data = NULL;
	gboolean valid;
	gint cmp;

	valid = gtk_tree_model_iter_children(model, iter, p_iter);

	while (valid) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &data, -1);
		if (data) {
			cmp = g_ascii_strcasecmp (data, node_data);
			if (cmp == 0) {
				g_free(data);
				return TRUE;
			}
			else if (cmp > 0) {
				g_free(data);
				return FALSE;
			}
		g_free(data);
		}
		valid = gtk_tree_model_iter_next(model, iter);
	}
	return FALSE;
}

/* Appends a child (iter) to p_iter with given data. NOTE that iter
 * and p_iter must be created outside this function */

static void add_child_node_by_tag(GtkTreeModel *model, GtkTreeIter *iter,
	GtkTreeIter *p_iter, GdkPixbuf *pixbuf, const gchar *node_data,
	int node_type, int location_id)
{
	gtk_tree_store_prepend(GTK_TREE_STORE(model), iter, p_iter);

	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
		L_PIXBUF, pixbuf,
		L_NODE_DATA, node_data,
		L_NODE_TYPE, node_type,
		L_LOCATION_ID, location_id,
		L_MACH, FALSE,
		L_VISIBILE, TRUE, -1);
}

static void
add_child_node_folder(GtkTreeModel *model,
		      GtkTreeIter *iter,
		      GtkTreeIter *p_iter,
		      const gchar *node_data,
		      struct con_win *cwin)
{
	GtkTreeIter l_iter;
	gchar *data = NULL;
	gint l_node_type;
	gboolean valid;

	/* Find position of the last directory that is a child of p_iter */
	valid = gtk_tree_model_iter_children(model, &l_iter, p_iter);
	while (valid) {
		gtk_tree_model_get(model, &l_iter, L_NODE_TYPE, &l_node_type, -1);
		if (l_node_type != NODE_FOLDER)
			break;
		gtk_tree_model_get(model, &l_iter, L_NODE_DATA, &data, -1);
		if (g_ascii_strcasecmp(data, node_data) >= 0) {
			g_free(data);
			break;
		}
		g_free(data);

		valid = gtk_tree_model_iter_next(model, &l_iter);
	}

	/* Insert the new folder after the last subdirectory by order */
	gtk_tree_store_insert_before(GTK_TREE_STORE(model), iter, p_iter, valid ? &l_iter : NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
			   L_PIXBUF, cwin->pixbuf->pixbuf_dir,
			   L_NODE_DATA, node_data,
			   L_NODE_TYPE, NODE_FOLDER,
			   L_LOCATION_ID, 0,
			   L_MACH, FALSE,
			   L_VISIBILE, TRUE,
			   -1);
}

/* Appends a child (iter) to p_iter with given data. NOTE that iter
 * and p_iter must be created outside this function */

static void
add_child_node_file(GtkTreeModel *model,
		    GtkTreeIter *iter,
		    GtkTreeIter *p_iter,
		    const gchar *node_data,
		    int location_id,
		    struct con_win *cwin)
{
	GtkTreeIter l_iter;
	gchar *data = NULL;
	gint l_node_type;
	gboolean valid;

	/* Find position of the last file that is a child of p_iter */
	valid = gtk_tree_model_iter_children(model, &l_iter, p_iter);
	while (valid) {
		gtk_tree_model_get(model, &l_iter, L_NODE_TYPE, &l_node_type, -1);
		gtk_tree_model_get(model, &l_iter, L_NODE_DATA, &data, -1);

		if ((l_node_type != NODE_FOLDER) || (g_ascii_strcasecmp(data, node_data) >= 0)) {
			g_free(data);
			break;
		}
		g_free(data);

		valid = gtk_tree_model_iter_next(model, &l_iter);
	}

	/* Insert the new file after the last file by order */
	gtk_tree_store_insert_before(GTK_TREE_STORE(model), iter, p_iter, valid ? &l_iter : NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
			   L_PIXBUF, cwin->pixbuf->pixbuf_track,
			   L_NODE_DATA, node_data,
			   L_NODE_TYPE, NODE_BASENAME,
			   L_LOCATION_ID, location_id,
			   L_MACH, FALSE,
			   L_VISIBILE, TRUE,
			   -1);
}

/* Adds a file and its parent directories to the library tree */

static void add_folder_file(gchar *path, int location_id,
	struct con_win *cwin, GtkTreeModel *model, GtkTreeIter *p_iter)
{
	gchar *prefix = NULL, *filepath = NULL;	/* Do not free */
	gchar **subpaths = NULL;		/* To be freed */

	GtkTreeIter iter, iter2, search_iter;
	int i = 0 , len = 0;

	/* Search all library directories for the one that matches the path */
	while ((prefix = g_slist_nth_data(cwin->cpref->library_dir, i++))) {
		if (g_str_has_prefix(path, prefix)) {
			break;
		}
	}

	g_assert(prefix != NULL);

	/* Point after library directory prefix */

	filepath = path + strlen(prefix) + 1;
	subpaths = g_strsplit(filepath, G_DIR_SEPARATOR_S, -1);

	len = g_strv_length (subpaths);
	len--;

	/* If not fuse_folders add the prefix */
	if (!cwin->cpref->fuse_folders) {
		if (!find_child_node(prefix, &search_iter, p_iter, model)) {
			add_child_node_folder(model, &iter, p_iter, prefix, cwin);
			p_iter = &iter;
		}
		else {
			iter2 = search_iter;
			p_iter = &iter2;
		}
	}

	/* Add all subdirectories and filename to the tree */
	for (i = 0; subpaths[i]; i++) {
		if (!find_child_node(subpaths[i], &search_iter, p_iter, model)) {
			if(i < len)
				add_child_node_folder(model, &iter, p_iter, subpaths[i], cwin);
			else
				add_child_node_file(model, &iter, p_iter, subpaths[i], location_id, cwin);
			p_iter = &iter;
		}
		else {
			iter2 = search_iter;
			p_iter = &iter2;
		}
	}

	g_strfreev(subpaths);
}

/* Adds an entry to the library tree by tag (genre, artist...) */

static void
add_child_node_by_tags (GtkTreeModel *model,
                       GtkTreeIter *p_iter,
                       gint location_id,
                       const gchar *location,
                       const gchar *genre,
                       const gchar *album,
                       const gchar *year,
                       const gchar *artist,
                       const gchar *track,
                       struct con_win *cwin)
{
	GtkTreeIter iter, iter2, search_iter;
	gchar *node_data = NULL;
	GdkPixbuf *node_pixbuf = NULL;
	enum node_type node_type = 0;
	gint node_level = 0, tot_levels = 0;
	gboolean need_gfree = FALSE;

	/* Iterate through library tree node types */ 
	tot_levels = g_slist_length(cwin->cpref->library_tree_nodes);
	while (node_level < tot_levels) {
		/* Set data to be added to the tree node depending on the type of node */
		node_type = GPOINTER_TO_INT(g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level));
		switch (node_type) {
			case NODE_TRACK:
				node_pixbuf = cwin->pixbuf->pixbuf_track;
				node_data = track ? (gchar *)track : get_display_filename(location, FALSE);
				if (!track) need_gfree = TRUE;
				break;
			case NODE_ARTIST:
				node_pixbuf = cwin->pixbuf->pixbuf_artist;
				node_data = artist ? (gchar *)artist : _("Unknown Artist");
				break;
			case NODE_ALBUM:
				node_pixbuf = cwin->pixbuf->pixbuf_album;
				if (cwin->cpref->sort_by_year) {
					node_data = g_strconcat ((year && (atoi(year) > 0)) ? year : _("Unknown"), " - ", album ? album : _("Unknown Album"), NULL);
					need_gfree = TRUE;
				}
				else {
					node_data = album ? (gchar *)album : _("Unknown Album");
				}
				break;
			case NODE_GENRE:
				node_pixbuf = cwin->pixbuf->pixbuf_genre;
				node_data = genre ? (gchar *)genre : _("Unknown Genre");
				break;
			case NODE_FOLDER:
			case NODE_PLAYLIST:
			case NODE_RADIO:
			case NODE_BASENAME:
				g_warning("add_by_tag: Bad node type.");
				break;
		}

		/* Find / add child node if it's not already added */
		if (node_type != NODE_TRACK) {
			if (!find_child_node(node_data, &search_iter, p_iter, model)) {
				add_child_node_by_tag(model, &iter, p_iter, node_pixbuf,
					node_data, node_type, 0);
				p_iter = &iter;
			}
			else {
				iter2 = search_iter;
				p_iter = &iter2;
			}
		}
		else {
			add_child_node_by_tag(model, &iter, p_iter, node_pixbuf,
						node_data, NODE_TRACK, location_id);
		}

		/* Free node_data if needed */
		if (need_gfree) {
			need_gfree = FALSE;
			g_free(node_data);
		}
		node_level++;
	}
}

/* Append to the given array the path of
   all the nodes under the given path */

static void get_path_array(GtkTreePath *path,
				  GArray *ref_arr,
				  GtkTreeModel *model,
				  struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	GtkTreePath *cpath;
	gboolean valid;

	cwin->cstate->view_change = TRUE;

	gtk_tree_model_get_iter(model, &r_iter, path);

	/* If this path is a track/radio/playlist, just append it to the array */

	gtk_tree_model_get(model, &r_iter, L_NODE_TYPE, &node_type, -1);

	if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME) ||
	    (node_type == NODE_PLAYLIST) || (node_type == NODE_RADIO)) {
		cpath = gtk_tree_path_copy(path);
		ref_arr = g_array_prepend_val(ref_arr, cpath);
	}

	/* For all other node types do a recursive add */
	valid = gtk_tree_model_iter_children(model, &t_iter, &r_iter);
	while (valid) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);

		path = gtk_tree_model_get_path(model, &t_iter);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME) ||
		    (node_type == NODE_PLAYLIST) || (node_type == NODE_RADIO)) {
			cpath = gtk_tree_path_copy(path);
			ref_arr = g_array_prepend_val(ref_arr, cpath);
		}
		else {
			get_path_array(path, ref_arr, model, cwin);
		}
		gtk_tree_path_free(path);

		valid = gtk_tree_model_iter_next(model, &t_iter);
	}

	cwin->cstate->view_change = FALSE;
}

/* Append to the given array the location ids of
   all the nodes under the given path */

static void get_location_ids(GtkTreePath *path,
			     GArray *loc_arr,
			     GtkTreeModel *model,
			     struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gint location_id;
	gint j = 0;

	cwin->cstate->view_change = TRUE;

	gtk_tree_model_get_iter(model, &r_iter, path);

	/* If this path is a track, just append it to the array */

	gtk_tree_model_get(model, &r_iter, L_NODE_TYPE, &node_type, -1);
	if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
		gtk_tree_model_get(model, &r_iter, L_LOCATION_ID, &location_id, -1);
		g_array_append_val(loc_arr, location_id);
	}

	/* For all other node types do a recursive add */

	while (gtk_tree_model_iter_nth_child(model, &t_iter, &r_iter, j++)) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			g_array_append_val(loc_arr, location_id);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			get_location_ids(path, loc_arr, model, cwin);
			gtk_tree_path_free(path);
		}
	}

	cwin->cstate->view_change = FALSE;
}

/* Add all the tracks under the given path to the current playlist */

GList *
append_library_row_to_mobj_list(GtkTreePath *path,
				GtkTreeModel *row_model,
				GList *list,
				struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gint location_id;
	PraghaMusicobject *mobj = NULL;
	gchar *data = NULL;
	gint j = 0;

	/* If this path is a track, just append it to the current playlist */

	gtk_tree_model_get_iter(row_model, &r_iter, path);

	gtk_tree_model_get(row_model, &r_iter, L_NODE_TYPE, &node_type, -1);
	gtk_tree_model_get(row_model, &r_iter, L_LOCATION_ID, &location_id, -1);
	gtk_tree_model_get(row_model, &r_iter, L_NODE_DATA, &data, -1);

	switch (node_type) {
		case NODE_GENRE:
		case NODE_ARTIST:
		case NODE_ALBUM:
		case NODE_FOLDER:
			/* For all other node types do a recursive add */
			while (gtk_tree_model_iter_nth_child(row_model, &t_iter, &r_iter, j++)) {
				path = gtk_tree_model_get_path(row_model, &t_iter);
				list = append_library_row_to_mobj_list(path, row_model, list, cwin);
				gtk_tree_path_free(path);
			}
			break;
		case NODE_TRACK:
		case NODE_BASENAME:
			mobj = new_musicobject_from_db(location_id, cwin);
			if(G_LIKELY(mobj))
				list = g_list_prepend(list, mobj);
			break;
		case NODE_PLAYLIST:
			list = add_playlist_to_mobj_list(data, list, FALSE, cwin);
			break;
		case NODE_RADIO:
			list = add_radio_to_mobj_list(data, list, FALSE, cwin);
			break;
		default:
			break;
	}

	g_free(data);

	return list;
}

static void delete_row_from_db(GtkTreePath *path, GtkTreeModel *model,
			       struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gboolean valid;
	gint location_id;

	/* If this path is a track, delete it immediately */

	gtk_tree_model_get_iter(model, &r_iter, path);
	gtk_tree_model_get(model, &r_iter, L_NODE_TYPE, &node_type, -1);
	if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
		gtk_tree_model_get(model, &r_iter, L_LOCATION_ID, &location_id, -1);
		delete_location_db(location_id, cwin->cdbase);
	}

	/* For all other node types do a recursive deletion */

	valid = gtk_tree_model_iter_children(model, &t_iter, &r_iter);
	while (valid) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			delete_location_db(location_id, cwin->cdbase);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			delete_row_from_db(path, model, cwin);
			gtk_tree_path_free(path);
		}

		valid = gtk_tree_model_iter_next(model, &t_iter);
	}
}

static void trash_or_unlink_row(GArray *loc_arr, gboolean unlink,
				struct con_win *cwin)
{
	GtkWidget *question_dialog;
	gchar *query = NULL, *filename = NULL;
	gchar *primary, *secondary;
	gint response, location_id = 0;
	guint i;
	gboolean deleted = FALSE;
	struct db_result result;

	if (!loc_arr)
		return;

	for(i = 0; i < loc_arr->len; i++) {
		location_id = g_array_index(loc_arr, gint, i);
		if (location_id) {
			query = g_strdup_printf("SELECT name FROM LOCATION "
						"WHERE id = '%d';",
						location_id);
			if (exec_sqlite_query(query, cwin->cdbase, &result)) {
				filename = result.resultp[result.no_columns];
				if(filename && g_file_test(filename, G_FILE_TEST_EXISTS)) {
					GError *error = NULL;
					GFile *file = g_file_new_for_path(filename);

					if(!unlink && !(deleted = g_file_trash(file, NULL, &error))) {
						primary = g_strdup (_("Cannot move file to trash, do you want to delete immediately?"));
						secondary = g_strdup_printf (_("The file \"%s\" cannot be moved to the trash. Details: %s"),
										g_file_get_basename (file), error->message);

						question_dialog = gtk_message_dialog_new (GTK_WINDOW (cwin->mainwindow),
					                                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					                                                GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", primary);
						gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (question_dialog), "%s", secondary);

						gtk_dialog_add_button (GTK_DIALOG (question_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
						if (loc_arr->len > 1) {
						        gtk_dialog_add_button (GTK_DIALOG (question_dialog), PRAGHA_BUTTON_SKIP, PRAGHA_RESPONSE_SKIP);
						        gtk_dialog_add_button (GTK_DIALOG (question_dialog), PRAGHA_BUTTON_SKIP_ALL, PRAGHA_RESPONSE_SKIP_ALL);
				        		gtk_dialog_add_button (GTK_DIALOG (question_dialog), PRAGHA_BUTTON_DELETE_ALL, PRAGHA_RESPONSE_DELETE_ALL);
						}
						gtk_dialog_add_button (GTK_DIALOG (question_dialog), GTK_STOCK_DELETE, GTK_RESPONSE_ACCEPT);

						response = gtk_dialog_run (GTK_DIALOG (question_dialog));
						gtk_widget_destroy (question_dialog);
						g_free (primary);
						g_free (secondary);
						g_error_free (error);
						error = NULL;

						switch (response)
						{
							case PRAGHA_RESPONSE_DELETE_ALL:
								unlink = TRUE;
								break;
							case GTK_RESPONSE_ACCEPT:
								g_unlink(filename);
								deleted = TRUE;
								break;
							case PRAGHA_RESPONSE_SKIP:
								break;
							case PRAGHA_RESPONSE_SKIP_ALL:
							case GTK_RESPONSE_CANCEL:
							case GTK_RESPONSE_DELETE_EVENT:
							default:
								sqlite3_free_table(result.resultp);
								return;
						}
					}
					if(unlink) {
						g_unlink(filename);
						deleted = TRUE;
					}
					g_object_unref(G_OBJECT(file));
				}
			}
			if (deleted) {
				delete_location_db(location_id, cwin->cdbase);
			}
			sqlite3_free_table(result.resultp);
		}
	}
}

/******************/
/* Event handlers */
/******************/

void library_tree_row_activated_cb(GtkTreeView *library_tree,
				   GtkTreePath *path,
				   GtkTreeViewColumn *column,
				   struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *filter_model;
	enum node_type node_type;
	gint prev_tracks = 0;
	GList *list = NULL;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	gtk_tree_model_get_iter(filter_model, &iter, path);
	gtk_tree_model_get(filter_model, &iter, L_NODE_TYPE, &node_type, -1);

	switch(node_type) {
	case NODE_ARTIST:
	case NODE_ALBUM:
	case NODE_GENRE:
	case NODE_FOLDER:
		if (!gtk_tree_view_row_expanded(GTK_TREE_VIEW(cwin->library_tree),
						path))
			gtk_tree_view_expand_row(GTK_TREE_VIEW(cwin->library_tree),
						 path,
						 TRUE);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(cwin->library_tree),
						   path);
		break;
	case NODE_TRACK:
	case NODE_BASENAME:
	case NODE_PLAYLIST:
	case NODE_RADIO:
		set_watch_cursor (cwin->mainwindow);
		prev_tracks = pragha_playlist_get_no_tracks(cwin->cplaylist);

		list = append_library_row_to_mobj_list(path, filter_model, list, cwin);
		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 list);
		g_list_free(list);

		remove_watch_cursor (cwin->mainwindow);
		select_numered_path_of_current_playlist(cwin->cplaylist, prev_tracks, TRUE);
		update_status_bar_playtime(cwin);
		break;
	default:
		break;
	}
}

gboolean library_tree_button_press_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin)
{
	GtkWidget *popup_menu, *item_widget;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gboolean many_selected = FALSE;
	enum node_type node_type;
	gint n_select = 0, prev_tracks = 0;
	GList *list = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));

	if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL)){
		switch(event->button) {
		case 1:
			if (gtk_tree_selection_path_is_selected(selection, path)
			    && !(event->state & GDK_CONTROL_MASK)
			    && !(event->state & GDK_SHIFT_MASK)) {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_false, cwin, NULL);
			}
			else {
				gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cwin, NULL);
			}
			break;
		case 2:
			if (!gtk_tree_selection_path_is_selected(selection, path)){
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}

			list = append_library_row_to_mobj_list (path, model, list, cwin);

			if(list) {
				prev_tracks = pragha_playlist_get_no_tracks(cwin->cplaylist);

				pragha_playlist_append_mobj_list(cwin->cplaylist, list);

				select_numered_path_of_current_playlist(cwin->cplaylist, prev_tracks, TRUE);
				update_status_bar_playtime(cwin);
			}
			break;
		case 3:
			if (!(gtk_tree_selection_path_is_selected(selection, path))){
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}

			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);

			n_select = gtk_tree_selection_count_selected_rows(selection);

			if (node_type == NODE_PLAYLIST || node_type == NODE_RADIO) {
				popup_menu = gtk_ui_manager_get_widget(cwin->playlist_tree_context_menu,
									"/popup");

				item_widget = gtk_ui_manager_get_widget(cwin->playlist_tree_context_menu,
									"/popup/Rename");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget),
							  n_select == 1 && gtk_tree_path_get_depth(path) > 1);

				item_widget = gtk_ui_manager_get_widget(cwin->playlist_tree_context_menu,
									"/popup/Delete");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget),
							  gtk_tree_path_get_depth(path) > 1);

				item_widget = gtk_ui_manager_get_widget(cwin->playlist_tree_context_menu,
									"/popup/Export");
				gtk_widget_set_sensitive (GTK_WIDGET(item_widget),
							  n_select == 1 &&
							  gtk_tree_path_get_depth(path) > 1 &&
							  node_type == NODE_PLAYLIST);
			}
			else {
				if (gtk_tree_path_get_depth(path) > 1)
					popup_menu = gtk_ui_manager_get_widget(cwin->library_tree_context_menu,
									       "/popup");
				else
					popup_menu = gtk_ui_manager_get_widget(cwin->header_library_tree_context_menu,
									       "/popup");
			}

			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
	
			/* If more than one track is selected, don't propagate event */
	
			if (n_select > 1)
				many_selected = TRUE;
			else
				many_selected = FALSE;
			break;
		default:
			many_selected = FALSE;
			break;
		}
	gtk_tree_path_free(path);
	}
	else gtk_tree_selection_unselect_all(selection);

	return many_selected;
}

gboolean library_tree_button_release_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));

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

gboolean library_page_right_click_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin)
{
	static GtkWidget *popup_menu = NULL;
	gboolean ret = FALSE;

	if(!popup_menu){
		popup_menu = gtk_ui_manager_get_widget(cwin->library_page_context_menu,
						       "/popup");
		gtk_menu_attach_to_widget(GTK_MENU(popup_menu), widget, NULL);
	}

	if (!cwin->cstate->view_change) {
		switch(event->button) {
		case 3: {
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
			ret = TRUE;
			break;
		}
		case 1: {
			if (widget == cwin->combo_order){
				gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL,
						(GtkMenuPositionFunc) menu_position, widget, 
						0, gtk_get_current_event_time());
				ret = TRUE;
			}
			break;
		}
		default:
			ret = FALSE;
			break;
		}
	}
	return ret;
}

/*******/
/* DnD */
/*******/

gboolean dnd_library_tree_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin)
{
	cwin->cstate->dragging = TRUE;
	return FALSE;
}

/* Callback for DnD signal 'drag-data-get' */

void dnd_library_tree_get(GtkWidget *widget,
			  GdkDragContext *context,
			  GtkSelectionData *data,
			  enum dnd_target info,
			  guint time,
			  struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *list = NULL, *l;
	GArray *ref_arr;

	switch(info) {
	case TARGET_REF_LIBRARY:
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							cwin->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		if (!list) {
			gtk_selection_data_set(data, gtk_selection_data_get_data_type(data), 8, NULL, 0);
			break;
		}

		ref_arr = g_array_new(TRUE, TRUE, sizeof(GtkTreePath *));

		l = list;
		while(l) {
			get_path_array(l->data, ref_arr, model, cwin);
			gtk_tree_path_free(l->data);
			l = l->next;
		}
		g_list_free(list);

		gtk_selection_data_set(data,
				       gtk_selection_data_get_data_type(data),
				       8,
				       (guchar *)&ref_arr,
				       sizeof(GArray *));
 		break;
	default:
		g_warning("Unknown DND type");
	}
}

/**********/
/* Search */
/**********/

static gboolean set_all_visible(GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data)
{
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
			   L_MACH, FALSE,
			   L_VISIBILE, TRUE,
			   -1);
	return FALSE;
}

static void filter_tree_expand_func(GtkTreeView *view,
				    GtkTreePath *path,
				    gpointer data)
{
	GtkTreeModel *filter_model;
	GtkTreeIter iter;
	enum node_type node_type;
	gboolean node_mach;

	filter_model = data;

	gtk_tree_model_get_iter(filter_model, &iter, path);
	gtk_tree_model_get(filter_model, &iter, L_NODE_TYPE, &node_type, -1);
	gtk_tree_model_get(filter_model, &iter, L_MACH, &node_mach, -1);

	/* Collapse any non-leaf node that matches the seach entry */

	if (node_mach &&
	    (node_type != NODE_TRACK) &&
	    (node_type != NODE_BASENAME))
		gtk_tree_view_collapse_row(view, path);
}

static void
set_visible_parents_nodes(GtkTreeModel *model, GtkTreeIter *c_iter)
{
	GtkTreeIter t_iter, parent;

	t_iter = *c_iter;

	while(gtk_tree_model_iter_parent(model, &parent, &t_iter)) {
		gtk_tree_store_set(GTK_TREE_STORE(model), &parent,
				   L_VISIBILE, TRUE,
				   -1);
		t_iter = parent;
	}
}

static gboolean
any_parent_node_mach(GtkTreeModel *model, GtkTreeIter *iter)
{
	GtkTreeIter t_iter, parent;
	gboolean p_mach = FALSE;

	t_iter = *iter;
	while(gtk_tree_model_iter_parent(model, &parent, &t_iter)) {
		gtk_tree_model_get(model, &parent,
				   L_MACH, &p_mach,
				   -1);
		if (p_mach)
			return TRUE;

		t_iter = parent;
	}

	return FALSE;
}

static gboolean filter_tree_func(GtkTreeModel *model,
				 GtkTreePath *path,
				 GtkTreeIter *iter,
				 gpointer data)
{
	struct con_win *cwin = data;
	gchar *node_data = NULL, *u_str;
	gboolean p_mach;

	/* Mark node and its parents visible if search entry matches.
	   If search entry doesn't match, check if _any_ ancestor has
	   been marked as visible and if so, mark current node as visible too. */

	if (cwin->cstate->filter_entry) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &node_data, -1);
		u_str = g_utf8_strdown(node_data, -1);
		if (pragha_strstr_lv(u_str, cwin->cstate->filter_entry, cwin)) {
			/* Set visible the match row */
			gtk_tree_store_set(GTK_TREE_STORE(model), iter,
					   L_MACH, TRUE,
					   L_VISIBILE, TRUE,
					   -1);

			/* Also set visible the parents */
			set_visible_parents_nodes(model, iter);
		}
		else {
			/* Check parents. If any node is visible due it mach,
			 * also shows. So, show the children of coincidences. */
			p_mach = any_parent_node_mach(model, iter);
			gtk_tree_store_set(GTK_TREE_STORE(model), iter,
					   L_MACH, FALSE,
					   L_VISIBILE, p_mach,
					   -1);
		}
		g_free(u_str);
		g_free(node_data);
	}
	else
		return TRUE;

	return FALSE;
}

gboolean do_refilter(struct con_win *cwin )
{
	GtkTreeModel *filter_model;

	/* Remove the model of widget. */
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);

	/* Set visibility of rows in the library store. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(cwin->library_store),
				filter_tree_func,
				cwin);

	/* Set the model again.*/
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand all and then reduce properly. */
	gtk_tree_view_expand_all(GTK_TREE_VIEW(cwin->library_tree));
	gtk_tree_view_map_expanded_rows(GTK_TREE_VIEW(cwin->library_tree),
		filter_tree_expand_func,
		filter_model);

	cwin->cstate->timeout_id = 0;

	return FALSE;
}

void queue_refilter (struct con_win *cwin)
{
	if(cwin->cstate->timeout_id)
		g_source_remove(cwin->cstate->timeout_id);

	cwin->cstate->timeout_id = g_timeout_add(500, (GSourceFunc)do_refilter, cwin);
}

gboolean simple_library_search_keyrelease_handler(GtkEntry *entry,
						  struct con_win *cwin)
{
	gchar *text = NULL;
	gboolean has_text;
	
	if (!pragha_preferences_get_instant_search(cwin->preferences))
		return FALSE;

	if (cwin->cstate->filter_entry != NULL) {
		g_free (cwin->cstate->filter_entry);
		cwin->cstate->filter_entry = NULL;
	}

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		cwin->cstate->filter_entry = g_utf8_strdown (text, -1);

		queue_refilter(cwin);
	}
	else {
		clear_library_search (cwin);
	}

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	g_free (text);

	return FALSE;
}

gboolean simple_library_search_activate_handler(GtkEntry *entry,
						struct con_win *cwin)
{
	gchar *text = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (cwin->cstate->filter_entry != NULL) {
		g_free (cwin->cstate->filter_entry);
		cwin->cstate->filter_entry = NULL;
	}

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		cwin->cstate->filter_entry = g_utf8_strdown (text, -1);

		do_refilter (cwin);
	}
	else {
		clear_library_search (cwin);
	}
	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	g_free (text);

	return FALSE;
}

void clear_library_search(struct con_win *cwin)
{
	GtkTreeModel *filter_model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean valid;

	/* Remove the model of widget. */
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);

	/* Set all nodes visibles. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(cwin->library_store),
			       set_all_visible,
			       cwin);

	/* Set the model again. */
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand the categories. */

	valid = gtk_tree_model_get_iter_first (filter_model, &iter);
	while (valid) {
		path = gtk_tree_model_get_path(filter_model, &iter);
		gtk_tree_view_expand_row (GTK_TREE_VIEW(cwin->library_tree), path, FALSE);
		gtk_tree_path_free(path);

		valid = gtk_tree_model_iter_next(filter_model, &iter);
	}
}

/********************************/
/* Library view order selection */
/********************************/

void folders_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_FOLDER));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_BASENAME));

	cwin->cpref->cur_library_view = FOLDERS;

	init_library_view(cwin);
}

void artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ARTIST));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = ARTIST;

	init_library_view(cwin);
}

void album_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ALBUM));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = ALBUM;

	init_library_view(cwin);
}

void genre_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_GENRE));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = GENRE;

	init_library_view(cwin);
}

void artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ARTIST));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ALBUM));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = ARTIST_ALBUM;

	init_library_view(cwin);
}

void genre_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_GENRE));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ALBUM));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = GENRE_ALBUM;

	init_library_view(cwin);
}

void genre_artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_GENRE));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ARTIST));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = GENRE_ARTIST;

	init_library_view(cwin);
}

void genre_artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	g_slist_free (cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_GENRE));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ARTIST));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_ALBUM));
	cwin->cpref->library_tree_nodes =
		g_slist_append(cwin->cpref->library_tree_nodes,
			       GINT_TO_POINTER(NODE_TRACK));

	cwin->cpref->cur_library_view = GENRE_ARTIST_ALBUM;

	init_library_view(cwin);
}

/*****************/
/* Menu handlers */
/*****************/

static void
library_tree_replace_playlist (struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *mlist = NULL, *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		set_watch_cursor (cwin->mainwindow);

		pragha_playlist_remove_all (cwin->cplaylist);

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			mlist = append_library_row_to_mobj_list (path, model, mlist, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}

		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 mlist);
		remove_watch_cursor (cwin->mainwindow);

		if(!pragha_playlist_is_shuffle(cwin->cplaylist))
			select_numered_path_of_current_playlist(cwin->cplaylist, 0, FALSE);
		update_status_bar_playtime(cwin);
		
		g_list_free(list);
	}
}

void library_tree_replace_playlist_action(GtkAction *action, struct con_win *cwin)
{
	library_tree_replace_playlist (cwin);
}

void library_tree_replace_and_play(GtkAction *action, struct con_win *cwin)
{
	library_tree_replace_playlist (cwin);

	if (pragha_backend_get_state (cwin->backend) != ST_STOPPED)
		pragha_playback_next_track(cwin);
	else
		pragha_playback_play_pause_resume(cwin);
}

void library_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *mlist = NULL, *list, *i;
	gint prev_tracks = 0;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		set_watch_cursor (cwin->mainwindow);
		prev_tracks = pragha_playlist_get_no_tracks(cwin->cplaylist);

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			mlist = append_library_row_to_mobj_list(path, model, mlist, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}
		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 mlist);

		select_numered_path_of_current_playlist(cwin->cplaylist, prev_tracks, TRUE);
		remove_watch_cursor (cwin->mainwindow);
		update_status_bar_playtime(cwin);

		g_list_free(list);
	}
}

void library_tree_delete_db(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;
	gint result;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					_("Are you sure you want to delete current file from library?\n\n"
					"Warning: To recover we must rescan the entire library."));

		result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if( result == GTK_RESPONSE_YES ){
			/* Delete all the rows */

			db_begin_transaction(cwin->cdbase);

			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				delete_row_from_db(path, model, cwin);

				/* Have to give control to GTK periodically ... */
				if (pragha_process_gtk_events ())
					return;
			}

			db_commit_transaction(cwin->cdbase);

			flush_stale_entries_db(cwin->cdbase);
			init_library_view(cwin);
		}

		g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
	}
}

void library_tree_delete_hdd(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *toggle_unlink;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;
	gint result;
	GArray *loc_arr;
	gboolean unlink = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						_("Really want to move the files to trash?"));

		toggle_unlink = gtk_check_button_new_with_label(_("Delete immediately instead of move to trash"));
		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), toggle_unlink, TRUE, TRUE, 0);

		gtk_widget_show_all(dialog);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		unlink = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_unlink));
		gtk_widget_destroy(dialog);

		if(result == GTK_RESPONSE_YES){
			loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));

			db_begin_transaction(cwin->cdbase);
			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				get_location_ids(path, loc_arr, model, cwin);
				trash_or_unlink_row(loc_arr, unlink, cwin);

				/* Have to give control to GTK periodically ... */
				if (pragha_process_gtk_events ())
					return;
			}
			db_commit_transaction(cwin->cdbase);

			g_array_free(loc_arr, TRUE);

			flush_stale_entries_db(cwin->cdbase);
			init_library_view(cwin);
		}

		g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
	}
}

/***************/
/* Tag Editing */
/***************/

void library_tree_edit_tags(GtkAction *action, struct con_win *cwin)
{
	enum node_type node_type = 0;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GArray *loc_arr = NULL;
	GPtrArray *file_arr = NULL;
	gint sel, location_id, changed = 0;
	gchar *node_data = NULL, **split_album = NULL, *file = NULL;
	gint elem = 0, ielem;
	gchar *query = NULL;
	struct db_result result;

	PraghaMusicobject *omobj, *nmobj;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	sel = gtk_tree_selection_count_selected_rows(selection);
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	/* Setup initial entries */

	if (sel == 1) {
		path = list->data;

		if (!gtk_tree_model_get_iter(model, &iter, path))
			goto exit;

		gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);

		if (node_type == NODE_TRACK || node_type == NODE_BASENAME) {
			gtk_tree_model_get(model, &iter,
					   L_LOCATION_ID, &location_id, -1);

			omobj = new_musicobject_from_db(location_id, cwin);
		}
		else {
			omobj = pragha_musicobject_new();
			gtk_tree_model_get(model, &iter, L_NODE_DATA, &node_data, -1);

			switch(node_type) {
			case NODE_ARTIST:
				pragha_musicobject_set_artist(omobj, node_data);
				break;
			case NODE_ALBUM:
				if (cwin->cpref->sort_by_year) {
					split_album = g_strsplit(node_data, " - ", 2);
					pragha_musicobject_set_year(omobj, atoi (split_album[0]));
					pragha_musicobject_set_album(omobj, split_album[1]);

				}
				else {
					pragha_musicobject_set_album(omobj, node_data);

				}
				break;
			case NODE_GENRE:
				pragha_musicobject_set_genre(omobj, node_data);
				break;
			default:
				break;
			}
		}
	}

	nmobj = pragha_musicobject_new();
	changed = tag_edit_dialog(omobj, 0, nmobj, cwin);

	if (!changed)
		goto exit;

	/* Store the new tags */

	loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));

	for (i=list; i != NULL; i = i->next) {
		path = i->data;
		/* Form an array of location ids */
		get_location_ids(path, loc_arr, model, cwin);
	}

	/* Check if user is trying to set the same track no for multiple tracks */
	if (changed & TAG_TNO_CHANGED) {
		if (loc_arr->len > 1) {
			if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(nmobj), cwin))
				goto exit;
		}
	}

	/* Check if user is trying to set the same title/track no for
	   multiple tracks */
	if (changed & TAG_TITLE_CHANGED) {
		if (loc_arr->len > 1) {
			if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(nmobj), cwin))
				goto exit;
		}
	}

	/* Updata the db changes */
	pragha_db_update_local_files_change_tag(cwin->cdbase, loc_arr, changed, nmobj);
	init_library_view(cwin);

	/* Get a array of files and update it */
	file_arr = g_ptr_array_new();
	for(ielem = 0; ielem < loc_arr->len; ielem++) {
		elem = g_array_index(loc_arr, gint, ielem);
		if (elem) {
			query = g_strdup_printf("SELECT name FROM LOCATION "
						"WHERE id = '%d';",
						elem);

			if (exec_sqlite_query(query, cwin->cdbase, &result)) {
				file = result.resultp[result.no_columns];
				g_ptr_array_add(file_arr, file);
			}
		}
	}
	pragha_update_local_files_change_tag(file_arr, changed, nmobj);
	g_ptr_array_free(file_arr, TRUE);
	g_array_free(loc_arr, TRUE);

exit:
	/* Cleanup */

	g_free(node_data);
	g_strfreev (split_album);

	g_object_unref(nmobj);
	g_object_unref(omobj);
		
	g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
}

static void add_entry_playlist(gchar *playlist,
			       int node_type,
			       GtkTreeIter *root,
			       GtkTreeModel *model,
			       struct con_win *cwin)
{
	GtkTreeIter iter;

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      root);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_NODE_TYPE, node_type,
			   L_PIXBUF, cwin->pixbuf->pixbuf_track,
			   L_NODE_DATA, playlist,
			   -1);
}

/********/
/* Init */
/********/

void init_library_view(struct con_win *cwin)
{
	gint i = 0;
	gchar *query;
	struct db_result result;
	GtkTreeModel *model, *filter_model;
	GtkTreeIter iter;
	gchar *order_str = NULL;

	cwin->cstate->view_change = TRUE;

	set_watch_cursor (cwin->mainwindow);

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	g_object_ref(filter_model);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->search_entry), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->library_tree), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);

	gtk_tree_store_clear(GTK_TREE_STORE(model));

	/* Playlists.*/

	query = g_strdup_printf("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\" ORDER BY NAME;",
				SAVE_PLAYLIST_STATE);
	exec_sqlite_query(query, cwin->cdbase, &result);

	if (result.no_rows) {
		gtk_tree_store_append(GTK_TREE_STORE(model),
				      &iter,
				      NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				   L_PIXBUF, cwin->pixbuf->pixbuf_dir,
				   L_NODE_DATA, _("Playlists"),
				   L_NODE_TYPE, NODE_PLAYLIST,
				   -1);

		for_each_result_row(result, i) {
			add_entry_playlist(result.resultp[i],
					   NODE_PLAYLIST,
					   &iter, model, cwin);

			/* Have to give control to GTK periodically ... */
			#if GTK_CHECK_VERSION (3, 0, 0)
			pragha_process_gtk_events ();
			#else
			if (pragha_process_gtk_events ()) {
				sqlite3_free_table(result.resultp);
				return;
			}
			#endif
		}
	}
	sqlite3_free_table(result.resultp);

	/* Radios. */

	query = g_strdup_printf("SELECT NAME FROM RADIO ORDER BY NAME");
	exec_sqlite_query(query, cwin->cdbase, &result);

	if(result.no_rows) {
		gtk_tree_store_append(GTK_TREE_STORE(model),
				      &iter,
				      NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				   L_PIXBUF, cwin->pixbuf->pixbuf_dir,
				   L_NODE_DATA, _("Radios"),
				   L_NODE_TYPE, NODE_RADIO,
				   -1);

		for_each_result_row(result, i) {
			add_entry_playlist(result.resultp[i],
					   NODE_RADIO,
					   &iter, model, cwin);

			/* Have to give control to GTK periodically ... */
			#if GTK_CHECK_VERSION (3, 0, 0)
			pragha_process_gtk_events ();
			#else
			if (pragha_process_gtk_events ()) {
				sqlite3_free_table(result.resultp);
				return;
			}
			#endif
		}
	}
	sqlite3_free_table(result.resultp);

	/* Library. */

	switch(cwin->cpref->cur_library_view) {
	case FOLDERS:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Folders structure"));
		break;
	case ARTIST:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Artist"));
		order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Album"));
		if (cwin->cpref->sort_by_year)
			order_str = g_strdup("YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		else
			order_str = g_strdup("ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case GENRE:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Artist / Album"));
		if (cwin->cpref->sort_by_year)
			order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		else
			order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	case GENRE_ARTIST:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Artist"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case GENRE_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Album"));
		if (cwin->cpref->sort_by_year)
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		else
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	case GENRE_ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Artist / Album"));
		if (cwin->cpref->sort_by_year)
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		else
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	default:
		break;
	}

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, cwin->pixbuf->pixbuf_dir,
			   L_NODE_DATA, _("Library"),
			   -1);

	if (cwin->cpref->cur_library_view != FOLDERS) {
		/* Common query for all tag based library views */
		query = g_strdup_printf("SELECT TRACK.title, ARTIST.name, YEAR.year, ALBUM.name, GENRE.name, LOCATION.name, LOCATION.id "
					"FROM TRACK, ARTIST, YEAR, ALBUM, GENRE, LOCATION "
					"WHERE ARTIST.id = TRACK.artist AND TRACK.year = YEAR.id AND ALBUM.id = TRACK.album AND GENRE.id = TRACK.genre AND LOCATION.id = TRACK.location "
					"ORDER BY %s;", order_str);
		g_free(order_str);
			
		exec_sqlite_query(query, cwin->cdbase, &result);
		for_each_result_row(result, i) {
			add_child_node_by_tags(model,
			                       &iter,
			                       atoi(result.resultp[i+6]),
			                       sanitize_string_from_sqlite3(result.resultp[i+5]),
			                       sanitize_string_from_sqlite3(result.resultp[i+4]),
			                       sanitize_string_from_sqlite3(result.resultp[i+3]),
			                       sanitize_string_from_sqlite3(result.resultp[i+2]),
			                       sanitize_string_from_sqlite3(result.resultp[i+1]),
			                       sanitize_string_from_sqlite3(result.resultp[i]),
			                       cwin);

			/* Have to give control to GTK periodically ... */
			#if GTK_CHECK_VERSION (3, 0, 0)
			pragha_process_gtk_events ();
			#else
			if (pragha_process_gtk_events ()) {
				sqlite3_free_table(result.resultp);
				return;
			}
			#endif
		}
	}
	else {
		/* Query for folders view */
		query = g_strdup("SELECT name, id FROM LOCATION ORDER BY name DESC");
		exec_sqlite_query(query, cwin->cdbase, &result);
		for_each_result_row(result, i) {
			add_folder_file(result.resultp[i], atoi(result.resultp[i+1]), cwin, model, &iter);

			/* Have to give control to GTK periodically ... */
			#if GTK_CHECK_VERSION (3, 0, 0)
			pragha_process_gtk_events ();
			#else
			if (pragha_process_gtk_events ()) {
				sqlite3_free_table(result.resultp);
				return;
			}
			#endif
		}
	}
	sqlite3_free_table(result.resultp);

	/* Refresh tag completion entries, sensitive, set model and filter */

	refresh_tag_completion_entries(cwin);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->search_entry), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->library_tree), TRUE);

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);

	g_signal_emit_by_name (G_OBJECT (cwin->search_entry), "activate", cwin);

	remove_watch_cursor (cwin->mainwindow);

	cwin->cstate->view_change = FALSE;
}
