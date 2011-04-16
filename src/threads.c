/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

static gpointer play_thread_wav(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_wav(cwin);

	return NULL;
}

static gpointer play_thread_mp3(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_mp3(cwin);

	return NULL;
}

static gpointer play_thread_flac(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_flac(cwin);

	return NULL;
}

static gpointer play_thread_oggvorbis(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_oggvorbis(cwin);

	return NULL;
}

static gpointer play_thread_modplug(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_modplug(cwin);

	return NULL;
}

static gpointer play_thread_cdda(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	play_cdda(cwin);

	return NULL;
}

static gpointer lastfm_update(gpointer data)
{
	struct lastfm_track *ltrack = data;
	struct con_win *cwin = ltrack->cwin;
	gint ret;

	if (!cwin->cpref->lw.lastfm_support)
		goto free_exit;

	if (ltrack->tags.length < LASTFM_MIN_PLAYTIME)
		goto free_exit;

	if (ltrack->play_duration <
	    MIN(LASTFM_MIN_DURATION, (ltrack->tags.length / 2)))
		goto free_exit;

	CDEBUG(DBG_INFO, "Submitting to Last.fm");

	g_mutex_lock(cwin->cstate->l_mutex);
	ret = lastfm_submit(cwin, ltrack->tags.title,
			    ltrack->tags.artist,
			    ltrack->tags.album,
			    ltrack->tags.track_no,
			    ltrack->tags.length,
			    ltrack->start_time,
			    ltrack->tsource);
	g_mutex_unlock(cwin->cstate->l_mutex);

	if (ret < 0) {
		CDEBUG(DBG_INFO, "Last.fm libcurl submission failed");
	}

free_exit:
	g_slice_free(struct lastfm_track, ltrack);
	return NULL;
}

static gboolean advance_playback(gpointer data)
{
	struct con_win *cwin = data;
	GtkTreePath *path;
	struct musicobject *mobj = NULL;
	GThread *thread;

	if (cwin->cstate->advance_track) {

		CDEBUG(DBG_INFO, "Advancing to next track");

		if (cwin->cstate->c_thread)
			g_thread_join(cwin->cstate->c_thread);

		gdk_threads_enter();
		unset_current_song_info(cwin);
		unset_track_progress_bar(cwin);
		gdk_threads_leave();

		cwin->cstate->c_thread = NULL;

		if (cwin->cstate->curr_mobj_clear)
			delete_musicobject(cwin->cstate->curr_mobj);

		/* Get the next track to be played */

		path = current_playlist_get_next(cwin);

		cwin->cstate->state = ST_STOPPED;
		play_button_toggle_state(cwin);

		/* No more tracks */

		if (!path)
			return FALSE;

		/* Start playing new track */

		mobj = current_playlist_mobj_at_path(path, cwin);

		thread = start_playback(mobj, cwin);
		if (!thread)
			g_critical("Unable to create playback thread");
		else
			update_current_state(thread, path, PLAYLIST_NEXT, cwin);

		gtk_tree_path_free(path);
	}

	return FALSE;
}

void playback_end_cleanup(struct con_win *cwin, struct lastfm_track *ltrack,
			  gint ret, gboolean lastfm_f)
{
	GError *err = NULL;

	if (lastfm_f) {
		if (g_thread_create(lastfm_update, ltrack, FALSE, &err) == NULL) {
			g_warning("Unable to spawn last.fm submission thread");
			g_error_free(err);
			err = NULL;
		}
	}

	if (!ret) {
		cwin->cstate->advance_track = TRUE;
		g_idle_add(advance_playback, cwin);
	}
	else if (ret == -1)
		cwin->cstate->advance_track = FALSE;
}

gboolean update_track_progress_bar(gpointer data)
{
	struct con_win *cwin = data;

	gdk_threads_enter();
	__update_track_progress_bar(cwin, cwin->cstate->newsec);
	gdk_threads_leave();

	return FALSE;
}

gboolean update_current_song_info(gpointer data)
{
	struct con_win *cwin = data;

	gdk_threads_enter();
	__update_current_song_info(cwin, cwin->cstate->newsec);
	gdk_threads_leave();

	return FALSE;
}

GThread* start_playback(struct musicobject *mobj, struct con_win *cwin)
{
	GThread *thread = NULL;
	GError *error = NULL;

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return NULL;
	}

	if (!cwin->cstate->audio_init) {
		g_idle_add(dialog_audio_init, cwin);
		return NULL;
	}

	if ((cwin->cstate->state == ST_PLAYING) ||
	    (cwin->cstate->state == ST_PAUSED)) {
		stop_playback(cwin);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->cmd = 0;
	cwin->cstate->curr_mobj_clear = FALSE;

	switch(mobj->file_type) {
	case FILE_WAV:
		thread = g_thread_create(play_thread_wav, cwin, TRUE, &error);
		break;
	case FILE_MP3:
		thread = g_thread_create(play_thread_mp3, cwin, TRUE, &error);
		break;
	case FILE_FLAC:
		thread = g_thread_create(play_thread_flac, cwin, TRUE, &error);
		break;
	case FILE_OGGVORBIS:
		thread = g_thread_create(play_thread_oggvorbis, cwin, TRUE, &error);
		break;
	case FILE_MODPLUG:
		thread = g_thread_create(play_thread_modplug, cwin, TRUE, &error);
		break;
	case FILE_CDDA:
		thread = g_thread_create(play_thread_cdda, cwin, TRUE, &error);
		break;
	default:
		g_warning("Unknown file type");
	}

	if (error) {
		g_critical("Unable to create playback thread : %s",
			   error->message);
		return NULL;
	}
	else {
		cwin->cstate->state = ST_PLAYING;
		play_button_toggle_state(cwin);

		CDEBUG(DBG_INFO, "Starting playback");

		return thread;
	}
}

