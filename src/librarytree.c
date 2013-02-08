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

/* Prepend a child (iter) to p_iter with given data. NOTE that iter
 * and p_iter must be created outside this function */

static void
library_store_prepend_node(GtkTreeModel *model,
                           GtkTreeIter *iter,
                           GtkTreeIter *p_iter,
                           GdkPixbuf *pixbuf,
                           const gchar *node_data,
                           int node_type,
                           int location_id)
{
	gtk_tree_store_prepend(GTK_TREE_STORE(model), iter, p_iter);

	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
	                   L_PIXBUF, pixbuf,
	                   L_NODE_DATA, node_data,
	                   L_NODE_TYPE, node_type,
	                   L_LOCATION_ID, location_id,
	                   L_MACH, FALSE,
	                   L_VISIBILE, TRUE,
	                   -1);
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
			   L_PIXBUF, cwin->clibrary->pixbuf_dir,
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

		if ((l_node_type == NODE_BASENAME) && (g_ascii_strcasecmp(data, node_data) >= 0)) {
			g_free(data);
			break;
		}
		g_free(data);

		valid = gtk_tree_model_iter_next(model, &l_iter);
	}

	/* Insert the new file after the last file by order */
	gtk_tree_store_insert_before(GTK_TREE_STORE(model), iter, p_iter, valid ? &l_iter : NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
			   L_PIXBUF, cwin->clibrary->pixbuf_track,
			   L_NODE_DATA, node_data,
			   L_NODE_TYPE, NODE_BASENAME,
			   L_LOCATION_ID, location_id,
			   L_MACH, FALSE,
			   L_VISIBILE, TRUE,
			   -1);
}

/* Adds a file and its parent directories to the library tree */

static void
add_folder_file(GtkTreeModel *model,
                const gchar *filepath,
                int location_id,
                GtkTreeIter *p_iter,
                struct con_win *cwin)
{
	gchar **subpaths = NULL;		/* To be freed */

	GtkTreeIter iter, iter2, search_iter;
	int i = 0 , len = 0;

	/* Point after library directory prefix */

	subpaths = g_strsplit(filepath, G_DIR_SEPARATOR_S, -1);

	len = g_strv_length (subpaths);
	len--;

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
	tot_levels = g_slist_length(cwin->clibrary->library_tree_nodes);
	while (node_level < tot_levels) {
		/* Set data to be added to the tree node depending on the type of node */
		node_type = GPOINTER_TO_INT(g_slist_nth_data(cwin->clibrary->library_tree_nodes, node_level));
		switch (node_type) {
			case NODE_TRACK:
				node_pixbuf = cwin->clibrary->pixbuf_track;
				if (string_is_not_empty(track)) {
					node_data = (gchar *)track;
				}
				else {
					node_data = get_display_filename(location, FALSE);
					need_gfree = TRUE;
				}
				break;
			case NODE_ARTIST:
				node_pixbuf = cwin->clibrary->pixbuf_artist;
				node_data = string_is_not_empty(artist) ? (gchar *)artist : _("Unknown Artist");
				break;
			case NODE_ALBUM:
				node_pixbuf = cwin->clibrary->pixbuf_album;
				if (cwin->cpref->sort_by_year) {
					node_data = g_strconcat ((string_is_not_empty(year) && (atoi(year) > 0)) ? year : _("Unknown"),
					                          " - ",
					                          string_is_not_empty(album) ? album : _("Unknown Album"),
					                          NULL);
					need_gfree = TRUE;
				}
				else {
					node_data = string_is_not_empty(album) ? (gchar *)album : _("Unknown Album");
				}
				break;
			case NODE_GENRE:
				node_pixbuf = cwin->clibrary->pixbuf_genre;
				node_data = string_is_not_empty(genre) ? (gchar *)genre : _("Unknown Genre");
				break;
			case NODE_CATEGORY:
			case NODE_FOLDER:
			case NODE_PLAYLIST:
			case NODE_RADIO:
			case NODE_BASENAME:
			default:
				g_warning("add_by_tag: Bad node type.");
				break;
		}

		/* Find / add child node if it's not already added */
		if (node_type != NODE_TRACK) {
			if (!find_child_node(node_data, &search_iter, p_iter, model)) {
				library_store_prepend_node(model,
				                           &iter,
				                           p_iter,
				                           node_pixbuf,
				                           node_data,
				                           node_type,
				                           0);
				p_iter = &iter;
			}
			else {
				iter2 = search_iter;
				p_iter = &iter2;
			}
		}
		else {
			library_store_prepend_node(model,
			                           &iter,
			                           p_iter,
			                           node_pixbuf,
			                           node_data,
			                           NODE_TRACK,
			                           location_id);
		}

		/* Free node_data if needed */
		if (need_gfree) {
			need_gfree = FALSE;
			g_free(node_data);
		}
		node_level++;
	}
}

