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

#include <glib.h>

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef HAVE_LIBPEAS
#include <libpeas-gtk/peas-gtk.h>
#endif

#include <gdk/gdkkeysyms.h>

#include "pragha-preferences-dialog.h"
#include "pragha-hig.h"
#include "pragha-utils.h"
#include "pragha-simple-widgets.h"
#include "pragha.h"

struct _PreferencesTab {
	GtkWidget *widget;
	GtkWidget *vbox;
	GtkWidget *label;
};
typedef struct _PreferencesTab PreferencesTab;

struct _PreferencesDialog {
	PraghaApplication *pragha;
	PraghaPreferences *preferences;

	GtkWidget         *widget;
	GtkWidget         *notebook;
	PreferencesTab    *audio_tab;
	PreferencesTab    *desktop_tab;
	PreferencesTab    *services_tab;

#ifndef G_OS_WIN32
	GtkWidget *audio_device_w;
	GtkWidget *audio_sink_combo_w;
	GtkWidget *soft_mixer_w;
#endif
	GtkWidget *use_hint_w;
	GtkWidget *album_art_w;
	GtkWidget *album_art_size_w;
	GtkWidget *album_art_pattern_w;

	GtkWidget *library_view_w;
	GtkWidget *fuse_folders_w;
	GtkWidget *sort_by_year_w;

	GtkWidget *instant_filter_w;
	GtkWidget *aproximate_search_w;
	GtkWidget *window_state_combo_w;
	GtkWidget *restore_playlist_w;
	GtkWidget *show_icon_tray_w;
	GtkWidget *close_to_tray_w;
	GtkWidget *add_recursively_w;
};

/*
 * Dinamic Tabs
 */

static PreferencesTab *
pragha_preferences_tab_new (const gchar *label)
{
	PreferencesTab *tab;
	tab = g_slice_new0(PreferencesTab);

	tab->label = gtk_label_new(label);
	tab->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

	return tab;
}

static void
pragha_preferences_tab_append_setting (PreferencesTab *tab, GtkWidget *widget, gboolean expand)
{
	gtk_box_pack_start (GTK_BOX(tab->vbox), widget, expand, expand, 0);
	gtk_widget_show_all (tab->vbox);
}

static void
pragha_preferences_tab_remove_setting (PreferencesTab *tab, GtkWidget *widget)
{
	GList *list = NULL;
	gtk_container_remove (GTK_CONTAINER(tab->vbox), widget);

	list = gtk_container_get_children (GTK_CONTAINER(tab->vbox));

	if (g_list_length(list) == 0)
		gtk_widget_hide (tab->vbox);
	g_list_free(list);
}

static void
pragha_preferences_notebook_append_tab (GtkWidget *notebook, PreferencesTab *tab)
{
	GList *list = NULL;

	gtk_notebook_append_page (GTK_NOTEBOOK(notebook),
	                          tab->vbox, tab->label);

	list = gtk_container_get_children (GTK_CONTAINER(tab->vbox));
	if (g_list_length(list) == 0)
		gtk_widget_hide (tab->vbox);
	else
		gtk_widget_show_all (tab->vbox);
}

void
pragha_preferences_append_audio_setting (PraghaApplication *pragha, GtkWidget *widget, gboolean expand)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_append_setting (dialog->audio_tab, widget, expand);
}

void
pragha_preferences_remove_audio_setting (PraghaApplication *pragha, GtkWidget *widget)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_remove_setting (dialog->audio_tab, widget);
}

void
pragha_preferences_append_desktop_setting (PraghaApplication *pragha, GtkWidget *widget, gboolean expand)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_append_setting (dialog->desktop_tab, widget, expand);
}

void
pragha_preferences_remove_desktop_setting (PraghaApplication *pragha, GtkWidget *widget)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_remove_setting (dialog->desktop_tab, widget);
}

void
pragha_preferences_append_services_setting (PraghaApplication *pragha, GtkWidget *widget, gboolean expand)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_append_setting (dialog->services_tab, widget, expand);
}

void
pragha_preferences_remove_services_setting (PraghaApplication *pragha, GtkWidget *widget)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);
	pragha_preferences_tab_remove_setting (dialog->services_tab, widget);
}


const gchar *album_art_pattern_info = N_("Patterns should be of the form:\
<filename>;<filename>;....\nA maximum of six patterns are allowed.\n\
Wildcards are not accepted as of now ( patches welcome :-) ).");

