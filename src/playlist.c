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

/* Build a dialog to get a new playlist name */

static gchar*
get_playlist_dialog(enum playlist_mgmt *choice,
		    enum playlist_mgmt type,
		    struct con_win *cwin)
{
	gchar **playlists, *playlist = NULL;
	gint result, i=0;
	GtkWidget *dialog, *radio_new, *radio_add;
	GtkWidget *hbox, *vbox1, *vbox2;
	GtkWidget *entry;
	GtkWidget *combo_add;

	/* Retrieve list of playlist names from DB */

	playlists = get_playlist_names_db(cwin->cdbase);

	/* Create dialog window */

	hbox = gtk_hbox_new(TRUE, 2);
	vbox1 = gtk_vbox_new(TRUE, 2);
	vbox2 = gtk_vbox_new(TRUE, 2);
	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	combo_add = gtk_combo_box_text_new();
	radio_new = gtk_radio_button_new_with_label(NULL, _("Save as a new playlist"));
	radio_add = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(radio_new), _("Append to an existing playlist"));

	if (playlists) {
		while (playlists[i]) {
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_add),
						  playlists[i]);
			i++;
		}
		g_strfreev(playlists);
	}
	else {
		gtk_widget_set_sensitive(combo_add, FALSE);
		gtk_widget_set_sensitive(radio_add, FALSE);
	}

	dialog = gtk_dialog_new_with_buttons(NULL,
			     GTK_WINDOW(cwin->mainwindow),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
			     NULL);

	if(type == SAVE_COMPLETE)
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save playlist"));
	else
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save selection"));

	gtk_box_pack_start(GTK_BOX(vbox1), radio_new, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), radio_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox2), combo_add, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 2);

	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(entry));

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Export"), GTK_RESPONSE_HELP);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		/* Get playlist name */
		/* Store a copy because the dialog box is destroyed on return */

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_new))) {
			playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			*choice = NEW_PLAYLIST;
		}
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_add))) {
			playlist = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_add));
			*choice = APPEND_PLAYLIST;
		}
		break;
	case GTK_RESPONSE_HELP:
		playlist = g_strdup(_("New playlist"));
		*choice = EXPORT_PLAYLIST;
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return playlist;
}

/* Get a new playlist name that is not reserved */

gchar*
get_playlist_name(struct con_win *cwin,
		  enum playlist_mgmt type,
		  enum playlist_mgmt *choice)
{
	gchar *playlist = NULL;
	enum playlist_mgmt sel = 0;

	do {
		playlist = get_playlist_dialog(&sel, type, cwin);
		if (playlist && !g_ascii_strcasecmp(playlist, SAVE_PLAYLIST_STATE)) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				_("<b>con_playlist</b> is a reserved playlist name"));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_free(playlist);
			continue;
		} else {
			break;
		}
	} while (1);

	*choice = sel;
	return playlist;
}

static gboolean overwrite_existing_playlist(const gchar *playlist,
					    struct con_win *cwin)
{
	gboolean choice = FALSE;
	gint ret;
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Do you want to overwrite the playlist: %s ?"),
				playlist);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));

	switch(ret) {
	case GTK_RESPONSE_YES:
		choice = TRUE;
		break;
	case GTK_RESPONSE_NO:
		choice = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);
	return choice;
}

static GIOChannel* create_m3u_playlist(gchar *file, struct con_win *cwin)
{
	GIOChannel *chan = NULL;
	GIOStatus status;
	GError *err = NULL;
	gsize bytes = 0;

	chan = g_io_channel_new_file(file, "w+", &err);
	if (!chan) {
		g_critical("Unable to create M3U playlist IO channel: %s", file);
		goto exit_failure;
	}

	status = g_io_channel_write_chars(chan, "#EXTM3U\n", -1, &bytes, &err);
	if (status != G_IO_STATUS_NORMAL) {
		g_critical("Unable to write to M3U playlist: %s", file);
		goto exit_failure;
	}

	CDEBUG(DBG_INFO, "Created M3U playlist file: %s", file);
	return chan;

exit_failure:
	g_error_free(err);
	err = NULL;

	if (chan) {
		g_io_channel_shutdown(chan, FALSE, &err);
		g_io_channel_unref(chan);
	}

	return NULL;
}

