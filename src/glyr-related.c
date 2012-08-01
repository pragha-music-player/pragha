/*************************************************************************/
/* Copyright (C) 2011-2012 matias <mati86dl@gmail.com>			 */
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

#define ISO_639_1 _("en")

#ifdef HAVE_LIBGLYR

typedef struct
{
	struct con_win	*cwin;
	GlyrQuery	query;
	GlyrMemCache	*head;
}
glyr_struct;

gboolean
show_generic_related_text_info_dialog (gpointer data)
{
	GtkWidget *dialog, *header, *view, *scrolled;
	GtkTextBuffer *buffer;
	gchar *artist = NULL, *title = NULL, *provider = NULL;
	gchar *title_header = NULL, *subtitle_header = NULL;

	glyr_struct *glyr_info = data;

	artist = g_strdup(glyr_info->query.artist);
	title = g_strdup(glyr_info->query.title);
	provider = g_strdup(glyr_info->head->prov);

	if(glyr_info->head->type == GLYR_TYPE_LYRICS) {
		title_header =  g_strdup_printf(_("Lyrics thanks to %s"), provider);
		subtitle_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), title, artist);
	}
	else {
		title_header =  g_strdup_printf(_("Artist info"));
		subtitle_header = g_strdup_printf(_("%s <small><span weight=\"light\">thanks to</span></small> %s"), artist, provider);
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_set_text (buffer, glyr_info->head->data, -1);

	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);

	gtk_container_set_border_width (GTK_CONTAINER (scrolled), 8);

	dialog = gtk_dialog_new_with_buttons(title_header,
					     GTK_WINDOW(glyr_info->cwin->mainwindow),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (subtitle_header, NULL, glyr_info->cwin);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scrolled, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	glyr_free_list(glyr_info->head);
	glyr_query_destroy(&glyr_info->query);
	g_slice_free(glyr_struct, glyr_info);

	g_free(title_header);
	g_free(subtitle_header);
	g_free(artist);
	g_free(title);
	g_free(provider);

	return FALSE;
}

gpointer
get_related_text_info_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	set_watch_cursor_on_thread(glyr_info->cwin);

	head = glyr_get(&glyr_info->query, &error, NULL);

	if(head != NULL) {
		glyr_info->head = head;
		gdk_threads_add_idle(show_generic_related_text_info_dialog, glyr_info);

		remove_watch_cursor_on_thread(NULL, glyr_info->cwin);
	}
	else {
		remove_watch_cursor_on_thread((glyr_info->query.type == GLYR_GET_LYRICS) ? _("Lyrics not found.") : _("Artist information not found."),
					       glyr_info->cwin);
		g_warning("Error searching song info: %s", glyr_strerror(error));

		glyr_query_destroy(&glyr_info->query);
		g_slice_free(glyr_struct, glyr_info);
	}

	return NULL;
}

void
configure_and_launch_get_text_info_dialog(GLYR_GET_TYPE type, gchar *artist, gchar *title, struct con_win *cwin)
{
	glyr_struct *glyr_info;
	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init(&glyr_info->query);

	glyr_opt_type(&glyr_info->query, type);

	if(type == GLYR_GET_LYRICS) {
		glyr_opt_artist(&glyr_info->query, artist);
		glyr_opt_title(&glyr_info->query, title);
	}
	else {
		glyr_opt_artist(&glyr_info->query, artist);

		glyr_opt_lang (&glyr_info->query, ISO_639_1);
		glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
	}

	glyr_opt_lookup_db(&glyr_info->query, cwin->cache_db);
	glyr_opt_db_autowrite(&glyr_info->query, TRUE);

	glyr_info->cwin = cwin;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Glyr get text", get_related_text_info_idle_func, glyr_info);
	#else
	g_thread_create(get_related_text_info_idle_func, glyr_info, FALSE, NULL);
	#endif
}

void related_get_artist_info_action (GtkAction *action, struct con_win *cwin)
{
	gchar *artist = NULL;

	CDEBUG(DBG_INFO, "Get Artist info Action");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if (strlen(cwin->cstate->curr_mobj->tags->artist) == 0)
		return;

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);

	configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, cwin);

	g_free(artist);
}

void related_get_lyric_action(GtkAction *action, struct con_win *cwin)
{
	gchar *artist = NULL, *title = NULL;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	if(cwin->cstate->state == ST_STOPPED)
		return;

	if ((strlen(cwin->cstate->curr_mobj->tags->artist) == 0) ||
	    (strlen(cwin->cstate->curr_mobj->tags->title) == 0))
		return;

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);
	title = g_strdup(cwin->cstate->curr_mobj->tags->title);

	configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, cwin);

	g_free(artist);
	g_free(title);
}

