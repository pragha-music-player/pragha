/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

gchar *main_menu_xml = "<ui>							\
	<menubar name=\"Menubar\">						\
		<menu action=\"FileMenu\">					\
			<menuitem action=\"Add files\"/>			\
			<menuitem action=\"Play audio CD\"/>			\
			<separator/>						\
			<menuitem action=\"Prev\"/>				\
			<menuitem action=\"Play_pause\"/>			\
			<menuitem action=\"Stop\"/>				\
			<menuitem action=\"Next\"/>				\
			<separator/>						\
			<menuitem action=\"Properties\"/>			\
			<separator/>						\
			<menuitem action=\"Quit\"/>				\
		</menu>								\
		<menu action=\"EditMenu\">					\
			<menuitem action=\"Remove\"/>		    		\
			<menuitem action=\"Crop\"/>		    		\
			<menuitem action=\"Clear playlist\"/>	    		\
			<separator/>				    		\
			<menuitem action=\"Save selection\"/>			\
			<menuitem action=\"Save playlist\"/>			\
			<separator/>						\
			<menuitem action=\"Shuffle\"/>				\
			<menuitem action=\"Repeat\"/>				\
			<separator/>						\
			<menuitem action=\"Preferences\"/>			\
		</menu>								\
		<menu action=\"ViewMenu\">					\
			<menuitem action=\"Fullscreen\"/>			\
			<separator/>						\
			<menu action=\"Lateral panel\">				\
				<menuitem action=\"Library\"/>			\
				<menuitem action=\"Playlists\"/>		\
			</menu>							\
			<menuitem action=\"Status bar\"/>			\
			<separator/>						\
			<menuitem action=\"Jump to playing song\"/>		\
		</menu>								\
		<menu action=\"ToolsMenu\">					\
			<menuitem action=\"Add the library\"/>	    		\
			<separator/>						\
			<menuitem action=\"Search lyric\"/>			\
			<separator/>						\
			<menuitem action=\"Search in playlist\"/>		\
			<separator/>						\
			<menuitem action=\"Rescan library\"/>			\
			<menuitem action=\"Update library\"/>			\
			<separator/>						\
			<menuitem action=\"Statistics\"/>			\
		</menu>								\
		<menu action=\"HelpMenu\">					\
			<menuitem action=\"Home\"/>				\
			<menuitem action=\"Community\"/>			\
			<menuitem action=\"Wiki\"/>				\
			<separator/>						\
			<menuitem action=\"About\"/>				\
		</menu>								\
	</menubar>								\
</ui>";

gchar *cp_context_menu_xml = "<ui>		    				\
	<popup>					    				\
	<menuitem action=\"Queue\"/>						\
	<menuitem action=\"Enqueue\"/>						\
	<separator/>				    				\
	<menuitem action=\"Remove\"/>		    				\
	<menuitem action=\"Crop\"/>		    				\
	<menuitem action=\"Clear playlist\"/>	    				\
	<separator/>				    				\
	<menuitem action=\"Save selection\"/>					\
	<menuitem action=\"Save playlist\"/>					\
	<separator/>				    				\
	<menuitem action=\"Properties\"/>	    				\
	<menuitem action=\"Edit tags\"/>					\
	<separator/>				    				\
	</popup>				    				\
	</ui>";

gchar *playlist_tree_context_menu_xml = "<ui>	\
	<popup>					\
	<menuitem action=\"Add to playlist\"/>	\
	<menuitem action=\"Replace playlist\"/>	\
	<separator/>				\
	<menuitem action=\"Delete\"/>		\
	<menuitem action=\"Export\"/>		\
	</popup>				\
	</ui>";

gchar *library_tree_context_menu_xml = "<ui>		\
	<popup>						\
	<menuitem action=\"Add to playlist\"/>		\
	<menuitem action=\"Replace playlist\"/>		\
	<separator/>					\
	<menuitem action=\"Edit\"/>			\
	<menuitem action=\"Delete (From library)\"/>	\
	<menuitem action=\"Delete (From HDD)\"/>	\
	</popup>					\
	</ui>";

gchar *library_page_context_menu_xml = "<ui>			\
	<popup>							\
	<menuitem action=\"Expand library\"/>			\
	<menuitem action=\"Collapse library\"/>			\
	<separator/>						\
	<menuitem action=\"folder_file\"/>			\
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

gchar *systray_menu_xml = "<ui>				\
	<popup>						\
		<menuitem action=\"About\"/>		\
		<separator/>				\
		<menuitem action=\"Add files\"/>	\
		<menuitem action=\"Play audio CD\"/>	\
		<separator/>				\
		<menuitem action=\"Prev\"/>		\
		<menuitem action=\"Play_Pause\"/>	\
		<menuitem action=\"Stop\"/>		\
		<menuitem action=\"Next\"/>		\
		<separator/>				\
		<menuitem action=\"Properties\"/>	\
		<separator/>				\
		<menuitem action=\"Quit\"/>		\
	</popup>					\
	</ui>";

