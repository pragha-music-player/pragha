/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-lastfm.h"
#include "pragha-simple-async.h"
#include "pragha-library-pane.h"
#include "pragha-utils.h"
#include "pragha-tagger.h"
#include "pragha-tags-dialog.h"
#include "pragha-tags-mgmt.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"
#include "xml_helper.h"
#include "pragha.h"

#ifdef HAVE_LIBCLASTFM

#include <clastfm.h>

typedef struct {
	GList *list;
	guint query_type;
	guint query_count;
	struct con_win *cwin;
} AddMusicObjectListData;

void
update_menubar_lastfm_state (struct con_win *cwin)
{
	GtkAction *action;

	gboolean playing = pragha_backend_get_state (cwin->backend) != ST_STOPPED;
	gboolean logged = cwin->clastfm->status == LASTFM_STATUS_OK;
	gboolean lfm_inited = cwin->clastfm->session_id != NULL;
	gboolean has_user = lfm_inited && string_is_not_empty(cwin->cpref->lastfm_user);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Lastfm/Love track");
	gtk_action_set_sensitive (GTK_ACTION (action), playing && logged);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Lastfm/Unlove track");
	gtk_action_set_sensitive (GTK_ACTION (action), playing && logged);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Lastfm/Add favorites");
	gtk_action_set_sensitive (GTK_ACTION (action), has_user);

	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ToolsMenu/Lastfm/Add similar");
	gtk_action_set_sensitive (GTK_ACTION (action), playing && lfm_inited);

	action = gtk_ui_manager_get_action(pragha_playlist_get_context_menu(cwin->cplaylist), "/SelectionPopup/ToolsMenu/Love track");
	gtk_action_set_sensitive (GTK_ACTION (action), logged);

	action = gtk_ui_manager_get_action(pragha_playlist_get_context_menu(cwin->cplaylist), "/SelectionPopup/ToolsMenu/Unlove track");
	gtk_action_set_sensitive (GTK_ACTION (action), logged);

	action = gtk_ui_manager_get_action(pragha_playlist_get_context_menu(cwin->cplaylist), "/SelectionPopup/ToolsMenu/Add similar");
	gtk_action_set_sensitive (GTK_ACTION (action), lfm_inited);
}


/* Find a song with the artist and title independently of the album and adds it to the playlist */

static GList *
prepend_song_with_artist_and_title_to_mobj_list(const gchar *artist,
						const gchar *title,
						GList *list,
						struct con_win *cwin)
{
	PraghaMusicobject *mobj = NULL;
	gint location_id = 0;

	if(pragha_mobj_list_already_has_title_of_artist(list, title, artist) ||
	   pragha_playlist_already_has_title_of_artist(cwin->cplaylist, title, artist))
		return list;

	const gchar *sql = "SELECT TRACK.title, ARTIST.name, LOCATION.id "
				"FROM TRACK, ARTIST, LOCATION "
				"WHERE ARTIST.id = TRACK.artist AND LOCATION.id = TRACK.location "
				"AND TRACK.title = ? COLLATE NOCASE "
				"AND ARTIST.name = ? COLLATE NOCASE "
				"ORDER BY RANDOM() LIMIT 1;";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cwin->cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, title);
	pragha_prepared_statement_bind_string (statement, 2, artist);

	if (pragha_prepared_statement_step (statement)) {
		location_id = pragha_prepared_statement_get_int (statement, 2);
		mobj = new_musicobject_from_db (cwin->cdbase, location_id);
		list = g_list_prepend (list, mobj);
	}

	pragha_prepared_statement_free (statement);

	return list;
}

/* Set correction basedm on lastfm now playing segestion.. */

static void
pragha_corrected_by_lastfm_dialog_response (GtkWidget      *dialog,
                                            gint            response_id,
                                            struct con_win *cwin)
{
	PraghaMusicobject *nmobj, *bmobj;
	PraghaTagger *tagger;
	gint changed = 0;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, cwin->mainwindow);
		return;
	}

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if(changed) {
			nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

			if(pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
				pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
				if(pragha_musicobject_compare(nmobj, cwin->cstate->curr_mobj) == 0) {
					/* Update public current song */
					pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, nmobj);
					pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

					/* Update current song on playlist */
					pragha_playlist_update_current_track(cwin->cplaylist, changed, nmobj);

					/* Update current song on backend */
					bmobj = g_object_ref(pragha_backend_get_musicobject(cwin->backend));
					pragha_update_musicobject_change_tag(bmobj, changed, nmobj);
					g_object_unref(bmobj);

					__update_current_song_info(cwin);
					mpris_update_metadata_changed(cwin);
				}
				else
					pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);
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

	gtk_widget_hide(cwin->clastfm->ntag_lastfm_button);
	gtk_widget_destroy (dialog);
}

