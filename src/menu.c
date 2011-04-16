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

gulong signal_search_click;
gulong signal_search_key;

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

/* Signal handler for deleting rescan dialog box */

static gboolean rescan_dialog_delete_cb(GtkWidget *widget,
					GdkEvent *event,
					struct con_win *cwin)
{
	cwin->cstate->stop_scan = TRUE;
	return TRUE;
}

/* Signal handler for cancelling rescan dialog box */

static void rescan_dialog_response_cb(GtkDialog *dialog,
				      gint response_id,
				      struct con_win *cwin)
{
	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		cwin->cstate->stop_scan = TRUE;
		break;
	default:
		break;
	}
}

/* Add selected files from the file chooser to the current playlist */

void handle_selected_file(gpointer data, gpointer udata)
{
	enum file_type type;
	GtkRecentData recent_data;

	struct musicobject *mobj;
	struct con_win *cwin = (struct con_win*)udata;

	if (!data)
		return;

	if (g_file_test(data, G_FILE_TEST_IS_DIR)){
		if(cwin->cpref->add_recursively_files)
			__recur_add(data, cwin);
		else
			__non_recur_add(data, TRUE, cwin);
		g_free(data);
		return;
	}

	if (is_m3u_playlist(data)){
		open_m3u_playlist(data, cwin);
		recent_data.mime_type = "audio/x-mpegurl";
	}
	else{
		mobj = new_musicobject_from_file(data);
		if (mobj)
			append_current_playlist(mobj, cwin);

		/* generate the recently-used data */

		type = get_file_type(data);
		switch(type) {
		case FILE_WAV:
			recent_data.mime_type = "audio/x-wav";
			break;
		case FILE_MP3:
			recent_data.mime_type = "audio/mpeg";
			break;
		case FILE_FLAC:
			recent_data.mime_type = "audio/x-flac";
			break;
		case FILE_OGGVORBIS:
			recent_data.mime_type = "audio/x-vorbis+ogg";
			break;
		case FILE_MODPLUG:
			recent_data.mime_type = "audio/x-mod";
			break;
		default:
			break;
		}
	}

	recent_data.display_name = NULL;
	recent_data.description = NULL;
	recent_data.app_name = "Pragha Music Player";
	recent_data.app_exec = "pragha %f";
	recent_data.groups = NULL;
	recent_data.is_private = FALSE;
	
	gtk_recent_manager_add_full(gtk_recent_manager_get_default(), g_filename_to_uri(data, NULL, NULL), &recent_data);

	g_free(data);
}

/* Create a dialog box with a progress bar for rescan/update library */

static GtkWidget* lib_progress_bar(struct con_win *cwin, int update)
{
	GtkWidget *progress_bar;

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

	/* Set various properties */

	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(progress_bar),
					 GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_widget_set_size_request(progress_bar,
				    PROGRESS_BAR_WIDTH,
				    -1);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(GTK_DIALOG(
						 library_dialog)->action_area),
				  GTK_BUTTONBOX_SPREAD);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar),
				  PROGRESS_BAR_TEXT);

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(GTK_WINDOW(library_dialog)), "delete_event",
			 G_CALLBACK(rescan_dialog_delete_cb), cwin);
	g_signal_connect(G_OBJECT(library_dialog), "response",
			 G_CALLBACK(rescan_dialog_response_cb), cwin);

	/* Add the progress bar to the dialog box's vbox and show everything */

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(library_dialog)->vbox),
			  progress_bar);
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
	GtkWidget *window = g_object_get_data(data, "window");
	GtkWidget *chooser = g_object_get_data(data, "chooser");
	GtkWidget *toggle = g_object_get_data(data, "toggle-button");
	struct con_win *cwin = g_object_get_data(data, "cwin");

	cwin->cpref->add_recursively_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle));

	GSList *files;

	files = gtk_file_chooser_get_filenames((GtkFileChooser *) chooser);
	if (files) {
		cwin->cstate->last_folder = gtk_file_chooser_get_current_folder ((GtkFileChooser *) chooser);
		g_slist_foreach(files, handle_selected_file, cwin);
		g_slist_free(files);
	}

	gtk_widget_destroy(window);
}

static gboolean
open_file_on_keypress(GtkWidget *dialog,
                        GdkEventKey *event,
                        gpointer data)
{
    if (event->keyval == GDK_Escape) {
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
	GtkFileFilter *media_filter, *all_filter;

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
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(media_filter), "*.m3u");

	all_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(all_filter), _("All files"));
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(all_filter), "*.*");

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser),
				    GTK_FILE_FILTER(media_filter));
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
	g_signal_connect(window, "key_press_event",
			G_CALLBACK(open_file_on_keypress), NULL);

	gtk_window_set_transient_for(GTK_WINDOW (window), GTK_WINDOW(cwin->mainwindow));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);

	gtk_widget_show_all(window);
}

