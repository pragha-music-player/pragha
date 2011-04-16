/*
   lastfm_helper.c - Helper functions for handling liblastfm stuff
   See README for Copyright and License
*/

#include <stdio.h>
#include <pthread.h>
#include <pragha.h>

static void *do_lastfm_scrob(gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Scrobbler");

	rv = LASTFM_track_scrobble (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->album,
		cwin->cstate->curr_mobj->tags->artist,
		cwin->clastfm->playback_started,
		cwin->cstate->curr_mobj->tags->length,
		cwin->cstate->curr_mobj->tags->track_no,
		0);

	if (rv != 0)
		CDEBUG(DBG_LASTFM, "Last.fm submission failed");

	return NULL;
}

static gboolean lastfm_scrob_handler(gpointer data)
{
	pthread_t tid;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Scrobbler Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return FALSE;

	pthread_create(&tid, NULL, do_lastfm_scrob, cwin);

	return FALSE;
}

static void *do_lastfm_now_playing(gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Now playing on Lastfm");

	rv = LASTFM_track_update_now_playing (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->album,
		cwin->cstate->curr_mobj->tags->artist,
		cwin->cstate->curr_mobj->tags->length,
		cwin->cstate->curr_mobj->tags->track_no,
		0);

	if (rv != 0)
		CDEBUG(DBG_LASTFM, "Last.fm now playing failed");

	return NULL;
}

static gboolean lastfm_now_playing_handler(gpointer data)
{
	pthread_t tid;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Now player Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return FALSE;

	pthread_create(&tid, NULL, do_lastfm_now_playing, cwin);

	return FALSE;
}

void update_lastfm(struct con_win *cwin)
{
	int length;

	CDEBUG(DBG_LASTFM, "Update lastfm");

	if(cwin->clastfm->lastfm_handler_id)
		g_source_remove(cwin->clastfm->lastfm_handler_id);

	/* Kick the lastfm scrobbler on
	 * Note: Only scrob if tracks is more than 30s.
	 * and scrob when track is at 50% or 4mins, whichever comes
	 * first */

	if(cwin->cstate->state != ST_PLAYING)
		return;
	else
		lastfm_now_playing_handler(cwin);

	if(cwin->cstate->curr_mobj->tags->length < 30) return;

	if((cwin->cstate->curr_mobj->tags->length / 2) > 240) {
		length = 240;
	}
	else {
		length = (cwin->cstate->curr_mobj->tags->length / 2);
	}

	cwin->clastfm->lastfm_handler_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, length,
			lastfm_scrob_handler, cwin, NULL);
}
