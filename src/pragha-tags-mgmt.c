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

gboolean
confirm_tno_multiple_tracks(gint tno, GtkWidget *parent)
{
	GtkWidget *dialog;
	gint response;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                GTK_MESSAGE_QUESTION,
	                                GTK_BUTTONS_YES_NO,
	                                _("Do you want to set the track number of ALL of the selected tracks to: %d ?"), tno);

	response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	return (response == GTK_RESPONSE_YES);
}

gboolean
confirm_title_multiple_tracks(const gchar *title, GtkWidget *parent)
{
	GtkWidget *dialog;
	gint response;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                GTK_MESSAGE_QUESTION,
	                                GTK_BUTTONS_YES_NO, _("Do you want to set the title tag of ALL of the selected tracks to: %s ?"), title);

	response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	return (response == GTK_RESPONSE_YES);
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

/* Copy a tag change to all selected songs. */

void copy_tags_selection_current_playlist(PraghaMusicobject *omobj, gint changed, struct con_win *cwin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *tmobj, *current_mobj = NULL;
	GList *rlist;
	gboolean need_update;

	playlist = pragha_application_get_playlist (cwin);

	clear_sort_current_playlist_cb (NULL, playlist);

	pragha_playlist_set_changing (playlist, TRUE);

	rlist = pragha_playlist_get_selection_ref_list (playlist);
	tmobj = pragha_musicobject_dup(omobj);

	/* Update the view and save tag change on db and disk.*/
	need_update = pragha_playlist_update_ref_list_change_tags (playlist, rlist, changed, tmobj);
	pragha_playlist_set_changing (playlist, FALSE);

	/* If change current song, update gui and mpris. */
	if(need_update) {
		/* Update the public mobj */
		backend = pragha_application_get_backend (cwin);
		current_mobj = pragha_backend_get_musicobject (backend);
		pragha_update_musicobject_change_tag (current_mobj, changed, omobj);

		if(pragha_backend_get_state (backend) != ST_STOPPED) {
			toolbar = pragha_application_get_toolbar (cwin);
			pragha_toolbar_set_title (toolbar, current_mobj);
			mpris_update_metadata_changed(cwin);
		}
	}

	g_object_unref(tmobj);
	g_list_foreach (rlist, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free (rlist);
}

/* Edit tags for selected track(s) */

static void
pragha_edit_tags_playlist_dialog_response (GtkWidget      *dialog,
                                           gint            response_id,
                                           struct con_win *cwin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *current_mobj = NULL, *nmobj;
	gint changed = 0;
	GList *rlist = NULL;
	gboolean need_update = FALSE;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, pragha_application_get_window(cwin));
		return;
	}

	rlist = g_object_get_data (G_OBJECT (dialog), "reference-list");

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if(!changed)
			goto no_change;

		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

		if(rlist) {
			if (changed & TAG_TNO_CHANGED) {
				if (g_list_length(rlist) > 1) {
					if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(nmobj), pragha_application_get_window(cwin)))
						return;
				}
			}
			if (changed & TAG_TITLE_CHANGED) {
				if (g_list_length(rlist) > 1) {
					if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(nmobj), pragha_application_get_window(cwin)))
						return;
				}
			}
			playlist = pragha_application_get_playlist (cwin);
			clear_sort_current_playlist_cb (NULL, playlist);

			need_update = pragha_playlist_update_ref_list_change_tags (playlist, rlist, changed, nmobj);
		}

		if(need_update) {
			/* Update the public mobj */
			backend = pragha_application_get_backend (cwin);
			current_mobj = pragha_backend_get_musicobject (backend);
			pragha_update_musicobject_change_tag (current_mobj, changed, nmobj);

			if(pragha_backend_get_state (backend) != ST_STOPPED) {
				toolbar = pragha_application_get_toolbar (cwin);
				pragha_toolbar_set_title (toolbar, current_mobj);
				mpris_update_metadata_changed(cwin);
			}
		}
	}

no_change:
	g_list_foreach (rlist, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free (rlist);

	gtk_widget_destroy (dialog);
}

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	PraghaPlaylist *playlist;
	GtkWidget *dialog;
	GList *rlist = NULL;
	PraghaMusicobject *mobj;

	dialog = pragha_tags_dialog_new();

	/* Get a list of references and music objects selected. */

	playlist = pragha_application_get_playlist (cwin);
	rlist = pragha_playlist_get_selection_ref_list (playlist);
	if(g_list_length(rlist) == 1) {
		mobj = pragha_playlist_get_selected_musicobject (playlist);
		pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), mobj);
	}
	g_object_set_data (G_OBJECT (dialog), "reference-list", rlist);
 
	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_edit_tags_playlist_dialog_response), cwin);

	gtk_widget_show (dialog);
}
