/*************************************************************************/
/* Copyright (C) 2007-2009 sujtih <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

/* Accepts only basename */

static gchar* form_new_file_tree_pwd(const gchar *new_dir, struct con_win *cwin)
{
	gchar *ab;

	/* Check if going back in directory tree */

	if (!g_ascii_strncasecmp(new_dir, UP_DIR, 3))
		ab = g_path_get_dirname(cwin->cstate->file_tree_pwd);
	else {

		/* Check if we have reached root */

		if (!g_ascii_strcasecmp(cwin->cstate->file_tree_pwd, "/"))
			ab = g_strdup_printf("%s%s",
					     cwin->cstate->file_tree_pwd,
					     new_dir);
		else
			ab = g_strdup_printf("%s/%s",
					     cwin->cstate->file_tree_pwd,
					     new_dir);
	}

	return ab;
}

/* Clear the current playlist if 'clear' is TRUE and append a new track */

static gboolean file_tree_add_file(GtkTreePath *path,
				   gboolean clear,
				   struct con_win *cwin)
{
	GError *error = NULL;
	struct musicobject *mobj = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *file = NULL, *ab_file = NULL, *f_file = NULL;
	gboolean ret = FALSE;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, F_NAME, &file, -1);

	if (file) {
		f_file = g_filename_from_utf8(file, -1, NULL, NULL, &error);
		if (!f_file) {
			g_warning("Unable to get filename from "
				  "UTF-8 string: %s",
				  file);
			g_error_free(error);
			g_free(file);
			goto exit;
		}

		ab_file = g_strdup_printf("%s/%s", cwin->cstate->file_tree_pwd,
					  f_file);
		if (is_playable_file(ab_file)) {
			mobj = new_musicobject_from_file(ab_file);
			if (mobj) {
				if (clear)
					clear_current_playlist(NULL, cwin);
				append_current_playlist(mobj, cwin);
				ret = TRUE;
			}
			CDEBUG(DBG_VERBOSE, "Add file from file_tree: %s",
			       ab_file);
		}
		g_free(ab_file);
		g_free(file);
		g_free(f_file);
	}
exit:
	return ret;
}

static void file_tree_add_recur_row(GtkTreePath *path, struct con_win *cwin)
{
	GError *error = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *file = NULL, *ab_file = NULL, *f_file = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, F_NAME, &file, -1);

	if (file) {
		f_file = g_filename_from_utf8(file, -1, NULL, NULL, &error);
		if (!f_file) {
			g_warning("Unable to get filename from "
				  "UTF-8 string: %s",
				  file);
			g_error_free(error);
			g_free(file);
			return;
		}

		if (is_base_dir_and_accessible(f_file, cwin)) {
			ab_file = g_strdup_printf("%s/%s",
						  cwin->cstate->file_tree_pwd,
						  f_file);
			__recur_add(ab_file, cwin);
			g_free(ab_file);
		}
		g_free(file);
		g_free(f_file);
	}
}

static void file_tree_add_non_recur_row(GtkTreePath *path, struct con_win *cwin)
{
	GError *error = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *file = NULL, *ab_file = NULL, *f_file = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, F_NAME, &file, -1);

	if (file) {
		f_file = g_filename_from_utf8(file, -1, NULL, NULL, &error);
		if (!f_file) {
			g_warning("Unable to get filename from "
				  "UTF-8 string: %s",
				  file);
			g_error_free(error);
			g_free(file);
			return;
		}

		if (is_base_dir_and_accessible(f_file, cwin)) {
			ab_file = g_strdup_printf("%s/%s",
						  cwin->cstate->file_tree_pwd,
						  f_file);
			__non_recur_add(ab_file, FALSE, cwin);
			g_free(ab_file);
		}
		g_free(file);
		g_free(f_file);
	}
}

