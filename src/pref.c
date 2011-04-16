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

const gchar *album_art_pattern_info = N_("Patterns should be of the form:\
<filename>;<filename>;....\nA maximum of six patterns are allowed.\n\
Wildcards are not accepted as of now ( patches welcome :-) ).");

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
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/* Handler for the preferences dialog */

static void pref_dialog_cb(GtkDialog *dialog, gint response_id,
			   struct con_win *cwin)
{
	GError *error = NULL;
	gboolean aa, ctt, ar, osd, ret, test_fuse_folders;
	gchar *u_folder = NULL, *audio_sink = NULL, *window_state_sink = NULL;
	gchar *audio_alsa_device = NULL, *audio_oss_device = NULL, *folder = NULL;
	const gchar *album_art_pattern, *audio_cd_device;
	GtkTreeIter iter;
	GtkTreeModel *model;

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_OK:
		/* Windows init start option */

		window_state_sink = gtk_combo_box_get_active_text(GTK_COMBO_BOX(cwin->cpref->window_state_combo));

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

		/* Validate album art pattern, if invalid bail out immediately */

		if (GTK_WIDGET_VISIBLE(cwin->cpref->album_art_pattern_w)){

			album_art_pattern = gtk_entry_get_text(GTK_ENTRY(cwin->cpref->album_art_pattern_w));

			if (album_art_pattern){
				if (!validate_album_art_pattern(album_art_pattern)) {
					album_art_pattern_helper(dialog, cwin);
					return;
				}
				/* Proper pattern, store in preferences */
				g_free(cwin->cpref->album_art_pattern);
				cwin->cpref->album_art_pattern = g_strdup(album_art_pattern);
			}
		}

		/* Album art */

		aa = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->cpref->album_art_w));
		if (aa)
			cwin->cpref->show_album_art = TRUE;
		else
			cwin->cpref->show_album_art = FALSE;

		cwin->cpref->album_art_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(cwin->cpref->album_art_size_w));

		album_art_toggle_state(cwin);

		/* Close_to_tray */

		ctt = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->cpref->close_to_tray_w));
		if (ctt)
			cwin->cpref->close_to_tray = TRUE;
		else
			cwin->cpref->close_to_tray = FALSE;

		/* Add recurcively */

		ar = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->cpref->add_recurcively_w));
		if (ar)
			cwin->cpref->add_recursively_files = TRUE;
		else
			cwin->cpref->add_recursively_files = FALSE;

		/* OSD */

		osd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						   cwin->cpref->osd_w));
		if (osd) {
			cwin->cpref->show_osd = TRUE;
			if (!notify_is_initted()) {
				if (!notify_init(PACKAGE_NAME))
					cwin->cpref->show_osd = FALSE;
			}
		}
		else
			cwin->cpref->show_osd = FALSE;

		/* Save playlist */

		cwin->cpref->save_playlist =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->cpref->save_playlist_w));

		/* Last.fm */

		cwin->cpref->lw.lastfm_support =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->cpref->lw.lastfm_w));

		if (cwin->cpref->lw.lastfm_user) {
			g_free(cwin->cpref->lw.lastfm_user);
			cwin->cpref->lw.lastfm_user = NULL;
		}
		if (cwin->cpref->lw.lastfm_pass) {
			g_free(cwin->cpref->lw.lastfm_pass);
			cwin->cpref->lw.lastfm_pass = NULL;
		}

		if (cwin->cpref->lw.lastfm_support) {
			cwin->cpref->lw.lastfm_user =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(
					    cwin->cpref->lw.lastfm_uname_w)));
			cwin->cpref->lw.lastfm_pass =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(
					    cwin->cpref->lw.lastfm_pass_w)));
		}

		lastfm_init_thread(cwin);

		/* Software mixer */

		cwin->cpref->software_mixer =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->cpref->soft_mixer));

		/* CDDB Server */

		cwin->cpref->use_cddb =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						     cwin->cpref->use_cddb_w));

		/* Audio sink */

		audio_sink =
			gtk_combo_box_get_active_text(GTK_COMBO_BOX(
						      cwin->cpref->audio_sink_combo));

		g_free(cwin->cpref->audio_sink);
		cwin->cpref->audio_sink = g_strdup(audio_sink);
		g_free(audio_sink);

		/* Audio device, store based on the audio sink */

		if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)) {
			audio_alsa_device = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
							  cwin->cpref->audio_device_w));
			g_free(cwin->cpref->audio_alsa_device);
			cwin->cpref->audio_alsa_device = g_strdup(audio_alsa_device);
			g_free(audio_alsa_device);
		} else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK)) {
			audio_oss_device = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
							 cwin->cpref->audio_device_w));
			g_free(cwin->cpref->audio_oss_device);
			cwin->cpref->audio_oss_device = g_strdup(audio_oss_device);
			g_free(audio_oss_device);
		}

		/* Audio CD Device */

		audio_cd_device = gtk_entry_get_text(GTK_ENTRY(
						     cwin->cpref->audio_cd_device_w));
		if (audio_cd_device) {
			g_free(cwin->cpref->audio_cd_device);
			cwin->cpref->audio_cd_device = g_strdup(audio_cd_device);
		}

		/* Library */

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->cpref->library_view));
		ret = gtk_tree_model_get_iter_first(model, &iter);

		/* Free the list of libraries and rebuild it again */

		free_str_list(cwin->cpref->library_dir);
		cwin->cpref->library_dir = NULL;

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

		test_fuse_folders = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cwin->cpref->fuse_folders_w));

		if ((cwin->cpref->fuse_folders != test_fuse_folders) && (cwin->cpref->cur_library_view == FOLDERS)) {
			cwin->cpref->fuse_folders = test_fuse_folders;
			init_library_view(cwin);
		}

		save_preferences(cwin);

		break;
	default:
		break;
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
	cwin->cpref->library_view = NULL;
}

