/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2020 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <locale.h> /* require LC_ALL */
#include <libintl.h>
#include <tag_c.h>

#include "pragha-hig.h"
#include "pragha-window.h"
#include "pragha-playback.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-menubar.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-music-enum.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-database-provider.h"

#ifdef G_OS_WIN32
#include "win32/win32dep.h"
#endif

struct _PraghaApplication {
	GtkApplication base_instance;

	/* Main window and icon */

	GtkWidget         *mainwindow;

	/* Main stuff */

	PraghaBackend          *backend;
	PraghaPreferences      *preferences;
	PraghaDatabase         *cdbase;
	PraghaDatabaseProvider *provider;
	PraghaArtCache         *art_cache;
	PraghaMusicEnum        *enum_map;

	PraghaScanner     *scanner;

	PraghaPreferencesDialog *setting_dialog;

	/* Main widgets */

	GtkUIManager      *menu_ui_manager;
	GtkBuilder        *menu_ui;
	PraghaToolbar     *toolbar;
	GtkWidget         *infobox;
	GtkWidget         *overlay;
	GtkWidget         *pane1;
	GtkWidget         *pane2;
	PraghaSidebar     *sidebar1;
	GtkWidget         *main_stack;
	PraghaSidebar     *sidebar2;
	PraghaLibraryPane *library;
	PraghaPlaylist    *playlist;
	PraghaStatusbar   *statusbar;

	PraghaStatusIcon  *status_icon;

	GBinding          *sidebar2_binding;

#ifdef HAVE_LIBPEAS
	PraghaPluginsEngine *plugins_engine;
#endif
};

G_DEFINE_TYPE (PraghaApplication, pragha_application, GTK_TYPE_APPLICATION);

/*
 * Some calbacks..
 */

/* Handler for the 'Open' item in the File menu */

static void
pragha_open_files_dialog_close_button_cb (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

static void
pragha_open_files_dialog_add_button_cb (GtkWidget *widget, gpointer data)
{
	PraghaPlaylist *playlist;
	GSList *files = NULL, *l;
	gboolean add_recursively;
	GList *mlist = NULL;

	GtkWidget *window = g_object_get_data(data, "window");
	GtkWidget *chooser = g_object_get_data(data, "chooser");
	GtkWidget *toggle = g_object_get_data(data, "toggle-button");
	PraghaApplication *pragha = g_object_get_data(data, "pragha");

	PraghaPreferences *preferences = pragha_application_get_preferences (pragha);

	add_recursively = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle));
	pragha_preferences_set_add_recursively (preferences, add_recursively);

	gchar *last_folder = gtk_file_chooser_get_current_folder ((GtkFileChooser *) chooser);
	pragha_preferences_set_last_folder (preferences, last_folder);
	g_free (last_folder);

	files = gtk_file_chooser_get_filenames((GtkFileChooser *) chooser);

	gtk_widget_destroy(window);

	if (files) {
		for (l = files; l != NULL; l = l->next) {
			mlist = append_mobj_list_from_unknown_filename(mlist, l->data);
		}
		g_slist_free_full(files, g_free);

		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list (playlist, mlist);
		g_list_free (mlist);
	}
}

static gboolean
pragha_open_files_dialog_keypress (GtkWidget   *dialog,
                                   GdkEventKey *event,
                                   gpointer     data)
{
    if (event->keyval == GDK_KEY_Escape) {
        gtk_widget_destroy(dialog);
        return TRUE;
    }
    return FALSE;
}

