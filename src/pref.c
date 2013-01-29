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

const gchar *album_art_pattern_info = N_("Patterns should be of the form:\
<filename>;<filename>;....\nA maximum of six patterns are allowed.\n\
Wildcards are not accepted as of now ( patches welcome :-) ).");

static void
album_art_pattern_helper_response(GtkDialog *dialog,
				gint response,
				struct con_win *cwin)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void album_art_pattern_helper(GtkDialog *parent, struct con_win *cwin)
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
			G_CALLBACK(album_art_pattern_helper_response), cwin);

	gtk_widget_show_all (dialog);
}

/* Handler for the preferences dialog */

static void pref_dialog_cb(GtkDialog *dialog, gint response_id,
			   struct con_win *cwin)
{
	GError *error = NULL;
	gboolean ret, osd, test_change;
	gchar *u_folder = NULL, *audio_sink = NULL, *window_state_sink = NULL, *folder = NULL;
	const gchar *album_art_pattern, *audio_cd_device, *audio_device;
	gboolean show_album_art, instant_search, approximate_search, restore_playlist, add_recursively;
	gint album_art_size;
	GtkTreeIter iter;
	GtkTreeModel *model;

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_OK:
		/* Audio preferences */
		audio_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cwin->preferences_w->audio_sink_combo_w));
		if(audio_sink) {
			pragha_preferences_set_audio_sink(cwin->preferences, audio_sink);
			g_free(audio_sink);
		}

		audio_device = gtk_entry_get_text(GTK_ENTRY(cwin->preferences_w->audio_device_w));
		if(audio_device) {
			pragha_preferences_set_audio_device(cwin->preferences, audio_device);
		}

		audio_cd_device = gtk_entry_get_text(GTK_ENTRY(cwin->preferences_w->audio_cd_device_w));
		if (audio_cd_device) {
			pragha_preferences_set_audio_cd_device(cwin->preferences, audio_cd_device);
		}

		gboolean software_mixer = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->soft_mixer_w));
		pragha_preferences_set_software_mixer(cwin->preferences, software_mixer);
		pragha_backend_set_soft_volume(cwin->backend, software_mixer);

		/* Library Preferences */

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->preferences_w->library_view_w));
		ret = gtk_tree_model_get_iter_first(model, &iter);

		/* Free the list of libraries and rebuild it again */
		if(cwin->cpref->library_scanned == NULL) {
			cwin->cpref->library_scanned = cwin->cpref->library_dir;
			cwin->cpref->library_dir = NULL;
		}
		else {
			free_str_list(cwin->cpref->library_dir);
			cwin->cpref->library_dir = NULL;
		}

		while (ret) {
			gtk_tree_model_get(model, &iter, 0, &u_folder, -1);
			if (u_folder) {
				folder = g_filename_from_utf8(u_folder, -1,
							      NULL, NULL, &error);
				if (!folder) {
					g_warning("Unable to get filename from "
						  "UTF-8 string: %s",
						  u_folder);
					g_error_free(error);
					g_free(u_folder);
					ret = gtk_tree_model_iter_next(model,
								       &iter);
					continue;
				}
				cwin->cpref->library_dir =
					g_slist_append(cwin->cpref->library_dir,
						       folder);
			}
			g_free(u_folder);
			ret = gtk_tree_model_iter_next(model, &iter);
		}

		if (cwin->cpref->cur_library_view == FOLDERS) {
			test_change = cwin->cpref->fuse_folders;
			cwin->cpref->fuse_folders = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cwin->preferences_w->fuse_folders_w));
			if (cwin->cpref->fuse_folders != test_change)
				init_library_view(cwin);
		}
		else {
			test_change = cwin->cpref->sort_by_year;
			cwin->cpref->sort_by_year = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cwin->preferences_w->sort_by_year_w));
			if (cwin->cpref->sort_by_year != test_change)
				init_library_view(cwin);
		}

		/* General preferences */

		window_state_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cwin->preferences_w->window_state_combo_w));

		if (!g_ascii_strcasecmp(window_state_sink, _("Start normal"))){
			cwin->cpref->remember_window_state = FALSE;
			g_free(cwin->cpref->start_mode);
			cwin->cpref->start_mode = g_strdup(NORMAL_STATE);
		}
		else if (!g_ascii_strcasecmp(window_state_sink, _("Start fullscreen"))){
			cwin->cpref->remember_window_state = FALSE;
			g_free(cwin->cpref->start_mode);
			cwin->cpref->start_mode = g_strdup(FULLSCREEN_STATE);
		}
		else if (!g_ascii_strcasecmp(window_state_sink, _("Start in system tray"))){
			cwin->cpref->remember_window_state = FALSE;
			g_free(cwin->cpref->start_mode);
			cwin->cpref->start_mode = g_strdup(ICONIFIED_STATE);
		}
		else 	cwin->cpref->remember_window_state = TRUE;

		g_free(window_state_sink);

		instant_search =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->instant_filter_w));
		pragha_preferences_set_instant_search(cwin->preferences, instant_search);

		approximate_search =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->aproximate_search_w));
		pragha_preferences_set_approximate_search(cwin->preferences, approximate_search);

		restore_playlist =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->restore_playlist_w));
		pragha_preferences_set_restore_playlist(cwin->preferences, restore_playlist);


		cwin->cpref->show_icon_tray =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->show_icon_tray_w));

		cwin->cpref->close_to_tray =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->close_to_tray_w));

		add_recursively =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->add_recursively_w));
		pragha_preferences_set_add_recursively(cwin->preferences, add_recursively);

		show_album_art =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->album_art_w));

		album_art_size =
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
						cwin->preferences_w->album_art_size_w));

		if (show_album_art) {
			album_art_pattern = gtk_entry_get_text(GTK_ENTRY(cwin->preferences_w->album_art_pattern_w));

			if (album_art_pattern) {
				if (!validate_album_art_pattern(album_art_pattern)) {
					album_art_pattern_helper(dialog, cwin);
					return;
				}
				/* Proper pattern, store in preferences */
				g_free(cwin->cpref->album_art_pattern);
				cwin->cpref->album_art_pattern = g_strdup(album_art_pattern);
			}
		}

		pragha_album_art_set_size(cwin->albumart, album_art_size);
		pragha_album_art_set_visible(cwin->albumart, show_album_art);

		/* Notification preferences */

		osd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						   cwin->preferences_w->show_osd_w));
		if (osd) {
			cwin->cpref->show_osd = TRUE;
			if (!notify_is_initted()) {
				if (!notify_init(PACKAGE_NAME))
					cwin->cpref->show_osd = FALSE;
			}
		}
		else
			cwin->cpref->show_osd = FALSE;

