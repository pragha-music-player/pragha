/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#include <tag_c.h>

#include "pragha-tags-mgmt.h"
#include "pragha-hig.h"
#include "pragha-library-pane.h"
#include "pragha-utils.h"
#include "pragha-tags-dialog.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"
#include "pragha.h"

gboolean
pragha_musicobject_set_tags_from_file(PraghaMusicobject *mobj, const gchar *file)
{
	gboolean ret = TRUE;
	TagLib_File *tfile = NULL;
	TagLib_Tag *tag;
	const TagLib_AudioProperties *audio_prop;

	/* workaround for crash in taglib
	   https://github.com/taglib/taglib/issues/78 */
	if (!g_file_test (file, G_FILE_TEST_EXISTS)) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	tfile = taglib_file_new(file);
	if (!tfile) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	tag = taglib_file_tag(tfile);
	if (!tag) {
		g_warning("Unable to locate tag in file %s", file);
		ret = FALSE;
		goto exit;
	}

	audio_prop = taglib_file_audioproperties(tfile);
	if (!audio_prop) {
		g_warning("Unable to locate audio properties in file %s", file);
		ret = FALSE;
		goto exit;
	}

	g_object_set (mobj,
	              "title", taglib_tag_title(tag),
	              "artist", taglib_tag_artist(tag),
	              "album", taglib_tag_album(tag),
	              "genre", taglib_tag_genre(tag),
	              "comment", taglib_tag_comment(tag),
	              "year", taglib_tag_year(tag),
	              "track-no", taglib_tag_track(tag),
	              "length", taglib_audioproperties_length(audio_prop),
	              "bitrate", taglib_audioproperties_bitrate(audio_prop),
	              "channels", taglib_audioproperties_channels(audio_prop),
	              "samplerate", taglib_audioproperties_samplerate(audio_prop),
	              NULL);

exit:
	taglib_tag_free_strings();
	taglib_file_free(tfile);

	return ret;
}

gboolean
pragha_musicobject_save_tags_to_file(gchar *file, PraghaMusicobject *mobj, int changed)
{
	gboolean ret = TRUE;
	TagLib_File *tfile;
	TagLib_Tag *tag;

	if (!file || !changed)
		return FALSE;

	tfile = taglib_file_new(file);
	if (!tfile) {
		g_warning("Unable to open file using taglib : %s", file);
		return FALSE;
	}

	tag = taglib_file_tag(tfile);
	if (!tag) {
		g_warning("Unable to locate tag in file %s", file);
		ret = FALSE;
		goto exit;
	}

	if (changed & TAG_TNO_CHANGED)
		taglib_tag_set_track(tag, pragha_musicobject_get_track_no(mobj));
	if (changed & TAG_TITLE_CHANGED)
		taglib_tag_set_title(tag,
			pragha_musicobject_get_title(mobj));
	if (changed & TAG_ARTIST_CHANGED)
		taglib_tag_set_artist(tag,
			pragha_musicobject_get_artist(mobj));
	if (changed & TAG_ALBUM_CHANGED)
		taglib_tag_set_album(tag,
			pragha_musicobject_get_album(mobj));
	if (changed & TAG_GENRE_CHANGED)
		taglib_tag_set_genre(tag,
			pragha_musicobject_get_genre(mobj));
	if (changed & TAG_YEAR_CHANGED)
		taglib_tag_set_year(tag, pragha_musicobject_get_year(mobj));
	if (changed & TAG_COMMENT_CHANGED)
		taglib_tag_set_comment(tag,
			pragha_musicobject_get_comment(mobj));

	CDEBUG(DBG_VERBOSE, "Saving tags for file: %s", file);

	if (!taglib_file_save(tfile)) {
		g_warning("Unable to save tags for: %s\n", file);
		ret = FALSE;
	}

	taglib_tag_free_strings();
exit:
	taglib_file_free(tfile);

	return ret;
}

/***************/
/* Tag Editing */
/***************/

static void
add_entry_tag_completion (const gchar *entry, GtkTreeModel *model)
{
	GtkTreeIter iter;

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, entry, -1);
}

