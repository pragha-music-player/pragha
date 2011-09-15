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

	glyr_init_query(&q);
	glyr_opt_type(&q, GLYR_GET_ARTISTBIO);

	glyr_opt_artist(&q, cwin->cstate->curr_mobj->tags->artist);
	glyr_opt_lang (&q, ISO_639_1);

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
	glyr_destroy_query(&q);

	return NULL;
}

void related_get_artist_info_action (GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	CDEBUG(DBG_LASTFM, "Get Artist info Action");

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

	glyr_init_query(&q);
	glyr_opt_type(&q, GLYR_GET_LYRICS);

	glyr_opt_artist(&q, artist);
	glyr_opt_title(&q, title);
	
	GlyrMemCache *head = glyr_get(&q, &err,NULL);

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
	glyr_destroy_query(&q);

	g_free(artist);
	g_free(title);

	return NULL;
}

void related_get_lyric_action(GtkAction *action, struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	pthread_create(&tid, NULL, do_get_lyrics_dialog, cwin);
}

int uninit_glyr_related (struct con_win *cwin)
{
	glyr_cleanup ();

	return 0;
}

int init_glyr_related (struct con_win *cwin)
{
	glyr_init();

	return 0;
}

#endif