#if !NOTIFY_CHECK_VERSION (0, 7, 0)
		cwin->cpref->osd_in_systray =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->osd_in_systray_w));
#endif

		cwin->cpref->albumart_in_osd =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->albumart_in_osd_w));
		cwin->cpref->actions_in_osd =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->preferences_w->actions_in_osd_w));

		/* Services internet preferences */
#ifdef HAVE_LIBCLASTFM
		cwin->cpref->lastfm_support =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->lastfm_w));

		if (cwin->cpref->lastfm_user) {
			g_free(cwin->cpref->lastfm_user);
			cwin->cpref->lastfm_user = NULL;
		}
		if (cwin->cpref->lastfm_pass) {
			g_free(cwin->cpref->lastfm_pass);
			cwin->cpref->lastfm_pass = NULL;
		}

		if (cwin->cpref->lastfm_support) {
			cwin->cpref->lastfm_user =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(
					    cwin->preferences_w->lastfm_uname_w)));
			cwin->cpref->lastfm_pass =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(
					    cwin->preferences_w->lastfm_pass_w)));

			if (cwin->clastfm->session_id != NULL) {
				LASTFM_dinit(cwin->clastfm->session_id);

				cwin->clastfm->session_id = NULL;
				cwin->clastfm->status = LASTFM_STATUS_INVALID;
			}
			just_init_lastfm(cwin);
		}
#endif
#ifdef HAVE_LIBGLYR
		cwin->cpref->get_album_art =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->get_album_art_w));
#endif
		cwin->cpref->use_cddb =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->use_cddb_w));

		cwin->cpref->use_mpris2 =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->preferences_w->use_mpris2_w));
		if(!cwin->cpref->use_mpris2) {
			if(NULL != cwin->cmpris2->dbus_connection)
				mpris_close(cwin->cmpris2);
		} else {
			if(NULL == cwin->cmpris2->dbus_connection)
				mpris_init(cwin);
		}

		save_preferences(cwin);

		break;
	default:
		break;
	}

	g_slice_free(PreferencesWidgets, cwin->preferences_w);

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

/* Handler for adding a new library */
static void
library_add_cb_response(GtkDialog *dialog,
			gint response,
			struct con_win *cwin)
{
	gchar *u_folder, *folder;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GError *error = NULL;

	switch (response) {
	case GTK_RESPONSE_ACCEPT:
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->preferences_w->library_view_w));
		folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void library_add_cb(GtkButton *button, struct con_win *cwin)
{
	GtkWidget *dialog;

	/* Create a folder chooser dialog */

	dialog = gtk_file_chooser_dialog_new(_("Select a folder to add to library"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(library_add_cb_response), cwin);

	gtk_widget_show_all (dialog);
}

/* Handler for removing a library */

static void library_remove_cb(GtkButton *button, struct con_win *cwin)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						cwin->preferences_w->library_view_w));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

/* Toggle displaying last.fm username/password widgets */
#ifdef HAVE_LIBCLASTFM
static void toggle_lastfm(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->preferences_w->lastfm_w));

	gtk_widget_set_sensitive(cwin->preferences_w->lastfm_uname_w, is_active);
	gtk_widget_set_sensitive(cwin->preferences_w->lastfm_pass_w, is_active);

	if(!is_active && cwin->clastfm->session_id) {
		CDEBUG(DBG_INFO, "Shutdown LASTFM");

		LASTFM_dinit(cwin->clastfm->session_id);

		cwin->clastfm->session_id = NULL;
		cwin->clastfm->status = LASTFM_STATUS_INVALID;
	}
	/* Insensitive lastfm menus. */
	update_menubar_lastfm_state (cwin);
}
#endif

/* Toggle hint of playlist */

static void toggle_use_hint (GtkToggleButton *button, struct con_win *cwin)
{
	gboolean use_hint;
	use_hint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	pragha_playlist_set_rules_hint(cwin->cplaylist, use_hint);

	pragha_preferences_set_use_hint(cwin->preferences, use_hint);
}

/* Toggle album art pattern */

