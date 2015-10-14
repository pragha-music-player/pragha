/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
/*									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/*									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the	 */
/* GNU General Public License for more details.				 */
/*									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-tags-mgmt.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>

#include <stdlib.h>
#include <tag_c.h>

#include "pragha-tagger.h"
#include "pragha-hig.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"

gboolean
pragha_musicobject_set_tags_from_file(PraghaMusicobject *mobj, const gchar *file)
{
	gboolean ret = TRUE;
	TagLib_File *tfile = NULL;
	TagLib_Tag *tag;
	const TagLib_AudioProperties *audio_prop;
	gchar *title = NULL, *artist = NULL, *album = NULL, *genre = NULL, *comment = NULL;

	/* workaround for crash in taglib
	   https://github.com/taglib/taglib/issues/78 */
	if (!g_file_test (file, G_FILE_TEST_EXISTS)) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	#ifdef G_OS_WIN32
	GError *err = NULL;
	gchar *encoded_file = g_locale_from_utf8(file, -1, NULL, NULL, &err);
	if (!encoded_file) {
		g_warning("Unable to get filename from UTF-8 string: %s", file);
		g_error_free(err);
		err = NULL;
	}
	else {
		tfile = taglib_file_new(encoded_file);
		g_free(encoded_file);
	}
	#else
	tfile = taglib_file_new(file);
	#endif
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

	title = taglib_tag_title(tag);
	artist = taglib_tag_artist(tag);
	album = taglib_tag_album(tag);
	genre = taglib_tag_genre(tag);
	comment = taglib_tag_comment(tag);

	g_object_set (mobj,
		      "title", title,
		      "artist", artist,
		      "album", album,
		      "genre", genre,
		      "comment", comment,
		      "year", taglib_tag_year(tag),
		      "track-no", taglib_tag_track(tag),
		      "length", taglib_audioproperties_length(audio_prop),
		      "bitrate", taglib_audioproperties_bitrate(audio_prop),
		      "channels", taglib_audioproperties_channels(audio_prop),
		      "samplerate", taglib_audioproperties_samplerate(audio_prop),
		      NULL);

	free(title);
	free(artist);
	free(album);
	free(genre);
	free(comment);

exit:
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
					GTK_DIALOG_MODAL |
					GTK_DIALOG_DESTROY_WITH_PARENT |
					GTK_DIALOG_USE_HEADER_BAR,
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