void
pragha_application_open_files (PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	GtkWidget *window, *hbox, *vbox, *chooser, *bbox, *toggle, *close_button, *add_button;
	gpointer storage;
	gint i = 0;
	GtkFileFilter *unsupported_filter, *media_filter, *playlist_filter, *all_filter;
	const gchar *last_folder = NULL;

	/* Create a file chooser dialog */

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_title(GTK_WINDOW(window), (_("Select a file to play")));
	gtk_window_set_default_size(GTK_WINDOW(window), 700, 450);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_set_name (GTK_WIDGET(window), "GtkFileChooserDialog");
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_name (GTK_WIDGET(vbox), "dialog-vbox1");

	gtk_container_add(GTK_CONTAINER(window), vbox);

	chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);

	/* Set various properties */

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), TRUE);

	preferences = pragha_application_get_preferences (pragha);
	last_folder = pragha_preferences_get_last_folder (preferences);
	if (string_is_not_empty(last_folder))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), last_folder);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	toggle = gtk_check_button_new_with_label(_("Add files recursively"));
	if(pragha_preferences_get_add_recursively (preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);

	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(bbox), 4);

	close_button = gtk_button_new_with_mnemonic (_("_Cancel"));
	add_button = gtk_button_new_with_mnemonic (_("_Add"));
	gtk_container_add(GTK_CONTAINER(bbox), close_button);
	gtk_container_add(GTK_CONTAINER(bbox), add_button);

	gtk_box_pack_start(GTK_BOX(hbox), toggle, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(hbox), bbox, FALSE, FALSE, 0);

	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), chooser, TRUE, TRUE, 0);

	/* Create file filters  */

	unsupported_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name(GTK_FILE_FILTER(unsupported_filter), _("Supported & partially supported media"));
	gtk_file_filter_add_mime_type(GTK_FILE_FILTER(unsupported_filter), "audio/*");
	gtk_file_filter_add_mime_type(GTK_FILE_FILTER(unsupported_filter), "video/*");

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

	i = 0;
	while (mime_asf[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
		                              mime_asf[i++]);
	i = 0;
	while (mime_mp4[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
		                              mime_mp4[i++]);
	i = 0;
	while (mime_ape[i])
		gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
		                              mime_ape[i++]);
	i = 0;
	while (mime_tracker[i])
	 gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter),
		                              mime_tracker[i++]);

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

	all_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (GTK_FILE_FILTER(all_filter), _("All files"));
	gtk_file_filter_add_pattern (GTK_FILE_FILTER(all_filter), "*");

	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser),
	                             GTK_FILE_FILTER(unsupported_filter));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser),
	                             GTK_FILE_FILTER(media_filter));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser),
	                             GTK_FILE_FILTER(playlist_filter));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser),
	                             GTK_FILE_FILTER(all_filter));

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser),
	                            GTK_FILE_FILTER(media_filter));

	storage = g_object_new(G_TYPE_OBJECT, NULL);
	g_object_set_data(storage, "window", window);
	g_object_set_data(storage, "chooser", chooser);
	g_object_set_data(storage, "toggle-button", toggle);
	g_object_set_data(storage, "pragha", pragha);

	g_signal_connect (add_button, "clicked",
	                  G_CALLBACK(pragha_open_files_dialog_add_button_cb), storage);
	g_signal_connect (chooser, "file-activated",
	                  G_CALLBACK(pragha_open_files_dialog_add_button_cb), storage);
	g_signal_connect (close_button, "clicked",
	                  G_CALLBACK(pragha_open_files_dialog_close_button_cb), window);
	g_signal_connect (window, "destroy",
	                  G_CALLBACK(gtk_widget_destroy), window);
	g_signal_connect (window, "key-press-event",
	                  G_CALLBACK(pragha_open_files_dialog_keypress), NULL);

	gtk_window_set_transient_for(GTK_WINDOW (window), GTK_WINDOW(pragha_application_get_window(pragha)));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);

	gtk_widget_show_all(window);
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

