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
#include "pragha-art-cache.h"
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
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Want to play a song previously played");

	/* Get the next (prev) track to be played */

	playlist = pragha_application_get_playlist (cwin);

	mobj = pragha_playlist_get_prev_track (playlist);

	if (!mobj)
		return;

	backend = pragha_application_get_backend (cwin);

	/* Stop currently playing track */
	pragha_backend_stop (backend);

	pragha_backend_set_musicobject (backend, mobj);
	pragha_backend_play (backend);
}

/* Start playback of a new track, or resume playback of current track */

void pragha_playback_play_pause_resume(struct con_win *cwin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Play pause or resume a track based on the current state");

	/* New action is based on the current state */

	/************************************/
	/* State     Action                 */
	/*                                  */
	/* Playing   Pause playback         */
	/* Paused    Resume playback        */
	/* Stopped   Start playback         */
	/************************************/

	backend = pragha_application_get_backend (cwin);

	switch (pragha_backend_get_state (backend)) {
	case ST_PLAYING:
		pragha_backend_pause (backend);
		break;
	case ST_PAUSED:
		pragha_backend_resume (backend);
		break;
	case ST_STOPPED:
		playlist = pragha_application_get_playlist (cwin);

		mobj = pragha_playlist_get_any_track (playlist);
		if (!mobj)
			return;

		pragha_backend_set_musicobject (backend, mobj);
		pragha_backend_play (backend);
		break;
	default:
		break;
	}
}

/* Stop the playback */

void pragha_playback_stop(struct con_win *cwin)
{
	PraghaBackend *backend;

	CDEBUG(DBG_BACKEND, "Stopping the current song");

	backend = pragha_application_get_backend (cwin);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	pragha_backend_stop (backend);
}

/* Play next song when terminate a song. */

void pragha_advance_playback (struct con_win *cwin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Advancing to next track");

	backend = pragha_application_get_backend (cwin);

	/* Stop to set ready and clear all info */

	pragha_backend_stop (backend);

	playlist = pragha_application_get_playlist (cwin);
	mobj = pragha_playlist_get_next_track (playlist);

	if (!mobj)
		return;

	pragha_backend_set_musicobject (backend, mobj);
	pragha_backend_play (backend);
}

/* Play next track in current_playlist */

void pragha_playback_next_track(struct con_win *cwin)
{
	PraghaBackend *backend;

	CDEBUG(DBG_BACKEND, "Want to advancing to next track");

	/* Are we playing right now ? */

	backend = pragha_application_get_backend (cwin);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
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
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *mobj = NULL;

	enum player_state state = pragha_backend_get_state (backend);

	switch (state) {
		case ST_PLAYING:
			/* New song?. */
			playlist = pragha_application_get_playlist (cwin);
			if(pragha_playlist_get_current_update_action (playlist) != PLAYLIST_NONE) {
				mobj = pragha_backend_get_musicobject (backend);

				CDEBUG(DBG_BACKEND, "Definitely play a new song: %s",
				                     pragha_musicobject_get_file(mobj));

				/* Update current song info */
				toolbar = pragha_application_get_toolbar (cwin);
				pragha_toolbar_set_title (toolbar, mobj);
				pragha_toolbar_update_progress (toolbar, pragha_musicobject_get_length(mobj), 0);

				/* Update album art */
				pragha_playback_update_current_album_art (cwin, mobj);

				/* Show osd, and inform new album art. */
				if (cwin->notify)
					pragha_notify_show_osd (cwin->notify);
				mpris_update_metadata_changed(cwin);

				pragha_playlist_report_finished_action (playlist);
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
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *nmobj;

	if(pragha_backend_get_state (backend) != ST_PLAYING)
		return;

	nmobj = pragha_backend_get_musicobject(backend);

	/* Update change on gui */
	toolbar = pragha_application_get_toolbar (cwin);
	pragha_toolbar_set_title(toolbar, nmobj);
	mpris_update_metadata_changed(cwin);

	/* Update the playlist */

	playlist = pragha_application_get_playlist (cwin);
	pragha_playlist_update_current_track (playlist, changed, nmobj);
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


	album_path = pragha_art_cache_get (cwin->art_cache,
	                                   pragha_musicobject_get_artist(mobj),
	                                   pragha_musicobject_get_album(mobj));

	if (album_path == NULL) {
		path = g_path_get_dirname(pragha_musicobject_get_file(mobj));

		album_path = get_pref_image_path_dir(cwin->preferences, path);
		if (!album_path)
			album_path = get_image_path_from_dir(path);

		g_free(path);
	}

	toolbar = pragha_application_get_toolbar (cwin);
	pragha_toolbar_set_image_album_art (toolbar, album_path);
	g_free(album_path);
}

void
pragha_playback_show_current_album_art (GObject *object, struct con_win *cwin)
{
	PraghaAlbumArt *albumart;
	gchar *uri = NULL;

	PraghaBackend *backend = pragha_application_get_backend (cwin);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	albumart = pragha_toolbar_get_album_art (pragha_application_get_toolbar (cwin));

	const gchar *albumart_path = pragha_album_art_get_path (albumart);

	if (!albumart_path)
		return;

	uri = g_filename_to_uri (albumart_path, NULL, NULL);
	open_url(uri, pragha_application_get_window (cwin));
	g_free (uri);
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

	pragha_backend_seek (backend, seek);
}
