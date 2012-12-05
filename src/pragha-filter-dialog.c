/*************************************************************************/
/* Copyright (C) 2011-2012 matias <mati86dl@gmail.com>			 */
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

typedef struct {
	GtkWidget *filter_view;
	GtkTreeModel *filter_model;
	gchar *filter_string;
	guint timeout_id;
	PraghaPlaylist *cplaylist;
	PraghaPreferences *preferences;
} PraghaFilterDialog;

static void
pragha_filter_dialog_select_row_on_current_playlist(GtkTreeView *fliter_view,
						    GtkTreePath *filter_path,
						    PraghaFilterDialog *fdialog)
{
	GtkTreeIter iter;
	GtkTreeModel *filter_model;
	gint track_i = 0;

	filter_model = gtk_tree_view_get_model (GTK_TREE_VIEW(fliter_view));

	if(gtk_tree_model_get_iter (filter_model, &iter, filter_path)) {
		gtk_tree_model_get (filter_model, &iter, 0, &track_i, -1);
		select_numered_path_of_current_playlist(fdialog->cplaylist, track_i - 1, TRUE);
	}
}

static int
pragha_filter_dialog_key_press (GtkWidget *fliter_view,
				GdkEventKey *event,
				PraghaFilterDialog *fdialog)
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
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(fliter_view));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			pragha_filter_dialog_select_row_on_current_playlist (GTK_TREE_VIEW (fliter_view), list->data, fdialog);
			gtk_tree_path_free (list->data);
			g_list_free (list);

			toggle_queue_selected_current_playlist (fdialog->cplaylist);
		}
		return TRUE;
	}
	return FALSE;
}

static gboolean
simple_filter_search_activate_handler(GtkEntry *entry,
				    PraghaFilterDialog *fdialog)
{

	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (fdialog->filter_string != NULL) {
		g_free (fdialog->filter_string);
		fdialog->filter_string = NULL;
	}

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		u_str = g_utf8_strdown (text, -1);
		fdialog->filter_string = u_str;
	}

	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(fdialog->filter_model));

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);

	g_free (text);

	return FALSE;
}

static gboolean
do_filter_dialog_refilter (PraghaFilterDialog *fdialog)
{
	gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(fdialog->filter_model));

	fdialog->timeout_id = 0;

	return FALSE;
}

static void
queue_filter_dialog_refilter (PraghaFilterDialog *fdialog)
{
	if(fdialog->timeout_id)
		g_source_remove(fdialog->timeout_id);

	fdialog->timeout_id = g_timeout_add(500, (GSourceFunc)do_filter_dialog_refilter, fdialog);
}

static gboolean
simple_filter_search_keyrelease_handler(GtkEntry *entry,
					PraghaFilterDialog *fdialog)
{
	gchar *text = NULL;
	gchar *u_str = NULL;
	gboolean has_text;

	if (fdialog->filter_string != NULL) {
		g_free (fdialog->filter_string);
		fdialog->filter_string = NULL;
	}

	has_text = gtk_entry_get_text_length (GTK_ENTRY(entry)) > 0;

	if (has_text) {
		text = gtk_editable_get_chars (GTK_EDITABLE(entry), 0, -1);
		u_str = g_utf8_strdown (text, -1);
		fdialog->filter_string = u_str;
	}

	gtk_entry_set_icon_sensitive (GTK_ENTRY(entry),
				GTK_ENTRY_ICON_SECONDARY,
				has_text);

	if (!pragha_preferences_get_instant_search(fdialog->preferences))
		return FALSE;

	queue_filter_dialog_refilter(fdialog);

	return FALSE;
}

static gboolean
filter_model_visible_func (GtkTreeModel *model, GtkTreeIter *iter, PraghaFilterDialog *fdialog)
{
	gchar *haystack = NULL, *haystackd = NULL, *needle = NULL;
	gboolean approximate, visible = FALSE;

	if(!fdialog->filter_string)
		return TRUE;

	gtk_tree_model_get(model, iter, 1, &haystack, -1);

	needle = fdialog->filter_string;

	haystackd = g_utf8_strdown (haystack, -1);

	approximate = pragha_preferences_get_approximate_search(fdialog->preferences);

	if(g_strstr_lv(haystackd, needle, approximate ? 1 : 0))
		visible = TRUE;

	g_free(haystack);
	g_free(haystackd);

	return visible;
}

