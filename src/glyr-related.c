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

#ifdef HAVE_LIBGLYR

typedef struct
{
	struct con_win	*cwin;
	GlyrQuery	query;
	GlyrMemCache	*head;
}
glyr_struct;

//FIXME_GLYR_CAST: drop discarding const when we switch to enough new glyr, see https://github.com/sahib/glyr/issues/29

/* Use the download info on glyr thread and show a dialog. */

static void
pragha_text_info_dialog_response(GtkDialog *dialog,
				gint response,
				struct con_win *cwin)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
pragha_show_related_text_info_dialog (glyr_struct *glyr_info, gchar *title_header, gchar *subtitle_header)
{
	GtkWidget *dialog, *header, *view, *scrolled;
	GtkTextBuffer *buffer;

	struct con_win *cwin = glyr_info->cwin;

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
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (subtitle_header, NULL);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scrolled, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(pragha_text_info_dialog_response), cwin);

	gtk_widget_show_all(dialog);
}

/* Save the downloaded album art in cache, and updates the gui.*/

static void
pragha_update_downloaded_album_art (glyr_struct *glyr_info)
{
	const gchar *artist = NULL, *album = NULL;
	gchar *album_art_path = NULL;
	GdkPixbuf *album_art = NULL;
	GError *error = NULL;

	struct con_win *cwin = glyr_info->cwin;

	artist = glyr_info->query.artist;
	album = glyr_info->query.album;

	album_art_path = g_strdup_printf("%s/album-%s-%s.jpeg",
					cwin->cpref->cache_folder,
					artist,
					album);
	if(glyr_info->head->data)
		album_art = vgdk_pixbuf_new_from_memory(glyr_info->head->data, glyr_info->head->size);

	if (album_art) {
		if (gdk_pixbuf_save(album_art, album_art_path, "jpeg", &error, "quality", "100", NULL)) {
			if((pragha_backend_get_state (cwin->backend) != ST_STOPPED) &&
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

	g_free(album_art_path);
}

/* Manages the results of glyr threads. */

static gboolean
glyr_finished_thread_update (gpointer data)
{
	glyr_struct *glyr_info = data;
	gchar *title_header = NULL, *subtitle_header = NULL;

	switch (glyr_info->head->type) {
	case GLYR_TYPE_COVERART:
		pragha_update_downloaded_album_art(glyr_info);
		break;
	case GLYR_TYPE_LYRICS:
		title_header =  g_strdup_printf(_("Lyrics thanks to %s"), glyr_info->head->prov);
		subtitle_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), glyr_info->query.title, glyr_info->query.artist);
		pragha_show_related_text_info_dialog(glyr_info, title_header, subtitle_header);
		break;
#if GLYR_CHECK_VERSION (1, 0, 0)
	case GLYR_TYPE_ARTIST_BIO:
#else
	case GLYR_TYPE_ARTISTBIO:
#endif
		title_header =  g_strdup_printf(_("Artist info"));
		subtitle_header = g_strdup_printf(_("%s <small><span weight=\"light\">thanks to</span></small> %s"), glyr_info->query.artist, glyr_info->head->prov);
		pragha_show_related_text_info_dialog(glyr_info, title_header, subtitle_header);
		break;
	default:
		break;
	}

	glyr_free_list(glyr_info->head);
	glyr_query_destroy(&glyr_info->query);
	g_slice_free(glyr_struct, glyr_info);

	g_free(title_header);
	g_free(subtitle_header);

	return FALSE;
}

/* Get artist bio or lyric on a thread. */

static gpointer
get_related_text_info_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	set_watch_cursor_on_thread(glyr_info->cwin);

	head = glyr_get(&glyr_info->query, &error, NULL);

	if(head != NULL) {
		glyr_info->head = head;
		g_idle_add(glyr_finished_thread_update, glyr_info);

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

/* Configure the thrad to get the artist bio or lyric. */

static void
configure_and_launch_get_text_info_dialog(GLYR_GET_TYPE type, const gchar *artist, const gchar *title, struct con_win *cwin)
{
	glyr_struct *glyr_info;
	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init(&glyr_info->query);
	glyr_opt_type(&glyr_info->query, type);

	switch (type) {
#if GLYR_CHECK_VERSION (1, 0, 0)
	case GLYR_GET_ARTIST_BIO:
#else
	case GLYR_GET_ARTISTBIO:
#endif
		glyr_opt_artist(&glyr_info->query, (char*)artist); //FIXME_GLYR_CAST

		glyr_opt_lang (&glyr_info->query, "auto");
		glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
		break;
	case GLYR_GET_LYRICS:
		glyr_opt_artist(&glyr_info->query, (char*)artist); //FIXME_GLYR_CAST
		glyr_opt_title(&glyr_info->query, (char*)title); //FIXME_GLYR_CAST
		break;
	default:
		break;
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

/* Handlers to get lyric and artist bio of current song. */

void related_get_artist_info_action (GtkAction *action, struct con_win *cwin)
{
	const gchar *artist = cwin->cstate->curr_mobj->tags->artist;

	CDEBUG(DBG_INFO, "Get Artist info Action");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if (strlen(artist) == 0)
		return;

	configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, cwin);
}

void related_get_lyric_action(GtkAction *action, struct con_win *cwin)
{
	const gchar *artist = cwin->cstate->curr_mobj->tags->artist;
	const gchar *title = cwin->cstate->curr_mobj->tags->title;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if ((strlen(artist) == 0) ||
	    (strlen(title) == 0))
		return;

	configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, cwin);
}

/* Handlers to get lyric and artist bio of current playlist selection. */

void
related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin)
{
	struct musicobject *mobj = get_selected_musicobject(cwin);
	const gchar *artist = mobj->tags->artist;

	CDEBUG(DBG_INFO, "Get Artist info Action of current playlist selection");

	if (strlen(artist) == 0)
		return;

	configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, cwin);
}