GtkActionEntry main_aentries[] = {
	{"FileMenu", NULL, N_("_File")},
	{"EditMenu", NULL, N_("_Edit")},
	{"ViewMenu", NULL, N_("_View")},
	{"ToolsMenu", NULL, N_("_Tools")},
	{"HelpMenu", NULL, N_("_Help")},
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 NULL, N_("Open a media file"), G_CALLBACK(open_file_action)},
	{"Play audio CD", GTK_STOCK_CDROM, N_("_Play audio CD"),
	 NULL, "Play a audio CD", G_CALLBACK(play_audio_cd_action)},
	{"Prev", GTK_STOCK_MEDIA_PREVIOUS, N_("Prev track"),
	 "<Alt>Left", "Prev track", G_CALLBACK(prev_action)},
	{"Play_pause", GTK_STOCK_MEDIA_PLAY, N_("Play / Pause"),
	 "<Control>space", "Play / Pause", G_CALLBACK(play_pause_action)},
	{"Stop", GTK_STOCK_MEDIA_STOP, N_("Stop"),
	 NULL, "Stop", G_CALLBACK(stop_action)},
	{"Next", GTK_STOCK_MEDIA_NEXT, N_("Next track"),
	 "<Alt>Right", "Next track", G_CALLBACK(next_action)},
	{"Properties", GTK_STOCK_PROPERTIES, N_("_Properties"),
	 NULL, "Properties", G_CALLBACK(track_properties_current_playing_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 "<Control>Q", "Quit pragha", G_CALLBACK(quit_action)},
	{"Add the library", GTK_STOCK_ADD, N_("_Add the library"),
	 NULL, "Add all the library", G_CALLBACK(add_all_action)},
	{"Remove", GTK_STOCK_REMOVE, N_("Remove"),
	 NULL, "Delete this entry", G_CALLBACK(remove_current_playlist)},
	{"Crop", GTK_STOCK_REMOVE, N_("Crop"),
	 "<Control>C", "Crop the playlist", G_CALLBACK(crop_current_playlist)},
	{"Clear playlist", GTK_STOCK_CLEAR, N_("Clear playlist"),
	 "<Control>L", "Clear the playlist", G_CALLBACK(clear_current_playlist)},
	{"Save selection", GTK_STOCK_SAVE_AS, N_("Save selection"),
	 "<control><shift>s", "Save selected tracks as playlist", G_CALLBACK(save_selected_playlist)},
	{"Save playlist", GTK_STOCK_SAVE, N_("Save playlist"),
	 "<control>s", "Save the complete playlist", G_CALLBACK(save_current_playlist)},
	{"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"),
	 "<Control>P", "Set preferences", G_CALLBACK(pref_action)},
	{"Lateral panel", NULL, N_("Lateral _panel")},
	{"Jump to playing song", GTK_STOCK_JUMP_TO, N_("Jump to playing song"),
	 "<Control>J", "Jump to playing song", G_CALLBACK(jump_to_playing_song_action)},
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "<Control>Y", "Search lyric", G_CALLBACK(lyric_action)},
	{"Search in playlist", GTK_STOCK_FIND, N_("_Search in playlist"),
	 "<Control>F", "Search in playlist", G_CALLBACK(search_playlist_action)},
	{"Rescan library", GTK_STOCK_EXECUTE, N_("_Rescan library"),
	 NULL, "Rescan library", G_CALLBACK(rescan_library_action)},
	{"Update library", GTK_STOCK_EXECUTE, N_("_Update library"),
	 NULL, "Update library", G_CALLBACK(update_library_action)},
	{"Statistics", GTK_STOCK_INFO, N_("_Statistics"),
	 NULL, "Statistics", G_CALLBACK(statistics_action)},
	{"About", GTK_STOCK_ABOUT, N_("About"),
	 NULL, "About pragha", G_CALLBACK(about_action)},
	{"Home", GTK_STOCK_HOME, N_("Homepage"),
	 NULL, "Homepage", G_CALLBACK(home_action)},
	{"Community", GTK_STOCK_INFO, N_("Community"),
	 NULL, "Forum or pragha", G_CALLBACK(community_action)},
	{"Wiki", GTK_STOCK_YES, N_("Wiki"),
	 NULL, "Wiki of pragha", G_CALLBACK(wiki_action)},
};

GtkToggleActionEntry toggles_entries[] = {
	{"Shuffle", NULL, N_("_Shuffle"),
	 "<Control>U", "Shuffle Songs", G_CALLBACK(shuffle_action),
	 FALSE},
	{"Repeat", NULL, N_("_Repeat"),
	 "<Control>R", "Repeat Songs", G_CALLBACK(repeat_action), 
	 FALSE},
	{"Fullscreen", NULL, N_("_Fullscreen"),
	 "F11", "Switch between full screen and windowed mode", G_CALLBACK(fullscreen_action),
	FALSE},
	{"Library", NULL, N_("Library"),
	 NULL, "Library", G_CALLBACK(library_pane_action),
	TRUE},
	{"Playlists", NULL, N_("Playlists"),
	 NULL, "Playlists", G_CALLBACK(playlists_pane_action),
	FALSE},
	{"Status bar", NULL, N_("Status bar"),
	 NULL, "Status bar", G_CALLBACK(status_bar_action),
	TRUE}
};

GtkActionEntry cp_context_aentries[] = {
	{"Queue", GTK_STOCK_ADD, N_("Add to playback queue"),
	 NULL, "Add to playback queue", G_CALLBACK(queue_current_playlist)},
	{"Enqueue", GTK_STOCK_REMOVE, N_("Remove to playback queue"),
	 NULL, "Remove to playback queue", G_CALLBACK(enqueue_current_playlist)},
	{"Remove", GTK_STOCK_REMOVE, N_("Remove"),
	 NULL, "Delete this entry", G_CALLBACK(remove_current_playlist)},
	{"Crop", GTK_STOCK_REMOVE, N_("Crop"),
	 NULL, "Crop the playlist", G_CALLBACK(crop_current_playlist)},
	{"Edit tags", GTK_STOCK_INFO, N_("Edit tags"),
	 NULL, "Edit tag for this track", G_CALLBACK(edit_tags_current_playlist)},
	{"Properties", GTK_STOCK_PROPERTIES, N_("Properties"),
	 NULL, "Track Properties", G_CALLBACK(track_properties_current_playlist_action)},
	{"Save selection", GTK_STOCK_SAVE, N_("Save selection"),
	 NULL, "Save selected tracks as playlist", G_CALLBACK(save_selected_playlist)},
	{"Save playlist", GTK_STOCK_SAVE, N_("Save playlist"),
	 NULL, "Save the complete playlist", G_CALLBACK(save_current_playlist)},
	{"Clear playlist", GTK_STOCK_CLEAR, N_("Clear playlist"),
	 NULL, "Clear the playlist", G_CALLBACK(clear_current_playlist)}
};

