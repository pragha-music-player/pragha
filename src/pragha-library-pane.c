/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
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

/*
 * Menus definitions
 *
 **/

gchar *library_page_context_menu_xml = "<ui>			\
	<popup>							\
	<menuitem action=\"Expand library\"/>			\
	<menuitem action=\"Collapse library\"/>			\
	<separator/>						\
	<menuitem action=\"folders\"/>				\
	<separator/>						\
	<menuitem action=\"artist\"/>				\
	<menuitem action=\"album\"/>				\
	<menuitem action=\"genre\"/>				\
	<separator/>						\
	<menuitem action=\"artist_album\"/>			\
	<menuitem action=\"genre_artist\"/>			\
	<menuitem action=\"genre_album\"/>			\
	<separator/>						\
	<menuitem action=\"genre_artist_album\"/>		\
	</popup>						\
	</ui>";

GtkActionEntry library_page_context_aentries[] = {
	{"Expand library", GTK_STOCK_ADD, N_("_Expand library"),
	 "", "Expand the library", G_CALLBACK(expand_all_action)},
	{"Collapse library", GTK_STOCK_REMOVE, N_("_Collapse library"),
	 "", "Collapse the library", G_CALLBACK(collapse_all_action)},
	{"folders", GTK_STOCK_REFRESH, N_("Folders structure"),
	 "", "Folders structure", G_CALLBACK(folders_library_tree)},
	{"artist", GTK_STOCK_REFRESH, N_("Artist"),
	 "", "Artist", G_CALLBACK(artist_library_tree)},
	{"album", GTK_STOCK_REFRESH, N_("Album"),
	 "", "Album", G_CALLBACK(album_library_tree)},
	{"genre", GTK_STOCK_REFRESH, N_("Genre"),
	 "", "Genre", G_CALLBACK(genre_library_tree)},
	{"artist_album", GTK_STOCK_REFRESH, N_("Artist / Album"),
	 "", "Artist / Album", G_CALLBACK(artist_album_library_tree)},
	{"genre_album", GTK_STOCK_REFRESH, N_("Genre / Album"),
	 "", "Genre / Album", G_CALLBACK(genre_album_library_tree)},
	{"genre_artist", GTK_STOCK_REFRESH, N_("Genre / Artist"),
	 "", "Genre / Artist", G_CALLBACK(genre_artist_library_tree)},
	{"genre_artist_album", GTK_STOCK_REFRESH, N_("Genre / Artist / Album"),
	 "", "Genre / Artist / Album", G_CALLBACK(genre_artist_album_library_tree)}
};

gchar *playlist_tree_context_menu_xml = "<ui>	\
	<popup>					\
	<menuitem action=\"Add to current playlist\"/>	\
	<menuitem action=\"Replace current playlist\"/>	\
	<menuitem action=\"Replace and play\"/>	\
	<separator/>				\
	<menuitem action=\"Rename\"/>		\
	<menuitem action=\"Delete\"/>		\
	<menuitem action=\"Export\"/>		\
	</popup>				\
	</ui>";

GtkActionEntry playlist_tree_context_aentries[] = {
	{"Add to current playlist", GTK_STOCK_ADD, N_("_Add to current playlist"),
	 "", "Add to current playlist", G_CALLBACK(library_tree_add_to_playlist_action)},
	{"Replace current playlist", NULL, N_("_Replace current playlist"),
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist_action)},
	{"Replace and play", GTK_STOCK_MEDIA_PLAY, N_("Replace and _play"),
	 "", "Replace and play", G_CALLBACK(library_tree_replace_and_play)},
	{"Rename", NULL, N_("Rename"),
	 "", "Rename", G_CALLBACK(playlist_tree_rename)},
	{"Delete", GTK_STOCK_REMOVE, N_("Delete"),
	 "", "Delete", G_CALLBACK(playlist_tree_delete)},
	{"Export", GTK_STOCK_SAVE, N_("Export"),
	 "", "Export", G_CALLBACK(playlist_tree_export)}
};

gchar *library_tree_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"Add to current playlist\"/>	\
	<menuitem action=\"Replace current playlist\"/>	\
	<menuitem action=\"Replace and play\"/>		\
	<separator/>					\
	<menuitem action=\"Edit tags\"/>		\
	<separator/>					\
	<menuitem action=\"Move to trash\"/>		\
	<menuitem action=\"Delete from library\"/>	\
	</popup>					\
	</ui>";

