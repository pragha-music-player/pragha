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

static gchar *audio_backend = NULL;
static gchar *audio_device = NULL;
static gchar *audio_mixer = NULL;
static gchar* logfile = NULL;

GOptionEntry cmd_entries[] = {
	{"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_version, "Version", NULL},
	{"debug", 'e', 0, G_OPTION_ARG_INT,
	 &debug_level, "Enable Debug ( Levels: 1,2,3,4 )", NULL},
	{ "log-file", 'l', 0, G_OPTION_ARG_FILENAME,
	&logfile, "Redirects console warnings to the specified FILENAME", N_("FILENAME")},
	{"play", 'p', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_play, "Play", NULL},
	{"stop", 's', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_stop, "Stop", NULL},
	{"pause", 't', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_pause, "Play/Pause/Resume", NULL},
	{"prev", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_prev, "Prev", NULL},
	{"next", 'n', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_next, "Next", NULL},
	{"shuffle", 'f', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_shuffle, "Shuffle", NULL},
	{"repeat", 'u', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_repeat, "Repeat", NULL},
	{"inc_vol", 'i', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_inc_volume, "Increase volume by 1", NULL},
	{"dec_vol", 'd', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_dec_volume, "Decrease volume by 1", NULL},
	{"show_osd", 'o', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_show_osd, "Show OSD notification", NULL},
	{"toggle_view", 'x', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_toggle_view, "Toggle player visibility", NULL},
	{"current_state", 'c', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_current_state, "Get current player state", NULL},
	{"audio_backend", 'a', 0, G_OPTION_ARG_STRING,
	 &audio_backend, "Audio backend (valid options: alsa/oss)", NULL},
	{"audio_device", 'g', 0, G_OPTION_ARG_STRING,
	 &audio_device, "Audio Device (For ALSA: hw:0,0 etc.., For OSS: /dev/dsp etc..)", NULL},
	{"audio_mixer", 'm', 0, G_OPTION_ARG_STRING,
	 &audio_mixer, "Mixer Element (For ALSA: Master, PCM, etc.., For OSS: /dev/mixer, etc...)", NULL},
	{G_OPTION_REMAINING, 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_CALLBACK,
	 cmd_add_file, "", "[FILE1 [FILE2...]]"},
	{NULL}
};

#if GTK_CHECK_VERSION (3, 0, 0)
static void init_gui_state(struct con_win *cwin)
{
	init_tag_completion(cwin);

	init_library_view(cwin);

	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		init_current_playlist_view(cwin);

	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		mainwindow_add_widget_to_info_box(cwin, info_bar);
	}
}
#else
static gboolean _init_gui_state(gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_process_gtk_events ())
		return TRUE;
	init_tag_completion(cwin);

	if (pragha_process_gtk_events ())
		return TRUE;
	init_library_view(cwin);

	if (pragha_process_gtk_events ())
		return TRUE;
	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		init_current_playlist_view(cwin);

	if (info_bar_import_music_will_be_useful(cwin)) {
		GtkWidget* info_bar = create_info_bar_import_music(cwin);
		mainwindow_add_widget_to_info_box(cwin, info_bar);
	}

	return TRUE;
}
#endif

gint init_options(struct con_win *cwin, int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;
	GOptionGroup *group;

	CDEBUG(DBG_INFO, "Initializing Command line options");

	context = g_option_context_new("- A lightweight music player");
	group = g_option_group_new("General", "General", "General Options", cwin, NULL);
	g_option_group_add_entries(group, cmd_entries);
	g_option_context_set_main_group(context, group);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	#ifdef HAVE_LIBXFCE4UI
	g_option_context_add_group (context, xfce_sm_client_get_option_group (argc, argv));
	#endif

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		gchar *txt;

		g_message("Unable to parse options. Some options need another instance of pragha running.");
		txt = g_option_context_get_help(context, TRUE, NULL);
		g_print("%s", txt);
		g_free(txt);
		g_option_context_free(context);
		g_error_free(error);
		return -1;
	}

	if (logfile)
		g_log_set_default_handler (pragha_log_to_file, (gpointer)logfile);

	g_option_context_free(context);
	return 0;
}