GString *
append_pragha_uri_string_list(GtkTreeIter *r_iter,
                              GString *list,
                              GtkTreeModel *model)
{
	GtkTreeIter t_iter;
	enum node_type node_type = 0;
	gint location_id;
	gchar *data, *uri = NULL;
	gboolean valid;

	gtk_tree_model_get(model, r_iter, L_NODE_TYPE, &node_type, -1);

	switch (node_type) {
		case NODE_CATEGORY:
		case NODE_FOLDER:
		case NODE_GENRE:
		case NODE_ARTIST:
		case NODE_ALBUM:
			valid = gtk_tree_model_iter_children(model, &t_iter, r_iter);
			while (valid) {
				list = append_pragha_uri_string_list(&t_iter, list, model);

				valid = gtk_tree_model_iter_next(model, &t_iter);
			}
			pragha_process_gtk_events ();
	 		break;
		case NODE_TRACK:
		case NODE_BASENAME:
			gtk_tree_model_get(model, r_iter, L_LOCATION_ID, &location_id, -1);
			uri = g_strdup_printf("Location:/%d", location_id);
			break;
		case NODE_PLAYLIST:
			gtk_tree_model_get(model, r_iter, L_NODE_DATA, &data, -1);
			uri = g_strdup_printf("Playlist:/%s", data);
			g_free(data);
			break;
		case NODE_RADIO:
			gtk_tree_model_get(model, r_iter, L_NODE_DATA, &data, -1);
			uri = g_strdup_printf("Radio:/%s", data);
			g_free(data);
			break;
		default:
			break;
	}

	if(uri) {
		g_string_append (list, uri);
		g_string_append (list, "\r\n");
		g_free(uri);
	}

	return list;
}

GString *
append_uri_string_list(GtkTreeIter *r_iter,
                       GString *list,
                       GtkTreeModel *model,
                       struct con_win *cwin)
{
	GtkTreeIter t_iter;
	enum node_type node_type = 0;
	gint location_id;
	gchar *filename = NULL, *uri = NULL;
	gboolean valid;

	gtk_tree_model_get(model, r_iter, L_NODE_TYPE, &node_type, -1);

	switch (node_type) {
		case NODE_CATEGORY:
		case NODE_FOLDER:
		case NODE_GENRE:
		case NODE_ARTIST:
		case NODE_ALBUM:
			valid = gtk_tree_model_iter_children(model, &t_iter, r_iter);
			while (valid) {
				list = append_uri_string_list(&t_iter, list, model, cwin);

				valid = gtk_tree_model_iter_next(model, &t_iter);
			}
			pragha_process_gtk_events ();
			break;
		case NODE_TRACK:
		case NODE_BASENAME:
			gtk_tree_model_get(model, r_iter, L_LOCATION_ID, &location_id, -1);
			filename = pragha_database_get_filename_from_location_id(cwin->clibrary->cdbase, location_id);
			break;
		case NODE_PLAYLIST:
		case NODE_RADIO:
			g_message("Drag Radios and Playlist not yet implemented");
			break;
		default:
			break;
	}

	if(filename) {
		uri = g_filename_to_uri(filename, NULL, NULL);
		if(uri) {
			g_string_append (list, uri);
			g_string_append (list, "\r\n");

			g_free(uri);
		}
		g_free(filename);
	}

	return list;
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

	cwin->clibrary->view_change = TRUE;

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

	cwin->clibrary->view_change = FALSE;
}

/* Add all the tracks under the given path to the current playlist */