void pause_playback(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Pause playback");

	if (cwin->cstate->state == ST_PLAYING) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_PAUSE;
		g_mutex_unlock(cwin->cstate->c_mutex);

		cwin->cstate->state = ST_PAUSED;
		play_button_toggle_state(cwin);
	}
}

void resume_playback(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Resuming playback");

	if (cwin->cstate->state == ST_PAUSED) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_RESUME;
		g_cond_signal(cwin->cstate->c_cond);
		g_mutex_unlock(cwin->cstate->c_mutex);

		cwin->cstate->state = ST_PLAYING;
		play_button_toggle_state(cwin);
	}
}

void stop_playback(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Stopping playback");

	if ((cwin->cstate->state == ST_PAUSED) ||
	    (cwin->cstate->state == ST_PLAYING)) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_STOP;
		g_cond_signal(cwin->cstate->c_cond);
		g_mutex_unlock(cwin->cstate->c_mutex);

		if (cwin->cstate->c_thread)
			g_thread_join(cwin->cstate->c_thread);

		if (cwin->cstate->curr_mobj_clear) {
			delete_musicobject(cwin->cstate->curr_mobj);
			cwin->cstate->curr_mobj_clear = FALSE;
		}

		unset_current_song_info(cwin);
		unset_track_progress_bar(cwin);
		unset_album_art(cwin);

		cwin->cstate->c_thread = NULL;
		cwin->cstate->state = ST_STOPPED;
		play_button_toggle_state(cwin);
	}
}

void seek_playback(struct con_win *cwin, gint seek, gdouble fraction)
{
	CDEBUG(DBG_INFO, "Seeking to new len: %d", seek);

	if (cwin->cstate->state == ST_PLAYING) {
		g_mutex_lock(cwin->cstate->c_mutex);
		cwin->cstate->cmd = CMD_PLAYBACK_SEEK;
		cwin->cstate->seek_len = seek;
		cwin->cstate->seek_fraction = fraction;
		g_mutex_unlock(cwin->cstate->c_mutex);
	}
}

gint process_thread_command(struct con_win *cwin)
{
	gint ret = 0;
	enum thread_cmd cmd = 0;

	g_mutex_lock(cwin->cstate->c_mutex);
	cmd = cwin->cstate->cmd;
	g_mutex_unlock(cwin->cstate->c_mutex);

	/* Stop playing */

	if (cmd == CMD_PLAYBACK_STOP)
		ret = CMD_PLAYBACK_STOP;

	/* Pause playing */

	else if (cmd == CMD_PLAYBACK_PAUSE) {

		/* Wait for resume command to come through */

		ao_close(cwin->clibao->ao_dev);
		g_mutex_lock(cwin->cstate->c_mutex);

		while (cwin->cstate->cmd == CMD_PLAYBACK_PAUSE)
			g_cond_wait(cwin->cstate->c_cond, cwin->cstate->c_mutex);

		/* A stop command may have been given while waiting for resume */

		if (cwin->cstate->cmd == CMD_PLAYBACK_STOP)
			ret = CMD_PLAYBACK_STOP;

		/* Received a resume command */

		else if (cwin->cstate->cmd == CMD_PLAYBACK_RESUME)
			ret = CMD_PLAYBACK_RESUME;

		g_mutex_unlock(cwin->cstate->c_mutex);
		if (open_audio_device(0, 0, TRUE, cwin) == -1) {
			g_warning("Unable to resume");
		}
	}

	/* Seek to a new position */

	else if (cmd == CMD_PLAYBACK_SEEK)
		ret = CMD_PLAYBACK_SEEK;

	return ret;
}

gint reset_thread_command(struct con_win *cwin, gint cmd)
{
	gint c = cmd;

	g_mutex_lock(cwin->cstate->c_mutex);

	/*
	 * A STOP command may have come if the seek was too slow.
	 * Handle it here, otherwise we end up in a deadlock.
	 */

	if (cwin->cstate->cmd == CMD_PLAYBACK_STOP) {
		c = CMD_PLAYBACK_STOP;
		goto unlock;
	}

	cwin->cstate->cmd = 0;

unlock:
	g_mutex_unlock(cwin->cstate->c_mutex);
	return c;
}

static gpointer do_lastfm_init_thread(gpointer data)
{
	struct con_win *cwin = (struct con_win *)data;

	if (lastfm_handshake(cwin) < 0)
		g_warning("Unable to complete last.fm handshake");

	return NULL;
}

void lastfm_init_thread(struct con_win *cwin)
{
	GThread *thread = NULL;
	GError *error = NULL;

	thread = g_thread_create(do_lastfm_init_thread, cwin, TRUE, &error);
	if (error)
		g_critical("Unable to create last.fm init thread: %s",
			   error->message);
}