gint init_config(struct con_win *cwin)
{
	GError *error = NULL;
	gint *win_size, *win_position;
	gchar **libs, **nodes, *last_rescan_time;
	gchar *u_file;
	gboolean err = FALSE;
	gsize cnt = 0, i;

	gboolean last_folder_f, recursively_f, album_art_pattern_f, timer_remaining_mode_f, show_icon_tray_f, close_to_tray_f;
	gboolean libs_f, lib_add_f, lib_delete_f, nodes_f, cur_lib_view_f, fuse_folders_f, sort_by_year_f;
	gboolean audio_sink_f, audio_device_f, software_mixer_f;
	gboolean remember_window_state_f, start_mode_f, window_size_f, window_position_f, sidebar_size_f, lateral_panel_f, album_f, controls_below_f, status_bar_f;
	gboolean show_osd_f, osd_in_systray_f, albumart_in_osd_f, actions_in_osd_f;
	gboolean use_cddb_f, use_mpris2_f;
	gboolean all_f;

	CDEBUG(DBG_INFO, "Initializing configuration");

	last_folder_f = recursively_f = album_art_pattern_f = timer_remaining_mode_f = show_icon_tray_f = close_to_tray_f = FALSE;
	libs_f = lib_add_f = lib_delete_f = nodes_f = cur_lib_view_f = fuse_folders_f = sort_by_year_f = FALSE;
	audio_sink_f = audio_device_f = software_mixer_f = FALSE;
	remember_window_state_f = start_mode_f = window_size_f = window_position_f = sidebar_size_f = lateral_panel_f = album_f = controls_below_f = status_bar_f = FALSE;
	show_osd_f = osd_in_systray_f = albumart_in_osd_f = actions_in_osd_f = FALSE;
	use_cddb_f = use_mpris2_f = FALSE;
	#ifdef HAVE_LIBCLASTFM
	gboolean lastfm_f = FALSE;
	#endif
	#ifdef HAVE_LIBGLYR
	gboolean get_album_art_f = FALSE;
	gchar *cache_folder = NULL;
	#endif

	all_f = FALSE;

	/* Share keyfile with PraghaPreferences */

	cwin->cpref->configrc_keyfile = pragha_preferences_share_key_file(cwin->preferences);
	cwin->cpref->configrc_file = pragha_preferences_share_uri_file(cwin->preferences);

	if (cwin->cpref->configrc_keyfile != NULL &&
	    cwin->cpref->configrc_file != NULL) {
		/* Retrieve version */

		cwin->cpref->installed_version =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_INSTALLED_VERSION,
					      &error);
		if (!cwin->cpref->installed_version) {
			cwin->cstate->first_run = TRUE;
			g_error_free(error);
			error = NULL;
		}

		/* Retrieve Window preferences */

		win_size = g_key_file_get_integer_list(cwin->cpref->configrc_keyfile,
						       GROUP_WINDOW,
						       KEY_WINDOW_SIZE,
						       &cnt,
						       &error);
		if (win_size) {
			cwin->cpref->window_width = win_size[0];
			cwin->cpref->window_height = win_size[1];
			g_free(win_size);
		}
		else {
			g_error_free(error);
			error = NULL;
			window_size_f = TRUE;
		}

		win_position = g_key_file_get_integer_list(cwin->cpref->configrc_keyfile,
							   GROUP_WINDOW,
							   KEY_WINDOW_POSITION,
							   &cnt,
							   &error);
		if (win_position) {
			cwin->cpref->window_x = win_position[0];
			cwin->cpref->window_y = win_position[1];
			g_free(win_position);
		}
		else {
			g_error_free(error);
			error = NULL;
			window_position_f = TRUE;
		}

		cwin->cpref->status_bar =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_WINDOW,
					       KEY_STATUS_BAR,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			status_bar_f = TRUE;
		}

		cwin->cpref->controls_below =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_WINDOW,
					       KEY_CONTROLS_BELOW,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			controls_below_f = TRUE;
		}

		cwin->cpref->sidebar_size =
			g_key_file_get_integer(cwin->cpref->configrc_keyfile,
						GROUP_WINDOW,
						KEY_SIDEBAR_SIZE,
						&error);
		if (error) {
			g_error_free(error);
			error = NULL;
			sidebar_size_f = TRUE;
		}

		cwin->cpref->lateral_panel =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					      GROUP_WINDOW,
					      KEY_SIDEBAR,
					      &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			lateral_panel_f = TRUE;
		}

		/* Retrieve Audio preferences */

		cwin->cpref->audio_sink =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_AUDIO,
					      KEY_AUDIO_SINK,
					      &error);
		if (!cwin->cpref->audio_sink) {
			g_error_free(error);
			error = NULL;
			audio_sink_f = TRUE;
		}

		cwin->cpref->audio_device =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_AUDIO,
					      KEY_AUDIO_DEVICE,
					      &error);
		if (!cwin->cpref->audio_device) {
			g_error_free(error);
			error = NULL;
			audio_device_f = TRUE;
		}

		cwin->cpref->audio_cd_device =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_AUDIO,
					      KEY_AUDIO_CD_DEVICE,
					      &error);
		if (!cwin->cpref->audio_cd_device) {
			g_error_free(error);
			error = NULL;
		}

		cwin->cpref->software_mixer =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_AUDIO,
					       KEY_SOFTWARE_MIXER,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			software_mixer_f = TRUE;
		}

		/* Retrieve Collection preferences */

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_DIR,
						  &cnt,
						  &error);
		if (libs) {
			for (i=0; i<cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("Unable to get filename "
						  "from UTF-8 string: %s",
						  libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->library_dir =
					g_slist_append(cwin->cpref->library_dir,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			libs_f = TRUE;
		}

		nodes = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						   GROUP_LIBRARY,
						   KEY_LIBRARY_TREE_NODES,
						   &cnt,
						   &error);
		if (nodes) {
			for (i=0; i<cnt; i++) {
				if (!g_ascii_strcasecmp(P_TITLE_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_TRACK));
				else if (!g_ascii_strcasecmp(P_ARTIST_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_ARTIST));
				else if (!g_ascii_strcasecmp(P_ALBUM_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_ALBUM));
				else if (!g_ascii_strcasecmp(P_GENRE_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_GENRE));
				else if (!g_ascii_strcasecmp(P_ALBUM_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_BASENAME));
				else if (!g_ascii_strcasecmp(P_GENRE_STR, nodes[i]))
					cwin->cpref->library_tree_nodes =
						g_slist_append(cwin->cpref->library_tree_nodes,
							       GINT_TO_POINTER(NODE_FOLDER));
			}
			g_strfreev(nodes);
		}
		else {
			g_error_free(error);
			error = NULL;
			nodes_f = TRUE;
		}

		cwin->cpref->cur_library_view =
			g_key_file_get_integer(cwin->cpref->configrc_keyfile,
					       GROUP_LIBRARY,
					       KEY_LIBRARY_VIEW_ORDER,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			cur_lib_view_f = TRUE;
		}

		last_rescan_time =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_LIBRARY,
					      KEY_LIBRARY_LAST_SCANNED,
					      &error);
		if (!last_rescan_time) {
			g_error_free(error);
			error = NULL;
		} else {
			if (!g_time_val_from_iso8601(last_rescan_time,
						     &cwin->cpref->last_rescan_time))
				g_warning("Unable to convert last rescan time");
			g_free(last_rescan_time);
		}

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_ADD,
						  &cnt,
						  &error);
		if (libs) {
			for (i = 0; i < cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("Unable to get filename "
						  "from UTF-8 string: %s",
						  libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->lib_add =
					g_slist_append(cwin->cpref->lib_add,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			lib_add_f = TRUE;
		}

		libs = g_key_file_get_string_list(cwin->cpref->configrc_keyfile,
						  GROUP_LIBRARY,
						  KEY_LIBRARY_DELETE,
						  &cnt,
						  &error);
		if (libs) {
			for (i = 0; i < cnt; i++) {
				gchar *file = g_filename_from_utf8(libs[i],
						   -1, NULL, NULL, &error);
				if (!file) {
					g_warning("Unable to get filename "
						  "from UTF-8 string: %s",
						  libs[i]);
					g_error_free(error);
					error = NULL;
					continue;
				}
				cwin->cpref->lib_delete =
					g_slist_append(cwin->cpref->lib_delete,
						       g_strdup(file));
				g_free(file);
			}
			g_strfreev(libs);
		}
		else {
			g_error_free(error);
			error = NULL;
			lib_delete_f = TRUE;
		}

		cwin->cpref->fuse_folders =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_LIBRARY,
					       KEY_FUSE_FOLDERS,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			fuse_folders_f = TRUE;
		}

		cwin->cpref->sort_by_year =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_LIBRARY,
					       KEY_SORT_BY_YEAR,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			sort_by_year_f = TRUE;
		}

		/* Retrieve General preferences */

		cwin->cpref->remember_window_state =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_WINDOW,
					       KEY_REMEMBER_STATE,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			remember_window_state_f = TRUE;
		}

		cwin->cpref->start_mode =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_WINDOW,
					      KEY_START_MODE,
					      &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			start_mode_f = TRUE;
		}

		cwin->cpref->show_icon_tray =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_ICON_TRAY,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			show_icon_tray_f = TRUE;
		}

		cwin->cpref->close_to_tray =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_CLOSE_TO_TRAY,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			close_to_tray_f = TRUE;
		}

		cwin->cpref->add_recursively_files =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_ADD_RECURSIVELY_FILES,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			recursively_f = TRUE;
		}

		cwin->cpref->album_art_pattern =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_GENERAL,
					      KEY_ALBUM_ART_PATTERN,
					      &error);
		if (!cwin->cpref->album_art_pattern) {
			g_error_free(error);
			error = NULL;
			album_art_pattern_f = TRUE;
		}

		cwin->cpref->timer_remaining_mode =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_TIMER_REMAINING_MODE,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			timer_remaining_mode_f = TRUE;
		}

		u_file = g_key_file_get_string(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_LAST_FOLDER,
					       &error);
		if (!u_file) {
			g_error_free(error);
			error = NULL;
			last_folder_f = TRUE;
		} else {
			cwin->cstate->last_folder = g_filename_from_utf8(u_file,
							   -1, NULL, NULL, &error);
			if (!cwin->cstate->last_folder) {
				g_warning("Unable to get filename "
					  "from UTF-8 string: %s",
					  u_file);
				g_error_free(error);
				error = NULL;
				last_folder_f = TRUE;
			}
			g_free(u_file);
		}

		/* Retrieve Notification preferences */

		cwin->cpref->show_osd =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_OSD,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			show_osd_f = TRUE;
		}

		cwin->cpref->osd_in_systray =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_OSD_IN_TRAY,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			osd_in_systray_f = TRUE;
		}

		cwin->cpref->albumart_in_osd =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_ALBUM_ART_OSD,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			albumart_in_osd_f = TRUE;
		}
		cwin->cpref->actions_in_osd =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_GENERAL,
					       KEY_SHOW_ACTIONS_OSD,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			actions_in_osd_f = TRUE;
		}

		/* Retrieve Services Internet preferences */
		#ifdef HAVE_LIBCLASTFM
		cwin->cpref->lastfm_support =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_SERVICES,
					       KEY_LASTFM,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			lastfm_f = TRUE;
		}

		cwin->cpref->lastfm_user =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_USER,
					      &error);
		if (error) {
			g_error_free(error);
			error = NULL;
		}

		cwin->cpref->lastfm_pass =
			g_key_file_get_string(cwin->cpref->configrc_keyfile,
					      GROUP_SERVICES,
					      KEY_LASTFM_PASS,
					      &error);
		if (error) {
			g_error_free(error);
			error = NULL;
		}
		#endif
		#ifdef HAVE_LIBGLYR
		cwin->cpref->get_album_art =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_SERVICES,
					       KEY_GET_ALBUM_ART,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			get_album_art_f = TRUE;
		}
		#endif
		cwin->cpref->use_cddb =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_SERVICES,
					       KEY_USE_CDDB,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			use_cddb_f = TRUE;
		}
		cwin->cpref->use_mpris2 =
			g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
					       GROUP_SERVICES,
					       KEY_ALLOW_MPRIS2,
					       &error);
		if (error) {
			g_error_free(error);
			error = NULL;
			use_mpris2_f = TRUE;
		}
	}
	else {
		err = TRUE;
	}

	/* Get cache of downloaded albums arts */
	#ifdef HAVE_LIBGLYR
	cache_folder = g_build_path(G_DIR_SEPARATOR_S, g_get_user_cache_dir(), "/pragha", NULL);

	if (g_file_test(cache_folder, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) == FALSE)
		g_mkdir(cache_folder, S_IRWXU);
	cwin->cpref->cache_folder = cache_folder;
	#endif

	/* Fill up with failsafe defaults */

	if (all_f || window_size_f) {
		cwin->cpref->window_width = MIN_WINDOW_WIDTH;
		cwin->cpref->window_height = MIN_WINDOW_HEIGHT;
	}
	if (all_f || window_position_f) {
		cwin->cpref->window_x = -1;
		cwin->cpref->window_y = -1;
	}
	if (all_f || sidebar_size_f)
		cwin->cpref->sidebar_size = DEFAULT_SIDEBAR_SIZE;
	if (all_f || lateral_panel_f)
		cwin->cpref->lateral_panel = TRUE;
	if (all_f || libs_f)
		cwin->cpref->library_dir = NULL;
	if (all_f || lib_add_f)
		cwin->cpref->lib_add = NULL;
	if (all_f || lib_delete_f)
		cwin->cpref->lib_delete = NULL;
	if (all_f || album_art_pattern_f)
		cwin->cpref->album_art_pattern = NULL;
	if (all_f || nodes_f) {
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       GINT_TO_POINTER(NODE_ARTIST));
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       GINT_TO_POINTER(NODE_ALBUM));
		cwin->cpref->library_tree_nodes =
			g_slist_append(cwin->cpref->library_tree_nodes,
				       GINT_TO_POINTER(NODE_TRACK));
	}
	if (all_f || fuse_folders_f)
		cwin->cpref->fuse_folders = FALSE;
	if (all_f || sort_by_year_f)
		cwin->cpref->sort_by_year = FALSE;
	if (all_f || cur_lib_view_f)
		cwin->cpref->cur_library_view = FOLDERS;
	if (all_f || recursively_f)
		cwin->cpref->add_recursively_files = FALSE;
	if (all_f || last_folder_f)
		cwin->cstate->last_folder = g_strdup (g_get_home_dir());
	if (all_f || timer_remaining_mode_f)
		cwin->cpref->timer_remaining_mode = FALSE;
	if (all_f || show_osd_f)
		cwin->cpref->show_osd = TRUE;
	if (all_f || osd_in_systray_f)
		cwin->cpref->osd_in_systray = TRUE;
	if (all_f || albumart_in_osd_f)
		cwin->cpref->albumart_in_osd = TRUE;
	if (all_f || actions_in_osd_f)
		cwin->cpref->actions_in_osd = FALSE;
	if (all_f || remember_window_state_f)
		cwin->cpref->remember_window_state = TRUE;
	if (all_f || start_mode_f)
		cwin->cpref->start_mode = g_strdup(NORMAL_STATE);
	if (all_f || show_icon_tray_f)
		cwin->cpref->show_icon_tray = TRUE;
	if (all_f || close_to_tray_f)
		cwin->cpref->close_to_tray = TRUE;
	if (all_f || status_bar_f)
		cwin->cpref->status_bar = TRUE;
	if (all_f || controls_below_f)
		cwin->cpref->controls_below = FALSE;
	if (all_f || software_mixer_f)
		cwin->cpref->software_mixer = TRUE;
	if (all_f || audio_sink_f)
		cwin->cpref->audio_sink = g_strdup(DEFAULT_SINK);
	if (all_f || audio_device_f)
		cwin->cpref->audio_device = g_strdup(ALSA_DEFAULT_DEVICE);
	#ifdef HAVE_LIBCLASTFM
	if (all_f || lastfm_f)
		cwin->cpref->lastfm_support = FALSE;
	#endif
	#ifdef HAVE_LIBGLYR
	if (all_f || get_album_art_f)
		cwin->cpref->get_album_art = FALSE;
	#endif
	if (all_f || use_cddb_f)
		cwin->cpref->use_cddb = TRUE;
	if (all_f || use_mpris2_f)
		cwin->cpref->use_mpris2 = TRUE;

	if (err)
		return -1;
	else
		return 0;
}