static void get_filenames(GtkTreePath *path,
			  GArray *file_arr,
			  GtkTreeModel *model,
			  struct con_win *cwin)
{
	gboolean add = FALSE;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file, *ab_dir, *bname, *fname;
	GtkTreeIter iter;
	GError *error = NULL;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, F_NAME, &bname, -1);

	if (bname) {
		fname = g_filename_from_utf8(bname, -1, NULL, NULL, &error);
		if (!fname) {
			g_warning("Unable to get filename from "
				  "UTF-8 string: %s",
				  bname);
			g_error_free(error);
			g_free(bname);
			return;
		}


		if (is_base_dir_and_accessible(fname, cwin)) {
			ab_dir = g_strdup_printf("%s/%s",
						 cwin->cstate->file_tree_pwd,
						 fname);
			dir = g_dir_open(ab_dir, 0, &error);
			if (!dir) {
				g_critical("Unable to open library : %s",
					   ab_dir);
				g_error_free(error);
				return;
			}

			next_file = g_dir_read_name(dir);
			while (next_file) {
				ab_file = g_strconcat(ab_dir, "/", next_file, NULL);
				if (!g_file_test(ab_file, G_FILE_TEST_IS_DIR)) {
					if (is_playable_file(ab_file)) {
						CDEBUG(DBG_VERBOSE,
						       "Play file from file_tree: %s",
						       ab_file);
						g_array_append_val(file_arr, ab_file);
						add = TRUE;
					}
				}
				if (!add)
					g_free(ab_file);

				add = FALSE;
				next_file = g_dir_read_name(dir);
			}
			g_dir_close(dir);
			g_free(ab_dir);
		}
		else {
			ab_file = g_strdup_printf("%s/%s",
						  cwin->cstate->file_tree_pwd,
						  fname);
			if (is_playable_file(ab_file)) {
				CDEBUG(DBG_VERBOSE, "Play file from file_tree: %s",
				       ab_file);
				g_array_append_val(file_arr, ab_file);
			}
		}
		g_free(bname);
		g_free(fname);
	}
}

void __non_recur_add(gchar *dir_name, gboolean init, struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (!g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (mobj)
					append_current_playlist(mobj, cwin);
				CDEBUG(DBG_VERBOSE, "Play file from file_tree: %s",
				       ab_file);
			}

		/* Have to give control to GTK periodically ... */

		if (!init) {
			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE))
					return;
			}
		}

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

void __recur_add(gchar *dir_name, struct con_win *cwin)
{
	struct musicobject *mobj = NULL;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		return;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			__recur_add(ab_file, cwin);
		else {
			if (is_playable_file(ab_file)) {
				mobj = new_musicobject_from_file(ab_file);
				if (mobj) {
					append_current_playlist(mobj, cwin);
					CDEBUG(DBG_VERBOSE,
					       "Play file from file_tree: %s",
					       ab_file);
				}
			}
		}

		/* Have to give control to GTK periodically ... */
		/* If gtk_main_quit has been called, return -
		   since main loop is no more. */

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE))
				return;
		}

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
}

/* Handler for double clicking on a row */

void file_tree_row_activated_cb(GtkTreeView *file_tree,
				GtkTreePath *path,
				GtkTreeViewColumn *column,
				struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *new_dir = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, F_NAME, &new_dir, -1);
	if (new_dir)
		update_file_tree(new_dir, cwin);
	else
		return;

	g_free(new_dir);
}

/* Handler for right click menu */

gboolean file_tree_right_click_cb(GtkWidget *widget,
				  GdkEventButton *event,
				  struct con_win *cwin)
{
	GtkTreeSelection *selection = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkWidget *popup_menu = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;
	gchar *file, *ab_file, *f_file;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));

	switch(event->button) {
	case 3:
		if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
			gtk_tree_model_get(model, &iter, F_NAME, &file, -1);

			if (file) {
				f_file = g_filename_from_utf8(file, -1, NULL, NULL, &error);
				if (!f_file) {
					g_warning("Unable to get filename "
						  "from UTF-8 string: %s",
						  file);
					g_error_free(error);
					g_free(file);
					ret = TRUE;
					break;
				}

				ab_file = g_strdup_printf("%s/%s",
							  cwin->cstate->file_tree_pwd,
							  f_file);
				if (g_file_test(ab_file, G_FILE_TEST_IS_DIR) &&
				    g_ascii_strncasecmp(file, UP_DIR, 3))
					popup_menu = gtk_ui_manager_get_widget(
						cwin->file_tree_dir_context_menu,
						"/popup");
				else if (is_playable_file(ab_file))
					popup_menu = gtk_ui_manager_get_widget(
						cwin->file_tree_file_context_menu,
						"/popup");
				g_free(ab_file);
				g_free(file);
				g_free(f_file);
			}
			if (popup_menu)
				gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
					       event->button, event->time);
		}
		break;
	default:
		ret = FALSE;
		break;
	}

	return ret;
}

