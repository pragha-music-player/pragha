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

gchar *cp_context_menu_xml = "<ui>		    				\
	<popup>					    				\
	<menuitem action=\"Queue track\"/>					\
	<menuitem action=\"Dequeue track\"/>					\
	<separator/>				    				\
	<menuitem action=\"Remove from playlist\"/>				\
	<menuitem action=\"Crop playlist\"/>					\
	<menuitem action=\"Clear playlist\"/>					\
	<separator/>				    				\
	<menuitem action=\"Add to another playlist\"/>				\
	<menuitem action=\"Save playlist\"/>					\
	<separator/>				    				\
	<menu action=\"ToolsMenu\">						\
		<menuitem action=\"Search lyric\"/>				\
		<menuitem action=\"Search artist info\"/>			\
		<separator/>							\
		<menuitem action=\"Love track\"/>				\
		<menuitem action=\"Unlove track\"/>				\
		<separator/>							\
		<menuitem action=\"Add similar\"/>				\
	</menu>									\
	<separator/>				    				\
	<menuitem action=\"Copy tag to selection\"/>				\
	<separator/>				    				\
	<menuitem action=\"Edit tags\"/>					\
	</popup>				    				\
	</ui>";

gchar *cp_null_context_menu_xml = "<ui>		    				\
	<popup>					    				\
	<menuitem action=\"Add files\"/>					\
	<menuitem action=\"Add Audio CD\"/>					\
	<menuitem action=\"Add location\"/>					\
	<separator/>				    				\
	<menuitem action=\"Add the library\"/>					\
	<separator/>				    				\
	<menuitem action=\"Lateral panel\"/>					\
	<separator/>				    				\
	<menuitem action=\"Quit\"/>						\
	</popup>				    				\
	</ui>";

GtkActionEntry cp_context_aentries[] = {
	{"Queue track", GTK_STOCK_ADD, N_("Add to playback queue"),
	 "", "Add to playback queue", G_CALLBACK(queue_current_playlist)},
	{"Dequeue track", GTK_STOCK_REMOVE, N_("Remove to playback queue"),
	 "", "Remove to playback queue", G_CALLBACK(dequeue_current_playlist)},
	{"Remove from playlist", GTK_STOCK_REMOVE, N_("Remove from playlist"),
	 "", "Remove selection from playlist", G_CALLBACK(remove_from_playlist)},
	{"Crop playlist", GTK_STOCK_REMOVE, N_("Crop playlist"),
	 "", "Remove no telected tracks of playlist", G_CALLBACK(crop_current_playlist)},
	{"Clear playlist", GTK_STOCK_CLEAR, N_("Clear playlist"),
	 "", "Clear the current playlist", G_CALLBACK(clear_current_playlist)},
	{"Add to another playlist", GTK_STOCK_SAVE_AS, N_("Add to another playlist")},
	{"Save playlist", GTK_STOCK_SAVE, N_("Save playlist")},
	{"ToolsMenu", NULL, N_("_Tools")},
	#ifdef HAVE_LIBGLYR
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "", "Search lyric", G_CALLBACK(related_get_lyric_current_playlist_action)},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", G_CALLBACK(related_get_artist_info_current_playlist_action)},
	#else
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "", "Search lyric", NULL},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", NULL},
	#endif
	{"Lastfm", NULL, N_("_Lastfm")},
	#ifdef HAVE_LIBCLASTFM
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", G_CALLBACK(lastfm_track_current_playlist_love_action)},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", G_CALLBACK(lastfm_track_current_playlist_unlove_action)},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", G_CALLBACK(lastfm_get_similar_current_playlist_action)},
	#else
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", NULL},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", NULL},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", NULL},
	#endif
	{"Copy tag to selection", GTK_STOCK_COPY, NULL,
	 "", "Copy tag to selection", G_CALLBACK(copy_tags_to_selection_action)},
	{"Edit tags", GTK_STOCK_EDIT, N_("Edit track information"),
	 "", "Edit information for this track", G_CALLBACK(edit_tags_current_playlist)}
};

GtkToggleActionEntry cp_null_toggles_entries[] = {
	{"Lateral panel", NULL, N_("Lateral _panel"),
	 "", "Lateral panel", G_CALLBACK(library_pane_action),
	TRUE}
};

