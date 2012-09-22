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

static GtkWidget *library_dialog;

static gchar *license = "This program is free software: "
	"you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <http://www.gnu.org/licenses/>.";

static const gchar *main_menu_xml = "<ui>					\
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

static GtkActionEntry main_aentries[] = {
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

static GtkToggleActionEntry toggles_entries[] = {
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

/* Sentitive menubar actions depending on the playback status. */

static void
update_menubar_playback_state (struct con_win *cwin)
{
	GtkAction *action;

	gboolean playing = (pragha_backend_get_state (cwin->backend) != ST_STOPPED);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Prev");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Stop");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Next");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Edit tags");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Jump to playing song");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	#ifdef HAVE_LIBGLYR
	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Search lyric");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Search artist info");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);
	#endif

	#ifdef HAVE_LIBCLASTFM
	update_menubar_lastfm_state (cwin);
	#endif

	action = gtk_ui_manager_get_action(cwin->systray_menu, "/popup/Prev");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->systray_menu, "/popup/Stop");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->systray_menu, "/popup/Next");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action(cwin->systray_menu, "/popup/Edit tags");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	/* HACK TO TEST VISIBILITY OF PRAGHA_ALBUM_ART. */
	pragha_album_art_set_visible(cwin->albumart, playing);
}

static void
update_menubar_playback_state_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;
	update_menubar_playback_state (cwin);
}

/* Signal handler for deleting rescan dialog box */

static gboolean rescan_dialog_delete_cb(GtkWidget *widget,
					GdkEvent *event,
					gpointer user_data)
{
	GCancellable *cancellable = user_data;

	g_cancellable_cancel (cancellable);

	return TRUE;
}

/* Signal handler for cancelling rescan dialog box */

static void rescan_dialog_response_cb(GtkDialog *dialog,
				      gint response_id,
				      gpointer user_data)
{
	GCancellable *cancellable = user_data;

	if (response_id == GTK_RESPONSE_CANCEL)
		g_cancellable_cancel (cancellable);
}

/* Generate and add the recently-used data */

void add_recent_file (const gchar *filename)
{
	GtkRecentData recent_data;
	gchar *uri = NULL;

	recent_data.mime_type = get_mime_type(filename);
	if (recent_data.mime_type == NULL)
		return;

	recent_data.display_name = g_filename_display_basename (filename);
	recent_data.app_name = g_strdup (g_get_application_name ());
	recent_data.app_exec =  g_strjoin (" ", g_get_prgname (), "%u", NULL);
	recent_data.description = NULL;
	recent_data.groups = NULL;
	recent_data.is_private = FALSE;

	uri = g_filename_to_uri(filename, NULL, NULL);
	gtk_recent_manager_add_full(gtk_recent_manager_get_default(), uri, &recent_data);

	g_free (recent_data.display_name);
	g_free (recent_data.mime_type);
	g_free (recent_data.app_name);
	g_free (recent_data.app_exec);
	g_free (uri);
}

/* Add selected files from the file chooser to the current playlist */

void handle_selected_file(gpointer data, gpointer udata)
{
	struct musicobject *mobj;
	struct con_win *cwin = udata;

	if (!data)
		return;

	if (g_file_test(data, G_FILE_TEST_IS_DIR)){
		if(cwin->cpref->add_recursively_files)
			__recur_add(data, cwin);
		else
			__non_recur_add(data, TRUE, cwin);
	}
	else if (pragha_pl_parser_guess_format_from_extension(data) != PL_FORMAT_UNKNOWN) {
		pragha_pl_parser_open_from_file_by_extension(data, cwin);
	}
	else{
		mobj = new_musicobject_from_file(data);
		if (mobj) {
			append_current_playlist(NULL, mobj, cwin);
			add_recent_file(data);
		}
	}

	/* Have to give control to GTK periodically ... */
	while(gtk_events_pending())
		gtk_main_iteration_do(FALSE);
}

/* Create a dialog box with a progress bar for rescan/update library */