static void
edit_tags_corrected_by_lastfm(GtkButton *button, struct con_win *cwin)
{
	PraghaMusicobject *tmobj, *nmobj;
	gchar *otitle = NULL, *oartist = NULL, *oalbum = NULL;
	gchar *ntitle = NULL, *nartist = NULL, *nalbum = NULL;
	gint prechanged = 0;
	GtkWidget *dialog;

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	/* Get all info of current track */
	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	tmobj = pragha_musicobject_dup (cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	g_object_get(tmobj,
	             "title", &otitle,
	             "artist", &oartist,
	             "album", &oalbum,
	             NULL);

	/* Get all info of suggestions
	 * Temp Musicobject to not block tag edit dialog */
	pragha_mutex_lock (cwin->clastfm->nmobj_mutex);
	nmobj = pragha_musicobject_dup(cwin->clastfm->nmobj);
	pragha_mutex_unlock (cwin->clastfm->nmobj_mutex);

	g_object_get(nmobj,
	             "title", &ntitle,
	             "artist", &nartist,
	             "album", &nalbum,
	             NULL);

	/* Compare original mobj and suggested from lastfm */
	if(g_ascii_strcasecmp(otitle, ntitle)) {
		pragha_musicobject_set_title(tmobj, ntitle);
		prechanged |= TAG_TITLE_CHANGED;
	}
	if(g_ascii_strcasecmp(oartist, nartist)) {
		pragha_musicobject_set_artist(tmobj, nartist);
		prechanged |= TAG_ARTIST_CHANGED;
	}
	if(g_ascii_strcasecmp(oalbum, nalbum)) {
		pragha_musicobject_set_album(tmobj, nalbum);
		prechanged |= TAG_ALBUM_CHANGED;
	}

	dialog = pragha_tags_dialog_new();

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_corrected_by_lastfm_dialog_response), cwin);

	pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), tmobj);
	pragha_tags_dialog_set_changed(PRAGHA_TAGS_DIALOG(dialog), prechanged);

	gtk_widget_show (dialog);
}

static GtkWidget*
pragha_lastfm_tag_suggestion_button_new(struct con_win *cwin)
{
	GtkWidget* ntag_lastfm_button;

	ntag_lastfm_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(ntag_lastfm_button), GTK_RELIEF_NONE);
	gtk_button_set_image(GTK_BUTTON(ntag_lastfm_button),
                         gtk_image_new_from_stock(GTK_STOCK_SPELL_CHECK, GTK_ICON_SIZE_MENU));
	gtk_widget_set_tooltip_text(GTK_WIDGET(ntag_lastfm_button),
	                            _("Last.fm suggested a tag correction"));

	g_signal_connect(G_OBJECT(ntag_lastfm_button), "clicked",
	                 G_CALLBACK(edit_tags_corrected_by_lastfm), cwin);

	return ntag_lastfm_button;
}

/* Love and unlove music object */

gpointer
do_lastfm_love_mobj (const gchar *title, const gchar *artist, struct con_win *cwin)
{
	gint rv;

	CDEBUG(DBG_LASTFM, "Love mobj on thread");

	rv = LASTFM_track_love (cwin->clastfm->session_id,
	                        title,
	                        artist);

	if (rv != LASTFM_STATUS_OK)
		return _("Love song on Last.fm failed.");
	else
		return NULL;
}

gpointer
do_lastfm_unlove_mobj (const gchar *title, const gchar *artist, struct con_win *cwin)
{
	gint rv;

	CDEBUG(DBG_LASTFM, "Unlove mobj on thread");

	rv = LASTFM_track_unlove (cwin->clastfm->session_id,
	                          title,
	                          artist);

	if (rv != LASTFM_STATUS_OK)
		return _("Unlove song on Last.fm failed.");
	else
		return NULL;
}


/* Functions related to current playlist. */

gpointer
do_lastfm_current_playlist_love (gpointer data)
{
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	struct con_win *cwin = data;

	mobj = pragha_playlist_get_selected_musicobject(cwin->cplaylist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	return do_lastfm_love_mobj(title, artist, cwin);
}

void
lastfm_track_current_playlist_love_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Love handler to current playlist");

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	pragha_async_launch(do_lastfm_current_playlist_love,
			    pragha_async_set_idle_message,
			    cwin);
}