void
related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin)
{
	struct musicobject *mobj = get_selected_musicobject(cwin);
	const gchar *artist = mobj->tags->artist;
	const gchar *title = mobj->tags->title;

	CDEBUG(DBG_INFO, "Get lyrics Action of current playlist selection.");

	if ((strlen(artist) == 0) ||
	    (strlen(title) == 0))
		return;

	configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, cwin);
}

/* Download the album art in a thread. */

static gpointer
get_album_art_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	head = glyr_get(&glyr_info->query, &error, NULL);

	if(head != NULL) {
		glyr_info->head = head;
		g_idle_add(glyr_finished_thread_update, glyr_info);
	}
	else {
		if (error != GLYRE_OK)
			g_warning("Error searching album art: %s", glyr_strerror(error));

		glyr_query_destroy(&glyr_info->query);
		g_slice_free(glyr_struct, glyr_info);
	}

	return NULL;
}

static void
related_get_album_art_handler (struct con_win *cwin)
{
	glyr_struct *glyr_info;
	const gchar *artist, *album;
	gchar *album_art_path;

	CDEBUG(DBG_INFO, "Get album art handler");

	if (pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if ((strlen(cwin->cstate->curr_mobj->tags->artist) == 0) ||
	    (strlen(cwin->cstate->curr_mobj->tags->album) == 0))
		return;

	artist = cwin->cstate->curr_mobj->tags->artist;
	album = cwin->cstate->curr_mobj->tags->album;

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

	glyr_opt_artist(&glyr_info->query, (char*)artist); //FIXME_GLYR_CAST
	glyr_opt_album(&glyr_info->query, (char*)album); //FIXME_GLYR_CAST

	glyr_info->cwin = cwin;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Glyr get album", get_album_art_idle_func, glyr_info);
	#else
	g_thread_create(get_album_art_idle_func, glyr_info, FALSE, NULL);
	#endif

exists:
	g_free(album_art_path);
}

static gboolean
update_related_handler (gpointer data)
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

static void
update_related_state_cb (GObject *gobject, gint state, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Configuring thread to update Lastfm and get the cover art");

	if(cwin->related_timeout_id)
		g_source_remove(cwin->related_timeout_id);

	if(state != ST_PLAYING)
		return;

	if(cwin->cstate->curr_mobj->file_type == FILE_HTTP)
		return;

	#ifdef HAVE_LIBCLASTFM
	if (cwin->clastfm->status == LASTFM_STATUS_OK)
		time(&cwin->clastfm->playback_started);
	#endif

	cwin->related_timeout_id = g_timeout_add_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, WAIT_UPDATE,
			update_related_handler, cwin, NULL);
}

void glyr_related_free (struct con_win *cwin)
{
	g_signal_handlers_disconnect_by_func (cwin->backend, update_related_state_cb, cwin);
	glyr_db_destroy(cwin->cache_db);
	glyr_cleanup ();
}

int init_glyr_related (struct con_win *cwin)
{
	glyr_init();

	cwin->cache_db = glyr_db_init(cwin->cpref->cache_folder);

	g_signal_connect (cwin->backend, "state-change", G_CALLBACK (update_related_state_cb), cwin);

	return 0;
}
#endif