GList *
append_library_row_to_mobj_list(PraghaDatabase *cdbase,
                                GtkTreePath *path,
                                GtkTreeModel *row_model,
                                GList *list)
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
		case NODE_CATEGORY:
		case NODE_FOLDER:
		case NODE_GENRE:
		case NODE_ARTIST:
		case NODE_ALBUM:
			/* For all other node types do a recursive add */
			while (gtk_tree_model_iter_nth_child(row_model, &t_iter, &r_iter, j++)) {
				path = gtk_tree_model_get_path(row_model, &t_iter);
				list = append_library_row_to_mobj_list(cdbase, path, row_model, list);
				gtk_tree_path_free(path);
			}
			break;
		case NODE_TRACK:
		case NODE_BASENAME:
			mobj = new_musicobject_from_db(cdbase, location_id);
			if (G_LIKELY(mobj))
				list = g_list_append(list, mobj);
			break;
		case NODE_PLAYLIST:
			list = add_playlist_to_mobj_list(cdbase, data, list);
			break;
		case NODE_RADIO:
			list = add_radio_to_mobj_list(cdbase, data, list);
			break;
		default:
			break;
	}

	g_free(data);

	return list;
}

static void
delete_row_from_db(PraghaDatabase *cdbase,
                   GtkTreePath *path,
                   GtkTreeModel *model)
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
		pragha_database_forget_location(cdbase, location_id);
	}

	/* For all other node types do a recursive deletion */

	valid = gtk_tree_model_iter_children(model, &t_iter, &r_iter);
	while (valid) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			pragha_database_forget_location(cdbase, location_id);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			delete_row_from_db(cdbase, path, model);
			gtk_tree_path_free(path);
		}

		valid = gtk_tree_model_iter_next(model, &t_iter);
	}
}

static void trash_or_unlink_row(GArray *loc_arr, gboolean unlink,
				struct con_win *cwin)
{
	GtkWidget *question_dialog;
	gchar *primary, *secondary, *filename = NULL;
	gint response, location_id = 0;
	guint i;
	gboolean deleted = FALSE;
	GError *error = NULL;
	GFile *file = NULL;

	if (!loc_arr)
		return;

	for(i = 0; i < loc_arr->len; i++) {
		location_id = g_array_index(loc_arr, gint, i);
		if (location_id) {
			filename = pragha_database_get_filename_from_location_id(cwin->clibrary->cdbase, location_id);
			if (filename && g_file_test(filename, G_FILE_TEST_EXISTS)) {
				file = g_file_new_for_path(filename);

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
							return;
					}
				}
				if(unlink) {
					g_unlink(filename);
					deleted = TRUE;
				}
				g_object_unref(G_OBJECT(file));
			}
			if (deleted) {
				pragha_database_forget_location(cwin->clibrary->cdbase, location_id);
			}
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
	GList *list = NULL;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	gtk_tree_model_get_iter(filter_model, &iter, path);
	gtk_tree_model_get(filter_model, &iter, L_NODE_TYPE, &node_type, -1);

