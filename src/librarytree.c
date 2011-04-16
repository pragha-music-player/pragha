/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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
	gint i = 0;

	while (gtk_tree_model_iter_nth_child(model, iter, p_iter, i++)) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &data, -1);
		if (data && !g_ascii_strcasecmp(data, node_data)) {
			g_free(data);
			return TRUE;
		}
		g_free(data);
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
		L_VISIBILE, TRUE, -1);
}

/* Appends a child (iter) to p_iter with given data. NOTE that iter
 * and p_iter must be created outside this function */

static void add_child_node_by_folder(GtkTreeModel *model, GtkTreeIter *iter,
	GtkTreeIter *p_iter, GdkPixbuf *pixbuf, const gchar *node_data, 
	int node_type, int location_id)
{
	GtkTreeIter l_iter;
	gchar *data = NULL;
	gint i = 0, pos = 0, l_node_type;
       
	if (node_type == NODE_FOLDER) {
		/* Find position of the last directory that is a child of p_iter */
		while (gtk_tree_model_iter_nth_child(model, &l_iter, p_iter, i++)) {
			gtk_tree_model_get(model, &l_iter, L_NODE_TYPE, &l_node_type, -1);
			gtk_tree_model_get(model, &l_iter, L_NODE_DATA, &data, -1);
			if (l_node_type != NODE_FOLDER)
				break;
			if (g_ascii_strcasecmp(data, node_data) < 0)
				pos++;
			g_free(data);
		}
	}
	else {
		/* Find position of the last file that is a child of p_iter */
		while (gtk_tree_model_iter_nth_child(model, &l_iter, p_iter, i++)) {
			gtk_tree_model_get(model, &l_iter, L_NODE_TYPE, &l_node_type, -1);
			gtk_tree_model_get(model, &l_iter, L_NODE_DATA, &data, -1);

			if ((l_node_type == NODE_FOLDER) || (g_ascii_strcasecmp(data, node_data) < 0))
				pos++;
            		g_free(data);
		}
	}
	/* Insert the new file after the last subdirectory/file by order */
	gtk_tree_store_insert(GTK_TREE_STORE(model), iter, p_iter, pos);

	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
		L_PIXBUF, pixbuf,
		L_NODE_DATA, node_data,
		L_NODE_TYPE, node_type,
		L_LOCATION_ID, location_id,
		L_VISIBILE, TRUE, -1);
}

/* Helper function for add_folder_file() */

static void add_subpath(const gchar *path, int location_id,
	struct con_win *cwin, GtkTreeModel *model)
{
	static GtkTreeIter iter1, iter2, *p_iter = NULL;
	GtkTreeIter search_iter;
			
	if (!find_child_node(path, &search_iter, p_iter, model)) {
		add_child_node_by_folder(model, &iter1, p_iter,
			location_id ? cwin->pixbuf->pixbuf_track : cwin->pixbuf->pixbuf_dir,
			path, location_id ? NODE_BASENAME : NODE_FOLDER, location_id);
		p_iter = location_id ? NULL : &iter1;
	}
	else {
		iter2 = search_iter;
		p_iter = location_id ? NULL : &iter2;
	}
}

/* Adds a file and its parent directories to the library tree */

static void add_folder_file(const gchar *path, int location_id,
	struct con_win *cwin, GtkTreeModel *model)
{
	gchar *filename = NULL, *fullpath = NULL, **subpaths = NULL;	/* To be freed */
	gchar *prefix = NULL, *filepath = NULL;				/* Do not free */
	int i = 0;
		
	/* Search all library directories for the one that matches the path */
	while ((prefix = g_slist_nth_data(cwin->cpref->library_dir, i++))) {	
		if (g_str_has_prefix(path, prefix)) {
			fullpath = get_display_filename(path, TRUE);
			filename = get_display_filename(path, FALSE);

			if (!cwin->cpref->fuse_folders)
				add_subpath(prefix, 0, cwin, model);
			break;
		}
	}

	/* Point after library directory prefix */
	filepath = fullpath + strlen(prefix) + 1;
	subpaths = g_strsplit(filepath, G_DIR_SEPARATOR_S, -1);