static void
album_art_pattern_helper_response(GtkDialog *dialog,
				gint response,
				PraghaApplication *pragha)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void album_art_pattern_helper(GtkDialog *parent, PraghaApplication *pragha)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",
					album_art_pattern_info);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Album art pattern"));

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(album_art_pattern_helper_response), pragha);

	gtk_widget_show_all (dialog);
}

static GSList *
pragha_preferences_dialog_get_library_list (PreferencesDialog *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *library_list = NULL;
	gchar *u_folder = NULL, *folder = NULL;
	GError *error = NULL;
	gboolean ret;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(dialog->library_view_w));

	ret = gtk_tree_model_get_iter_first(model, &iter);
	while (ret) {
		gtk_tree_model_get (model, &iter, 0, &u_folder, -1);
		if (u_folder) {
			folder = g_filename_from_utf8 (u_folder, -1, NULL, NULL, &error);
			if (!folder) {
				g_warning ("Unable to get filename from UTF-8 string: %s", u_folder);
				g_error_free(error);
			}
			else {
				library_list = g_slist_append(library_list, folder);
			}
			g_free (u_folder);
		}
		ret = gtk_tree_model_iter_next(model, &iter);
	}
	return library_list;
}

/* Handler for the preferences dialog */

static void
pragha_preferences_dialog_response(GtkDialog *dialog_w, gint response_id, PreferencesDialog *dialog)
{
	PraghaLibraryPane *library;
	gboolean test_change, pref_setted, pref_toggled;
	gchar *window_state_sink = NULL;
	const gchar *album_art_pattern;
#ifndef G_OS_WIN32
	const gchar *audio_device;
	gchar *audio_sink = NULL;
#endif
	gboolean show_album_art, instant_search, approximate_search, restore_playlist, add_recursively;
	gint album_art_size;
	GSList *list, *library_dir = NULL, *folder_scanned = NULL;
	PraghaLibraryStyle style;
	GtkWidget *infobar;

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_OK:
		/* Audio preferences */
		#ifndef G_OS_WIN32
		audio_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(dialog->audio_sink_combo_w));
		if(audio_sink) {
			pragha_preferences_set_audio_sink(dialog->preferences, audio_sink);
			g_free(audio_sink);
		}

		audio_device = gtk_entry_get_text(GTK_ENTRY(dialog->audio_device_w));
		if(audio_device) {
			pragha_preferences_set_audio_device(dialog->preferences, audio_device);
		}

		gboolean software_mixer = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->soft_mixer_w));
		pragha_preferences_set_software_mixer(dialog->preferences, software_mixer);

		pragha_backend_set_soft_volume(pragha_application_get_backend(dialog->pragha), software_mixer);
		#endif

		/* Save new library folders */

		library_dir = pragha_preferences_dialog_get_library_list (dialog);
		if (library_dir) {
			pragha_preferences_set_filename_list (dialog->preferences,
				                                  GROUP_LIBRARY,
				                                  KEY_LIBRARY_DIR,
				                                  library_dir);
		}
		else {
			pragha_preferences_remove_key (dialog->preferences,
			                               GROUP_LIBRARY,
			                               KEY_LIBRARY_DIR);
		}

		/* Get scanded folders and compare. If changed show infobar */

		folder_scanned =
			pragha_preferences_get_filename_list (dialog->preferences,
			                                      GROUP_LIBRARY,
			                                      KEY_LIBRARY_SCANNED);

		if (folder_scanned && library_dir) {
			test_change = FALSE;
			for(list = folder_scanned; list != NULL; list = list->next) {
				if(is_present_str_list(list->data, library_dir))
					continue;
				test_change = TRUE;
				break;
			}
			for(list = library_dir; list != NULL; list = list->next) {
				if(is_present_str_list(list->data, folder_scanned))
					continue;
				test_change = TRUE;
				break;
			}

			if(test_change) {
				infobar = create_info_bar_update_music(dialog->pragha);
				pragha_window_add_widget_to_infobox(dialog->pragha, infobar);
			}
		}

		if (library_dir)
			free_str_list(library_dir);
		if (folder_scanned)
			free_str_list(folder_scanned);

		library = pragha_application_get_library (dialog->pragha);

		style = pragha_preferences_get_library_style (dialog->preferences);

		/* Save fuse folders preference, and reload view if needed */

		pref_setted = pragha_preferences_get_fuse_folders (dialog->preferences);
		pref_toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->fuse_folders_w));

		pragha_preferences_set_fuse_folders (dialog->preferences, pref_toggled);

		if ((style == FOLDERS) && (pref_setted != pref_toggled))
			library_pane_view_reload (library);

		/* Save sort by year preference, and reload view if needed */

		pref_setted = pragha_preferences_get_sort_by_year (dialog->preferences);
		pref_toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(dialog->sort_by_year_w));

		pragha_preferences_set_sort_by_year (dialog->preferences, pref_toggled);

		if ((style != FOLDERS) && (pref_setted != pref_toggled))
			library_pane_view_reload (library);

		/* General preferences */

		window_state_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(dialog->window_state_combo_w));

		if (!g_ascii_strcasecmp(window_state_sink, _("Start normal"))) {
			pragha_preferences_set_remember_state(dialog->preferences, FALSE);
			pragha_preferences_set_start_mode(dialog->preferences, NORMAL_STATE);
		}
		else if (!g_ascii_strcasecmp(window_state_sink, _("Start fullscreen"))){
			pragha_preferences_set_remember_state(dialog->preferences, FALSE);
			pragha_preferences_set_start_mode(dialog->preferences, FULLSCREEN_STATE);
		}
		else if (!g_ascii_strcasecmp(window_state_sink, _("Start in system tray"))){
			pragha_preferences_set_remember_state(dialog->preferences, FALSE);
			pragha_preferences_set_start_mode(dialog->preferences, ICONIFIED_STATE);
		}
		else
			pragha_preferences_set_remember_state(dialog->preferences, TRUE);

		g_free(window_state_sink);

		instant_search =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->instant_filter_w));
		pragha_preferences_set_instant_search(dialog->preferences, instant_search);

		approximate_search =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->aproximate_search_w));
		pragha_preferences_set_approximate_search(dialog->preferences, approximate_search);

		restore_playlist =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->restore_playlist_w));
		pragha_preferences_set_restore_playlist(dialog->preferences, restore_playlist);

		pragha_preferences_set_show_status_icon(dialog->preferences,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->show_icon_tray_w)));

		pragha_preferences_set_hide_instead_close(dialog->preferences,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->close_to_tray_w)));

		add_recursively =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->add_recursively_w));
		pragha_preferences_set_add_recursively(dialog->preferences, add_recursively);

		show_album_art =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->album_art_w));
		pragha_preferences_set_show_album_art(dialog->preferences, show_album_art);

		album_art_size =
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dialog->album_art_size_w));
		pragha_preferences_set_album_art_size(dialog->preferences, album_art_size);

		if (show_album_art) {
			album_art_pattern = gtk_entry_get_text(GTK_ENTRY(dialog->album_art_pattern_w));

			if (string_is_not_empty(album_art_pattern)) {
				if (!validate_album_art_pattern(album_art_pattern)) {
					album_art_pattern_helper(GTK_DIALOG(dialog->widget), dialog->pragha);
					return;
				}
				/* Proper pattern, store in preferences */
				pragha_preferences_set_album_art_pattern (dialog->preferences,
				                                          album_art_pattern);
				
			}
		}
		break;
	default:
		break;
	}

	gtk_widget_hide(GTK_WIDGET(dialog->widget));
}