static GtkWidget* lib_progress_bar(struct con_win *cwin, int update, gpointer cb_data)
{
	GtkWidget *hbox, *spinner, *progress_bar;

	/* Create a dialog with a Cancel button */

	library_dialog =
		gtk_dialog_new_with_buttons((update) ?
					    _("Update Library") : _("Rescan Library"),
					    GTK_WINDOW(cwin->mainwindow),
					    GTK_DIALOG_MODAL,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL,
					    NULL);

	/* Create a progress bar */

	progress_bar = gtk_progress_bar_new();
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress_bar));

	hbox = gtk_hbox_new (FALSE, 5);

	spinner = gtk_spinner_new ();
	gtk_container_add (GTK_CONTAINER (hbox), spinner);
	gtk_spinner_start(GTK_SPINNER(spinner));

	gtk_container_add (GTK_CONTAINER (hbox), progress_bar);

	/* Set various properties */

	gtk_widget_set_size_request(progress_bar,
				    PROGRESS_BAR_WIDTH,
				    -1);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(library_dialog))),
				  GTK_BUTTONBOX_SPREAD);

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(GTK_WINDOW(library_dialog)), "delete_event",
			 G_CALLBACK(rescan_dialog_delete_cb), cb_data);
	g_signal_connect(G_OBJECT(library_dialog), "response",
			 G_CALLBACK(rescan_dialog_response_cb), cb_data);

	/* Add the progress bar to the dialog box's vbox and show everything */

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(library_dialog))), hbox);
	gtk_widget_show_all(library_dialog);

	return progress_bar;
}

/* Add Files a folders to play list based on Audacius code.*/
/* /src/ui_fileopen.c */
static void
close_button_cb(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

static void
add_button_cb(GtkWidget *widget, gpointer data)
{
	GSList *files = NULL;

	GtkWidget *window = g_object_get_data(data, "window");
	GtkWidget *chooser = g_object_get_data(data, "chooser");
	GtkWidget *toggle = g_object_get_data(data, "toggle-button");
	struct con_win *cwin = g_object_get_data(data, "cwin");

	cwin->cpref->add_recursively_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle));
	g_free (cwin->cstate->last_folder);
	cwin->cstate->last_folder = gtk_file_chooser_get_current_folder ((GtkFileChooser *) chooser);

	files = gtk_file_chooser_get_filenames((GtkFileChooser *) chooser);

	gtk_widget_destroy(window);

	if (files) {
		set_watch_cursor (cwin->mainwindow);

		g_slist_foreach(files, handle_selected_file, cwin);
		g_slist_free_full(files, g_free);

		remove_watch_cursor (cwin->mainwindow);
	}
	select_last_path_of_current_playlist(cwin);
	update_status_bar(cwin);
}

static gboolean
open_file_on_keypress(GtkWidget *dialog,
                        GdkEventKey *event,
                        gpointer data)
{
    if (event->keyval == GDK_KEY_Escape) {
        gtk_widget_destroy(dialog);
        return TRUE;
    }

    return FALSE;
}

/* Handler for the 'Open' item in the File menu */