	/* Add all subdirectories to the tree */
	for (i = 0; subpaths[i]; i++) {
		add_subpath(subpaths[i], 0, cwin, model);
	}

	/* Finally add filename */
	add_subpath(filename, location_id, cwin, model);

	g_strfreev(subpaths);
	g_free(filename);
	g_free(fullpath);
}

/* Adds an entry to the library tree by tag (genre, artist...) */

static void add_by_tag(gint location_id, gchar *location, gchar *genre,
	gchar *artist, gchar *album, gchar *track, struct con_win *cwin,
	GtkTreeModel *model)
{
	GtkTreeIter iter, iter2, search_iter, *p_iter = NULL;
	gchar *node_data = NULL, *node = NULL;
	GdkPixbuf *node_pixbuf = NULL;
	enum node_type node_type = 0;
	gint node_level = 0, tot_levels = 0;
	gboolean need_gfree = FALSE;

	/* Iterate through library tree node types */ 
	tot_levels = g_slist_length(cwin->cpref->library_tree_nodes);
	while (node_level < tot_levels) {
		/* Set data to be added to the tree node depending on the type of node */
		node = g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level);

		if (!g_ascii_strcasecmp(P_TITLE_STR, node)) {
			node_type = NODE_TRACK;
			node_pixbuf = cwin->pixbuf->pixbuf_track;
			node_data = strlen(track) ? track : get_display_filename(location, FALSE);
			if (!strlen(track)) need_gfree = TRUE;
		}
		else if (!g_ascii_strcasecmp(P_ARTIST_STR, node)) {
			node_type = NODE_ARTIST;
			node_pixbuf = cwin->pixbuf->pixbuf_artist;
			node_data = strlen(artist) ? artist : g_strdup(_("Unknown Artist"));
			if (!strlen(artist)) need_gfree = TRUE;
		}
		else if (!g_ascii_strcasecmp(P_ALBUM_STR, node)) {
			node_type = NODE_ALBUM;
			node_pixbuf = cwin->pixbuf->pixbuf_album;
			node_data = strlen(album) ? album : g_strdup(_("Unknown Album"));
			if (!strlen(album)) need_gfree = TRUE;
		}
		else if (!g_ascii_strcasecmp(P_GENRE_STR, node)) {
			node_type = NODE_GENRE;
			node_pixbuf = cwin->pixbuf->pixbuf_genre;
			node_data = strlen(genre) ? genre : g_strdup(_("Unknown Genre"));
			if (!strlen(genre)) need_gfree = TRUE;
		}

		/* Find / add child node if it's not already added */
		if (!find_child_node(node_data, &search_iter, p_iter, model)) {
			add_child_node_by_tag(model, &iter, p_iter, node_pixbuf,
				node_data, node_type, node_type == NODE_TRACK ? location_id : 0);
			p_iter = &iter;
		}
		else {
			iter2 = search_iter;
			p_iter = &iter2;
		}
		/* Free node_data if needed */
		if (need_gfree) {
			need_gfree = FALSE;
			g_free(node_data);
		}
		node_level++;
	}
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
}

/* Add all the tracks under the given path to the current playlist */

static void add_row_current_playlist(GtkTreePath *path,
				     GtkTreeModel *model,
				     struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gint location_id;
	struct musicobject *mobj = NULL;
	gint j = 0;

	/* If this path is a track, just append it to the current playlist */

	gtk_tree_model_get_iter(model, &r_iter, path);
	gtk_tree_model_get(model, &r_iter, L_NODE_TYPE, &node_type, -1);
	if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
		gtk_tree_model_get(model, &r_iter, L_LOCATION_ID, &location_id, -1);
		mobj = new_musicobject_from_db(location_id, cwin);
		if (!mobj)
			g_warning("Unable to retrieve details "
				  "for location_id : %d",
				  location_id);
		else
			append_current_playlist(mobj, cwin);
	}

	/* For all other node types do a recursive add */

	while (gtk_tree_model_iter_nth_child(model, &t_iter, &r_iter, j++)) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			mobj = new_musicobject_from_db(location_id, cwin);
			if (!mobj)
				g_warning("Unable to retrieve details "
					  "for location_id : %d",
					  location_id);
			else
				append_current_playlist(mobj, cwin);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			add_row_current_playlist(path, model, cwin);
			gtk_tree_path_free(path);
		}
	}
}