void
pragha_application_add_location (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	PraghaDatabase *cdbase;
	PraghaMusicobject *mobj;
	GtkWidget *dialog, *table, *uri_entry, *label_name, *name_entry;
	const gchar *uri = NULL, *name = NULL;
	gchar *clipboard_location = NULL, *real_name = NULL;
	GSList *list = NULL, *i = NULL;
	GList *mlist = NULL;
	guint row = 0;
	gint result;

	/* Create dialog window */

	table = pragha_hig_workarea_table_new ();
	pragha_hig_workarea_table_add_section_title(table, &row, _("Enter the URL of an internet radio stream"));

	uri_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(uri_entry), 255);

	pragha_hig_workarea_table_add_wide_control (table, &row, uri_entry);

	label_name = gtk_label_new_with_mnemonic(_("Give it a name to save"));
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 255);

	pragha_hig_workarea_table_add_row (table, &row, label_name, name_entry);

	/* Get item from clipboard to fill GtkEntry */
	clipboard_location = totem_open_location_set_from_clipboard (uri_entry);
	if (clipboard_location != NULL && strcmp (clipboard_location, "") != 0) {
		gtk_entry_set_text (GTK_ENTRY(uri_entry), clipboard_location);
		g_free (clipboard_location);
	}

	dialog = gtk_dialog_new_with_buttons (_("Add a location"),
	                                      GTK_WINDOW(pragha_application_get_window(pragha)),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Ok"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, -1);

	gtk_entry_set_activates_default (GTK_ENTRY(uri_entry), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(name_entry), TRUE);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		if (gtk_entry_get_text_length (GTK_ENTRY(uri_entry)))
			uri = gtk_entry_get_text(GTK_ENTRY(uri_entry));

		playlist = pragha_application_get_playlist (pragha);

		if (string_is_not_empty(uri)) {
			if (gtk_entry_get_text_length (GTK_ENTRY(name_entry)))
				name = gtk_entry_get_text(GTK_ENTRY(name_entry));

			#ifdef HAVE_PLPARSER
			list = pragha_totem_pl_parser_parse_from_uri (uri);
			#else
			list = g_slist_append (list, g_strdup(uri));
			#endif

			for (i = list; i != NULL; i = i->next) {
				if (string_is_not_empty(name))
					real_name = new_radio (playlist, i->data, name);

				mobj = new_musicobject_from_location (i->data, real_name);
				mlist = g_list_append(mlist, mobj);

				if (real_name) {
					g_free (real_name);
					real_name = NULL;
				}
				g_free(i->data);
			}
			g_slist_free(list);

			/* Append playlist and save on database */

			pragha_playlist_append_mobj_list (playlist, mlist);
			g_list_free(mlist);

			cdbase = pragha_application_get_database (pragha);
			pragha_database_change_playlists_done (cdbase);
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

/* Handler for 'Add All' action in the Tools menu */

void
pragha_application_append_entery_libary (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	PraghaDatabase *cdbase;
	GList *list = NULL;
	PraghaMusicobject *mobj;

	/* Query and insert entries */

	set_watch_cursor (pragha_application_get_window(pragha));

	cdbase = pragha_application_get_database (pragha);

	const gchar *sql = "SELECT id FROM LOCATION";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		gint location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db (cdbase, location_id);

		if (G_LIKELY(mobj))
			list = g_list_prepend (list, mobj);
		else
			g_warning ("Unable to retrieve details for"
			            " location_id : %d",
			            location_id);

		pragha_process_gtk_events ();
	}

	pragha_prepared_statement_free (statement);

	remove_watch_cursor (pragha_application_get_window(pragha));

	if (list) {
		list = g_list_reverse(list);
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list (playlist, list);
		g_list_free(list);
	}
}

/* Handler for the 'About' action in the Help menu */

void
pragha_application_about_dialog (PraghaApplication *pragha)
{
	GtkWidget *mainwindow;

	mainwindow = pragha_application_get_window (pragha);

	const gchar *authors[] = {
		"Matias De lellis <mati86dl@gmail.com>",
		NULL
	};

	gtk_show_about_dialog(GTK_WINDOW(mainwindow),
	                      "logo-icon-name", "pragha",
	                      "authors", authors,
	                      "comments", "A lightweight GTK+ music player",
	                      "copyright", "(C) 2009-2019 Matias \n Consonance (C) 2007-2009 Sujith",
	                      "license-type", GTK_LICENSE_GPL_3_0,
	                      "name", PACKAGE_NAME,
	                      "version", PACKAGE_VERSION,
	                      NULL);
}

static void
pragha_library_pane_append_tracks (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (pragha->playlist);

		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks_and_play (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (pragha->playlist);

		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);

		if (pragha_backend_get_state (pragha->backend) != ST_STOPPED)
			pragha_playback_next_track(pragha);
		else
			pragha_playback_play_pause_resume(pragha);

		g_list_free(list);
	}
}

static void
pragha_library_pane_addto_playlist_and_play (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_append_mobj_list(pragha->playlist, list);
		pragha_playlist_activate_unique_mobj(pragha->playlist, g_list_first(list)->data);
		
		g_list_free(list);
	}
}

static void
pragha_playlist_update_change_tags (PraghaPlaylist *playlist, gint changed, PraghaMusicobject *mobj, PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaMusicobject *cmobj = NULL;

	backend = pragha_application_get_backend (pragha);

	if(pragha_backend_get_state (backend) != ST_STOPPED) {
		cmobj = pragha_backend_get_musicobject (backend);
		pragha_update_musicobject_change_tag (cmobj, changed, mobj);

		toolbar = pragha_application_get_toolbar (pragha);
		pragha_toolbar_set_title (toolbar, cmobj);
	}
}

static void
pragha_playlist_update_statusbar_playtime (PraghaPlaylist *playlist, PraghaApplication *pragha)
{
	PraghaStatusbar *statusbar;
	gint total_playtime = 0, no_tracks = 0;
	gchar *str, *tot_str;

	if(pragha_playlist_is_changing(playlist))
		return;

	total_playtime = pragha_playlist_get_total_playtime (playlist);
	no_tracks = pragha_playlist_get_no_tracks (playlist);

	tot_str = convert_length_str(total_playtime);
	str = g_strdup_printf("%i %s - %s",
	                      no_tracks,
	                      ngettext("Track", "Tracks", no_tracks),
	                      tot_str);

	CDEBUG(DBG_VERBOSE, "Updating status bar with new playtime: %s", tot_str);

	statusbar = pragha_application_get_statusbar (pragha);
	pragha_statusbar_set_main_text(statusbar, str);

	g_free(tot_str);
	g_free(str);
}

