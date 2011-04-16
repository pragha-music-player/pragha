/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
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

#include "consonance.h"

const gchar *album_art_pattern_info = "Patterns should be of the form:\
<filename>;<filename>;....\nA maximum of six patterns are allowed.\n\
Wildcards are not accepted as of now ( patches welcome :-) ).";

static void album_art_pattern_helper(GtkDialog *parent, struct con_win *cwin)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",
					album_art_pattern_info);
	gtk_window_set_title(GTK_WINDOW(dialog), "Album art pattern");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

/* Handler for the preferences dialog */

static void pref_dialog_cb(GtkDialog *dialog, gint response_id,
			   struct con_win *cwin)
{
	GError *error = NULL;
	gboolean hf, aa, osd, ret;
	gchar *u_folder = NULL, *audio_sink = NULL;
	gchar *audio_alsa_device = NULL, *audio_oss_device = NULL, *folder = NULL;
	const gchar *album_art_pattern, *audio_cd_device;
	GtkTreeIter iter;
	GtkTreeModel *model;

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_OK:
		/* Validate album art pattern, if invalid bail out immediately */

		if (GTK_WIDGET_VISIBLE(cwin->cpref->album_art_pattern_w)) {
			album_art_pattern =
				gtk_entry_get_text(GTK_ENTRY(
					   cwin->cpref->album_art_pattern_w));

			if (album_art_pattern) {
				if (!validate_album_art_pattern(album_art_pattern)) {
					album_art_pattern_helper(dialog, cwin);
					return;
				}

				/* Proper pattern, store in preferences */

				g_free(cwin->cpref->album_art_pattern);
				cwin->cpref->album_art_pattern =
					g_strdup(album_art_pattern);
			}
		}

		/* Hidden files */

		hf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->cpref->hidden_files));
		if (hf)
			cwin->cpref->show_hidden_files = TRUE;
		else
			cwin->cpref->show_hidden_files = FALSE;

		/* Album art */

		aa = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						  cwin->cpref->album_art));
		if (aa)
			cwin->cpref->show_album_art = TRUE;
		else
			cwin->cpref->show_album_art = FALSE;

		album_art_toggle_state(cwin);

		/* OSD */

		osd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						   cwin->cpref->osd));
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

		save_preferences(cwin);

		break;
	default:
		break;
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
	cwin->cpref->hidden_files = NULL;
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

	dialog = gtk_file_chooser_dialog_new("Select a folder to add to library",
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
						 cwin->cpref->album_art));

	if (is_active) {
		gtk_widget_show(cwin->cpref->album_art_pattern_w);
		gtk_widget_show(cwin->cpref->album_art_pattern_label_w);
	} else {
		gtk_widget_hide(cwin->cpref->album_art_pattern_w);
		gtk_widget_hide(cwin->cpref->album_art_pattern_label_w);
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

	/* Update Hidden Files */

	if (cwin->cpref->show_hidden_files)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->hidden_files),
					     TRUE);

	/* Update album art */

	if (cwin->cpref->show_album_art)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->album_art),
					     TRUE);

	/* Update OSD */

	if (cwin->cpref->show_osd)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					     cwin->cpref->osd), TRUE);

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
}

