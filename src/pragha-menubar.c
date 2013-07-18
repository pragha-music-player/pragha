/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <gdk/gdkkeysyms.h>
#include "pragha-menubar.h"
#include "pragha-playback.h"
#include "pragha-library-pane.h"
#include "pragha-lastfm.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-filter-dialog.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-tagger.h"
#include "pragha-tags-dialog.h"
#include "pragha-tags-mgmt.h"
#include "pragha-preferences-dialog.h"
#include "pragha-glyr.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-equalizer-dialog.h"
#include "pragha.h"

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
			<menuitem action=\"Save playlist\"/>			\
			<menuitem action=\"Save selection\"/>			\
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
			<placeholder name=\"pragha-glyr-placeholder\"/>		\
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
	 "<Control>L", "Clear the current playlist", G_CALLBACK(current_playlist_clear_action)},
	{"Save playlist", GTK_STOCK_SAVE_AS, N_("Save playlist")},
	{"Save selection", NULL, N_("Save selection")},
	{"Search in playlist", GTK_STOCK_FIND, N_("_Search in playlist"),
	 "<Control>F", "Search in playlist", G_CALLBACK(search_playlist_action)},
	{"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"),
	 "<Control>P", "Set preferences", G_CALLBACK(pref_action)},
	{"Jump to playing song", GTK_STOCK_JUMP_TO, N_("Jump to playing song"),
	 "<Control>J", "Jump to playing song", G_CALLBACK(jump_to_playing_song_action)},
	{"Equalizer", NULL, N_("E_qualizer"),
	 "", "Equalizer", G_CALLBACK(show_equalizer_action)},
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
	 "<Control>U", "Shuffle Songs", NULL,
	 FALSE},
	{"Repeat", NULL, N_("_Repeat"),
	 "<Control>R", "Repeat Songs", NULL,
	 FALSE},
	{"Fullscreen", NULL, N_("_Fullscreen"),
	 "F11", "Switch between full screen and windowed mode", G_CALLBACK(fullscreen_action),
	FALSE},
	{"Lateral panel", NULL, N_("Lateral _panel"),
	 "F9", "Lateral panel", NULL,
	TRUE},
	{"Playback controls below", NULL, N_("Playback controls below"),
	 NULL, "Show playback controls below", G_CALLBACK(show_controls_below_action),
	FALSE},
	{"Status bar", NULL, N_("Status bar"),
	 "", "Status bar", NULL,
	TRUE}
};

/* Sentitive menubar actions depending on the playback status. */