static void
pragha_filter_dialog_fill_model (GtkListStore *filter_model, PraghaPlaylist *cplaylist)
{
	GtkTreeIter filter_iter;
	PraghaMusicobject *mobj = NULL;
	gchar *ch_title = NULL, *ch_artist = NULL, *ch_album = NULL;
	gchar *track_data_markup = NULL;
	gint track_i = 0;
	GList *list = NULL, *i;

	list = pragha_playlist_get_mobj_list(cplaylist);

	track_i = pragha_playlist_get_no_tracks(cplaylist);

	if(list != NULL) {
		for (i=list; i != NULL; i = i->next) {
			mobj = i->data;

			ch_title = strlen(pragha_musicobject_get_title(mobj)) ? g_strdup(pragha_musicobject_get_title(mobj)) : get_display_filename (pragha_musicobject_get_file(mobj), FALSE);
			ch_artist = strlen(pragha_musicobject_get_artist(mobj)) ? g_strdup(pragha_musicobject_get_artist(mobj)) : g_strdup(_("Unknown Artist"));
			ch_album = strlen(pragha_musicobject_get_album(mobj)) ? g_strdup(pragha_musicobject_get_album(mobj)) : g_strdup(_("Unknown Album"));

			track_data_markup = g_markup_printf_escaped ("%s - %s - %s", ch_title, ch_artist, ch_album);

			if (track_data_markup != NULL) {
				gtk_list_store_prepend (filter_model, &filter_iter);
				gtk_list_store_set (filter_model, &filter_iter,
							0, track_i,
							1, track_data_markup,
							-1);
			}

			track_i--;

			g_free (ch_title);
			g_free (ch_artist);
			g_free (ch_album);
			g_free (track_data_markup);
		}
		g_list_free(list);
	}
}

static void
pragha_filter_dialog_activated_cb(GtkTreeView *fliter_view,
				  GtkTreePath *filter_path,
				  GtkTreeViewColumn *column,
				  GtkWidget *dialog)
{
	gtk_dialog_response (GTK_DIALOG(dialog), GTK_RESPONSE_APPLY);
}

static void
pragha_filter_dialog_response(GtkDialog *dialog,
			      gint response,
			      PraghaFilterDialog *fdialog)
{
	GList *list;
	GtkTreeSelection *selection;

	switch (response)
	{
	case GTK_RESPONSE_ACCEPT:
		/* Get row selected on jump list and select the row on current playlist. */
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(fdialog->filter_view));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			pragha_filter_dialog_select_row_on_current_playlist (GTK_TREE_VIEW (fdialog->filter_view), list->data, fdialog);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
		toggle_queue_selected_current_playlist (fdialog->cplaylist);
		break;
	case GTK_RESPONSE_APPLY:
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(fdialog->filter_view));
		list = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (list) {
			pragha_filter_dialog_select_row_on_current_playlist (GTK_TREE_VIEW (fdialog->filter_view), list->data, fdialog);
			gtk_tree_path_free (list->data);
			g_list_free (list);
		}
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	g_free(fdialog->filter_string);
	g_object_unref(G_OBJECT(fdialog->preferences));
	g_slice_free(PraghaFilterDialog, fdialog);

	gtk_widget_grab_focus (pragha_playlist_get_view(fdialog->cplaylist));
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

void
pragha_filter_dialog (struct con_win *cwin)
{
	GtkWidget *dialog, *scrollwin, *vbox, *search_entry;
	GtkWidget *filter_view = NULL;
	GtkListStore *filter_store;
	GtkTreeModel *filter_model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	PraghaFilterDialog *fdialog;
	fdialog = g_slice_new0(PraghaFilterDialog);

	/* Crete the filter entry */

	search_entry = pragha_search_entry_new(cwin);

	g_signal_connect (G_OBJECT(search_entry), "changed",
			 G_CALLBACK(simple_filter_search_keyrelease_handler), fdialog);
	g_signal_connect (G_OBJECT(search_entry), "activate",
			 G_CALLBACK(simple_filter_search_activate_handler), fdialog);

	/* Create the view */

	filter_store = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", 0, NULL);
	gtk_tree_view_column_set_spacing (column, 4);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "markup", 1, NULL);
	gtk_tree_view_column_set_spacing (column, 4);

	/* Fill the filter tree view with current playlist */

	pragha_filter_dialog_fill_model(filter_store, cwin->cplaylist);

	filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(filter_store), NULL);
	g_object_unref(filter_store);

	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filter_model),
						(GtkTreeModelFilterVisibleFunc)filter_model_visible_func,
						fdialog,
						NULL);

	/* Create the tree view */

	filter_view = gtk_tree_view_new_with_model(filter_model);
	gtk_tree_view_append_column (GTK_TREE_VIEW(filter_view), column);
	g_object_unref(G_OBJECT(filter_model));

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(filter_view), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(filter_view), FALSE);
	gtk_tree_view_set_enable_search (GTK_TREE_VIEW(filter_view), FALSE);

	/* Store references */

	fdialog->filter_view = filter_view;
	fdialog->filter_model = filter_model;
	fdialog->filter_string = NULL;
	fdialog->timeout_id = 0;
	fdialog->cplaylist = cwin->cplaylist;
	fdialog->preferences = pragha_preferences_get();

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

	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER(scrollwin), filter_view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrollwin),
					GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

	/* Connect signals */
	
	g_signal_connect (filter_view, "row-activated",
			G_CALLBACK(pragha_filter_dialog_activated_cb), dialog);
	g_signal_connect (filter_view, "key_press_event",
			  G_CALLBACK (pragha_filter_dialog_key_press), fdialog);

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(pragha_filter_dialog_response), fdialog);

	gtk_widget_show_all (dialog);
}