gboolean confirm_tno_multiple_tracks(gint tno, GtkWidget *parent)
{
	GtkWidget *dialog;
	gint result;
	gboolean ret = FALSE;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Do you want to set the track number of ALL of the selected tracks to: %d ?"),
				tno);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_YES:
		ret = TRUE;
		break;
	case GTK_RESPONSE_NO:
		ret = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return ret;
}

gboolean confirm_title_multiple_tracks(const gchar *title, GtkWidget *parent)
{
	GtkWidget *dialog;
	gint result;
	gboolean ret = FALSE;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Do you want to set the title tag of ALL of the selected tracks to: %s ?"),
				title);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_YES:
		ret = TRUE;
		break;
	case GTK_RESPONSE_NO:
		ret = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return ret;
}

void
pragha_update_local_files_change_tag(GPtrArray *file_arr, gint changed, PraghaMusicobject *mobj)
{
	guint i = 0;
	gchar *elem;

	if (!changed)
		return;

	if (!file_arr)
		return;

	CDEBUG(DBG_VERBOSE, "Tags Changed: 0x%x", changed);

	/* This is so fscking horrible. */

	if (file_arr) {
		for (i = 0; i < file_arr->len; i++) {
			elem = g_ptr_array_index(file_arr, i);
			if (elem)
				(void)pragha_musicobject_save_tags_to_file(elem, mobj, changed);
		}
	}
}

/* Save tag change on db and disk. */

void
pragha_save_mobj_list_change_tags(struct con_win *cwin, GList *list, gint changed, PraghaMusicobject *nmobj)
{
	PraghaMusicobject *mobj = NULL;
	gint location_id;
	gchar *tfile;
	GArray *loc_arr = NULL;
	GPtrArray *file_arr = NULL;
	GList *i;

	loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
	file_arr = g_ptr_array_new_with_free_func(g_free);

	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;

		if (G_LIKELY(pragha_musicobject_is_local_file(mobj))) {
			location_id = pragha_database_find_location(cwin->cdbase, pragha_musicobject_get_file(mobj));
			if (G_LIKELY(location_id))
				g_array_append_val(loc_arr, location_id);

			tfile = g_strdup(pragha_musicobject_get_file(mobj));
			g_ptr_array_add(file_arr, tfile);
		}
	}

	/* Save new tags in db */
	if(loc_arr->len) {
		pragha_database_update_local_files_change_tag(cwin->cdbase, loc_arr, changed, nmobj);
		if(pragha_library_need_update(cwin->clibrary, changed))
			pragha_database_change_tracks_done(cwin->cdbase);
	}

	/* Save new tags in files */
	if(file_arr->len)
		pragha_update_local_files_change_tag(file_arr, changed, nmobj);

	g_array_free(loc_arr, TRUE);
	g_ptr_array_free(file_arr, TRUE);
}

/* Update tag change to a list of mobj. */

void
pragha_update_mobj_list_change_tag(GList *list, gint changed, PraghaMusicobject *nmobj)
{
	PraghaMusicobject *mobj = NULL;
	GList *i;

	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;
		pragha_update_musicobject_change_tag(mobj, changed, nmobj);
	}
}

/* Copy a tag change to all selected songs. */

void copy_tags_selection_current_playlist(PraghaMusicobject *omobj, gint changed, struct con_win *cwin)
{
	GList *rlist, *mlist;
	gboolean need_update;

	clear_sort_current_playlist_cb(NULL, cwin->cplaylist);

	pragha_playlist_set_changing(cwin->cplaylist, TRUE);
	rlist = pragha_playlist_get_selection_ref_list(cwin->cplaylist);
	mlist = pragha_playlist_get_selection_mobj_list(cwin->cplaylist);

	/* Update all mobj selected minus which provide the information. */
	mlist = g_list_remove (mlist, omobj);
	pragha_update_mobj_list_change_tag(mlist, changed, omobj);

	/* Update the view. */
	need_update = pragha_playlist_update_ref_list_change_tag(cwin->cplaylist, rlist, changed);
	pragha_playlist_set_changing(cwin->cplaylist, FALSE);

	/* If change current song, update gui and mpris. */

	if(need_update) {
		/* Update the public mobj */
		pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, omobj);

		if(pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
			__update_current_song_info(cwin);
			mpris_update_metadata_changed(cwin);
		}
	}

	/* Save tag change on db and disk. */
	pragha_save_mobj_list_change_tags(cwin, mlist, changed, omobj);

	g_list_foreach (rlist, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free (rlist);

	g_list_free (mlist);
}