void save_preferences(struct con_win *cwin)
{
	const gchar *col_name;
	gchar *data, **libs, **columns, **nodes, *last_rescan_time;
	gchar *u_file = NULL;
	gsize length;
	gint cnt = 0, i = 0, *col_widths, *window_size;
	gint win_width, win_height;
	GError *error = NULL;
	GSList *list;
	GList *cols, *j;
	GtkTreeViewColumn *col;

	/* Version */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_GENERAL,
			      KEY_INSTALLED_VERSION,
			      PACKAGE_VERSION);

	/* Shuffle and repeat options */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHUFFLE,
			       cwin->cpref->shuffle);
	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_REPEAT,
			       cwin->cpref->repeat);

	/* Library view order */

	g_key_file_set_integer(cwin->cpref->configrc_keyfile,
			       GROUP_LIBRARY,
			       KEY_LIBRARY_VIEW_ORDER,
			       cwin->cpref->cur_library_view);

	/* Hidden Files option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_HIDDEN_FILE,
			       cwin->cpref->show_hidden_files);

	/* Filetree PWD */

	u_file = g_filename_to_utf8(cwin->cstate->file_tree_pwd, -1,
				    NULL, NULL, &error);
	if (!u_file) {
		g_warning("Unable to convert file to UTF-8: %s",
			  cwin->cstate->file_tree_pwd);
		g_error_free(error);
		error = NULL;
	} else {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_GENERAL,
				      KEY_FILETREE_PWD,
				      u_file);
		g_free(u_file);
	}

	/* Album art option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_ALBUM_ART,
			       cwin->cpref->show_album_art);

	/* Album art pattern */

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
	} else if (cwin->cpref->album_art_pattern) {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_GENERAL,
				      KEY_ALBUM_ART_PATTERN,
				      cwin->cpref->album_art_pattern);
	}

	/* OSD option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SHOW_OSD,
			       cwin->cpref->show_osd);

	/* Save playlist option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_GENERAL,
			       KEY_SAVE_PLAYLIST,
			       cwin->cpref->save_playlist);

	/* Audio CD Device */

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
	} else if (cwin->cpref->audio_cd_device) {
		g_key_file_set_string(cwin->cpref->configrc_keyfile,
				      GROUP_AUDIO,
				      KEY_AUDIO_CD_DEVICE,
				      cwin->cpref->audio_cd_device);
	}

	/* last.fm option */

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
	} else {
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

	/* Software mixer option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_AUDIO,
			       KEY_SOFWARE_MIXER,
			       cwin->cpref->software_mixer);

	/* CDDB server option */

	g_key_file_set_boolean(cwin->cpref->configrc_keyfile,
			       GROUP_AUDIO,
			       KEY_USE_CDDB,
			       cwin->cpref->use_cddb);
	/* Window size */

	window_size = g_new0(gint, 2);
	gtk_window_get_size(GTK_WINDOW(cwin->mainwindow),
			    &win_width,
			    &win_height);
	window_size[0] = win_width;
	window_size[1] = win_height;

	g_key_file_set_integer_list(cwin->cpref->configrc_keyfile,
				    GROUP_GENERAL,
				    KEY_WINDOW_SIZE,
				    window_size,
				    2);
	g_free(window_size);

	/* List of libraries */

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

	/* List of columns visible in current playlist */

	if (cwin->cpref->playlist_columns) {
		list = cwin->cpref->playlist_columns;
		cnt = g_slist_length(cwin->cpref->playlist_columns);
		columns = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			columns[i] = (gchar*)list->data;
			list = list->next;
		}

		g_key_file_set_string_list(cwin->cpref->configrc_keyfile,
					   GROUP_GENERAL,
					   KEY_PLAYLIST_COLUMNS,
					   (const gchar **)columns,
					   cnt);
		g_free(columns);
	}

	/* Column widths */

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
					    GROUP_GENERAL,
					    KEY_PLAYLIST_COLUMN_WIDTHS,
					    col_widths,
					    i);
		g_list_free(cols);
		g_free(col_widths);
	}

	/* Library tree nodes */

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

	/* Audio sink */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_SINK,
			      cwin->cpref->audio_sink);

	/* ALSA device */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_ALSA_DEVICE,
			      cwin->cpref->audio_alsa_device);

	/* OSS device */

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_AUDIO_OSS_DEVICE,
			      cwin->cpref->audio_oss_device);

	/* last rescan time */

	last_rescan_time = g_time_val_to_iso8601(&cwin->cpref->last_rescan_time);
	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_LIBRARY,
			      KEY_LIBRARY_LAST_SCANNED,
			      last_rescan_time);
	g_free(last_rescan_time);

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

	/* Save to conrc */

	data = g_key_file_to_data(cwin->cpref->configrc_keyfile, &length, &error);
	if (!g_file_set_contents(cwin->cpref->configrc_file, data, length, &error))
		g_critical("Unable to write preferences file : %s",
			   error->message);

	g_free(data);
}