gpointer
do_lastfm_current_playlist_unlove (gpointer data)
{
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	struct con_win *cwin = data;

	mobj = pragha_playlist_get_selected_musicobject(cwin->cplaylist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	return do_lastfm_unlove_mobj(title, artist, cwin);
}

void lastfm_track_current_playlist_unlove_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Unlove Handler to current playlist");

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	pragha_async_launch(do_lastfm_current_playlist_unlove,
			    pragha_async_set_idle_message,
			    cwin);
}

static gboolean
append_mobj_list_current_playlist_idle(gpointer user_data)
{
	gchar *summary = NULL;
	guint songs_added = 0;

	AddMusicObjectListData *data = user_data;

	GList *list = data->list;
	struct con_win *cwin = data->cwin;

	if(list != NULL) {
		pragha_playlist_append_mobj_list(cwin->cplaylist,
						 list);

		songs_added = g_list_length(list);
		g_list_free(list);
	}
	else {
		remove_watch_cursor (cwin->mainwindow);
	}

	switch(data->query_type) {
		case LASTFM_GET_SIMILAR:
			if(data->query_count > 0)
				summary = g_strdup_printf(_("Added %d songs of %d sugested from Last.fm."),
							  songs_added, data->query_count);
			else
				summary = g_strdup_printf(_("Last.fm not suggest any similar song."));
			break;
		case LASTFM_GET_LOVED:
			if(data->query_count > 0)
				summary = g_strdup_printf(_("Added %d songs of the last %d loved on Last.fm."),
							  songs_added, data->query_count);
			else
				summary = g_strdup_printf(_("You had no favorite songs on Last.fm."));
			break;
		case LASTFM_NONE:
		default:
			break;
	}

	if (summary != NULL) {
		pragha_statusbar_set_misc_text(cwin->statusbar, summary);
		g_free(summary);
	}

	g_slice_free (AddMusicObjectListData, data);

	return FALSE;
}

gpointer
do_lastfm_get_similar(const gchar *title, const gchar *artist, struct con_win *cwin)
{
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track = NULL;
	guint query_count = 0;
	GList *list = NULL;
	gint rv;

	AddMusicObjectListData *data;

	if(string_is_not_empty(title) && string_is_not_empty(artist)) {
		rv = LASTFM_track_get_similar(cwin->clastfm->session_id,
					      title,
					      artist,
					      50, &results);

		for(li=results; li && rv == LASTFM_STATUS_OK; li=li->next) {
			track = li->data;
			list = prepend_song_with_artist_and_title_to_mobj_list(track->artist, track->name, list, cwin);
			query_count += 1;
		}
	}

	data = g_slice_new (AddMusicObjectListData);
	data->list = list;
	data->query_type = LASTFM_GET_SIMILAR;
	data->query_count = query_count;
	data->cwin = cwin;

	LASTFM_free_track_info_list (results);

	return data;
}

gpointer
do_lastfm_get_similar_current_playlist_action (gpointer user_data)
{
	PraghaMusicobject *mobj = NULL;
	const gchar *title, *artist;

	AddMusicObjectListData *data;

	struct con_win *cwin = user_data;

	mobj = pragha_playlist_get_selected_musicobject(cwin->cplaylist);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);

	data = do_lastfm_get_similar(title, artist, cwin);

	return data;
}

void
lastfm_get_similar_current_playlist_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Get similar action to current playlist");

	if(cwin->clastfm->session_id == NULL) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	set_watch_cursor (cwin->mainwindow);
	pragha_async_launch(do_lastfm_get_similar_current_playlist_action,
			    append_mobj_list_current_playlist_idle,
			    cwin);
}

/* Functions that respond to menu options. */

static void
lastfm_import_xspf_response(GtkDialog *dialog,
				gint response,
				struct con_win *cwin)
{
	XMLNode *xml = NULL, *xi, *xc, *xt;
	gchar *contents, *summary;
	gint try = 0, added = 0;
	GList *list = NULL;

	GFile *file;
	gsize size;

	if(response != GTK_RESPONSE_ACCEPT)
		goto cancel;

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