static void
pragha_art_cache_changed_handler (PraghaArtCache *cache, PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaMusicobject *mobj = NULL;
	gchar *album_art_path = NULL;
	const gchar *artist = NULL, *album = NULL;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		mobj = pragha_backend_get_musicobject (backend);

		artist = pragha_musicobject_get_artist (mobj);
		album = pragha_musicobject_get_album (mobj);

		album_art_path = pragha_art_cache_get_album_uri (cache, artist, album);

		if (album_art_path) {
			toolbar = pragha_application_get_toolbar (pragha);
			pragha_toolbar_set_image_album_art (toolbar, album_art_path);
			g_free (album_art_path);
		}
	}
}

static void
pragha_libary_list_changed_cb (PraghaPreferences *preferences, PraghaApplication *pragha)
{
	GtkWidget *infobar = create_info_bar_update_music (pragha);
	pragha_window_add_widget_to_infobox (pragha, infobar);
}

static void
pragha_application_provider_want_update (PraghaDatabaseProvider *provider,
                                         gint                    provider_id,
                                         PraghaApplication      *pragha)
{
	PraghaDatabase *database;
	PraghaScanner *scanner;
	PraghaPreparedStatement *statement;
	const gchar *sql, *provider_type = NULL;

	sql = "SELECT name FROM provider_type WHERE id IN (SELECT type FROM provider WHERE id = ?)";

	database = pragha_application_get_database (pragha);
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, provider_id);
	if (pragha_prepared_statement_step (statement))
		provider_type = pragha_prepared_statement_get_string (statement, 0);

	if (g_ascii_strcasecmp (provider_type, "local") == 0)
	{
		scanner = pragha_application_get_scanner (pragha);
		pragha_scanner_update_library (scanner);
	}
	pragha_prepared_statement_free (statement);
}

static void
pragha_application_provider_want_upgrade (PraghaDatabaseProvider *provider,
                                          gint                    provider_id,
                                          PraghaApplication      *pragha)
{
	PraghaDatabase *database;
	PraghaScanner *scanner;
	PraghaPreparedStatement *statement;
	const gchar *sql, *provider_type = NULL;

	sql = "SELECT name FROM provider_type WHERE id IN (SELECT type FROM provider WHERE id = ?)";

	database = pragha_application_get_database (pragha);
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, provider_id);
	if (pragha_prepared_statement_step (statement))
		provider_type = pragha_prepared_statement_get_string (statement, 0);

	if (g_ascii_strcasecmp (provider_type, "local") == 0)
	{
		scanner = pragha_application_get_scanner (pragha);
		pragha_scanner_scan_library (scanner);
	}
	pragha_prepared_statement_free (statement);
}

static void
pragha_need_restart_cb (PraghaPreferences *preferences, PraghaApplication *pragha)
{
	GtkWidget *infobar = pragha_info_bar_need_restart (pragha);
	pragha_window_add_widget_to_infobox (pragha, infobar);
}

static void
pragha_system_titlebar_changed_cb (PraghaPreferences *preferences, GParamSpec *pspec, PraghaApplication *pragha)
{
	PraghaToolbar *toolbar;
	GtkWidget *window, *parent, *menubar;
	GtkAction *action;

	window = pragha_application_get_window (pragha);
	toolbar = pragha_application_get_toolbar (pragha);
	menubar = pragha_application_get_menubar (pragha);
	g_object_ref(toolbar);

	parent  = gtk_widget_get_parent (GTK_WIDGET(menubar));

	if (pragha_preferences_get_system_titlebar (preferences)) {
		gtk_widget_hide(GTK_WIDGET(window));

		action = pragha_application_get_menu_action (pragha,
			"/Menubar/ViewMenu/Fullscreen");
		gtk_action_set_sensitive (GTK_ACTION (action), TRUE);

		action = pragha_application_get_menu_action (pragha,
			"/Menubar/ViewMenu/Playback controls below");
		gtk_action_set_sensitive (GTK_ACTION (action), TRUE);

		gtk_window_set_titlebar (GTK_WINDOW (window), NULL);
		gtk_window_set_title (GTK_WINDOW(window), _("Pragha Music Player"));

		gtk_box_pack_start (GTK_BOX(parent), GTK_WIDGET(toolbar),
		                    FALSE, FALSE, 0);
		gtk_box_reorder_child(GTK_BOX(parent), GTK_WIDGET(toolbar), 1);

		pragha_toolbar_set_style(toolbar, TRUE);

		gtk_widget_show(GTK_WIDGET(window));

	}
	else {
		gtk_widget_hide(GTK_WIDGET(window));

		pragha_preferences_set_controls_below(preferences, FALSE);

		action = pragha_application_get_menu_action (pragha,
			"/Menubar/ViewMenu/Fullscreen");
		gtk_action_set_sensitive (GTK_ACTION (action), FALSE);

		action = pragha_application_get_menu_action (pragha,
			"/Menubar/ViewMenu/Playback controls below");
		gtk_action_set_sensitive (GTK_ACTION (action), FALSE);

		gtk_container_remove (GTK_CONTAINER(parent), GTK_WIDGET(toolbar));
		gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET(toolbar));

		pragha_toolbar_set_style(toolbar, FALSE);

		gtk_widget_show(GTK_WIDGET(window));
	}
	g_object_unref(toolbar);
}


