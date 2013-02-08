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

/****************/
/* Library tree */
/****************/

static GtkUIManager* create_library_tree_context_menu(GtkWidget *library_tree,
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

static GtkUIManager* create_header_library_tree_context_menu(GtkWidget *library_tree,
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

static const GtkTargetEntry lentries[] = {
	{"REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY},
	{"text/uri-list", GTK_TARGET_OTHER_APP, TARGET_URI_LIST},
	{"text/plain", GTK_TARGET_OTHER_APP, TARGET_PLAIN_TEXT}
};

static void init_library_dnd(struct con_win *cwin)
{
	/* Source: Library View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->clibrary->library_tree),
					       GDK_BUTTON1_MASK,
					       lentries,
					       G_N_ELEMENTS(lentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cwin->clibrary->library_tree)),
			 "drag-begin",
			 G_CALLBACK(dnd_library_tree_begin),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->clibrary->library_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_library_tree_get),
			 cwin);
}

static GtkWidget* create_library_tree(struct con_win *cwin)
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

	cwin->clibrary->library_store = store;
	cwin->clibrary->library_tree = library_tree;

	init_library_dnd(cwin);

	g_object_unref(library_filter_tree);
	
	return library_tree;
}

/*****************/
/* Playlist Tree */
/*****************/

static GtkUIManager* create_playlist_tree_context_menu(struct con_win *cwin)
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

/***************************/
/* Left pane (Browse mode) */
/***************************/

static GtkUIManager* create_library_page_context_menu(struct con_win *cwin)
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

static void
pragha_sidebar_close (GtkWidget *widget, struct con_win *cwin)
{
	GtkAction *action;
	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Lateral panel");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);
}

GtkWidget *
create_close_button(struct con_win *cwin)
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
create_library_view_options_combo(struct con_win *cwin)
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

	cwin->clibrary->combo_order = button;
	cwin->clibrary->combo_order_label = label_order;

	return button;
}

GtkWidget *
create_sidebar_header(struct con_win *cwin)
{
	GtkWidget *hbox, *combo, *close_button;

	hbox = gtk_hbox_new(FALSE, 0);

	combo = create_library_view_options_combo(cwin);
	close_button = create_close_button(cwin);

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

static void
update_library_playlist_changes(PraghaDatabase *database, struct con_win *cwin)
{
	/*
	 * Rework to olny update Playlist and radio tree!!!.
	 **/
	init_library_view(cwin);
	update_menu_playlist_changes(cwin);
}

static GtkWidget* create_browse_mode_view(struct con_win *cwin)
{
	GtkWidget *vbox_lib;
	GtkWidget *library_tree, *library_tree_scroll;
	GtkWidget *order_selector, *search_entry;

	vbox_lib = gtk_vbox_new(FALSE, 2);

	search_entry = pragha_search_entry_new(cwin);

	g_signal_connect (G_OBJECT(search_entry), "changed",
			 G_CALLBACK(simple_library_search_keyrelease_handler), cwin);
	g_signal_connect (G_OBJECT(search_entry), "activate",
			 G_CALLBACK(simple_library_search_activate_handler), cwin);

	order_selector = create_sidebar_header (cwin);

	library_tree_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(library_tree_scroll),
					GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(library_tree_scroll), 2);

	library_tree = create_library_tree(cwin);

	gtk_container_add(GTK_CONTAINER(library_tree_scroll), library_tree);

	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   order_selector,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   search_entry,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   library_tree_scroll,
			   TRUE,
			   TRUE,
			   0);

	cwin->clibrary->browse_mode = vbox_lib;
	cwin->clibrary->search_entry = search_entry;

	g_signal_connect (cwin->cdbase, "PlaylistsChanged", G_CALLBACK (update_library_playlist_changes), cwin);

	return vbox_lib;
}

static GtkWidget*
create_playlist_pane_view(struct con_win *cwin)
{
	GtkWidget *vbox;
	PraghaPlaylist *cplaylist;
	PraghaStatusbar *statusbar;

	vbox = gtk_vbox_new(FALSE, 2);

	statusbar = pragha_statusbar_get();
	cplaylist = cplaylist_new(cwin);

	/* Pack everything */

	gtk_box_pack_start(GTK_BOX(vbox),
			   pragha_playlist_get_widget(cplaylist),
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(statusbar),
			   FALSE,
			   FALSE,
			   0);

	/* Store references*/

	cwin->cplaylist = cplaylist;
	cwin->statusbar = statusbar;

	return vbox;
}

/*****************/
/* DnD functions */
/*****************/
/* These two functions are only callbacks that must be passed to
gtk_tree_selection_set_select_function() to chose if GTK is allowed
to change selection itself or if we handle it ourselves */

gboolean tree_selection_func_true(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data)
{
	return TRUE;
}

gboolean tree_selection_func_false(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data)
{
	return FALSE;
}

/********************************/
/* Externally visible functions */
/********************************/

GtkWidget* create_main_region(struct con_win *cwin)
{
	GtkWidget *hpane;
	GtkWidget *browse_mode, *playlist_pane;


	/* A two paned container */

	hpane = gtk_hpaned_new();

	/* Left pane contains a notebook widget holding the various views */

	browse_mode = create_browse_mode_view(cwin);

	/* Right pane contains the current playlist */

	playlist_pane = create_playlist_pane_view(cwin);

	/* Set initial sizes */

	gtk_paned_set_position (GTK_PANED (hpane), cwin->cpref->sidebar_size);

	/* Pack everything into the hpane */

	gtk_paned_pack1 (GTK_PANED (hpane), browse_mode, FALSE, TRUE);
	gtk_paned_pack2 (GTK_PANED (hpane), playlist_pane, TRUE, FALSE);

	/* Store references*/

	cwin->paned = hpane;

	return hpane;
}

static void
buffering_cb (PraghaBackend *backend, gint percent, gpointer user_data)
{
	GtkProgressBar *track_progress_bar = user_data;
	gtk_progress_bar_set_fraction(track_progress_bar, (gdouble)percent/100);
}

GtkWidget* create_playing_box(struct con_win *cwin)
{
	GtkWidget *now_playing_label,*track_length_label,*track_time_label;
	GtkWidget *track_progress_bar;
	GtkWidget *track_length_align, *track_time_align, *track_progress_align, *vbox_align;
	GtkWidget *pack_vbox, *playing_hbox, *time_hbox;
	GtkWidget *track_length_event_box, *track_playing_event_box;

	#if GTK_CHECK_VERSION (3, 0, 0)
	GtkWidget *track_progress_event_box;
	#endif
 
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *ntag_lastfm_button;
	#endif

	playing_hbox = gtk_hbox_new(FALSE, 2);

	track_playing_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_playing_event_box), FALSE);

	g_signal_connect (G_OBJECT(track_playing_event_box), "button-press-event",
			G_CALLBACK(edit_tags_playing_event), cwin);

	now_playing_label = gtk_label_new(NULL);
	gtk_label_set_ellipsize (GTK_LABEL(now_playing_label), PANGO_ELLIPSIZE_END);
	gtk_label_set_markup(GTK_LABEL(now_playing_label),_("<b>Not playing</b>"));
	gtk_misc_set_alignment (GTK_MISC(now_playing_label), 0, 1);

	gtk_container_add(GTK_CONTAINER(track_playing_event_box), now_playing_label);

	#ifdef HAVE_LIBCLASTFM
	ntag_lastfm_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(ntag_lastfm_button), GTK_RELIEF_NONE);
	gtk_button_set_image(GTK_BUTTON(ntag_lastfm_button),
			     gtk_image_new_from_stock(GTK_STOCK_SPELL_CHECK,
						      GTK_ICON_SIZE_MENU));
	gtk_widget_set_tooltip_text(GTK_WIDGET(ntag_lastfm_button), _("Last.fm suggested a tag correction"));
	g_signal_connect(G_OBJECT(ntag_lastfm_button), "clicked",
	                 G_CALLBACK(edit_tags_corrected_by_lastfm), cwin);
	#endif

	gtk_box_pack_start(GTK_BOX(playing_hbox),
			   GTK_WIDGET(track_playing_event_box),
			   TRUE, TRUE, 0);
	#ifdef HAVE_LIBCLASTFM
	gtk_box_pack_start(GTK_BOX(playing_hbox),
			   GTK_WIDGET(ntag_lastfm_button),
			   FALSE, FALSE, 0);
	#endif

	time_hbox = gtk_hbox_new(FALSE, 2);

	/* Setup track progress */

	track_progress_align = gtk_alignment_new(0, 0.5, 1, 0);

	#if GTK_CHECK_VERSION (3, 0, 0)
	track_progress_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_progress_event_box), FALSE);
	#endif

	track_progress_bar = gtk_progress_bar_new();

	gtk_widget_set_size_request(GTK_WIDGET(track_progress_bar), -1, 12);

	#if GTK_CHECK_VERSION (3, 0, 0)
	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_event_box);
	gtk_container_add(GTK_CONTAINER(track_progress_event_box), track_progress_bar);

	g_signal_connect(G_OBJECT(track_progress_event_box), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);
	#else
	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_bar);
	gtk_widget_set_events(track_progress_bar, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(G_OBJECT(track_progress_bar), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);
	#endif
	g_signal_connect (cwin->backend, "buffering", G_CALLBACK(buffering_cb), track_progress_bar);

	track_time_label = gtk_label_new(NULL);
	track_length_label = gtk_label_new(NULL);

	track_time_align = gtk_alignment_new(1, 0.5, 0, 0);
	track_length_align = gtk_alignment_new(0, 0.5, 0, 0);

	gtk_container_add(GTK_CONTAINER(track_time_align), track_time_label);
	gtk_container_add(GTK_CONTAINER(track_length_align), track_length_label);

	gtk_label_set_markup(GTK_LABEL(track_length_label),"<small>--:--</small>");
	gtk_label_set_markup(GTK_LABEL(track_time_label),"<small>00:00</small>");

	track_length_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(track_length_event_box), FALSE);

	g_signal_connect (G_OBJECT(track_length_event_box), "button-press-event",
			G_CALLBACK(timer_remaining_mode_change), cwin);
	gtk_container_add(GTK_CONTAINER(track_length_event_box), track_length_align);

	cwin->track_progress_bar = 	track_progress_bar;
	cwin->now_playing_label = 	now_playing_label;
	#ifdef HAVE_LIBCLASTFM
	cwin->ntag_lastfm_button =	ntag_lastfm_button;
	#endif
	cwin->track_time_label =	track_time_label;
	cwin->track_length_label = 	track_length_label;

	gtk_box_pack_start(GTK_BOX(time_hbox),
			   GTK_WIDGET(track_time_align),
			   FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(time_hbox),
 			   GTK_WIDGET(track_progress_align),
 			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(time_hbox),
			   GTK_WIDGET(track_length_event_box),
			   FALSE, FALSE, 3);

	pack_vbox = gtk_vbox_new(FALSE, 1);

	gtk_box_pack_start(GTK_BOX(pack_vbox),
			   GTK_WIDGET(playing_hbox),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pack_vbox),
			   GTK_WIDGET(time_hbox),
			   FALSE, FALSE, 0);

	vbox_align = gtk_alignment_new(0.5, 0.5, 1, 0);
	gtk_container_add(GTK_CONTAINER(vbox_align), pack_vbox);

	return vbox_align;
}

