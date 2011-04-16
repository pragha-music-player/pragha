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

#include "pragha.h"

/* Return the path of the node that matches given data */

static GtkTreePath* find_root_node(gchar *node_data,
				   GtkTreeModel *model)
{
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gchar *data = NULL;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	do {
		gtk_tree_model_get(model, &iter, L_NODE_DATA, &data, -1);
		if (data && !g_ascii_strcasecmp(data, node_data)) {
			g_free(data);
			path = gtk_tree_model_get_path(model, &iter);
			break;
		}
		g_free(data);
	} while (gtk_tree_model_iter_next(model, &iter));

	return path;
}

/* Return the path of the node of r_iter's children that matches given data */

static GtkTreePath* find_child_node(gchar *node_data,
				    GtkTreeIter r_iter,
				    GtkTreeModel *model)
{
	GtkTreePath *path = NULL;
	gchar *data = NULL;
	gint i = 0;
	GtkTreeIter t_iter;

	while (gtk_tree_model_iter_nth_child(model, &t_iter, &r_iter, i++)) {
		gtk_tree_model_get(model, &t_iter, L_NODE_DATA, &data, -1);
		if (data && !g_ascii_strcasecmp(data, node_data)) {
			g_free(data);
			path = gtk_tree_model_get_path(model, &t_iter);
			break;
		}
		g_free(data);
	}

	return path;
}

/* Return node data at the given level */

static gchar* choose_node_data(gchar *location,
			       gchar *genre,
			       gchar *artist,
			       gchar *album,
			       gchar *track,
			       gint level,
			       struct con_win *cwin)
{
	gchar *node;

	node = g_slist_nth_data(cwin->cpref->library_tree_nodes, level);

	if (!g_ascii_strcasecmp(P_FOLDER_STR, node))
		return location;
	else if (!g_ascii_strcasecmp(P_GENRE_STR, node))
		return genre;
	else if (!g_ascii_strcasecmp(P_ALBUM_STR, node))
		return album;
	else if (!g_ascii_strcasecmp(P_ARTIST_STR, node))
		return artist;
	else if (!g_ascii_strcasecmp(P_TITLE_STR, node))
		return track;
	else
		return NULL;
}

/* Return node type at the given level */

static enum node_type choose_node_type(gint level, struct con_win *cwin)
{
	gchar *node;

	node = g_slist_nth_data(cwin->cpref->library_tree_nodes, level);

	if (!g_ascii_strcasecmp(P_FOLDER_STR, node))
		return NODE_FOLDER;
	else if (!g_ascii_strcasecmp(P_BASENAME_STR, node))
		return NODE_BASENAME;
	else if (!g_ascii_strcasecmp(P_GENRE_STR, node))
		return NODE_GENRE;
	else if (!g_ascii_strcasecmp(P_ALBUM_STR, node))
		return NODE_ALBUM;
	else if (!g_ascii_strcasecmp(P_ARTIST_STR, node))
		return NODE_ARTIST;
	else if (!g_ascii_strcasecmp(P_TITLE_STR, node))
		return NODE_TRACK;
	else
		return -1;
}

/* Return node pixbuf at the given level */

static GdkPixbuf* choose_node_pixbuf(gint level, struct con_win *cwin)
{
	gchar *node;

	node = g_slist_nth_data(cwin->cpref->library_tree_nodes, level);

	if (!g_ascii_strcasecmp(P_FOLDER_STR, node))
		return cwin->pixbuf->pixbuf_dir;
	else if (!g_ascii_strcasecmp(P_BASENAME_STR, node))
		return cwin->pixbuf->pixbuf_file;
	else if (!g_ascii_strcasecmp(P_GENRE_STR, node))
		return cwin->pixbuf->pixbuf_genre;
	else if (!g_ascii_strcasecmp(P_ALBUM_STR, node))
		return cwin->pixbuf->pixbuf_album;
	else if (!g_ascii_strcasecmp(P_ARTIST_STR, node))
		return cwin->pixbuf->pixbuf_artist;
	else if (!g_ascii_strcasecmp(P_TITLE_STR, node))
		return cwin->pixbuf->pixbuf_track;
	else
		return NULL;
}

