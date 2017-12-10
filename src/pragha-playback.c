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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-playback.h"

#include "pragha-tags-dialog.h"
#include "pragha-tagger.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha.h"

static void pragha_playback_update_current_album_art (PraghaApplication *pragha, PraghaMusicobject *mobj);

/**********************/
/* Playback functions */
/**********************/

/* Play prev track in current playlist */

void pragha_playback_prev_track(PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;

	CDEBUG(DBG_BACKEND, "Want to play a song previously played");

	/* Are we playing right now ? */

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_go_prev_track (playlist);
}

/* Start playback of a new track, or resume playback of current track */

void pragha_playback_play_pause_resume(PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;

	CDEBUG(DBG_BACKEND, "Play pause or resume a track based on the current state");

	/* New action is based on the current state */

	/************************************/
	/* State     Action                 */
	/*                                  */
	/* Playing   Pause playback         */
	/* Paused    Resume playback        */
	/* Stopped   Start playback         */
	/************************************/

	backend = pragha_application_get_backend (pragha);

	switch (pragha_backend_get_state (backend)) {
	case ST_PLAYING:
		pragha_backend_pause (backend);
		break;
	case ST_PAUSED:
		pragha_backend_resume (backend);
		break;
	case ST_STOPPED:
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_go_any_track (playlist);
		break;
	default:
		break;
	}
}

/* Stop the playback */

void pragha_playback_stop(PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;

	CDEBUG(DBG_BACKEND, "Stopping the current song");

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	pragha_backend_stop (backend);

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_stopped_playback (playlist);
}

/* Play next song when terminate a song. */

void pragha_advance_playback (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;

	CDEBUG(DBG_BACKEND, "Advancing to next track");

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_go_next_track (playlist);
}

/* Play next track in current_playlist */

void pragha_playback_next_track(PraghaApplication *pragha)
{
	PraghaBackend *backend;

	CDEBUG(DBG_BACKEND, "Want to advancing to next track");

	/* Are we playing right now ? */

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	/* Play a new song */
	pragha_advance_playback (pragha);
}

gboolean
pragha_playback_can_go_prev (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	gboolean can_go_prev = FALSE;

	playlist = pragha_application_get_playlist (pragha);
	if (pragha_playlist_get_no_unplayed_tracks(playlist) < pragha_playlist_get_no_tracks(playlist))
		can_go_prev = TRUE;

	return can_go_prev;
}

gboolean
pragha_playback_can_go_next (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	gboolean can_go_next = FALSE;

	playlist = pragha_application_get_playlist (pragha);
	if (pragha_playlist_get_no_unplayed_tracks(playlist) > 0)
		can_go_next = TRUE;

	return can_go_next;
}

gint
pragha_playback_get_no_tracks (PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	playlist = pragha_application_get_playlist (pragha);
	return pragha_playlist_get_no_tracks(playlist);
}


/******************************************/
/* Update playback state based on backend */
/******************************************/

void
pragha_playback_set_playlist_track (PraghaPlaylist *playlist, PraghaMusicobject *mobj, PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;

	CDEBUG(DBG_BACKEND, "Set track activated on playlist");

	/* Stop to set ready and clear all info */
	backend = pragha_application_get_backend (pragha);
	pragha_backend_stop (backend);

	if (!mobj)
		return;

	/* Play new song. */
	pragha_backend_set_musicobject (backend, mobj);
	pragha_backend_play (backend);

	/* Update current song info */
	toolbar = pragha_application_get_toolbar (pragha);
	pragha_toolbar_set_title (toolbar, mobj);
	pragha_toolbar_update_progress (toolbar, pragha_musicobject_get_length(mobj), 0);

	/* Update album art */
	pragha_playback_update_current_album_art (pragha, mobj);
}

void
pragha_backend_finished_song (PraghaBackend *backend, PraghaApplication *pragha)
{
	pragha_advance_playback(pragha);
}

void
pragha_backend_tags_changed (PraghaBackend *backend, gint changed, PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *nmobj;

	if(pragha_backend_get_state (backend) != ST_PLAYING)
		return;

	nmobj = pragha_backend_get_musicobject(backend);

	/* Update change on gui */
	toolbar = pragha_application_get_toolbar (pragha);
	pragha_toolbar_set_title(toolbar, nmobj);

	/* Update the playlist */

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_update_current_track (playlist, changed, nmobj);
}

