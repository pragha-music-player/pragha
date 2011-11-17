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

#include "pragha.h"
#include <pthread.h>

#define ISO_639_1 _("en")

#ifdef HAVE_LIBGLYR
void *do_get_album_art (gpointer data)
{
	GError *error = NULL;
	GdkPixbuf *album_art = NULL;
	gchar *album_art_path = NULL, *artist = NULL, *album = NULL;
	GdkCursor *cursor;
	GlyrQuery q;
	GLYR_ERROR err;

	GlyrMemCache *head = NULL;

	struct con_win *cwin = data;

	CDEBUG(DBG_INFO, "Get album art thread");

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);
	album = g_strdup(cwin->cstate->curr_mobj->tags->album);

	album_art_path = g_strdup_printf("%s/album-%s-%s.jpeg",
					cwin->cpref->cache_folder,
					artist,
					album);

	if (g_file_test(album_art_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE)
		goto exists;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_COVERART);

	glyr_opt_artist(&q, cwin->cstate->curr_mobj->tags->artist);
	glyr_opt_album(&q, cwin->cstate->curr_mobj->tags->album);

	glyr_opt_from(&q, "all;-picsearch;-google");

	head = glyr_get(&q, &err, NULL);

	if(head == NULL) {
		g_warning(glyr_strerror(err));
		gdk_threads_enter ();
		set_status_message(_("Album art not found."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		goto bad;
	}

	gdk_threads_enter ();
	if(head->data)
		album_art = vgdk_pixbuf_new_from_memory(head->data, head->size);

	if (album_art) {
		gdk_pixbuf_save(album_art, album_art_path, "jpeg", &error, "quality", "100", NULL);

		if((0 == g_strcmp0(artist, cwin->cstate->curr_mobj->tags->artist)) &&
		   (0 == g_strcmp0(album, cwin->cstate->curr_mobj->tags->album)))
			update_album_art(cwin->cstate->curr_mobj, cwin);

		g_object_unref(G_OBJECT(album_art));
	}
	else {
		set_status_message(_("Album art not found."), cwin);
	}

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
	gdk_threads_leave ();

	glyr_free_list(head);

bad:
	glyr_query_destroy(&q);
exists:
	g_free(album_art_path);
	g_free(artist);
	g_free(album);

	return NULL;
}

void *do_get_album_art_idle (gpointer data)
{
	GError *error = NULL;
	GdkPixbuf *album_art = NULL;
	gchar *album_art_path = NULL, *artist = NULL, *album = NULL;
	GlyrQuery q;
	GLYR_ERROR err;

	GlyrMemCache *head = NULL;

	struct con_win *cwin = data;

	CDEBUG(DBG_INFO, "Get album art idle");

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);
	album = g_strdup(cwin->cstate->curr_mobj->tags->album);

	album_art_path = g_strdup_printf("%s/album-%s-%s.jpeg",
					cwin->cpref->cache_folder,
					artist,
					album);

	if (g_file_test(album_art_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE)
		goto exists;

	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_COVERART);

	glyr_opt_artist(&q, artist);
	glyr_opt_album(&q, album);

	glyr_opt_from(&q, "all;-picsearch;-google");

	head = glyr_get(&q, &err, NULL);

	if(head == NULL) {
		g_warning(glyr_strerror(err));
		goto no_albumart;
	}

	gdk_threads_enter ();
	if(head->data)
		album_art = vgdk_pixbuf_new_from_memory(head->data, head->size);

	if (album_art) {
		gdk_pixbuf_save(album_art, album_art_path, "jpeg", &error, "quality", "100", NULL);

		if((0 == g_strcmp0(artist, cwin->cstate->curr_mobj->tags->artist)) &&
		   (0 == g_strcmp0(album, cwin->cstate->curr_mobj->tags->album)))
			update_album_art(cwin->cstate->curr_mobj, cwin);

		g_object_unref(G_OBJECT(album_art));
	}
	gdk_threads_leave ();

	glyr_free_list(head);

no_albumart:
	glyr_query_destroy(&q);
exists:
	g_free(album_art_path);
	g_free(artist);
	g_free(album);

	return NULL;
}

void related_get_album_art_handler (struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_INFO, "Get album art handler");

	if (cwin->cstate->state == ST_STOPPED)
		return;

	if (cwin->cpref->show_album_art == FALSE)
		return;

	if ((strlen(cwin->cstate->curr_mobj->tags->artist) == 0) ||
	    (strlen(cwin->cstate->curr_mobj->tags->album) == 0))
		return;

	pthread_create(&tid, NULL, do_get_album_art_idle, cwin);

	return;
}