/* Add a new entry to the library tree */

static void add_entry_library(gint location_id,
			      gchar *location,
			      gchar *genre,
			      gchar *artist,
			      gchar *album,
			      gchar *track,
			      GtkTreeModel *model,
			      struct con_win *cwin)
{
	gboolean u_flag = FALSE;
	gchar *node_data;
	gint node_level = 0, tot_levels = 0;
	GdkPixbuf *node_pixbuf;
	GtkTreeIter iter = {0}, p_iter = {0};
	GtkTreePath *p_path = NULL;
	enum node_type node_type = 0;

	tot_levels = g_slist_length(cwin->cpref->library_tree_nodes);

	node_data = choose_node_data(location, genre, artist,
				     album, track, node_level, cwin);
	node_type = choose_node_type(node_level, cwin);
	node_pixbuf = choose_node_pixbuf(node_level, cwin);

	/* Only root node can be of type NODE_FOLDER */

	if (node_type == NODE_FOLDER) {
		node_data = get_containing_folder(location);
	} else {
		if (G_UNLIKELY(g_utf8_strlen(node_data, -1) == 0)) {
			if(!g_strncasecmp("Artist",g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level),4))
					node_data = g_strdup_printf(_("Unknown Artist"));
			else if(!g_strncasecmp("Album",g_slist_nth_data(cwin->cpref->library_tree_nodes,node_level),4))
					node_data = g_strdup_printf(_("Unknown Album"));
			else if(!g_strncasecmp("Genre", g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level), 4))
						node_data = g_strdup_printf(_("Unknown Genre"));
			u_flag = TRUE;
		}
	}

	/* Check if root node is already present */

	p_path = find_root_node(node_data, model);
	if (!p_path) {
		gtk_tree_store_append(GTK_TREE_STORE(model),
				      &iter,
				      NULL);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				   L_PIXBUF, node_pixbuf,
				   L_NODE_DATA, node_data,
				   L_NODE_TYPE, node_type,
				   L_LOCATION_ID, 0,
				   L_VISIBILE, TRUE,
				   -1);
		p_iter = iter;
	}
	else {
		gtk_tree_model_get_iter(model, &p_iter, p_path);
		gtk_tree_path_free(p_path);
		p_path = NULL;
	}

	if (node_type == NODE_FOLDER || u_flag) {
		g_free(node_data);
		u_flag = FALSE;
	}

	node_level++;

	/* Run through the remaining child nodes */

	while (node_level < tot_levels) {
		node_data = choose_node_data(location, genre, artist,
					     album, track, node_level, cwin);
		node_type = choose_node_type(node_level, cwin);
		node_pixbuf = choose_node_pixbuf(node_level, cwin);


		if (node_type == NODE_BASENAME)
			node_data = g_path_get_basename(location);
		else {
			if (G_UNLIKELY(g_utf8_strlen(node_data, -1) == 0)) {
				if (node_type == NODE_TRACK)
					node_data = g_path_get_basename(location);
				else {
				if(!g_strncasecmp("Artist",g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level),4))
						node_data = g_strdup_printf(_("Unknown Artist"));
				else if(!g_strncasecmp("Album",g_slist_nth_data(cwin->cpref->library_tree_nodes,node_level),4))
						node_data = g_strdup_printf(_("Unknown Album"));
				else if(!g_strncasecmp("Genre", g_slist_nth_data(cwin->cpref->library_tree_nodes, node_level), 4))
						node_data = g_strdup_printf(_("Unknown Genre"));
				}
				u_flag = TRUE;
			}
		}

		/* Check if the child node is already present
		   For leaves (title/basename), append the entry
		   irrespective of duplicate presence */

		if ((node_type == NODE_BASENAME) || (node_type == NODE_TRACK))
			p_path = NULL;
		else
			p_path = find_child_node(node_data, p_iter, model);

		if (!p_path) {
			gtk_tree_store_append(GTK_TREE_STORE(model),
					      &iter,
					      &p_iter);
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
					   L_PIXBUF, node_pixbuf,
					   L_NODE_DATA, node_data,
					   L_NODE_TYPE, node_type,
					   L_LOCATION_ID,
					   ((node_type == NODE_TRACK) ||
					    (node_type == NODE_BASENAME)) ?
					   location_id : 0,
					   L_VISIBILE, TRUE,
					   -1);
			p_iter = iter;
		}
		else {
			gtk_tree_model_get_iter(model, &p_iter, p_path);
			gtk_tree_path_free(p_path);
			p_path = NULL;
		}

		if (node_type == NODE_BASENAME || u_flag) {
			g_free(node_data);
			u_flag = FALSE;
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

static void delete_row_from_hdd(GtkTreePath *path, GtkTreeModel *model,
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
		if (delete_location_hdd(location_id, cwin) == 0)
			delete_location_db(location_id, cwin);
	}

	/* For all other node types do a recursive deletion */

	while (gtk_tree_model_iter_nth_child(model, &t_iter, &r_iter, j++)) {
		gtk_tree_model_get(model, &t_iter, L_NODE_TYPE, &node_type, -1);
		if ((node_type == NODE_TRACK) || (node_type == NODE_BASENAME)) {
			gtk_tree_model_get(model, &t_iter,
					   L_LOCATION_ID, &location_id, -1);
			if (delete_location_hdd(location_id, cwin) == 0)
				delete_location_db(location_id, cwin);
		}
		else {
			path = gtk_tree_model_get_path(model, &t_iter);
			delete_row_from_hdd(path, model, cwin);
			gtk_tree_path_free(path);
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
						 FALSE);
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

gboolean library_tree_right_click_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin)
{
	GtkWidget *popup_menu;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	gboolean many_selected = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));

	switch(event->button) {
	case 2:
		if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(cwin->library_tree),
						  event->x, event->y,
						  &path, NULL, NULL, NULL)){
			if (!gtk_tree_selection_path_is_selected(selection, path)){
				model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));

				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
			}
			library_tree_add_to_playlist(cwin);
			gtk_tree_path_free(path);
		}
		else gtk_tree_selection_unselect_all(selection);

		break;
	case 3:
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
	return many_selected;
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

