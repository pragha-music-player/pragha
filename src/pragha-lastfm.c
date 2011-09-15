/*************************************************************************/
/* Copyright (C) 2011 matias <mati86dl@gmail.com>			 */
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

#include <stdio.h>
#include <pthread.h>
#include "pragha.h"

#define WAIT_UPDATE 5

#ifdef HAVE_LIBCLASTFM
gint find_nocase_artist_db(const gchar *artist, struct con_win *cwin)
{
	gint artist_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM ARTIST WHERE name = '%s' COLLATE NOCASE;", artist);
	if (exec_sqlite_query(query, cwin, &result)) {
		if(result.no_rows)
			artist_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return artist_id;
}

gint try_add_track_from_db(gchar *artist, gchar *title, struct con_win *cwin)
{
	gchar *query, *s_artist, *s_title;
	gint location_id = 0, artist_id = 0;
	struct db_result result;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_LASTFM, "Try to add: %s - %s", artist, title);

	s_artist = sanitize_string_sqlite3(artist);
	s_title = sanitize_string_sqlite3(title);

	artist_id = find_nocase_artist_db(s_artist, cwin);
	if(artist_id == 0)
		goto bad;

	query = g_strdup_printf("SELECT location FROM TRACK WHERE TRACK.title = \"%s\" COLLATE NOCASE AND TRACK.artist = %d;", title, artist_id);
	exec_sqlite_query(query, cwin, &result);
	if (!result.no_rows)
		goto bad;

	location_id = atoi(result.resultp[result.no_columns]);

	mobj = new_musicobject_from_db(location_id, cwin);

	append_current_playlist(mobj, cwin);
bad:
	g_free(s_artist);
	g_free(s_title);

	return location_id;
}

void lastfm_import_xspf_action (GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkFileFilter *media_filter;
	XMLNode *xml = NULL, *xi, *xc, *xt;
	gchar *contents, *summary;
	gint try = 0, added = 0;
	GFile *file;
	gsize size;

	dialog = gtk_file_chooser_dialog_new (_("Import a XSPF playlist"),
				      GTK_WINDOW(cwin->mainwindow),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	media_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(GTK_FILE_FILTER(media_filter), _("Supported media"));
	gtk_file_filter_add_mime_type(GTK_FILE_FILTER(media_filter), "application/xspf+xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(media_filter));

	if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy (dialog);
		return;
	}

	file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

	if (!g_file_load_contents (file, NULL, &contents, &size, NULL, NULL)) {
		goto out;
    	}

	if (g_utf8_validate (contents, -1, NULL) == FALSE) {
		gchar *fixed;
		fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
		if (fixed != NULL) {
			g_free (contents);
			contents = fixed;
		}
	}

	xml = tinycxml_parse(contents);

	xi = xmlnode_get(xml,CCA { "playlist","trackList","track",NULL},NULL,NULL);
	for(;xi;xi= xi->next) {
		try++;
		xt = xmlnode_get(xi,CCA {"track","title",NULL},NULL,NULL);
		xc = xmlnode_get(xi,CCA {"track","creator",NULL},NULL,NULL);

		if (xt && xc && try_add_track_from_db (xc->content, xt->content, cwin))
			added++;
	}

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);

	set_status_message(summary, cwin);

	gtk_widget_destroy (dialog);
	xmlnode_free(xml);
	g_free (contents);
	g_free(summary);
out:
	g_object_unref (file);
}

void *do_lastfm_add_favorites_action (gpointer data)
{
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track;
	gint i = 1, try = 0, added = 0;
	gchar *summary = NULL;

	struct con_win *cwin = data;

	while (LASTFM_user_get_loved_tracks(cwin->clastfm->session_id,
			cwin->cpref->lw.lastfm_user,
			i, &results)) {
		gdk_threads_enter();
		for(li=results; li; li=li->next) {
			track = li->data;
			try++;
			if (try_add_track_from_db (track->artist, track->name, cwin))
				added++;
		}
		gdk_threads_leave();
		i++;
		LASTFM_free_track_info_list (results);
	}

	if(try > 0)
		summary = g_strdup_printf(_("Added %d songs of the last %d loved on Last.fm."), added, try);
	else
		summary = g_strdup_printf(_("You had no favorite songs on Last.fm."));

	gdk_threads_enter();
	set_status_message(summary, cwin);
	gdk_threads_leave();

	g_free(summary);

	return NULL;
}

void lastfm_add_favorites_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_LASTFM, "Add Favorites action");

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}
	pthread_create (&tid, NULL, do_lastfm_add_favorites_action, cwin);
}