static gboolean
pragha_preferences_dialog_delete (GtkWidget *widget, GdkEvent *event, PreferencesDialog *dialog)
{
	return TRUE;
}

/* Handler for adding a new library */
static void
library_add_cb_response (GtkDialog *add_dialog, gint response, PreferencesDialog *dialog)
{
	gchar *u_folder, *folder;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GError *error = NULL;

	switch (response) {
	case GTK_RESPONSE_ACCEPT:
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(dialog->library_view_w));
		folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(add_dialog));
		if (!folder)
			break;

		u_folder = g_filename_to_utf8(folder, -1,
					      NULL, NULL, &error);
		if (!u_folder) {
			g_warning("Unable to get UTF-8 from "
				  "filename: %s",
				  folder);
			g_error_free(error);
			g_free(folder);
			break;
		}

		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0,
				   u_folder, -1);

		g_free(u_folder);
		g_free(folder);

		break;
	default:
		break;
	}
	gtk_widget_destroy(GTK_WIDGET(add_dialog));
}

static void
library_add_cb (GtkButton *button, PreferencesDialog *dialog)
{
	GtkWidget *add_dialog;

	/* Create a folder chooser dialog */

	add_dialog = gtk_file_chooser_dialog_new (_("Select a folder to add to library"),
	                                          GTK_WINDOW(dialog->widget),
	                                          GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
	                                          _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                          _("_Open"), GTK_RESPONSE_ACCEPT,
	                                          NULL);

	gtk_window_set_modal(GTK_WINDOW(add_dialog), TRUE);

	g_signal_connect (G_OBJECT(add_dialog), "response",
	                  G_CALLBACK(library_add_cb_response), dialog);

	gtk_widget_show_all (add_dialog);
}