static void
update_menubar_playback_state_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;
	enum player_state state = pragha_backend_get_state (cwin->backend);
	GtkAction *action;

	gboolean playing = (state != ST_STOPPED);

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

	#ifdef HAVE_LIBCLASTFM
	update_menubar_lastfm_state (cwin);
	#endif
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
	GSList *files = NULL, *l;
	gboolean add_recursively;
	GList *mlist = NULL;

	GtkWidget *window = g_object_get_data(data, "window");
	GtkWidget *chooser = g_object_get_data(data, "chooser");
	GtkWidget *toggle = g_object_get_data(data, "toggle-button");
	struct con_win *cwin = g_object_get_data(data, "cwin");

	add_recursively = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle));
	pragha_preferences_set_add_recursively(cwin->preferences, add_recursively);

	gchar *last_folder = gtk_file_chooser_get_current_folder ((GtkFileChooser *) chooser);
	pragha_preferences_set_last_folder (cwin->preferences, last_folder);
	g_free (last_folder);

	files = gtk_file_chooser_get_filenames((GtkFileChooser *) chooser);

	gtk_widget_destroy(window);

	if (files) {
		for (l = files; l != NULL; l = l->next) {
			mlist = append_mobj_list_from_unknown_filename(mlist, l->data);
		}
		g_slist_free_full(files, g_free);

		pragha_playlist_append_mobj_list(cwin->cplaylist, mlist);
	}
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

	const gchar *last_folder = pragha_preferences_get_last_folder (cwin->preferences);
	if (string_is_not_empty(last_folder))
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), last_folder);

	hbox = gtk_hbox_new(FALSE, 0);

	toggle = gtk_check_button_new_with_label(_("Add recursively files"));
	if(pragha_preferences_get_add_recursively(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);

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
	PraghaMusicobject *mobj;
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

			mobj = new_musicobject_from_location(uri, name);

			pragha_playlist_append_single_song(cwin->cplaylist, mobj);

			if (name) {
				new_radio(cwin->cplaylist, uri, name);
				pragha_database_change_playlists_done(cwin->cdbase);
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
	pragha_playback_prev_track(cwin);
}

/* Handler for the 'Play / Pause' item in the pragha menu */

void play_pause_action(GtkAction *action, struct con_win *cwin)
{
	pragha_playback_play_pause_resume(cwin);
}

/* Handler for the 'Stop' item in the pragha menu */

void stop_action(GtkAction *action, struct con_win *cwin)
{
	pragha_playback_stop(cwin);
}

/* Handler for the 'Next' item in the pragha menu */

void next_action (GtkAction *action, struct con_win *cwin)
{
	pragha_playback_next_track(cwin);
}

static void
pragha_edit_tags_dialog_response (GtkWidget      *dialog,
                                  gint            response_id,
                                  struct con_win *cwin)
{
	PraghaMusicobject *nmobj, *bmobj;
	PraghaTagger *tagger;
	gint changed = 0;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, cwin->mainwindow);
		return;
	}

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if(changed) {
			nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

			if(pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
				PraghaMusicobject *current_mobj = pragha_backend_get_musicobject (cwin->backend);
				if (pragha_musicobject_compare (nmobj, current_mobj) == 0) {
					/* Update public current song */
					pragha_update_musicobject_change_tag (current_mobj, changed, nmobj);

					/* Update current song on playlist */
					pragha_playlist_update_current_track(cwin->cplaylist, changed, nmobj);

					/* Update current song on backend */
					bmobj = g_object_ref(pragha_backend_get_musicobject(cwin->backend));
					pragha_update_musicobject_change_tag(bmobj, changed, nmobj);
					g_object_unref(bmobj);

					__update_current_song_info(cwin);
					mpris_update_metadata_changed(cwin);
				}
			}

			if(G_LIKELY(pragha_musicobject_is_local_file (nmobj))) {
				tagger = pragha_tagger_new();
				pragha_tagger_add_file (tagger, pragha_musicobject_get_file(nmobj));
				pragha_tagger_set_changes(tagger, nmobj, changed);
				pragha_tagger_apply_changes (tagger);
				g_object_unref(tagger);
			}
		}
	}
	gtk_widget_destroy (dialog);
}

void edit_tags_playing_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	dialog = pragha_tags_dialog_new();

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_edit_tags_dialog_response), cwin);

	pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), pragha_backend_get_musicobject (cwin->backend));
	
	gtk_widget_show (dialog);
}

/* Handler for the 'Quit' item in the pragha menu */

void quit_action(GtkAction *action, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
}

/* Handler for 'Search Playlist' option in the Edit menu */

void search_playlist_action(GtkAction *action, struct con_win *cwin)
{
	pragha_filter_dialog (cwin);
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
		gtk_widget_hide(GTK_WIDGET(menu_bar));
	}
	else {
		state = gdk_window_get_state (gtk_widget_get_window (cwin->mainwindow));
		if (state & GDK_WINDOW_STATE_FULLSCREEN)
			gtk_window_unfullscreen(GTK_WINDOW(cwin->mainwindow));
		gtk_widget_show(GTK_WIDGET(menu_bar));
	}
}

/* Handler for the 'Show_controls_below_action' item in the view menu */

void
show_controls_below_action (GtkAction *action, struct con_win *cwin)
{
	pragha_preferences_set_controls_below (cwin->preferences,
		gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));

	GtkWidget *toolbar = pragha_toolbar_get_widget(cwin->toolbar);
	GtkWidget *parent = gtk_widget_get_parent(toolbar);

	gint position = pragha_preferences_get_controls_below(cwin->preferences) ? 3 : 1;

	gtk_box_reorder_child(GTK_BOX(parent), toolbar, position);
}