GtkActionEntry cp_null_context_aentries[] = {
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 "", N_("Open a media file"), G_CALLBACK(open_file_action)},
	{"Add Audio CD", GTK_STOCK_CDROM, N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(add_audio_cd_action)},
	{"Add location", GTK_STOCK_NETWORK, N_("Add _location"),
	 "", "Add a no local stream", G_CALLBACK(add_location_action)},
	{"Add the library", GTK_STOCK_ADD, N_("_Add the library"),
	"", "Add all the library", G_CALLBACK(add_libary_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 "<Control>Q", "Quit pragha", G_CALLBACK(quit_action)}
};

static GtkWidget* create_header_context_menu(struct con_win *cwin)
{
	GtkWidget *menu;
	GtkWidget *toggle_track,
		*toggle_title,
		*toggle_artist,
		*toggle_album,
		*toggle_genre,
		*toggle_bitrate,
		*toggle_year,
		*toggle_comment,
		*toggle_length,
		*toggle_filename,
		*separator,
		*action_clear_sort;

	menu = gtk_menu_new();

	/* Create the checkmenu items */

	toggle_track = gtk_check_menu_item_new_with_label(_("Track"));
	toggle_title = gtk_check_menu_item_new_with_label(_("Title"));
	toggle_artist = gtk_check_menu_item_new_with_label(_("Artist"));
	toggle_album = gtk_check_menu_item_new_with_label(_("Album"));
	toggle_genre = gtk_check_menu_item_new_with_label(_("Genre"));
	toggle_bitrate = gtk_check_menu_item_new_with_label(_("Bitrate"));
	toggle_year = gtk_check_menu_item_new_with_label(_("Year"));
	toggle_comment = gtk_check_menu_item_new_with_label(_("Comment"));
	toggle_length = gtk_check_menu_item_new_with_label(_("Length"));
	toggle_filename = gtk_check_menu_item_new_with_label(_("Filename"));
	separator = gtk_separator_menu_item_new ();

	action_clear_sort = gtk_image_menu_item_new_with_label(_("Clear sort"));
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(action_clear_sort),
                gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));

	/* Add the items to the menu */

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_track);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_title);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_artist);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_album);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_genre);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_bitrate);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_year);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_comment);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_length);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_filename);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), action_clear_sort);

	/* Initialize the state of the items */

	if (is_present_str_list(P_TRACK_NO_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_track), TRUE);
	if (is_present_str_list(P_TITLE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_title), TRUE);
	if (is_present_str_list(P_ARTIST_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_artist), TRUE);
	if (is_present_str_list(P_ALBUM_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_album), TRUE);
	if (is_present_str_list(P_GENRE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_genre), TRUE);
	if (is_present_str_list(P_BITRATE_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_bitrate), TRUE);
	if (is_present_str_list(P_YEAR_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_year), TRUE);
	if (is_present_str_list(P_COMMENT_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_comment), TRUE);
	if (is_present_str_list(P_LENGTH_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_length), TRUE);
	if (is_present_str_list(P_FILENAME_STR, cwin->cpref->playlist_columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_filename), TRUE);

	/* Setup the individual signal handlers */

	g_signal_connect(G_OBJECT(toggle_track), "toggled",
			 G_CALLBACK(playlist_track_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_title), "toggled",
			 G_CALLBACK(playlist_title_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_artist), "toggled",
			 G_CALLBACK(playlist_artist_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_album), "toggled",
			 G_CALLBACK(playlist_album_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_genre), "toggled",
			 G_CALLBACK(playlist_genre_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_bitrate), "toggled",
			 G_CALLBACK(playlist_bitrate_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_year), "toggled",
			 G_CALLBACK(playlist_year_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_comment), "toggled",
			 G_CALLBACK(playlist_comment_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_length), "toggled",
			 G_CALLBACK(playlist_length_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(toggle_filename), "toggled",
			 G_CALLBACK(playlist_filename_column_change_cb), cwin);
	g_signal_connect(G_OBJECT(action_clear_sort), "activate",
			 G_CALLBACK(clear_sort_current_playlist_cb), cwin);

	gtk_widget_show_all(menu);

	return menu;
}