gint init_threads(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Initializing threads");

	#if !GLIB_CHECK_VERSION(2,31,0)
	if (!g_thread_supported())
		g_thread_init(NULL);
	#endif

	#if !GLIB_CHECK_VERSION(2,35,1)
	g_type_init ();
	#endif

	return 0;
}

gint init_first_state(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Initializing state");

	cwin->cstate->filter_entry = NULL;

	cwin->cstate->dragging = FALSE;
	cwin->cstate->curr_mobj_clear = FALSE;

	cwin->cstate->curr_mobj = NULL;

	cwin->cstate->view_change = TRUE;

	/* Init others default flags */

#ifdef HAVE_LIBCLASTFM
	cwin->clastfm->session_id = NULL;
	cwin->clastfm->status = LASTFM_STATUS_INVALID;
	init_tag_struct(cwin->clastfm->ntags);
#endif
	cwin->osd_notify = NULL;

	return 0;
}

void state_free (struct con_state *cstate)
{
	if (cstate->cdda_drive)
		cdio_cddap_close(cstate->cdda_drive);
	if (cstate->cddb_disc)
		cddb_disc_destroy(cstate->cddb_disc);
	if (cstate->cddb_conn) {
		cddb_destroy(cstate->cddb_conn);
		libcddb_shutdown();
	}

	g_free(cstate->last_folder);

	g_slice_free(struct con_state, cstate);
}