void
jump_to_playing_song_action (GtkAction *action, struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	path = current_playlist_get_actual(cwin->cplaylist);

	jump_to_path_on_current_playlist (cwin->cplaylist, path, TRUE);

	gtk_tree_path_free(path);
}

/* Handler for the 'Equalizer' item in the Tools menu */

void
show_equalizer_action(GtkAction *action, struct con_win *cwin)
{
	pragha_equalizer_dialog_show(cwin);
}


/* Handler for the 'Rescan Library' item in the Tools menu */

void rescan_library_action(GtkAction *action, struct con_win *cwin)
{
	pragha_scanner_scan_library(cwin->scanner);
}

/* Handler for the 'Update Library' item in the Tools menu */

void update_library_action(GtkAction *action, struct con_win *cwin)
{
	pragha_scanner_update_library(cwin->scanner);
}

/* Handler for 'Add All' action in the Tools menu */

void add_libary_action(GtkAction *action, struct con_win *cwin)
{
	GList *list = NULL;
	PraghaMusicobject *mobj;

	/* Query and insert entries */

	set_watch_cursor (cwin->mainwindow);

	const gchar *sql = "SELECT id FROM LOCATION";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cwin->cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		gint location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db (cwin->cdbase, location_id);

		if (!mobj)
			g_warning ("Unable to retrieve details for"
				   " location_id : %d",
				   location_id);
		else {
			list = g_list_prepend (list, mobj);
		}

		pragha_process_gtk_events ();
	}

	pragha_prepared_statement_free (statement);

	remove_watch_cursor (cwin->mainwindow);

	list = g_list_reverse(list);
	pragha_playlist_append_mobj_list(cwin->cplaylist, list);

	g_list_free(list);
}

/* Handler for 'Statistics' action in the Tools menu */

void statistics_action(GtkAction *action, struct con_win *cwin)
{
	gint n_artists, n_albums, n_tracks;
	GtkWidget *dialog;

	n_artists = pragha_database_get_artist_count (cwin->cdbase);
	n_albums = pragha_database_get_album_count (cwin->cdbase);
	n_tracks = pragha_database_get_track_count (cwin->cdbase);

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
				"logo", cwin->pixbuf_app,
				"authors", authors,
				"translator-credits", _("translator-credits"),
				"comments", "A lightweight GTK+ music player",
				"copyright", "(C) 2007-2009 Sujith\n(C) 2009-2013 Matias",
				"license", license,
				"name", PACKAGE_NAME,
				"version", PACKAGE_VERSION,
				NULL);
}

void home_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(uri, cwin->mainwindow);
}

void community_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://bbs.archlinux.org/viewtopic.php?id=46171";
	open_url(uri, cwin->mainwindow);
}

void wiki_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://pragha.wikispaces.com/";
	open_url(uri, cwin->mainwindow);
}

void translate_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *uri = "http://www.transifex.net/projects/p/Pragha/";
	open_url(uri, cwin->mainwindow);
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
	const GBindingFlags binding_flags = G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

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

	GtkAction *action_shuffle = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Shuffle");
	g_object_bind_property (cwin->preferences, "shuffle", action_shuffle, "active", binding_flags);

	GtkAction *action_repeat = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/PlaybackMenu/Repeat");
	g_object_bind_property (cwin->preferences, "repeat", action_repeat, "active", binding_flags);

	GtkAction *action_lateral = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Lateral panel");
	g_object_bind_property (cwin->preferences, "lateral-panel", action_lateral, "active", binding_flags);

	GtkAction *action_status_bar = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Status bar");
	g_object_bind_property (cwin->preferences, "show-status-bar", action_status_bar, "active", binding_flags);

	g_signal_connect (cwin->backend, "notify::state", G_CALLBACK (update_menubar_playback_state_cb), cwin);

	gtk_widget_show_all(gtk_ui_manager_get_widget(main_menu, "/Menubar"));

	g_object_unref (main_actions);

	return main_menu;
}