/* Callback for DnD signal 'drag-data-get' */

void dnd_library_tree_get(GtkWidget *widget,
			  GdkDragContext *context,
			  GtkSelectionData *data,
			  enum dnd_target info,
			  guint time,
			  struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model, *filter_model;
	GList *list = NULL, *l;
	GArray *loc_arr;

	switch(info) {
	case TARGET_LOCATION_ID:
		filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						       cwin->library_tree));
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

void folder_file_library_tree(GtkAction *action, struct con_win *cwin)
{
	free_str_list(cwin->cpref->library_tree_nodes);
	cwin->cpref->library_tree_nodes = NULL;

	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_FOLDER_STR));
	cwin->cpref->library_tree_nodes = g_slist_append(cwin->cpref->library_tree_nodes,
							 g_strdup(P_BASENAME_STR));
	cwin->cpref->cur_library_view = FOLDER_FILE;

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Folder / File"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Artist"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Album"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Genre"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Artist / Album"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Genre / Album"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Genre / Artist"));

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

	gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),
			_("Genre / Artist / Album"));

	init_library_view(cwin);
}

/*****************/
/* Menu handlers */
/*****************/

void library_tree_replace_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model, *filter_model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
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

void library_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin)
{
	library_tree_add_to_playlist(cwin);
}
void library_tree_add_to_playlist(struct con_win *cwin)
{
	GtkTreeModel *model, *filter_model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
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
	GtkTreeModel *model, *filter_model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;
	gchar *query;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {

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
	}

	flush_stale_entries_db(cwin);
	init_library_view(cwin);
}

void library_tree_delete_hdd(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model, *filter_model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GList *list, *i;
	gchar *query;

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {

		/* Delete all the rows */

		query = g_strdup_printf("BEGIN;");
		exec_sqlite_query(query, cwin, NULL);

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			delete_row_from_hdd(path, model, cwin);
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
	}

	flush_stale_entries_db(cwin);
	init_library_view(cwin);
}