/* Handler for removing a library */

static void library_remove_cb(GtkButton *button, PreferencesDialog *dialog)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dialog->library_view_w));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

/* Toggle hint of playlist */

static void toggle_use_hint (GtkToggleButton *button, PreferencesDialog *dialog)
{
	gboolean use_hint;
	use_hint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	pragha_preferences_set_use_hint(dialog->preferences, use_hint);
}

/* Toggle album art pattern */

static void toggle_album_art(GtkToggleButton *button, PreferencesDialog *dialog)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->album_art_w));

	gtk_widget_set_sensitive(dialog->album_art_pattern_w, is_active);
	gtk_widget_set_sensitive(dialog->album_art_size_w, is_active);
}

/* Toggle show status icon. */

static void toggle_show_icon_tray(GtkToggleButton *button, PreferencesDialog *dialog)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->show_icon_tray_w));

	pragha_preferences_set_show_status_icon (dialog->preferences, is_active);
}

/* Some audios toggles handlers */

#ifndef G_OS_WIN32
static void update_audio_device_alsa(PreferencesDialog *dialog)
{
	gtk_widget_set_sensitive(dialog->audio_device_w, TRUE);
	gtk_widget_set_sensitive(dialog->soft_mixer_w, TRUE);
}

static void update_audio_device_oss4(PreferencesDialog *dialog)
{
	gtk_widget_set_sensitive(dialog->audio_device_w, TRUE);
	gtk_widget_set_sensitive(dialog->soft_mixer_w, TRUE);
}

static void update_audio_device_oss(PreferencesDialog *dialog)
{
	gtk_widget_set_sensitive(dialog->audio_device_w, TRUE);
	gtk_widget_set_sensitive(dialog->soft_mixer_w, TRUE);
}

static void update_audio_device_pulse(PreferencesDialog *dialog)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->soft_mixer_w), FALSE);
	gtk_widget_set_sensitive(dialog->audio_device_w, FALSE);
	gtk_widget_set_sensitive(dialog->soft_mixer_w, FALSE);
	pragha_preferences_set_software_mixer(dialog->preferences, FALSE);
}

static void update_audio_device_default(PreferencesDialog *dialog)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->soft_mixer_w), FALSE);
	gtk_widget_set_sensitive(dialog->audio_device_w, FALSE);
	gtk_widget_set_sensitive(dialog->soft_mixer_w, FALSE);
	pragha_preferences_set_software_mixer(dialog->preferences, FALSE);
}

/* The enumerated audio devices have to be changed here */

static void
change_audio_sink(GtkComboBox *combo, PreferencesDialog *dialog)
{
	gchar *audio_sink;

	audio_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(dialog->audio_sink_combo_w));

	if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
		update_audio_device_alsa(dialog);
	else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
		update_audio_device_oss4(dialog);
	else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
		update_audio_device_oss(dialog);
	else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
		update_audio_device_pulse(dialog);
	else
		update_audio_device_default(dialog);

	g_free(audio_sink);
}
#endif

static void
pragha_preferences_dialog_init_settings(PreferencesDialog *dialog)
{
	gint cnt = 0, i;
	GSList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GError *error = NULL;
	GSList *library_dir = NULL;
#ifndef G_OS_WIN32
	const gchar *audio_sink = pragha_preferences_get_audio_sink(dialog->preferences);
	const gchar *audio_device = pragha_preferences_get_audio_device(dialog->preferences);
#endif
	const gchar *start_mode = pragha_preferences_get_start_mode(dialog->preferences);

	/* Audio Options */
#ifndef G_OS_WIN32
	if (string_is_not_empty(audio_sink)) {
		if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->audio_sink_combo_w), 1);
		else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->audio_sink_combo_w), 2);
		else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->audio_sink_combo_w), 3);
		else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->audio_sink_combo_w), 4);
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->audio_sink_combo_w), 0);
	}

	if (string_is_not_empty(audio_sink)) {
		if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
			update_audio_device_alsa(dialog);
		else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
			update_audio_device_oss4(dialog);
		else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
			update_audio_device_oss(dialog);
		else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
			update_audio_device_pulse(dialog);
		else
			update_audio_device_default(dialog);
	}

	if (string_is_not_empty(audio_device))
		gtk_entry_set_text(GTK_ENTRY(dialog->audio_device_w), audio_device);

	if (pragha_preferences_get_software_mixer(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->soft_mixer_w), TRUE);