	switch(node_type) {
	case NODE_CATEGORY:
	case NODE_ARTIST:
	case NODE_ALBUM:
	case NODE_GENRE:
	case NODE_FOLDER:
		if (!gtk_tree_view_row_expanded(GTK_TREE_VIEW(cwin->clibrary->library_tree),
						path))
			gtk_tree_view_expand_row(GTK_TREE_VIEW(cwin->clibrary->library_tree),
						 path,
						 TRUE);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(cwin->clibrary->library_tree),
						   path);
		break;
	case NODE_TRACK:
	case NODE_BASENAME:
	case NODE_PLAYLIST:
	case NODE_RADIO:
		list = append_library_row_to_mobj_list(cwin->clibrary->cdbase, path, filter_model, list);
		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 list);
		g_list_free(list);
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
	gint n_select = 0;
	GList *list = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));

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

			list = append_library_row_to_mobj_list (cwin->clibrary->cdbase, path, model, list);

			if(list)
				pragha_playlist_append_mobj_list(cwin->cplaylist, list);
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
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));

	if((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK) || (cwin->clibrary->dragging == TRUE) || (event->button!=1)){
		gtk_tree_selection_set_select_function(selection, &tree_selection_func_true, cwin, NULL);
		cwin->clibrary->dragging = FALSE;
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

	if (!cwin->clibrary->view_change) {
		switch(event->button) {
		case 3: {
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
			ret = TRUE;
			break;
		}
		case 1: {
			if (widget == cwin->clibrary->combo_order){
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
	cwin->clibrary->dragging = TRUE;
	return FALSE;
}

gboolean
gtk_selection_data_set_pragha_uris (GtkSelectionData  *selection_data,
                                    GString *list)
{
	gchar *result;
	gsize length;

	result = g_convert (list->str, list->len,
	                    "ASCII", "UTF-8",
	                    NULL, &length, NULL);

	if (result) {
		gtk_selection_data_set (selection_data,
		                        gtk_selection_data_get_target(selection_data),
		                        8, (guchar *) result, length);
		g_free (result);

		return TRUE;
	}

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
	GString *rlist;
	GtkTreeIter s_iter;

	switch(info) {
	case TARGET_REF_LIBRARY:
		rlist = g_string_new (NULL);

		set_watch_cursor (cwin->mainwindow);
		cwin->clibrary->view_change = TRUE;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							cwin->clibrary->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		l = list;
		while(l) {
			if(gtk_tree_model_get_iter(model, &s_iter, l->data))
				rlist = append_pragha_uri_string_list(&s_iter, rlist, model);
			gtk_tree_path_free(l->data);
			l = l->next;
		}

		cwin->clibrary->view_change = FALSE;
		remove_watch_cursor (cwin->mainwindow);

		gtk_selection_data_set_pragha_uris(data, rlist);

		g_list_free(list);
		g_string_free (rlist, TRUE);
 		break;
 	case TARGET_URI_LIST:
		rlist = g_string_new (NULL);

		set_watch_cursor (cwin->mainwindow);
		cwin->clibrary->view_change = TRUE;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							cwin->clibrary->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		l = list;
		while(l) {
			if(gtk_tree_model_get_iter(model, &s_iter, l->data))
				rlist = append_uri_string_list(&s_iter, rlist, model, cwin);
			l = l->next;
		}

		cwin->clibrary->view_change = FALSE;
		remove_watch_cursor (cwin->mainwindow);

		gtk_selection_data_set_pragha_uris(data, rlist);

		g_list_free(list);
		g_string_free (rlist, TRUE);
		break;
	case TARGET_PLAIN_TEXT:
	default:
		g_warning("Unknown DND type");
		break;
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

	if (cwin->clibrary->filter_entry) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &node_data, -1);
		u_str = g_utf8_strdown(node_data, -1);
		if (pragha_strstr_lv(u_str, cwin->clibrary->filter_entry, cwin)) {
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
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), NULL);

	/* Set visibility of rows in the library store. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(cwin->clibrary->library_store),
				filter_tree_func,
				cwin);

	/* Set the model again.*/
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand all and then reduce properly. */
	gtk_tree_view_expand_all(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	gtk_tree_view_map_expanded_rows(GTK_TREE_VIEW(cwin->clibrary->library_tree),
		filter_tree_expand_func,
		filter_model);

	cwin->clibrary->timeout_id = 0;

	return FALSE;
}

void queue_refilter (struct con_win *cwin)
{
	if(cwin->clibrary->timeout_id)
		g_source_remove(cwin->clibrary->timeout_id);

	cwin->clibrary->timeout_id = g_timeout_add(500, (GSourceFunc)do_refilter, cwin);
}

gboolean simple_library_search_keyrelease_handler(GtkEntry *entry,
						  struct con_win *cwin)
{
	gchar *text = NULL;
	gboolean has_text;
	
	if (!pragha_preferences_get_instant_search(cwin->clibrary->preferences))
		return FALSE;

	if (cwin->clibrary->filter_entry != NULL) {
		g_free (cwin->clibrary->filter_entry);
		cwin->clibrary->filter_entry = NULL;
	}

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		cwin->clibrary->filter_entry = g_utf8_strdown (text, -1);

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

	if (cwin->clibrary->filter_entry != NULL) {
		g_free (cwin->clibrary->filter_entry);
		cwin->clibrary->filter_entry = NULL;
	}

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		cwin->clibrary->filter_entry = g_utf8_strdown (text, -1);

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
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), NULL);

	/* Set all nodes visibles. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(cwin->clibrary->library_store),
			       set_all_visible,
			       cwin);

	/* Set the model again. */
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand the categories. */

	valid = gtk_tree_model_get_iter_first (filter_model, &iter);
	while (valid) {
		path = gtk_tree_model_get_path(filter_model, &iter);
		gtk_tree_view_expand_row (GTK_TREE_VIEW(cwin->clibrary->library_tree), path, FALSE);
		gtk_tree_path_free(path);

		valid = gtk_tree_model_iter_next(filter_model, &iter);
	}
}

/********************************/
/* Library view order selection */
/********************************/

static void
library_pane_change_style (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;
	PraghaLibraryPane *clibrary = cwin->clibrary;

	g_slist_free (clibrary->library_tree_nodes);
	clibrary->library_tree_nodes = NULL;

	switch (pragha_preferences_get_library_style(clibrary->preferences)) {
		case FOLDERS:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
					       GINT_TO_POINTER(NODE_FOLDER));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				              GINT_TO_POINTER(NODE_BASENAME));
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Folders structure"));
			break;
		case ARTIST:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ARTIST));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Artist"));
			break;
		case ALBUM:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ALBUM));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Album"));
			break;
		case GENRE:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_GENRE));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Genre"));
			break;
		case ARTIST_ALBUM:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ARTIST));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ALBUM));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Artist / Album"));
			break;
		case GENRE_ARTIST:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_GENRE));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ARTIST));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Genre / Artist"));
			break;
		case GENRE_ALBUM:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_GENRE));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ALBUM));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Genre / Album"));
			break;
		case GENRE_ARTIST_ALBUM:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_GENRE));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ARTIST));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ALBUM));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(cwin->clibrary->combo_order_label), _("Genre / Artist / Album"));
			break;
		default:
			break;
	}

	library_pane_view_reload(cwin);
}