static void
pragha_enum_map_removed_handler (PraghaMusicEnum *enum_map, gint enum_removed, PraghaApplication *pragha)
{
	pragha_playlist_crop_music_type (pragha->playlist, enum_removed);
}

/*
 * Some public actions.
 */

PraghaPreferences *
pragha_application_get_preferences (PraghaApplication *pragha)
{
	return pragha->preferences;
}

PraghaDatabase *
pragha_application_get_database (PraghaApplication *pragha)
{
	return pragha->cdbase;
}

PraghaArtCache *
pragha_application_get_art_cache (PraghaApplication *pragha)
{
	return pragha->art_cache;
}

PraghaBackend *
pragha_application_get_backend (PraghaApplication *pragha)
{
	return pragha->backend;
}

#ifdef HAVE_LIBPEAS
PraghaPluginsEngine *
pragha_application_get_plugins_engine (PraghaApplication *pragha)
{
	return pragha->plugins_engine;
}
#endif

PraghaScanner *
pragha_application_get_scanner (PraghaApplication *pragha)
{
	return pragha->scanner;
}

GtkWidget *
pragha_application_get_window (PraghaApplication *pragha)
{
	return pragha->mainwindow;
}

PraghaPlaylist *
pragha_application_get_playlist (PraghaApplication *pragha)
{
	return pragha->playlist;
}

PraghaLibraryPane *
pragha_application_get_library (PraghaApplication *pragha)
{
	return pragha->library;
}

PraghaPreferencesDialog *
pragha_application_get_preferences_dialog (PraghaApplication *pragha)
{
	return pragha->setting_dialog;
}

PraghaToolbar *
pragha_application_get_toolbar (PraghaApplication *pragha)
{
	return pragha->toolbar;
}

GtkWidget *
pragha_application_get_overlay (PraghaApplication *pragha)
{
	return pragha->overlay;
}

PraghaSidebar *
pragha_application_get_first_sidebar (PraghaApplication *pragha)
{
	return pragha->sidebar1;
}

GtkWidget *
pragha_application_get_main_stack (PraghaApplication *pragha)
{
	return pragha->main_stack;
}

PraghaSidebar *
pragha_application_get_second_sidebar (PraghaApplication *pragha)
{
	return pragha->sidebar2;
}

PraghaStatusbar *
pragha_application_get_statusbar (PraghaApplication *pragha)
{
	return pragha->statusbar;
}

PraghaStatusIcon *
pragha_application_get_status_icon (PraghaApplication *pragha)
{
	return pragha->status_icon;
}

GtkBuilder *
pragha_application_get_menu_ui (PraghaApplication *pragha)
{
	return pragha->menu_ui;
}

GtkUIManager *
pragha_application_get_menu_ui_manager (PraghaApplication *pragha)
{
	return pragha->menu_ui_manager;
}

GtkAction *
pragha_application_get_menu_action (PraghaApplication *pragha, const gchar *path)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_action (ui_manager, path);
}

GtkWidget *
pragha_application_get_menu_action_widget (PraghaApplication *pragha, const gchar *path)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_widget (ui_manager, path);
}

GtkWidget *
pragha_application_get_menubar (PraghaApplication *pragha)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_widget (ui_manager, "/Menubar");
}

GtkWidget *
pragha_application_get_infobox_container (PraghaApplication *pragha)
{
	return pragha->infobox;
}

GtkWidget *
pragha_application_get_first_pane (PraghaApplication *pragha)
{
	return pragha->pane1;
}

GtkWidget *
pragha_application_get_second_pane (PraghaApplication *pragha)
{
	return pragha->pane2;
}

gboolean
pragha_application_is_first_run (PraghaApplication *pragha)
{
	return string_is_empty (pragha_preferences_get_installed_version (pragha->preferences));
}