void *do_lastfm_get_similar_action (gpointer data)
{
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track = NULL;
	gint rv, added, try;
	gchar *summary = NULL;

	struct con_win *cwin = data;

	rv = LASTFM_track_get_similar(cwin->clastfm->session_id,
			cwin->cstate->curr_mobj->tags->title,
			cwin->cstate->curr_mobj->tags->artist,
			50, &results);

	gdk_threads_enter();

	if(rv != LASTFM_STATUS_OK) {
		set_status_message("Error searching similar songs on Last.fm.", cwin);
		gdk_threads_leave();
		return NULL;
	}

	for(li=results, added=0, try=0 ; li; li=li->next) {
		track = li->data;
		try++;
		if (try_add_track_from_db (track->artist, track->name, cwin))
			added++;
	}
	if(try > 0)
		summary = g_strdup_printf(_("Added %d songs of %d sugested from Last.fm."), added, try);
	else
		summary = g_strdup_printf(_("Last.fm not suggest any similar song."));

	set_status_message(summary, cwin);

	gdk_threads_leave();

	LASTFM_free_track_info_list (results);
	g_free(summary);

	return NULL;
}

void lastfm_get_similar_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	CDEBUG(DBG_LASTFM, "Get similar action");

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}
	pthread_create (&tid, NULL, do_lastfm_get_similar_action, cwin);
}

void *do_lastfm_get_album_art (gpointer data)
{
	GError *error = NULL;
	GdkPixbuf *album_art = NULL;
	GdkCursor *cursor;
	gchar *album_art_path = NULL;

	struct con_win *cwin = data;
	
	LASTFM_ALBUM_INFO *album = NULL;

	album_art_path = g_strdup_printf("%s/%s - %s.jpeg",
					cwin->cpref->cache_album_art_folder,
					cwin->cstate->curr_mobj->tags->artist,
					cwin->cstate->curr_mobj->tags->album);

	if (g_file_test(album_art_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE)
		goto exists;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	album = LASTFM_album_get_info (cwin->clastfm->session_id, cwin->cstate->curr_mobj->tags->artist, cwin->cstate->curr_mobj->tags->album);

	if(!album) {
		gdk_threads_enter ();
		set_status_message(_("Album art not found."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		goto exists;
	}

	gdk_threads_enter ();
	if(album->image)
		album_art = vgdk_pixbuf_new_from_memory(album->image, album->image_size);

	if (album_art) {
		gdk_pixbuf_save(album_art, album_art_path, "jpeg", &error, "quality", "100", NULL);

		update_album_art(cwin->cstate->curr_mobj, cwin);
		g_object_unref(G_OBJECT(album_art));
	}
	else {
		set_status_message(_("Album art not found."), cwin);
	}

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
	gdk_threads_leave ();

	LASTFM_free_album_info(album);

exists:
	g_free(album_art_path);

	return NULL;
}

void lastfm_get_album_art_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (!cwin->cpref->show_album_art)
		return;

	if (NULL == cwin->cstate->curr_mobj->tags->artist || NULL == cwin->cstate->curr_mobj->tags->album);
		return;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_get_album_art, cwin);
}

void *do_lastfm_love (gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Love thread");

	rv = LASTFM_track_love (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->artist);

	if (rv != 0) {
		gdk_threads_enter ();
		set_status_message(_("Love song on Last.fm failed."), cwin);
		gdk_threads_leave ();
	}

	return NULL;
}

void lastfm_track_love_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_LASTFM, "Love Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_love, cwin);
}

void *do_lastfm_unlove (gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Unlove thread");

	rv = LASTFM_track_love (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->artist);

	if (rv != 0) {
		gdk_threads_enter ();
		set_status_message(_("Unlove song on Last.fm failed."), cwin);
		gdk_threads_leave ();
	}

	return NULL;
}

void lastfm_track_unlove_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_LASTFM, "Unlove Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_unlove, cwin);
}

void *do_lastfm_scrob (gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Scrobbler thread");

	rv = LASTFM_track_scrobble (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->album,
		cwin->cstate->curr_mobj->tags->artist,
		cwin->clastfm->playback_started,
		cwin->cstate->curr_mobj->tags->length,
		cwin->cstate->curr_mobj->tags->track_no,
		0, NULL);

	gdk_threads_enter ();
	if (rv != 0)
		set_status_message("Last.fm submission failed", cwin);
	else
		set_status_message("Track scrobbled on Last.fm", cwin);
	gdk_threads_leave ();


	return NULL;
}

gboolean lastfm_scrob_handler(gpointer data)
{
	pthread_t tid;
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Scrobbler Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return FALSE;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return FALSE;
	}

	pthread_create(&tid, NULL, do_lastfm_scrob, cwin);

	return FALSE;
}