GtkActionEntry library_tree_context_aentries[] = {
	{"Add to current playlist", GTK_STOCK_ADD, N_("_Add to current playlist"),
	 "", "Add to current playlist", G_CALLBACK(library_tree_add_to_playlist_action)},
	{"Replace current playlist", NULL, N_("_Replace current playlist"),
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist_action)},
	{"Replace and play", GTK_STOCK_MEDIA_PLAY, N_("Replace and _play"),
	 "", "Replace and play", G_CALLBACK(library_tree_replace_and_play)},
	{"Edit tags", GTK_STOCK_EDIT, N_("Edit tags"),
	 "", "Edit tags", G_CALLBACK(library_tree_edit_tags)},
	{"Move to trash", "user-trash", N_("Move to _trash"),
	 "", "Move to trash", G_CALLBACK(library_tree_delete_hdd)},
	{"Delete from library", GTK_STOCK_REMOVE, N_("Delete from library"),
	 "", "Delete from library", G_CALLBACK(library_tree_delete_db)}
};

gchar *header_library_tree_context_menu_xml = "<ui>	\
	<popup>						\
	<menuitem action=\"Add to current playlist\"/>	\
	<menuitem action=\"Replace current playlist\"/>	\
	<menuitem action=\"Replace and play\"/>		\
	<separator/>					\
	<menuitem action=\"Rescan library\"/>		\
	<menuitem action=\"Update library\"/>		\
	</popup>					\
	</ui>";

GtkActionEntry header_library_tree_context_aentries[] = {
	{"Add to current playlist", GTK_STOCK_ADD, N_("_Add to current playlist"),
	 "", "Add to current playlist", G_CALLBACK(library_tree_add_to_playlist_action)},
	{"Replace current playlist", NULL, N_("_Replace current playlist"),
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist_action)},
	{"Replace and play", GTK_STOCK_MEDIA_PLAY, N_("Replace and _play"),
	 "", "Replace and play", G_CALLBACK(library_tree_replace_and_play)},
	{"Rescan library", GTK_STOCK_EXECUTE, N_("_Rescan library"),
	 "", "Rescan library", G_CALLBACK(rescan_library_action)},
	{"Update library", GTK_STOCK_EXECUTE, N_("_Update library"),
	 "", "Update library", G_CALLBACK(update_library_action)}
};

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
		      PraghaLibraryPane *clibrary)
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
			   L_PIXBUF, clibrary->pixbuf_dir,
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
		    PraghaLibraryPane *clibrary)
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
			   L_PIXBUF, clibrary->pixbuf_track,
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
                PraghaLibraryPane *clibrary)
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
				add_child_node_folder(model, &iter, p_iter, subpaths[i], clibrary);
			else
				add_child_node_file(model, &iter, p_iter, subpaths[i], location_id, clibrary);
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
                       PraghaLibraryPane *clibrary)
{
	GtkTreeIter iter, iter2, search_iter;
	gchar *node_data = NULL;
	GdkPixbuf *node_pixbuf = NULL;
	enum node_type node_type = 0;
	gint node_level = 0, tot_levels = 0;
	gboolean need_gfree = FALSE;

	/* Iterate through library tree node types */ 
	tot_levels = g_slist_length(clibrary->library_tree_nodes);
	while (node_level < tot_levels) {
		/* Set data to be added to the tree node depending on the type of node */
		node_type = GPOINTER_TO_INT(g_slist_nth_data(clibrary->library_tree_nodes, node_level));
		switch (node_type) {
			case NODE_TRACK:
				node_pixbuf = clibrary->pixbuf_track;
				if (string_is_not_empty(track)) {
					node_data = (gchar *)track;
				}
				else {
					node_data = get_display_filename(location, FALSE);
					need_gfree = TRUE;
				}
				break;
			case NODE_ARTIST:
				node_pixbuf = clibrary->pixbuf_artist;
				node_data = string_is_not_empty(artist) ? (gchar *)artist : _("Unknown Artist");
				break;
			case NODE_ALBUM:
				node_pixbuf = clibrary->pixbuf_album;
				if (pragha_preferences_get_sort_by_year(clibrary->preferences)) {
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
				node_pixbuf = clibrary->pixbuf_genre;
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

static GString *
append_uri_string_list(GtkTreeIter *r_iter,
                       GString *list,
                       GtkTreeModel *model,
                       PraghaLibraryPane *clibrary)
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
				list = append_uri_string_list(&t_iter, list, model, clibrary);

				valid = gtk_tree_model_iter_next(model, &t_iter);
			}
			pragha_process_gtk_events ();
			break;
		case NODE_TRACK:
		case NODE_BASENAME:
			gtk_tree_model_get(model, r_iter, L_LOCATION_ID, &location_id, -1);
			filename = pragha_database_get_filename_from_location_id(clibrary->cdbase, location_id);
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
			     PraghaLibraryPane *clibrary)
{
	GtkTreeIter t_iter, r_iter;
	enum node_type node_type = 0;
	gint location_id;
	gint j = 0;

	clibrary->view_change = TRUE;

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
			get_location_ids(path, loc_arr, model, clibrary);
			gtk_tree_path_free(path);
		}
	}

	clibrary->view_change = FALSE;
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

int library_tree_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin)
{
	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK)))
		return FALSE;
	if (event->keyval == GDK_KEY_Delete){
		library_tree_delete_db(NULL, cwin);
		return TRUE;
	}
	return FALSE;
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