static void toggle_album_art(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->preferences_w->album_art_w));

	gtk_widget_set_sensitive(cwin->preferences_w->album_art_pattern_w, is_active);
	gtk_widget_set_sensitive(cwin->preferences_w->album_art_size_w, is_active);

	gtk_widget_set_sensitive(cwin->preferences_w->albumart_in_osd_w, is_active);
}

/* Toggle show status icon. */

static void toggle_show_icon_tray(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->preferences_w->show_icon_tray_w));

	if (!is_active)
		gtk_widget_set_sensitive(cwin->preferences_w->albumart_in_osd_w, FALSE);

	gtk_status_icon_set_visible(cwin->status_icon, is_active);
}

/* Toggle album art pattern */

static void toggle_show_osd(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->preferences_w->show_osd_w));

	#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	gtk_widget_set_sensitive(cwin->preferences_w->osd_in_systray_w, is_active);
	#endif
	gtk_widget_set_sensitive(cwin->preferences_w->albumart_in_osd_w, is_active);
	if (can_support_actions())
		gtk_widget_set_sensitive(cwin->preferences_w->actions_in_osd_w, is_active);
}

static void update_audio_device_alsa(struct con_win *cwin)
{
	gtk_widget_set_sensitive(cwin->preferences_w->audio_device_w, TRUE);
	gtk_widget_set_sensitive(cwin->preferences_w->soft_mixer_w, TRUE);
}

static void update_audio_device_oss4(struct con_win *cwin)
{
	gtk_widget_set_sensitive(cwin->preferences_w->audio_device_w, TRUE);
	gtk_widget_set_sensitive(cwin->preferences_w->soft_mixer_w, TRUE);
}

static void update_audio_device_oss(struct con_win *cwin)
{
	gtk_widget_set_sensitive(cwin->preferences_w->audio_device_w, TRUE);
	gtk_widget_set_sensitive(cwin->preferences_w->soft_mixer_w, TRUE);
}

static void update_audio_device_pulse(struct con_win *cwin)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->preferences_w->soft_mixer_w), FALSE);
	gtk_widget_set_sensitive(cwin->preferences_w->audio_device_w, FALSE);
	gtk_widget_set_sensitive(cwin->preferences_w->soft_mixer_w, FALSE);
	pragha_preferences_set_software_mixer(cwin->preferences, FALSE);
}

static void update_audio_device_default(struct con_win *cwin)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->preferences_w->soft_mixer_w), FALSE);
	gtk_widget_set_sensitive(cwin->preferences_w->audio_device_w, FALSE);
	gtk_widget_set_sensitive(cwin->preferences_w->soft_mixer_w, FALSE);
	pragha_preferences_set_software_mixer(cwin->preferences, FALSE);
}

/* The enumerated audio devices have to be changed here */

static void change_audio_sink(GtkComboBox *combo, gpointer udata)
{
	struct con_win *cwin = udata;
	gchar *audio_sink;

	audio_sink = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(
					      cwin->preferences_w->audio_sink_combo_w));

	if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
		update_audio_device_alsa(cwin);
	else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
		update_audio_device_oss4(cwin);
	else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
		update_audio_device_oss(cwin);
	else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
		update_audio_device_pulse(cwin);
	else
		update_audio_device_default(cwin);

	g_free(audio_sink);
}

static void update_preferences(struct con_win *cwin)
{
	gint cnt = 0, i;
	GSList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GError *error = NULL;
	const gchar *audio_sink = pragha_preferences_get_audio_sink(cwin->preferences);
	const gchar *audio_device = pragha_preferences_get_audio_device(cwin->preferences);
	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device(cwin->preferences);

	/* Audio Options */

	if (audio_sink) {
		if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->preferences_w->audio_sink_combo_w),
						 1);
		else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->preferences_w->audio_sink_combo_w),
						 2);
		else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->preferences_w->audio_sink_combo_w),
						 3);
		else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->preferences_w->audio_sink_combo_w),
						 4);
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->preferences_w->audio_sink_combo_w),
						 0);
	}

	if (audio_sink) {
		if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
			update_audio_device_alsa(cwin);
		else if (!g_ascii_strcasecmp(audio_sink, OSS4_SINK))
			update_audio_device_oss4(cwin);
		else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
			update_audio_device_oss(cwin);
		else if (!g_ascii_strcasecmp(audio_sink, PULSE_SINK))
			update_audio_device_pulse(cwin);
		else
			update_audio_device_default(cwin);
	}

	if (audio_device)
		gtk_entry_set_text(GTK_ENTRY(cwin->preferences_w->audio_device_w),
				   audio_device);

	if (audio_cd_device)
		gtk_entry_set_text(GTK_ENTRY(cwin->preferences_w->audio_cd_device_w),
				   audio_cd_device);

	if (pragha_preferences_get_software_mixer(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->soft_mixer_w),
					     TRUE);

	/* General Options */

	if(cwin->cpref->remember_window_state)
		gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->preferences_w->window_state_combo_w), 0);
	else{
		if(cwin->cpref->start_mode){
			if (!g_ascii_strcasecmp(cwin->cpref->start_mode, NORMAL_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->preferences_w->window_state_combo_w), 1);
			else if(!g_ascii_strcasecmp(cwin->cpref->start_mode, FULLSCREEN_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->preferences_w->window_state_combo_w), 2);
			else if(!g_ascii_strcasecmp(cwin->cpref->start_mode, ICONIFIED_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->preferences_w->window_state_combo_w), 3);
		}
	}

	if (pragha_preferences_get_use_hint(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->use_hint_w),
					     TRUE);

	if (pragha_preferences_get_instant_search(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->instant_filter_w),
					     TRUE);

	if (pragha_preferences_get_approximate_search(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->aproximate_search_w),
					     TRUE);

	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->restore_playlist_w),
					     TRUE);

	if (cwin->cpref->show_icon_tray)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->show_icon_tray_w),
					     TRUE);

	if (cwin->cpref->close_to_tray)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->close_to_tray_w),
					     TRUE);

	if (pragha_preferences_get_add_recursively(cwin->preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->add_recursively_w),
					     TRUE);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
				     cwin->preferences_w->album_art_w),
				     pragha_album_art_get_visible(cwin->albumart));

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cwin->preferences_w->album_art_size_w),
				   (int) pragha_album_art_get_size(cwin->albumart));

	if (cwin->cpref->album_art_pattern)
		gtk_entry_set_text(GTK_ENTRY(cwin->preferences_w->album_art_pattern_w),
				   cwin->cpref->album_art_pattern);

	/* Lbrary Options */

	if (cwin->cpref->library_dir) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->preferences_w->library_view_w));

		cnt = g_slist_length(cwin->cpref->library_dir);
		list = cwin->cpref->library_dir;

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
	}
	if (cwin->cpref->fuse_folders)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->fuse_folders_w),
					     TRUE);
	if (cwin->cpref->sort_by_year)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->sort_by_year_w),
					     TRUE);

	/* Notifications options */

	if (cwin->cpref->show_osd)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->show_osd_w),
					     TRUE);