static gint
save_mobj_list_to_m3u_playlist(GList *list, GIOChannel *chan, gchar *filename)
{
	gchar *str = NULL, *uri = NULL, *base_m3u = NULL, *base = NULL;
	PraghaMusicobject *mobj = NULL;
	GIOStatus status;
	gsize bytes = 0;
	GError *err = NULL;
	GList *i;
	gint ret = 0;

	base_m3u = get_display_filename(filename, TRUE);

	/* Export all selected tracks to the given file */
	for (i=list; i != NULL; i = i->next) {
		mobj = i->data;
		if (pragha_musicobject_get_file_type(mobj) != FILE_CDDA &&
		    pragha_musicobject_get_file_type(mobj) != FILE_HTTP) {
			base = get_display_filename(pragha_musicobject_get_file(mobj), TRUE);

			if (g_ascii_strcasecmp(base_m3u, base) == 0)
				uri = get_display_filename(pragha_musicobject_get_file(mobj), FALSE);
			else
				uri = g_strdup(pragha_musicobject_get_file(mobj));

			/* Format: "#EXTINF:seconds, title" */
			str = g_strdup_printf("#EXTINF:%d,%s\n%s\n",
					      pragha_musicobject_get_length(mobj),
					      pragha_musicobject_get_title(mobj),
					      uri);

			status = g_io_channel_write_chars(chan, str, -1, &bytes, &err);
			if (status != G_IO_STATUS_NORMAL) {
				g_critical("Unable to write to M3U playlist: %s", filename);
				ret = -1;
				goto exit_list;
			}
			g_free(base);
			g_free(uri);
		}

		/* Have to give control to GTK periodically ... */
		if (pragha_process_gtk_events ()) {
			g_list_free(list);
			return 0;
		}
	}

exit_list:
	g_free(base_m3u);
	if (err) {
		g_error_free(err);
		err = NULL;
	}
	return ret;
}

static gint save_complete_m3u_playlist(GIOChannel *chan, gchar *filename, struct con_win *cwin)
{
	gint ret = 0;
	GList *list = NULL;

	list = pragha_playlist_get_mobj_list(cwin->cplaylist);

	if(list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);
		g_list_free(list);
	}

	return ret;
}

static gint save_selected_to_m3u_playlist(GIOChannel *chan, gchar *filename, struct con_win *cwin)
{
	GList *list = NULL;
	gint ret = 0;

	list = pragha_playlist_get_selection_mobj_list(cwin->cplaylist);

	if (list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);
		g_list_free(list);
	}

	return ret;
}

static gint save_m3u_playlist(GIOChannel *chan, gchar *playlist, gchar *filename,
			      PraghaDatabase *cdbase)
{
	GList *list = NULL;
	gint ret = 0;

	list = add_playlist_to_mobj_list(cdbase, playlist, list);

	if (list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);

		g_list_foreach(list, (GFunc) g_object_unref, NULL);
		g_list_free(list);
	}

	return ret;
}

/**********************/
/* External functions */
/**********************/

/* Append the given playlist to the current playlist */

void add_playlist_current_playlist(GtkTreeModel *model, gchar *playlist, struct con_win *cwin)
{
	GList *list = NULL;

	set_watch_cursor (cwin->mainwindow);

	list = add_playlist_to_mobj_list(cwin->cdbase, playlist, list);

	if(list) {
		pragha_playlist_append_mobj_list(cwin->cplaylist, list);
		update_status_bar_playtime(cwin);
	}
	remove_watch_cursor (cwin->mainwindow);
}

/* Append the given playlist to the mobj list. */