GtkActionEntry playlist_tree_context_aentries[] = {
	{"Add to playlist", GTK_STOCK_ADD, N_("_Add to playlist"),
	 NULL, "Add to playlist", G_CALLBACK(playlist_tree_add_to_playlist_action)},
	{"Replace playlist", NULL, N_("_Replace playlist"),
	 NULL, "Replace playlist", G_CALLBACK(playlist_tree_replace_playlist)},
	{"Delete", GTK_STOCK_REMOVE, N_("Delete"),
	 NULL, "Delete", G_CALLBACK(playlist_tree_delete)},
	{"Export", GTK_STOCK_SAVE, N_("Export"),
	 NULL, "Export", G_CALLBACK(playlist_tree_export)}
};

GtkActionEntry library_tree_context_aentries[] = {
	{"Add to playlist", GTK_STOCK_ADD, N_("_Add to playlist"),
	 NULL, "Add to playlist", G_CALLBACK(library_tree_add_to_playlist_action)},
	{"Replace playlist", NULL, N_("_Replace playlist"),
	 NULL, "Replace playlist", G_CALLBACK(library_tree_replace_playlist)},
	{"Edit", GTK_STOCK_EDIT, N_("Edit tags"),
	 NULL, "Edit tags", G_CALLBACK(library_tree_edit_tags)},
	{"Delete (From library)", GTK_STOCK_REMOVE, N_("Delete from library"),
	 NULL, "Delete from library", G_CALLBACK(library_tree_delete_db)},
	{"Delete (From HDD)", GTK_STOCK_REMOVE, N_("Delete from HDD"),
	 NULL, "Delete from HDD", G_CALLBACK(library_tree_delete_hdd)}
};

GtkActionEntry library_page_context_aentries[] = {
	{"Expand library", GTK_STOCK_ADD, N_("_Expand library"),
	 NULL, "Expand the library", G_CALLBACK(expand_all_action)},
	{"Collapse library", GTK_STOCK_REMOVE, N_("_Collapse library"),
	 NULL, "Collapse the library", G_CALLBACK(collapse_all_action)},
	{"folder_file", GTK_STOCK_REFRESH, N_("Folder / File"),
	 NULL, "Folder / File", G_CALLBACK(folder_file_library_tree)},
	{"artist", GTK_STOCK_REFRESH, N_("Artist"),
	 NULL, "Artist", G_CALLBACK(artist_library_tree)},
	{"album", GTK_STOCK_REFRESH, N_("Album"),
	 NULL, "Album", G_CALLBACK(album_library_tree)},
	{"genre", GTK_STOCK_REFRESH, N_("Genre"),
	 NULL, "Genre", G_CALLBACK(genre_library_tree)},
	{"artist_album", GTK_STOCK_REFRESH, N_("Artist / Album"),
	 NULL, "Artist / Album", G_CALLBACK(artist_album_library_tree)},
	{"genre_album", GTK_STOCK_REFRESH, N_("Genre / Album"),
	 NULL, "Genre / Album", G_CALLBACK(genre_album_library_tree)},
	{"genre_artist", GTK_STOCK_REFRESH, N_("Genre / Artist"),
	 NULL, "Genre / Artist", G_CALLBACK(genre_artist_library_tree)},
	{"genre_artist_album", GTK_STOCK_REFRESH, N_("Genre / Artist / Album"),
	 NULL, "Genre / Artist / Album", G_CALLBACK(genre_artist_album_library_tree)}
};

GtkActionEntry systray_menu_aentries[] = {
	{"About", GTK_STOCK_ABOUT, N_("About"),
	 NULL, NULL, G_CALLBACK(about_action)},
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 NULL, NULL, G_CALLBACK(open_file_action)},
	{"Play audio CD", GTK_STOCK_CDROM, N_("_Play audio CD"),
	 NULL, "Play a audio CD", G_CALLBACK(play_audio_cd_action)},
	{"Prev", GTK_STOCK_MEDIA_PREVIOUS, N_("Prev Track"),
	 NULL, "Prev Track", G_CALLBACK(prev_action)},
	{"Play_Pause", GTK_STOCK_MEDIA_PLAY, N_("Play / Pause"),
	 NULL, "Play / Pause", G_CALLBACK(play_pause_action)},
	{"Stop", GTK_STOCK_MEDIA_STOP, N_("Stop"),
	 NULL, "Stop", G_CALLBACK(stop_action)},
	{"Next", GTK_STOCK_MEDIA_NEXT, N_("Next Track"),
	 NULL, "Next Track", G_CALLBACK(next_action)},
	{"Properties", GTK_STOCK_PROPERTIES, N_("_Properties"),
	 NULL, "Properties", G_CALLBACK(track_properties_current_playing_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 NULL, "Quit", G_CALLBACK(systray_quit)}
};