void preferences_dialog(struct con_win *cwin)
{
	GtkWidget *dialog, *vbox_all, *pref_notebook, *hbox_library;
	GtkWidget *general_vbox, *audio_vbox, *library_vbox, *lastfm_vbox;
	GtkWidget *label_general, *label_library, *label_audio, *label_lastfm;
	GtkWidget *hidden_files, *album_art, *osd, *save_playlist, *use_cddb;
	GtkWidget *lastfm_check, *lastfm_uname, *lastfm_pass, *album_art_pattern;
	GtkWidget *lastfm_uhbox, *lastfm_ulabel, *lastfm_phbox, *lastfm_plabel;
	GtkWidget *soft_mixer, *library_view, *album_art_pattern_label;
	GtkWidget *audio_cd_device_label, *audio_cd_device_entry;
	GtkWidget *library_bbox_align, *library_bbox, *library_add, *library_remove;
	GtkWidget *audio_sink_combo, *sink_label, *hbox_sink;
	GtkWidget *hbox_album_art_pattern, *hbox_audio_cd_device;
	GtkWidget *audio_device_combo, *audio_device_label, *hbox_audio_device;
	GtkListStore *library_store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* The main preferences dialog */

	dialog = xfce_titled_dialog_new_with_buttons ("Preferencias",
							GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (cwin->mainwindow))),
							GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_CANCEL,
							GTK_STOCK_OK,
							GTK_RESPONSE_OK,
							NULL);

	xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), "Confifurar Consonance Music Manager");

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "gtk-preferences");

	vbox_all = gtk_vbox_new(FALSE, 2);

	/* Labels */

	label_general = gtk_label_new("General");
	label_audio = gtk_label_new("Audio");
	label_library = gtk_label_new("Library");
	label_lastfm = gtk_label_new("Last.fm");

	/* Boxes */

	general_vbox = gtk_vbox_new(FALSE, 2);
	audio_vbox = gtk_vbox_new(FALSE, 2);
	library_vbox = gtk_vbox_new(FALSE, 2);
	lastfm_vbox = gtk_vbox_new(FALSE, 2);

	/* Notebook, pages et al. */

	pref_notebook = gtk_notebook_new();

	gtk_container_set_border_width (pref_notebook, 8);

	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), general_vbox,
				 label_general);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), audio_vbox,
				 label_audio);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), library_vbox,
				 label_library);
	gtk_notebook_append_page(GTK_NOTEBOOK(pref_notebook), lastfm_vbox,
				 label_lastfm);

	/* Hidden files */

	hidden_files = gtk_check_button_new_with_label("Show Hidden Files in "
						       "File View");

	/* Album art */

	album_art = gtk_check_button_new_with_label("Show Album art in Panel");

	album_art_pattern = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(album_art_pattern),
				 ALBUM_ART_PATTERN_LEN);
	gtk_widget_set_tooltip_text(album_art_pattern, album_art_pattern_info);
	album_art_pattern_label = gtk_label_new("Album art file pattern");

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

	/* Notification */

	osd = gtk_check_button_new_with_label("Show OSD for track change");

	/* Save current playlist */

	save_playlist = gtk_check_button_new_with_label("Save/Restore "
							"current playlist");

	/* Pack general items */

	gtk_box_pack_start(GTK_BOX(general_vbox),
			   hidden_files,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   osd,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   save_playlist,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   album_art,
			   FALSE,
			   FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(general_vbox),
			   hbox_album_art_pattern,
			   FALSE,
			   FALSE,
			   0);

	/* Software mixer */

	soft_mixer = gtk_check_button_new_with_label("Use software mixer");
	gtk_widget_set_tooltip_text(GTK_WIDGET(soft_mixer), "Restart Required");

	/* CDDB */

	use_cddb = gtk_check_button_new_with_label("Connect to CDDB server");

	/* Audio Sink */

	audio_sink_combo = gtk_combo_box_new_text();
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_sink_combo),
				    "Restart Required");

	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  DEFAULT_SINK);
	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  ALSA_SINK);
	gtk_combo_box_append_text(GTK_COMBO_BOX(audio_sink_combo),
				  OSS_SINK);

	sink_label = gtk_label_new("Audio sink ");
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
	audio_device_label = gtk_label_new("Audio Device ");
	hbox_audio_device = gtk_hbox_new(FALSE, 2);
	gtk_widget_set_tooltip_text(GTK_WIDGET(audio_device_combo),
				    "Restart Required");

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

	audio_cd_device_label = gtk_label_new("Audio CD Device");
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
			   use_cddb,
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

	hbox_library = gtk_hbox_new(FALSE, 2);

	library_store = gtk_list_store_new(1, G_TYPE_STRING);
	library_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(library_store));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Folder",
							  renderer,
							  "text",
							  0,
							  NULL);
	gtk_tree_view_column_set_resizable(column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(library_view), column);

	library_bbox_align = gtk_alignment_new(0, 0.5, 0, 0);
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
			   library_view,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_library),
			   library_bbox_align,
			   FALSE,
			   FALSE,
			   0);
	
	/* Pack all library items */

	gtk_box_pack_start(GTK_BOX(library_vbox),
			   hbox_library,
			   FALSE,
			   FALSE,
			   0);

	/* Last.fm */

	lastfm_check = gtk_check_button_new_with_label("Last.fm Support");
	lastfm_uname = gtk_entry_new();
	lastfm_pass = gtk_entry_new();
	lastfm_ulabel = gtk_label_new("Username");
	lastfm_plabel = gtk_label_new("Password");
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
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox_all);

	/* Store references */

	cwin->cpref->hidden_files = hidden_files;
	cwin->cpref->album_art = album_art;
	cwin->cpref->osd = osd;
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
	cwin->cpref->album_art = album_art;
	cwin->cpref->album_art_pattern_w = album_art_pattern;
	cwin->cpref->album_art_pattern_label_w = album_art_pattern_label;

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
	toggle_album_art(GTK_TOGGLE_BUTTON(cwin->cpref->album_art), cwin);
}