GList *
add_playlist_to_mobj_list(PraghaDatabase *cdbase,
                          gchar *playlist,
                          GList *list)
{
	gchar *s_playlist, *query, *file;
	gint playlist_id, location_id, i = 0;
	PraghaDbResponse result;
	PraghaMusicobject *mobj;

	s_playlist = sanitize_string_to_sqlite3(playlist);
	playlist_id = find_playlist_db(s_playlist, cdbase);

	if(playlist_id == 0)
		goto bad;

	query = g_strdup_printf("SELECT FILE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d",
				playlist_id);
	pragha_database_exec_sqlite_query(cdbase, query, &result);

	for_each_result_row(result, i) {
		file = sanitize_string_to_sqlite3(result.resultp[i]);

		if ((location_id = find_location_db(file, cdbase)))
			mobj = new_musicobject_from_db(cdbase, location_id);
		else
			mobj = new_musicobject_from_file(result.resultp[i]);

		list = g_list_append(list, mobj);

		g_free(file);
	}
	sqlite3_free_table(result.resultp);

bad:
	g_free(s_playlist);

	return list;
}

/* Append the given radio to the mobj list. */

GList *
add_radio_to_mobj_list(PraghaDatabase *cdbase,
                       gchar *radio,
                       GList *list)
{
	gchar *s_radio, *query;
	gint radio_id, i = 0;
	PraghaDbResponse result;
	PraghaMusicobject *mobj;

	s_radio = sanitize_string_to_sqlite3(radio);
	radio_id = find_radio_db(s_radio, cdbase);

	if(radio_id == 0)
		goto bad;

	query = g_strdup_printf("SELECT URI FROM RADIO_TRACKS WHERE RADIO=%d",
				radio_id);

	pragha_database_exec_sqlite_query(cdbase, query, &result);
	for_each_result_row(result, i) {
		mobj = new_musicobject_from_location(result.resultp[i], radio);

		list = g_list_append(list, mobj);

	}
	sqlite3_free_table(result.resultp);

bad:
	g_free(s_radio);

	return list;
}

/* Build a dialog to get a new playlist name */

gchar* rename_playlist_dialog(const gchar * oplaylist, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *label_new, *entry;
	gchar *playlist = NULL;
	gint result;

	/* Create dialog window */

	hbox = gtk_hbox_new(FALSE, 2);

	label_new = gtk_label_new_with_mnemonic(_("Choose a new name"));

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);

	gtk_entry_set_text(GTK_ENTRY(entry), oplaylist);

	dialog = gtk_dialog_new_with_buttons(_("Rename"),
			     GTK_WINDOW(cwin->mainwindow),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
			     NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_box_pack_start(GTK_BOX(hbox), label_new, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		/* Get playlist name */
		playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}
	gtk_widget_destroy(dialog);

	return playlist;
}

void playlist_tree_rename(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list;
	gchar *playlist = NULL, *s_playlist = NULL, *n_playlist = NULL;
	gint node_type;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		path = list->data;
		if (gtk_tree_path_get_depth(path) > 1) {
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, L_NODE_DATA, &playlist, -1);

			s_playlist = sanitize_string_to_sqlite3(playlist);

			n_playlist = rename_playlist_dialog(s_playlist, cwin);
			if(n_playlist != NULL) {
				gtk_tree_model_get(model, &iter, L_NODE_TYPE, &node_type, -1);

				if(node_type == NODE_PLAYLIST) {
					update_playlist_name_db(s_playlist, n_playlist, cwin->cdbase);
					update_menu_playlist_changes(cwin);
				}
				else if (node_type == NODE_RADIO)
					update_radio_name_db(s_playlist, n_playlist, cwin->cdbase);

				g_free(n_playlist);

				init_library_view(cwin);
			}
			g_free(s_playlist);
			g_free(playlist);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

static gboolean delete_existing_item_dialog(const gchar *item,
					    struct con_win *cwin)
{
	gboolean choice = FALSE;
	gint ret;
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Do you want to delete the item: %s ?"),
				item);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));

	switch(ret) {
	case GTK_RESPONSE_YES:
		choice = TRUE;
		break;
	case GTK_RESPONSE_NO:
		choice = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return choice;
}