void init_tag_completion(struct con_win *cwin)
{
	GtkListStore *artist_tag_model, *album_tag_model, *genre_tag_model;

	/* Artist */

	artist_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[0] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[0],
				       GTK_TREE_MODEL(artist_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[0], 0);
	g_object_unref(artist_tag_model);

	/* Album */

	album_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[1] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[1],
				       GTK_TREE_MODEL(album_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[1], 0);
	g_object_unref(album_tag_model);

	/* Genre */

	genre_tag_model = gtk_list_store_new(1, G_TYPE_STRING);
	cwin->completion[2] = gtk_entry_completion_new();
	gtk_entry_completion_set_model(cwin->completion[2],
				       GTK_TREE_MODEL(genre_tag_model));
	gtk_entry_completion_set_text_column(cwin->completion[2], 0);
	g_object_unref(genre_tag_model);
}

void init_toggle_buttons(struct con_win *cwin)
{
	gboolean shuffle, repeat;

	shuffle = pragha_preferences_get_shuffle(cwin->preferences);
	repeat = pragha_preferences_get_repeat(cwin->preferences);

	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(cwin->shuffle_button), shuffle);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(cwin->repeat_button), repeat);
}

void init_menu_actions(struct con_win *cwin)
{
	GtkAction *action = NULL;
	gboolean shuffle, repeat;

	shuffle = pragha_preferences_get_shuffle(cwin->preferences);
	repeat = pragha_preferences_get_repeat(cwin->preferences);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/PlaybackMenu/Shuffle");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), shuffle);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/PlaybackMenu/Repeat");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), repeat);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Fullscreen");
	if(!g_ascii_strcasecmp(cwin->cpref->start_mode, FULLSCREEN_STATE))
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), TRUE);
	else
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Lateral panel");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), cwin->cpref->lateral_panel);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Status bar");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), cwin->cpref->status_bar);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/ViewMenu/Playback controls below");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), cwin->cpref->controls_below);
}