#endif

	/* General Options */

	if(pragha_preferences_get_remember_state(dialog->preferences))
		gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->window_state_combo_w), 0);
	else {
		if(string_is_not_empty(start_mode)) {
			if (!g_ascii_strcasecmp(start_mode, NORMAL_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->window_state_combo_w), 1);
			else if(!g_ascii_strcasecmp(start_mode, FULLSCREEN_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->window_state_combo_w), 2);
			else if(!g_ascii_strcasecmp(start_mode, ICONIFIED_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(dialog->window_state_combo_w), 3);
		}
	}

	if (pragha_preferences_get_use_hint(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->use_hint_w), TRUE);

	if (pragha_preferences_get_instant_search(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->instant_filter_w), TRUE);

	if (pragha_preferences_get_approximate_search(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->aproximate_search_w), TRUE);

	if (pragha_preferences_get_restore_playlist(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->restore_playlist_w), TRUE);

	if (pragha_preferences_get_show_status_icon(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->show_icon_tray_w), TRUE);

	if (pragha_preferences_get_hide_instead_close(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->close_to_tray_w), TRUE);

	if (pragha_preferences_get_add_recursively(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->add_recursively_w), TRUE);

	if (pragha_preferences_get_show_album_art(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->album_art_w), TRUE);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(dialog->album_art_size_w),
	                           pragha_preferences_get_album_art_size(dialog->preferences));

	gtk_entry_set_text(GTK_ENTRY(dialog->album_art_pattern_w),
	                   pragha_preferences_get_album_art_pattern(dialog->preferences));

	/* Lbrary Options */

	library_dir =
		pragha_preferences_get_filename_list (dialog->preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR);

	if (library_dir) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(dialog->library_view_w));

		cnt = g_slist_length(library_dir);
		list = library_dir;

		for (i=0; i < cnt; i++) {
			/* Convert to UTF-8 before adding to the model */
			gchar *u_file = g_filename_to_utf8(list->data, -1,
							   NULL, NULL, &error);
			if (!u_file) {
				g_warning("Unable to convert file to UTF-8");
				g_error_free(error);
				error = NULL;
				list = list->next;
				continue;
			}
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model),
					   &iter, 0, u_file, -1);
			list = list->next;
			g_free(u_file);
		}
		free_str_list(library_dir);
	}

	if (pragha_preferences_get_fuse_folders(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->fuse_folders_w), TRUE);
	if (pragha_preferences_get_sort_by_year(dialog->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->sort_by_year_w), TRUE);
}

gint library_view_key_press (GtkWidget *win, GdkEventKey *event, PreferencesDialog *dialog)
{
	if (event->state != 0 &&
	    ((event->state & GDK_CONTROL_MASK) ||
	     (event->state & GDK_MOD1_MASK) ||
	     (event->state & GDK_MOD3_MASK) ||
	     (event->state & GDK_MOD4_MASK) ||
	     (event->state & GDK_MOD5_MASK))) {
		return FALSE;
	}
	if (event->keyval == GDK_KEY_Delete) {
		library_remove_cb(NULL, dialog);
		return TRUE;
	}

	return FALSE;
}