void folders_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, FOLDERS);
}

void artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, ARTIST);
}

void album_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, ALBUM);
}

void genre_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, GENRE);
}

void artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, ARTIST_ALBUM);
}

void genre_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, GENRE_ALBUM);
}

void genre_artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, GENRE_ARTIST);
}

void genre_artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_library_style(cwin->clibrary->preferences, GENRE_ARTIST_ALBUM);
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

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		pragha_playlist_remove_all (cwin->cplaylist);

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			mlist = append_library_row_to_mobj_list (cwin->clibrary->cdbase, path, model, mlist);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}

		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 mlist);

		g_list_free(list);
		g_list_free(mlist);
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

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			mlist = append_library_row_to_mobj_list(cwin->clibrary->cdbase, path, model, mlist);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}
		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 mlist);

		g_list_free(list);
		g_list_free(mlist);
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

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
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

			db_begin_transaction(cwin->clibrary->cdbase);

			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				delete_row_from_db(cwin->clibrary->cdbase, path, model);

				/* Have to give control to GTK periodically ... */
				if (pragha_process_gtk_events ())
					return;
			}

			db_commit_transaction(cwin->clibrary->cdbase);

			flush_stale_entries_db(cwin->clibrary->cdbase);
			library_pane_view_reload(cwin);
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

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
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

			db_begin_transaction(cwin->clibrary->cdbase);
			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				get_location_ids(path, loc_arr, model, cwin);
				trash_or_unlink_row(loc_arr, unlink, cwin);

				/* Have to give control to GTK periodically ... */
				if (pragha_process_gtk_events ())
					return;
			}
			db_commit_transaction(cwin->clibrary->cdbase);

			g_array_free(loc_arr, TRUE);

			flush_stale_entries_db(cwin->clibrary->cdbase);
			library_pane_view_reload(cwin);
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

	PraghaMusicobject *omobj = NULL, *nmobj = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
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

			omobj = new_musicobject_from_db(cwin->clibrary->cdbase, location_id);
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
	} else {
		omobj = pragha_musicobject_new ();
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
			if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(nmobj), cwin->mainwindow))
				goto exit;
		}
	}

	/* Check if user is trying to set the same title/track no for
	   multiple tracks */
	if (changed & TAG_TITLE_CHANGED) {
		if (loc_arr->len > 1) {
			if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(nmobj), cwin->mainwindow))
				goto exit;
		}
	}

	/* Updata the db changes */
	pragha_db_update_local_files_change_tag(cwin->clibrary->cdbase, loc_arr, changed, nmobj);
	library_pane_view_reload(cwin);

	/* Get a array of files and update it */
	file_arr = g_ptr_array_new();
	for(ielem = 0; ielem < loc_arr->len; ielem++) {
		elem = g_array_index(loc_arr, gint, ielem);
		if (elem) {
			file = pragha_database_get_filename_from_location_id(cwin->clibrary->cdbase, elem);
			if(file)
				g_ptr_array_add(file_arr, file);
		}
	}
	pragha_update_local_files_change_tag(file_arr, changed, nmobj);
	g_ptr_array_free(file_arr, TRUE);
	g_array_free(loc_arr, TRUE);