static GtkUIManager* create_cp_context_menu(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("CP Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       cp_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create current playlist context menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(context_actions,
				     cp_context_aentries,
				     G_N_ELEMENTS(cp_context_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

static GtkUIManager* create_cp_null_context_menu(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("CP Null Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu,
					       cp_null_context_menu_xml,
					       -1, &error)) {
		g_critical("Unable to create current playlist null context menu, err : %s",
			   error->message);
	}
	gtk_action_group_add_actions(context_actions,
				     cp_null_context_aentries,
				     G_N_ELEMENTS(cp_null_context_aentries),
				     (gpointer)cwin);
	gtk_action_group_add_toggle_actions (context_actions, 
					cp_null_toggles_entries,
					G_N_ELEMENTS(cp_null_toggles_entries), 
					cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(context_menu));
	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	return context_menu;
}

/* Initialize columns of current playlist */

void init_current_playlist_columns(GtkWidget *current_playlist, struct con_win *cwin)
{
	const gchar *col_name;
	GtkTreeViewColumn *col;
	GList *list = NULL, *i;
	GSList *j;
	gint k = 0;

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(current_playlist));

	/* Mark only the columns that are present in
	   cwin->cpref->playlist_columns as visible.
	   And set their sizes */

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			col = i->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cwin->cpref->playlist_columns)) {
				j = g_slist_nth(cwin->cpref->playlist_column_widths,
						k++);
				gtk_tree_view_column_set_visible(col, TRUE);
				gtk_tree_view_column_set_sizing(col,
						GTK_TREE_VIEW_COLUMN_FIXED);
				if (GPOINTER_TO_INT(j->data) > COL_WIDTH_THRESH)
					gtk_tree_view_column_set_fixed_width(col,
						     GPOINTER_TO_INT(j->data));
				else
					gtk_tree_view_column_set_fixed_width(col,
						     DEFAULT_PLAYLIST_COL_WIDTH);
			}
			else
				gtk_tree_view_column_set_visible(col, FALSE);
		}
		g_list_free(list);
	}
	else
		g_warning("(%s): No columns in playlist view", __func__);

	/* Always show queue and status pixbuf colum */
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(current_playlist), 0);
	gtk_tree_view_column_set_visible(col, TRUE);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(col, 32);
}

/* Comparison function for column names */

static gint compare_playlist_column_name(gconstpointer a, gconstpointer b)
{
	const gchar *e1 = a;
	const gchar *e2 = b;

	return g_ascii_strcasecmp(e1, e2);
}

/* Function to add/delete columns from preferences */

static void modify_current_playlist_columns(struct con_win *cwin,
					    const gchar *col_name,
					    gboolean state)
{
	gboolean pref_present;
	GSList *element;

	if (!col_name) {
		g_warning("Invalid column name");
		return;
	}

	pref_present = is_present_str_list(col_name, cwin->cpref->playlist_columns);

	/* Already in preferences */

	if (pref_present && state) {
		return;
	}

	/* Remove from preferences */

	else if (pref_present && !state) {
		element = g_slist_find_custom(cwin->cpref->playlist_columns,
				      col_name, compare_playlist_column_name);
		if (element) {
			g_free(element->data);
			cwin->cpref->playlist_columns =
				g_slist_delete_link(cwin->cpref->playlist_columns,
						    element);
		}
		else
			g_warning("Column : %s not found in preferences",
				  col_name);
	}

	/* Add to preferences */
 
	else if (!pref_present && state) { 
		cwin->cpref->playlist_columns =
			g_slist_append(cwin->cpref->playlist_columns,
				       g_strdup(col_name));
	}
}

/* Callback for adding/deleting track_no column */

void playlist_track_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_TRACK_NO - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting title column */

void playlist_title_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_TITLE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting artist column */

void playlist_artist_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_ARTIST - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting album column */

void playlist_album_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_ALBUM - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting genre column */

void playlist_genre_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_GENRE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting bitrate column */

void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_BITRATE - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting year column */

void playlist_year_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_YEAR - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting comment column */

void playlist_comment_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_COMMENT - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting length column */

void playlist_length_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_LENGTH - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Callback for adding/deleting filename column */

void playlist_filename_column_change_cb(GtkCheckMenuItem *item, struct con_win *cwin)
{
	const gchar *col_name;
	gboolean state;
	GtkTreeViewColumn *col;

	state = gtk_check_menu_item_get_active(item);
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(CURRENT_PLAYLIST),
				       P_FILENAME - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, state);
	modify_current_playlist_columns(cwin, col_name, state);
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist_cb(GtkMenuItem *item, struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(CURRENT_PLAYLIST));

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Comparison function for track numbers */

gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->track_no < mobj_b->tags->track_no)
		return -1;
	else if (mobj_a->tags->track_no > mobj_b->tags->track_no)
		return 1;
	else
		return 0;
}

/* Comparison function for bitrates */

gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->bitrate < mobj_b->tags->bitrate)
		return -1;
	else if (mobj_a->tags->bitrate > mobj_b->tags->bitrate)
		return 1;
	else
		return 0;
}

/* Comparison function for years */

gint compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->year < mobj_b->tags->year)
		return -1;
	else if (mobj_a->tags->year > mobj_b->tags->year)
		return 1;
	else
		return 0;
}

/* Comparison function for lengths */

gint compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	struct musicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (mobj_a->tags->length < mobj_b->tags->length)
		return -1;
	else if (mobj_a->tags->length > mobj_b->tags->length)
		return 1;
	else
		return 0;
}

static void create_current_playlist_columns(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *state_pixbuf,
		*label_track,
		*label_title,
		*label_artist,
		*label_album,
		*label_genre,
		*label_bitrate,
		*label_year,
		*label_comment,
		*label_length,
		*label_filename;
	GtkWidget *col_button;

	label_track = gtk_label_new(_("Track"));
	label_title = gtk_label_new(_("Title"));
	label_artist = gtk_label_new(_("Artist"));
	label_album = gtk_label_new(_("Album"));
	label_genre = gtk_label_new(_("Genre"));
	label_bitrate = gtk_label_new(_("Bitrate"));
	label_year = gtk_label_new(_("Year"));
	label_comment = gtk_label_new(_("Comment"));
	label_length = gtk_label_new(_("Length"));
	label_filename = gtk_label_new(_("Filename"));

	state_pixbuf = gtk_image_new_from_icon_name ("audio-volume-high", GTK_ICON_SIZE_MENU);

	/* Column : Queue Bubble and Status Pixbuf */

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_bubble_new ();
	gtk_cell_renderer_set_fixed_size (renderer, 12, -1);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
	gtk_tree_view_column_set_attributes(column, renderer, "markup", P_QUEUE, "show-bubble", P_BUBBLE, NULL);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", P_STATUS_PIXBUF, NULL);

	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(current_playlist), column);

	gtk_tree_view_column_set_widget (column, state_pixbuf);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_widget_show (state_pixbuf);

	/* Column : Track No */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TRACK_NO_STR,
							  renderer,
							  "text",
							  P_TRACK_NO,
							  NULL);

	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TRACK_NO);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_track);
	gtk_widget_show(label_track);
	col_button = gtk_widget_get_ancestor(label_track, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Title */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TITLE_STR,
							  renderer,
							  "text",
							  P_TITLE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TITLE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_title);
	gtk_widget_show(label_title);
	col_button = gtk_widget_get_ancestor(label_title, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Artist */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_ARTIST_STR,
							  renderer,
							  "text",
							  P_ARTIST,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ARTIST);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_artist);
	gtk_widget_show(label_artist);
	col_button = gtk_widget_get_ancestor(label_artist, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Album */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_ALBUM_STR,
							  renderer,
							  "text",
							  P_ALBUM,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ALBUM);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_album);
	gtk_widget_show(label_album);
	col_button = gtk_widget_get_ancestor(label_album, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Genre */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_GENRE_STR,
							  renderer,
							  "text",
							  P_GENRE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_GENRE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_genre);
	gtk_widget_show(label_genre);
	col_button = gtk_widget_get_ancestor(label_genre, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Bitrate */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_BITRATE_STR,
							  renderer,
							  "text",
							  P_BITRATE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_BITRATE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_bitrate);
	gtk_widget_show(label_bitrate);
	col_button = gtk_widget_get_ancestor(label_bitrate, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Year */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_YEAR_STR,
							  renderer,
							  "text",
							  P_YEAR,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_YEAR);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_year);
	gtk_widget_show(label_year);
	col_button = gtk_widget_get_ancestor(label_year, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Comment */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_COMMENT_STR,
							  renderer,
							  "text",
							  P_COMMENT,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_COMMENT);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_comment);
	gtk_widget_show(label_comment);
	col_button = gtk_widget_get_ancestor(label_comment, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Length */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_LENGTH_STR,
							  renderer,
							  "text",
							  P_LENGTH,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_LENGTH);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_length);
	gtk_widget_show(label_length);
	col_button = gtk_widget_get_ancestor(label_length, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);

	/* Column : Filename */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_FILENAME_STR,
							  renderer,
							  "text",
							  P_FILENAME,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_FILENAME);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);
	gtk_tree_view_column_set_widget(column, label_filename);
	gtk_widget_show(label_filename);
	col_button = gtk_widget_get_ancestor(label_filename, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cwin);
}

