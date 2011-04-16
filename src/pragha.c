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

gint debug_level;

/* FIXME: Cleanup track refs */
void common_cleanup(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Cleaning up");

	if ((cwin->cstate->state == ST_STOPPED) && (cwin->cstate->curr_mobj_clear))
		delete_musicobject(cwin->cstate->curr_mobj);

	if ((cwin->cstate->state == ST_PLAYING) || (cwin->cstate->state == ST_PAUSED))
		stop_playback(cwin);

	save_preferences(cwin);

	g_object_unref(cwin->library_store);
	g_object_unref(cwin->pixbuf->image_play);
	g_object_unref(cwin->pixbuf->image_pause);

	if (cwin->pixbuf->pixbuf_app)
		g_object_unref(cwin->pixbuf->pixbuf_app);
	if (cwin->pixbuf->pixbuf_dir)
		g_object_unref(cwin->pixbuf->pixbuf_dir);
	if (cwin->pixbuf->pixbuf_artist)
		g_object_unref(cwin->pixbuf->pixbuf_artist);
	if (cwin->pixbuf->pixbuf_album)
		g_object_unref(cwin->pixbuf->pixbuf_album);
	if (cwin->pixbuf->pixbuf_track)
		g_object_unref(cwin->pixbuf->pixbuf_track);
	if (cwin->pixbuf->pixbuf_genre)
		g_object_unref(cwin->pixbuf->pixbuf_genre);

	g_slice_free(struct pixbuf, cwin->pixbuf);

	if (cwin->album_art)
		gtk_widget_destroy(cwin->album_art);

	if (cwin->cstate->cdda_drive)
		cdio_cddap_close(cwin->cstate->cdda_drive);
	if (cwin->cstate->cddb_disc)
		cddb_disc_destroy(cwin->cstate->cddb_disc);
	if (cwin->cstate->cddb_conn) {
		cddb_destroy(cwin->cstate->cddb_conn);
		libcddb_shutdown();
	}

	g_free(cwin->cpref->lw.lastfm_user);
	g_free(cwin->cpref->lw.lastfm_pass);
	g_free(cwin->cpref->configrc_file);
	g_free(cwin->cpref->installed_version);
	g_free(cwin->cpref->audio_sink);
	g_free(cwin->cpref->audio_alsa_device);
	g_free(cwin->cpref->audio_oss_device);
	g_free(cwin->cpref->album_art_pattern);
	g_free(cwin->cpref->audio_cd_device);
	g_free(cwin->cpref->start_mode);
	g_free(cwin->cpref->sidebar_pane);
	g_key_file_free(cwin->cpref->configrc_keyfile);
	free_str_list(cwin->cpref->library_dir);
	free_str_list(cwin->cpref->lib_add);
	free_str_list(cwin->cpref->lib_delete);
	free_str_list(cwin->cpref->library_tree_nodes);
	free_str_list(cwin->cpref->playlist_columns);
	g_slist_free(cwin->cpref->playlist_column_widths);
	g_slice_free(struct con_pref, cwin->cpref);

	g_rand_free(cwin->cstate->rand);
	g_free(cwin->cstate->last_folder);
	g_mutex_free(cwin->cstate->c_mutex);

	/* Hack, hack */
	if (g_mutex_trylock(cwin->cstate->l_mutex) == TRUE) {
		g_mutex_unlock(cwin->cstate->l_mutex);
		g_mutex_free(cwin->cstate->l_mutex);
	}
	g_cond_free(cwin->cstate->c_cond);
	g_slice_free(struct con_state, cwin->cstate);

	g_free(cwin->cdbase->db_file);
	sqlite3_close(cwin->cdbase->db);
	g_slice_free(struct con_dbase, cwin->cdbase);

	if (cwin->cstate->audio_init && cwin->cmixer)
		cwin->cmixer->deinit_mixer(cwin);
	if (cwin->clibao->ao_dev) {
		CDEBUG(DBG_INFO, "Freeing ao dev");
		ao_close(cwin->clibao->ao_dev);
	}
	ao_shutdown();
	g_slice_free(struct con_mixer, cwin->cmixer);
	g_slice_free(struct con_libao, cwin->clibao);

	g_free(cwin->clastfm->session_id);
	g_free(cwin->clastfm->submission_url);
	if (cwin->clastfm->curl_handle)
		curl_easy_cleanup(cwin->clastfm->curl_handle);
	curl_global_cleanup();
	g_slice_free(struct con_lastfm, cwin->clastfm);

	dbus_connection_remove_filter(cwin->con_dbus,
				      dbus_filter_handler,
				      cwin);
	dbus_bus_remove_match(cwin->con_dbus,
			      "type='signal',path='/org/pragha/DBus'",
			      NULL);
	dbus_connection_unref(cwin->con_dbus);

	if (notify_is_initted())
		notify_uninit();

	#if GTK_CHECK_VERSION(2,20,0)
	/* for Keybinder you need Gtk >= 2.20 */
	keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
	keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
	keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
	keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
	keybinder_unbind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler);
	#endif

	g_option_context_free(cwin->cmd_context);

	g_slice_free(struct con_win, cwin);
}