void init_pixbufs(struct con_win *cwin)
{
	GtkIconTheme *icontheme = gtk_icon_theme_get_default();

	cwin->pixbuf->pixbuf_app = gdk_pixbuf_new_from_file(PIXMAPDIR"/pragha.png", NULL);
	if (!cwin->pixbuf->pixbuf_app)
		g_warning("Unable to load pragha png");

	cwin->pixbuf->pixbuf_artist = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
									"/artist.png",
									16,
									16,
									TRUE,
									NULL);
	if (!cwin->pixbuf->pixbuf_artist)
		g_warning("Unable to load artist png");

	cwin->pixbuf->pixbuf_album = gtk_icon_theme_load_icon(icontheme,
							      "media-optical",
							      16,
							      0,
							      NULL);
	if (!cwin->pixbuf->pixbuf_album)
		cwin->pixbuf->pixbuf_album = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
										"/album.png",
										16,
										16,
										TRUE,
										NULL);
	if (!cwin->pixbuf->pixbuf_album)
		g_warning("Unable to load album png");

	cwin->pixbuf->pixbuf_track = gtk_icon_theme_load_icon(icontheme,
							     "audio-x-generic",
							     16,
							     0,
							     NULL);
	if (!cwin->pixbuf->pixbuf_track)
		cwin->pixbuf->pixbuf_track = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
										"/track.png",
										16,
										16,
										TRUE,
										NULL);
	if (!cwin->pixbuf->pixbuf_track)
		g_warning("Unable to load track png");

	cwin->pixbuf->pixbuf_genre = gdk_pixbuf_new_from_file_at_scale(PIXMAPDIR
								       "/genre.png",
								       16,
								       16,
								       TRUE,
								       NULL);
	if (!cwin->pixbuf->pixbuf_genre)
		g_warning("Unable to load genre png");

	cwin->pixbuf->pixbuf_dir = gtk_icon_theme_load_icon(icontheme,
							    "folder-music",
							    16,
							    0,
							    NULL);
	if (!cwin->pixbuf->pixbuf_dir)
		cwin->pixbuf->pixbuf_dir = gtk_icon_theme_load_icon(icontheme,
										"folder",
										16,
										0,
										NULL);
	if (!cwin->pixbuf->pixbuf_dir)
		g_warning("Unable to load folder png");
}