/* Handler for adding a new library */

static void library_add_cb(GtkButton *button, struct con_win *cwin)
{
	GError *error = NULL;
	GtkWidget *dialog;
	gint resp;
	gchar *u_folder, *folder;
	GtkTreeIter iter;
	GtkTreeModel *model;

	/* Create a folder chooser dialog */

	dialog = gtk_file_chooser_dialog_new(_("Select a folder to add to library"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					     NULL);

	/* Show it and get the folder */

	resp = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (resp) {
	case GTK_RESPONSE_ACCEPT:
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->cpref->library_view));
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

		cwin->cpref->lib_delete =
			delete_from_str_list(folder, cwin->cpref->lib_delete);
		cwin->cpref->lib_add =
			g_slist_append(cwin->cpref->lib_add,
				       g_strdup(folder));

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

/* Handler for removing a library */

static void library_remove_cb(GtkButton *button, struct con_win *cwin)
{
	GError *error = NULL;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *folder, *u_folder;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						cwin->cpref->library_view));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, 0, &u_folder, -1);
		if (!u_folder)
			return;

		folder = g_filename_from_utf8(u_folder, -1,
					      NULL, NULL, &error);
		if (!folder) {
			g_warning("Unable to get UTF-8 from "
				  "filename: %s",
				  u_folder);
			g_error_free(error);
			g_free(u_folder);
			return;
		}

		cwin->cpref->lib_delete =
			g_slist_append(cwin->cpref->lib_delete, g_strdup(folder));
		cwin->cpref->lib_add =
			delete_from_str_list(folder, cwin->cpref->lib_add);

		g_free(u_folder);
		g_free(folder);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
}

/* Toggle displaying last.fm username/password widgets */

static void toggle_lastfm(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->cpref->lw.lastfm_w));

	if (is_active) {
		gtk_widget_show(cwin->cpref->lw.lastfm_uname_w);
		gtk_widget_show(cwin->cpref->lw.lastfm_pass_w);
		gtk_widget_show(cwin->cpref->lw.lastfm_ulabel_w);
		gtk_widget_show(cwin->cpref->lw.lastfm_plabel_w);
	} else {
		gtk_widget_hide(cwin->cpref->lw.lastfm_uname_w);
		gtk_widget_hide(cwin->cpref->lw.lastfm_pass_w);
		gtk_widget_hide(cwin->cpref->lw.lastfm_ulabel_w);
		gtk_widget_hide(cwin->cpref->lw.lastfm_plabel_w);
	}
}

/* Toggle album art pattern */

static void toggle_album_art(GtkToggleButton *button, struct con_win *cwin)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 cwin->cpref->album_art_w));

	if (is_active) {
		gtk_widget_show(cwin->cpref->album_art_pattern_w);
		gtk_widget_show(cwin->cpref->album_art_pattern_label_w);
		gtk_widget_show(cwin->cpref->album_art_size_w);
		gtk_widget_show(cwin->cpref->album_art_size_label_w);
	} else {
		gtk_widget_hide(cwin->cpref->album_art_pattern_w);
		gtk_widget_hide(cwin->cpref->album_art_pattern_label_w);
		gtk_widget_hide(cwin->cpref->album_art_size_w);
		gtk_widget_hide(cwin->cpref->album_art_size_label_w);
	}
}