GtkWidget* create_info_box(struct con_win *cwin)
{
	GtkWidget *info_box;

	info_box = gtk_vbox_new(FALSE, 0);

	cwin->info_box = info_box;

	return info_box;
}

gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin)
{
	if(cwin->cpref->close_to_tray) {
		if(cwin->cpref->show_icon_tray &&
		   gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon)))
			toogle_main_window(cwin, FALSE);
		else
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
	}
	else {
		exit_pragha(widget, cwin);
	}
	return TRUE;
}

void toogle_main_window (struct con_win *cwin, gboolean ignoreActivity)
{
	gint x = 0, y = 0;

	if (gtk_widget_get_visible (cwin->mainwindow)) {
		if (ignoreActivity || gtk_window_is_active (GTK_WINDOW(cwin->mainwindow))){
			gtk_window_get_position (GTK_WINDOW(cwin->mainwindow), &x, &y);
			gtk_widget_hide (GTK_WIDGET(cwin->mainwindow));
			gtk_window_move (GTK_WINDOW(cwin->mainwindow), x ,y);
		}
		else gtk_window_present (GTK_WINDOW(cwin->mainwindow));
	}
	else {
		gtk_widget_show (GTK_WIDGET(cwin->mainwindow));
	}
}

void mainwindow_add_widget_to_info_box(struct con_win *cwin, GtkWidget *widget)
{
	gtk_container_add(GTK_CONTAINER(cwin->info_box), widget);
}

