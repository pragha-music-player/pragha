/*************************************************************************/
/* Copyright (C) 2011- matias <mati86dl@gmail.com>			 */
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

void jump_select_row_on_current_playlist (GtkTreeView *jump_tree,
					GtkTreePath *jump_path,
					struct con_win *cwin)
{
	GtkTreeIter iter;
	GtkTreeModel *jump_store;
	gchar *path_string = NULL;
	gint track_i = 0;
	GtkTreePath *path=NULL;
	GtkTreeSelection *selection;
	gint cx, cy;

	GdkRectangle vrect;
	GdkRectangle crect;

	jump_store = gtk_tree_view_get_model (GTK_TREE_VIEW(jump_tree));

	gtk_tree_model_get_iter (jump_store, &iter, jump_path);
	gtk_tree_model_get (jump_store, &iter, 0, &track_i, -1);

	path_string = g_strdup_printf ("%d", track_i -1);
	path = gtk_tree_path_new_from_string (path_string);
	g_free (path_string);

	if (path) {
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(cwin->current_playlist));

		gtk_tree_selection_unselect_all (selection);
		gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection), path);

		gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(cwin->current_playlist), &vrect);
		gtk_tree_view_get_cell_area (GTK_TREE_VIEW(cwin->current_playlist), path, NULL, &crect);

		gtk_tree_view_convert_widget_to_tree_coords (GTK_TREE_VIEW(cwin->current_playlist), crect.x, crect.y, &cx, &cy);

		if ((cy < vrect.y) || (cy + crect.height > vrect.y + vrect.height)) {
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(cwin->current_playlist),
							path, NULL, TRUE, 0.5, 0.0);
		}
		gtk_tree_view_set_cursor (GTK_TREE_VIEW(cwin->current_playlist),
						path, NULL, FALSE);

		gtk_tree_path_free (path);
	}
}

int jump_key_press (GtkWidget *jump_tree, GdkEventKey *event, struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GList *list;

	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK))) {
		return FALSE;
	}
	else if(event->keyval == GDK_KEY_q || event->keyval == GDK_KEY_Q) {
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(cwin->jump_tree));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			jump_select_row_on_current_playlist (GTK_TREE_VIEW (cwin->jump_tree), list->data, cwin);
			gtk_tree_path_free (list->data);
			g_list_free (list);

			toggle_queue_selected_current_playlist (cwin);
		}
		return TRUE;
	}
	return FALSE;
}

gboolean
simple_jump_search_activate_handler (GtkEntry *entry,
				     struct con_win *cwin)
{

	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (cwin->cstate->jump_filter != NULL) {
		g_free (cwin->cstate->jump_filter);
		cwin->cstate->jump_filter = NULL;
	}

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		u_str = g_utf8_strdown (text, -1);
		cwin->cstate->jump_filter = u_str;
	}

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(cwin->jump_model_filter));

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);

	g_free (text);

	return FALSE;
}

static gboolean
do_refilter (struct con_win *cwin)
{
	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(cwin->jump_model_filter));

	cwin->cstate->timeout_id = 0;

	return FALSE;
}

void queue_jump_refilter (struct con_win *cwin)
{
	if(cwin->cstate->timeout_id)
		g_source_remove(cwin->cstate->timeout_id);

	cwin->cstate->timeout_id = g_timeout_add(500, (GSourceFunc)do_refilter, cwin);
}

gboolean simple_jump_search_keyrelease_handler (GtkEntry *entry,
						struct con_win *cwin)
{
	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	if (cwin->cstate->jump_filter != NULL) {
		g_free (cwin->cstate->jump_filter);
		cwin->cstate->jump_filter = NULL;
	}

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		u_str = g_utf8_strdown (text, -1);
		cwin->cstate->jump_filter = u_str;
	}

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);

	if (!cwin->cpref->instant_filter)
		return FALSE;

	queue_jump_refilter(cwin);

	return FALSE;
}

static gboolean
jump_filter_visible_func (GtkTreeModel *model, GtkTreeIter *iter, struct con_win *cwin)
{
	gchar *haystack = NULL, *haystackd = NULL, *needle = NULL;
	gboolean visible = FALSE;

	if(!cwin->cstate->jump_filter)
		return TRUE;

	gtk_tree_model_get(model, iter, 1, &haystack, -1);

	needle = cwin->cstate->jump_filter;

	haystackd = g_utf8_strdown (haystack, -1);

	if (g_strstr_lv (haystack, needle, cwin->cpref->aproximate_search ? 1 : 0))
		visible = TRUE;

	g_free(haystack);
	g_free(haystackd);

	return visible;
}