void playlist_tree_delete(GtkAction *action, struct con_win *cwin)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	gchar *playlist, *s_playlist;
	gint node_type;
	gboolean removed = FALSE;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
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
					s_playlist = sanitize_string_to_sqlite3(playlist);

					if(node_type == NODE_PLAYLIST) {
						delete_playlist_db(s_playlist, cwin->cdbase);
						update_menu_playlist_changes(cwin);
					}
					else if (node_type == NODE_RADIO) {
						delete_radio_db(s_playlist, cwin->cdbase);
					}
					g_free(s_playlist);
					removed = TRUE;
				}
				g_free(playlist);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}

	if(removed)
		init_library_view(cwin);
}

/* Export selection/current playlist to a M3U file */

void export_playlist (gint choice, struct con_win *cwin)
{
	GtkWidget *dialog;
	gchar *filename = NULL, *playlistm3u = NULL;
	GIOChannel *chan = NULL;
	GError *err = NULL;
	gint resp;

	dialog = gtk_file_chooser_dialog_new(_("Export playlist to file"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
						       TRUE);

	playlistm3u = g_strdup_printf("%s.m3u", _("Playlists"));
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), playlistm3u);

	resp = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (resp) {
	case GTK_RESPONSE_ACCEPT:
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		break;
	default:
		goto exit;
	}

	if (!filename)
		goto exit;

	chan = create_m3u_playlist(filename, cwin);
	if (!chan) {
		g_warning("Unable to create M3U playlist file: %s", filename);
		goto exit;
	}

	switch(choice) {
	case SAVE_COMPLETE:
		if (save_complete_m3u_playlist(chan, filename, cwin) < 0) {
			g_warning("Unable to save M3U playlist: %s", filename);
			goto exit;
		}
		break;
	case SAVE_SELECTED:
		if (save_selected_to_m3u_playlist(chan, filename, cwin) < 0) {
			g_warning("Unable to save M3U playlist: %s", filename);
			goto exit;
		}
		break;
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

exit:
	g_free(playlistm3u);
	g_free(filename);
	gtk_widget_destroy(dialog);
}

/* Export saved playlist to a M3U file */

void playlist_tree_export(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GIOChannel *chan = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list, *i;
	GError *err = NULL;
	gint resp, cnt;
	gchar *filename = NULL, *playlist = NULL, *playlistpath = NULL, *playlistm3u = NULL;
	gint node_type;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->library_tree));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->library_tree));
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

	dialog = gtk_file_chooser_dialog_new(_("Export playlist to file"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
						       TRUE);

	playlistm3u = g_strdup_printf("%s.m3u", playlistpath);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), playlistm3u);

	resp = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (resp) {
	case GTK_RESPONSE_ACCEPT:
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		break;
	default:
		goto exit;
	}

	if (!filename)
		goto exit;

	chan = create_m3u_playlist(filename, cwin);
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
	g_free(playlistm3u);
	g_free(playlistpath);
	g_free(filename);
	gtk_widget_destroy(dialog);
}

#ifdef HAVE_PLPARSER
static void _on_pl_entry_parsed(TotemPlParser *parser, gchar *uri,
				gpointer metadata, GSList **plitems)
{
	gchar *filename = NULL;

	if (uri != NULL) {
		/* Convert the uri into a filename to taglib.*/
		if (g_str_has_prefix (uri, "file:"))
			filename = g_filename_from_uri (uri, NULL, NULL);
		else
			filename = g_strdup(uri);

		*plitems = g_slist_append(*plitems, filename);
	}
}