void open_file_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *window, *hbox, *vbox, *chooser, *bbox, *toggle, *close_button, *add_button;
	gpointer storage;
	gint i=0;
	GtkFileFilter *media_filter, *playlist_filter, *all_filter;

	/* Create a file chooser dialog */

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_title(GTK_WINDOW(window), (_("Select a file to play")));
	gtk_window_set_default_size(GTK_WINDOW(window), 700, 450);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	vbox = gtk_vbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);

	/* Set various properties */

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), TRUE);

	if (cwin->cstate->last_folder)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser),
		cwin->cstate->last_folder);

	hbox = gtk_hbox_new(FALSE, 0);

	toggle = gtk_check_button_new_with_label(_("Add recursively files"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
					cwin->cpref->add_recursively_files ? TRUE : FALSE);
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(bbox), 6);

	close_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	add_button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_container_add(GTK_CONTAINER(bbox), close_button);
	gtk_container_add(GTK_CONTAINER(bbox), add_button);

	gtk_box_pack_start(GTK_BOX(hbox), toggle, TRUE, TRUE, 3);
	gtk_box_pack_end(GTK_BOX(hbox), bbox, FALSE, FALSE, 3);

	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);
	gtk_box_pack_end(GTK_BOX(vbox), chooser, TRUE, TRUE, 3);

	/* Create file filters  */

	media_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(media_filter), _("Supported media"));
	
	while (mime_wav[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_wav[i++]);
	i = 0;
	while (mime_mpeg[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_mpeg[i++]);
	i = 0;
	while (mime_flac[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_flac[i++]);
	i = 0;
	while (mime_ogg[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_ogg[i++]);

	#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
	i = 0;
	while (mime_asf[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_asf[i++]);
	#endif
	#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
	i = 0;
	while (mime_mp4[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_mp4[i++]);
	#endif
	#ifdef HAVE_TAGLIB_1_7
	i = 0;
	while (mime_ape[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_ape[i++]);
	#endif

	#ifdef HAVE_PLPARSER
	i = 0;
	while (mime_playlist[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_playlist[i++]);
	i = 0;
	while (mime_dual[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
					      mime_dual[i++]);
	#else
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.m3u");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.M3U");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.pls");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.PLS");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.xspf");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.XSPF");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.wax");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.WAX");
	#endif

	playlist_filter = gtk_file_filter_new();

	#ifdef HAVE_PLPARSER
	i = 0;
	while (mime_playlist[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(playlist_filter),
					      mime_playlist[i++]);
	i = 0;
	while (mime_dual[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(playlist_filter),
					      mime_dual[i++]);
	#else
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.m3u");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.M3U");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.pls");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.PLS");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.xspf");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.XSPF");

	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.wax");
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(playlist_filter), "*.WAX");
	#endif

	gtk_file_filter_set_name(GTK_FILE_FILTER(playlist_filter), _("Playlists"));

	all_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(all_filter), _("All files"));
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(all_filter), "*");

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser),
				    GTK_FILE_FILTER(media_filter));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser),
				    GTK_FILE_FILTER(playlist_filter));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser),
				    GTK_FILE_FILTER(all_filter));

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser),
				    GTK_FILE_FILTER(media_filter));

	storage = g_object_new(G_TYPE_OBJECT, NULL);
	g_object_set_data(storage, "window", window);
	g_object_set_data(storage, "chooser", chooser);
	g_object_set_data(storage, "toggle-button", toggle);
	g_object_set_data(storage, "cwin", cwin);

	g_signal_connect(add_button, "clicked",
		G_CALLBACK(add_button_cb), storage);
	g_signal_connect(chooser, "file-activated",
		G_CALLBACK(add_button_cb), storage);
	g_signal_connect(close_button, "clicked",
			G_CALLBACK(close_button_cb), window);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_widget_destroy), window);
	g_signal_connect(window, "key-press-event",
			G_CALLBACK(open_file_on_keypress), NULL);

	gtk_window_set_transient_for(GTK_WINDOW (window), GTK_WINDOW(cwin->mainwindow));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);

	gtk_widget_show_all(window);
}

/* Handler for the 'Add Audio CD' item in the pragha menu */

void add_audio_cd_action(GtkAction *action, struct con_win *cwin)
{
	add_audio_cd(cwin);
}

/* Build a dialog to get a new playlist name */

static char *
totem_open_location_set_from_clipboard (GtkWidget *open_location)
{
	GtkClipboard *clipboard;
	gchar *clipboard_content;

	/* Initialize the clipboard and get its content */
	clipboard = gtk_clipboard_get_for_display (gtk_widget_get_display (GTK_WIDGET (open_location)), GDK_SELECTION_CLIPBOARD);
	clipboard_content = gtk_clipboard_wait_for_text (clipboard);

	/* Check clipboard for "://". If it exists, return it */
	if (clipboard_content != NULL && strcmp (clipboard_content, "") != 0)
	{
		if (g_strrstr (clipboard_content, "://") != NULL)
			return clipboard_content;
	}

	g_free (clipboard_content);
	return NULL;
}