/* Handler for the 'Play Audio CD' item in the pragha menu */

void play_audio_cd_action(GtkAction *action, struct con_win *cwin)
{
	play_audio_cd(cwin);
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
	stop_playback(cwin);
}

/* Handler for the 'Next' item in the pragha menu */

void next_action (GtkAction *action, struct con_win *cwin)
{
	play_next_track(cwin);
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
	gboolean ret;

	gtk_widget_grab_focus(cwin->current_playlist);
	g_signal_emit_by_name(G_OBJECT(cwin->current_playlist),
			      "start-interactive-search", &ret);
}

/* Handler for 'Shuffle' option in the Edit menu */

void shuffle_action(GtkToggleAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "shuffle_action");

	cwin->cpref->shuffle = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	g_signal_handlers_block_by_func (cwin->shuffle_button, shuffle_button_handler, cwin);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->shuffle_button), cwin->cpref->shuffle);
		shuffle_button(cwin);

	g_signal_handlers_unblock_by_func (cwin->shuffle_button, shuffle_button_handler, cwin);
}

/* Handler for 'Repeat' option in the Edit menu */

void repeat_action(GtkToggleAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Repeat_action");

	cwin->cpref->repeat = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	g_signal_handlers_block_by_func (cwin->repeat_button, repeat_button_handler, cwin);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->repeat_button), cwin->cpref->repeat);
		
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
		gtk_widget_show(cwin->unfull_button);
		gtk_widget_hide(GTK_WIDGET(menu_bar));
	}
	else {
		state = gdk_window_get_state (GTK_WIDGET (cwin->mainwindow)->window);
		if (state & GDK_WINDOW_STATE_FULLSCREEN){
			gtk_window_unfullscreen(GTK_WINDOW(cwin->mainwindow));
			gtk_widget_hide(cwin->unfull_button);
			gtk_widget_show(GTK_WIDGET(menu_bar));
		}
	}
}

/* Handler for the 'Library panel' item in the Edit menu */

void
library_pane_action (GtkAction *action, struct con_win *cwin)
{
	gboolean ret;
	ret = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(cwin->toggle_lib), ret);
}

/* Handler for the 'File panel' item in the Edit menu */

void
playlists_pane_action (GtkAction *action, struct con_win *cwin)
{
	gboolean ret;
	ret = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(cwin->toggle_playlists), ret);
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

void
jump_to_playing_song_action (GtkAction *action, struct con_win *cwin)
{
	jump_to_playing_song(cwin);
}

/* Handler for the 'Rescan Library' item in the Tools menu */

