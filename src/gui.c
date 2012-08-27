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

gchar *main_menu_xml = "<ui>							\
	<menubar name=\"Menubar\">						\
		<menu action=\"PlaybackMenu\">					\
			<separator/>						\
			<menuitem action=\"Prev\"/>				\
			<menuitem action=\"Play_pause\"/>			\
			<menuitem action=\"Stop\"/>				\
			<menuitem action=\"Next\"/>				\
			<separator/>						\
			<menuitem action=\"Shuffle\"/>				\
			<menuitem action=\"Repeat\"/>				\
			<separator/>						\
			<menuitem action=\"Edit tags\"/>			\
			<separator/>						\
			<menuitem action=\"Quit\"/>				\
		</menu>								\
		<menu action=\"PlaylistMenu\">					\
			<menuitem action=\"Add files\"/>			\
			<menuitem action=\"Add Audio CD\"/>			\
			<menuitem action=\"Add location\"/>			\
			<separator/>				    		\
			<menuitem action=\"Remove from playlist\"/>		\
			<menuitem action=\"Crop playlist\"/>			\
			<menuitem action=\"Clear playlist\"/>			\
			<separator/>				    		\
			<menuitem action=\"Add to another playlist\"/>		\
			<menuitem action=\"Save playlist\"/>			\
			<separator/>						\
			<menuitem action=\"Search in playlist\"/>		\
		</menu>								\
		<menu action=\"ViewMenu\">					\
			<menuitem action=\"Fullscreen\"/>			\
			<separator/>						\
			<menuitem action=\"Lateral panel\"/>		\
			<menuitem action=\"Playback controls below\"/>	\
			<menuitem action=\"Status bar\"/>			\
			<separator/>						\
			<menuitem action=\"Jump to playing song\"/>	\
		</menu>								\
		<menu action=\"ToolsMenu\">					\
			<separator/>						\
			<menuitem action=\"Equalizer\"/>			\
			<separator/>						\
			<menuitem action=\"Search lyric\"/>			\
			<menuitem action=\"Search artist info\"/>		\
			<separator/>						\
			<menu action=\"Lastfm\">				\
				<menuitem action=\"Love track\"/>		\
				<menuitem action=\"Unlove track\"/>		\
				<separator/>					\
				<menuitem action=\"Import a XSPF playlist\"/>	\
				<menuitem action=\"Add favorites\"/>		\
				<menuitem action=\"Add similar\"/>		\
			</menu>							\
			<separator/>						\
			<menuitem action=\"Rescan library\"/>			\
			<menuitem action=\"Update library\"/>			\
			<separator/>						\
			<menuitem action=\"Statistics\"/>			\
			<separator/>						\
			<menuitem action=\"Preferences\"/>			\
		</menu>								\
		<menu action=\"HelpMenu\">					\
			<menuitem action=\"Home\"/>				\
			<menuitem action=\"Community\"/>			\
			<menuitem action=\"Wiki\"/>				\
			<separator/>						\
			<menuitem action=\"Translate Pragha\"/>			\
			<separator/>						\
			<menuitem action=\"About\"/>				\
		</menu>								\
	</menubar>								\
</ui>";

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