		if (xt && xc)
			list = prepend_song_with_artist_and_title_to_mobj_list(xc->content, xt->content, list, cwin);
	}

	if(list)
		pragha_playlist_append_mobj_list(cwin->cplaylist, list);

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);
	pragha_statusbar_set_misc_text(cwin->statusbar, summary);
	g_free(summary);

	xmlnode_free(xml);
	g_list_free(list);
	g_free (contents);
out:
	g_object_unref (file);
cancel:
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

void
lastfm_import_xspf_action (GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkFileFilter *media_filter;

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

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(lastfm_import_xspf_response), cwin);

	gtk_widget_show_all (dialog);
}

gpointer
do_lastfm_add_favorites_action (gpointer user_data)
{
	LFMList *results = NULL, *li;
	LASTFM_TRACK_INFO *track;
	gint rpages = 0, cpage = 0;
	AddMusicObjectListData *data;
	guint query_count = 0;
	GList *list = NULL;

	struct con_win *cwin = user_data;

	do {
		rpages = LASTFM_user_get_loved_tracks(cwin->clastfm->session_id,
						     cwin->cpref->lastfm_user,
						     cpage,
						     &results);

		for(li=results; li; li=li->next) {
			track = li->data;
			list = prepend_song_with_artist_and_title_to_mobj_list(track->artist, track->name, list, cwin);
			query_count += 1;
		}
		LASTFM_free_track_info_list (results);
		cpage++;
	} while(rpages != 0);

	data = g_slice_new (AddMusicObjectListData);
	data->list = list;
	data->query_type = LASTFM_GET_LOVED;
	data->query_count = query_count;
	data->cwin = cwin;

	return data;
}

void
lastfm_add_favorites_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Add Favorites action");

	if ((cwin->clastfm->session_id == NULL) ||
	    string_is_empty(cwin->cpref->lastfm_user)) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	set_watch_cursor (cwin->mainwindow);
	pragha_async_launch(do_lastfm_add_favorites_action,
			    append_mobj_list_current_playlist_idle,
			    cwin);
}

gpointer
do_lastfm_get_similar_action (gpointer user_data)
{
	AddMusicObjectListData *data;
	gchar *title = NULL, *artist = NULL;

	struct con_win *cwin = user_data;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	data = do_lastfm_get_similar(title, artist, cwin);

	g_free(title);
	g_free(artist);

	return data;
}

void
lastfm_get_similar_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Get similar action");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if(cwin->clastfm->session_id == NULL) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	set_watch_cursor (cwin->mainwindow);
	pragha_async_launch(do_lastfm_get_similar_action,
			    append_mobj_list_current_playlist_idle,
			    cwin);
}

gpointer
do_lastfm_current_song_love (gpointer data)
{
	gchar *title = NULL, *artist = NULL;
	gpointer msg_data = NULL;

	struct con_win *cwin = data;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	msg_data = do_lastfm_love_mobj(title, artist, cwin);

	g_free(title);
	g_free(artist);

	return msg_data;
}

void
lastfm_track_love_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Love Handler");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	pragha_async_launch(do_lastfm_current_song_love,
			    pragha_async_set_idle_message,
			    cwin);
}

gpointer
do_lastfm_current_song_unlove (gpointer data)
{
	gchar *title = NULL, *artist = NULL;
	gpointer msg_data = NULL;

	struct con_win *cwin = data;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	msg_data = do_lastfm_unlove_mobj(title, artist, cwin);

	g_free(title);
	g_free(artist);

	return msg_data;
}

void
lastfm_track_unlove_action (GtkAction *action, struct con_win *cwin)
{
	CDEBUG(DBG_LASTFM, "Unlove Handler");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	pragha_async_launch(do_lastfm_current_song_unlove,
			    pragha_async_set_idle_message,
			    cwin);
}

gpointer
do_lastfm_scrob (gpointer data)
{
	gint rv;
	struct con_win *cwin = data;
	gchar *title = NULL, *artist = NULL, *album = NULL;
	gint track_no, length;

	CDEBUG(DBG_LASTFM, "Scrobbler thread");

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             "album", &album,
	             "track-no", &track_no,
	             "length", &length,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	rv = LASTFM_track_scrobble (cwin->clastfm->session_id,
	                            title,
	                            album,
	                            artist,
	                            cwin->clastfm->playback_started,
	                            length,
	                            track_no,
	                            0, NULL);

	g_free(title);
	g_free(artist);
	g_free(album);

	if (rv != LASTFM_STATUS_OK)
		return _("Last.fm submission failed");
	else
		return _("Track scrobbled on Last.fm");
}