GtkTargetEntry tentries[] = {
	{"LOCATION_ID", GTK_TARGET_SAME_APP, TARGET_LOCATION_ID},
	{"PLAYLIST", GTK_TARGET_SAME_APP, TARGET_PLAYLIST},
	{"text/uri-list", GTK_TARGET_OTHER_APP, TARGET_URI_LIST},
	{"text/plain", GTK_TARGET_OTHER_APP, TARGET_PLAIN_TEXT}
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

static GtkWidget* create_library_tree(struct con_win *cwin)
{
	GError *error = NULL;
	GtkWidget *library_tree;
	GtkTreeModel *library_filter_tree;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	gint width, height;

	/* Create the tree store */

	store = gtk_tree_store_new(N_L_COLUMNS,
				   GDK_TYPE_PIXBUF, /* Pixbuf */
				   G_TYPE_STRING,   /* Node */
				   G_TYPE_INT,      /* Node type : Artist / Album / Track */
				   G_TYPE_INT,      /* Location id (valid only for Track) */
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

	/* Load pixbufs */

	gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height);

	cwin->pixbuf->pixbuf_artist = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
									"/artist.png",
									width,
									height,
									TRUE,
									&error);
	if (!cwin->pixbuf->pixbuf_artist) {
		g_warning("Unable to load artist png : %s", error->message);
		g_error_free(error);
		error = NULL;
	}

	cwin->pixbuf->pixbuf_album = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
							      "media-optical",
							      width,
							      0,
							      &error);
	if (!cwin->pixbuf->pixbuf_album) {
		g_warning("Unable to load album png : %s", error->message);
		g_error_free(error);
		error = NULL;
	}

	cwin->pixbuf->pixbuf_track = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
								       "/track.png",
								       width,
								       height,
								       TRUE,
								       &error);
	if (!cwin->pixbuf->pixbuf_track) {
		g_warning("Unable to load track png : %s", error->message);
		g_error_free(error);
		error = NULL;
	}

	cwin->pixbuf->pixbuf_genre = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
								       "/genre.png",
								       width,
								       height,
								       TRUE,
								       &error);
	if (!cwin->pixbuf->pixbuf_genre) {
		g_warning("Unable to load genre png : %s", error->message);
		g_error_free(error);
		error = NULL;
	}

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

	cwin->library_store = store;
	cwin->library_tree = library_tree;
	g_signal_connect(G_OBJECT(library_tree), "row-activated",
			 G_CALLBACK(library_tree_row_activated_cb), cwin);

	/* Create right click popup menu */

	cwin->library_tree_context_menu = create_library_tree_context_menu(library_tree,
									   cwin);

	/* Signal handler for right-clicking and selection */
 
 	g_signal_connect(G_OBJECT(GTK_WIDGET(library_tree)), "button-press-event",
			 G_CALLBACK(library_tree_button_press_cb), cwin);
 
	g_signal_connect(G_OBJECT(GTK_WIDGET(library_tree)), "button-release-event",
			 G_CALLBACK(library_tree_button_release_cb), cwin);

	g_object_unref(library_filter_tree);
	
	return library_tree;
}

/*****************/
/* Playlist Tree */
/*****************/

static GtkUIManager* create_playlist_tree_context_menu(GtkWidget *playlist_tree,
						       struct con_win *cwin)
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

static GtkWidget* create_playlist_tree(struct con_win *cwin)
{
	GtkWidget *playlist_tree;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	GError *error = NULL;

	/* Create the tree store */

	store = gtk_tree_store_new(N_PL_COLUMNS,
				   GDK_TYPE_PIXBUF, /* Pixbuf */
				   G_TYPE_STRING);  /* Playlist name */

	/* Create the tree view */

	playlist_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(playlist_tree), FALSE);
	gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(playlist_tree), TRUE);

	/* Selection mode is multiple */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(playlist_tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create column and cell renderers */

	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "pixbuf", P_PIXBUF,
					    NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "text", P_PLAYLIST,
					    NULL);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(playlist_tree), column);

	/* Create pixbufs */

	cwin->pixbuf->pixbuf_dir = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
							    "gtk-directory",
							    16, 0, &error);
	if( error != NULL )
		g_warning("Unable to load gtk-directory icon, err : %s", error->message);

	cwin->pixbuf->pixbuf_file = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
							     "gtk-file",
							     16, 0, &error);
	if( error != NULL )
		g_warning("Unable to load gtk-file icon, err : %s", error->message);

	/* Create right click popup menu */

	cwin->playlist_tree_context_menu = create_playlist_tree_context_menu(playlist_tree,
									     cwin);

	/* Signal handler for right-clicking */

	g_signal_connect(G_OBJECT(playlist_tree), "row-activated",
			 G_CALLBACK(playlist_tree_row_activated_cb), cwin);

	g_signal_connect(G_OBJECT(GTK_WIDGET(playlist_tree)), "button-press-event",
			 G_CALLBACK(playlist_tree_button_press_cb), cwin);
	g_signal_connect(G_OBJECT(GTK_WIDGET(playlist_tree)), "button-release-event",
			 G_CALLBACK(playlist_tree_button_release_cb), cwin);

	g_object_unref(store);

	cwin->playlist_tree = playlist_tree;

	return playlist_tree;
}

/***************************/
/* Left pane (Browse mode) */
/***************************/

static GtkUIManager* create_library_page_context_menu(GtkWidget *library_page,
						      struct con_win *cwin)
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

static GtkWidget * create_toggles_buttons(struct con_win *cwin)
{
	GtkWidget *vbox_btns;
	GtkWidget *w, *l;

	vbox_btns = gtk_vbox_new(FALSE, 0);
	
	w = gtk_toggle_button_new_with_mnemonic( NULL );
	gtk_button_set_relief( GTK_BUTTON( w ), GTK_RELIEF_NONE );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w),TRUE);
		
	l = gtk_label_new_with_mnemonic(_("Library"));
	gtk_label_set_angle(GTK_LABEL(l), 90);
	gtk_container_add(GTK_CONTAINER(w),GTK_WIDGET(l));

	gtk_box_pack_start( GTK_BOX( vbox_btns ), w, FALSE, FALSE, 0 );
	cwin->toggle_lib=w;

	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(cwin->toggle_lib)), "toggled", G_CALLBACK( toggled_cb ), cwin );
	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(cwin->toggle_lib)), "button-press-event",
			G_CALLBACK(library_page_right_click_cb), cwin);

	/* Create library page context menu */
	cwin->library_page_context_menu = create_library_page_context_menu(cwin->toggle_lib, cwin);

	w = gtk_toggle_button_new_with_mnemonic( NULL );
	gtk_button_set_relief( GTK_BUTTON( w ), GTK_RELIEF_NONE );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w),FALSE);

	l = gtk_label_new_with_mnemonic(_("Playlists"));
	gtk_label_set_angle(GTK_LABEL(l), 90);
	gtk_container_add(GTK_CONTAINER(w),GTK_WIDGET(l));
	gtk_box_pack_start( GTK_BOX( vbox_btns ), w, FALSE, FALSE, 0 );
	cwin->toggle_playlists=w;

	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(cwin->toggle_playlists)), "toggled",
			G_CALLBACK( toggled_cb ), cwin );
	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(cwin->toggle_playlists)), "button-press-event",
			G_CALLBACK(library_page_right_click_cb), cwin);

	l = gtk_label_new_with_mnemonic(_("Pragha Music Manager"));
	gtk_label_set_angle(GTK_LABEL(l), 90);
	gtk_misc_set_alignment (GTK_MISC(l),0.5,1);

	gtk_box_pack_start( GTK_BOX( vbox_btns ), l, TRUE, TRUE, 0 );

	return vbox_btns;
}