exit:
	/* Cleanup */

	g_free(node_data);
	g_strfreev (split_album);

	if (nmobj)
		g_object_unref (nmobj);
	if (omobj)
		g_object_unref (omobj);
		
	g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
}

static void
library_view_append_playlists(GtkTreeModel *model,
                              GtkTreeIter *p_iter,
                              struct con_win *cwin)
{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *playlist = NULL;
	GtkTreeIter iter;

	sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE DESC";
	statement = pragha_database_create_statement (cwin->clibrary->cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		playlist = pragha_prepared_statement_get_string(statement, 0);

		library_store_prepend_node(model,
		                           &iter,
		                           p_iter,
		                           cwin->clibrary->pixbuf_track,
		                           playlist,
		                           NODE_PLAYLIST,
		                           0);

		#if GTK_CHECK_VERSION (3, 0, 0)
		pragha_process_gtk_events ();
		#else
		if (pragha_process_gtk_events ()) {
			pragha_prepared_statement_free (statement);
			return;
		}
		#endif
	}
	pragha_prepared_statement_free (statement);
}

static void
library_view_append_radios(GtkTreeModel *model,
                           GtkTreeIter *p_iter,
                           struct con_win *cwin)
{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *radio = NULL;
	GtkTreeIter iter;

	sql = "SELECT name FROM RADIO ORDER BY name COLLATE NOCASE DESC";
	statement = pragha_database_create_statement (cwin->clibrary->cdbase, sql);
	while (pragha_prepared_statement_step (statement)) {
		radio = pragha_prepared_statement_get_string(statement, 0);

		library_store_prepend_node(model,
		                           &iter,
		                           p_iter,
		                           cwin->clibrary->pixbuf_track,
		                           radio,
		                           NODE_RADIO,
		                           0);

		#if GTK_CHECK_VERSION (3, 0, 0)
		pragha_process_gtk_events ();
		#else
		if (pragha_process_gtk_events ()) {
			pragha_prepared_statement_free (statement);
			return;
		}
		#endif
	}
	pragha_prepared_statement_free (statement);
}

void
library_view_complete_folder_view(GtkTreeModel *model,
                                  GtkTreeIter *p_iter,
                                  struct con_win *cwin)

{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *filepath = NULL;
	gchar *mask = NULL;
	GtkTreeIter iter, *f_iter;
	GSList *list = NULL, *library_dir = NULL;

	library_dir =
		pragha_preferences_get_filename_list(cwin->clibrary->preferences,
			                             GROUP_LIBRARY,
			                             KEY_LIBRARY_DIR);

	for(list = library_dir ; list != NULL ; list=list->next) {
		/*If no need to fuse folders, add headers and set p_iter */
		if(!cwin->cpref->fuse_folders) {
			gtk_tree_store_append(GTK_TREE_STORE(model),
					      &iter,
					      p_iter);
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
					   L_PIXBUF, cwin->clibrary->pixbuf_dir,
					   L_NODE_DATA, list->data,
					   L_NODE_TYPE, NODE_FOLDER,
					   L_LOCATION_ID, 0,
					   L_MACH, FALSE,
					   L_VISIBILE, TRUE,
					   -1);
			f_iter = &iter;
		}
		else {
			f_iter = p_iter;
		}