/***************/
/* Tag Editing */
/***************/

void library_tree_edit_tags(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	struct musicobject *mobj = NULL;
	enum node_type node_type = 0;
	GtkTreeModel *model, *filter_model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GArray *loc_arr = NULL;
	gint sel, location_id, changed = 0;
	gchar *node_data;
	gchar *title, *artist, *album, *genre, *year;

	node_data = title = artist = album = genre = year = NULL;
	memset(&otag, 0, sizeof(struct tags));
	memset(&ntag, 0, sizeof(struct tags));

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
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
			} else {
				otag.track_no = mobj->tags->track_no;
				otag.title = mobj->tags->title;
				otag.artist = mobj->tags->artist;
				otag.album = mobj->tags->album;
				otag.genre = mobj->tags->genre;
				otag.year =  mobj->tags->year;
			}
		} else {
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
		}
	}

	/* Prompt the user for tag changes */

	changed = tag_edit_dialog(&otag, &ntag, cwin);
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

	g_free(year);
	g_free(node_data);

	g_free(ntag.title);
	g_free(ntag.artist);
	g_free(ntag.album);
	g_free(ntag.genre);

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

	const gchar *order_str[] = {
		"LOCATION.name ASC",
		"ARTIST.name ASC, TRACK.title ASC",
		"ALBUM.name ASC, TRACK.title ASC",
		"GENRE.name ASC, TRACK.title ASC",
		"ARTIST.name ASC, ALBUM.name ASC, TRACK.track_no ASC",
		"GENRE.name ASC, ARTIST.name ASC, TRACK.title ASC",
		"GENRE.name ASC, ALBUM.name ASC, TRACK.track_no ASC",
		"GENRE.name ASC, ARTIST.name ASC, ALBUM.name ASC, TRACK.track_no ASC"};

	cwin->cstate->view_change = TRUE;

	switch(cwin->cpref->cur_library_view) {
	case FOLDER_FILE:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Folder / File"));
		break;
	case ARTIST:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Artist"));		
		break;
	case ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Album"));	
		break;
	case GENRE:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Genre"));
		break;
	case ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Artist / Album"));
		break;
	case GENRE_ARTIST:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Genre / Artist"));
		break;
	case GENRE_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Genre / Album"));
		break;
	case GENRE_ARTIST_ALBUM:
		gtk_label_set_text (GTK_LABEL(cwin->combo_order_label),_("Genre / Artist / Album"));
		break;
	default:
		break;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->combo_order), FALSE);
	gtk_editable_set_editable (GTK_EDITABLE(cwin->search_entry), FALSE);

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	g_object_ref(filter_model); 
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), NULL);
	gtk_tree_store_clear(GTK_TREE_STORE(model));

	/* Query and insert entries */

	query = g_strdup_printf("SELECT TRACK.title, ALBUM.name, ARTIST.name,"
				"GENRE.name, LOCATION.name, LOCATION.id "
				"FROM TRACK, ALBUM, ARTIST, GENRE, LOCATION "
				"WHERE ALBUM.id = TRACK.album AND "
				"ARTIST.id = TRACK.artist AND "
				"GENRE.id = TRACK.genre AND "
				"LOCATION.id = TRACK.location "
				"ORDER BY %s;",
				order_str[cwin->cpref->cur_library_view]);
	exec_sqlite_query(query, cwin, &result);

	for_each_result_row(result, i) {
		add_entry_library(atoi(result.resultp[i+5]),
				  result.resultp[i+4],
				  result.resultp[i+3],
				  result.resultp[i+2],
				  result.resultp[i+1],
				  result.resultp[i],
				  model,
				  cwin);

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE)) {
				sqlite3_free_table(result.resultp);
				return;
			}
		}
	}
	sqlite3_free_table(result.resultp);

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Refresh tag completion entries too */

	refresh_tag_completion_entries(cwin);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->combo_order), TRUE);
	gtk_editable_set_editable (GTK_EDITABLE(cwin->search_entry), TRUE);

	cwin->cstate->view_change = FALSE;
}