static void delete_row_from_db(GtkTreePath *path, GtkTreeModel *model,
			       struct con_win *cwin)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gint location_id;
	gint j = 0;

	/* If this path is a track, delete it immediately */

	gtk_tree_model_get_iter(model, &r_iter, path);
	gtk_tree_model_get(model, &r_iter, L_NODE_TYPE, &node_type, -1);
	if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
		gtk_tree_model_get(model, &r_iter, L_LOCATION_ID, &location_id, -1);
		delete_location_db(location_id, cwin);
	}

	/* For all other node types do a recursive deletion */

	while (gtk_tree_model_iter_nth_child(model, &t_iter, &r_iter, j++)) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			delete_location_db(location_id, cwin);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			delete_row_from_db(path, model, cwin);
			gtk_tree_path_free(path);
		}
	}
}

static void trash_or_unlink_row(GArray *loc_arr, gboolean unlink,
				struct con_win *cwin)
{
	GtkWidget *question_dialog;
	gchar *query = NULL, *filename = NULL;
	gchar *primary, *secondary;
	gint response, i, location_id = 0;
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
			if (exec_sqlite_query(query, cwin, &result)) {
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
				delete_location_db(location_id, cwin);
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
		add_row_current_playlist(path, filter_model, cwin);
		break;
	default:
		break;
	}
}