void rescan_library_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *msg_dialog;
	GtkWidget *progress_bar;
	gint no_files = 0, i, cnt = 0;
	GSList *list;
	gchar *lib;
	gchar *query;

	/* Check if Library is set */

	if (!cwin->cpref->library_dir) {
		g_warning("Library is not set, flushing existing library");
		flush_db(cwin);
		init_library_view(cwin);
		return ;
	}

	/* Check if versions are incompatible, if so drop tables and
	   initialize schema, otherwise, just flush the library database */

	if (is_incompatible_upgrade(cwin)) {
		if (drop_dbase_schema(cwin) == -1) {
			g_critical("Unable to drop database schema");
			return;
		}
		if (init_dbase_schema(cwin) == -1) {
			g_critical("Unable to init database schema");
			return;
		}
	} else {
		flush_db(cwin);
	}

	/* Create the dialog */

	progress_bar = lib_progress_bar(cwin, 0);

	/* Start the scan */

	list = cwin->cpref->library_dir;
	cnt = g_slist_length(cwin->cpref->library_dir);
	cwin->cstate->stop_scan = FALSE;

	for (i=0; i<cnt; i++) {
		lib = (gchar*)list->data;
		no_files = dir_file_count(lib, 1);

		query = g_strdup_printf("BEGIN TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		rescan_db(lib, no_files, progress_bar, 1, cwin);

		query = g_strdup_printf("END TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		list = list->next;
	}

	init_library_view(cwin);
	gtk_widget_destroy(library_dialog);

	if (!cwin->cstate->stop_scan) {
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));
		gtk_dialog_run(GTK_DIALOG(msg_dialog));
		gtk_widget_destroy(msg_dialog);
	}
	
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
	gint no_files = 0, i, cnt = 0;
	GSList *list;
	gchar *lib;
	gchar *query;

	/* Create the dialog */

	progress_bar = lib_progress_bar(cwin, 1);

	/* To track user termination */

	cwin->cstate->stop_scan = FALSE;

	/* Check if any library has been removed */

	list = cwin->cpref->lib_delete;
	cnt = g_slist_length(cwin->cpref->lib_delete);

	for (i=0; i<cnt; i++) {
		lib = (gchar*)list->data;
		no_files = dir_file_count(lib, 1);

		query = g_strdup_printf("BEGIN TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		delete_db(lib, no_files, progress_bar, 1, cwin);

		query = g_strdup_printf("END TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		if (cwin->cstate->stop_scan)
			goto exit;
		list = list->next;
	}

	/* Check if any library has been added */

	list = cwin->cpref->lib_add;
	cnt = g_slist_length(cwin->cpref->lib_add);

	for (i=0; i<cnt; i++) {
		lib = (gchar*)list->data;
		no_files = dir_file_count(lib, 1);

		query = g_strdup_printf("BEGIN TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		rescan_db(lib, no_files, progress_bar, 1, cwin);

		query = g_strdup_printf("END TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		if (cwin->cstate->stop_scan)
			goto exit;
		list = list->next;
	}

	/* Check if any files in the existing library dirs
	   have been modified */

	list = cwin->cpref->library_dir;
	cnt = g_slist_length(cwin->cpref->library_dir);

	for (i=0; i<cnt; i++) {
		lib = (gchar*)list->data;

		/* Don't rescan if lib is present in lib_add,
		   we just rescanned it above */

		if (is_present_str_list(lib, cwin->cpref->lib_add)) {
			list = list->next;
			continue;
		}

		no_files = dir_file_count(lib, 1);

		query = g_strdup_printf("BEGIN TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		update_db(lib, no_files, progress_bar, 1, cwin);

		query = g_strdup_printf("END TRANSACTION");
		exec_sqlite_query(query, cwin, NULL);

		if (cwin->cstate->stop_scan)
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
	init_library_view(cwin);
	gtk_widget_destroy(library_dialog);

	if (!cwin->cstate->stop_scan) {
		msg_dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
						    GTK_DIALOG_MODAL,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "%s",
						    _("Library scan complete"));
		gtk_dialog_run(GTK_DIALOG(msg_dialog));
		gtk_widget_destroy(msg_dialog);
	}
}

/* Handler for 'Add All' action in the Tools menu */

void add_all_action(GtkAction *action, struct con_win *cwin)
{
	gint i = 0, location_id = 0, cnt = 0;
	gchar *query;
	struct db_result result;
	struct musicobject *mobj;

	clear_current_playlist(action, cwin);

	/* Query and insert entries */
	/* NB: Optimization */

	query = g_strdup_printf("SELECT id FROM LOCATION;");
	if (exec_sqlite_query(query, cwin, &result)) {
		for_each_result_row(result, i) {
			location_id = atoi(result.resultp[i]);
			mobj = new_musicobject_from_db(location_id, cwin);

			if (!mobj)
				g_warning("Unable to retrieve details for"
					  " location_id : %d",
					  location_id);
			else
				append_current_playlist(mobj, cwin);

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
}

/* Handler for 'Statistics' action in the Tools menu */

void statistics_action(GtkAction *action, struct con_win *cwin)
{
	gchar *query;
	gint n_artists = 0, n_albums = 0, n_tracks = 0;
	struct db_result result;
	GtkWidget *dialog;

	query = g_strdup_printf("SELECT COUNT() FROM ARTIST;");
	if (exec_sqlite_query(query, cwin, &result)) {
		n_artists = atoi(result.resultp[1]);
		sqlite3_free_table(result.resultp);
	}
	query = g_strdup_printf("SELECT COUNT() FROM ALBUM;");
	if (exec_sqlite_query(query, cwin, &result)) {
		n_albums = atoi(result.resultp[1]);
		sqlite3_free_table(result.resultp);
	}
	query = g_strdup_printf("SELECT COUNT() FROM TRACK;");
	if (exec_sqlite_query(query, cwin, &result)) {
		n_tracks = atoi(result.resultp[1]);
		sqlite3_free_table(result.resultp);
	}

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
				"copyright", "(C) 2007-2009 Sujith\n(C) 2009-2010 Matias",
				"license", license,
				"name", PACKAGE_NAME,
				"version", PACKAGE_VERSION,
				NULL);
}

void lyric_action(GtkAction *action, struct con_win *cwin)
{
	if (cwin->cstate->state != ST_STOPPED){
		gchar *uri = g_markup_printf_escaped("http://www.lyricsplugin.com/winamp03/plugin/?artist=%s&title=%s",
							cwin->cstate->curr_mobj->tags->artist,
							cwin->cstate->curr_mobj->tags->title);
	open_url(cwin, uri);
	g_free(uri);
	}
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

void about_action(GtkAction *action, struct con_win *cwin)
{
	about_widget(cwin);
}