GSList *pragha_totem_pl_parser_parse_from_uri(const gchar *uri)
{
	static TotemPlParser *pl_parser = NULL;
	GSList *plitems = NULL;
	gchar *base;

	pl_parser = totem_pl_parser_new ();
	g_object_set(pl_parser, "recurse", TRUE, NULL);

	g_signal_connect(G_OBJECT(pl_parser), "entry-parsed",
				G_CALLBACK(_on_pl_entry_parsed), &plitems);

	base = get_display_filename(uri, TRUE);

	if (totem_pl_parser_parse_with_base(pl_parser, uri, base, FALSE)
	    != TOTEM_PL_PARSER_RESULT_SUCCESS) {
		/* An error happens while parsing */
		goto bad;
	}
	g_object_unref (pl_parser);

bad:
	g_free(base);

	return plitems;
}
#else
GSList *
pragha_pl_parser_parse_xspf (const gchar *filename)
{
	XMLNode *xml = NULL, *xi, *xl;
	gchar *contents, *f_file, *uri, *base;
	GSList *list = NULL;
	GError *err = NULL;
	GFile *file;
	gsize size;

	file = g_file_new_for_path (filename);

	if (!g_file_load_contents (file, NULL, &contents, &size, NULL, NULL)) {
		goto out;
    	}

	if (g_utf8_validate (contents, -1, NULL) == FALSE) {
		gchar *fixed;
		fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
		if (fixed != NULL) {
			g_free (contents);
			contents = fixed;
		}
	}

	base = get_display_filename(filename, TRUE);

	xml = tinycxml_parse(contents);

	xi = xmlnode_get(xml,CCA { "playlist","trackList","track",NULL},NULL,NULL);
	for(;xi;xi= xi->next) {
		xl = xmlnode_get(xi,CCA {"track","location",NULL},NULL,NULL);

		if (xl && xl->content) {
			f_file = g_filename_from_uri(xl->content, NULL, &err);

			if (!f_file) {
				g_warning("Unable to get filename from UTF-8 string: %s",
					  xl->content);
				g_error_free(err);
				err = NULL;
				continue;
			}

			if (g_path_is_absolute(f_file))
				uri = g_strdup(f_file);
			else {
				uri = g_build_filename (base, f_file, NULL);
			}
			list = g_slist_append (list, uri);
			g_free(f_file);
		}
	}

	xmlnode_free(xml);
	g_free(contents);
	g_free(base);

out:
	g_object_unref (file);
	
	return list;
}

GSList *
pragha_pl_parser_parse_pls (const gchar *file)
{
	GKeyFile *plskeyfile;
	GError *error = NULL;
	GSList *list = NULL;
	guint i, nentries;
	gchar key[128], *file_entry = NULL, *uri = NULL, *base = NULL;

	base = get_display_filename(file, TRUE);

	plskeyfile = g_key_file_new();

	if (!g_key_file_load_from_file(plskeyfile, file, G_KEY_FILE_NONE, &error)) {
		g_critical("Unable to load pls playlist, err: %s", error->message);
	}
	else {
		nentries = g_key_file_get_integer (plskeyfile, "playlist", "NumberOfEntries", NULL);
		if (nentries > 0) {
			for (i = 1; i <= nentries; i++) {
				g_snprintf (key, 128, "File%d", i);
				file_entry = g_key_file_get_string(plskeyfile, "playlist", key, NULL);
				if (NULL == file_entry)
					continue;

				if (g_path_is_absolute(file_entry))
					uri = g_strdup(file_entry);
				else {
					uri = g_build_filename (base, file_entry, NULL);
				}
				list = g_slist_append (list, uri);
				g_free(file_entry);
			}
		}
	}
	g_key_file_free (plskeyfile);
	g_free(base);

	return list;
}

/* Load a M3U playlist, and add tracks to current playlist */