static void
pragha_application_construct_window (PraghaApplication *pragha)
{
	/* Main window */

	pragha->mainwindow = gtk_application_window_new (GTK_APPLICATION (pragha));

	gtk_window_set_icon_name (GTK_WINDOW(pragha->mainwindow), "pragha");


	/* Get all widgets instances */

	pragha->menu_ui_manager = pragha_menubar_new ();
	pragha->menu_ui = pragha_gmenu_toolbar_new (pragha);
	pragha->toolbar = pragha_toolbar_new ();
	pragha->infobox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	pragha->overlay = gtk_overlay_new ();
	pragha->pane1 = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	pragha->main_stack = gtk_stack_new ();
	pragha->pane2 = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	pragha->sidebar1 = pragha_sidebar_new ();
	pragha->sidebar2 = pragha_sidebar_new ();
	pragha->library = pragha_library_pane_new ();
	pragha->playlist = pragha_playlist_new ();
	pragha->statusbar = pragha_statusbar_get ();
	pragha->scanner = pragha_scanner_new();

	pragha->status_icon = pragha_status_icon_new (pragha);

	pragha_menubar_connect_signals (pragha->menu_ui_manager, pragha);

	/* Contruct the window. */

	pragha_window_new (pragha);

	gtk_window_set_title (GTK_WINDOW(pragha->mainwindow),
	                      _("Pragha Music Player"));
}

static void
pragha_application_dispose (GObject *object)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (object);

	CDEBUG(DBG_INFO, "Cleaning up");

#ifdef HAVE_LIBPEAS
	if (pragha->plugins_engine) {
		g_object_unref (pragha->plugins_engine);
		pragha->plugins_engine = NULL;
	}
#endif

	if (pragha->setting_dialog) {
		// Explicit destroy dialog.
		// TODO: Evaluate if needed.
		gtk_widget_destroy (GTK_WIDGET(pragha->setting_dialog));
		pragha->setting_dialog = NULL;
	}

	if (pragha->backend) {
		g_object_unref (pragha->backend);
		pragha->backend = NULL;
	}
	if (pragha->art_cache) {
		g_object_unref (pragha->art_cache);
		pragha->art_cache = NULL;
	}
	if (pragha->enum_map) {
		g_object_unref (pragha->enum_map);
		pragha->enum_map = NULL;
	}
	if (pragha->scanner) {
		pragha_scanner_free (pragha->scanner);
		pragha->scanner = NULL;
	}
	if (pragha->menu_ui_manager) {
		g_object_unref (pragha->menu_ui_manager);
		pragha->menu_ui_manager = NULL;
	}
	if (pragha->menu_ui) {
		g_object_unref (pragha->menu_ui);
		pragha->menu_ui = NULL;
	}

	/* Save Preferences and database. */

	if (pragha->preferences) {
		g_object_unref (pragha->preferences);
		pragha->preferences = NULL;
	}
	if (pragha->provider) {
		g_object_unref (pragha->provider);
		pragha->provider = NULL;
	}
	if (pragha->cdbase) {
		g_object_unref (pragha->cdbase);
		pragha->cdbase = NULL;
	}

	G_OBJECT_CLASS (pragha_application_parent_class)->dispose (object);
}