GtkWidget *
create_current_playlist_container(struct con_win *cwin)
{
	GtkWidget *current_playlist_scroll;

	current_playlist_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(current_playlist_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(current_playlist_scroll), GTK_SHADOW_IN);

	return current_playlist_scroll;
}

GtkWidget *
create_current_playlist_view(struct con_win *cwin)
{
	GtkWidget *current_playlist;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;

	/* Create the tree store */

	store = gtk_list_store_new(N_P_COLUMNS,
				   G_TYPE_POINTER,	/* Pointer to musicobject */
				   G_TYPE_STRING,	/* Queue No String */
				   G_TYPE_BOOLEAN,	/* Show Bublle Queue */
				   GDK_TYPE_PIXBUF,	/* Playback status pixbuf */
				   G_TYPE_STRING,	/* Tag : Track No */
				   G_TYPE_STRING,	/* Tag : Title */
				   G_TYPE_STRING,	/* Tag : Artist */
				   G_TYPE_STRING,	/* Tag : Album */
				   G_TYPE_STRING,	/* Tag : Genre */
				   G_TYPE_STRING,	/* Tag : Bitrate */
				   G_TYPE_STRING,	/* Tag : Year */
				   G_TYPE_STRING,	/* Tag : Comment */
				   G_TYPE_STRING,	/* Tag : Length */
				   G_TYPE_STRING,	/* Filename */
				   G_TYPE_BOOLEAN);	/* Played flag */

	/* Create the tree view */

	current_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(current_playlist));
	sortable = GTK_TREE_SORTABLE(model);

	/* Disable interactive search */

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(current_playlist), FALSE);

	/* Set the sort functions */

	gtk_tree_sortable_set_sort_func(sortable,
					P_TRACK_NO,
					compare_track_no,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_BITRATE,
					compare_bitrate,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_YEAR,
					compare_year,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_LENGTH,
					compare_length,
					NULL,
					NULL);

	/* Set selection properties */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(current_playlist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create the columns and cell renderers */

	create_current_playlist_columns(current_playlist, cwin);

	/* Signal handler for double-clicking on a row */

	g_signal_connect(G_OBJECT(current_playlist), "row-activated",
			 G_CALLBACK(current_playlist_row_activated_cb), cwin);

	g_signal_connect (G_OBJECT (current_playlist), "key-press-event",
			  G_CALLBACK (current_playlist_key_press), cwin);

	/* Create contextual menus */

	cwin->cp_context_menu = create_cp_context_menu(current_playlist, cwin);
	cwin->cp_null_context_menu = create_cp_null_context_menu(current_playlist, cwin);
	cwin->header_context_menu = create_header_context_menu(cwin);

	/* Signal handler for right-clicking and selection */

	g_signal_connect(G_OBJECT(GTK_WIDGET(current_playlist)), "button-press-event",
			 G_CALLBACK(current_playlist_button_press_cb), cwin);

	g_signal_connect(G_OBJECT(GTK_WIDGET(current_playlist)), "button-release-event",
			 G_CALLBACK(current_playlist_button_release_cb), cwin);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(current_playlist), cwin->cpref->use_hint);

	/* Set initial column visibility */

	init_current_playlist_columns(current_playlist, cwin);

	g_object_unref(store);

	return current_playlist;
}

void
free_current_playlist(struct con_playlist* cplaylist)
{
	g_rand_free(cplaylist->rand);
	g_slice_free(struct con_playlist, cplaylist);
}

struct con_playlist*
new_current_playlist(struct con_win *cwin)
{
	struct con_playlist *cplaylist;

	cplaylist = g_slice_new0(struct con_playlist);

	cplaylist->container = create_current_playlist_container(cwin);
	cplaylist->wplaylist = create_current_playlist_view(cwin);

	gtk_container_add (GTK_CONTAINER(cplaylist->container), cplaylist->wplaylist);

	cplaylist->rand = g_rand_new();
	cplaylist->playlist_change = TRUE;

	return cplaylist;
}