static void pixbufs_free (struct pixbuf *pixbuf)
{
	if (pixbuf->pixbuf_app)
		g_object_unref(pixbuf->pixbuf_app);

	g_slice_free(struct pixbuf, pixbuf);
}

void gui_free (struct con_win *cwin)
{
	const gchar *user_config_dir;
	gchar *pragha_accels_path = NULL;

	/* Save menu accelerators edited */

	user_config_dir = g_get_user_config_dir();
	pragha_accels_path = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha/accels.scm", NULL);
	gtk_accel_map_save (pragha_accels_path);

	/* Free memory */

	g_object_unref(cwin->clibrary->library_store);

	pixbufs_free(cwin->pixbuf);
	cwin->pixbuf = NULL;
	gtk_widget_destroy(GTK_WIDGET(cwin->albumart));
	g_free(pragha_accels_path);
}

static void
backend_error_dialog_response_cb (GtkDialog *dialog, gint response, struct con_win *cwin)
{
	switch (response) {
		case GTK_RESPONSE_APPLY: {
			pragha_advance_playback (cwin);
			break;
		}
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_DELETE_EVENT:
		default: {
			pragha_backend_stop (cwin->backend);
			break;
		}
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
gui_backend_error_show_dialog_cb (PraghaBackend *backend, const GError *error, gpointer user_data)
{
	GtkWidget *dialog;
	gchar *file = NULL;

	struct con_win *cwin = user_data;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "file", &file,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (cwin->mainwindow),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_NONE,
					_("<b>Error playing current track.</b>\n(%s)\n<b>Reason:</b> %s"),
					file, error->message);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_STOP, GTK_RESPONSE_ACCEPT);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_NEXT, GTK_RESPONSE_APPLY);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY);

	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(backend_error_dialog_response_cb),
			 cwin);

	gtk_widget_show_all(dialog);

	g_free(file);
}

void
gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin)
{
	update_current_playlist_view_new_track (cwin->cplaylist, backend);
}