void add_location_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *vbox, *hbox;
	GtkWidget *label_new, *uri_entry, *label_name, *name_entry;
	const gchar *uri = NULL, *name = NULL;
	gchar *clipboard_location;
	struct musicobject *mobj;
	gint result;

	/* Create dialog window */
	vbox = gtk_vbox_new(TRUE, 2);

	label_new = gtk_label_new_with_mnemonic(_("Enter the URL of an internet radio stream"));
	uri_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(uri_entry), 255);

	hbox = gtk_hbox_new(FALSE, 2);
	label_name = gtk_label_new_with_mnemonic(_("Give it a name to save"));
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 255);

	gtk_box_pack_start(GTK_BOX(hbox), label_name, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), name_entry, TRUE, TRUE, 2);

	gtk_box_pack_start(GTK_BOX(vbox), label_new, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), uri_entry, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

	/* Get item from clipboard to fill GtkEntry */
	clipboard_location = totem_open_location_set_from_clipboard (uri_entry);
	if (clipboard_location != NULL && strcmp (clipboard_location, "") != 0) {
		gtk_entry_set_text (GTK_ENTRY(uri_entry), clipboard_location);
		g_free (clipboard_location);
	}

	dialog = gtk_dialog_new_with_buttons(_("Add a location"),
			     GTK_WINDOW(cwin->mainwindow),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
			     NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, -1);

	gtk_entry_set_activates_default (GTK_ENTRY(uri_entry), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(name_entry), TRUE);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		if (gtk_entry_get_text_length (GTK_ENTRY(uri_entry)))
			uri = gtk_entry_get_text(GTK_ENTRY(uri_entry));

		if(uri != NULL) {
			if (gtk_entry_get_text_length (GTK_ENTRY(name_entry)))
				name = gtk_entry_get_text(GTK_ENTRY(name_entry));

			mobj = new_musicobject_from_location(uri, name, cwin);

			append_current_playlist(NULL, mobj, cwin);
			update_status_bar(cwin);

			if (name) {
				new_radio(uri, name, cwin);
				init_library_view(cwin);
			}
		}
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}
	gtk_widget_destroy(dialog);

	return;
}

/* Handler for the 'Prev' item in the pragha menu */

void prev_action(GtkAction *action, struct con_win *cwin)
{
	play_prev_track(cwin);
}

/* Handler for the 'Play / Pause' item in the pragha menu */

void play_pause_action(GtkAction *action, struct con_win *cwin)
{
	play_pause_resume(cwin);
}

/* Handler for the 'Stop' item in the pragha menu */

void stop_action(GtkAction *action, struct con_win *cwin)
{
	pragha_backend_stop(cwin->backend, NULL);
}

/* Handler for the 'Next' item in the pragha menu */

void next_action (GtkAction *action, struct con_win *cwin)
{
	play_next_track(cwin);
}