static gboolean
dnd_library_tree_begin(GtkWidget *widget,
                       GdkDragContext *context,
                       PraghaLibraryPane *clibrary)
{
	clibrary->dragging = TRUE;
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

static void
dnd_library_tree_get(GtkWidget *widget,
                     GdkDragContext *context,
                     GtkSelectionData *data,
                     enum dnd_target info,
                     guint time,
                     PraghaLibraryPane *clibrary)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *list = NULL, *l;
	GString *rlist;
	GtkTreeIter s_iter;

	switch(info) {
	case TARGET_REF_LIBRARY:
		rlist = g_string_new (NULL);

		set_watch_cursor (clibrary->widget);
		clibrary->view_change = TRUE;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							clibrary->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		l = list;
		while(l) {
			if(gtk_tree_model_get_iter(model, &s_iter, l->data))
				rlist = append_pragha_uri_string_list(&s_iter, rlist, model);
			gtk_tree_path_free(l->data);
			l = l->next;
		}

		clibrary->view_change = FALSE;
		remove_watch_cursor (clibrary->widget);

		gtk_selection_data_set_pragha_uris(data, rlist);

		g_list_free(list);
		g_string_free (rlist, TRUE);
 		break;
 	case TARGET_URI_LIST:
		rlist = g_string_new (NULL);

		set_watch_cursor (clibrary->widget);
		clibrary->view_change = TRUE;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							clibrary->library_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		l = list;
		while(l) {
			if(gtk_tree_model_get_iter(model, &s_iter, l->data))
				rlist = append_uri_string_list(&s_iter, rlist, model, clibrary);
			l = l->next;
		}

		clibrary->view_change = FALSE;
		remove_watch_cursor (clibrary->widget);

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

static const GtkTargetEntry lentries[] = {
	{"REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY},
	{"text/uri-list", GTK_TARGET_OTHER_APP, TARGET_URI_LIST},
	{"text/plain", GTK_TARGET_OTHER_APP, TARGET_PLAIN_TEXT}
};

static void
library_pane_init_dnd(PraghaLibraryPane *clibrary)
{
	/* Source: Library View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(clibrary->library_tree),
					       GDK_BUTTON1_MASK,
					       lentries,
					       G_N_ELEMENTS(lentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(GTK_WIDGET(clibrary->library_tree)),
	                 "drag-begin",
	                 G_CALLBACK(dnd_library_tree_begin),
	                 clibrary);
	g_signal_connect(G_OBJECT(clibrary->library_tree),
	                 "drag-data-get",
	                 G_CALLBACK(dnd_library_tree_get),
	                 clibrary);
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
	PraghaLibraryPane *clibrary = data;
	gchar *node_data = NULL, *u_str;
	gboolean p_mach;

	/* Mark node and its parents visible if search entry matches.
	   If search entry doesn't match, check if _any_ ancestor has
	   been marked as visible and if so, mark current node as visible too. */

	if (clibrary->filter_entry) {
		gtk_tree_model_get(model, iter, L_NODE_DATA, &node_data, -1);
		u_str = g_utf8_strdown(node_data, -1);
		if (pragha_strstr_lv(u_str, clibrary->filter_entry, clibrary->preferences)) {
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

gboolean do_refilter(PraghaLibraryPane *clibrary)
{
	GtkTreeModel *filter_model;

	/* Remove the model of widget. */
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(clibrary->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), NULL);

	/* Set visibility of rows in the library store. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(clibrary->library_store),
				filter_tree_func,
				clibrary);

	/* Set the model again.*/
	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand all and then reduce properly. */
	gtk_tree_view_expand_all(GTK_TREE_VIEW(clibrary->library_tree));
	gtk_tree_view_map_expanded_rows(GTK_TREE_VIEW(clibrary->library_tree),
		filter_tree_expand_func,
		filter_model);

	clibrary->timeout_id = 0;

	return FALSE;
}

void queue_refilter (PraghaLibraryPane *clibrary)
{
	if(clibrary->timeout_id)
		g_source_remove(clibrary->timeout_id);

	clibrary->timeout_id = g_timeout_add(500, (GSourceFunc)do_refilter, clibrary);
}

gboolean simple_library_search_keyrelease_handler(GtkEntry *entry,
						  PraghaLibraryPane *clibrary)
{
	gchar *text = NULL;
	gboolean has_text;
	
	if (!pragha_preferences_get_instant_search(clibrary->preferences))
		return FALSE;

	if (clibrary->filter_entry != NULL) {
		g_free (clibrary->filter_entry);
		clibrary->filter_entry = NULL;
	}

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		clibrary->filter_entry = g_utf8_strdown (text, -1);

		queue_refilter(clibrary);
	}
	else {
		clear_library_search (clibrary);
	}

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	g_free (text);

	return FALSE;
}

gboolean simple_library_search_activate_handler(GtkEntry *entry,
						PraghaLibraryPane *clibrary)
{
	gchar *text = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (clibrary->filter_entry != NULL) {
		g_free (clibrary->filter_entry);
		clibrary->filter_entry = NULL;
	}

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		clibrary->filter_entry = g_utf8_strdown (text, -1);

		do_refilter (clibrary);
	}
	else {
		clear_library_search (clibrary);
	}
	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	g_free (text);

	return FALSE;
}

void clear_library_search(PraghaLibraryPane *clibrary)
{
	GtkTreeModel *filter_model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean valid;

	/* Remove the model of widget. */
	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(clibrary->library_tree));
	g_object_ref(filter_model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), NULL);

	/* Set all nodes visibles. */
	gtk_tree_model_foreach(GTK_TREE_MODEL(clibrary->library_store),
			       set_all_visible,
			       clibrary);

	/* Set the model again. */
	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	/* Expand the categories. */

	valid = gtk_tree_model_get_iter_first (filter_model, &iter);
	while (valid) {
		path = gtk_tree_model_get_path(filter_model, &iter);
		gtk_tree_view_expand_row (GTK_TREE_VIEW(clibrary->library_tree), path, FALSE);
		gtk_tree_path_free(path);

		valid = gtk_tree_model_iter_next(filter_model, &iter);
	}
}

/********************************/
/* Library view order selection */
/********************************/

static void
library_pane_update_style (PraghaLibraryPane *clibrary)
{
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
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Artist"));
			break;
		case ALBUM:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_ALBUM));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Album"));
			break;
		case GENRE:
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_GENRE));
			clibrary->library_tree_nodes =
				g_slist_append(clibrary->library_tree_nodes,
				               GINT_TO_POINTER(NODE_TRACK));
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Genre"));
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
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Artist / Album"));
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
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Genre / Artist"));
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
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Genre / Album"));
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
			gtk_label_set_text (GTK_LABEL(clibrary->combo_order_label), _("Genre / Artist / Album"));
			break;
		default:
			break;
	}
}