GSList *
pragha_pl_parser_parse_m3u (const gchar *file)
{
	GError *err = NULL;
	GIOChannel *chan = NULL;
	gint fd;
	gsize len, term;
	gchar *str, *filename, *f_file, *uri, *base;
	GSList *list = NULL;

	fd = g_open(file, O_RDONLY, 0);
	if (fd == -1) {
		g_critical("Unable to open file : %s",
			   file);
		return NULL;
	}

	chan = g_io_channel_unix_new(fd);
	if (!chan) {
		g_critical("Unable to open an IO channel for file: %s", file);
		goto exit_close;
	}

	base = get_display_filename(file, TRUE);

	while (g_io_channel_read_line(chan, &str, &len, &term, &err) ==
	       G_IO_STATUS_NORMAL) {

		if (!str || !len)
			break;

		/* Skip lines containing #EXTM3U or #EXTINF */

		if (g_strrstr(str, "#EXTM3U") || g_strrstr(str, "#EXTINF")) {
			g_free(str);
			continue;
		}

		filename = g_strndup(str, term);

		f_file = g_filename_from_utf8(filename, -1, NULL, NULL, &err);
		if (!f_file) {
			g_warning("Unable to get filename from UTF-8 string: %s",
				  filename);
			g_error_free(err);
			err = NULL;
			goto continue_read;
		}

		if (g_path_is_absolute(f_file))
			uri = g_strdup(f_file);
		else {
			uri = g_build_filename (base, f_file, NULL);
		}
		list = g_slist_append (list, uri);

		if (pragha_process_gtk_events ()) {
			g_free(filename);
			g_free(f_file);
			g_free(str);
			goto exit_chan;
		}

		g_free(f_file);
	continue_read:
		g_free(filename);
		g_free(str);
	}

	CDEBUG(DBG_INFO, "Loaded M3U playlist: %s", file);

	g_free(base);

exit_chan:
	if (g_io_channel_shutdown(chan, TRUE, &err) != G_IO_STATUS_NORMAL) {
		g_critical("Unable to open M3U playlist: %s", file);
		g_error_free(err);
		err = NULL;
	}
	g_io_channel_unref(chan);

exit_close:
	close(fd);

	return list;
}

GSList *
pragha_pl_parser_parse (enum playlist_type format, const gchar *filename)
{
	GSList *list = NULL;

	switch (format)
	{
	case PL_FORMAT_M3U:
		list = pragha_pl_parser_parse_m3u (filename);
		break;
	case PL_FORMAT_PLS:
		list = pragha_pl_parser_parse_pls (filename);
		break;
	case PL_FORMAT_ASX:
		//list = pragha_pl_parser_parse_asx (filename);
		break;
	case PL_FORMAT_XSPF:
		list = pragha_pl_parser_parse_xspf (filename);
		break;
	default:
	break;
    }

    return list;
}

GSList *pragha_pl_parser_parse_from_file_by_extension (const gchar *filename)
{
	enum playlist_type format = PL_FORMAT_UNKNOWN;
	GSList *list = NULL;

	if ((format = pragha_pl_parser_guess_format_from_extension (filename)) != PL_FORMAT_UNKNOWN) {
		list = pragha_pl_parser_parse (format, filename);
	}
	else {
		g_debug ("Unable to guess playlist format : %s", filename);
	}

	return list;
}
#endif

void pragha_pl_parser_open_from_file_by_extension (const gchar *file, struct con_win *cwin)
{
	GSList *list = NULL, *i = NULL;
	gchar *summary;
	gint try = 0, added = 0;
	PraghaMusicobject *mobj;

	set_watch_cursor (cwin->mainwindow);

#ifdef HAVE_PLPARSER
	gchar *uri = g_filename_to_uri (file, NULL, NULL);
	list = pragha_totem_pl_parser_parse_from_uri(uri);
	g_free (uri);
#else
	list = pragha_pl_parser_parse_from_file_by_extension (file);
#endif

	for (i = list; i != NULL; i = i->next) {
		try++;
		mobj = new_musicobject_from_file(i->data);
		if (mobj) {
			added++;
			append_current_playlist(cwin->cplaylist, mobj);
		}

		if (pragha_process_gtk_events ())
			return;

		g_free(i->data);
	}

	remove_watch_cursor (cwin->mainwindow);

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);
	set_status_message(summary, cwin);
	g_free(summary);

	g_slist_free(list);
}

/* Appennd a tracks list to a playlist using the given type */

static void
append_files_to_playlist(PraghaDatabase *cdbase, GSList *list, gint playlist_id)
{
	gchar *file, *s_file;
	GSList *i = NULL;

	db_begin_transaction(cdbase);

	for (i=list; i != NULL; i = i->next) {
		file = i->data;
		s_file = sanitize_string_to_sqlite3(file);
		add_track_playlist_db(s_file, playlist_id, cdbase);
		g_free(s_file);
		g_free(file);
	}

	db_commit_transaction(cdbase);
}

/* Save tracks to a playlist using the given type */