static void
pragha_application_startup (GApplication *application)
{
	PraghaToolbar *toolbar;
	PraghaPlaylist *playlist;
	const gchar *version = NULL;
	const gchar *desktop = NULL;
	gint playlist_id = 0;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	G_APPLICATION_CLASS (pragha_application_parent_class)->startup (application);

	/* Allocate memory for simple structures */

	pragha->preferences = pragha_preferences_get();

	pragha->cdbase = pragha_database_get();
	if (pragha_database_start_successfully(pragha->cdbase) == FALSE) {
		g_error("Unable to init music dbase");
	}

	version = pragha_preferences_get_installed_version (pragha->preferences);
	if (string_is_not_empty (version) && (g_ascii_strcasecmp (version, "1.3.90") < 0)) {
		CDEBUG(DBG_INFO, "Compatibilize database to new version.");
		pragha_database_compatibilize_version (pragha->cdbase);
	}

	playlist_id = pragha_database_find_playlist (pragha->cdbase, _("Favorites"));
	if (playlist_id == 0)
		pragha_database_add_new_playlist (pragha->cdbase, _("Favorites"));

	pragha->provider = pragha_database_provider_get ();
	g_signal_connect (pragha->provider, "want-upgrade",
	                  G_CALLBACK(pragha_application_provider_want_upgrade), pragha);
	g_signal_connect (pragha->provider, "want-update",
	                  G_CALLBACK(pragha_application_provider_want_update), pragha);

	pragha->enum_map = pragha_music_enum_get ();
	g_signal_connect (pragha->enum_map, "enum-removed",
	                  G_CALLBACK(pragha_enum_map_removed_handler), pragha);

#ifdef HAVE_LIBPEAS
	pragha->plugins_engine = pragha_plugins_engine_new (G_OBJECT(pragha));
#endif

	pragha->art_cache = pragha_art_cache_get ();
	g_signal_connect (pragha->art_cache, "cache-changed",
	                  G_CALLBACK(pragha_art_cache_changed_handler), pragha);

	pragha->backend = pragha_backend_new ();

	g_signal_connect (pragha->backend, "finished",
	                  G_CALLBACK(pragha_backend_finished_song), pragha);
	g_signal_connect (pragha->backend, "tags-changed",
	                  G_CALLBACK(pragha_backend_tags_changed), pragha);

	g_signal_connect (pragha->backend, "error",
	                  G_CALLBACK(gui_backend_error_update_current_playlist_cb), pragha);
	g_signal_connect (pragha->backend, "error",
	                 G_CALLBACK(pragha_backend_finished_error), pragha);
	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), pragha);

	/*
	 * Collect widgets and construct the window.
	 */

	pragha_application_construct_window (pragha);

	/* Connect Signals and Bindings. */

	toolbar = pragha->toolbar;
	g_signal_connect_swapped (toolbar, "prev",
	                          G_CALLBACK(pragha_playback_prev_track), pragha);
	g_signal_connect_swapped (toolbar, "play",
	                          G_CALLBACK(pragha_playback_play_pause_resume), pragha);
	g_signal_connect_swapped (toolbar, "stop",
	                          G_CALLBACK(pragha_playback_stop), pragha);
	g_signal_connect_swapped (toolbar, "next",
	                          G_CALLBACK(pragha_playback_next_track), pragha);
	g_signal_connect (toolbar, "unfull-activated",
	                  G_CALLBACK(pragha_window_unfullscreen), pragha);
	g_signal_connect (toolbar, "album-art-activated",
	                  G_CALLBACK(pragha_playback_show_current_album_art), pragha);
	g_signal_connect_swapped (toolbar, "track-info-activated",
	                          G_CALLBACK(pragha_playback_edit_current_track), pragha);
	g_signal_connect (toolbar, "track-progress-activated",
	                  G_CALLBACK(pragha_playback_seek_fraction), pragha);
	g_signal_connect (toolbar, "favorite-toggle",
	                  G_CALLBACK(pragha_playback_toogle_favorite), pragha);

	playlist = pragha->playlist;
	g_signal_connect (playlist, "playlist-set-track",
	                  G_CALLBACK(pragha_playback_set_playlist_track), pragha);
	g_signal_connect (playlist, "playlist-change-tags",
	                  G_CALLBACK(pragha_playlist_update_change_tags), pragha);
	g_signal_connect (playlist, "playlist-changed",
	                  G_CALLBACK(pragha_playlist_update_statusbar_playtime), pragha);
	pragha_playlist_update_statusbar_playtime (playlist, pragha);

	g_signal_connect (pragha->library, "library-append-playlist",
	                  G_CALLBACK(pragha_library_pane_append_tracks), pragha);
	g_signal_connect (pragha->library, "library-replace-playlist",
	                  G_CALLBACK(pragha_library_pane_replace_tracks), pragha);
	g_signal_connect (pragha->library, "library-replace-playlist-and-play",
	                  G_CALLBACK(pragha_library_pane_replace_tracks_and_play), pragha);
	g_signal_connect (pragha->library, "library-addto-playlist-and-play",
	                  G_CALLBACK(pragha_library_pane_addto_playlist_and_play), pragha);
	                  
	g_signal_connect (G_OBJECT(pragha->mainwindow), "window-state-event",
	                  G_CALLBACK(pragha_toolbar_window_state_event), toolbar);
	g_signal_connect (G_OBJECT(toolbar), "notify::timer-remaining-mode",
	                  G_CALLBACK(pragha_toolbar_show_ramaning_time_cb), pragha->backend);

	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK(pragha_toolbar_playback_state_cb), toolbar);
	g_signal_connect (pragha->backend, "tick",
	                 G_CALLBACK(pragha_toolbar_update_playback_progress), toolbar);
	g_signal_connect (pragha->backend, "buffering",
	                  G_CALLBACK(pragha_toolbar_update_buffering_cb), toolbar);

	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK (update_current_playlist_view_playback_state_cb), pragha->playlist);

	g_object_bind_property (pragha->backend, "volume",
	                        toolbar, "volume",
	                        binding_flags);

	g_object_bind_property (pragha->preferences, "timer-remaining-mode",
	                        toolbar, "timer-remaining-mode",
	                        binding_flags);

	g_signal_connect (pragha->preferences, "LibraryChanged",
	                  G_CALLBACK (pragha_libary_list_changed_cb), pragha);
	g_signal_connect (pragha->preferences, "NeedRestart",
	                  G_CALLBACK (pragha_need_restart_cb), pragha);

	g_signal_connect (pragha->preferences, "notify::system-titlebar",
	                  G_CALLBACK (pragha_system_titlebar_changed_cb), pragha);

	pragha->sidebar2_binding =
		g_object_bind_property (pragha->preferences, "secondary-lateral-panel",
		                        pragha->sidebar2, "visible",
		                        binding_flags);

	pragha->setting_dialog = pragha_preferences_dialog_get ();
	pragha_preferences_dialog_set_parent (pragha->setting_dialog, GTK_WIDGET (pragha->mainwindow));

	#ifdef HAVE_LIBPEAS
	gboolean sidebar2_visible = // FIXME: Hack to allow hide sidebar when init.
		pragha_preferences_get_secondary_lateral_panel(pragha->preferences);

	pragha_plugins_engine_startup (pragha->plugins_engine);

	pragha_preferences_set_secondary_lateral_panel(pragha->preferences,
	                                               sidebar2_visible);
	#endif

	/* If first run and the desktop is gnome adapts style. */

	if (pragha_application_is_first_run (pragha)) {
		desktop = g_getenv ("XDG_CURRENT_DESKTOP");
		if (desktop && (g_strcmp0(desktop, "GNOME") == 0) &&
			gdk_screen_is_composited (gdk_screen_get_default())) {
			pragha_preferences_set_system_titlebar (pragha->preferences, FALSE);
			pragha_preferences_set_toolbar_size (pragha->preferences, GTK_ICON_SIZE_SMALL_TOOLBAR);
			pragha_preferences_set_show_menubar (pragha->preferences, FALSE);
		}
	}

	/* Forse update menubar and toolbar playback actions */

	pragha_menubar_update_playback_state_cb (pragha->backend, NULL, pragha);
	pragha_toolbar_playback_state_cb (pragha->backend, NULL, pragha->toolbar);

	/* Finally fill the library and the playlist */

	pragha_init_gui_state (pragha);
}