static void
library_pane_change_style (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;

	library_pane_update_style(cwin->clibrary);
	library_pane_view_reload(cwin->clibrary);
}


/*********************************/
/* Functions to reload playlist. */
/*********************************/

static void
library_view_append_playlists(GtkTreeModel *model,
                              GtkTreeIter *p_iter,
                              PraghaLibraryPane *clibrary)
{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *playlist = NULL;
	GtkTreeIter iter;

	sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE DESC";
	statement = pragha_database_create_statement (clibrary->cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		playlist = pragha_prepared_statement_get_string(statement, 0);

		library_store_prepend_node(model,
		                           &iter,
		                           p_iter,
		                           clibrary->pixbuf_track,
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
                           PraghaLibraryPane *clibrary)
{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *radio = NULL;
	GtkTreeIter iter;

	sql = "SELECT name FROM RADIO ORDER BY name COLLATE NOCASE DESC";
	statement = pragha_database_create_statement (clibrary->cdbase, sql);
	while (pragha_prepared_statement_step (statement)) {
		radio = pragha_prepared_statement_get_string(statement, 0);

		library_store_prepend_node(model,
		                           &iter,
		                           p_iter,
		                           clibrary->pixbuf_track,
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
                                  PraghaLibraryPane *clibrary)

{
	PraghaPreparedStatement *statement;
	const gchar *sql = NULL, *filepath = NULL;
	gchar *mask = NULL;
	GtkTreeIter iter, *f_iter;
	GSList *list = NULL, *library_dir = NULL;

	library_dir =
		pragha_preferences_get_filename_list(clibrary->preferences,
			                             GROUP_LIBRARY,
			                             KEY_LIBRARY_DIR);

	for(list = library_dir ; list != NULL ; list=list->next) {
		/*If no need to fuse folders, add headers and set p_iter */
		if(!pragha_preferences_get_fuse_folders(clibrary->preferences)) {
			gtk_tree_store_append(GTK_TREE_STORE(model),
					      &iter,
					      p_iter);
			gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
					   L_PIXBUF, clibrary->pixbuf_dir,
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
		statement = pragha_database_create_statement (clibrary->cdbase, sql);
		mask = g_strconcat (list->data, "%", NULL);
		pragha_prepared_statement_bind_string (statement, 1, mask);
		while (pragha_prepared_statement_step (statement)) {
			filepath = pragha_prepared_statement_get_string(statement, 0) + strlen(list->data) + 1;
			add_folder_file(model,
			                filepath,
			                pragha_prepared_statement_get_int(statement, 1),
			                f_iter,
			                clibrary);

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
                                PraghaLibraryPane *clibrary)
{
	PraghaPreparedStatement *statement;
	gchar *order_str = NULL, *sql = NULL;

	/* Get order needed to sqlite query. */
	switch(pragha_preferences_get_library_style(clibrary->preferences)) {
		case FOLDERS:
			break;
		case ARTIST:
			order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case ALBUM:
			if (pragha_preferences_get_sort_by_year(clibrary->preferences))
				order_str = g_strdup("YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			else
				order_str = g_strdup("ALBUM.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case GENRE:
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case ARTIST_ALBUM:
			if (pragha_preferences_get_sort_by_year(clibrary->preferences))
				order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			else
				order_str = g_strdup("ARTIST.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			break;
		case GENRE_ARTIST:
			order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ARTIST.name COLLATE NOCASE DESC, TRACK.title COLLATE NOCASE DESC");
			break;
		case GENRE_ALBUM:
			if (pragha_preferences_get_sort_by_year(clibrary->preferences))
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, YEAR.year COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			else
				order_str = g_strdup("GENRE.name COLLATE NOCASE DESC, ALBUM.name COLLATE NOCASE DESC, TRACK.track_no COLLATE NOCASE DESC");
			break;
		case GENRE_ARTIST_ALBUM:
			if (pragha_preferences_get_sort_by_year(clibrary->preferences))
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

	statement = pragha_database_create_statement (clibrary->cdbase, sql);
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
		                       clibrary);

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

void
library_pane_view_reload(PraghaLibraryPane *clibrary)
{
	GtkTreeModel *model, *filter_model;
	GtkTreeIter iter;

	clibrary->view_change = TRUE;

	set_watch_cursor (clibrary->widget);

	filter_model = gtk_tree_view_get_model(GTK_TREE_VIEW(clibrary->library_tree));
	model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(filter_model));

	g_object_ref(filter_model);

	gtk_widget_set_sensitive(GTK_WIDGET(clibrary->search_entry), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(clibrary->library_tree), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), NULL);

	gtk_tree_store_clear(GTK_TREE_STORE(model));

	/* Playlists.*/

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Playlists"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	library_view_append_playlists(model, &iter, clibrary);

	/* Radios. */

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Radios"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	library_view_append_radios(model, &iter, clibrary);

	/* Add library header */

	gtk_tree_store_append(GTK_TREE_STORE(model),
			      &iter,
			      NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			   L_PIXBUF, clibrary->pixbuf_dir,
			   L_NODE_DATA, _("Library"),
			   L_NODE_TYPE, NODE_CATEGORY,
			   -1);

	if (pragha_preferences_get_library_style(clibrary->preferences) == FOLDERS) {
		library_view_complete_folder_view(model, &iter, clibrary);
	}
	else {
		library_view_complete_tags_view(model, &iter, clibrary);
	}

	/* Refresh tag completion entries, sensitive, set model and filter */

	/* TODO: Move to database?
	 *refresh_tag_completion_entries(cwin); */

	gtk_widget_set_sensitive(GTK_WIDGET(clibrary->search_entry), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(clibrary->library_tree), TRUE);

	gtk_tree_view_set_model(GTK_TREE_VIEW(clibrary->library_tree), filter_model);
	g_object_unref(filter_model);

	g_signal_emit_by_name (G_OBJECT (clibrary->search_entry), "activate", clibrary);

	remove_watch_cursor (clibrary->widget);

	clibrary->view_change = FALSE;
}

static void
update_library_playlist_changes(PraghaDatabase *database, struct con_win *cwin)
{
	/*
	 * Rework to olny update Playlist and radio tree!!!.
	 **/
	library_pane_view_reload(cwin->clibrary);
	update_menu_playlist_changes(cwin);
}

/*************************************/
/* All menu handlers of library pane */
/*************************************/

/*
 * library_page_context_menu calbacks
 */
void expand_all_action(GtkAction *action, struct con_win *cwin)
{
	gtk_tree_view_expand_all(GTK_TREE_VIEW(cwin->clibrary->library_tree));
}

void collapse_all_action(GtkAction *action, struct con_win *cwin)
{
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(cwin->clibrary->library_tree));
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

/*
 * playlist_tree_context_menu calbacks
 */
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

void playlist_tree_rename(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list;
	gchar *playlist = NULL, *n_playlist = NULL;
	gint node_type;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		path = list->data;
		if (gtk_tree_path_get_depth(path) > 1) {
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, L_NODE_DATA, &playlist, -1);

			n_playlist = rename_playlist_dialog (playlist, cwin);
			if(n_playlist != NULL) {
				gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);

				if(node_type == NODE_PLAYLIST)
					pragha_database_update_playlist_name (cwin->cdbase, playlist, n_playlist);
				else if (node_type == NODE_RADIO)
					pragha_database_update_radio_name (cwin->cdbase, playlist, n_playlist);

				pragha_database_change_playlists_done(cwin->cdbase);

				g_free(n_playlist);
			}
			g_free(playlist);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void playlist_tree_delete(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	gchar *playlist;
	gint node_type;
	gboolean removed = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		/* Delete selected playlists */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			if (gtk_tree_path_get_depth(path) > 1) {
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);
				gtk_tree_model_get(model, &iter, L_NODE_DATA,
						   &playlist, -1);

				if(delete_existing_item_dialog(playlist, cwin)) {
					if(node_type == NODE_PLAYLIST) {
						delete_playlist_db (playlist, cwin->cdbase);
					}
					else if (node_type == NODE_RADIO) {
						delete_radio_db(playlist, cwin->cdbase);
					}
					removed = TRUE;
				}
				g_free(playlist);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}

	if(removed)
		pragha_database_change_playlists_done(cwin->cdbase);
}

void playlist_tree_export(GtkAction *action, struct con_win *cwin)
{
	GIOChannel *chan = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GError *err = NULL;
	gint cnt;
	gchar *filename = NULL, *playlist = NULL, *playlistpath = NULL;
	gint node_type;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->clibrary->library_tree));
	cnt = (gtk_tree_selection_count_selected_rows(selection));

	list = gtk_tree_selection_get_selected_rows(selection, NULL);
	path = list->data;

	/* If only is 'Playlist' node, just return, else get playlistname. */
	if ((cnt == 1) && (gtk_tree_path_get_depth(path) == 1)) {
		gtk_tree_path_free(path);
		g_list_free(list);
		return;
	}
	else {
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model, &iter, L_NODE_DATA, &playlistpath, -1);

		gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);
		if(node_type != NODE_PLAYLIST) {
			gtk_tree_path_free(path);
			g_list_free(list);
			return;
		}
	}

	filename = playlist_export_dialog_get_filename(playlistpath, cwin->mainwindow);

	if (!filename)
		goto exit;

	chan = create_m3u_playlist(filename);
	if (!chan) {
		g_warning("Unable to create M3U playlist file: %s", filename);
		goto exit;
	}

	set_watch_cursor (cwin->mainwindow);

	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	if (list) {
		/* Export all the playlists to the given file */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			if (gtk_tree_path_get_depth(path) > 1) {
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, L_NODE_DATA,
						   &playlist, -1);
				if (save_m3u_playlist(chan, playlist,
						      filename, cwin->cdbase) < 0) {
					g_warning("Unable to save M3U playlist: %s",
						  filename);
					g_free(playlist);
					goto exit_list;
				}
				g_free(playlist);
			}
			gtk_tree_path_free(path);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ()) {
				g_list_free(list);
				return;
			}
		}
	}

	if (chan) {
		if (g_io_channel_shutdown(chan, TRUE, &err) != G_IO_STATUS_NORMAL) {
			g_critical("Unable to save M3U playlist: %s", filename);
			g_error_free(err);
			err = NULL;
		} else {
			CDEBUG(DBG_INFO, "Saved M3U playlist: %s", filename);
		}
		g_io_channel_unref(chan);
	}

exit_list:
	remove_watch_cursor (cwin->mainwindow);

	if (list)
		g_list_free(list);
exit:
	g_free(playlistpath);
	g_free(filename);
}

/*
 * library_tree_context_menu_xml calbacks
 */
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
				if (pragha_preferences_get_sort_by_year(cwin->clibrary->preferences)) {
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
		get_location_ids(path, loc_arr, model, cwin->clibrary);
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
	library_pane_view_reload(cwin->clibrary);

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
				get_location_ids(path, loc_arr, model, cwin->clibrary);
				trash_or_unlink_row(loc_arr, unlink, cwin);

				/* Have to give control to GTK periodically ... */
				if (pragha_process_gtk_events ())
					return;
			}
			db_commit_transaction(cwin->clibrary->cdbase);

			g_array_free(loc_arr, TRUE);

			flush_stale_entries_db(cwin->clibrary->cdbase);
			library_pane_view_reload(cwin->clibrary);
		}

		g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
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
			library_pane_view_reload(cwin->clibrary);
		}

		g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);
	}
}