static void update_audio_device_alsa(struct con_win *cwin)
{
	GtkTreeModel *model;
	GSList *devices, *l;
	gint cnt, i;
	gint active = 0;

	devices = alsa_pcm_devices(cwin);
	if (devices == NULL)
		return;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(
					cwin->cpref->audio_device_w));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	cnt = g_slist_length(devices);
	l = devices;

	/* Append 'default' device first */
	gtk_combo_box_append_text(GTK_COMBO_BOX(
				  cwin->cpref->audio_device_w),
				  ALSA_DEFAULT_DEVICE);

	/* Now append the obtained devices */
	for (i = 0; i < cnt; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(
					  cwin->cpref->audio_device_w),
					  l->data);
		l = l->next;
	}

	/* Set the active entry */

	l = devices;
	if (cwin->cpref->audio_alsa_device) {
		for (i = 1; i <= cnt; i++) {
			if (!g_ascii_strcasecmp(cwin->cpref->audio_alsa_device,
						ALSA_DEFAULT_DEVICE))
				goto set_active;

			if (!g_ascii_strcasecmp(cwin->cpref->audio_alsa_device,
						l->data)) {
				active = i;
				goto set_active;
			}
			l = l->next;
		}
		gtk_combo_box_insert_text(GTK_COMBO_BOX(
					  cwin->cpref->audio_device_w),
					  0, cwin->cpref->audio_alsa_device);
	}

set_active:
	gtk_combo_box_set_active(GTK_COMBO_BOX(
				 cwin->cpref->audio_device_w),
				 active);
	gtk_widget_set_sensitive(cwin->cpref->audio_device_w, TRUE);

	free_str_list(devices);
}

static void update_audio_device_oss(struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(
					cwin->cpref->audio_device_w));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	/* Append 'default' device */
	gtk_combo_box_append_text(GTK_COMBO_BOX(
				  cwin->cpref->audio_device_w),
				  OSS_DEFAULT_DEVICE);

	if (cwin->cpref->audio_oss_device) {
		if (g_ascii_strcasecmp(cwin->cpref->audio_oss_device,
				       OSS_DEFAULT_DEVICE)) {
			gtk_combo_box_insert_text(GTK_COMBO_BOX(
						  cwin->cpref->audio_device_w),
						  0, cwin->cpref->audio_oss_device);
		}
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(
				 cwin->cpref->audio_device_w),
				 0);
	gtk_widget_set_sensitive(cwin->cpref->audio_device_w, TRUE);
}

static void update_audio_device_default(struct con_win *cwin)
{
	GtkTreeModel *model;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(
					cwin->cpref->audio_device_w));
	gtk_list_store_clear(GTK_LIST_STORE(model));

	/* Append dummy default device */
	gtk_combo_box_append_text(GTK_COMBO_BOX(
				  cwin->cpref->audio_device_w),
				  "default");
	gtk_combo_box_set_active(GTK_COMBO_BOX(
				 cwin->cpref->audio_device_w),
				 0);

	gtk_widget_set_sensitive(cwin->cpref->audio_device_w, FALSE);
}

/* The enumerated audio devices have to be changed here */

static void change_audio_sink(GtkComboBox *combo, gpointer udata)
{
	struct con_win *cwin = (struct con_win *)udata;
	gchar *audio_sink;

	audio_sink = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
					      cwin->cpref->audio_sink_combo));

	if (!g_ascii_strcasecmp(audio_sink, ALSA_SINK))
		update_audio_device_alsa(cwin);
	else if (!g_ascii_strcasecmp(audio_sink, OSS_SINK))
		update_audio_device_oss(cwin);
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

	if(cwin->cpref->remember_window_state)
		gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->cpref->window_state_combo), 0);
	else{
		if(cwin->cpref->start_mode){
			if (!g_ascii_strcasecmp(cwin->cpref->start_mode, NORMAL_STATE))		
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->cpref->window_state_combo), 1);		
			else if(!g_ascii_strcasecmp(cwin->cpref->start_mode, FULLSCREEN_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->cpref->window_state_combo), 2);
			else if(!g_ascii_strcasecmp(cwin->cpref->start_mode, ICONIFIED_STATE))
				gtk_combo_box_set_active(GTK_COMBO_BOX(cwin->cpref->window_state_combo), 3);
		}
	}

	/* Update album art */

	if (cwin->cpref->show_album_art)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->album_art_w),
					     TRUE);

	/* Update close to tray */

	if (cwin->cpref->close_to_tray)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->close_to_tray_w),
					     TRUE);
	/* Update add recurcively */

	if (cwin->cpref->add_recursively_files)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->add_recurcively_w),
					     TRUE);


	/* Update OSD */

	if (cwin->cpref->show_osd)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->osd_w),
					     TRUE);

	/* Update save playlist */

	if (cwin->cpref->save_playlist)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->save_playlist_w),
					     TRUE);

	/* Update last.fm */

	if (cwin->cpref->lw.lastfm_support) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->lw.lastfm_w),
					     TRUE);
		gtk_entry_set_text(GTK_ENTRY(cwin->cpref->lw.lastfm_uname_w),
				   cwin->cpref->lw.lastfm_user);
		gtk_entry_set_text(GTK_ENTRY(cwin->cpref->lw.lastfm_pass_w),
				   cwin->cpref->lw.lastfm_pass);
	}

	/* Update software mixer */

	if (cwin->cpref->software_mixer)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->soft_mixer),
					     TRUE);
	/* Update CDDB server */

	if (cwin->cpref->use_cddb)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->use_cddb_w),
					     TRUE);
	/* Update audio sink */

	if (cwin->cpref->audio_sink) {
		if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, DEFAULT_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->cpref->audio_sink_combo),
						 0);
		else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->cpref->audio_sink_combo),
						 1);
		else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK))
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						 cwin->cpref->audio_sink_combo),
						 2);
	}

	/* Update audio device, based on audio sink */

	if (cwin->cpref->audio_sink) {
		if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, DEFAULT_SINK))
			update_audio_device_default(cwin);
		else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK))
			update_audio_device_alsa(cwin);
		else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK))
			update_audio_device_oss(cwin);
	}

	/* Update album art pattern */

	if (cwin->cpref->album_art_pattern) {
		gtk_entry_set_text(GTK_ENTRY(cwin->cpref->album_art_pattern_w),
				   cwin->cpref->album_art_pattern);
	}

	if (cwin->cpref->album_art_size) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(cwin->cpref->album_art_size_w),  (int)cwin->cpref->album_art_size);
	}


	/* Update Audio CD device */

	if (cwin->cpref->audio_cd_device)
		gtk_entry_set_text(GTK_ENTRY(cwin->cpref->audio_cd_device_w),
				   cwin->cpref->audio_cd_device);

	/* Append libraries, if any */

	if (cwin->cpref->library_dir) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(
						cwin->cpref->library_view));

		cnt = g_slist_length(cwin->cpref->library_dir);
		list = cwin->cpref->library_dir;

		for (i=0; i < cnt; i++) {
			/* Convert to UTF-8 before adding to the model */
			gchar *u_file = g_filename_to_utf8((gchar*)list->data, -1,
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
					     cwin->cpref->fuse_folders_w),
					     TRUE);
}