void
related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin)
{
	gchar *artist = NULL, *title = NULL;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_INFO, "Get lyrics Action of current playlist selection.");

	mobj = get_selected_musicobject(cwin);

	if ((strlen(mobj->tags->artist) == 0) ||
	    (strlen(mobj->tags->title) == 0))
		return;

	artist = g_strdup(mobj->tags->artist);
	title = g_strdup(mobj->tags->title);

	configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, cwin);

	g_free(artist);
	g_free(title);
}

void
related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin)
{
	gchar *artist = NULL;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_INFO, "Get Artist info Action of current playlist selection");

	mobj = get_selected_musicobject(cwin);

	if (strlen(mobj->tags->artist) == 0)
		return;

	artist = g_strdup(mobj->tags->artist);

	configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, cwin);

	g_free(artist);
}

/* Dowload album art and save it in cache.*/

gboolean
update_downloaded_album_art (gpointer data)
{
	gchar *artist = NULL, *album = NULL, *album_art_path = NULL;
	GdkPixbuf *album_art = NULL;
	GError *error = NULL;

	glyr_struct *glyr_info = data;
	struct con_win *cwin = glyr_info->cwin;

	artist = g_strdup(glyr_info->query.artist);
	album = g_strdup(glyr_info->query.album);

	album_art_path = g_strdup_printf("%s/album-%s-%s.jpeg",
					cwin->cpref->cache_folder,
					artist,
					album);
	if(glyr_info->head->data)
		album_art = vgdk_pixbuf_new_from_memory(glyr_info->head->data, glyr_info->head->size);

	if (album_art) {
		if (gdk_pixbuf_save(album_art, album_art_path, "jpeg", &error, "quality", "100", NULL)) {
			if((cwin->cstate->state != ST_STOPPED) &&
			   (0 == g_strcmp0(artist, cwin->cstate->curr_mobj->tags->artist)) &&
			   (0 == g_strcmp0(album, cwin->cstate->curr_mobj->tags->album))) {
				update_album_art(cwin->cstate->curr_mobj, cwin);
				mpris_update_metadata_changed(cwin);
			}
		}
		else {
			g_warning("Failed to save albumart file %s: %s\n", album_art_path, error->message);
			g_error_free(error);
		}
		g_object_unref(G_OBJECT(album_art));
	}

	glyr_free_list(glyr_info->head);
	glyr_query_destroy(&glyr_info->query);
	g_slice_free(glyr_struct, glyr_info);

	g_free(artist);
	g_free(album);
	g_free(album_art_path);

	return FALSE;
}

gpointer
get_album_art_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	head = glyr_get(&glyr_info->query, &error, NULL);

	if(head != NULL) {
		glyr_info->head = head;
		gdk_threads_add_idle(update_downloaded_album_art, glyr_info);
	}
	else {
		if (error != GLYRE_OK)
			g_warning("Error searching album art: %s", glyr_strerror(error));

		glyr_query_destroy(&glyr_info->query);
		g_slice_free(glyr_struct, glyr_info);
	}

	return NULL;
}

void related_get_album_art_handler (struct con_win *cwin)
{
	glyr_struct *glyr_info;
	gchar *artist, *album, *album_art_path;

	CDEBUG(DBG_INFO, "Get album art handler");

	if (cwin->cstate->state == ST_STOPPED)
		return;

	if ((strlen(cwin->cstate->curr_mobj->tags->artist) == 0) ||
	    (strlen(cwin->cstate->curr_mobj->tags->album) == 0))
		return;

	artist = g_strdup(cwin->cstate->curr_mobj->tags->artist);
	album = g_strdup(cwin->cstate->curr_mobj->tags->album);

	album_art_path = g_strdup_printf("%s/album-%s-%s.jpeg",
					cwin->cpref->cache_folder,
					artist,
					album);

	if (g_file_test(album_art_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE)
		goto exists;

	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init(&glyr_info->query);

	glyr_opt_type(&glyr_info->query, GLYR_GET_COVERART);
	glyr_opt_from(&glyr_info->query, "lastfm;musicbrainz");

	glyr_opt_artist(&glyr_info->query, artist);
	glyr_opt_album(&glyr_info->query, album);

	glyr_info->cwin = cwin;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Glyr get album", get_album_art_idle_func, glyr_info);
	#else
	g_thread_create(get_album_art_idle_func, glyr_info, FALSE, NULL);
	#endif

exists:
	g_free(album_art_path);
	g_free(artist);
	g_free(album);

	return;
}

int uninit_glyr_related (struct con_win *cwin)
{
	glyr_db_destroy(cwin->cache_db);
	glyr_cleanup ();

	return 0;
}

int init_glyr_related (struct con_win *cwin)
{
	glyr_init();

	cwin->cache_db = glyr_db_init(cwin->cpref->cache_folder);

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