#if HAVE_LIBXFCE4UI
static void
pragha_session_quit (XfceSMClient *sm_client, struct con_win *cwin)
{
	gtk_main_quit();
}

void
pragha_session_save_state (XfceSMClient *sm_client, struct con_win *cwin)
{
	if (pragha_preferences_get_restore_playlist(cwin->preferences))
		save_current_playlist_state(cwin);
	save_preferences(cwin);
}

gint init_session_support(struct con_win *cwin)
{
	XfceSMClient *client;
	GError *error = NULL;
 
	client =  xfce_sm_client_get ();
	xfce_sm_client_set_priority (client, XFCE_SM_CLIENT_PRIORITY_DEFAULT);
	xfce_sm_client_set_restart_style (client, XFCE_SM_CLIENT_RESTART_NORMAL);
	xfce_sm_client_set_desktop_file(client, DESKTOPENTRY);

	g_signal_connect (G_OBJECT (client), "quit",
			  G_CALLBACK (pragha_session_quit), cwin);
	g_signal_connect (G_OBJECT (client), "save-state",
			  G_CALLBACK (pragha_session_save_state), cwin);

	if(!xfce_sm_client_connect (client, &error)) {
		g_warning ("Failed to connect to session manager: %s", error->message);
		g_error_free (error);
	}

	return 0;
}
#endif