GtkActionEntry main_aentries[] = {
	{"PlaybackMenu", NULL, N_("_Playback")},
	{"PlaylistMenu", NULL, N_("Play_list")},
	{"ViewMenu", NULL, N_("_View")},
	{"ToolsMenu", NULL, N_("_Tools")},
	{"HelpMenu", NULL, N_("_Help")},
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 NULL, N_("Open a media file"), G_CALLBACK(open_file_action)},
	{"Add Audio CD", GTK_STOCK_CDROM, N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(add_audio_cd_action)},
	{"Add location", GTK_STOCK_NETWORK, N_("Add _location"),
	 "", "Add a no local stream", G_CALLBACK(add_location_action)},
	{"Prev", GTK_STOCK_MEDIA_PREVIOUS, N_("Prev track"),
	 "<Alt>Left", "Prev track", G_CALLBACK(prev_action)},
	{"Play_pause", GTK_STOCK_MEDIA_PLAY, N_("Play / Pause"),
	 "<Control>space", "Play / Pause", G_CALLBACK(play_pause_action)},
	{"Stop", GTK_STOCK_MEDIA_STOP, N_("Stop"),
	 "", "Stop", G_CALLBACK(stop_action)},
	{"Next", GTK_STOCK_MEDIA_NEXT, N_("Next track"),
	 "<Alt>Right", "Next track", G_CALLBACK(next_action)},
	{"Edit tags", GTK_STOCK_EDIT, N_("Edit track information"),
	 "<Control>E", "Edit information of current track", G_CALLBACK(edit_tags_playing_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 "<Control>Q", "Quit pragha", G_CALLBACK(quit_action)},
	{"Remove from playlist", GTK_STOCK_REMOVE, N_("Remove selection from playlist"),
	 "", "Remove selection from playlist", G_CALLBACK(remove_from_playlist)},
	{"Crop playlist", GTK_STOCK_REMOVE, N_("Crop playlist"),
	 "<Control>C", "Crop playlist", G_CALLBACK(crop_current_playlist)},
	{"Clear playlist", GTK_STOCK_CLEAR, N_("Clear playlist"),
	 "<Control>L", "Clear the current playlist", G_CALLBACK(clear_current_playlist)},
	{"Add to another playlist", NULL, N_("Add selection to another playlist")},
	{"Save playlist", GTK_STOCK_SAVE_AS, N_("Save playlist")},
	{"Search in playlist", GTK_STOCK_FIND, N_("_Search in playlist"),
	 "<Control>F", "Search in playlist", G_CALLBACK(search_playlist_action)},
	{"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"),
	 "<Control>P", "Set preferences", G_CALLBACK(pref_action)},
	{"Jump to playing song", GTK_STOCK_JUMP_TO, N_("Jump to playing song"),
	 "<Control>J", "Jump to playing song", G_CALLBACK(jump_to_playing_song_action)},
	{"Equalizer", NULL, N_("E_qualizer"),
	 "", "Equalizer", G_CALLBACK(show_equalizer_action)},
	#ifdef HAVE_LIBGLYR
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "<Control>Y", "Search lyric", G_CALLBACK(related_get_lyric_action)},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", G_CALLBACK(related_get_artist_info_action)},
	#else
	{"Search lyric", GTK_STOCK_JUSTIFY_FILL, N_("Search _lyric"),
	 "<Control>Y", "Search lyric", NULL},
	{"Search artist info", GTK_STOCK_INFO, N_("Search _artist info"),
	 "", "Search artist info", NULL},
	#endif
	{"Lastfm", NULL, N_("_Lastfm")},
	#ifdef HAVE_LIBCLASTFM
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", G_CALLBACK(lastfm_track_love_action)},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", G_CALLBACK(lastfm_track_unlove_action)},
	{"Import a XSPF playlist", NULL, N_("Import a XSPF playlist"),
	 "", "Import a XSPF playlist", G_CALLBACK(lastfm_import_xspf_action)},
	{"Add favorites", NULL, N_("Add favorites"),
	 "", "Add favorites", G_CALLBACK(lastfm_add_favorites_action)},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", G_CALLBACK(lastfm_get_similar_action)},
	#else
	{"Love track", NULL, N_("Love track"),
	 "", "Love track", NULL},
	{"Unlove track", NULL, N_("Unlove track"),
	 "", "Unlove track", NULL},
	{"Import a XSPF playlist", NULL, N_("Import a XSPF playlist"),
	 "", "Import a XSPF playlist", NULL},
	{"Add favorites", NULL, N_("Add favorites"),
	 "", "Add favorites", NULL},
	{"Add similar", NULL, N_("Add similar"),
	 "", "Add similar", NULL},
	#endif
	{"Rescan library", GTK_STOCK_EXECUTE, N_("_Rescan library"),
	 "", "Rescan library", G_CALLBACK(rescan_library_action)},
	{"Update library", GTK_STOCK_EXECUTE, N_("_Update library"),
	 "", "Update library", G_CALLBACK(update_library_action)},
	{"Statistics", GTK_STOCK_INFO, N_("_Statistics"),
	 "", "Statistics", G_CALLBACK(statistics_action)},
	{"Home", GTK_STOCK_HOME, N_("Homepage"),
	 "", "Homepage", G_CALLBACK(home_action)},
	{"Community", GTK_STOCK_INFO, N_("Community"),
	 "", "Forum of pragha", G_CALLBACK(community_action)},
	{"Wiki", GTK_STOCK_YES, N_("Wiki"),
	 "", "Wiki of pragha", G_CALLBACK(wiki_action)},
	{"Translate Pragha", "preferences-desktop-locale", N_("Translate Pragha"),
	 "", "Translate Pragha", G_CALLBACK(translate_action)},
	{"About", GTK_STOCK_ABOUT, N_("About"),
	 "", "About pragha", G_CALLBACK(about_action)},
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
	{"Lateral panel", NULL, N_("Lateral _panel"),
	 "F9", "Lateral panel", G_CALLBACK(library_pane_action),
	TRUE},
	{"Playback controls below", NULL, N_("Playback controls below"),
	 NULL, "Show playback controls below", G_CALLBACK(show_controls_below_action),
	FALSE},
	{"Status bar", NULL, N_("Status bar"),
	 "", "Status bar", G_CALLBACK(status_bar_action),
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

GtkToggleActionEntry cp_null_toggles_entries[] = {
	{"Lateral panel", NULL, N_("Lateral _panel"),
	 "", "Lateral panel", G_CALLBACK(library_pane_action),
	TRUE}
};

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

GtkActionEntry playlist_tree_context_aentries[] = {
	{"Add to current playlist", GTK_STOCK_ADD, N_("_Add to current playlist"),
	 "", "Add to current playlist", G_CALLBACK(library_tree_add_to_playlist_action)},
	{"Replace current playlist", NULL, N_("_Replace current playlist"),
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist)},
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
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist)},
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
	 "", "Replace current playlist", G_CALLBACK(library_tree_replace_playlist)},
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