gboolean library_tree_button_press_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin)
{
	GtkWidget *popup_menu;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	gboolean many_selected = FALSE;

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
			library_tree_add_to_playlist(cwin);
			break;
		case 3:
			if (!(gtk_tree_selection_path_is_selected(selection, path))){
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}
	
			popup_menu = gtk_ui_manager_get_widget(cwin->library_tree_context_menu,
							       "/popup");
	
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
	
			/* If more than one track is selected, don't propagate event */
	
			if (gtk_tree_selection_count_selected_rows(selection) > 1)
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

	if ((gtk_notebook_get_current_page(GTK_NOTEBOOK(cwin->browse_mode)) == 0) &&
		(!cwin->cstate->view_change)){
		switch(event->button) {
		case 3: {
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, event->time);
			ret = TRUE;
		}
		case 1: {
			if (widget == cwin->combo_order){
				gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL,
						(GtkMenuPositionFunc) menu_position, widget, 
						0, gtk_get_current_event_time());
				ret = TRUE;
			}
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
	GArray *loc_arr;

	switch(info) {
	case TARGET_LOCATION_ID:
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							cwin->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		/* No selections */

		if (!list) {
			gtk_selection_data_set(data, data->type, 8, NULL, 0);
			break;
		}

		/* Form an array of location ids */

		loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
		l = list;

		while(l) {
			get_location_ids(l->data, loc_arr, model, cwin);
			gtk_tree_path_free(l->data);
			l = l->next;
		}

		gtk_selection_data_set(data,
				       data->type,
				       8,
				       (guchar *)&loc_arr,
				       sizeof(GArray *));

		CDEBUG(DBG_VERBOSE, "Fill DnD data, selection: %p, loc_arr: %p",
		       data->data, loc_arr);

		/* Cleanup */

		g_list_free(list);

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
			   L_VISIBILE, TRUE, -1);
	return FALSE;
}

static void filter_tree_expand_func(GtkTreeView *view,
				    GtkTreePath *path,
				    gpointer data)
{
	struct con_win *cwin = data;
	GtkTreeModel *filter_model;
	GtkTreeIter iter;
	gchar *node_data = NULL, *u_str = NULL;
	enum node_type node_type;

	filter_model = gtk_tree_view_get_model(view);
	gtk_tree_model_get_iter(filter_model, &iter, path);
	gtk_tree_model_get(filter_model, &iter, L_NODE_DATA, &node_data, -1);
	gtk_tree_model_get(filter_model, &iter, L_NODE_TYPE, &node_type, -1);

	u_str = g_utf8_strdown(node_data, -1);

	/* Collapse any non-leaf node that matches the seach entry */

	if (cwin->cstate->filter_entry &&
	    (node_type != NODE_TRACK) &&
	    (node_type != NODE_BASENAME) &&
	    g_strrstr(u_str, cwin->cstate->filter_entry))
		gtk_tree_view_collapse_row(view, path);

	g_free(u_str);
	g_free(node_data);
}

static gboolean filter_tree_func(GtkTreeModel *model,
				 GtkTreePath *path,
				 GtkTreeIter *iter,
				 gpointer data)
{
	struct con_win *cwin = data;
	gchar *node_data = NULL, *t_node_data, *u_str;
	gboolean visible, t_flag = FALSE;
	GtkTreePath *t_path;
	GtkTreeIter t_iter;

	/* Mark node and its parents visible if search entry matches.
	   If search entry doesn't match, check if _any_ ancestor has
	   been marked as visible and if so, mark current node as visible too. */

	if (cwin->cstate->filter_entry) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &node_data, -1);
		u_str = g_utf8_strdown(node_data, -1);
		if (g_strrstr(u_str, cwin->cstate->filter_entry)) {
			gtk_tree_store_set(GTK_TREE_STORE(model), iter,
					   L_VISIBILE, TRUE, -1);
			t_path = gtk_tree_model_get_path(model, iter);
			while (gtk_tree_path_up(t_path)) {
				if (gtk_tree_path_get_depth(t_path) > 0) {
					gtk_tree_model_get_iter(model, &t_iter, t_path);
					gtk_tree_store_set(GTK_TREE_STORE(model), &t_iter,
							   L_VISIBILE, TRUE, -1);
				}
			}
			gtk_tree_path_free(t_path);
		} else {
			t_path = gtk_tree_model_get_path(model, iter);
			while (gtk_tree_path_up(t_path)) {
				if (gtk_tree_path_get_depth(t_path) > 0) {
					gtk_tree_model_get_iter(model, &t_iter,
								t_path);
					gtk_tree_model_get(model,
							   &t_iter,
							   L_NODE_DATA,
							   &t_node_data,
							   -1);
					gtk_tree_model_get(model,
							   &t_iter,
							   L_VISIBILE,
							   &visible,
							   -1);

					gchar *u_str = g_utf8_strdown(t_node_data, -1);

					if (visible && g_strrstr(u_str,
								 cwin->cstate->filter_entry))
						t_flag = TRUE;

					g_free(u_str);
					g_free(t_node_data);
				}
			}

			if (t_flag)
				gtk_tree_store_set(GTK_TREE_STORE(model), iter,
						   L_VISIBILE, TRUE, -1);
			else
				gtk_tree_store_set(GTK_TREE_STORE(model), iter,
						   L_VISIBILE, FALSE, -1);

			gtk_tree_path_free(t_path);
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

	cwin->cstate->timeout_id = 0;

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(cwin->library_store),
				filter_tree_func,
				cwin);
	filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(cwin->library_store),
			NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(filter_model),
			L_VISIBILE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(cwin->library_tree));
	gtk_tree_view_map_expanded_rows(GTK_TREE_VIEW(cwin->library_tree),
		filter_tree_expand_func,
		cwin);

	return FALSE;
}

gboolean simple_library_search_keyrelease_handler(GtkEntry *entry,
						  struct con_win *cwin)
{

	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if( cwin->cstate->timeout_id ){
		g_source_remove( cwin->cstate->timeout_id );
		cwin->cstate->timeout_id = 0;
	}
	if(has_text){
		text = gtk_editable_get_chars( GTK_EDITABLE(entry), 0, -1 );
		u_str = g_utf8_strdown(text, -1);
		cwin->cstate->filter_entry = u_str;
		cwin->cstate->timeout_id = g_timeout_add( 300, (GSourceFunc)do_refilter, cwin );
	}
	else{
		clear_library_search(cwin);
	}
	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	return FALSE;
}

void clear_library_search(struct con_win *cwin)
{
	GtkTreeModel *model, *filter_model;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	cwin->cstate->filter_entry = NULL;
	gtk_tree_model_foreach(model, set_all_visible, cwin);
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(cwin->library_tree));
}

/********************************/
/* Library view order selection */
/********************************/