#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	if (cwin->cpref->osd_in_systray)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->osd_in_systray_w),
					     TRUE);
#endif
	if (cwin->cpref->albumart_in_osd)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->albumart_in_osd_w),
					     TRUE);
	if (!can_support_actions())
		gtk_widget_set_sensitive(cwin->preferences_w->actions_in_osd_w, FALSE);
	else if (cwin->cpref->actions_in_osd)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->actions_in_osd_w),
					     TRUE);

	/* Service Internet Option */
#ifdef HAVE_LIBCLASTFM
	if (cwin->cpref->lastfm_support) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->lastfm_w),
					     TRUE);
		gtk_entry_set_text(GTK_ENTRY(cwin->preferences_w->lastfm_uname_w),
				   cwin->cpref->lastfm_user);
		gtk_entry_set_text(GTK_ENTRY(cwin->preferences_w->lastfm_pass_w),
				   cwin->cpref->lastfm_pass);
	}
#endif
#ifdef HAVE_LIBGLYR
	if(cwin->cpref->get_album_art)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->get_album_art_w),
					     TRUE);
#endif
	if (cwin->cpref->use_cddb)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->use_cddb_w),
					     TRUE);
	if (cwin->cpref->use_mpris2)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->preferences_w->use_mpris2_w),
					     TRUE);
}