/* Edit tags for selected track(s) */

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	PraghaMusicobject *omobj = NULL, *nmobj;
	GList *rlist, *mlist;
	gint sel = 0, changed = 0;
	gboolean need_update;

	/* Get a list of references and music objects selected. */
	rlist = pragha_playlist_get_selection_ref_list(cwin->cplaylist);
	mlist = pragha_playlist_get_selection_mobj_list(cwin->cplaylist);

	if(g_list_length(mlist) == g_list_length(rlist))
		sel = g_list_length(mlist);

	if(sel == 0)
		return;

	/* Setup initial entries */
	if(sel == 1)
		omobj = mlist->data;
	else
		omobj = pragha_musicobject_new();

	nmobj = pragha_musicobject_new();

	/* Get new tags edited */
	changed = tag_edit_dialog(omobj, 0, nmobj, cwin);

	if (!changed)
		goto exit;

	/* Check if user is trying to set the same track no for multiple tracks */
	if (changed & TAG_TNO_CHANGED) {
		if (sel > 1) {
			if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(nmobj), cwin->mainwindow))
				goto exit;
		}
	}

	/* Check if user is trying to set the same title/track no for
	   multiple tracks */
	if (changed & TAG_TITLE_CHANGED) {
		if (sel > 1) {
			if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(nmobj), cwin->mainwindow))
				goto exit;
		}
	}

	clear_sort_current_playlist_cb(NULL, cwin->cplaylist);

	/* Update all mobj selected. */
	pragha_update_mobj_list_change_tag(mlist, changed, nmobj);

	/* Update the view. */
	need_update = pragha_playlist_update_ref_list_change_tag(cwin->cplaylist, rlist, changed);

	/* If change current song, update gui and mpris. */
	if(need_update) {
		/* Update the public mobj */
		pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, nmobj);

		if(pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
			__update_current_song_info(cwin);
			mpris_update_metadata_changed(cwin);
		}
	}

	/* Save tag change on db and disk. */
	pragha_save_mobj_list_change_tags(cwin, mlist, changed, nmobj);

exit:
	g_list_foreach (rlist, (GFunc) gtk_tree_row_reference_free, NULL);
	g_object_unref(nmobj);
	if(sel > 1)
		g_object_unref(omobj);
	g_list_free (rlist);
	g_list_free (mlist);
}

void refresh_tag_completion_entries(struct con_win *cwin)
{
	GtkTreeModel *artist_tag_model, *album_tag_model, *genre_tag_model;
	const gchar *sql;
	PraghaPreparedStatement *statement;

	artist_tag_model = gtk_entry_completion_get_model(cwin->completion[0]);
	album_tag_model = gtk_entry_completion_get_model(cwin->completion[1]);
	genre_tag_model = gtk_entry_completion_get_model(cwin->completion[2]);

	gtk_list_store_clear(GTK_LIST_STORE(artist_tag_model));
	gtk_list_store_clear(GTK_LIST_STORE(album_tag_model));
	gtk_list_store_clear(GTK_LIST_STORE(genre_tag_model));

	sql = "SELECT name FROM ARTIST";
	statement = pragha_database_create_statement (cwin->cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		add_entry_tag_completion (name, artist_tag_model);
	}

	pragha_prepared_statement_free (statement);

	sql = "SELECT name FROM ALBUM";
	statement = pragha_database_create_statement (cwin->cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		add_entry_tag_completion (name, album_tag_model);
	}

	pragha_prepared_statement_free (statement);

	sql = "SELECT name FROM GENRE";
	statement = pragha_database_create_statement (cwin->cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		add_entry_tag_completion (name, genre_tag_model);
	}

	pragha_prepared_statement_free (statement);
}