static GtkWidget* create_browse_mode_view(struct con_win *cwin)
{
	GtkWidget *browse_mode;
	GtkWidget *vbox_lib, *vbox_page;
	GtkWidget *library_tree, *playlist_tree;
	GtkWidget *playlists_tree_scroll, *library_tree_scroll;
	GtkWidget *order_selector, *search_bar;

	browse_mode = gtk_notebook_new();

	vbox_lib = gtk_vbox_new(FALSE, 0);
	library_tree_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(library_tree_scroll),
					GTK_SHADOW_IN);
	search_bar = create_search_bar(cwin);
	order_selector = create_combo_order (cwin);
	library_tree = create_library_tree(cwin);

	gtk_container_add(GTK_CONTAINER(library_tree_scroll), library_tree);

	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   order_selector,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   search_bar,
			   FALSE,
			   FALSE,
			   2);

	gtk_box_pack_start(GTK_BOX(vbox_lib),
			   library_tree_scroll,
			   TRUE,
			   TRUE,
			   0);

	vbox_page = gtk_vbox_new(FALSE, 0);
	playlists_tree_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(playlists_tree_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(playlists_tree_scroll),
					GTK_SHADOW_IN);
	playlist_tree = create_playlist_tree(cwin);
	gtk_container_add(GTK_CONTAINER(playlists_tree_scroll), playlist_tree);

	/* Append the notebook page widgets */

	gtk_notebook_append_page(GTK_NOTEBOOK(browse_mode),
				 vbox_lib,
				 NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(browse_mode),
				 playlists_tree_scroll,
				 NULL);

	cwin->browse_mode = browse_mode;

	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(browse_mode), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK(browse_mode), FALSE);

	/* Signal handler for right-clicking on a page */

	g_signal_connect(G_OBJECT(GTK_NOTEBOOK(browse_mode)), "button-press-event",
			 G_CALLBACK(library_page_right_click_cb), cwin);

	gtk_notebook_popup_disable(GTK_NOTEBOOK(browse_mode));

	return browse_mode;
}

/*********************************/
/* Right pane (Current playlist) */
/*********************************/

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

static void create_current_playlist_columns(GtkWidget *current_playlist,
					    struct con_win *cwin)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *label_track,
		*label_title,
		*label_artist,
		*label_album,
		*label_genre,
		*label_bitrate,
		*label_year,
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
	label_length = gtk_label_new(_("Length"));
	label_filename = gtk_label_new(_("Filename"));

	/* Column : Queue Bubble*/

	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_bubble_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", P_QUEUE);
	gtk_tree_view_column_add_attribute (column, renderer, "show-bubble", P_BUBBLE);
	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(current_playlist), column);

	/* Column : Track No */

	renderer = gtk_cell_renderer_text_new();
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

	/* Column : Length */

	renderer = gtk_cell_renderer_text_new();
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

static GtkWidget* create_current_playlist_view(struct con_win *cwin)
{
	GtkWidget *current_playlist_scroll;
	GtkWidget *current_playlist;
	GtkUIManager *cp_context_menu;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;

	/* The scrollbar widget */

	current_playlist_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(current_playlist_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);

	/* Create the tree store */

	store = gtk_list_store_new(N_P_COLUMNS,
				   G_TYPE_POINTER,	/* Pointer to musicobject */
				   G_TYPE_STRING,	/* Queue No String */
				   G_TYPE_BOOLEAN,	/* Show Bublle Queue */
				   G_TYPE_STRING,	/* Tag : Track No */
				   G_TYPE_STRING,	/* Tag : Title */
				   G_TYPE_STRING,	/* Tag : Artist */
				   G_TYPE_STRING,	/* Tag : Album */
				   G_TYPE_STRING,	/* Tag : Genre */
				   G_TYPE_STRING,	/* Tag : Bitrate */
				   G_TYPE_STRING,	/* Tag : Year */
				   G_TYPE_STRING,	/* Tag : Length */
				   G_TYPE_STRING,	/* Filename */
				   G_TYPE_BOOLEAN);	/* Played flag */

	/* Create the tree view */

	current_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(current_playlist));
	sortable = GTK_TREE_SORTABLE(model);

	/* Set the search function for interactive search */

	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(current_playlist),
					    current_playlist_search_compare,
					    cwin,
					    NULL);
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

	gtk_widget_add_events (GTK_WIDGET (current_playlist), GDK_KEY_PRESS_MASK);
	g_signal_connect (G_OBJECT (current_playlist), "key_press_event",
			  G_CALLBACK (current_playlist_key_press), cwin);

	/* Create contextual menus */

	cp_context_menu = create_cp_context_menu(current_playlist, cwin);
	cwin->cp_context_menu = cp_context_menu;
	cwin->header_context_menu = create_header_context_menu(cwin);

	/* Signal handler for right-clicking and selection */

	g_signal_connect(G_OBJECT(GTK_WIDGET(current_playlist)), "button-press-event",
			 G_CALLBACK(current_playlist_button_press_cb), cwin);

	g_signal_connect(G_OBJECT(GTK_WIDGET(current_playlist)), "button-release-event",
			 G_CALLBACK(current_playlist_button_release_cb), cwin);

	/* Store the treeview in the scrollbar widget */

	gtk_container_add(GTK_CONTAINER(current_playlist_scroll), current_playlist);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(current_playlist_scroll),GTK_SHADOW_IN);
	cwin->current_playlist = current_playlist;

	/* Set initial column visibility */

	init_current_playlist_columns(cwin);

	g_object_unref(store);

	return current_playlist_scroll;
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