#ifndef G_OS_WIN32
static GtkWidget*
pref_create_audio_page (PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *audio_device_entry, *audio_device_label, *audio_sink_combo, *sink_label, *soft_mixer;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Audio"));

	sink_label = gtk_label_new(_("Audio sink"));

	audio_sink_combo = gtk_combo_box_text_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_sink_combo),
				    _("Restart Required"));

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(audio_sink_combo),
				  DEFAULT_SINK);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(audio_sink_combo),
				  ALSA_SINK);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(audio_sink_combo),
				  OSS4_SINK);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(audio_sink_combo),
				  OSS_SINK);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(audio_sink_combo),
				  PULSE_SINK);

	pragha_hig_workarea_table_add_row (table, &row, sink_label, audio_sink_combo);

	audio_device_label = gtk_label_new(_("Audio Device"));
	gtk_misc_set_alignment(GTK_MISC (audio_device_label), 0, 0);

	audio_device_entry = gtk_entry_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_device_entry), _("Restart Required"));
	gtk_entry_set_activates_default (GTK_ENTRY(audio_device_entry), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, audio_device_label, audio_device_entry);

	soft_mixer = gtk_check_button_new_with_label(_("Use software mixer"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(soft_mixer), _("Restart Required"));

	pragha_hig_workarea_table_add_wide_control(table, &row, soft_mixer);

	/* Store references */

	dialog->audio_sink_combo_w = audio_sink_combo;
	dialog->audio_device_w = audio_device_entry;
	dialog->soft_mixer_w = soft_mixer;

	/* Setup signal handlers */

	g_signal_connect (G_OBJECT(audio_sink_combo), "changed",
	                  G_CALLBACK(change_audio_sink), dialog);

	return table;
}
#endif

static GtkWidget*
pref_create_library_page (PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *library_view, *library_view_scroll, *library_bbox_align, *library_bbox, *library_add, *library_remove, \
		  *hbox_library, *fuse_folders, *sort_by_year;
	GtkListStore *library_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Library"));

	hbox_library = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

	library_store = gtk_list_store_new(1, G_TYPE_STRING);
	library_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(library_store));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes (_("Folders"),
	                                                   renderer,
	                                                   "text",
	                                                   0,
	                                                   NULL);

	gtk_tree_view_column_set_resizable(column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(library_view), column);

	library_view_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(library_view_scroll),
	                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(library_view_scroll),
	                                     GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(library_view_scroll), library_view);

	library_bbox_align = gtk_alignment_new(0, 0, 0, 0);
	library_bbox = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
	library_add = gtk_button_new_with_mnemonic (_("_Add"));
	library_remove = gtk_button_new_with_mnemonic (_("_Remove"));

	gtk_box_pack_start (GTK_BOX(library_bbox), library_add,
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(library_bbox), library_remove,
	                    FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(library_bbox_align), library_bbox);

	gtk_box_pack_start (GTK_BOX(hbox_library), library_view_scroll,
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(hbox_library), library_bbox_align,
	                    FALSE, FALSE, 0);

	pragha_hig_workarea_table_add_wide_tall_control(table, &row, hbox_library);

	fuse_folders = gtk_check_button_new_with_label(_("Merge folders in the folders estructure view"));
	pragha_hig_workarea_table_add_wide_control(table, &row, fuse_folders);

	sort_by_year = gtk_check_button_new_with_label(_("Sort albums by release year"));
	pragha_hig_workarea_table_add_wide_control(table, &row, sort_by_year);

	/* Store references */

	dialog->library_view_w = library_view;
	dialog->fuse_folders_w = fuse_folders;
	dialog->sort_by_year_w = sort_by_year;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(library_add), "clicked",
			 G_CALLBACK(library_add_cb), dialog);
	g_signal_connect(G_OBJECT(library_remove), "clicked",
			 G_CALLBACK(library_remove_cb), dialog);
	g_signal_connect (G_OBJECT (library_view), "key_press_event",
			  G_CALLBACK(library_view_key_press), dialog);

	return table;
}

static GtkWidget*
pref_create_appearance_page(PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *use_hint, *album_art, *album_art_pattern_label, *album_art_size, *album_art_size_label, *album_art_pattern;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Playlist"));

	use_hint = gtk_check_button_new_with_label(_("Highlight rows on current playlist"));
	pragha_hig_workarea_table_add_wide_control(table, &row, use_hint);

	pragha_hig_workarea_table_add_section_title(table, &row, _("Controls"));

	album_art = gtk_check_button_new_with_label(_("Show Album art in Panel"));
	pragha_hig_workarea_table_add_wide_control(table, &row, album_art);

	album_art_size_label = gtk_label_new(_("Size of Album art"));
	album_art_size = gtk_spin_button_new_with_range (DEFAULT_ALBUM_ART_SIZE, 128, 2);

	pragha_hig_workarea_table_add_row (table, &row, album_art_size_label, album_art_size);

	album_art_pattern_label = gtk_label_new(_("Album art file pattern"));

	album_art_pattern = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(album_art_pattern), ALBUM_ART_PATTERN_LEN);
	gtk_entry_set_activates_default (GTK_ENTRY(album_art_pattern), TRUE);

	gtk_widget_set_tooltip_text(album_art_pattern, album_art_pattern_info);

	pragha_hig_workarea_table_add_row (table, &row, album_art_pattern_label, album_art_pattern);

	/* Store references */

	dialog->use_hint_w = use_hint;
	dialog->album_art_w = album_art;
	dialog->album_art_size_w = album_art_size;
	dialog->album_art_pattern_w = album_art_pattern;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(use_hint), "toggled",
			 G_CALLBACK(toggle_use_hint), dialog);
	g_signal_connect(G_OBJECT(album_art), "toggled",
			 G_CALLBACK(toggle_album_art), dialog);

	return table;
}