static gboolean
window_state_event (GtkWidget *widget, GdkEventWindowState *event, struct con_win *cwin)
{
	GtkAction *action_fullscreen;

 	if (event->type == GDK_WINDOW_STATE && (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
		action_fullscreen = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Fullscreen");

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_fullscreen), (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0);
	}

	return TRUE;
}

void init_gui(gint argc, gchar **argv, struct con_win *cwin)
{
	GtkUIManager *menu;
	GtkWidget *vbox, *toolbar, *info_box, *hbox_main, *status_bar, *menu_bar;

	CDEBUG(DBG_INFO, "Initializing gui");

	gtk_init(&argc, &argv);

	g_set_application_name(_("Pragha Music Player"));
	g_setenv("PULSE_PROP_media.role", "audio", TRUE);

	init_pixbufs(cwin);

	/* Main window */

	cwin->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	if (cwin->pixbuf->pixbuf_app)
		gtk_window_set_icon(GTK_WINDOW(cwin->mainwindow),
				    cwin->pixbuf->pixbuf_app);

	gtk_window_set_title(GTK_WINDOW(cwin->mainwindow), _("Pragha Music Player"));

#if GTK_CHECK_VERSION (3, 0, 0)
	//XXX remove this?
#else
	GdkScreen *screen = gtk_widget_get_screen(cwin->mainwindow);
	GdkColormap *colormap = gdk_screen_get_rgba_colormap (screen);
	if (colormap && gdk_screen_is_composited (screen)){
		gtk_widget_set_default_colormap(colormap);
	}
#endif

	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "window-state-event",
			 G_CALLBACK(window_state_event), cwin);
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "delete_event",
			 G_CALLBACK(exit_gui), cwin);
	g_signal_connect(G_OBJECT(cwin->mainwindow),
			 "destroy",
			 G_CALLBACK(exit_pragha), cwin);

	/* Set Default Size */

	gtk_window_set_default_size(GTK_WINDOW(cwin->mainwindow),
				    cwin->cpref->window_width,
				    cwin->cpref->window_height);

	/* Set Position */

	if (cwin->cpref->window_x != -1) {
		gtk_window_move(GTK_WINDOW(cwin->mainwindow),
				cwin->cpref->window_x,
				cwin->cpref->window_y);
	}
	else {
		gtk_window_set_position(GTK_WINDOW(cwin->mainwindow),
					GTK_WIN_POS_CENTER);
	}

	/* Systray */

	create_status_icon(cwin);

	/* Main Vbox */

	vbox = gtk_vbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(cwin->mainwindow), vbox);

	/* Create hboxen */

	menu = create_menu(cwin);
	toolbar = create_toolbar(cwin);
	info_box = create_info_box(cwin);
	hbox_main = create_main_region(cwin);
	status_bar = create_status_bar(cwin);
	menu_bar = gtk_ui_manager_get_widget(menu, "/Menubar");

	/* Pack all hboxen into vbox */

	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(menu_bar),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(toolbar),
			   FALSE,FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(info_box),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(hbox_main),
			   TRUE,TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   GTK_WIDGET(status_bar),
			   FALSE,FALSE, 0);

	/* Send notifications on gui, OSD and mpris of new songs */

	g_signal_connect(cwin->backend,
			 "notify::state",
			 G_CALLBACK(pragha_playback_notificate_new_track), cwin);

	/* Init window state */

	if(!g_ascii_strcasecmp(cwin->cpref->start_mode, FULLSCREEN_STATE)) {
		gtk_widget_show_all(cwin->mainwindow);
	}
	else if(!g_ascii_strcasecmp(cwin->cpref->start_mode, ICONIFIED_STATE)) {
		if(gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))) {
			gtk_widget_hide(GTK_WIDGET(cwin->mainwindow));
		}
		else {
			g_warning("(%s): No embedded status_icon.", __func__);
			gtk_window_iconify (GTK_WINDOW (cwin->mainwindow));
			gtk_widget_show_all(cwin->mainwindow);
			gtk_widget_hide(GTK_WIDGET(cwin->unfull_button));
			#ifdef HAVE_LIBCLASTFM
			gtk_widget_hide(cwin->ntag_lastfm_button);
			#endif
		}
	}
	else {
		gtk_widget_show_all(cwin->mainwindow);
		gtk_widget_hide(GTK_WIDGET(cwin->unfull_button));
		#ifdef HAVE_LIBCLASTFM
		gtk_widget_hide(cwin->ntag_lastfm_button);
		#endif
	}

	init_menu_actions(cwin);
	init_toggle_buttons(cwin);
	update_menu_playlist_changes(cwin);

	gtk_widget_grab_focus(GTK_WIDGET(cwin->play_button));

	#if HAVE_LIBXFCE4UI
	init_session_support(cwin);
	#else
	/* set a unique role on each window (for session management) */
	gchar *role = g_strdup_printf ("Pragha-%p-%d-%d", cwin->mainwindow, (gint) getpid (), (gint) time (NULL));
	gtk_window_set_role (GTK_WINDOW (cwin->mainwindow), role);
	g_free (role);
	#endif

	#if GTK_CHECK_VERSION (3, 0, 0)
	init_gui_state(cwin);
	#else
	gtk_init_add(_init_gui_state, cwin);
	#endif
}
