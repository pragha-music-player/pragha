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
#include "pragha-tagger.h"
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
	PraghaTagger *tagger;
	GList *i;

	tagger = pragha_tagger_new();
	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;
		if (G_LIKELY(pragha_musicobject_is_local_file(mobj)))
			pragha_tagger_add_file (tagger, pragha_musicobject_get_file(mobj));
	}
	pragha_tagger_set_changes(tagger, nmobj, changed);
	pragha_tagger_apply_changes (tagger);
	g_object_unref(tagger);
}

void
pragha_save_disk_ref_list_change_tags(struct con_win *cwin, GList *list, gint changed, PraghaMusicobject *nmobj)
{
	PraghaMusicobject *mobj = NULL;
	PraghaTagger *tagger;
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	GList *i;

	tagger = pragha_tagger_new();

	model = pragha_playlist_get_model(cwin->cplaylist);
	for (i = list; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);

		if (G_LIKELY(gtk_tree_model_get_iter(model, &iter, path))) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (G_LIKELY(pragha_musicobject_is_local_file(mobj)))
				pragha_tagger_add_file (tagger, pragha_musicobject_get_file(mobj));
		}
		gtk_tree_path_free(path);
	}
	pragha_tagger_set_changes(tagger, nmobj, changed);
	pragha_tagger_apply_changes (tagger);
	g_object_unref(tagger);
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
		pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
		pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, omobj);
		pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

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

static void
pragha_edit_tags_playlist_dialog_response (GtkWidget      *dialog,
                                           gint            response_id,
                                           struct con_win *cwin)
{
	PraghaMusicobject *nmobj, *omobj;
	gint changed = 0;
	GList *rlist = NULL;
	gboolean need_update = FALSE;

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if(!changed)
			goto no_change;

		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		rlist = pragha_tags_dialog_get_ref_playlist(PRAGHA_TAGS_DIALOG(dialog));

		if(rlist) {
			if (changed & TAG_TNO_CHANGED) {
				if (g_list_length(rlist) > 1) {
					if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(nmobj), NULL))
						return;
				}
			}
			if (changed & TAG_TITLE_CHANGED) {
				if (g_list_length(rlist) > 1) {
					if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(nmobj), NULL))
						return;
				}
			}
			clear_sort_current_playlist_cb(NULL, cwin->cplaylist);

			need_update = pragha_playlist_update_ref_list_change_tags(cwin->cplaylist, rlist, changed, nmobj);
			pragha_save_disk_ref_list_change_tags(cwin, rlist, changed, nmobj);

			g_list_foreach (rlist, (GFunc) gtk_tree_row_reference_free, NULL);
			g_list_free (rlist);
		}

		if(need_update) {
			/* Update the public mobj */
			pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
			pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, nmobj);
			pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

			/* Update current song on backend */
			omobj = g_object_ref(pragha_backend_get_musicobject(cwin->backend));
			pragha_update_musicobject_change_tag(omobj, changed, nmobj);
			g_object_unref(omobj);

			if(pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
				__update_current_song_info(cwin);
				mpris_update_metadata_changed(cwin);
			}
		}
	}

no_change:
	gtk_widget_destroy (dialog);
}

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GList *rlist = NULL;
	PraghaMusicobject *mobj;

	dialog = pragha_tags_dialog_new();

	/* Get a list of references and music objects selected. */
	rlist = pragha_playlist_get_selection_ref_list(cwin->cplaylist);
	pragha_tags_dialog_set_ref_playlist(PRAGHA_TAGS_DIALOG(dialog), rlist);

	if(g_list_length(rlist) == 1) {
		mobj = pragha_playlist_get_selected_musicobject(cwin->cplaylist);
		pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), mobj);
	}

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_edit_tags_playlist_dialog_response), cwin);

	gtk_widget_show (dialog);
}