gboolean
lastfm_scrob_handler(gpointer data)
{
	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Scrobbler Handler");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return FALSE;

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return FALSE;
	}

	pragha_async_launch(do_lastfm_scrob,
			    pragha_async_set_idle_message,
			    cwin);
	return FALSE;
}

static gboolean
show_lastfm_sugest_corrrection_button (gpointer user_data)
{
	gchar *cfile = NULL, *nfile = NULL;

	struct con_win *cwin = user_data;

	/* Hack to safe!.*/
	if(!cwin->clastfm->ntag_lastfm_button) {
		cwin->clastfm->ntag_lastfm_button =
			pragha_lastfm_tag_suggestion_button_new(cwin);
		pragha_toolbar_add_extention_widget(cwin->toolbar, cwin->clastfm->ntag_lastfm_button);
	}

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "file", &cfile,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	pragha_mutex_lock (cwin->clastfm->nmobj_mutex);
	g_object_get(cwin->clastfm->nmobj,
	             "file", &nfile,
	             NULL);
	pragha_mutex_unlock (cwin->clastfm->nmobj_mutex);

	if(g_ascii_strcasecmp(cfile, nfile) == 0)
		gtk_widget_show(cwin->clastfm->ntag_lastfm_button);

	g_free(cfile);
	g_free(nfile);

	return FALSE;
}

gpointer
do_lastfm_now_playing (gpointer data)
{
	PraghaMusicobject *tmobj;
	gchar *title = NULL, *artist = NULL, *album = NULL;
	gint track_no, length;
	LFMList *list = NULL;
	LASTFM_TRACK_INFO *ntrack = NULL;
	gint changed = 0, rv;

	struct con_win *cwin = data;

	CDEBUG(DBG_LASTFM, "Update now playing thread");

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             "album", &album,
	             "track-no", &track_no,
	             "length", &length,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	rv = LASTFM_track_update_now_playing (cwin->clastfm->session_id,
	                                      title,
	                                      album,
	                                      artist,
	                                      length,
	                                      track_no,
	                                      0,
	                                      &list);

	if (rv == LASTFM_STATUS_OK) {
		/* Fist check lastfm response, and compare tags. */
		if(list != NULL) {
			ntrack = list->data;
			if(ntrack->name && g_ascii_strcasecmp(title, ntrack->name))
				changed |= TAG_TITLE_CHANGED;
			if(ntrack->artist && g_ascii_strcasecmp(artist, ntrack->artist))
				changed |= TAG_ARTIST_CHANGED;
			if(ntrack->album && g_ascii_strcasecmp(album, ntrack->album))
				changed |= TAG_ALBUM_CHANGED;
		}

		if (changed) {
			pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
			tmobj = pragha_musicobject_dup(cwin->cstate->curr_mobj);
			pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

			if(changed & TAG_TITLE_CHANGED)
				pragha_musicobject_set_title(tmobj, ntrack->name);
			if(changed & TAG_ARTIST_CHANGED)
				pragha_musicobject_set_artist(tmobj, ntrack->artist);
			if(changed & TAG_ALBUM_CHANGED)
				pragha_musicobject_set_album(tmobj, ntrack->album);

			pragha_mutex_lock (cwin->clastfm->nmobj_mutex);
			g_object_unref(cwin->clastfm->nmobj);
			cwin->clastfm->nmobj = tmobj;
			pragha_mutex_unlock (cwin->clastfm->nmobj_mutex);

			g_idle_add (show_lastfm_sugest_corrrection_button, cwin);
		}
	}

	LASTFM_free_track_info_list(list);

	g_free(title);
	g_free(artist);
	g_free(album);

	if (rv != LASTFM_STATUS_OK)
		return _("Update current song on Last.fm failed.");
	else
		return NULL;
}