		sql = "SELECT name, id FROM LOCATION WHERE name LIKE ? ORDER BY name DESC";
		statement = pragha_database_create_statement (cwin->clibrary->cdbase, sql);
		mask = g_strconcat (list->data, "%", NULL);
		pragha_prepared_statement_bind_string (statement, 1, mask);
		while (pragha_prepared_statement_step (statement)) {
			filepath = pragha_prepared_statement_get_string(statement, 0) + strlen(list->data) + 1;
			add_folder_file(model,
			                filepath,
			                pragha_prepared_statement_get_int(statement, 1),
			                f_iter,
			                cwin);

			#if GTK_CHECK_VERSION (3, 0, 0)
			pragha_process_gtk_events ();
			#else
			if (pragha_process_gtk_events ()) {
				pragha_prepared_statement_free (statement);
				free_str_list(library_dir);
				g_free(mask);
				return;
			}
			#endif
		}
		pragha_prepared_statement_free (statement);
		g_free(mask);
	}
	free_str_list(library_dir);
}

void
library_view_complete_tags_view(GtkTreeModel *model,
                                GtkTreeIter *p_iter,
                                struct con_win *cwin)
{
	PraghaPreparedStatement *statement;
	gchar *order_str = NULL, *sql = NULL;

	/* Get order needed to sqlite query. */
	switch(pragha_preferences_get_library_style(cwin->clibrary->preferences)) {
		case FOLDERS:
			break;
		case ARTIST:
			order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case ALBUM:
			if (cwin->cpref->sort_by_year)
				order_str = g_strdup("YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			else
				order_str = g_strdup("ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case GENRE:
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case ARTIST_ALBUM:
			if (cwin->cpref->sort_by_year)
				order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			else
				order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			break;
		case GENRE_ARTIST:
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case GENRE_ALBUM:
			if (cwin->cpref->sort_by_year)
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			else
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			break;
		case GENRE_ARTIST_ALBUM:
			if (cwin->cpref->sort_by_year)
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			else
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			break;
		default:
			break;
	}

	/* Common query for all tag based library views */
	sql = g_strdup_printf("SELECT TRACK.title, ARTIST.name, YEAR.year, ALBUM.name, GENRE.name, LOCATION.name, LOCATION.id "
	                        "FROM TRACK, ARTIST, YEAR, ALBUM, GENRE, LOCATION "
	                        "WHERE ARTIST.id = TRACK.artist AND TRACK.year = YEAR.id AND ALBUM.id = TRACK.album AND GENRE.id = TRACK.genre AND LOCATION.id = TRACK.location "
	                        "ORDER BY %s;", order_str);

	statement = pragha_database_create_statement (cwin->clibrary->cdbase, sql);
	while (pragha_prepared_statement_step (statement)) {
		add_child_node_by_tags(model,
		                       p_iter,
		                       pragha_prepared_statement_get_int(statement, 6),
		                       pragha_prepared_statement_get_string(statement, 5),
		                       pragha_prepared_statement_get_string(statement, 4),
		                       pragha_prepared_statement_get_string(statement, 3),
		                       pragha_prepared_statement_get_string(statement, 2),
		                       pragha_prepared_statement_get_string(statement, 1),
		                       pragha_prepared_statement_get_string(statement, 0),
		                       cwin);

		/* Have to give control to GTK periodically ... */
		#if GTK_CHECK_VERSION (3, 0, 0)
		pragha_process_gtk_events ();
		#else
		if (pragha_process_gtk_events ()) {
			pragha_prepared_statement_free (statement);
			return;
		}
		#endif
	}
	pragha_prepared_statement_free (statement);

	g_free(order_str);
	g_free(sql);
}

/********/
/* Init */
/********/

void
library_pane_view_reload(struct con_win *cwin)
{
	GtkTreeModel *model, *filter_model;
	GtkTreeIter iter;

	cwin->clibrary->view_change = TRUE;

	set_watch_cursor (cwin->mainwindow);

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	g_object_ref(filter_model);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->clibrary->search_entry), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->clibrary->library_tree), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), NULL);

	gtk_tree_store_clear(GTK_TREE_STORE(model));

	/* Playlists.*/

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, cwin->clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Playlists"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	library_view_append_playlists(model, &iter, cwin);

	/* Radios. */

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, cwin->clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Radios"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	library_view_append_radios(model, &iter, cwin);

	/* Add library header */

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, cwin->clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Library"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	if (pragha_preferences_get_library_style(cwin->clibrary->preferences) == FOLDERS) {
		library_view_complete_folder_view(model, &iter, cwin);
	}
	else {
		library_view_complete_tags_view(model, &iter, cwin);
	}

	/* Refresh tag completion entries, sensitive, set model and filter */

	/* TODO: Move to database?
	 *refresh_tag_completion_entries(cwin); */

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->clibrary->search_entry), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->clibrary->library_tree), TRUE);

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	g_signal_emit_by_name (G_OBJECT (cwin->clibrary->search_entry), "activate", cwin);

	remove_watch_cursor (cwin->mainwindow);

	cwin->clibrary->view_change = FALSE;
}