void folders_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_FOLDER_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_BASENAME_STR));
	cwin->cpref->cur_library_view = FOLDERS;

	init_library_view(cwin);
}

void artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ARTIST_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = ARTIST;

	init_library_view(cwin);
}

void album_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ALBUM_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = ALBUM;

	init_library_view(cwin);
}

void genre_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_GENRE_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = GENRE;

	init_library_view(cwin);
}

void artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ARTIST_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ALBUM_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = ARTIST_ALBUM;

	init_library_view(cwin);
}

void genre_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_GENRE_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ALBUM_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = GENRE_ALBUM;

	init_library_view(cwin);
}

void genre_artist_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_GENRE_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ARTIST_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = GENRE_ARTIST;

	init_library_view(cwin);
}

void genre_artist_album_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_GENRE_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ARTIST_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_ALBUM_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_TITLE_STR));
	cwin->cpref->cur_library_view = GENRE_ARTIST_ALBUM;

	init_library_view(cwin);
}

/*****************/
/* Menu handlers */
/*****************/

void library_tree_replace_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {

		clear_current_playlist(action, cwin);

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			add_row_current_playlist(path, model, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			while(gtk_events_pending())
				if (gtk_main_iteration_do(FALSE))
					return;
		}
		
		g_list_free(list);
	}
}

void library_tree_replace_and_play(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		clear_current_playlist(action, cwin);

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			add_row_current_playlist(path, model, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			while(gtk_events_pending())
				if (gtk_main_iteration_do(FALSE))
					return;
		}
		
		g_list_free(list);
	}
	play_first_current_playlist(cwin);
}

void library_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin)
{
	library_tree_add_to_playlist(cwin);
}
void library_tree_add_to_playlist(struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {

		/* Add all the rows to the current playlist */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			add_row_current_playlist(path, model, cwin);
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			while(gtk_events_pending())
				if (gtk_main_iteration_do(FALSE))
					return;
		}
		
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
	gchar *query;
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

			query = g_strdup_printf("BEGIN;");
			exec_sqlite_query(query, cwin, NULL);

			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				delete_row_from_db(path, model, cwin);
				gtk_tree_path_free(path);

				/* Have to give control to GTK periodically ... */
				/* If gtk_main_quit has been called, return -
				   since main loop is no more. */

				while(gtk_events_pending())
					if (gtk_main_iteration_do(FALSE))
						return;
			}
			g_list_free(list);

			query = g_strdup_printf("END;");
			exec_sqlite_query(query, cwin, NULL);

			flush_stale_entries_db(cwin);
			init_library_view(cwin);
		}
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
	gchar *query;
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
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), toggle_unlink, TRUE, TRUE, 0);

		gtk_widget_show_all(dialog);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		unlink = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_unlink));
		gtk_widget_destroy(dialog);

		if(result == GTK_RESPONSE_YES){
			query = g_strdup_printf("BEGIN;");
			exec_sqlite_query(query, cwin, NULL);

			loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));

			for (i=list; i != NULL; i = i->next) {
				path = i->data;
				get_location_ids(path, loc_arr, model, cwin);
				trash_or_unlink_row(loc_arr, unlink, cwin);

				gtk_tree_path_free(path);

				/* Have to give control to GTK periodically ... */
				/* If gtk_main_quit has been called, return -
				   since main loop is no more. */

				while(gtk_events_pending())
					if (gtk_main_iteration_do(FALSE))
						return;
			}
			g_list_free(list);
			if (loc_arr)
				g_array_free(loc_arr, TRUE);

			query = g_strdup_printf("END;");
			exec_sqlite_query(query, cwin, NULL);

			flush_stale_entries_db(cwin);
			init_library_view(cwin);
		}

	}
}

/***************/
/* Tag Editing */
/***************/