static void
pragha_playback_update_current_album_art (PraghaApplication *pragha, PraghaMusicobject *mobj)
{
	PraghaToolbar *toolbar;
	PraghaPreferences *preferences;
	PraghaArtCache *art_cache;

	gchar *album_path = NULL, *path = NULL;

	CDEBUG(DBG_INFO, "Update album art");

	if (G_UNLIKELY(!mobj))
		return;

	preferences = pragha_application_get_preferences (pragha);
	if (!pragha_preferences_get_show_album_art (preferences))
		return;

	art_cache = pragha_application_get_art_cache (pragha);
	album_path = pragha_art_cache_get_uri (art_cache,
	                                       pragha_musicobject_get_artist(mobj),
	                                       pragha_musicobject_get_album(mobj));

	if (album_path == NULL) {
		if (!pragha_musicobject_is_local_file(mobj))
			return;

		path = g_path_get_dirname(pragha_musicobject_get_file(mobj));

		album_path = get_pref_image_path_dir (preferences, path);
		if (!album_path)
			album_path = get_image_path_from_dir(path);

		g_free(path);
	}

	toolbar = pragha_application_get_toolbar (pragha);
	pragha_toolbar_set_image_album_art (toolbar, album_path);
	g_free(album_path);
}

void
pragha_playback_show_current_album_art (GObject *object, PraghaApplication *pragha)
{
	PraghaAlbumArt *albumart;
	gchar *uri = NULL;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	albumart = pragha_toolbar_get_album_art (pragha_application_get_toolbar (pragha));

	const gchar *albumart_path = pragha_album_art_get_path (albumart);

	if (!albumart_path)
		return;

	#ifdef G_OS_WIN32
	uri = g_strdup (albumart_path);
	#else
	uri = g_filename_to_uri (albumart_path, NULL, NULL);
	#endif

	open_url(uri, pragha_application_get_window (pragha));
	g_free (uri);
}

static void
pragha_edit_tags_dialog_response (GtkWidget         *dialog,
                                  gint               response_id,
                                  PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaPlaylist *playlist;
	PraghaMusicobject *nmobj, *bmobj;
	PraghaTagger *tagger;
	gint changed = 0;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, pragha_application_get_window(pragha));
		return;
	}

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if(changed) {
			nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

			backend = pragha_application_get_backend (pragha);

			if(pragha_backend_get_state (backend) != ST_STOPPED) {
				PraghaMusicobject *current_mobj = pragha_backend_get_musicobject (backend);
				if (pragha_musicobject_compare (nmobj, current_mobj) == 0) {
					toolbar = pragha_application_get_toolbar (pragha);
					playlist = pragha_application_get_playlist (pragha);

					/* Update public current song */
					pragha_update_musicobject_change_tag (current_mobj, changed, nmobj);

					/* Update current song on playlist */
					pragha_playlist_update_current_track(playlist, changed, nmobj);

					/* Update current song on backend */
					bmobj = g_object_ref(pragha_backend_get_musicobject(backend));
					pragha_update_musicobject_change_tag(bmobj, changed, nmobj);
					g_object_unref(bmobj);

					pragha_toolbar_set_title(toolbar, current_mobj);
				}
			}

			if(G_LIKELY(pragha_musicobject_is_local_file (nmobj))) {
				tagger = pragha_tagger_new();
				pragha_tagger_add_file (tagger, pragha_musicobject_get_file(nmobj));
				pragha_tagger_set_changes(tagger, nmobj, changed);
				pragha_tagger_apply_changes (tagger);
				g_object_unref(tagger);
			}
		}
	}
	gtk_widget_destroy (dialog);
}

void
pragha_playback_edit_current_track (PraghaApplication *pragha)
{
	PraghaBackend *backend;
	GtkWidget *dialog;

	backend = pragha_application_get_backend (pragha);

	if(pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	dialog = pragha_tags_dialog_new();
	gtk_window_set_transient_for (GTK_WINDOW(dialog),
		GTK_WINDOW(pragha_application_get_window (pragha)));

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_edit_tags_dialog_response), pragha);

	pragha_tags_dialog_set_musicobject (PRAGHA_TAGS_DIALOG(dialog),
	                                    pragha_backend_get_musicobject (backend));
	
	gtk_widget_show (dialog);
}

void
pragha_playback_seek_fraction (GObject *object, gdouble fraction, PraghaApplication *pragha)
{
	gint seek = 0, length = 0;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

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