/* Handler for 'Play' on a file */

void file_tree_play(GtkAction *action, struct con_win *cwin)
{
	GtkTreeSelection *selection = NULL;
	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		if (file_tree_add_file(path, TRUE, cwin))
			play_first_current_playlist(cwin);
		gtk_tree_path_free(path);
	}
}

/* Handler for 'Enqueue' on  a file */

void file_tree_enqueue(GtkAction *action, struct con_win *cwin)
{
	GtkTreeSelection *selection = NULL;
	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		file_tree_add_file(path, FALSE, cwin);
		gtk_tree_path_free(path);
	}
}

/* Handler for 'Enqueue(Recursive)' on a directory */

void file_tree_enqueue_recur(GtkAction *action, struct con_win *cwin)
{
	GtkTreeSelection *selection = NULL;
	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		file_tree_add_recur_row(path, cwin);
		gtk_tree_path_free(path);
	}
}

/* Handler for 'Enqueue(Non-Recursive)' on a directory */

void file_tree_enqueue_non_recur(GtkAction *action, struct con_win *cwin)
{
	GtkTreeSelection *selection = NULL;
	GtkTreePath *path = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		file_tree_add_non_recur_row(path, cwin);
		gtk_tree_path_free(path);
	}
}

/* Callback for DnD signal 'drag-data-get' */

void dnd_file_tree_get(GtkWidget *widget,
		       GdkDragContext *context,
		       GtkSelectionData *data,
		       guint info,
		       guint time,
		       struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GList *list = NULL, *l;
	GArray *file_arr;

	switch(info) {
	case TARGET_FILENAME:
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->file_tree));
		list = gtk_tree_selection_get_selected_rows(selection, &model);

		/* No selections */

		if (!list) {
			gtk_selection_data_set(data, data->type, 8, NULL, 0);
			break;
		}

		/* Form an array of file names */

		file_arr = g_array_new(TRUE, TRUE, sizeof(gchar *));
		l = list;

		while(l) {
			get_filenames(l->data, file_arr, model, cwin);
			gtk_tree_path_free(l->data);
			l = l->next;
		}

		gtk_selection_data_set(data,
				       data->type,
				       8,
				       (guchar *)&file_arr,
				       sizeof(GArray *));

		CDEBUG(DBG_VERBOSE, "Fill DnD data, selection: %p, file_arr: %p",
		       data->data, file_arr);

		/* Cleanup */

		g_list_free(list);

		break;
	default:
		g_warning("Unknown DND type");
	}

}

/* Accepts only basename for new_dir */