void save_preferences(struct con_win *cwin)
{
	gchar **libs, **nodes;
	gchar *u_file = NULL;
	gint cnt = 0, i = 0, *window_size, *window_position;
	gint win_width, win_height, win_x, win_y, sidebar_size;
	GError *error = NULL;
	GSList *list;
	GdkWindowState state;

	/* General options*/

	/* Save version */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_GENERAL,
			      KEY_INSTALLED_VERSION,
			      PACKAGE_VERSION);

	/* Save last folder used in file chooser */

	u_file = g_filename_to_utf8(cwin->cstate->last_folder, -1,
				    NULL, NULL, &error);
	if (!u_file) {
		g_warning("Unable to convert file to UTF-8: %s",
			  cwin->cstate->last_folder);
		g_error_free(error);
		error = NULL;
	} else {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_GENERAL,
				      KEY_LAST_FOLDER,
				      u_file);
		g_free(u_file);
	}

	/* Save album art pattern */

	if (string_is_empty(cwin->cpref->album_art_pattern)) {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_GENERAL) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_GENERAL,
				       KEY_ALBUM_ART_PATTERN,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_ALBUM_ART_PATTERN,
					      &error);
		}
	}
	else {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_GENERAL,
				      KEY_ALBUM_ART_PATTERN,
				      cwin->cpref->album_art_pattern);
	}

	/* Save time remaining mode option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_TIMER_REMAINING_MODE,
			       cwin->cpref->timer_remaining_mode);

	/* Save show status icon option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_ICON_TRAY,
			       cwin->cpref->show_icon_tray);

	/* Save close to tray option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_CLOSE_TO_TRAY,
			       cwin->cpref->close_to_tray);

	/* Save show OSD option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_OSD,
			       cwin->cpref->show_osd);
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_OSD_IN_TRAY,
			       cwin->cpref->osd_in_systray);
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_ALBUM_ART_OSD,
			       cwin->cpref->albumart_in_osd);
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_ACTIONS_OSD,
			       cwin->cpref->actions_in_osd);

	current_playlist_save_preferences(cwin->cplaylist);

	/* Library Options */

	/* Save the list of libraries folders */

	if (cwin->cpref->library_dir) {
		list = cwin->cpref->library_dir;
		cnt = g_slist_length(cwin->cpref->library_dir);
		libs = g_new0(gchar *, cnt);

		for (i = 0; i < cnt; i++) {
			u_file = g_filename_to_utf8(list->data, -1,
						    NULL, NULL, &error);
			if (!u_file) {
				g_warning("Unable to convert file to UTF-8: %s",
					  libs[i]);
				g_error_free(error);
				error = NULL;
				list = list->next;
				continue;
			}
			libs[i] = u_file;
			list = list->next;
		}

		g_key_file_set_string_list(cwin->cpref->configrc_keyfile,
					   GROUP_LIBRARY,
					   KEY_LIBRARY_DIR,
					   (const gchar **)libs,
					   cnt);

		for(i = 0; i < cnt; i++) {
			g_free(libs[i]);
		}
		g_free(libs);
	}
	else {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_LIBRARY) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_LIBRARY,
				       KEY_LIBRARY_DIR,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_DIR,
					      &error);
		}
	}

	/* Save last folders scanned on db */

	if (cwin->cpref->library_scanned) {
		list = cwin->cpref->library_scanned;
		cnt = g_slist_length(cwin->cpref->library_scanned);
		libs = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			u_file = g_filename_to_utf8(list->data, -1,
						    NULL, NULL, &error);
			if (!u_file) {
				g_warning("Unable to convert "
					  "file to UTF-8: %s",
					  libs[i]);
				g_error_free(error);
				error = NULL;
				list = list->next;
				continue;
			}
			libs[i] = u_file;
			list = list->next;
		}

		g_key_file_set_string_list(cwin->cpref->configrc_keyfile,
					   GROUP_LIBRARY,
					   KEY_LIBRARY_SCANNED,
					   (const gchar **)libs,
					   cnt);

		for(i = 0; i < cnt; i++) {
			g_free(libs[i]);
		}
		g_free(libs);
	}
	else {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_LIBRARY) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_LIBRARY,
				       KEY_LIBRARY_SCANNED,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_SCANNED,
					      &error);
		}
	}

	/* Save the library tree nodes */

	if (cwin->cpref->library_tree_nodes) {
		list = cwin->cpref->library_tree_nodes;
		cnt = g_slist_length(cwin->cpref->library_tree_nodes);
		nodes = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			switch (GPOINTER_TO_INT(list->data)) {
				case NODE_TRACK:
					nodes[i] = P_TITLE_STR;
					break;
				case NODE_ARTIST:
					nodes[i] = P_ARTIST_STR;
					break;
				case NODE_ALBUM:
					nodes[i] = P_ALBUM_STR;
					break;
				case NODE_GENRE:
					nodes[i] = P_GENRE_STR;
					break;
				case NODE_FOLDER:
					nodes[i] = P_FOLDER_STR;
					break;
				case NODE_BASENAME:
					nodes[i] = P_BASENAME_STR;
					break;
				case NODE_PLAYLIST:
				case NODE_RADIO:
					g_warning("Save library tree oreder: Bad node type.");
				break;
			}
			list = list->next;
		}

		g_key_file_set_string_list(cwin->cpref->configrc_keyfile,
					   GROUP_LIBRARY,
					   KEY_LIBRARY_TREE_NODES,
					   (const gchar **)nodes,
					   cnt);
		g_free(nodes);
	}

	/* Save the library view order */

	g_key_file_set_integer(cwin->cpref->configrc_keyfile,
			       GROUP_LIBRARY,
			       KEY_LIBRARY_VIEW_ORDER,
			       cwin->cpref->cur_library_view);

	/* Save fuse folders option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_LIBRARY,
			       KEY_FUSE_FOLDERS,
			       cwin->cpref->fuse_folders);

	/* Save sort by year option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_LIBRARY,
			       KEY_SORT_BY_YEAR,
			       cwin->cpref->sort_by_year);

	/* Audio Options */

	/* Save volume */

	if (pragha_preferences_get_software_mixer(cwin->preferences)) {
		g_key_file_set_integer(cwin->cpref->configrc_keyfile,
				       GROUP_AUDIO,
				       KEY_SOFTWARE_VOLUME,
				       pragha_backend_get_volume (cwin->backend));
	}

	/* Window Option */

	/* Save last window state */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_REMEMBER_STATE,
			       cwin->cpref->remember_window_state);

	state = gdk_window_get_state (gtk_widget_get_window (cwin->mainwindow));

	if(cwin->cpref->remember_window_state) {
		if(state & GDK_WINDOW_STATE_FULLSCREEN) {
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_WINDOW,
					      KEY_START_MODE,
					      FULLSCREEN_STATE);
		}
		else if(state & GDK_WINDOW_STATE_WITHDRAWN) {
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_WINDOW,
					      KEY_START_MODE,
					      ICONIFIED_STATE);
		}
		else {
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_WINDOW,
					      KEY_START_MODE,
					      NORMAL_STATE);
		}
	}
	else {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_WINDOW,
				      KEY_START_MODE,
				      cwin->cpref->start_mode);
	}

	/* Save geometry only if window is not maximized or fullscreened */

	if (!(state & GDK_WINDOW_STATE_MAXIMIZED) || !(state & GDK_WINDOW_STATE_FULLSCREEN)) {
		window_size = g_new0(gint, 2);
		gtk_window_get_size(GTK_WINDOW(cwin->mainwindow),
				    &win_width,
				    &win_height);
		window_size[0] = win_width;
		window_size[1] = win_height;

		window_position = g_new0(gint, 2);
		gtk_window_get_position(GTK_WINDOW(cwin->mainwindow),
					&win_x,
					&win_y);
		window_position[0] = win_x;
		window_position[1] = win_y;

		g_key_file_set_integer_list(cwin->cpref->configrc_keyfile,
					    GROUP_WINDOW,
					    KEY_WINDOW_SIZE,
					    window_size,
					    2);

		g_key_file_set_integer_list(cwin->cpref->configrc_keyfile,
					    GROUP_WINDOW,
					    KEY_WINDOW_POSITION,
					    window_position,
					    2);
		g_free(window_size);
		g_free(window_position);
	}

	/* Save sidebar size */

	sidebar_size = gtk_paned_get_position(GTK_PANED(cwin->paned));

	g_key_file_set_integer(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_SIDEBAR_SIZE,
			       sidebar_size);

	/* Save show album art option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_SHOW_ALBUM_ART,
			       pragha_album_art_get_visible(cwin->albumart));

	/* Save album art size */

	g_key_file_set_integer(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_ALBUM_ART_SIZE,
			       (int) pragha_album_art_get_size(cwin->albumart));

	/* Save show controls below option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_CONTROLS_BELOW,
			       cwin->cpref->controls_below);

	/* Save status bar option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_STATUS_BAR,
			       cwin->cpref->status_bar);

	/* Services internet */
	/* Save last.fm option */