/**************************************/
/* Construction menus of library pane */
/**************************************/

static GtkUIManager*
create_playlist_tree_context_menu(struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Playlist Tree Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       playlist_tree_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create playlist tree context menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     playlist_tree_context_aentries,
				     G_N_ELEMENTS(playlist_tree_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager*
create_library_tree_context_menu(GtkWidget *library_tree,
                                 struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Library Tree Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       library_tree_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create library tree context menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     library_tree_context_aentries,
				     G_N_ELEMENTS(library_tree_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager*
create_header_library_tree_context_menu(GtkWidget *library_tree,
                                        struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Header Library Tree Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       header_library_tree_context_menu_xml,
					       -1, &error)) {
		g_critical("(%s): Unable to create header library tree context menu, err : %s",
			   __func__, error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     header_library_tree_context_aentries,
				     G_N_ELEMENTS(header_library_tree_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager*
create_library_page_context_menu(struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Library Page Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       library_page_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create library page context menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     library_page_context_aentries,
				     G_N_ELEMENTS(library_page_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

/********************************/
/* Construction of library pane */
/********************************/

static GtkWidget* create_library_tree(PraghaLibraryPane *clibrary, struct con_win *cwin)
{
	GtkWidget *library_tree;
	GtkTreeModel *library_filter_tree;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	/* Create the tree store */

	store = gtk_tree_store_new(N_L_COLUMNS,
				   GDK_TYPE_PIXBUF, /* Pixbuf */
				   G_TYPE_STRING,   /* Node */
				   G_TYPE_INT,      /* Node type : Artist / Album / Track */
				   G_TYPE_INT,      /* Location id (valid only for Track) */
				   G_TYPE_BOOLEAN,  /* Flag to save mach when filtering */
				   G_TYPE_BOOLEAN); /* Row visibility */


	/* Create the filter model */

	library_filter_tree = gtk_tree_model_filter_new(GTK_TREE_MODEL(store), NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(library_filter_tree),
						 L_VISIBILE);
	/* Create the tree view */

	library_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(library_filter_tree));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(library_tree), FALSE);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(library_tree), TRUE);

	/* Selection mode is multiple */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(library_tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create column and cell renderers */

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", L_PIXBUF,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "text", L_NODE_DATA,
					    NULL);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(library_tree), column);

	/* Connect signals and create right click popup menu */

	g_signal_connect(G_OBJECT(library_tree), "row-activated",
			 G_CALLBACK(library_tree_row_activated_cb), cwin);
	g_signal_connect (G_OBJECT (library_tree), "key-press-event",
			  G_CALLBACK(library_tree_key_press), cwin);

	/* Create right click popup menu */

	cwin->library_tree_context_menu = create_library_tree_context_menu(library_tree,
									   cwin);

	cwin->header_library_tree_context_menu = create_header_library_tree_context_menu(library_tree,
											 cwin);

	/* Signal handler for right-clicking and selection */
 
 	g_signal_connect(G_OBJECT(GTK_WIDGET(library_tree)), "button-press-event",
			 G_CALLBACK(library_tree_button_press_cb), cwin);

	g_signal_connect(G_OBJECT(GTK_WIDGET(library_tree)), "button-release-event",
			 G_CALLBACK(library_tree_button_release_cb), cwin);

	/* Save references and configure dnd */

	clibrary->library_store = store;
	clibrary->library_tree = library_tree;

	library_pane_init_dnd(clibrary);

	g_object_unref(library_filter_tree);
	
	return library_tree;
}

static void
pragha_sidebar_close (GtkWidget *widget, struct con_win *cwin)
{
	GtkAction *action;
	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Lateral panel");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);
}

GtkWidget *
create_close_button(PraghaLibraryPane *clibrary, struct con_win *cwin)
{
	GtkWidget *button, *image;
    
	button = gtk_button_new ();
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_container_add (GTK_CONTAINER (button), image);

	g_signal_connect(G_OBJECT (button),
			 "clicked",
			 G_CALLBACK(pragha_sidebar_close),
			 cwin);

	return button;
}

GtkWidget *
create_library_view_options_combo(PraghaLibraryPane *clibrary, struct con_win *cwin)
{
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label_order, *arrow;

	hbox = gtk_hbox_new(FALSE, 0);

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

	label_order = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(label_order), 0, 0.5);
	arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);

	gtk_box_pack_start(GTK_BOX(hbox),
			   label_order,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox),
			   arrow,
			   FALSE,
			   FALSE,
			   0);

	gtk_container_add (GTK_CONTAINER(button), hbox);

	/* Create library page context menu */

	cwin->library_page_context_menu = create_library_page_context_menu(cwin);

	/* Create right click popup menu */

	cwin->playlist_tree_context_menu = create_playlist_tree_context_menu(cwin);

	g_signal_connect(G_OBJECT(button),
			 "button-press-event",
			 G_CALLBACK(library_page_right_click_cb),
			 cwin);

	gtk_widget_set_tooltip_text(GTK_WIDGET(button), _("Options of the library"));

	clibrary->combo_order = button;
	clibrary->combo_order_label = label_order;

	return button;
}

GtkWidget *
create_sidebar_header(PraghaLibraryPane *clibrary, struct con_win *cwin)
{
	GtkWidget *hbox, *combo, *close_button;

	hbox = gtk_hbox_new(FALSE, 0);

	combo = create_library_view_options_combo(clibrary, cwin);
	close_button = create_close_button(clibrary, cwin);

	gtk_box_pack_start(GTK_BOX(hbox),
			   combo,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox),
			   close_button,
			   FALSE,
			   FALSE,
			   0);

	return hbox;
}