GtkTargetEntry tentries[] = {
	{"REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY},
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

	cwin->library_store = store;
	cwin->library_tree = library_tree;

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

	cwin->combo_order = button;
	cwin->combo_order_label = label_order;

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

	cwin->browse_mode = vbox_lib;
	cwin->search_entry = search_entry;

	return vbox_lib;
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

static GtkWidget* create_current_playlist_view(struct con_win *cwin)
{
	GtkWidget *current_playlist_scroll;
	GtkWidget *current_playlist;
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

	/* Store the treeview in the scrollbar widget */

	gtk_container_add (GTK_CONTAINER(current_playlist_scroll), current_playlist);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(current_playlist_scroll), GTK_SHADOW_IN);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(current_playlist), cwin->cpref->use_hint);

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

void
gtk_tool_insert_generic_item(GtkToolbar *toolbar, GtkWidget *item)
{
	GtkWidget *align_box;
	GtkToolItem *boxitem;

	boxitem = gtk_tool_item_new ();

	align_box = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(align_box), item);

	gtk_container_add (GTK_CONTAINER(boxitem), align_box);
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);
}

GtkWidget*
create_toolbar(struct con_win *cwin)
{
	GtkWidget *toolbar, *box;
	GtkToolItem *boxitem, *prev_button, *play_button, *stop_button, *next_button;
	GtkWidget *album_art_frame = NULL, *playing;
	GtkToolItem *unfull_button, *shuffle_button, *repeat_button;
	GtkWidget *vol_button;

	toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkStyleContext * context = gtk_widget_get_style_context (toolbar);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
#endif

	/* Setup Left control buttons */

	prev_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), cwin);
	g_signal_connect (G_OBJECT (prev_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), _("Previous Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(prev_button));
	cwin->prev_button = prev_button;

	play_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), cwin);
	g_signal_connect (G_OBJECT (play_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(play_button), _("Play / Pause Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(play_button));
	cwin->play_button = play_button;

	stop_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), cwin);
	g_signal_connect (G_OBJECT (stop_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(stop_button), _("Stop playback"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(stop_button));
	cwin->stop_button = stop_button;

	next_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), cwin);
	g_signal_connect (G_OBJECT (next_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), _("Next Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(next_button));
	cwin->next_button = next_button;

	if (cwin->cpref->show_album_art) {
		boxitem = gtk_tool_item_new ();
		gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);
		box = gtk_hbox_new(FALSE, 0);

		album_art_frame = gtk_event_box_new ();
		gtk_event_box_set_visible_window(GTK_EVENT_BOX(album_art_frame), FALSE);
		g_signal_connect (G_OBJECT (album_art_frame),
				"button_press_event",
				G_CALLBACK (album_art_frame_press_callback),
				cwin);
		gtk_container_add (GTK_CONTAINER(boxitem), box);
		gtk_box_pack_start (GTK_BOX(box), album_art_frame, TRUE, TRUE, 2);
	}
	cwin->album_art_frame = album_art_frame;

	/* Setup playing box */

	boxitem = gtk_tool_item_new ();
	gtk_tool_item_set_expand (boxitem, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);

	playing = create_playing_box(cwin);

	box = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box), playing, TRUE, TRUE, 5);
	gtk_container_add (GTK_CONTAINER(boxitem), box);

	/* Setup Right control buttons */

	unfull_button = gtk_tool_button_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN);
	g_signal_connect(G_OBJECT(unfull_button), "clicked",
			 G_CALLBACK(unfull_button_handler), cwin);
	g_signal_connect(G_OBJECT (unfull_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(unfull_button), _("Leave Fullscreen"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(unfull_button));
	cwin->unfull_button = unfull_button;

	shuffle_button = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(shuffle_button), "media-playlist-shuffle");
	g_signal_connect(G_OBJECT(shuffle_button), "toggled",
			 G_CALLBACK(shuffle_button_handler), cwin);
	g_signal_connect(G_OBJECT (shuffle_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(shuffle_button), _("Play songs in a random order"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(shuffle_button));
	cwin->shuffle_button = shuffle_button;

	repeat_button = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(repeat_button), "media-playlist-repeat");
	g_signal_connect(G_OBJECT(repeat_button), "toggled",
			 G_CALLBACK(repeat_button_handler), cwin);
	g_signal_connect(G_OBJECT (repeat_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(repeat_button), _("Repeat playback list at the end"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(repeat_button));
	cwin->repeat_button = repeat_button;

	vol_button = gtk_volume_button_new();
	gtk_button_set_relief(GTK_BUTTON(vol_button), GTK_RELIEF_NONE);
	g_object_set(G_OBJECT(vol_button), "size", GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	g_signal_connect(G_OBJECT(vol_button), "value-changed",
			 G_CALLBACK(vol_button_handler), cwin);
	g_signal_connect (G_OBJECT (vol_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), vol_button);
	cwin->vol_button = vol_button;

	/* Insensitive Prev/Stop/Next buttons and set unknown album art. */

	update_panel_playback_state(cwin);

	cwin->toolbar = toolbar;

	return toolbar;
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

GtkWidget* create_status_bar(struct con_win *cwin)
{
	GtkWidget *status_bar;

	status_bar = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(status_bar), 0.99, 0);
	cwin->status_bar = status_bar;

	return status_bar;
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
	if (pixbuf->pixbuf_dir)
		g_object_unref(pixbuf->pixbuf_dir);
	if (pixbuf->pixbuf_artist)
		g_object_unref(pixbuf->pixbuf_artist);
	if (pixbuf->pixbuf_album)
		g_object_unref(pixbuf->pixbuf_album);
	if (pixbuf->pixbuf_track)
		g_object_unref(pixbuf->pixbuf_track);
	if (pixbuf->pixbuf_genre)
		g_object_unref(pixbuf->pixbuf_genre);

	g_object_unref(pixbuf->pixbuf_playing);
	g_object_unref(pixbuf->pixbuf_paused);

	g_slice_free(struct pixbuf, pixbuf);
}

void gui_free (struct con_win *cwin)
{
	g_object_unref(cwin->library_store);

	pixbufs_free(cwin->pixbuf);
	cwin->pixbuf = NULL;

	if (cwin->album_art)
		gtk_widget_destroy(cwin->album_art);
}