#ifdef HAVE_LIBCLASTFM
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_SERVICES,
			       KEY_LASTFM,
			       cwin->cpref->lastfm_support);

	if (!cwin->cpref->lastfm_support) {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_SERVICES) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_SERVICES,
				       KEY_LASTFM_USER,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_USER,
					      &error);
		}
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_SERVICES) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_SERVICES,
				       KEY_LASTFM_PASS,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_PASS,
					      &error);
		}
	}
	else {
		if (cwin->cpref->lastfm_user)
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_USER,
					      cwin->cpref->lastfm_user);
		if (cwin->cpref->lastfm_pass)
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_PASS,
					      cwin->cpref->lastfm_pass);
	}
#endif

	/* Save get album art option */
#ifdef HAVE_LIBGLYR
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_SERVICES,
			       KEY_GET_ALBUM_ART,
			       cwin->cpref->get_album_art);
#endif
	/* Save use CDDB server option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_SERVICES,
			       KEY_USE_CDDB,
			       cwin->cpref->use_cddb);

	/* Save allow MPRIS2 server option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_SERVICES,
			       KEY_ALLOW_MPRIS2,
			       cwin->cpref->use_mpris2);
}

int library_view_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin)
{
	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK)))
		return FALSE;
	if (event->keyval == GDK_KEY_Delete){
		library_remove_cb(NULL, cwin);
		return TRUE;
	}
	return FALSE;
}

static GtkWidget*
pref_create_audio_page(struct con_win *cwin)
{
	GtkWidget *table;
	GtkWidget *audio_device_entry, *audio_device_label, *audio_sink_combo, *sink_label, \
		  *soft_mixer, *audio_cd_device_label,*audio_cd_device_entry;
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
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_device_entry),
				    _("Restart Required"));

	pragha_hig_workarea_table_add_row (table, &row, audio_device_label, audio_device_entry);

	soft_mixer = gtk_check_button_new_with_label(_("Use software mixer"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(soft_mixer), _("Restart Required"));

	pragha_hig_workarea_table_add_wide_control(table, &row, soft_mixer);

	pragha_hig_workarea_table_add_section_title(table, &row, _("Audio CD"));

	audio_cd_device_label = gtk_label_new(_("Audio CD Device"));
	gtk_misc_set_alignment(GTK_MISC (audio_cd_device_label), 0, 0);

	audio_cd_device_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(audio_cd_device_entry),
				 AUDIO_CD_DEVICE_ENTRY_LEN);

	pragha_hig_workarea_table_add_row (table, &row, audio_cd_device_label, audio_cd_device_entry);

	/* Store references */

	cwin->preferences_w->audio_sink_combo_w = audio_sink_combo;
	cwin->preferences_w->audio_device_w = audio_device_entry;
	cwin->preferences_w->audio_cd_device_w = audio_cd_device_entry;
	cwin->preferences_w->soft_mixer_w = soft_mixer;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(audio_sink_combo), "changed",
			 G_CALLBACK(change_audio_sink), cwin);

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

static GtkWidget*
pref_create_library_page(struct con_win *cwin)
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

	hbox_library = gtk_hbox_new(FALSE, 6);

	library_store = gtk_list_store_new(1, G_TYPE_STRING);
	library_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(library_store));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Folders"),
							  renderer,
							  "text",
							  0,
							  NULL);
	gtk_tree_view_column_set_resizable(column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(library_view), column);

	library_view_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_view_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(library_view_scroll),
					GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(library_view_scroll), library_view);

	library_bbox_align = gtk_alignment_new(0, 0, 0, 0);
	library_bbox = gtk_vbutton_box_new();
	library_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
	library_remove = gtk_button_new_from_stock(GTK_STOCK_REMOVE);

	gtk_box_pack_start(GTK_BOX(library_bbox),
			   library_add,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(library_bbox),
			   library_remove,
			   FALSE,
			   FALSE,
			   0);

	gtk_container_add(GTK_CONTAINER(library_bbox_align), library_bbox);

	gtk_box_pack_start(GTK_BOX(hbox_library),
			   library_view_scroll,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_library),
			   library_bbox_align,
			   FALSE,
			   FALSE,
			   0);
	pragha_hig_workarea_table_add_wide_tall_control(table, &row, hbox_library);

	fuse_folders = gtk_check_button_new_with_label(_("Merge folders in the folders estructure view"));
	pragha_hig_workarea_table_add_wide_control(table, &row, fuse_folders);

	sort_by_year = gtk_check_button_new_with_label(_("Sort albums by release year"));
	pragha_hig_workarea_table_add_wide_control(table, &row, sort_by_year);

	/* Store references */

	cwin->preferences_w->library_view_w = library_view;
	cwin->preferences_w->fuse_folders_w = fuse_folders;
	cwin->preferences_w->sort_by_year_w = sort_by_year;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(library_add), "clicked",
			 G_CALLBACK(library_add_cb), cwin);
	g_signal_connect(G_OBJECT(library_remove), "clicked",
			 G_CALLBACK(library_remove_cb), cwin);
	g_signal_connect (G_OBJECT (library_view), "key_press_event",
			  G_CALLBACK(library_view_key_press), cwin);

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

