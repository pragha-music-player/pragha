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
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-debug.h"
#include "pragha.h"

static void pragha_playback_update_current_album_art (struct con_win *cwin, PraghaMusicobject *mobj);

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
	/* State     Action                 */
	/*                                  */
	/* Playing   Pause playback         */
	/* Paused    Resume playback        */
	/* Stopped   Start playback         */
	/************************************/

	switch (pragha_backend_get_state (cwin->backend)) {
	case ST_PLAYING:
		pragha_backend_pause(cwin->backend);
		break;
	case ST_PAUSED:
		pragha_backend_resume(cwin->backend);
		break;
	case ST_STOPPED:
		if (pragha_playlist_is_changing(cwin->cplaylist) ||
		    pragha_playlist_get_no_tracks(cwin->cplaylist) == 0)
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
	PraghaToolbar *toolbar;
	enum player_state state = pragha_backend_get_state (backend);
	PraghaMusicobject *mobj = NULL;

	switch (state) {
		case ST_PLAYING:
			/* New song?. */
			if(pragha_playlist_get_current_update_action(cwin->cplaylist) != PLAYLIST_NONE) {
				mobj = pragha_backend_get_musicobject (cwin->backend);

				CDEBUG(DBG_BACKEND, "Definitely play a new song: %s",
				                     pragha_musicobject_get_file(mobj));

				/* Update current song info */
				toolbar = pragha_window_get_toolbar (pragha_application_get_window(cwin));
				pragha_toolbar_set_title (toolbar, mobj);
				pragha_toolbar_update_progress (toolbar, pragha_musicobject_get_length(mobj), 0);

				/* Update and jump in current playlist */
				update_current_playlist_view_new_track(cwin->cplaylist, backend);

				/* Update album art */
				pragha_playback_update_current_album_art (cwin, mobj);

				/* Show osd, and inform new album art. */
				if (cwin->notify)
					pragha_notify_show_osd (cwin->notify);
				mpris_update_metadata_changed(cwin);

				pragha_playlist_report_finished_action(cwin->cplaylist);
			}
			break;
		case ST_PAUSED:
			/* Nothing here. */
			break;
		case ST_STOPPED:
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
pragha_backend_tags_changed (PraghaBackend *backend, gint changed, struct con_win *cwin)
{
	PraghaToolbar *toolbar;
	PraghaMusicobject *nmobj;

	if(pragha_backend_get_state (backend) != ST_PLAYING)
		return;

	nmobj = pragha_backend_get_musicobject(backend);

	/* Update change on gui */
	toolbar = pragha_window_get_toolbar (pragha_application_get_window(cwin));
	pragha_toolbar_set_title(toolbar, nmobj);
	mpris_update_metadata_changed(cwin);

	/* Update the playlist */
	pragha_playlist_update_current_track(cwin->cplaylist, changed, nmobj);
}

static void
pragha_playback_update_current_album_art (struct con_win *cwin, PraghaMusicobject *mobj)
{
	PraghaToolbar *toolbar;
	gchar *album_path = NULL, *path = NULL;

	CDEBUG(DBG_INFO, "Update album art");

	if (G_UNLIKELY(!mobj))
		return;

	if (!pragha_musicobject_is_local_file(mobj))
		return;

	if (!pragha_preferences_get_show_album_art(cwin->preferences))
		return;

	#ifdef HAVE_LIBGLYR
	album_path = pragha_glyr_get_image_path_from_cache (cwin->glyr,
	                                                    pragha_musicobject_get_artist(mobj),
	                                                    pragha_musicobject_get_album(mobj));
	#endif
	if (album_path == NULL) {
		path = g_path_get_dirname(pragha_musicobject_get_file(mobj));

		album_path = get_pref_image_path_dir(cwin->preferences, path);
		if (!album_path)
			album_path = get_image_path_from_dir(path);

		g_free(path);
	}

	toolbar = pragha_window_get_toolbar (pragha_application_get_window(cwin));
	pragha_toolbar_set_image_album_art (toolbar, album_path);
	g_free(album_path);
}

void
pragha_playback_show_current_album_art (GObject *object, struct con_win *cwin)
{
	PraghaWindow *window;
	PraghaAlbumArt *albumart;
	gchar *uri = NULL;

	PraghaBackend *backend = pragha_application_get_backend (cwin);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		window = pragha_application_get_window(cwin);

		albumart = pragha_toolbar_get_album_art (pragha_window_get_toolbar(window));
		uri = g_filename_to_uri (pragha_album_art_get_path (albumart), NULL, NULL);

		open_url(uri, pragha_window_get_mainwindow (window));
		g_free (uri);
	}
}

void
pragha_playback_edit_current_track (GObject *object, struct con_win *cwin)
{
	PraghaBackend *backend = pragha_application_get_backend (cwin);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		edit_tags_playing_action(NULL, cwin);
	}
}

void
pragha_playback_seek_fraction (GObject *object, gdouble fraction, struct con_win *cwin)
{
	PraghaToolbar *toolbar;
	gint seek = 0, length = 0;

	PraghaBackend *backend = pragha_application_get_backend (cwin);

	if (pragha_backend_get_state (backend) != ST_PLAYING)
		return;

	length = pragha_musicobject_get_length (pragha_backend_get_musicobject (backend));

	if (length == 0)
		return;

	seek = (length * fraction);

	if (seek >= length)
		seek = length;

	toolbar = pragha_window_get_toolbar (pragha_application_get_window(cwin));
	pragha_toolbar_update_progress (toolbar, length, seek);
	pragha_backend_seek (backend, seek);
}