void populate_file_tree(const gchar *new_dir, struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GError *error = NULL;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file, *ab_new_dir, *u_file = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));

	/* Create an iterator and add UP directory row */

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   F_PIXBUF, cwin->pixbuf->pixbuf_dir,
			   F_NAME, UP_DIR,
			   -1);

	/* Form the absolute path of the new directory and
	   update cstate->file_tree_pwd */

	if (new_dir) {
		ab_new_dir = form_new_file_tree_pwd(new_dir, cwin);
		g_free(cwin->cstate->file_tree_pwd);
		cwin->cstate->file_tree_pwd = g_strdup(ab_new_dir);
	}
	else
		ab_new_dir = g_strdup(cwin->cstate->file_tree_pwd);

	dir = g_dir_open(ab_new_dir, 0, &error);
	if (!dir) {
		g_critical("Unable to open directory: %s", ab_new_dir);
		goto exit;
	}

	/* Iterate over all the files in the new directory and
	   add them to the model */

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strdup_printf("%s/%s", cwin->cstate->file_tree_pwd, next_file);
		if (is_hidden_file(ab_file) && !cwin->cpref->show_hidden_files) { 
			g_free(ab_file);
			next_file = g_dir_read_name(dir);
			if (!next_file)
				break;
			else
				continue;
		}

		/* Convert to UTF-8 before adding to the model */
		u_file = g_filename_to_utf8(next_file, -1, NULL, NULL, &error);
		if (!u_file) {
			g_warning("Unable to convert file to UTF-8: %s",
				  next_file);
			g_error_free(error);
			error = NULL;
			g_free(ab_file);
			next_file = g_dir_read_name(dir);
			if (!next_file)
				break;
			else
				continue;
		}

		/* Store a directory */
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR)) {
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					   F_PIXBUF, cwin->pixbuf->pixbuf_dir,
					   F_NAME, u_file,
					   F_FILE_TYPE, F_TYPE_DIR,
					   -1);
		}
		/* Store a file */
		else if (is_playable_file(ab_file)) {
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
					   F_PIXBUF, cwin->pixbuf->pixbuf_file,
					   F_NAME, u_file,
					   F_FILE_TYPE, F_TYPE_FILE,
					   -1);
		}

		g_free(ab_file);
		g_free(u_file);

		next_file = g_dir_read_name(dir);
	}

	g_dir_close(dir);
exit:
	g_free(ab_new_dir);
}

void update_file_tree(gchar *new_dir, struct con_win *cwin)
{
	GError *error = NULL;
	GtkTreeModel *model;
	gchar *file = NULL, *f_dir = NULL;
	struct musicobject *mobj = NULL;

	f_dir = g_filename_from_utf8(new_dir, -1, NULL, NULL, &error);
	if (!f_dir) {
		g_warning("Unable to get filename from UTF-8 string: %s",
			  new_dir);
		g_error_free(error);
		error = NULL;
		return;
	}

	if (is_base_dir_and_accessible(f_dir, cwin)) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->file_tree));
		gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(
						GTK_TREE_VIEW(cwin->file_tree)));
		gtk_list_store_clear(GTK_LIST_STORE(model));
		populate_file_tree(f_dir, cwin);
	}
	else {
		file = g_strdup_printf("%s/%s", cwin->cstate->file_tree_pwd, f_dir);
		if (is_playable_file(file)) {
			mobj = new_musicobject_from_file(file);
			if (mobj)
				append_current_playlist(mobj, cwin);
			CDEBUG(DBG_VERBOSE, "Filetree Play : %s", file);
		}
		g_free(file);
	}

	g_free(f_dir);
}

gint file_tree_sort_func(GtkTreeModel *model, GtkTreeIter *a,
			 GtkTreeIter *b, gpointer data)
{
	gchar *file_a = NULL, *file_b = NULL;
	gboolean is_dir_a, is_dir_b;
	enum filetree_node type_a, type_b;
	gint ret = 0;

	gtk_tree_model_get(model, a, F_NAME, &file_a, -1);
	gtk_tree_model_get(model, b, F_NAME, &file_b, -1);

	gtk_tree_model_get(model, a, F_FILE_TYPE, &type_a, -1);
	gtk_tree_model_get(model, b, F_FILE_TYPE, &type_b, -1);

	if (!compare_utf8_str(file_a, UP_DIR)) {
		ret = -1;
		goto exit;
	} else if (!compare_utf8_str(file_b, UP_DIR)) {
		ret = 1;
		goto exit;
	}

	is_dir_a = (type_a == F_TYPE_DIR) ? TRUE : FALSE;
	is_dir_b = (type_b == F_TYPE_DIR) ? TRUE : FALSE;

	if (is_dir_a && !is_dir_b) {
		ret = -1;				/* Dir has pref over File */
	}
	else if (is_dir_b && !is_dir_a) {
		ret = 1;				/*	-do-		  */	
	}
	else {
		ret = compare_utf8_str(file_a, file_b); /* Dir/Dir  or  File/File */
	}
exit:
	g_free(file_a);
	g_free(file_b);

	return ret;
}