void library_tree_edit_tags(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	struct musicobject *mobj = NULL;
	enum node_type node_type = 0;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GArray *loc_arr = NULL;
	gint sel, location_id, changed = 0;
	gchar *node_data = NULL;

	memset(&otag, 0, sizeof(struct tags));
	memset(&ntag, 0, sizeof(struct tags));

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
			mobj = new_musicobject_from_db(location_id, cwin);
			if (!mobj) {
				g_warning("Unable to retrieve details for "
					  "location_id : %d",
					  location_id);
				goto exit;
			}
			else {
				otag.track_no = mobj->tags->track_no;
				otag.title = mobj->tags->title;
				otag.artist = mobj->tags->artist;
				otag.album = mobj->tags->album;
				otag.genre = mobj->tags->genre;
				otag.comment = mobj->tags->comment;
				otag.year =  mobj->tags->year;

				changed = tag_edit_dialog(&otag, &ntag, mobj->file, cwin);
			}
		}
		else {
			gtk_tree_model_get(model, &iter, L_NODE_DATA, &node_data, -1);

			switch(node_type) {
			case NODE_ARTIST:
				otag.artist = node_data;
				break;
			case NODE_ALBUM:
				otag.album = node_data;
				break;
			case NODE_GENRE:
				otag.genre = node_data;
				break;
			default:
				break;
			}
		changed = tag_edit_dialog(&otag, &ntag, NULL, cwin);
		}
	}

	if (!changed)
		goto exit;

	/* Store the new tags */

	for (i=list; i != NULL; i = i->next) {
		path = i->data;

		/* Form an array of location ids */

		loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
		get_location_ids(path, loc_arr, model, cwin);

		if (!loc_arr) {
			g_array_free(loc_arr, TRUE);
			continue;
		}

		tag_update(loc_arr, NULL, changed, &ntag, cwin);
		g_array_free(loc_arr, TRUE);
	}

	if (changed)
		init_library_view(cwin);
exit:
	/* Cleanup */

	g_free(node_data);

	g_free(ntag.title);
	g_free(ntag.artist);
	g_free(ntag.album);
	g_free(ntag.genre);
	g_free(ntag.comment);

	if (mobj)
		delete_musicobject(mobj);

	for (i=list; i != NULL; i = i->next) {
		path = i->data;
		gtk_tree_path_free(path);
	}
		
	g_list_free(list);
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
	gchar *order_str = NULL;

	cwin->cstate->view_change = TRUE;

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
		order_str = g_strdup("ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case GENRE:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Artist / Album"));
		order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	case GENRE_ARTIST:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Artist"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
		break;
	case GENRE_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Album"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	case GENRE_ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label), _("Genre / Artist / Album"));
		order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
		break;
	default:
		break;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->search_entry), FALSE);

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	g_object_ref(filter_model); 
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);
	gtk_tree_store_clear(GTK_TREE_STORE(model));

	if (cwin->cpref->cur_library_view != FOLDERS) {
		/* Common query for all tag based library views */
		query = g_strdup_printf("SELECT TRACK.title, ALBUM.name, ARTIST.name, GENRE.name, LOCATION.name, LOCATION.id "
					"FROM TRACK, ALBUM, ARTIST, GENRE, LOCATION "
					"WHERE ALBUM.id = TRACK.album AND ARTIST.id = TRACK.artist AND GENRE.id = TRACK.genre AND LOCATION.id = TRACK.location "
					"ORDER BY %s;", order_str);
		g_free(order_str);
			
		exec_sqlite_query(query, cwin, &result);
		for_each_result_row(result, i) {
			add_by_tag(atoi(result.resultp[i+5]), result.resultp[i+4], result.resultp[i+3],
				result.resultp[i+2], result.resultp[i+1], result.resultp[i], cwin, model);
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE)) {
					sqlite3_free_table(result.resultp);
					return;
				}
			}
		}	
	}
	else {
		/* Query for folders view */
		query = g_strdup("SELECT name, id FROM LOCATION ORDER BY name DESC");
		exec_sqlite_query(query, cwin, &result);
		for_each_result_row(result, i) {
			add_folder_file(result.resultp[i], atoi(result.resultp[i+1]), cwin, model);
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE)) {
					sqlite3_free_table(result.resultp);
					return;
				}
			}
		}
	}
	sqlite3_free_table(result.resultp);

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Refresh tag completion entries too */

	refresh_tag_completion_entries(cwin);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->search_entry), TRUE);
	g_signal_emit_by_name (G_OBJECT (cwin->search_entry), "changed", cwin);

	cwin->cstate->view_change = FALSE;
}