static void
dialog_jump_to_track_fill_model (GtkListStore *jump_store, struct con_win *cwin)
{
	GtkTreeModel *playlist_model;
	GtkTreeIter playlist_iter, jump_iter;
	struct musicobject *mobj = NULL;
	gchar *ch_title = NULL, *ch_artist = NULL, *ch_album = NULL;
	gchar *track_data_markup = NULL;
	gint track_i = 0;
	gboolean ret;

	playlist_model = gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->current_playlist));

	ret = gtk_tree_model_get_iter_first (playlist_model, &playlist_iter);

	while (ret) {
		gtk_tree_model_get (playlist_model, &playlist_iter, P_MOBJ_PTR, &mobj, -1);

		track_i++;

		ch_title = strlen(mobj->tags->title) ? g_strdup(mobj->tags->title) : get_display_filename (mobj->file, FALSE);
		ch_artist = strlen(mobj->tags->artist) ? g_strdup(mobj->tags->artist) : g_strdup(_("Unknown Artist"));
		ch_album = strlen(mobj->tags->album) ? g_strdup(mobj->tags->album) : g_strdup(_("Unknown Album"));

		track_data_markup = g_markup_printf_escaped ("%s - %s - %s", ch_title, ch_artist, ch_album);

		if (track_data_markup != NULL) {
			gtk_list_store_append (jump_store, &jump_iter);
			gtk_list_store_set (jump_store, &jump_iter,
						0, track_i,
						1, track_data_markup,
						-1);
		}

		g_free (ch_title);
		g_free (ch_artist);
		g_free (ch_album);
		g_free (track_data_markup);

		ret = gtk_tree_model_iter_next(playlist_model, &playlist_iter);
	}
}

void
jump_row_activated_cb (GtkTreeView *jump_tree,
			GtkTreePath *jump_path,
			GtkTreeViewColumn *column,
			GtkWidget *dialog)
{
	gtk_dialog_response (GTK_DIALOG(dialog), GTK_RESPONSE_APPLY);
}

static void
jump_to_track_dialog_response (GtkDialog *dialog,
				gint response,
				struct con_win *cwin)
{
	GList *list;
	GtkTreeSelection *selection;

	switch (response)
	{
	case GTK_RESPONSE_ACCEPT:
		/* Get row selected on jump list and select the row on current playlist. */
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(cwin->jump_tree));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			jump_select_row_on_current_playlist (GTK_TREE_VIEW (cwin->jump_tree), list->data, cwin);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
		toggle_queue_selected_current_playlist (cwin);
		break;
	case GTK_RESPONSE_APPLY:
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(cwin->jump_tree));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			jump_select_row_on_current_playlist (GTK_TREE_VIEW (cwin->jump_tree), list->data, cwin);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	gtk_widget_grab_focus (cwin->current_playlist);
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

void
dialog_jump_to_track (struct con_win *cwin)
{
	GtkWidget *dialog, *scrollwin, *vbox, *search_entry;
	GtkWidget *jump_treeview = NULL;
	GtkListStore *jump_store;
	GtkTreeModel *jump_filter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	jump_store = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", 0, NULL);
	gtk_tree_view_column_set_spacing (column, 4);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "markup", 1, NULL);
	gtk_tree_view_column_set_spacing (column, 4);

	dialog_jump_to_track_fill_model(jump_store, cwin);

	jump_filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(jump_store), NULL);
	g_object_unref(jump_store);
	cwin->jump_model_filter = jump_filter;

	jump_treeview = gtk_tree_view_new_with_model(jump_filter);
	gtk_tree_view_append_column (GTK_TREE_VIEW(jump_treeview), column);
	g_object_unref(G_OBJECT(jump_filter));

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(jump_treeview), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(jump_treeview), FALSE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW(jump_treeview), FALSE);

	cwin->jump_tree = jump_treeview;

	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(jump_filter),
						(GtkTreeModelFilterVisibleFunc)jump_filter_visible_func,
						cwin,
						NULL);

	search_entry = pragha_search_entry_new(cwin);

	g_signal_connect (G_OBJECT(search_entry), "changed",
			 G_CALLBACK(simple_jump_search_keyrelease_handler), cwin);
	g_signal_connect (G_OBJECT(search_entry), "activate",
			 G_CALLBACK(simple_jump_search_activate_handler), cwin);

	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER(scrollwin), jump_treeview);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_SHADOW_IN);

	/* The search dialog */

	dialog = gtk_dialog_new_with_buttons (_("_Search in playlist"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CLOSE,
					     GTK_RESPONSE_CANCEL,
					     NULL);

	gtk_dialog_add_button (GTK_DIALOG (dialog), _("Add to playback queue"), GTK_RESPONSE_ACCEPT);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_JUMP_TO, GTK_RESPONSE_APPLY);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 600, 500);

	/* Add to the dialog's main vbox */

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	gtk_box_pack_start (GTK_BOX(vbox), search_entry, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);
	
	g_signal_connect (jump_treeview, "row-activated",
			G_CALLBACK(jump_row_activated_cb), dialog);
	g_signal_connect (jump_treeview, "key_press_event",
			  G_CALLBACK (jump_key_press), cwin);

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(jump_to_track_dialog_response), cwin);

	gtk_widget_show_all (dialog);
}