static void init_dnd(struct con_win *cwin)
{
	/* Source: Library View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->library_tree),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cwin->library_tree)),
			 "drag-begin",
			 G_CALLBACK(dnd_library_tree_begin),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->library_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_library_tree_get),
			 cwin);

	/* Source: Playlist View */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->playlist_tree),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cwin->playlist_tree)),
			 "drag-begin",
			 G_CALLBACK(dnd_playlist_tree_begin),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->playlist_tree),
			 "drag-data-get",
			 G_CALLBACK(dnd_playlist_tree_get),
			 cwin);

	/* Source/Dest: Current Playlist */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cwin->current_playlist),
					       GDK_BUTTON1_MASK,
					       tentries,
					       G_N_ELEMENTS(tentries),
					       GDK_ACTION_COPY | GDK_ACTION_MOVE);

	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(cwin->current_playlist),
					     tentries,
					     G_N_ELEMENTS(tentries),
					     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cwin->current_playlist)),
			 "drag-begin",
			 G_CALLBACK(dnd_current_playlist_begin),
			 cwin);
	g_signal_connect (G_OBJECT(cwin->current_playlist),
			 "drag-data-get",
			 G_CALLBACK (drag_current_playlist_get_data),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->current_playlist),
			 "drag-drop",
			 G_CALLBACK(dnd_current_playlist_drop),
			 cwin);
	g_signal_connect(G_OBJECT(cwin->current_playlist),
			 "drag-data-received",
			 G_CALLBACK(dnd_current_playlist_received),
			 cwin);
}

GtkUIManager* create_systray_menu(struct con_win *cwin)
{
	GtkUIManager *menu = NULL;
	GtkActionGroup *actions;
	GError *error = NULL;

	actions = gtk_action_group_new("Systray Actions");
	menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(menu, systray_menu_xml, -1, &error)) {
		g_critical("Unable to create systray menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(actions,
				     systray_menu_aentries,
				     G_N_ELEMENTS(systray_menu_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(menu));
	gtk_ui_manager_insert_action_group(menu, actions, 0);

	return menu;
}

/********************************/
/* Externally visible functions */
/********************************/

GtkUIManager* create_menu(struct con_win *cwin)
{
	GtkUIManager *main_menu = NULL;
	GtkActionGroup *main_actions;
	GError *error = NULL;

	main_actions = gtk_action_group_new("Main Actions");
	main_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (main_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(main_menu, main_menu_xml, -1, &error)) {
		g_critical("Unable to create main menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(main_actions,
				     main_aentries,
				     G_N_ELEMENTS(main_aentries),
				     (gpointer)cwin);
	gtk_action_group_add_toggle_actions (main_actions, 
					toggles_entries, G_N_ELEMENTS(toggles_entries), 
					cwin);

	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(main_menu));
	gtk_ui_manager_insert_action_group(main_menu, main_actions, 0);

	cwin->bar_context_menu = main_menu;

	return main_menu;
}

GtkWidget* create_main_region(struct con_win *cwin)
{
	GtkWidget *hbox;
	GtkWidget *toggles_note;
	GtkWidget *paned_region;

	hbox = gtk_hbox_new(FALSE, 0);

	toggles_note = create_toggles_buttons(cwin);
	paned_region = create_paned_region(cwin);

	gtk_box_pack_start(GTK_BOX(hbox),
			   toggles_note,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox),
			   paned_region,
			   TRUE,
			   TRUE,
			   0);

	return hbox;
}


GtkWidget* create_paned_region(struct con_win *cwin)
{
	GtkWidget *hpane;
	GtkWidget *browse_mode;
	GtkWidget *current_playlist;

	/* A two paned container */

	hpane = gtk_hpaned_new();

	/* Left pane contains a notebook widget holding the various views */

	browse_mode = create_browse_mode_view(cwin);

	/* Right pane contains the current playlist */

	current_playlist = create_current_playlist_view(cwin);

	/* DnD */

	init_dnd(cwin);

	/* Set initial sizes */

	gtk_widget_set_size_request(GTK_WIDGET(browse_mode), cwin->cpref->sidebar_size, -1);

	/* Pack everything into the hpane */

	gtk_paned_pack1 (GTK_PANED (hpane), browse_mode, FALSE, TRUE);
	gtk_paned_pack2 (GTK_PANED (hpane), current_playlist, TRUE, FALSE);

	cwin->paned = hpane;

	return hpane;
}