void exit_pragha(GtkWidget *widget, struct con_win *cwin)
{
	if (cwin->cpref->save_playlist)
		save_current_playlist_state(cwin);
	common_cleanup(cwin);
	gtk_main_quit();

	CDEBUG(DBG_INFO, "Halt.");
}

gint main(gint argc, gchar *argv[])
{
	gint ret = 0;
	struct con_win *cwin;

	cwin = g_slice_new0(struct con_win);
	cwin->pixbuf = g_slice_new0(struct pixbuf);
	cwin->cpref = g_slice_new0(struct con_pref);
	cwin->cstate = g_slice_new0(struct con_state);
	cwin->cdbase = g_slice_new0(struct con_dbase);
	cwin->cmixer = g_slice_new0(struct con_mixer);
	cwin->clibao = g_slice_new0(struct con_libao);
	cwin->clastfm = g_slice_new0(struct con_lastfm);
	debug_level = 0;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	if (init_dbus(cwin) == -1) {
		g_critical("Unable to init dbus connection");
		return -1;
	}

	if (init_dbus_handlers(cwin) == -1) {
		g_critical("Unable to initialize DBUS filter handlers");
		return -1;
	}

	if (init_options(cwin, argc, argv) == -1)
		return -1;

	/* Allow only one instance */

	if (!cwin->cstate->unique_instance)
		return 0;

	if (init_config(cwin) == -1) {
		g_critical("Unable to init configuration");
		return -1;
	}

	if (init_musicdbase(cwin) == -1) {
		g_critical("Unable to init music dbase");
		return -1;
	}

	ret = init_audio(cwin);
	if (ret == -EINVAL) {
		g_critical("Unable to init audio");
		return -1;
	} else if (ret == -ENODEV) {
		g_critical("Audio init failed, choose appropriate settings "
			   "from the preferences.");
	}

	/* Init libcurl before spawning threads */

	if (init_lastfm(cwin) == -1) {
		g_critical("Unable to initialize curl");
	}

	if (init_threads(cwin) == -1) {
		g_critical("Unable to init threads");
		return -1;
	}

	if (init_notify(cwin) == -1) {
		g_critical("Unable to initialize libnotify");
		return -1;
	}

	#if GTK_CHECK_VERSION (2, 20, 0)
	/* for Keybinder you need Gtk >= 2.20 */
	if (init_keybinder(cwin) == -1) {
		g_critical("Unable to initialize keybinder");
		return -1;
	}
	#endif

	init_state(cwin);

	gdk_threads_enter();
	init_gui(argc, argv, cwin);
	CDEBUG(DBG_INFO, "Init done. Running ...");
	gtk_main();
	gdk_threads_leave();

	return 0;
}