static GtkWidget*
pref_create_general_page(PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *instant_filter, *aproximate_search, *window_state_combo, *restore_playlist, *add_recursively;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Search"));

	instant_filter = gtk_check_button_new_with_label(_("Refine the search while writing"));
	pragha_hig_workarea_table_add_wide_control(table, &row, instant_filter);

	aproximate_search = gtk_check_button_new_with_label(_("Search approximate words"));
	pragha_hig_workarea_table_add_wide_control(table, &row, aproximate_search);

	pragha_hig_workarea_table_add_section_title(table, &row, _("When starting pragha"));

	window_state_combo = gtk_combo_box_text_new ();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(window_state_combo), _("Remember last window state"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(window_state_combo), _("Start normal"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(window_state_combo), _("Start fullscreen"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(window_state_combo), _("Start in system tray"));
	pragha_hig_workarea_table_add_wide_control(table, &row, window_state_combo);

	restore_playlist = gtk_check_button_new_with_label(_("Restore last playlist"));
	pragha_hig_workarea_table_add_wide_control(table, &row, restore_playlist);

	pragha_hig_workarea_table_add_section_title(table, &row, _("When adding folders"));
	add_recursively = gtk_check_button_new_with_label(_("Add files recursively"));

	pragha_hig_workarea_table_add_wide_control(table, &row, add_recursively);

	/* Store references */

	dialog->instant_filter_w = instant_filter;
	dialog->aproximate_search_w = aproximate_search;
	dialog->window_state_combo_w = window_state_combo;
	dialog->restore_playlist_w = restore_playlist;
	dialog->add_recursively_w = add_recursively;

	return table;
}

static GtkWidget*
pref_create_desktop_page(PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *show_icon_tray, *close_to_tray;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Desktop"));

	show_icon_tray = gtk_check_button_new_with_label(_("Show Pragha icon in the notification area"));
	pragha_hig_workarea_table_add_wide_control(table, &row, show_icon_tray);

	close_to_tray = gtk_check_button_new_with_label(_("Minimize Pragha when close the window"));
	pragha_hig_workarea_table_add_wide_control(table, &row, close_to_tray);

	/* Setup signal handlers */

	g_signal_connect (G_OBJECT(show_icon_tray), "toggled",
	                  G_CALLBACK(toggle_show_icon_tray), dialog);

	/* Store references. */

	dialog->show_icon_tray_w = show_icon_tray;
	dialog->close_to_tray_w = close_to_tray;

	return table;
}

#ifdef HAVE_LIBPEAS
static GtkWidget*
pref_create_plugins_page (PreferencesDialog *dialog)
{
	GtkWidget *table;
	GtkWidget *manager;
	guint row = 0;

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title (table, &row, _("Plugins"));

	manager = peas_gtk_plugin_manager_new (peas_engine_get_default ());

	pragha_hig_workarea_table_add_wide_tall_control (table, &row, manager);

	return table;
}
#endif

void
pragha_preferences_dialog_connect_handler (PraghaApplication *pragha,
                                           GCallback          callback,
                                           gpointer           user_data)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);

	g_signal_connect (G_OBJECT(dialog->widget), "response",
	                  G_CALLBACK(callback), user_data);
}

void
pragha_preferences_dialog_disconnect_handler (PraghaApplication *pragha,
                                              GCallback          callback,
                                              gpointer           user_data)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);

	g_signal_handlers_disconnect_by_func (dialog->widget,
	                                      callback,
	                                      user_data);
}