void edit_tags_playing_action(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	GArray *loc_arr = NULL;
	GPtrArray *file_arr = NULL;
	gchar *sfile = NULL, *tfile = NULL;
	gint location_id, changed = 0;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	memset(&otag, 0, sizeof(struct tags));
	memset(&ntag, 0, sizeof(struct tags));

	if (cwin->cstate->curr_mobj) {
		otag.track_no = cwin->cstate->curr_mobj->tags->track_no;
		otag.title = cwin->cstate->curr_mobj->tags->title;
		otag.artist = cwin->cstate->curr_mobj->tags->artist;
		otag.album = cwin->cstate->curr_mobj->tags->album;
		otag.genre = cwin->cstate->curr_mobj->tags->genre;
		otag.comment = cwin->cstate->curr_mobj->tags->comment;
		otag.year =  cwin->cstate->curr_mobj->tags->year;

		changed = tag_edit_dialog(&otag, 0, &ntag, cwin->cstate->curr_mobj->file, cwin);
	}

	if (!changed)
		goto exit;

	/* Update the music object, the gui and them mpris */

	update_musicobject(cwin->cstate->curr_mobj, changed, &ntag , cwin);

	__update_current_song_info(cwin);

	mpris_update_metadata_changed(cwin);

	if ((path = current_playlist_get_actual(cwin)) != NULL) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
		if (gtk_tree_model_get_iter(model, &iter, path))
			update_track_current_playlist(&iter, changed, cwin->cstate->curr_mobj, cwin);
		gtk_tree_path_free(path);
	}

	/* Store the new tags */

	if (G_LIKELY(cwin->cstate->curr_mobj->file_type != FILE_CDDA &&
	    cwin->cstate->curr_mobj->file_type != FILE_HTTP)) {
		loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
		file_arr = g_ptr_array_new();

		sfile = sanitize_string_sqlite3(cwin->cstate->curr_mobj->file);
		location_id = find_location_db(sfile, cwin->cdbase);

		if (location_id)
			g_array_append_val(loc_arr, location_id);

		tfile = g_strdup(cwin->cstate->curr_mobj->file);
		g_ptr_array_add(file_arr, tfile);

		tag_update(loc_arr, file_arr, changed, &ntag, cwin);

		init_library_view(cwin);

		g_array_free(loc_arr, TRUE);
		g_ptr_array_free(file_arr, TRUE);

		g_free(sfile);
		g_free(tfile);
	}

exit:
	/* Cleanup */
	g_free(ntag.title);
	g_free(ntag.artist);
	g_free(ntag.album);
	g_free(ntag.genre);
	g_free(ntag.comment);	
}

/* Handler for the 'Quit' item in the pragha menu */

void quit_action(GtkAction *action, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
}

/* Handler for 'Expand All' option in the Edit menu */

void expand_all_action(GtkAction *action, struct con_win *cwin)
{
	gtk_tree_view_expand_all(GTK_TREE_VIEW(cwin->library_tree));
}

/* Handler for 'Collapse All' option in the Edit menu */

void collapse_all_action(GtkAction *action, struct con_win *cwin)
{
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(cwin->library_tree));
}

/* Handler for 'Search Library' option in the Edit menu */

void search_library_action(GtkAction *action, struct con_win *cwin)
{
	gtk_widget_grab_focus(GTK_WIDGET(cwin->search_entry));
}

/* Handler for 'Search Playlist' option in the Edit menu */

void search_playlist_action(GtkAction *action, struct con_win *cwin)
{
	dialog_jump_to_track (cwin);
}

/* Handler for 'Shuffle' option in the Edit menu */

void shuffle_action(GtkToggleAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "shuffle_action");

	cwin->cpref->shuffle = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	g_signal_handlers_block_by_func (cwin->shuffle_button, shuffle_button_handler, cwin);

		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(cwin->shuffle_button), cwin->cpref->shuffle);
		shuffle_button(cwin);

	g_signal_handlers_unblock_by_func (cwin->shuffle_button, shuffle_button_handler, cwin);
}

/* Handler for 'Repeat' option in the Edit menu */

void repeat_action(GtkToggleAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Repeat_action");

	cwin->cpref->repeat = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	g_signal_handlers_block_by_func (cwin->repeat_button, repeat_button_handler, cwin);

		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(cwin->repeat_button), cwin->cpref->repeat);
		
	g_signal_handlers_unblock_by_func (cwin->repeat_button, repeat_button_handler, cwin);
}

/* Handler for the 'Preferences' item in the Edit menu */

void pref_action(GtkAction *action, struct con_win *cwin)
{
	preferences_dialog(cwin);
}

/* Handler for the 'Full screen' item in the Edit menu */