void
lastfm_now_playing_handler (struct con_win *cwin)
{
	gchar *title = NULL, *artist = NULL;
	gint length;

	CDEBUG(DBG_LASTFM, "Update now playing Handler");

	if(pragha_backend_get_state (cwin->backend) == ST_STOPPED)
		return;

	if(string_is_empty(cwin->cpref->lastfm_user) ||
	   string_is_empty(cwin->cpref->lastfm_pass))
		return;

	if(cwin->clastfm->status != LASTFM_STATUS_OK) {
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
		return;
	}

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "title", &title,
	             "artist", &artist,
	             "length", &length,
	             NULL);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	if (string_is_empty(title) || string_is_empty(artist) || (length < 30))
		goto exit;

	pragha_async_launch(do_lastfm_now_playing,
			    pragha_async_set_idle_message,
			    cwin);

	/* Kick the lastfm scrobbler on
	 * Note: Only scrob if tracks is more than 30s.
	 * and scrob when track is at 50% or 4mins, whichever comes
	 * first */

	if((length / 2) > (240 - WAIT_UPDATE)) {
		length = 240 - WAIT_UPDATE;
	}
	else {
		length = (length / 2) - WAIT_UPDATE;
	}

	cwin->related_timeout_id = g_timeout_add_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, length,
			lastfm_scrob_handler, cwin, NULL);
exit:
	g_free(title);
	g_free(artist);

	return;
}

/* Init lastfm with a simple thread when change preferences and show error messages. */

gboolean
do_just_init_lastfm(gpointer data)
{
	struct con_win *cwin = data;

	cwin->clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	if (cwin->clastfm->session_id != NULL) {
		if(string_is_not_empty(cwin->cpref->lastfm_user) &&
		   string_is_not_empty(cwin->cpref->lastfm_pass)) {
			cwin->clastfm->status = LASTFM_login (cwin->clastfm->session_id,
							      cwin->cpref->lastfm_user,
							      cwin->cpref->lastfm_pass);

			if(cwin->clastfm->status != LASTFM_STATUS_OK) {
				CDEBUG(DBG_INFO, "Failure to login on lastfm");
				pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
			}
		}
	}
	else {
		CDEBUG(DBG_INFO, "Failure to init libclastfm");
		pragha_statusbar_set_misc_text(cwin->statusbar, _("No connection Last.fm has been established."));
	}
	update_menubar_lastfm_state (cwin);

	return FALSE;
}

gint
just_init_lastfm (struct con_win *cwin)
{
	if (cwin->cpref->lastfm_support) {
		CDEBUG(DBG_INFO, "Initializing LASTFM");
		g_idle_add (do_just_init_lastfm, cwin);
	}
	return 0;
}

/* When just launch pragha init lastfm immediately if has internet or otherwise waiting 30 seconds.
 * And no show any error. */

gboolean
do_init_lastfm_idle(gpointer data)
{
	struct con_win *cwin = data;

	cwin->clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	if (cwin->clastfm->session_id != NULL) {
		if(string_is_not_empty(cwin->cpref->lastfm_user) &&
		   string_is_not_empty(cwin->cpref->lastfm_pass)) {
			cwin->clastfm->status = LASTFM_login (cwin->clastfm->session_id,
							      cwin->cpref->lastfm_user,
							      cwin->cpref->lastfm_pass);

			if(cwin->clastfm->status != LASTFM_STATUS_OK)
				CDEBUG(DBG_INFO, "Failure to login on lastfm");
		}
	}
	else {
		CDEBUG(DBG_INFO, "Failure to init libclastfm");
	}

	update_menubar_lastfm_state (cwin);

	return FALSE;
}

gint
init_lastfm(struct con_win *cwin)
{
	/* Init struct flags */

	cwin->clastfm->session_id = NULL;
	cwin->clastfm->status = LASTFM_STATUS_INVALID;
	cwin->clastfm->nmobj = pragha_musicobject_new();
	pragha_mutex_create(cwin->clastfm->nmobj_mutex);
	cwin->clastfm->ntag_lastfm_button = NULL;

	/* Test internet and launch threads.*/

	if (cwin->cpref->lastfm_support) {
		CDEBUG(DBG_INFO, "Initializing LASTFM");

#if GLIB_CHECK_VERSION(2,32,0)
		if (g_network_monitor_get_network_available (g_network_monitor_get_default ()))
#else
		if(nm_is_online () == TRUE)
#endif
			g_idle_add (do_init_lastfm_idle, cwin);
		else
			g_timeout_add_seconds_full(
					G_PRIORITY_DEFAULT_IDLE, 30,
					do_init_lastfm_idle, cwin, NULL);
	}

	return 0;
}

void
lastfm_free(struct con_lastfm *clastfm)
{
	if (clastfm->session_id)
		LASTFM_dinit(clastfm->session_id);

	g_object_unref(clastfm->nmobj);
	pragha_mutex_free(clastfm->nmobj_mutex);

	g_slice_free(struct con_lastfm, clastfm);
}
#endif