void related_get_album_art_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_INFO, "Get album art action");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (cwin->cpref->show_album_art == FALSE)
		return;

	if ((strlen(cwin->cstate->curr_mobj->tags->artist) == 0) ||
	    (strlen(cwin->cstate->curr_mobj->tags->album) == 0))
		return;

	pthread_create(&tid, NULL, do_get_album_art, cwin);
}

/* Handler for 'Artist info' action in the Tools menu */
void *do_get_artist_info (gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *subtitle_header = NULL;
	GdkCursor *cursor;
	GlyrQuery q;
	GLYR_ERROR err;

	GlyrMemCache *head = NULL;

	struct con_win *cwin = data;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_ARTISTBIO);

	glyr_opt_artist(&q, cwin->cstate->curr_mobj->tags->artist);

	glyr_opt_lang (&q, ISO_639_1);
	glyr_opt_lang_aware_only (&q, TRUE);

	glyr_opt_lookup_db(&q, cwin->cdbase->cache_db);
	glyr_opt_db_autowrite(&q, TRUE);

	head = glyr_get(&q, &err, NULL);

	gdk_threads_enter ();

	if(head == NULL) {
		set_status_message(_("Artist information not found."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		goto bad;
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

	gtk_text_buffer_set_text (buffer, head->data, -1);

	dialog = gtk_dialog_new_with_buttons(_("Artist info"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	subtitle_header = g_strdup_printf(_("%s <small><span weight=\"light\">thanks to</span></small> %s"), cwin->cstate->curr_mobj->tags->artist, head->prov);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (subtitle_header, NULL, cwin);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	gdk_threads_leave ();

	glyr_free_list(head);

	g_free(subtitle_header);
bad:
	glyr_query_destroy(&q);

	return NULL;
}

void related_get_artist_info_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_INFO, "Get Artist info Action");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	pthread_create(&tid, NULL, do_get_artist_info, cwin);
}

void *do_get_lyrics_dialog (gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *title_header = NULL, *artist = NULL, *title = NULL;
	GdkCursor *cursor;
	GlyrQuery q;
	GLYR_ERROR err;

	struct con_win *cwin = data;

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);
	title = g_strdup(cwin->cstate->curr_mobj->tags->title);

	gdk_threads_enter ();
	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);
	gdk_threads_leave ();

	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_LYRICS);

	glyr_opt_artist(&q, artist);
	glyr_opt_title(&q, title);

	glyr_opt_lookup_db(&q, cwin->cdbase->cache_db);
	glyr_opt_db_autowrite(&q, TRUE);

	GlyrMemCache *head = glyr_get(&q, &err, NULL);

	if(head == NULL) {
		gdk_threads_enter ();
		set_status_message(_("Error searching Lyric."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();

		goto bad;
	}

	gdk_threads_enter ();
	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_set_text (buffer, head->data, -1);

	dialog = gtk_dialog_new_with_buttons(head->prov,
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	title_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), title, artist);
	header = sokoke_xfce_header_new (title_header, NULL, cwin);
	g_free(title_header);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	gdk_threads_leave ();

	glyr_free_list(head);

bad:
	glyr_query_destroy(&q);

	g_free(artist);
	g_free(title);

	return NULL;
}

void related_get_lyric_action(GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	pthread_create(&tid, NULL, do_get_lyrics_dialog, cwin);
}

int uninit_glyr_related (struct con_win *cwin)
{
	glyr_db_destroy(cwin->cdbase->cache_db);
	glyr_cleanup ();

	return 0;
}

int init_glyr_related (struct con_win *cwin)
{
	glyr_init();

	cwin->cdbase->cache_db = glyr_db_init(cwin->cpref->cache_folder);

	return 0;
}
#endif

gboolean update_related_handler (gpointer data)
{
#if HAVE_LIBCLASTFM || HAVE_LIBGLYR
	struct con_win *cwin = data;
#endif
	CDEBUG(DBG_INFO, "Updating Lastm and getting the cover art depending preferences");

#ifdef HAVE_LIBCLASTFM
	if (cwin->cpref->lw.lastfm_support)
		lastfm_now_playing_handler(cwin);
#endif
#ifdef HAVE_LIBGLYR
	if (cwin->cpref->get_album_art)
		related_get_album_art_handler(cwin);
#endif
	return FALSE;
}

void update_related_state (struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Configuring thread to update Lastfm and get the cover art");

	if(cwin->related_timeout_id)
		g_source_remove(cwin->related_timeout_id);

	if(cwin->cstate->state != ST_PLAYING)
		return;

	if(cwin->cstate->curr_mobj->file_type == FILE_HTTP)
		return;

	#ifdef HAVE_LIBCLASTFM
	if (cwin->clastfm->status == LASTFM_STATUS_OK)
		time(&cwin->clastfm->playback_started);
	#endif

	cwin->related_timeout_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, WAIT_UPDATE,
			update_related_handler, cwin, NULL);
}