void
fullscreen_action (GtkAction *action, struct con_win *cwin)
{
	GtkWidget *menu_bar;
	gboolean fullscreen;
	GdkWindowState state;

	menu_bar = gtk_ui_manager_get_widget(cwin->bar_context_menu, "/Menubar");

	fullscreen = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	if(fullscreen){
		gtk_window_fullscreen(GTK_WINDOW(cwin->mainwindow));
		gtk_widget_show(GTK_WIDGET(cwin->unfull_button));
		gtk_widget_hide(GTK_WIDGET(menu_bar));
	}
	else {
		state = gdk_window_get_state (gtk_widget_get_window (cwin->mainwindow));
		if (state & GDK_WINDOW_STATE_FULLSCREEN)
			gtk_window_unfullscreen(GTK_WINDOW(cwin->mainwindow));
		gtk_widget_hide(GTK_WIDGET(cwin->unfull_button));
		gtk_widget_show(GTK_WIDGET(menu_bar));
	}
}

/* Handler for the 'Library panel' item in the Edit menu and emply pĺaylist menu */

void
library_pane_action (GtkAction *action, struct con_win *cwin)
{
	GtkAction *maction, *paction;

	cwin->cpref->lateral_panel = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	gtk_widget_set_visible (GTK_WIDGET(cwin->browse_mode), cwin->cpref->lateral_panel);

	paction = gtk_ui_manager_get_action(cwin->cp_null_context_menu, "/popup/Lateral panel");

	g_signal_handlers_block_by_func (paction, library_pane_action, cwin);
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(paction), cwin->cpref->lateral_panel);
	g_signal_handlers_unblock_by_func (paction, library_pane_action, cwin);

	maction = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Lateral panel");

	g_signal_handlers_block_by_func (maction, library_pane_action, cwin);
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(maction), cwin->cpref->lateral_panel);
	g_signal_handlers_unblock_by_func (maction, library_pane_action, cwin);
}

/* Handler for the 'Status bar' item in the Edit menu */

void
status_bar_action (GtkAction *action, struct con_win *cwin)
{
	cwin->cpref->status_bar = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	if(cwin->cpref->status_bar)
		gtk_widget_show(GTK_WIDGET(cwin->status_bar));
	else
		gtk_widget_hide(GTK_WIDGET(cwin->status_bar));
}

/* Handler for the 'Show_controls_below_action' item in the view menu */

void
show_controls_below_action (GtkAction *action, struct con_win *cwin)
{
	cwin->cpref->controls_below = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(cwin->toolbar));

	gint position = cwin->cpref->controls_below ? 3 : 1;

	gtk_box_reorder_child(GTK_BOX(parent), cwin->toolbar, position);
}

void
jump_to_playing_song_action (GtkAction *action, struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	path = current_playlist_get_actual(cwin);

	jump_to_path_on_current_playlist (path, cwin);

	gtk_tree_path_free(path);
}

/* Handler for the 'Rescan Library' item in the Tools menu */
void rescan_library_action(GtkAction *action, struct con_win *cwin)
{
	rescan_library_handler(cwin);
}