void save_playlist(gint playlist_id, enum playlist_mgmt type,
		   struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	GList *mlist = NULL, *i;
	GSList *files = NULL;
	gchar *file = NULL;

	switch(type) {
	case SAVE_COMPLETE:
		mlist = pragha_playlist_get_mobj_list(cwin->cplaylist);
		break;
	case SAVE_SELECTED:
		mlist = pragha_playlist_get_selection_mobj_list(cwin->cplaylist);
		break;
	default:
		break;
	}

	if(mlist != NULL) {
		for (i=mlist; i != NULL; i = i->next) {
			mobj = i->data;
			if (pragha_musicobject_get_file_type(mobj) != FILE_CDDA &&
			    pragha_musicobject_get_file_type(mobj) != FILE_HTTP) {
			    	file = g_strdup(pragha_musicobject_get_file(mobj));
				files = g_slist_prepend(files, file);
			}
			else if(pragha_musicobject_get_file_type(mobj) == FILE_HTTP) {
				/* TODO: Fix this negradaaa!. */
				file = g_strdup_printf("Radio:%s", pragha_musicobject_get_file(mobj));
				files = g_slist_prepend(files, file);
			}
		}
		g_list_free(mlist);
	}

	if(files != NULL) {
		append_files_to_playlist(cwin->cdbase, files, playlist_id);
		g_slist_free(files);
	}
}

void new_playlist(const gchar *playlist, enum playlist_mgmt type,
		  struct con_win *cwin)
{
	gchar *s_playlist;
	gint playlist_id = 0;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	s_playlist = sanitize_string_to_sqlite3(playlist);

	if ((playlist_id = find_playlist_db(s_playlist, cwin->cdbase))) {
		if (overwrite_existing_playlist(playlist, cwin))
			delete_playlist_db(s_playlist, cwin->cdbase);
		else
			goto exit;
	}

	playlist_id = add_new_playlist_db(s_playlist, cwin->cdbase);
	save_playlist(playlist_id, type, cwin);

exit:
	g_free(s_playlist);
}

void append_playlist(const gchar *playlist, gint type, struct con_win *cwin)
{
	gchar *s_playlist;
	gint playlist_id;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	s_playlist = sanitize_string_to_sqlite3(playlist);
	playlist_id = find_playlist_db(s_playlist, cwin->cdbase);

	if (!playlist_id) {
		g_warning("Playlist doesn't exist\n");
		goto exit;
	}

	save_playlist(playlist_id, type, cwin);

exit:
	g_free(s_playlist);
}

void new_radio (const gchar *uri, const gchar *name, struct con_win *cwin)
{
	gchar *s_radio, *file = NULL;
	gint radio_id = 0;

	if (string_is_empty(name)) {
		g_warning("Radio name is NULL");
		return;
	}

	s_radio = sanitize_string_to_sqlite3(name);

	if ((radio_id = find_radio_db(s_radio, cwin->cdbase))) {
		if (overwrite_existing_playlist(name, cwin))
			delete_radio_db(s_radio, cwin->cdbase);
		else
			goto exit;
	}

	radio_id = add_new_radio_db(s_radio, cwin->cdbase);

	file = sanitize_string_to_sqlite3(uri);
	add_track_radio_db(file, radio_id, cwin->cdbase);
	g_free(file);

exit:
	g_free(s_radio);
}

void append_to_playlist(GtkMenuItem *menuitem, struct con_win *cwin)
{
	const gchar *playlist;

	playlist = gtk_menu_item_get_label (menuitem);

	append_playlist(playlist, SAVE_SELECTED, cwin);
}

void save_to_playlist(GtkMenuItem *menuitem, struct con_win *cwin)
{
	const gchar *playlist;

	playlist = gtk_menu_item_get_label (menuitem);

	new_playlist(playlist, SAVE_COMPLETE, cwin);
}

void update_menu_playlist_changes(struct con_win *cwin)
{
	complete_add_to_playlist_submenu (cwin);
	complete_save_playlist_submenu (cwin);
	complete_main_save_playlist_submenu(cwin);
	complete_main_add_to_playlist_submenu (cwin);
}