static void
pragha_application_shutdown (GApplication *application)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	CDEBUG(DBG_INFO, "Pragha shutdown: Saving curret state.");

	if (pragha_preferences_get_restore_playlist (pragha->preferences))
		pragha_playlist_save_playlist_state (pragha->playlist);

	pragha_window_save_settings (pragha);

	pragha_playback_stop (pragha);

	/* Shutdown plugins can hide sidebar before save settings. */
	if (pragha->sidebar2_binding) {
		g_object_unref (pragha->sidebar2_binding);
		pragha->sidebar2_binding = NULL;
	}

#ifdef HAVE_LIBPEAS
	pragha_plugins_engine_shutdown (pragha->plugins_engine);
#endif

	gtk_widget_destroy (pragha->mainwindow);

	G_APPLICATION_CLASS (pragha_application_parent_class)->shutdown (application);
}

static void
pragha_application_activate (GApplication *application)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	CDEBUG(DBG_INFO, G_STRFUNC);

	gtk_window_present (GTK_WINDOW (pragha->mainwindow));
}

static void
pragha_application_open (GApplication *application, GFile **files, gint n_files, const gchar *hint)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);
	gint i;
	GList *mlist = NULL;

	for (i = 0; i < n_files; i++) {
		gchar *path = g_file_get_path (files[i]);
		mlist = append_mobj_list_from_unknown_filename (mlist, path);
		g_free (path);
	}

	if (mlist) {
		pragha_playlist_append_mobj_list (pragha->playlist, mlist);
		g_list_free (mlist);
	}

	gtk_window_present (GTK_WINDOW (pragha->mainwindow));
}

static int
pragha_application_command_line (GApplication *application, GApplicationCommandLine *command_line)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);
	int ret = 0;
	gint argc;

	gchar **argv = g_application_command_line_get_arguments (command_line, &argc);

	if (argc <= 1) {
		pragha_application_activate (application);
		goto exit;
	}

	ret = handle_command_line (pragha, command_line, argc, argv);

exit:
	g_strfreev (argv);

	return ret;
}

//it's used for --help and --version
static gboolean
pragha_application_local_command_line (GApplication *application, gchar ***arguments, int *exit_status)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	gchar **argv = *arguments;
	gint argc = g_strv_length (argv);

	*exit_status = handle_command_line (pragha, NULL, argc, argv);

	return FALSE;
}

void
pragha_application_quit (PraghaApplication *pragha)
{
	g_application_quit (G_APPLICATION (pragha));
}

static void
pragha_application_class_init (PraghaApplicationClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GApplicationClass *application_class = G_APPLICATION_CLASS (class);

	object_class->dispose = pragha_application_dispose;

	application_class->startup = pragha_application_startup;
	application_class->shutdown = pragha_application_shutdown;
	application_class->activate = pragha_application_activate;
	application_class->open = pragha_application_open;
	application_class->command_line = pragha_application_command_line;
	application_class->local_command_line = pragha_application_local_command_line;
}

static void
pragha_application_init (PraghaApplication *pragha)
{
}

PraghaApplication *
pragha_application_new ()
{
	return g_object_new (PRAGHA_TYPE_APPLICATION,
	                     "application-id", "io.github.pragha_music_player",
	                     "flags", G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}
