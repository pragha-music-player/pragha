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
#include <pragha.h>

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
	GdkPixbuf *album_art = NULL, *scaled_album_art = NULL, *pix_frame = NULL, *scaled_frame;
	GdkCursor *cursor;

	struct con_win *cwin = data;
	
	LASTFM_ALBUM_INFO *album = NULL;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	album = LASTFM_album_get_info (cwin->clastfm->session_id, cwin->cstate->curr_mobj->tags->artist, cwin->cstate->curr_mobj->tags->album);

	if(!album) {
		gdk_threads_enter ();
		set_status_message(_("Album art not found on Last.fm."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		return NULL;
	}

	gdk_threads_enter ();
	if(album->image)
		album_art = vgdk_pixbuf_new_from_memory(album->image, album->image_size);

	if (album_art) {
		scaled_album_art = gdk_pixbuf_scale_simple (album_art, 112, 112, GDK_INTERP_BILINEAR);

		pix_frame = gdk_pixbuf_new_from_file (PIXMAPDIR"/cover.png", &error);
		gdk_pixbuf_copy_area(scaled_album_art, 0 ,0 ,112 ,112, pix_frame, 12, 8);

		scaled_frame = gdk_pixbuf_scale_simple (pix_frame,
							cwin->cpref->album_art_size,
							cwin->cpref->album_art_size,
							GDK_INTERP_BILINEAR);
		if (cwin->album_art) {
			gtk_image_clear(GTK_IMAGE(cwin->album_art));
			gtk_image_set_from_pixbuf(GTK_IMAGE(cwin->album_art), scaled_frame);
		}
		else {
			cwin->album_art = gtk_image_new_from_pixbuf(scaled_frame);
			gtk_container_add(GTK_CONTAINER(cwin->album_art_frame),
					  GTK_WIDGET(cwin->album_art));
			gtk_widget_show_all(cwin->album_art_frame);
		}
		g_object_unref(G_OBJECT(album_art));
		g_object_unref(G_OBJECT(scaled_album_art));
		g_object_unref(G_OBJECT(scaled_frame));
		g_object_unref(G_OBJECT(pix_frame));
	}
	else {
		set_status_message(_("Album art not found on Last.fm."), cwin);
	}

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
	gdk_threads_leave ();

	LASTFM_free_album_info(album);

	return NULL;
}

void lastfm_get_album_art_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (!cwin->cpref->show_album_art)
		return;

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_get_album_art, cwin);
}

/* Handler for 'Artist info' action in the Tools menu */
void *do_lastfm_get_artist_info (gpointer data)
{
	GtkWidget *dialog;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextTag *btag;
	gchar *value = NULL, *playcount = NULL, *wiki = NULL;
	gchar *summary_helper = NULL, *summary = NULL;
	GtkWidget *header, *view, *frame, *scrolled;
	gint i, result;
	GdkCursor *cursor;

	LASTFM_ARTIST_INFO *artist = NULL;

	struct con_win *cwin = data;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	artist = LASTFM_artist_get_info (cwin->clastfm->session_id, cwin->cstate->curr_mobj->tags->artist, ISO_639_1);

	gdk_threads_enter ();

	if(!artist) {
		set_status_message(_("Artist information not found on Last.fm."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		return NULL;
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &iter, 0);

	btag = gtk_text_buffer_create_tag (buffer, NULL, "weight", PANGO_WEIGHT_BOLD, NULL);

	playcount = g_strdup_printf("%d", artist->playcount);
	gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Playcount:"), -1, btag, NULL);
	value = g_strdup_printf (" %s\n\n", playcount);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
	g_free (playcount);
	g_free (value);

	gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Summary:"), -1, btag, NULL);

	if(artist->summary && strncmp (artist->summary, "<![CDATA[", 9) == 0) {
		summary_helper = artist->summary + 9;
		summary = g_strndup (summary_helper, strlen (summary_helper) - 3);
	}
	else {
		summary = g_strdup(artist->summary);
	}

	value = g_strdup_printf (" %s\n\n", summary);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
	g_free (summary);
	g_free (value);

	if(artist->similar != NULL) {
		gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Similar artists:"), -1, btag, NULL);

		for(i=0; artist->similar[i]; i++){
			value = g_strdup_printf ("\n\t%i: %s", i, artist->similar[i]);
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
			g_free (value);
		}
	}

	dialog = gtk_dialog_new_with_buttons(_("Lastfm artist info"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("View more..."), GTK_RESPONSE_HELP);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (artist->name, "lastfm", cwin);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
		case GTK_RESPONSE_HELP:
			wiki = g_strdup_printf("http://www.lastfm.es/music/%s/+wiki", artist->name);
			open_url (cwin, wiki);
			g_free (wiki);
			break;
		case GTK_RESPONSE_OK:
			break;
		default:
			break;
	}

	gtk_widget_destroy(dialog);
	gdk_threads_leave ();

	LASTFM_free_artist_info(artist);

	return NULL;
}

void lastfm_artist_info_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	CDEBUG(DBG_LASTFM, "Get Artist info Action");

	if (cwin->clastfm->session_id == NULL) {
		set_status_message(_("No connection Last.fm has been established."), cwin);
		return;
	}
	pthread_create(&tid, NULL, do_lastfm_get_artist_info, cwin);

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
		set_status_message(_("Love song on Last.fm failed"), cwin);
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
		set_status_message(_("Unlove song on Last.fm failed"), cwin);
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
		set_status_message(_("Update current song on Last.fm failed"), cwin);
		gdk_threads_leave ();
	}
	
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