void rescan_library_handler(struct con_win *cwin)
{
	GtkWidget *msg_dialog;
	GtkWidget *progress_bar;
	GCancellable *cancellable;
	gint no_files = 0, i, cnt = 0;
	GSList *list;
	gchar *lib;

	/* Check if Library is set */

	if (!cwin->cpref->library_dir) {
		g_warning("Library is not set, flushing existing library");
		flush_db(cwin->cdbase);
		init_library_view(cwin);
		return ;
	}

	/* Check if versions are incompatible, if so drop tables and
	   initialize schema, otherwise, just flush the library database */

	if (is_incompatible_upgrade(cwin)) {
		if (drop_dbase_schema(cwin->cdbase) == -1) {
			g_critical("Unable to drop database schema");
			return;
		}
		if (init_dbase_schema(cwin->cdbase) == -1) {
			g_critical("Unable to init database schema");
			return;
		}
	} else {
		flush_db(cwin->cdbase);
	}

	cancellable = g_cancellable_new ();

	/* Create the dialog */

	progress_bar = lib_progress_bar(cwin, 0, cancellable);

	/* Start the scan */

	list = cwin->cpref->library_dir;
	cnt = g_slist_length(cwin->cpref->library_dir);

	for (i=0; i<cnt; i++) {
		lib = list->data;
		no_files = dir_file_count(lib, 1);

		db_begin_transaction(cwin->cdbase);

		rescan_db(lib, no_files, progress_bar, 1, cancellable, cwin->cdbase);

		db_commit_transaction(cwin->cdbase);

		list = list->next;
	}

	update_menu_playlist_changes(cwin);
	init_library_view(cwin);

	gtk_widget_destroy(library_dialog);

	if (!g_cancellable_is_cancelled (cancellable)) {
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));
		gtk_dialog_run(GTK_DIALOG(msg_dialog));
		gtk_widget_destroy(msg_dialog);
	}

	g_object_unref(cancellable);

	/* Save rescan time */

	g_get_current_time(&cwin->cpref->last_rescan_time);

	/* Free lists */

	free_str_list(cwin->cpref->lib_add);
	free_str_list(cwin->cpref->lib_delete);

	cwin->cpref->lib_add = NULL;
	cwin->cpref->lib_delete = NULL;
}

/* Handler for the 'Update Library' item in the Tools menu */

void update_library_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *msg_dialog;
	GtkWidget *progress_bar;
	GCancellable *cancellable;
	gint no_files = 0, i, cnt = 0;
	GSList *list;
	gchar *lib;

	/* To track user termination */

	cancellable = g_cancellable_new ();

	/* Create the dialog */

	progress_bar = lib_progress_bar(cwin, 1, cancellable);

	/* Check if any library has been removed */

	list = cwin->cpref->lib_delete;
	cnt = g_slist_length(cwin->cpref->lib_delete);

	for (i=0; i<cnt; i++) {
		lib = list->data;
		no_files = dir_file_count(lib, 1);

		db_begin_transaction(cwin->cdbase);

		delete_db(lib, no_files, progress_bar, 1, cwin->cdbase);

		db_commit_transaction(cwin->cdbase);

		if (g_cancellable_is_cancelled (cancellable))
			goto exit;
		list = list->next;
	}

	/* Check if any library has been added */

	list = cwin->cpref->lib_add;
	cnt = g_slist_length(cwin->cpref->lib_add);

	for (i=0; i<cnt; i++) {
		lib = list->data;
		no_files = dir_file_count(lib, 1);

		db_begin_transaction(cwin->cdbase);

		rescan_db(lib, no_files, progress_bar, 1, cancellable, cwin->cdbase);

		db_commit_transaction(cwin->cdbase);

		if (g_cancellable_is_cancelled (cancellable))
			goto exit;
		list = list->next;
	}

	/* Check if any files in the existing library dirs
	   have been modified */

	list = cwin->cpref->library_dir;
	cnt = g_slist_length(cwin->cpref->library_dir);

	for (i=0; i<cnt; i++) {
		lib = list->data;

		/* Don't rescan if lib is present in lib_add,
		   we just rescanned it above */

		if (is_present_str_list(lib, cwin->cpref->lib_add)) {
			list = list->next;
			continue;
		}

		no_files = dir_file_count(lib, 1);

		db_begin_transaction(cwin->cdbase);

		update_db(lib, no_files, progress_bar, cwin->cpref->last_rescan_time, 1, cancellable, cwin->cdbase);

		db_commit_transaction(cwin->cdbase);

		if (g_cancellable_is_cancelled (cancellable))
			goto exit;
		list = list->next;
	}

	/* Save update time */

	g_get_current_time(&cwin->cpref->last_rescan_time);

	/* Free lists */

	free_str_list(cwin->cpref->lib_add);
	free_str_list(cwin->cpref->lib_delete);

	cwin->cpref->lib_add = NULL;
	cwin->cpref->lib_delete = NULL;