static GtkWidget*
pref_create_appearance_page(struct con_win *cwin)
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
	album_art_size = gtk_spin_button_new_with_range (ALBUM_ART_SIZE, 128, 2);

	pragha_hig_workarea_table_add_row (table, &row, album_art_size_label, album_art_size);

	album_art_pattern_label = gtk_label_new(_("Album art file pattern"));
	album_art_pattern = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(album_art_pattern),
				 ALBUM_ART_PATTERN_LEN);
	gtk_widget_set_tooltip_text(album_art_pattern, album_art_pattern_info);

	pragha_hig_workarea_table_add_row (table, &row, album_art_pattern_label, album_art_pattern);

	/* Store references */

	cwin->preferences_w->use_hint_w = use_hint;
	cwin->preferences_w->album_art_w = album_art;
	cwin->preferences_w->album_art_size_w = album_art_size;
	cwin->preferences_w->album_art_pattern_w = album_art_pattern;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(use_hint), "toggled",
			 G_CALLBACK(toggle_use_hint), cwin);
	g_signal_connect(G_OBJECT(album_art), "toggled",
			 G_CALLBACK(toggle_album_art), cwin);

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

static GtkWidget*
pref_create_general_page(struct con_win *cwin)
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

	cwin->preferences_w->instant_filter_w = instant_filter;
	cwin->preferences_w->aproximate_search_w = aproximate_search;
	cwin->preferences_w->window_state_combo_w = window_state_combo;
	cwin->preferences_w->restore_playlist_w = restore_playlist;
	cwin->preferences_w->add_recursively_w = add_recursively;

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

static GtkWidget*
pref_create_desktop_page(struct con_win *cwin)
{
	GtkWidget *table;
	GtkWidget *show_icon_tray, *close_to_tray;
	GtkWidget *show_osd, *albumart_in_osd, *actions_in_osd;
	#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	GtkWidget *osd_in_systray;
	#endif
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Desktop"));

	show_icon_tray = gtk_check_button_new_with_label(_("Show Pragha icon in the notification area"));
	pragha_hig_workarea_table_add_wide_control(table, &row, show_icon_tray);

	close_to_tray = gtk_check_button_new_with_label(_("Minimize Pragha when close the window"));
	pragha_hig_workarea_table_add_wide_control(table, &row, close_to_tray);

	pragha_hig_workarea_table_add_section_title(table, &row, _("Notifications"));

	show_osd = gtk_check_button_new_with_label(_("Show OSD for track change"));
	pragha_hig_workarea_table_add_wide_control(table, &row, show_osd);

	#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	osd_in_systray = gtk_check_button_new_with_label(_("Associate notifications to system tray"));
	pragha_hig_workarea_table_add_wide_control(table, &row, osd_in_systray);
	#endif

	albumart_in_osd = gtk_check_button_new_with_label(_("Show Album art in notifications"));
	pragha_hig_workarea_table_add_wide_control(table, &row, albumart_in_osd);

	actions_in_osd = gtk_check_button_new_with_label(_("Add actions to change track to notifications"));
	pragha_hig_workarea_table_add_wide_control(table, &row, actions_in_osd);

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(show_icon_tray), "toggled",
			 G_CALLBACK(toggle_show_icon_tray), cwin);
	g_signal_connect(G_OBJECT(show_osd), "toggled",
			 G_CALLBACK(toggle_show_osd), cwin);

	/* Store references. */

	cwin->preferences_w->show_icon_tray_w = show_icon_tray;
	cwin->preferences_w->close_to_tray_w = close_to_tray;
	cwin->preferences_w->show_osd_w = show_osd;
	#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	cwin->preferences_w->osd_in_systray_w = osd_in_systray;
	#endif
	cwin->preferences_w->albumart_in_osd_w = albumart_in_osd;
	cwin->preferences_w->actions_in_osd_w = actions_in_osd;

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