void complete_add_to_playlist_submenu (struct con_win *cwin)
{
	PraghaDbResponse result;
	GtkWidget *submenu, *menuitem;
	gchar *query;
	gint i;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_ui_manager_get_widget (cwin->cplaylist->cp_context_menu, "/popup/Add to another playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_selected_playlist), cwin);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	query = g_strdup_printf ("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";", SAVE_PLAYLIST_STATE);

	pragha_database_exec_sqlite_query(cwin->cdbase, query, &result);

	for_each_result_row (result, i) {
		menuitem = gtk_image_menu_item_new_with_label (result.resultp[i]);
		g_signal_connect (menuitem, "activate", G_CALLBACK(append_to_playlist), cwin);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	gtk_widget_show_all (submenu);
	sqlite3_free_table (result.resultp);
}

void complete_save_playlist_submenu (struct con_win *cwin)
{
	PraghaDbResponse result;
	GtkWidget *submenu, *menuitem;
	gchar *query;
	gint i;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_ui_manager_get_widget (cwin->cplaylist->cp_context_menu, "/popup/Save playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_current_playlist), cwin);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	query = g_strdup_printf ("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";", SAVE_PLAYLIST_STATE);

	pragha_database_exec_sqlite_query(cwin->cdbase, query, &result);

	for_each_result_row (result, i) {
		menuitem = gtk_image_menu_item_new_with_label (result.resultp[i]);
		g_signal_connect (menuitem, "activate", G_CALLBACK(save_to_playlist), cwin);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	gtk_widget_show_all (submenu);
	sqlite3_free_table (result.resultp);
}

void complete_main_save_playlist_submenu (struct con_win *cwin)
{
	PraghaDbResponse result;
	GtkWidget *submenu, *menuitem;
	GtkAccelGroup* accel_group;
	gchar *query;
	gint i;

	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM(gtk_ui_manager_get_widget(cwin->bar_context_menu,"/Menubar/PlaylistMenu/Save playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_current_playlist), cwin);

	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow), accel_group);
	gtk_menu_set_accel_group(GTK_MENU(submenu), accel_group);
	gtk_accel_map_add_entry ("<SubMenu>/New playlist", gdk_keyval_from_name ("s"), GDK_CONTROL_MASK);
	gtk_menu_item_set_accel_path (GTK_MENU_ITEM(menuitem), "<SubMenu>/New playlist");

	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	query = g_strdup_printf ("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";", SAVE_PLAYLIST_STATE);

	pragha_database_exec_sqlite_query(cwin->cdbase, query, &result);

	for_each_result_row (result, i) {
		menuitem = gtk_image_menu_item_new_with_label (result.resultp[i]);
		g_signal_connect (menuitem, "activate", G_CALLBACK(save_to_playlist), cwin);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	gtk_widget_show_all (submenu);
	sqlite3_free_table (result.resultp);
}

void complete_main_add_to_playlist_submenu (struct con_win *cwin)
{
	PraghaDbResponse result;
	GtkWidget *submenu, *menuitem;
	GtkAccelGroup* accel_group;
	gchar *query;
	gint i;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM(gtk_ui_manager_get_widget(cwin->bar_context_menu,"/Menubar/PlaylistMenu/Add to another playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_selected_playlist), cwin);

	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow), accel_group);
	gtk_menu_set_accel_group(GTK_MENU(submenu), accel_group);
	gtk_accel_map_add_entry ("<SubMenu>/Add to another playlist", gdk_keyval_from_name ("s"), GDK_CONTROL_MASK+GDK_SHIFT_MASK);
	gtk_menu_item_set_accel_path (GTK_MENU_ITEM(menuitem), "<SubMenu>/Add to another playlist");

	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	query = g_strdup_printf ("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";", SAVE_PLAYLIST_STATE);

	pragha_database_exec_sqlite_query(cwin->cdbase, query, &result);

	for_each_result_row (result, i) {
		menuitem = gtk_image_menu_item_new_with_label (result.resultp[i]);
		g_signal_connect (menuitem, "activate", G_CALLBACK(save_to_playlist), cwin);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	gtk_widget_show_all (submenu);
	sqlite3_free_table (result.resultp);
}