void init_library_view(struct con_win *cwin)
{
	library_pane_change_style(NULL, NULL, cwin);
}

static void
pragha_library_pane_init_pixbufs(PraghaLibraryPane *librarypane)
{
	GtkIconTheme *icontheme = gtk_icon_theme_get_default();

	librarypane->pixbuf_artist =
		gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR"/artist.png",
		                                  16, 16,
		                                  TRUE,
		                                  NULL);
	if (!librarypane->pixbuf_artist)
		g_warning("Unable to load artist png");

	librarypane->pixbuf_album =
		gtk_icon_theme_load_icon(icontheme,
		                         "media-optical",
		                         16, 0,
		                         NULL);
	if (!librarypane->pixbuf_album)
		librarypane->pixbuf_album =
			gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR"/album.png",
			                                  16, 16,
			                                  TRUE, NULL);
	if (!librarypane->pixbuf_album)
		g_warning("Unable to load album png");

	librarypane->pixbuf_track =
		gtk_icon_theme_load_icon(icontheme,
		                         "audio-x-generic",
		                         16, 0,
		                         NULL);
	if (!librarypane->pixbuf_track)
		librarypane->pixbuf_track =
			gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR "/track.png",
			                                  16, 16,
			                                  TRUE, NULL);

	if (!librarypane->pixbuf_track)
		g_warning("Unable to load track png");

	librarypane->pixbuf_genre =
		gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR"/genre.png",
			                                  16, 16,
			                                  TRUE, NULL);
	if (!librarypane->pixbuf_genre)
		g_warning("Unable to load genre png");

	librarypane->pixbuf_dir =
		gtk_icon_theme_load_icon(icontheme,
		                         "folder-music",
		                         16, 0,
		                         NULL);
	if (!librarypane->pixbuf_dir)
		librarypane->pixbuf_dir =
			gtk_icon_theme_load_icon(icontheme,
			                         "folder",
			                         16, 0,
			                         NULL);
	if (!librarypane->pixbuf_dir)
		g_warning("Unable to load folder png");
}

void
pragha_library_pane_free(PraghaLibraryPane *librarypane)
{
	if (librarypane->pixbuf_dir)
		g_object_unref(librarypane->pixbuf_dir);
	if (librarypane->pixbuf_artist)
		g_object_unref(librarypane->pixbuf_artist);
	if (librarypane->pixbuf_album)
		g_object_unref(librarypane->pixbuf_album);
	if (librarypane->pixbuf_track)
		g_object_unref(librarypane->pixbuf_track);
	if (librarypane->pixbuf_genre)
		g_object_unref(librarypane->pixbuf_genre);

	g_object_unref(librarypane->cdbase);
	g_object_unref(librarypane->preferences);

	g_slist_free(librarypane->library_tree_nodes);

	g_slice_free(PraghaLibraryPane, librarypane);
}

void
pragha_library_pane_init_preferences(PraghaLibraryPane *librarypane)
{

}

PraghaLibraryPane *
pragha_library_pane_new(struct con_win *cwin)
{
	PraghaLibraryPane *clibrary;

	clibrary = g_slice_new0(PraghaLibraryPane);

	clibrary->cdbase = pragha_database_get();
	clibrary->preferences = pragha_preferences_get();

	clibrary->filter_entry = NULL;
	clibrary->dragging = FALSE;
	clibrary->view_change = TRUE;
	clibrary->timeout_id = 0;
	g_slist_free(clibrary->library_tree_nodes);

	pragha_library_pane_init_pixbufs(clibrary);

	g_signal_connect(clibrary->preferences, "notify::library-style", G_CALLBACK (library_pane_change_style), cwin);

	return clibrary;
}