void *do_lastfm_now_playing (gpointer data)
{
	gint rv;
	gchar *album_art_path = NULL;
	GdkPixbuf *album_art = NULL;
	LASTFM_ALBUM_INFO *album = NULL;

	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Update now playing thread");

	rv = LASTFM_track_update_now_playing (cwin->clastfm->session_id,
		cwin->cstate->curr_mobj->tags->title,
		cwin->cstate->curr_mobj->tags->album,
		cwin->cstate->curr_mobj->tags->artist,
		cwin->cstate->curr_mobj->tags->length,
		cwin->cstate->curr_mobj->tags->track_no,
		0);

	if (rv != 0) {
		gdk_threads_enter ();
		set_status_message(_("Update current song on Last.fm failed."), cwin);
		gdk_threads_leave ();
	}

	if ((cwin->cpref->lw.lastfm_get_album_art == TRUE) && (cwin->cstate->curr_mobj->tags->album != NULL)) {
		album_art_path = g_strdup_printf("%s/%s - %s.jpeg",
						cwin->cpref->cache_album_art_folder,
						cwin->cstate->curr_mobj->tags->artist,
						cwin->cstate->curr_mobj->tags->album);

		if (g_file_test(album_art_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE)
			goto exists;

		album = LASTFM_album_get_info (cwin->clastfm->session_id, cwin->cstate->curr_mobj->tags->artist, cwin->cstate->curr_mobj->tags->album);

		if(album) {
			gdk_threads_enter ();
			if(album->image)
				album_art = vgdk_pixbuf_new_from_memory(album->image, album->image_size);

			if (album_art) {
				gdk_pixbuf_save(album_art, album_art_path, "jpeg", NULL, "quality", "100", NULL);

				update_album_art(cwin->cstate->curr_mobj, cwin);
				g_object_unref(G_OBJECT(album_art));
			}
			else {
				set_status_message(_("Album art not found."), cwin);
			}
			gdk_threads_leave ();
		}
	}
exists:
	g_free(album_art_path);
	
	return NULL;
}

gboolean lastfm_now_playing_handler (gpointer data)
{
	pthread_t tid;
	struct con_win *cwin = data;
	gint length;

	CDEBUG(DBG_LASTFM, "Update now playing Handler");

	if(cwin->cstate->state == ST_STOPPED)
		return FALSE;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return FALSE;
	}

	if (NULL == cwin->cstate->curr_mobj->tags->artist || NULL == cwin->cstate->curr_mobj->tags->title);
		pthread_create(&tid, NULL, do_lastfm_now_playing, cwin);

	/* Kick the lastfm scrobbler on
	 * Note: Only scrob if tracks is more than 30s.
	 * and scrob when track is at 50% or 4mins, whichever comes
	 * first */

	if(cwin->cstate->curr_mobj->tags->length < 30)
		return FALSE;

	if((cwin->cstate->curr_mobj->tags->length / 2) > (240 - WAIT_UPDATE)) {
		length = 240 - WAIT_UPDATE;
	}
	else {
		length = (cwin->cstate->curr_mobj->tags->length / 2);
	}

	cwin->clastfm->lastfm_handler_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, length,
			lastfm_scrob_handler, cwin, NULL);

	return FALSE;
}

void update_lastfm (struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Update lastfm thread state");

	if(cwin->clastfm->lastfm_handler_id)
		g_source_remove(cwin->clastfm->lastfm_handler_id);

	if(cwin->cstate->state != ST_PLAYING)
		return;

	time(&cwin->clastfm->playback_started);

	cwin->clastfm->lastfm_handler_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, WAIT_UPDATE,
			lastfm_now_playing_handler, cwin, NULL);
}

void *do_init_lastfm (gpointer data)
{
	gint rv;
	struct con_win *cwin = data;

	cwin->clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	rv = LASTFM_login (cwin->clastfm->session_id, cwin->cpref->lw.lastfm_user, cwin->cpref->lw.lastfm_pass);

	if(rv != LASTFM_STATUS_OK) {
		LASTFM_dinit(cwin->clastfm->session_id);
		cwin->clastfm->session_id = NULL;

		g_critical("Unable to login on Lastfm");
	}

	return NULL;
}

gint init_lastfm (struct con_win *cwin)
{
	pthread_t tid;

	if (cwin->cpref->lw.lastfm_support) {
		CDEBUG(DBG_INFO, "Initializing LASTFM");

		pthread_create (&tid, NULL, do_init_lastfm, cwin);
	}
	return 0;
}
#endif
