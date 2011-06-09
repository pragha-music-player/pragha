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

void jump_row_activated_cb (GtkTreeView *jump_tree,
			       GtkTreePath *jump_path,
			       GtkTreeViewColumn *column,
			       GtkWidget *dialog)
{
	gtk_dialog_response (GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}

gboolean do_jump_refilter(struct con_win *cwin)
{
	GtkTreeModel *playlist_model;
	GtkTreeIter playlist_iter, jump_iter;
	struct musicobject *mobj = NULL;
	GtkListStore *jump_store;
	gchar *u_str = NULL, *ch_title = NULL, *ch_artist = NULL, *ch_album = NULL, *track_data = NULL;
	gboolean ret;
	gint track_i = 0;

	jump_store = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->jump_tree)));

	g_object_ref (jump_store);
	gtk_tree_view_set_model (GTK_TREE_VIEW(cwin->jump_tree), NULL);
	gtk_list_store_clear (GTK_LIST_STORE(jump_store));

	playlist_model = gtk_tree_view_get_model (GTK_TREE_VIEW(cwin->current_playlist));

	ret = gtk_tree_model_get_iter_first (playlist_model, &playlist_iter);
	while (ret) {
		gtk_tree_model_get (playlist_model, &playlist_iter, P_MOBJ_PTR, &mobj, -1);

		track_i++;

		ch_title = strlen(mobj->tags->title) ? g_strdup(mobj->tags->title) :g_strdup(mobj->file);
		ch_artist = strlen(mobj->tags->artist) ? g_strdup(mobj->tags->artist) : g_strdup(_("Unknown Artist"));
		ch_album = strlen(mobj->tags->album) ? g_strdup(mobj->tags->album) : g_strdup(_("Unknown Album"));

		track_data =  g_strdup_printf ("%s - %s - %s", ch_title, ch_artist, ch_album);

		u_str = g_utf8_strdown (track_data, -1);

		if ( (cwin->cstate->jump_filter == NULL) || g_strrstr (u_str, cwin->cstate->jump_filter)) {
			gtk_list_store_append (jump_store, &jump_iter);
			gtk_list_store_set (jump_store, &jump_iter,
						0, track_i,
						1, track_data,
						-1);
		}
		g_free (ch_title);
		g_free (ch_artist);
		g_free (ch_album);
		g_free (track_data);
		g_free (u_str);

		ret = gtk_tree_model_iter_next(playlist_model, &playlist_iter);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->jump_tree), GTK_TREE_MODEL(jump_store));
	g_object_unref(jump_store);

	return FALSE;
}

gboolean simple_jump_search_keyrelease_handler (GtkEntry *entry,
						struct con_win *cwin)
{

	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (cwin->cstate->timeout_id){
		g_source_remove (cwin->cstate->timeout_id );
		cwin->cstate->timeout_id = 0;
	}
	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		u_str = g_utf8_strdown (text, -1);
		cwin->cstate->jump_filter = u_str;
		cwin->cstate->timeout_id = g_timeout_add (300, (GSourceFunc) do_jump_refilter, cwin );
	}
	else{
		do_jump_refilter (cwin);
	}
	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);
	return FALSE;
}

static void
filter_icon_pressed_cb (GtkEntry       *entry,
		gint            position,
		GdkEventButton *event,
		gpointer        data)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gtk_entry_set_text (entry, "");
		gtk_widget_grab_focus (GTK_WIDGET(entry));
	}
}

/* Search (simple) */

GtkWidget* create_jump_search_bar (struct con_win *cwin)
{
	GtkWidget *search_entry;

	search_entry = gtk_entry_new ();

	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

	gtk_entry_set_icon_sensitive (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, FALSE);

	/* Signal handlers */

	g_signal_connect (G_OBJECT(search_entry),
			"icon-press",
			G_CALLBACK (filter_icon_pressed_cb),
			cwin);
	g_signal_connect (G_OBJECT(search_entry),
			 "changed",
			 G_CALLBACK(simple_jump_search_keyrelease_handler),
			 cwin);

	return search_entry;
}

void
dialog_jump_to_track (struct con_win *cwin)
{
	GtkWidget *scrollwin;
	GtkWidget *vbox, *sep;
	GtkWidget *hbox, *search_label, *search_entry;
	GtkWidget *dialog;
	GtkWidget *jump_treeview = NULL;
	GtkListStore *jump_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GList *list;
	GtkTreeSelection *selection;
	gint result;

	vbox = gtk_vbox_new (FALSE, 5);
	
	jump_store = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);
	jump_treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(jump_store));
	g_object_unref (jump_store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(jump_treeview), TRUE);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(jump_treeview), FALSE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", 0, NULL);
	gtk_tree_view_column_set_spacing (column, 4);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", 1, NULL);
	gtk_tree_view_column_set_spacing (column, 4);
	gtk_tree_view_append_column( GTK_TREE_VIEW(jump_treeview), column);

	gtk_tree_view_set_enable_search (GTK_TREE_VIEW(jump_treeview), FALSE);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

	/* filter box */
	search_label = gtk_label_new (_("Filter: "));
	gtk_label_set_markup_with_mnemonic (GTK_LABEL(search_label), _("_Filter:"));
	gtk_box_pack_start (GTK_BOX(hbox), search_label, FALSE, FALSE, 0);

	search_entry = create_jump_search_bar (cwin);

	gtk_box_pack_start (GTK_BOX(hbox), search_entry, TRUE, TRUE, 3);

	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER(scrollwin), jump_treeview);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

	sep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX(vbox), sep, FALSE, FALSE, 0);

	cwin->jump_tree = jump_treeview;

	if (cwin->cstate->jump_filter != NULL)
		g_free (cwin->cstate->jump_filter);
	cwin->cstate->jump_filter = NULL;

	g_signal_emit_by_name (G_OBJECT (search_entry), "changed", cwin);

	/* The search dialog */

	dialog = gtk_dialog_new_with_buttons (_("_Search in playlist"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CLOSE,
					     GTK_RESPONSE_CANCEL,
					     NULL);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_PLAY, GTK_RESPONSE_ACCEPT);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_JUMP_TO, GTK_RESPONSE_APPLY);

	gtk_container_set_border_width (GTK_CONTAINER(dialog), 10);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 600, 500);

	/* Add to the dialog's main vbox */

	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	
	g_signal_connect (jump_treeview, "row-activated",
			G_CALLBACK(jump_row_activated_cb), dialog);

	gtk_widget_show_all (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (result)
	{
	case GTK_RESPONSE_ACCEPT:
		/* Get row selected on jump list and select the row on current playlist. */
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(jump_treeview));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			jump_select_row_on_current_playlist (GTK_TREE_VIEW (jump_treeview), list->data, cwin);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
		/* Activate row selected current playlist to play track. */
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(cwin->current_playlist));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			gtk_tree_view_row_activated (GTK_TREE_VIEW (cwin->current_playlist), list->data, NULL);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
		break;
	case GTK_RESPONSE_APPLY:
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(jump_treeview));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			jump_select_row_on_current_playlist (GTK_TREE_VIEW (jump_treeview), list->data, cwin);
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
	gtk_widget_destroy (dialog);
}