static GtkWidget*
library_pane_create_widgets(PraghaLibraryPane *clibrary, struct con_win *cwin)
{
	GtkWidget *vbox_lib;
	GtkWidget *header, *search_entry, *library_tree_scroll, *library_tree;

	vbox_lib = gtk_vbox_new(FALSE, 2);

	/* The header to select order and close the pane */
	header = create_sidebar_header (clibrary, cwin);

	/* The filter entry */
	search_entry = pragha_search_entry_new(cwin);

	/* The scrroll and library tree. */
	library_tree_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(library_tree_scroll),
					GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(library_tree_scroll), 2);

	library_tree = create_library_tree(clibrary, cwin);

	/* Package all */

	gtk_box_pack_start(GTK_BOX(vbox_lib),
	                   header,
	                   FALSE,
	                   FALSE,
	                   0);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
	                   search_entry,
	                   FALSE,
	                   FALSE,
	                   0);
	gtk_container_add(GTK_CONTAINER(library_tree_scroll),
	                  library_tree);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
	                   library_tree_scroll,
	                   TRUE,
	                   TRUE,
	                   0);

	/* Conect signals */
	g_signal_connect (G_OBJECT(search_entry),
	                  "changed",
	                  G_CALLBACK(simple_library_search_keyrelease_handler),
	                  clibrary);
	g_signal_connect (G_OBJECT(search_entry),
	                  "activate",
	                  G_CALLBACK(simple_library_search_activate_handler),
	                  clibrary);

	/* Save references */
	clibrary->search_entry = search_entry;

	return vbox_lib;
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
	g_object_unref(librarypane->library_store);

	g_slist_free(librarypane->library_tree_nodes);

	g_slice_free(PraghaLibraryPane, librarypane);
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
pragha_library_pane_init_view(PraghaLibraryPane *clibrary)
{
	library_pane_update_style(clibrary);
	library_pane_view_reload(clibrary);
}

PraghaLibraryPane *
pragha_library_pane_new(struct con_win *cwin)
{
	PraghaLibraryPane *clibrary;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	clibrary = g_slice_new0(PraghaLibraryPane);

	clibrary->cdbase = pragha_database_get();
	clibrary->preferences = pragha_preferences_get();

	clibrary->widget = library_pane_create_widgets(clibrary, cwin);

	clibrary->filter_entry = NULL;
	clibrary->dragging = FALSE;
	clibrary->view_change = TRUE;
	clibrary->timeout_id = 0;
	clibrary->library_tree_nodes = NULL;

	pragha_library_pane_init_pixbufs(clibrary);

	/* Conect signals */

	g_signal_connect (clibrary->cdbase, "PlaylistsChanged", G_CALLBACK (update_library_playlist_changes), cwin);
	g_signal_connect(clibrary->preferences, "notify::library-style", G_CALLBACK (library_pane_change_style), cwin);
	g_object_bind_property (clibrary->preferences, "lateral-panel", clibrary->widget, "visible", binding_flags);

	return clibrary;
}