void save_preferences(struct con_win *cwin)
{
	const gchar *col_name;
	gchar *data, **libs, **columns, **nodes, *last_rescan_time;
	gchar *u_file = NULL;
	gsize length;
	gint cnt = 0, i = 0, *col_widths, *window_size;
	gint win_width, win_height, sidebar_size;
	GError *error = NULL;
	GSList *list;
	GList *cols, *j;
	GtkTreeViewColumn *col;
	gchar *ref_char = NULL;
	GtkTreePath *path = NULL;
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

	/* Save add recursively in file chooser option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_ADD_RECURSIVELY_FILES,
			       cwin->cpref->add_recursively_files);

	/* Save album art pattern */

	if (!cwin->cpref->album_art_pattern ||
	    (cwin->cpref->album_art_pattern &&
	     !strlen(cwin->cpref->album_art_pattern))) {
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
	else if (cwin->cpref->album_art_pattern) {
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

	/* Save last.fm option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_LASTFM,
			       cwin->cpref->lw.lastfm_support);

	if (!cwin->cpref->lw.lastfm_support) {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_GENERAL) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_GENERAL,
				       KEY_LASTFM_USER,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_LASTFM_USER,
					      &error);
		}
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_GENERAL) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_GENERAL,
				       KEY_LASTFM_PASS,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_LASTFM_PASS,
					      &error);
		}
	}
	else {
		if (cwin->cpref->lw.lastfm_user)
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_LASTFM_USER,
					      cwin->cpref->lw.lastfm_user);
		if (cwin->cpref->lw.lastfm_pass)
			g_key_file_set_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_LASTFM_PASS,
					      cwin->cpref->lw.lastfm_pass);
	}

	/* Playlist options */

	/* Save playlist option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_PLAYLIST,
			       KEY_SAVE_PLAYLIST,
			       cwin->cpref->save_playlist);

	/* Save reference to current play */

	path = current_playlist_get_actual(cwin);

	if(path){
		ref_char = gtk_tree_path_to_string (path);
		gtk_tree_path_free(path);

		g_key_file_set_string(cwin->cpref->configrc_keyfile,
					GROUP_PLAYLIST,
					KEY_CURRENT_REF,
					ref_char);
		g_free (ref_char);
	}
	else {
		if (g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_PLAYLIST,
				       KEY_CURRENT_REF,
				       &error)){
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF,
					      &error);
		}
	}

	/* Shuffle and repeat options */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_PLAYLIST,
			       KEY_SHUFFLE,
			       cwin->cpref->shuffle);
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_PLAYLIST,
			       KEY_REPEAT,
			       cwin->cpref->repeat);

	/* Save list of columns visible in current playlist */

	if (cwin->cpref->playlist_columns) {
		list = cwin->cpref->playlist_columns;
		cnt = g_slist_length(cwin->cpref->playlist_columns);
		columns = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			columns[i] = (gchar*)list->data;
			list = list->next;
		}

		g_key_file_set_string_list(cwin->cpref->configrc_keyfile,
					   GROUP_PLAYLIST,
					   KEY_PLAYLIST_COLUMNS,
					   (const gchar **)columns,
					   cnt);
		g_free(columns);
	}

	/* Save column widths */

	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(cwin->current_playlist));
	cnt = g_list_length(cols);
	if (cols) {
		col_widths = g_new0(gint, cnt);
		for (j=cols, i=0; j != NULL; j = j->next) {
			col = j->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cwin->cpref->playlist_columns))
				col_widths[i++] =
					gtk_tree_view_column_get_width(col);
		}
		g_key_file_set_integer_list(cwin->cpref->configrc_keyfile,
					    GROUP_PLAYLIST,
					    KEY_PLAYLIST_COLUMN_WIDTHS,
					    col_widths,
					    i);
		g_list_free(cols);
		g_free(col_widths);
	}

	/* Library Options */

	/* Save the list of libraries folders */

	if (cwin->cpref->library_dir) {
		list = cwin->cpref->library_dir;
		cnt = g_slist_length(cwin->cpref->library_dir);
		libs = g_new0(gchar *, cnt);

		for (i = 0; i < cnt; i++) {
			u_file = g_filename_to_utf8((gchar *)list->data, -1,
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

	/* List of libraries to be added/deleted from db */

	if (cwin->cpref->lib_delete) {
		list = cwin->cpref->lib_delete;
		cnt = g_slist_length(cwin->cpref->lib_delete);
		libs = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			u_file = g_filename_to_utf8((gchar *)list->data, -1,
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
					   KEY_LIBRARY_DELETE,
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
				       KEY_LIBRARY_DELETE,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_DELETE,
					      &error);
		}
	}

	if (cwin->cpref->lib_add) {
		list = cwin->cpref->lib_add;
		cnt = g_slist_length(cwin->cpref->lib_add);
		libs = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			u_file = g_filename_to_utf8((gchar *)list->data, -1,
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
					   KEY_LIBRARY_ADD,
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
				       KEY_LIBRARY_ADD,
				       &error)) {
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_ADD,
					      &error);
		}
	}

	/* Save the library tree nodes */

	if (cwin->cpref->library_tree_nodes) {
		list = cwin->cpref->library_tree_nodes;
		cnt = g_slist_length(cwin->cpref->library_tree_nodes);
		nodes = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			nodes[i] = (gchar*)list->data;
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

	/* Save last rescan time */

	last_rescan_time = g_time_val_to_iso8601(&cwin->cpref->last_rescan_time);
	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_LIBRARY,
			      KEY_LIBRARY_LAST_SCANNED,
			      last_rescan_time);
	g_free(last_rescan_time);

	/* Save fuse folders option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_LIBRARY,
			       KEY_FUSE_FOLDERS,
			       cwin->cpref->fuse_folders);

	/* Audio Options */

	/* Save Audio sink */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_SINK,
			      cwin->cpref->audio_sink);

	/* Save ALSA device */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_ALSA_DEVICE,
			      cwin->cpref->audio_alsa_device);

	/* Save OSS device */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_OSS_DEVICE,
			      cwin->cpref->audio_oss_device);

	/* Save Software mixer option and volume */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_AUDIO,
			       KEY_SOFTWARE_MIXER,
			       cwin->cpref->software_mixer);

	if(cwin->cpref->software_mixer){
		g_key_file_set_integer(cwin->cpref->configrc_keyfile,
				       GROUP_AUDIO,
				       KEY_SOFTWARE_VOLUME,
				       cwin->cmixer->curr_vol);
	}

	/* Save audio CD Device */

	if (!cwin->cpref->audio_cd_device ||
	    (cwin->cpref->audio_cd_device &&
	     !strlen(cwin->cpref->audio_cd_device))) {
		if (g_key_file_has_group(cwin->cpref->configrc_keyfile,
					 GROUP_AUDIO) &&
		    g_key_file_has_key(cwin->cpref->configrc_keyfile,
				       GROUP_AUDIO,
				       KEY_AUDIO_CD_DEVICE,
				       &error))
			g_key_file_remove_key(cwin->cpref->configrc_keyfile,
					      GROUP_AUDIO,
					      KEY_AUDIO_CD_DEVICE,
					      &error);
	}
	else if (cwin->cpref->audio_cd_device) {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_AUDIO,
				      KEY_AUDIO_CD_DEVICE,
				      cwin->cpref->audio_cd_device);
	}

	/* Save use CDDB server option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_AUDIO,
			       KEY_USE_CDDB,
			       cwin->cpref->use_cddb);

	/* Window Option */

	/* Save last window state */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_REMEMBER_STATE,
			       cwin->cpref->remember_window_state);

	state = gdk_window_get_state (GTK_WIDGET (cwin->mainwindow)->window);

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

		g_key_file_set_integer_list(cwin->cpref->configrc_keyfile,
					    GROUP_WINDOW,
					    KEY_WINDOW_SIZE,
					    window_size,
					    2);
		g_free(window_size);
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
			       cwin->cpref->show_album_art);

	/* Save album art size */

	g_key_file_set_integer(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_ALBUM_ART_SIZE,
			       (int)cwin->cpref->album_art_size);

	/* Save status bar option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_WINDOW,
			       KEY_STATUS_BAR,
			       cwin->cpref->status_bar);

	/* Save to conrc */

	data = g_key_file_to_data(cwin->cpref->configrc_keyfile, &length, &error);
	if (!g_file_set_contents(cwin->cpref->configrc_file, data, length, &error))
		g_critical("Unable to write preferences file : %s",
			   error->message);

	g_free(data);
}

/* Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan */
gpointer sokoke_xfce_header_new(struct con_win *cwin)
{
	GtkWidget* entry = gtk_entry_new();
	gchar* markup;

	GtkWidget* xfce_heading = gtk_event_box_new();

	gtk_widget_modify_bg(xfce_heading,
				GTK_STATE_NORMAL,
				&entry->style->base[GTK_STATE_NORMAL]);
	GtkWidget* hbox = gtk_hbox_new(FALSE, 12);

	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
	GtkWidget* image = gtk_image_new_from_icon_name("gtk-preferences",
							GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	GtkWidget* label = gtk_label_new(NULL);
	gtk_widget_modify_fg(label,
				GTK_STATE_NORMAL,
				&entry->style->text[GTK_STATE_NORMAL]);
        markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", _("Preferences"));
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(xfce_heading), hbox);
	g_free(markup);

	return xfce_heading;
}

void preferences_dialog(struct con_win *cwin)
{
	GtkWidget *dialog, *vbox_all, *pref_notebook, *hbox_library;
	GtkWidget *general_vbox, *audio_vbox, *library_vbox, *lastfm_vbox;
	GtkWidget *label_general, *label_library, *label_audio, *label_lastfm;
	GtkWidget *start_label, *hbox_start;
	GtkWidget *window_state_combo, *close_to_tray, *album_art,*add_recurcively, *osd, *save_playlist, *use_cddb;
	GtkWidget *lastfm_check, *lastfm_uname, *lastfm_pass, *album_art_pattern;
	GtkWidget *lastfm_uhbox, *lastfm_ulabel, *lastfm_phbox, *lastfm_plabel;
	GtkWidget *soft_mixer, *library_view, *library_view_scroll, *album_art_pattern_label;
	GtkWidget *audio_cd_device_label, *audio_cd_device_entry;
	GtkWidget *library_bbox_align, *library_bbox, *library_add, *library_remove, *fuse_folders;
	GtkWidget *audio_sink_combo, *sink_label, *hbox_sink;
	GtkWidget *hbox_album_art_pattern, *hbox_audio_cd_device;
	GtkWidget *audio_device_combo, *audio_device_label, *hbox_audio_device;
	GtkWidget *album_art_size, *album_art_size_label, *hbox_album_art_size, *vbox_album_art;
	GtkWidget *header, *alignment;
	GtkListStore *library_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;


	/* The main preferences dialog */

	dialog = gtk_dialog_new_with_buttons(_("Preferences"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	vbox_all = gtk_vbox_new(FALSE, 2);

	/* Labels */

	label_general = gtk_label_new(_("General"));
	label_audio = gtk_label_new(_("Audio"));
	label_library = gtk_label_new(_("Library"));
	label_lastfm = gtk_label_new(_("Last.fm"));

	/* Boxes */

	general_vbox = gtk_vbox_new(FALSE, 2);
	audio_vbox = gtk_vbox_new(FALSE, 2);
	library_vbox = gtk_vbox_new(FALSE, 2);
	lastfm_vbox = gtk_vbox_new(FALSE, 2);

	/* Notebook, pages et al. */

	pref_notebook = gtk_notebook_new();

	gtk_container_set_border_width (GTK_CONTAINER(pref_notebook), 4);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), alignment,
				 label_general);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 6, 12, 6);
	gtk_container_add(GTK_CONTAINER(alignment), general_vbox);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), alignment,
				 label_audio);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 6, 12, 6);
	gtk_container_add(GTK_CONTAINER(alignment), audio_vbox);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), alignment,
				 label_library);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 6, 12, 6);
	gtk_container_add(GTK_CONTAINER(alignment), library_vbox);

	alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), alignment,
				 label_lastfm);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 6, 12, 6);
	gtk_container_add(GTK_CONTAINER(alignment), lastfm_vbox);

	/* CDDB */

	use_cddb = gtk_check_button_new_with_label(_("Connect to CDDB server"));

	/* Album art */

	album_art = gtk_check_button_new_with_label(_("Show Album art in Panel"));

	album_art_size = gtk_spin_button_new_with_range (ALBUM_ART_SIZE, 128, 2);
	album_art_size_label = gtk_label_new(_("Size of Album art"));

	hbox_album_art_size = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox_album_art_size),
			   album_art_size_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_album_art_size),
			 album_art_size,
			 TRUE,
			 TRUE,
			 0);

	album_art_pattern = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(album_art_pattern),
				 ALBUM_ART_PATTERN_LEN);
	gtk_widget_set_tooltip_text(album_art_pattern, album_art_pattern_info);
	album_art_pattern_label = gtk_label_new(_("Album art file pattern"));

	hbox_album_art_pattern = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox_album_art_pattern),
			   album_art_pattern_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_album_art_pattern),
			 album_art_pattern,
			 TRUE,
			 TRUE,
			 0);

	vbox_album_art = gtk_vbox_new(FALSE, 2);

	gtk_box_pack_start(GTK_BOX(vbox_album_art),
			   hbox_album_art_pattern,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox_album_art),
			   hbox_album_art_size,
			   FALSE,
			   FALSE,
			   0);

	/* Save Window State */

	window_state_combo = gtk_combo_box_new_text ();

	gtk_combo_box_append_text(GTK_COMBO_BOX(window_state_combo), _("Remember last window state"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(window_state_combo), _("Start normal"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(window_state_combo), _("Start fullscreen"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(window_state_combo), _("Start in system tray"));

	start_label = gtk_label_new(_("When Pragha start"));

	hbox_start = gtk_hbox_new(FALSE, 10);

	gtk_box_pack_start(GTK_BOX(hbox_start),
			   start_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_start),
			 window_state_combo,
			 FALSE,
			 FALSE,
			 0);

	/*General Check_buttons*/

	close_to_tray = gtk_check_button_new_with_label(_("Minimize Pragha when close the window"));
	save_playlist = gtk_check_button_new_with_label(_("Restore last playlist"));
	add_recurcively = gtk_check_button_new_with_label(_("Add files recurcively"));
	osd = gtk_check_button_new_with_label(_("Show OSD for track change"));

	/* Pack general items */

	gtk_box_pack_start(GTK_BOX(general_vbox),
			   hbox_start,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   close_to_tray,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   save_playlist,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   add_recurcively,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   osd,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   use_cddb,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   album_art,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   vbox_album_art,
			   FALSE,
			   FALSE,
			   0);

	/* Software mixer */

	soft_mixer = gtk_check_button_new_with_label(_("Use software mixer"));
	gtk_widget_set_tooltip_text(GTK_WIDGET(soft_mixer), _("Restart Required"));

	/* Audio Sink */

	audio_sink_combo = gtk_combo_box_new_text();
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_sink_combo),
				    _("Restart Required"));

	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  DEFAULT_SINK);
	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  ALSA_SINK);
	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  OSS_SINK);

	sink_label = gtk_label_new(_("Audio sink"));
	hbox_sink = gtk_hbox_new(FALSE, 2);

	gtk_box_pack_start(GTK_BOX(hbox_sink),
			   sink_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_sink),
			 audio_sink_combo,
			 FALSE,
			 FALSE,
			 0);

	/* Audio Device */

	audio_device_combo = gtk_combo_box_entry_new_text();
	audio_device_label = gtk_label_new(_("Audio Device"));
	hbox_audio_device = gtk_hbox_new(FALSE, 2);
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_device_combo),
				    _("Restart Required"));

	gtk_box_pack_start(GTK_BOX(hbox_audio_device),
			   audio_device_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_audio_device),
			 audio_device_combo,
			 FALSE,
			 FALSE,
			 0);

	/* Audio CD device */

	audio_cd_device_label = gtk_label_new(_("Audio CD Device"));
	audio_cd_device_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(audio_cd_device_entry),
				 AUDIO_CD_DEVICE_ENTRY_LEN);

	hbox_audio_cd_device = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox_audio_cd_device),
			   audio_cd_device_label,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(hbox_audio_cd_device),
			 audio_cd_device_entry,
			 TRUE,
			 TRUE,
			 0);

	/* Pack audio items */

	gtk_box_pack_start(GTK_BOX(audio_vbox),
			   soft_mixer,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(audio_vbox),
			   hbox_sink,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(audio_vbox),
			   hbox_audio_device,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(audio_vbox),
			   hbox_audio_cd_device,
			   FALSE,
			   FALSE,
			   0);

 	/* Library List */

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

	fuse_folders = gtk_check_button_new_with_label(_("Merge folders in the folders estructure view"));
	
	/* Pack all library items */

	gtk_box_pack_start(GTK_BOX(library_vbox),
			   hbox_library,
			   TRUE,
			   TRUE,
			   2);

	gtk_box_pack_start(GTK_BOX(library_vbox),
			   fuse_folders,
			   FALSE,
			   FALSE,
			   2);

	/* Last.fm */

	lastfm_check = gtk_check_button_new_with_label(_("Last.fm Support"));
	lastfm_uname = gtk_entry_new();
	lastfm_pass = gtk_entry_new();
	lastfm_ulabel = gtk_label_new(_("Username"));
	lastfm_plabel = gtk_label_new(_("Password"));
	lastfm_uhbox = gtk_hbox_new(FALSE, 2);
	lastfm_phbox = gtk_hbox_new(FALSE, 2);

	gtk_entry_set_max_length(GTK_ENTRY(lastfm_uname), LASTFM_UNAME_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(lastfm_pass), LASTFM_PASS_LEN);
	gtk_entry_set_visibility(GTK_ENTRY(lastfm_pass), FALSE);
	gtk_entry_set_invisible_char(GTK_ENTRY(lastfm_pass), '*');

	gtk_box_pack_start(GTK_BOX(lastfm_uhbox),
			   lastfm_ulabel,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(lastfm_uhbox),
			 lastfm_uname,
			 FALSE,
			 FALSE,
			 0);

	gtk_box_pack_start(GTK_BOX(lastfm_phbox),
			   lastfm_plabel,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_end(GTK_BOX(lastfm_phbox),
			 lastfm_pass,
			 FALSE,
			 FALSE,
			 0);

	gtk_box_pack_start(GTK_BOX(lastfm_vbox),
			   lastfm_check,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(lastfm_vbox),
			   lastfm_uhbox,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(lastfm_vbox),
			   lastfm_phbox,
			   FALSE,
			   FALSE,
			   0);

	/* Add to dialog */

	gtk_box_pack_start(GTK_BOX(vbox_all),
			   pref_notebook,
			   FALSE,
			   FALSE,
			   0);

	header = sokoke_xfce_header_new (cwin);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox_all, TRUE, TRUE, 0);

	/* Store references */
	cwin->cpref->window_state_combo = window_state_combo;
	cwin->cpref->close_to_tray_w = close_to_tray;
	cwin->cpref->album_art_w = album_art;
	cwin->cpref->add_recurcively_w = add_recurcively;
	cwin->cpref->osd_w = osd;
	cwin->cpref->save_playlist_w = save_playlist;
	cwin->cpref->lw.lastfm_w = lastfm_check;
	cwin->cpref->lw.lastfm_uname_w = lastfm_uname;
	cwin->cpref->lw.lastfm_pass_w = lastfm_pass;
	cwin->cpref->lw.lastfm_ulabel_w = lastfm_ulabel;
	cwin->cpref->lw.lastfm_plabel_w = lastfm_plabel;
	cwin->cpref->soft_mixer = soft_mixer;
	cwin->cpref->use_cddb_w = use_cddb;
	cwin->cpref->audio_sink_combo = audio_sink_combo;
	cwin->cpref->audio_device_w = audio_device_combo;
	cwin->cpref->audio_cd_device_w = audio_cd_device_entry;
	cwin->cpref->library_view = library_view;
	cwin->cpref->fuse_folders_w = fuse_folders;
	cwin->cpref->album_art_pattern_w = album_art_pattern;
	cwin->cpref->album_art_pattern_label_w = album_art_pattern_label;
	cwin->cpref->album_art_size_w = album_art_size;
	cwin->cpref->album_art_size_label_w = album_art_size_label;

	/* Setup signal handlers */

	g_signal_connect(G_OBJECT(album_art), "toggled",
			 G_CALLBACK(toggle_album_art), cwin);
	g_signal_connect(G_OBJECT(lastfm_check), "toggled",
			 G_CALLBACK(toggle_lastfm), cwin);
	g_signal_connect(G_OBJECT(dialog), "response",
			 G_CALLBACK(pref_dialog_cb), cwin);
	g_signal_connect(G_OBJECT(library_add), "clicked",
			 G_CALLBACK(library_add_cb), cwin);
	g_signal_connect(G_OBJECT(library_remove), "clicked",
			 G_CALLBACK(library_remove_cb), cwin);
	g_signal_connect(G_OBJECT(audio_sink_combo), "changed",
			 G_CALLBACK(change_audio_sink), cwin);

	update_preferences(cwin);

	gtk_widget_show_all(dialog);
	toggle_lastfm(GTK_TOGGLE_BUTTON(cwin->cpref->lw.lastfm_w), cwin);
	toggle_album_art(GTK_TOGGLE_BUTTON(cwin->cpref->album_art_w), cwin);
}