GtkWidget* create_panel(struct con_win *cwin)
{
	GtkWidget *left_controls_align, *right_controls_align;
	GtkWidget *playing;
	GtkWidget *vbox_order;
	GtkWidget *hbox_panel, *left_controls, *right_controls;
	GtkWidget *play_button, *stop_button, *prev_button, *next_button;
	GtkWidget *unfull_button, *sep, *shuffle_button, *repeat_button, *vol_button;
	GtkWidget *album_art_frame = NULL;
	GtkObject *vol_adjust;

	hbox_panel = gtk_hbox_new(FALSE, 5);
	vbox_order = gtk_vbox_new(FALSE, 1);
	left_controls = gtk_hbox_new(FALSE, 1);
	right_controls = gtk_hbox_new(FALSE, 1);

	playing = create_playing_box(cwin);

	/* Images for pause and play */

	cwin->pixbuf->image_pause =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);
	cwin->pixbuf->image_play =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);

	g_object_ref(cwin->pixbuf->image_play);
	g_object_ref(cwin->pixbuf->image_pause);

	/* Setup Left control buttons */

	prev_button = gtk_button_new();
	play_button = gtk_button_new();
	stop_button = gtk_button_new();
	next_button = gtk_button_new();

	gtk_button_set_relief(GTK_BUTTON(prev_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(stop_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(next_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(play_button), GTK_RELIEF_NONE);

	gtk_button_set_image(GTK_BUTTON(prev_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(stop_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(next_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));

	gtk_button_set_image(GTK_BUTTON(play_button),
			     cwin->pixbuf->image_play);

	gtk_box_pack_start(GTK_BOX(left_controls),
			   GTK_WIDGET(prev_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_controls),
			   GTK_WIDGET(play_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_controls),
			   GTK_WIDGET(stop_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_controls),
			   GTK_WIDGET(next_button),
			   FALSE, FALSE, 0);

	left_controls_align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(left_controls_align), left_controls);

	/* Setup Right control buttons */

	unfull_button = gtk_button_new_from_stock (GTK_STOCK_LEAVE_FULLSCREEN);
	gtk_button_set_relief(GTK_BUTTON(unfull_button), GTK_RELIEF_NONE);

	sep = gtk_vseparator_new();

	shuffle_button = gtk_toggle_button_new();
	repeat_button = gtk_toggle_button_new();

	gtk_button_set_relief(GTK_BUTTON(shuffle_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(repeat_button), GTK_RELIEF_NONE);
	vol_button = gtk_volume_button_new();
	vol_adjust = gtk_adjustment_new(0, 0, 100, 1, 5, 0);
	gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(vol_button),
					GTK_ADJUSTMENT(vol_adjust));

	gtk_button_set_image(GTK_BUTTON(shuffle_button),
			     gtk_image_new_from_icon_name ("media-playlist-shuffle",
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(repeat_button),
			     gtk_image_new_from_icon_name ("media-playlist-repeat",
						      GTK_ICON_SIZE_LARGE_TOOLBAR));

 	gtk_box_pack_end(GTK_BOX(right_controls),
			   GTK_WIDGET(vol_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(right_controls),
			   GTK_WIDGET(sep),
			   FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(right_controls),
			   GTK_WIDGET(shuffle_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(right_controls),
			   GTK_WIDGET(repeat_button),
			   FALSE, FALSE, 0);
 	gtk_box_pack_end(GTK_BOX(right_controls),
			   GTK_WIDGET(unfull_button),
			   FALSE, FALSE, 0);

	right_controls_align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(right_controls_align), right_controls);

	/* Signal handlers */

	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), cwin);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), cwin);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), cwin);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), cwin);

	g_signal_connect(G_OBJECT(GTK_BUTTON(unfull_button)), "clicked",
			 G_CALLBACK(unfull_button_handler), cwin );
	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(shuffle_button)), "toggled",
			 G_CALLBACK(shuffle_button_handler), cwin );
	g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(repeat_button)), "toggled",
			G_CALLBACK(repeat_button_handler), cwin );
	g_signal_connect(G_OBJECT(vol_button), "value-changed",
			 G_CALLBACK(vol_button_handler), cwin);

	/* Initial state of various widgets from stored preferences */

	gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_button),
				   SCALE_UP_VOL(cwin->cmixer->curr_vol));

	/* References to widgets */

	cwin->hbox_panel = hbox_panel;
	cwin->prev_button = prev_button;
	cwin->play_button = play_button;
	cwin->stop_button = stop_button;
	cwin->next_button = next_button;

	cwin->unfull_button = unfull_button;
	cwin->shuffle_button = shuffle_button;
	cwin->repeat_button = repeat_button;
	cwin->vol_button = vol_button;

	/* Tooltips */

	gtk_widget_set_tooltip_text(GTK_WIDGET(play_button), _("Play / Pause Track"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), _("Previous Track"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), _("Next Track"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(stop_button), _("Stop playback"));

	gtk_widget_set_tooltip_text(GTK_WIDGET(unfull_button), _("Leave Fullscreen"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(shuffle_button), _("Play songs in a random order"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(repeat_button), _("Repeat playback list at the end"));

	/* Pack panel widgets into hbox_panel */

	gtk_box_pack_end(GTK_BOX(hbox_panel),
			   GTK_WIDGET(right_controls_align),
			   FALSE, FALSE, 0);

 	gtk_box_pack_end(GTK_BOX(hbox_panel),
		   GTK_WIDGET(playing),
		   TRUE, TRUE, 0);

	if (cwin->cpref->show_album_art) {
		album_art_frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type (GTK_FRAME(album_art_frame),GTK_SHADOW_NONE);
		gtk_box_pack_end(GTK_BOX(hbox_panel),
				   GTK_WIDGET(album_art_frame),
				   FALSE, FALSE, 0);
		cwin->album_art_frame = album_art_frame;

		unset_album_art(cwin);
	}

	gtk_box_pack_end(GTK_BOX(hbox_panel),
			 GTK_WIDGET(left_controls_align),
			 FALSE, FALSE, 0);

	return hbox_panel;
}

GtkWidget* create_playing_box(struct con_win *cwin)
{
	GtkWidget *now_playing_label,*track_length_label,*track_time_label;
	GtkWidget *track_progress_bar;
	GtkWidget *track_length_align, *track_time_align, *track_progress_align, *vbox_align;
	GtkWidget *new_vbox,*new_hbox; 
	GtkWidget *track_length_event_box;

	now_playing_label = gtk_label_new(NULL);
	gtk_label_set_ellipsize (GTK_LABEL(now_playing_label), PANGO_ELLIPSIZE_END);
	gtk_label_set_markup(GTK_LABEL(now_playing_label),_("<b>Not playing</b>"));
	gtk_misc_set_alignment (GTK_MISC(now_playing_label) , 0, 1);

	new_vbox = gtk_vbox_new(FALSE, 2);
	new_hbox = gtk_hbox_new(FALSE, 1);

	/* Setup track progress */

	track_progress_align = gtk_alignment_new(0, 0.5, 1, 0);
	track_progress_bar = gtk_progress_bar_new();

        gtk_widget_set_size_request(GTK_WIDGET(track_progress_bar),
                                      -1,
                                      12);

	gtk_container_add(GTK_CONTAINER(track_progress_align), track_progress_bar);

	gtk_widget_set_events(track_progress_bar, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(G_OBJECT(track_progress_bar), "button-press-event",
			 G_CALLBACK(track_progress_change_cb), cwin);

	track_time_label = gtk_label_new(NULL);
	track_length_label = gtk_label_new(NULL);

	track_time_align = gtk_alignment_new(1, 0.5, 0, 0);
	track_length_align = gtk_alignment_new(0, 0.5, 0, 0);

	gtk_container_add(GTK_CONTAINER(track_time_align), track_time_label);
	gtk_container_add(GTK_CONTAINER(track_length_align), track_length_label);

	gtk_label_set_markup(GTK_LABEL(track_length_label),"<small>--:--</small>");
	gtk_label_set_markup(GTK_LABEL(track_time_label),"<small>00:00</small>");

	track_length_event_box = gtk_event_box_new();
	gtk_signal_connect( GTK_OBJECT( track_length_event_box ), "button-press-event",
			G_CALLBACK(timer_remaining_mode_change), cwin );
	gtk_container_add(GTK_CONTAINER(track_length_event_box), track_length_align);

	cwin->track_progress_bar = 	track_progress_bar;
	cwin->now_playing_label = 	now_playing_label;
	cwin->track_time_label =	track_time_label;
	cwin->track_length_label = 	track_length_label;

	gtk_box_pack_start(GTK_BOX(new_hbox),
			   GTK_WIDGET(track_time_align),
			   FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(new_hbox),
 			   GTK_WIDGET(track_progress_align),
 			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(new_hbox),
			   GTK_WIDGET(track_length_event_box),
			   FALSE, FALSE, 3);

	gtk_box_pack_start(GTK_BOX(new_vbox),
			   GTK_WIDGET(now_playing_label),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(new_vbox),
			   GTK_WIDGET(new_hbox),
			   FALSE, FALSE, 0);

	vbox_align = gtk_alignment_new(0.5, 0.5, 1, 0);
	gtk_container_add(GTK_CONTAINER(vbox_align), new_vbox);

return vbox_align;
}

GtkWidget* create_status_bar(struct con_win *cwin)
{
	GtkWidget *status_bar;

	status_bar = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(status_bar), 0.99, 0);
	cwin->status_bar = status_bar;

	return status_bar;
}

static void
icon_pressed_cb (GtkEntry       *entry,
		gint            position,
		GdkEventButton *event,
		gpointer        data)
{
	if (position == GTK_ENTRY_ICON_SECONDARY)
		gtk_entry_set_text (entry, "");
}

/* Search (simple) */

GtkWidget* create_search_bar(struct con_win *cwin)
{
	GtkWidget *search_entry;

	search_entry = gtk_entry_new ();

	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

	gtk_entry_set_icon_sensitive (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, FALSE);

	/* Signal handlers */

	g_signal_connect (G_OBJECT(search_entry),
			"icon-press",
			G_CALLBACK (icon_pressed_cb),
			cwin);
	g_signal_connect(G_OBJECT(search_entry),
			 "changed",
			 G_CALLBACK(simple_library_search_keyrelease_handler),
			 cwin);

	cwin->search_entry = search_entry;

	return search_entry;
}

void create_status_icon(struct con_win *cwin)
{
	GtkStatusIcon *status_icon;
	GtkUIManager *systray_menu;

	if (cwin->pixbuf->pixbuf_app)
		status_icon = gtk_status_icon_new_from_pixbuf(cwin->pixbuf->pixbuf_app);
	else
		status_icon = gtk_status_icon_new_from_stock(GTK_STOCK_NEW);
 
	g_signal_connect (status_icon, "button-press-event", G_CALLBACK (status_icon_clicked), cwin);
	g_signal_connect (status_icon, "scroll_event", G_CALLBACK(systray_volume_scroll), cwin);
 
	g_object_set (G_OBJECT(status_icon), "has-tooltip", TRUE, NULL);
	g_signal_connect(G_OBJECT(status_icon), "query-tooltip", 
			G_CALLBACK(status_get_tooltip_cb),
			cwin);

	/* Systray right click menu */

	systray_menu = create_systray_menu(cwin);

	/* Store reference */

	cwin->status_icon = status_icon;
	cwin->systray_menu = systray_menu;
}

gboolean dialog_audio_init(gpointer data)
{
	struct con_win *cwin = data;
	GtkWidget *dialog;

	gdk_threads_enter();
	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",
					"Audio could not be initialized. "
					"Use preferences to select a proper "
					"audio backend/device and restart the app.");
	gtk_window_set_title(GTK_WINDOW(dialog), "Audio initialization error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return FALSE;
}

gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin)
{
	if(cwin->cpref->close_to_tray){
		if(gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))){
			toogle_main_window(cwin);
		}
		else{
			g_warning("(%s): No embedded status_icon.", __func__);
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
		}
	}
	else{
		if (cwin->cpref->save_playlist)
			save_current_playlist_state(cwin);
		common_cleanup(cwin);
		gtk_main_quit();
	}
	return TRUE;
}

GtkWidget * create_combo_order(struct con_win *cwin)
{
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label_order, *arrow;

	button = gtk_button_new();
	gtk_button_set_relief( GTK_BUTTON( button ), GTK_RELIEF_NONE );
	hbox = gtk_hbox_new(FALSE, 0);

	label_order = gtk_label_new("");
	gtk_misc_set_alignment (GTK_MISC(label_order),0,0.5);
	arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);

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
	gtk_container_add (GTK_CONTAINER(button),
			   hbox);

	g_signal_connect(G_OBJECT(button),
			 "button-press-event",
			 G_CALLBACK(library_page_right_click_cb),
			 cwin);

	gtk_widget_set_tooltip_text(GTK_WIDGET(button), _("Options of the library"));

	cwin->combo_order = button;
	cwin->combo_order_label = label_order;

	return button;
}