void
pragha_preferences_dialog_show (PraghaApplication *pragha)
{
	PreferencesDialog *dialog;
	dialog = pragha_application_get_preferences_dialog (pragha);

	gtk_notebook_set_current_page (GTK_NOTEBOOK(dialog->notebook), 0);

	gtk_widget_show (dialog->widget);
}

void
pragha_preferences_dialog_free (PreferencesDialog *dialog)
{
	g_object_unref (dialog->preferences);
	gtk_widget_destroy (GTK_WIDGET(dialog->widget));

	g_slice_free (PreferencesDialog, dialog);
}

PreferencesDialog *
pragha_preferences_dialog_new (PraghaApplication *pragha)
{
	PreferencesDialog *dialog;
	GtkWidget *header, *pref_notebook;

	GtkWidget *audio_vbox, *appearance_vbox, *library_vbox, *general_vbox, *desktop_vbox;
	GtkWidget *label_appearance, *label_library, *label_general;
	#ifdef HAVE_LIBPEAS
	GtkWidget *plugins_vbox;
	GtkWidget *label_plugins;
	#endif

	dialog = g_slice_new0(PreferencesDialog);

	dialog->pragha = pragha;
	dialog->preferences = pragha_preferences_get();

	/* The main preferences dialog */

	dialog->widget = gtk_dialog_new_with_buttons (_("Preferences of Pragha"),
	                                              GTK_WINDOW(pragha_application_get_window(pragha)),
	                                              GTK_DIALOG_MODAL,
	                                              _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                              _("_Ok"), GTK_RESPONSE_OK,
	                                              NULL);

	/* Labels */

	label_appearance = gtk_label_new(_("Appearance"));
	label_library = gtk_label_new(_("Library"));
	label_general = gtk_label_new(_("General"));
	#ifdef HAVE_LIBPEAS
	label_plugins = gtk_label_new(_("Plugins"));
	#endif

	/* Notebook, pages et al. */

	pref_notebook = gtk_notebook_new();

	gtk_container_set_border_width (GTK_CONTAINER(pref_notebook), 4);

	library_vbox = pref_create_library_page(dialog);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), library_vbox, label_library);
	gtk_widget_show_all (library_vbox);

	dialog->audio_tab = pragha_preferences_tab_new (_("Audio"));
	#ifndef G_OS_WIN32
	audio_vbox = pref_create_audio_page(dialog);
	pragha_preferences_tab_append_setting (dialog->audio_tab, audio_vbox, FALSE);
	#endif
	pragha_preferences_notebook_append_tab (pref_notebook, dialog->audio_tab);

	appearance_vbox = pref_create_appearance_page(dialog);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), appearance_vbox, label_appearance);
	gtk_widget_show_all (appearance_vbox);

	general_vbox = pref_create_general_page(dialog);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), general_vbox, label_general);
	gtk_widget_show_all (general_vbox);

	dialog->desktop_tab = pragha_preferences_tab_new (_("Desktop"));
	desktop_vbox = pref_create_desktop_page(dialog);
	pragha_preferences_tab_append_setting (dialog->desktop_tab, desktop_vbox, FALSE);

	dialog->services_tab = pragha_preferences_tab_new (_("Services"));

	pragha_preferences_notebook_append_tab (pref_notebook, dialog->desktop_tab);
	pragha_preferences_notebook_append_tab (pref_notebook, dialog->services_tab);

	#ifdef HAVE_LIBPEAS
	plugins_vbox = pref_create_plugins_page(dialog);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), plugins_vbox, label_plugins);
	gtk_widget_show_all (plugins_vbox);
	#endif

	/* Add to dialog */

	header = sokoke_xfce_header_new (_("Preferences of Pragha"), "pragha");

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->widget))), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->widget))), pref_notebook, TRUE, TRUE, 0);
	gtk_widget_show_all (header);
	gtk_widget_show (pref_notebook);

	/* Setup signal handlers */

	g_signal_connect (G_OBJECT(dialog->widget), "response",
	                  G_CALLBACK(pragha_preferences_dialog_response), dialog);
	g_signal_connect (G_OBJECT(dialog->widget), "delete_event",
	                  G_CALLBACK(pragha_preferences_dialog_delete), dialog);

	pragha_preferences_dialog_init_settings(dialog);

	toggle_album_art(GTK_TOGGLE_BUTTON(dialog->album_art_w), dialog);

	gtk_dialog_set_default_response(GTK_DIALOG (dialog->widget), GTK_RESPONSE_OK);

	dialog->notebook = pref_notebook;

	return dialog;
}