static GtkWidget*
pref_create_services_page(struct con_win *cwin)
{
	GtkWidget *table;
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *lastfm_check, *lastfm_uname, *lastfm_pass, *lastfm_ulabel, *lastfm_plabel;
	#endif
	#ifdef HAVE_LIBGLYR
	GtkWidget *get_album_art;
	#endif
	GtkWidget *use_cddb, *use_mpris2;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	#ifdef HAVE_LIBCLASTFM
	pragha_hig_workarea_table_add_section_title(table, &row, "Last.fm");

	lastfm_check = gtk_check_button_new_with_label(_("Last.fm Support"));
	pragha_hig_workarea_table_add_wide_control(table, &row, lastfm_check);

	lastfm_ulabel = gtk_label_new(_("Username"));
	lastfm_uname = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(lastfm_uname), LASTFM_UNAME_LEN);

	pragha_hig_workarea_table_add_row (table, &row, lastfm_ulabel, lastfm_uname);

	lastfm_plabel = gtk_label_new(_("Password"));
	lastfm_pass = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(lastfm_pass), LASTFM_PASS_LEN);
	gtk_entry_set_visibility(GTK_ENTRY(lastfm_pass), FALSE);
	gtk_entry_set_invisible_char(GTK_ENTRY(lastfm_pass), '*');

	pragha_hig_workarea_table_add_row (table, &row, lastfm_plabel, lastfm_pass);
	#endif

	pragha_hig_workarea_table_add_section_title(table, &row, _("Others services"));

	#ifdef HAVE_LIBGLYR
	get_album_art = gtk_check_button_new_with_label(_("Get album art"));
	pragha_hig_workarea_table_add_wide_control(table, &row, get_album_art);
	#endif
	use_cddb = gtk_check_button_new_with_label(_("Connect to CDDB server"));
	pragha_hig_workarea_table_add_wide_control(table, &row, use_cddb);

	use_mpris2 = gtk_check_button_new_with_label(_("Allow remote control with MPRIS2 interface"));
	pragha_hig_workarea_table_add_wide_control(table, &row, use_mpris2);

	/* Store references. */

	#ifdef HAVE_LIBCLASTFM
	cwin->preferences_w->lastfm_w = lastfm_check;
	cwin->preferences_w->lastfm_uname_w = lastfm_uname;
	cwin->preferences_w->lastfm_pass_w = lastfm_pass;
	g_signal_connect(G_OBJECT(lastfm_check), "toggled",
			 G_CALLBACK(toggle_lastfm), cwin);
	#endif
	#ifdef HAVE_LIBGLYR
	cwin->preferences_w->get_album_art_w = get_album_art;
	#endif
	cwin->preferences_w->use_cddb_w = use_cddb;
	cwin->preferences_w->use_mpris2_w = use_mpris2;

	#ifdef HAVE_LIBCLASTFM
	toggle_lastfm(GTK_TOGGLE_BUTTON(cwin->preferences_w->lastfm_w), cwin);
	#endif

	pragha_hig_workarea_table_finish(table, &row);

	return table;
}

void preferences_dialog(struct con_win *cwin)
{
	GtkWidget *dialog, *header, *pref_notebook;

	GtkWidget *audio_vbox, *appearance_vbox, *library_vbox, *general_vbox, *desktop_vbox, *services_vbox;
	GtkWidget *label_audio, *label_appearance, *label_library, *label_general, *label_desktop, *label_services;

	cwin->preferences_w = g_slice_new0(PreferencesWidgets);

	/* The main preferences dialog */

	dialog = gtk_dialog_new_with_buttons(_("Preferences of Pragha"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	/* Labels */

	label_audio = gtk_label_new(_("Audio"));
	label_appearance = gtk_label_new(_("Appearance"));
	label_library = gtk_label_new(_("Library"));
	label_general = gtk_label_new(_("General"));
	label_desktop = gtk_label_new(_("Desktop"));
	label_services = gtk_label_new(_("Services"));

	/* Boxes */

	appearance_vbox = gtk_vbox_new(FALSE, 2);
	library_vbox = gtk_vbox_new(FALSE, 2);
	general_vbox = gtk_vbox_new(FALSE, 2);

	/* Notebook, pages et al. */

	pref_notebook = gtk_notebook_new();

	gtk_container_set_border_width (GTK_CONTAINER(pref_notebook), 4);

	audio_vbox = pref_create_audio_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), audio_vbox,
				 label_audio);

	appearance_vbox = pref_create_appearance_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), appearance_vbox,
				 label_appearance);

	library_vbox = pref_create_library_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), library_vbox,
				 label_library);

	general_vbox = pref_create_general_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), general_vbox,
				 label_general);

	desktop_vbox = pref_create_desktop_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), desktop_vbox,
				 label_desktop);

	services_vbox = pref_create_services_page(cwin);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), services_vbox,
				 label_services);

	/* Add to dialog */

	header = sokoke_xfce_header_new (_("Preferences of Pragha"), "pragha");

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), pref_notebook, TRUE, TRUE, 0);

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(pref_dialog_cb), cwin);

	update_preferences(cwin);

	toggle_album_art(GTK_TOGGLE_BUTTON(cwin->preferences_w->album_art_w), cwin);

	gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_widget_show_all(dialog);
}

void preferences_free (struct con_pref *cpref)
{
#ifdef HAVE_LIBCLASTFM
	g_free(cpref->lastfm_user);
	g_free(cpref->lastfm_pass);
#endif
#ifdef HAVE_LIBGLYR
	g_free(cpref->cache_folder);
#endif
	g_free(cpref->installed_version);
	g_free(cpref->album_art_pattern);
	g_free(cpref->start_mode);
	free_str_list(cpref->library_dir);
	free_str_list(cpref->library_scanned);
	g_slist_free(cpref->library_tree_nodes);

	g_slice_free(struct con_pref, cpref);
}
