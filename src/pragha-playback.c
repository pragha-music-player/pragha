/*************************************************************************/
/* Copyright (C) 2010-2012 matias <mati86dl@gmail.com>                   */
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

#include "pragha-playback.h"
#include "pragha-playlist.h"
#include "pragha-notify.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"
#include "pragha.h"

/**********************/
/* Playback functions */
/**********************/

/* Play prev track in current playlist */

void pragha_playback_prev_track(struct con_win *cwin)
{
	GtkTreePath *path;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Want to play a song previously played");

	/* Get the next (prev) track to be played */
	path = current_playlist_get_prev(cwin->cplaylist);

	/* No more tracks */
	if (!path)
		return;

	/* Stop currently playing track */
	pragha_backend_stop(cwin->backend);

	/* Start playing new track */
	pragha_playlist_set_current_update_action(cwin->cplaylist, PLAYLIST_PREV);
	pragha_playlist_update_current_playlist_state(cwin->cplaylist, path);

	mobj = current_playlist_mobj_at_path (path, cwin->cplaylist);
	pragha_backend_set_musicobject (cwin->backend, mobj);
	pragha_backend_play(cwin->backend);

	gtk_tree_path_free(path);
}

/* Start playback of a new track, or resume playback of current track */

void pragha_playback_play_pause_resume(struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	GtkTreePath *path=NULL;

	CDEBUG(DBG_BACKEND, "Play pause or resume a track based on the current state");

	/* New action is based on the current state */

	/************************************/
        /* State     Action		    */
	/* 				    */
	/* Playing   Pause playback	    */
	/* Paused    Resume playback	    */
	/* Stopped   Start playback	    */
        /************************************/

	switch (pragha_backend_get_state (cwin->backend)) {
	case ST_PLAYING:
		pragha_backend_pause(cwin->backend);
		break;
	case ST_PAUSED:
		pragha_backend_resume(cwin->backend);
		break;
	case ST_STOPPED:
		if(pragha_playlist_is_changing(cwin->cplaylist))
			break;
		if(pragha_playlist_has_queue(cwin->cplaylist))
			path = get_next_queue_track(cwin->cplaylist);
		if (!path)
			path = current_playlist_get_selection(cwin->cplaylist);

		if(!path) {
			if(pragha_preferences_get_shuffle(cwin->preferences))
				path = get_first_random_track(cwin->cplaylist);
			else
				path = gtk_tree_path_new_first();
		}

		if (pragha_preferences_get_shuffle(cwin->preferences))
			pragha_playlist_set_first_rand_ref(cwin->cplaylist, path);

		pragha_playlist_set_current_update_action(cwin->cplaylist, PLAYLIST_CURR);
		pragha_playlist_update_current_playlist_state(cwin->cplaylist, path);

		mobj = current_playlist_mobj_at_path (path, cwin->cplaylist);

		pragha_backend_set_musicobject (cwin->backend, mobj);
		pragha_backend_play(cwin->backend);

		gtk_tree_path_free(path);
		break;
	default:
		break;
	}
}

/* Stop the playback */

void pragha_playback_stop(struct con_win *cwin)
{
	CDEBUG(DBG_BACKEND, "Stopping the current song");

	if (pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	pragha_backend_stop(cwin->backend);
}

/* Play next song when terminate a song. */

void pragha_advance_playback (struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Advancing to next track");

	/* Stop to set ready and clear all info */
	pragha_backend_stop(cwin->backend);

	if(pragha_playlist_is_changing(cwin->cplaylist))
		return;

	/* Get the next track to be played */
	path = current_playlist_get_next (cwin->cplaylist);

	/* No more tracks */
	if (!path)
		return;

	/* Start playing new track */
	pragha_playlist_set_current_update_action(cwin->cplaylist, PLAYLIST_NEXT);
	pragha_playlist_update_current_playlist_state(cwin->cplaylist, path);

	mobj = current_playlist_mobj_at_path (path, cwin->cplaylist);
	pragha_backend_set_musicobject (cwin->backend, mobj);
	pragha_backend_play(cwin->backend);

	gtk_tree_path_free (path);
}

/* Play next track in current_playlist */

void pragha_playback_next_track(struct con_win *cwin)
{
	CDEBUG(DBG_BACKEND, "Want to advancing to next track");

	/* Are we playing right now ? */
	if (pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	/* Play a new song */
	pragha_advance_playback(cwin);
}

/******************************************/
/* Update playback state based on backend */
/******************************************/

void
pragha_backend_notificate_new_state (PraghaBackend *backend, GParamSpec *pspec, struct con_win *cwin)
{
	enum player_state state = pragha_backend_get_state (backend);

	switch (state) {
		case ST_PLAYING:
			/* New song?. */
			if(pragha_playlist_get_current_update_action(cwin->cplaylist) != PLAYLIST_NONE) {
				/* Set public musicobject based on backend. */
				pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
				cwin->cstate->curr_mobj = pragha_musicobject_dup(pragha_backend_get_musicobject(backend));
				g_object_ref(cwin->cstate->curr_mobj);
				pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

				pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
				CDEBUG(DBG_BACKEND, "Definitely play a new song: %s",
				                     pragha_musicobject_get_file(cwin->cstate->curr_mobj));
				pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

				/* Update current song info */
				__update_current_song_info(cwin);
				__update_progress_song_info(cwin, 0);

				/* Update and jump in current playlist */
				update_current_playlist_view_new_track(cwin->cplaylist, backend);

				/* Update album art */
				pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
				update_album_art(cwin->cstate->curr_mobj, cwin);
				pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

				/* Show osd, and inform new album art. */
				show_osd(cwin);
				mpris_update_metadata_changed(cwin);

				pragha_playlist_report_finished_action(cwin->cplaylist);
			}
			break;
		case ST_PAUSED:
			/* Nothing here. */
			break;
		case ST_STOPPED:
			pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
			if(cwin->cstate->curr_mobj) {
				g_object_unref(cwin->cstate->curr_mobj);
				cwin->cstate->curr_mobj = NULL;
			}
			pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);
			break;
		default:
			break;
	}
}

void
pragha_backend_finished_song (PraghaBackend *backend, struct con_win *cwin)
{
	pragha_advance_playback(cwin);
}

void
pragha_backend_tags_changed (PraghaBackend *backend, struct con_win *cwin)
{
	PraghaMusicobject *nmobj;
	gint changed = 0;

	if(pragha_backend_get_state (backend) != ST_PLAYING)
		return;

	nmobj = pragha_backend_get_musicobject(backend);
	changed = TAG_TITLE_CHANGED | TAG_ARTIST_CHANGED;

	/* Update the public mobj */
	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, nmobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	/* Update change on gui */
	__update_current_song_info(cwin);
	mpris_update_metadata_changed(cwin);

	/* Update the playlist */
	pragha_playlist_update_current_track(cwin->cplaylist, changed, nmobj);
}