exit:
	update_menu_playlist_changes(cwin);
	init_library_view(cwin);

	gtk_widget_destroy(library_dialog);

	if (!g_cancellable_is_cancelled (cancellable)) {
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));
		gtk_dialog_run(GTK_DIALOG(msg_dialog));
		gtk_widget_destroy(msg_dialog);
	}

	g_object_unref(cancellable);
}

/* Handler for 'Add All' action in the Tools menu */

void add_libary_action(GtkAction *action, struct con_win *cwin)
{
	gint i = 0, location_id = 0, cnt = 0;
	gchar *query;
	struct db_result result;
	struct musicobject *mobj;
	GtkTreeModel *model;

	set_watch_cursor (cwin->mainwindow);

	clear_current_playlist(action, cwin);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	g_object_ref(model);
	cwin->cstate->playlist_change = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), NULL);

	/* Query and insert entries */
	/* NB: Optimization */

	query = g_strdup_printf("SELECT id FROM LOCATION;");
	if (exec_sqlite_query(query, cwin->cdbase, &result)) {
		for_each_result_row(result, i) {
			location_id = atoi(result.resultp[i]);
			mobj = new_musicobject_from_db(location_id, cwin);

			if (!mobj)
				g_warning("Unable to retrieve details for"
					  " location_id : %d",
					  location_id);
			else
				append_current_playlist(model, mobj, cwin);

			/* Have to give control to GTK periodically ... */
			/* If gtk_main_quit has been called, return -
			   since main loop is no more. */

			if (cnt++ % 50)
				continue;

			while(gtk_events_pending()) {
				if (gtk_main_iteration_do(FALSE)) {
					sqlite3_free_table(result.resultp);
					return;
				}
			}
		}
		sqlite3_free_table(result.resultp);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(cwin->current_playlist), model);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->current_playlist), TRUE);
	cwin->cstate->playlist_change = FALSE;
	g_object_unref(model);

	remove_watch_cursor (cwin->mainwindow);

	select_last_path_of_current_playlist(cwin);
	update_status_bar(cwin);

	mpris_update_tracklist_replaced(cwin);
}


/* Handler for 'Statistics' action in the Tools menu */

void statistics_action(GtkAction *action, struct con_win *cwin)
{
	gint n_artists, n_albums, n_tracks;
	GtkWidget *dialog;

	n_artists = db_get_artist_count(cwin->cdbase);
	n_albums = db_get_album_count(cwin->cdbase);
	n_tracks = db_get_track_count(cwin->cdbase);

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s %d\n%s %d\n%s %d",
					_("Total Tracks:"),
					n_tracks,
					_("Total Artists:"),
					n_artists,
					_("Total Albums:"),
					n_albums);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Statistics"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/* Handler for the 'About' action in the Help menu */

void about_widget(struct con_win *cwin)
{
	const gchar *authors[] = {
		"sujith ( m.sujith@gmail.com )",
		"matias ( mati86dl@gmail.com )",
		NULL};

	gtk_show_about_dialog(GTK_WINDOW(cwin->mainwindow),
				"logo", cwin->pixbuf->pixbuf_app,
				"authors", authors,
				"translator-credits", _("translator-credits"),
				"comments", "A lightweight GTK+ music player",
				"copyright", "(C) 2007-2009 Sujith\n(C) 2009-2012 Matias",
				"license", license,
				"name", PACKAGE_NAME,
				"version", PACKAGE_VERSION,
				NULL);
}

void home_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(cwin, uri);
}

void community_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://bbs.archlinux.org/viewtopic.php?id=46171";
	open_url(cwin, uri);
}

void wiki_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(cwin, uri);
}

void translate_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://www.transifex.net/projects/p/Pragha/";
	open_url(cwin, uri);
}

void about_action(GtkAction *action, struct con_win *cwin)
{
	about_widget(cwin);
}

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

	update_menubar_playback_state (cwin);

	g_signal_connect (cwin->backend, "notify::state", G_CALLBACK (update_menubar_playback_state_cb), cwin);

	return main_menu;
}
