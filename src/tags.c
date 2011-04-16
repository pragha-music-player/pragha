/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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
#include <tag_c.h>

gboolean get_wav_info(gchar *file, struct tags *tags)
{
	SNDFILE *sfile = NULL;
	SF_INFO sinfo;

	memset(&sinfo, 0, sizeof(sinfo));

	sfile = sf_open(file, SFM_READ, &sinfo);
	if (!sfile) {
		g_warning("Unable to open file using sndfile : %s", file);
		return FALSE;
	}

	tags->artist = g_strdup("");
	tags->album = g_strdup("");
	tags->genre = g_strdup("");
	tags->comment = g_strdup("");
	tags->title = g_path_get_basename(file);
	tags->channels = sinfo.channels;
	tags->samplerate = sinfo.samplerate;
	tags->length = sinfo.frames / sinfo.samplerate;

	if (sf_close(sfile))
		g_warning("Unable to close file using sndfile : %s", file);

	return TRUE;
}

static gboolean get_info_taglib(gchar *file, struct tags *tags)
{
	gboolean ret = FALSE;
	TagLib_File *tfile;
	TagLib_Tag *tag;
	const TagLib_AudioProperties *audio_prop;

	tfile = taglib_file_new(file);
	if (!tfile) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	tag = taglib_file_tag(tfile);
	if (!tag) {
		g_warning("Unable to locate tag");
		ret = FALSE;
		goto exit;
	}

	audio_prop = taglib_file_audioproperties(tfile);
	if (!audio_prop) {
		g_warning("Unable to locate audio properties");
		ret = FALSE;
		goto exit;
	}

	tags->title = g_strdup(taglib_tag_title(tag));
	tags->artist = g_strdup(taglib_tag_artist(tag));
	tags->album = g_strdup(taglib_tag_album(tag));
	tags->genre = g_strdup(taglib_tag_genre(tag));
	tags->comment = g_strdup(taglib_tag_comment(tag));
	tags->track_no = taglib_tag_track(tag);
	tags->year = taglib_tag_year(tag);
	tags->bitrate = taglib_audioproperties_bitrate(audio_prop);
	tags->length = taglib_audioproperties_length(audio_prop);
	tags->channels = taglib_audioproperties_channels(audio_prop);
	tags->samplerate = taglib_audioproperties_samplerate(audio_prop);
	ret = TRUE;
exit:
	taglib_tag_free_strings();
	taglib_file_free(tfile);

	return ret;
}

gboolean get_mp3_info(gchar *file, struct tags *tags)
{
	return get_info_taglib(file, tags);
}

gboolean get_flac_info(gchar *file, struct tags *tags)
{
	return get_info_taglib(file, tags);
}

gboolean get_ogg_info(gchar *file, struct tags *tags)
{
	return get_info_taglib(file, tags);
}

gboolean get_mod_info(gchar *file, struct tags *tags)
{
	gchar *data;
	gsize length;
	ModPlugFile *mf;
	
	if(!g_file_get_contents(file, &data, &length, NULL))
	{
		g_critical("Unable to open file : %s", file);
		return FALSE;
	}

	mf = ModPlug_Load((const void*)data, (int)length);

	if(!mf) {
		g_critical("ModPlug_Load failed for %s", file);
		g_free(data);
		return FALSE;
	}

	;
	tags->artist = g_strdup("");
	tags->album = g_strdup("");
	tags->genre = g_strdup("");
	tags->comment = g_strdup("");
	tags->title = g_strdup(ModPlug_GetName(mf));
	tags->channels = 2;
	tags->samplerate = 44100;
	tags->length = ModPlug_GetLength(mf)/1000;

	ModPlug_Unload(mf);

	g_free(data);

	return TRUE;
}

gboolean save_tags_to_file(gchar *file, struct tags *ntag,
			   int changed, struct con_win *cwin)
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
		g_warning("Unable to locate tag");
		ret = FALSE;
		goto exit;
	}

	if (changed & TAG_TNO_CHANGED)
		taglib_tag_set_track(tag, ntag->track_no);
	if (changed & TAG_TITLE_CHANGED)
		taglib_tag_set_title(tag, ntag->title);
	if (changed & TAG_ARTIST_CHANGED)
		taglib_tag_set_artist(tag, ntag->artist);
	if (changed & TAG_ALBUM_CHANGED)
		taglib_tag_set_album(tag, ntag->album);
	if (changed & TAG_GENRE_CHANGED)
		taglib_tag_set_genre(tag, ntag->genre);
	if (changed & TAG_YEAR_CHANGED)
		taglib_tag_set_year(tag, ntag->year);
	if (changed & TAG_COMMENT_CHANGED)
		taglib_tag_set_comment(tag, ntag->comment);

	CDEBUG(DBG_VERBOSE, "Saving tags for file: %s", file);

	if (!taglib_file_save(tfile)) {
		g_warning("Unable to save tags for: %s\n", file);
		ret = FALSE;
	}
exit:
	taglib_file_free(tfile);

	return ret;
}

/***************/
/* Tag Editing */
/***************/

static void add_entry_tag_completion(gchar *entry, GtkTreeModel *model)
{
	GtkTreeIter iter;

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, entry, -1);
}

static gboolean confirm_tno_multiple_tracks(gint tno, struct con_win *cwin)
{
	GtkWidget *dialog;
	gint result;
	gboolean ret = FALSE;

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"Do you want to set the track number of ALL of the "
				"selected tracks to: %d ?",
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

static gboolean confirm_title_multiple_tracks(gchar *title, struct con_win *cwin)
{
	GtkWidget *dialog;
	gint result;
	gboolean ret = FALSE;

	dialog = gtk_message_dialog_new(GTK_WINDOW(cwin->mainwindow),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"Do you want to set the title tag of ALL of the "
				"selected tracks to: %s ?",
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

static gboolean entry_validate_cb(GtkWidget *entry, GdkEventKey *event,
				  gpointer data)
{
	gunichar unich;

	unich = gdk_keyval_to_unicode(event->keyval);

	if (g_unichar_isdigit(unich))
		return FALSE;

	if (g_unichar_isalpha(unich) || g_unichar_isspace(unich))
		return TRUE;

	return FALSE;
}

void tag_update(GArray *loc_arr, GArray *file_arr, gint changed, struct tags *ntag,
		struct con_win *cwin)
{
	gboolean ret = FALSE;
	gchar *query = NULL, *stitle = NULL, *sartist = NULL, *scomment= NULL;
	gchar *salbum = NULL, *sgenre = NULL, *file = NULL;
	gint i = 0, artist_id = 0, album_id = 0, genre_id = 0, year_id = 0, comment_id = 0;
	struct db_result result;

	if (!changed)
		return;

	if (!loc_arr && !file_arr)
		return;

	CDEBUG(DBG_VERBOSE, "Tags Changed: 0x%x", changed);

	/* Check if user is trying to set the same track no for multiple tracks */
	if (changed & TAG_TNO_CHANGED) {
		if (loc_arr->len > 1) {
			if (!confirm_tno_multiple_tracks(ntag->track_no, cwin))
				return;
		}
	}

	/* Check if user is trying to set the same title/track no for
	   multiple tracks */
	if (changed & TAG_TITLE_CHANGED) {
		if (loc_arr->len > 1) {
			if (!confirm_title_multiple_tracks(ntag->title, cwin))
				return;
		}

		stitle = sanitize_string_sqlite3(ntag->title);
	}
	if (changed & TAG_ARTIST_CHANGED) {
		sartist = sanitize_string_sqlite3(ntag->artist);
		artist_id = find_artist_db(sartist, cwin);
		if (!artist_id)
			artist_id = add_new_artist_db(sartist, cwin);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		salbum = sanitize_string_sqlite3(ntag->album);
		album_id = find_album_db(salbum, cwin);
		if (!album_id)
			album_id = add_new_album_db(salbum, cwin);
	}
	if (changed & TAG_GENRE_CHANGED) {
		sgenre = sanitize_string_sqlite3(ntag->genre);
		genre_id = find_genre_db(sgenre, cwin);
		if (!genre_id)
			genre_id = add_new_genre_db(sgenre, cwin);
	}
	if (changed & TAG_YEAR_CHANGED) {
		year_id = find_year_db(ntag->year, cwin);
		if (!year_id)
			year_id = add_new_year_db(ntag->year, cwin);
	}
	if (changed & TAG_COMMENT_CHANGED) {
		scomment = sanitize_string_sqlite3(ntag->comment);
		comment_id = find_comment_db(scomment, cwin);
		if (!comment_id)
			comment_id = add_new_comment_db(scomment, cwin);
	}

	/* This is so fscking horrible. */

	if (loc_arr) {
		gint elem = 0;
		for(i = 0; i < loc_arr->len; i++) {
			elem = g_array_index(loc_arr, gint, i);
			if (elem) {
				query = g_strdup_printf("SELECT name FROM LOCATION "
							"WHERE id = '%d';",
							elem);
				if (exec_sqlite_query(query, cwin, &result)) {
					file = result.resultp[result.no_columns];
					ret = save_tags_to_file(file, ntag,
								changed, cwin);
					sqlite3_free_table(result.resultp);
				}
				if (ret) {
					update_track_db(elem, changed,
							ntag->track_no, stitle,
							artist_id,
							album_id,
							genre_id,
							year_id,
							comment_id,
							cwin);
					ret = FALSE;
				}
			}
		}
	}

	if (file_arr) {
		gchar *elem;
		for (i = 0; i < file_arr->len; i++) {
			elem = g_array_index(file_arr, gchar *, i);
			if (elem)
				(void)save_tags_to_file(elem, ntag, changed, cwin);
		}
	}

	g_free(stitle);
	g_free(sartist);
	g_free(salbum);
	g_free(sgenre);
	g_free(scomment);
}

void check_entry(GtkEntry *entry, GtkCheckButton *check)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
}

static void
clear_pressed (GtkEntry       *entry,
		gint            position,
		GdkEventButton *event)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gtk_entry_set_text (entry, "");
		gtk_widget_grab_focus(GTK_WIDGET(entry));
	}
}

static void
directory_pressed (GtkEntry       *entry,
		gint            position,
		GdkEventButton *event,
		gchar *file)
{
	if (position == GTK_ENTRY_ICON_SECONDARY && file) {
		gchar *uri = get_display_filename(file, TRUE);
		open_url(NULL, uri);
		g_free(uri);
	}
}

gint tag_edit_dialog(struct tags *otag, struct tags *ntag, gchar *file,
		     struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *tag_table;
	GtkWidget *label_title, *label_artist, *label_album, *label_genre, *label_tno, *label_year, *label_comment, *label_file;
	GtkWidget *chk_title, *chk_artist, *chk_album, *chk_genre, *chk_tno, *chk_year, *chk_comment;
	GtkWidget *entry_title, *entry_artist, *entry_album, *entry_genre,  *entry_tno, *entry_year, *entry_comment, *entry_file;
	GtkWidget *hbox_title, *hbox_artist, *hbox_album, *hbox_genre, *hbox_tno, *hbox_year, *hbox_comment;
	GtkWidget *hbox_spins, *comment_view_scroll, *chk_alignment;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	gint location_id, result, changed = 0;
	struct musicobject *mobj = NULL;
	gchar *uri = NULL;

	/*Create table*/

	tag_table = gtk_table_new(8, 2, FALSE);

	gtk_table_set_col_spacings(GTK_TABLE(tag_table), 5);
	gtk_table_set_row_spacings(GTK_TABLE(tag_table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(tag_table), 5);

	/* Create labels */

	label_title = gtk_label_new(_("Title"));
	label_artist = gtk_label_new(_("Artist"));
	label_album = gtk_label_new(_("Album"));
	label_genre = gtk_label_new(_("Genre"));
	label_tno = gtk_label_new(_("Track No"));
	label_year = gtk_label_new(_("Year"));
	label_comment = gtk_label_new(_("Comment"));
	label_file = gtk_label_new(_("File"));

	gtk_misc_set_alignment(GTK_MISC (label_title), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_artist), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_album), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_genre), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_tno), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_year), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC (label_comment), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_file), 1, 0.5);

	/* Create entry fields */

	entry_title = gtk_entry_new();
	entry_artist = gtk_entry_new();
	entry_album = gtk_entry_new();
	entry_genre = gtk_entry_new();

	entry_tno = gtk_spin_button_new_with_range (0, 2030, 1);
	entry_year = gtk_spin_button_new_with_range (0, 2030, 1);

	entry_comment = gtk_text_view_new();
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (entry_comment), FALSE);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry_comment));

	entry_file = gtk_entry_new();

	gtk_entry_set_max_length(GTK_ENTRY(entry_title), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_artist), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_album), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_genre), TAG_MAX_LEN);

	gtk_entry_set_completion(GTK_ENTRY(entry_artist), cwin->completion[0]);
	gtk_entry_set_completion(GTK_ENTRY(entry_album), cwin->completion[1]);
	gtk_entry_set_completion(GTK_ENTRY(entry_genre), cwin->completion[2]);

	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_title), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_artist), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_album), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_genre), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_file), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);

	gtk_entry_set_editable (GTK_ENTRY(entry_file), FALSE);

	g_signal_connect(G_OBJECT(entry_tno), "key-press-event",
			 G_CALLBACK(entry_validate_cb), cwin);
	g_signal_connect(G_OBJECT(entry_year), "key-press-event",
			 G_CALLBACK(entry_validate_cb), cwin);

	/* Create checkboxes */

	chk_title = gtk_check_button_new();
	chk_artist = gtk_check_button_new();
	chk_album = gtk_check_button_new();
	chk_genre = gtk_check_button_new();
	chk_year = gtk_check_button_new();
	chk_tno = gtk_check_button_new();
	chk_comment = gtk_check_button_new();

	hbox_title = gtk_hbox_new(FALSE, 0);
	hbox_artist = gtk_hbox_new(FALSE, 0);
	hbox_album = gtk_hbox_new(FALSE, 0);
	hbox_genre = gtk_hbox_new(FALSE, 0);
	hbox_year = gtk_hbox_new(FALSE, 0);
	hbox_tno = gtk_hbox_new(FALSE, 0);
	hbox_comment = gtk_hbox_new(FALSE, 0);

	hbox_spins = gtk_hbox_new(FALSE, 5);

	/* Create hobxs(ENTRY CHECHK) and attach in table */

	gtk_box_pack_start(GTK_BOX(hbox_title),
			   entry_title,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_title),
			   chk_title,
			   FALSE,
			   FALSE,
			   0);

	gtk_table_attach(GTK_TABLE (tag_table), label_title,
			0, 1, 0, 1,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_title,
			1, 2, 0, 1,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_artist),
			   entry_artist,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_artist),
			   chk_artist,
			   FALSE,
			   FALSE,
			   0);
	gtk_table_attach(GTK_TABLE (tag_table), label_artist,
			0, 1, 1, 2,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_artist,
			1, 2, 1, 2,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_album),
			   entry_album,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_album),
			   chk_album,
			   FALSE,
			   FALSE,
			   0);
	gtk_table_attach(GTK_TABLE (tag_table), label_album,
			0, 1, 2, 3,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_album,
			1, 2, 2, 3,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_genre),
			   entry_genre,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_genre),
			   chk_genre,
			   FALSE,
			   FALSE,
			   0);
	gtk_table_attach(GTK_TABLE (tag_table), label_genre,
			0, 1, 3, 4,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_genre,
			1, 2, 3, 4,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_tno),
			   entry_tno,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_tno),
			   chk_tno,
			   FALSE,
			   FALSE,
			   0);

	gtk_box_pack_start(GTK_BOX(hbox_year),
			   label_year,
			   FALSE,
			   FALSE,
			   5);
	gtk_box_pack_start(GTK_BOX(hbox_year),
			   entry_year,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_year),
			   chk_year,
			   FALSE,
			   FALSE,
			   0);

	gtk_box_pack_start(GTK_BOX(hbox_spins),
			   hbox_tno,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_spins),
			   hbox_year,
			   TRUE,
			   TRUE,
			   0);

	gtk_table_attach(GTK_TABLE (tag_table), label_tno,
			0, 1, 4, 5,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_spins,
			1, 2, 4, 5,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	comment_view_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(comment_view_scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(comment_view_scroll),
					GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(comment_view_scroll), entry_comment);

	chk_alignment = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(chk_alignment), chk_comment);

	gtk_box_pack_start(GTK_BOX(hbox_comment),
			   comment_view_scroll,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox_comment),
			   chk_alignment,
			   FALSE,
			   FALSE,
			   0);
	gtk_table_attach(GTK_TABLE (tag_table), label_comment,
			0, 1, 5, 7,
			GTK_FILL, GTK_FILL|GTK_EXPAND,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_comment,
			1, 2, 5, 7,
			GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
			0, 0);

	gtk_table_attach(GTK_TABLE (tag_table), label_file,
			0, 1, 7, 8,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), entry_file,
			1, 2, 7, 8,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	/* The main edit dialog */

	dialog = gtk_dialog_new_with_buttons(_("Edit tags"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, -1);

	/* Add to the dialog's main vbox */

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), tag_table);

	/* Fill in initial entries */

	if (otag->title)
		gtk_entry_set_text(GTK_ENTRY(entry_title), otag->title);
	if (otag->artist)
		gtk_entry_set_text(GTK_ENTRY(entry_artist), otag->artist);
	if (otag->album)
		gtk_entry_set_text(GTK_ENTRY(entry_album), otag->album);
	if (otag->genre)
		gtk_entry_set_text(GTK_ENTRY(entry_genre), otag->genre);
	if (otag->track_no)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry_tno), (int)otag->track_no);
	if (otag->year)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry_year), (int)otag->year);
	if (otag->comment)
		gtk_text_buffer_set_text (buffer, otag->comment, -1);

	if (file) {
		gtk_entry_set_text(GTK_ENTRY(entry_file), file);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Details"), GTK_RESPONSE_HELP);
	}
	else
		gtk_widget_set_sensitive(GTK_WIDGET(entry_file), FALSE);

	g_signal_connect(G_OBJECT(entry_title),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_title);
	g_signal_connect(G_OBJECT(entry_artist),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_artist);
	g_signal_connect(G_OBJECT(entry_album),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_album);
	g_signal_connect(G_OBJECT(entry_genre),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_genre);
	g_signal_connect(G_OBJECT(entry_tno),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_tno);
	g_signal_connect(G_OBJECT(entry_year),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_year);
	g_signal_connect(G_OBJECT(buffer),
			 "changed",
			 G_CALLBACK(check_entry),
			 chk_comment);

	g_signal_connect (G_OBJECT(entry_title),
			"icon-press",
			G_CALLBACK (clear_pressed),
			NULL);
	g_signal_connect (G_OBJECT(entry_artist),
			"icon-press",
			G_CALLBACK (clear_pressed),
			NULL);
	g_signal_connect (G_OBJECT(entry_album),
			"icon-press",
			G_CALLBACK (clear_pressed),
			NULL);
	g_signal_connect (G_OBJECT(entry_genre),
			"icon-press",
			G_CALLBACK (clear_pressed),
			NULL);
	g_signal_connect (G_OBJECT(entry_file),
			"icon-press",
			G_CALLBACK (directory_pressed),
			file);

	gtk_widget_show_all(dialog);

	while ((result = gtk_dialog_run (GTK_DIALOG (dialog))) &&
		(result != GTK_RESPONSE_CANCEL) &&
		(result != GTK_RESPONSE_OK) &&
		(result != GTK_RESPONSE_DELETE_EVENT)) {

		if(result == GTK_RESPONSE_HELP){
			if (g_str_has_prefix(file, "cdda://"))
				mobj = new_musicobject_from_cdda(cwin, otag->track_no);
			else {
				uri = sanitize_string_sqlite3(file);

				if ((location_id = find_location_db(uri, cwin)))
					mobj = new_musicobject_from_db(location_id, cwin);
				else
					mobj = new_musicobject_from_file(file);
			}
			track_properties(mobj, cwin);
		}
	}


	switch (result)
	{
	case GTK_RESPONSE_OK:
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_tno))) {
			ntag->track_no =
				gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(entry_tno));
			changed |= TAG_TNO_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_title))) {
			ntag->title =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_title)));
			changed |= TAG_TITLE_CHANGED;
		}
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_artist))) {
			ntag->artist =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_artist)));
			changed |= TAG_ARTIST_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_album))) {
			ntag->album =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_album)));
			changed |= TAG_ALBUM_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_genre))) {
			ntag->genre =
				g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_genre)));
			changed |= TAG_GENRE_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_year))) {
			ntag->year =
				gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(entry_year));
			changed |= TAG_YEAR_CHANGED;
		}
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_comment))) {
			gtk_text_buffer_get_start_iter (buffer, &start);
			gtk_text_buffer_get_end_iter (buffer, &end);
			ntag->comment = g_strdup(gtk_text_buffer_get_text (buffer, &start, &end, FALSE));
			changed |= TAG_COMMENT_CHANGED;
		}
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}
	gtk_widget_destroy(dialog);

	if (mobj)
		delete_musicobject(mobj);
	g_free(uri);

	return changed;
}

/* Edit tags for selected track(s) */

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	struct musicobject *mobj = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path = NULL, *path_current = NULL;
	GtkTreeIter iter;
	GList *list, *i;
	GArray *loc_arr = NULL, *file_arr = NULL;
	gint sel = 0, location_id, changed = 0, j = 0;
	gchar *sfile = NULL, *tfile;

	memset(&otag, 0, sizeof(struct tags));
	memset(&ntag, 0, sizeof(struct tags));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cwin->current_playlist));
	sel = gtk_tree_selection_count_selected_rows(selection);

	if (!sel)
		return;

	list = gtk_tree_selection_get_selected_rows(selection, &model);
	path_current = current_playlist_get_actual(cwin);

	/* Setup initial entries */

	if (sel == 1) {
		path = list->data;

		if (!gtk_tree_model_get_iter(model, &iter, path))
			goto exit;

		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (!mobj) {
			g_warning("Invalid mobj pointer");
			goto exit;
		}

		memcpy(&otag, mobj->tags, sizeof(struct tags));
		changed = tag_edit_dialog(&otag, &ntag, mobj->file, cwin);
	}
	else {
		changed = tag_edit_dialog(&otag, &ntag, NULL, cwin);
	}

	if (!changed)
		goto exit;

	loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
	file_arr = g_array_new(TRUE, TRUE, sizeof(gchar *));

	/* Store the new tags */

	for (i = list; i != NULL; i = i->next) {
		path = i->data;
		mobj = NULL;
		if (!gtk_tree_model_get_iter(model, &iter, path))
			continue;
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (!mobj) {
			g_warning("Invalid mobj pointer");
			continue;
		}

		update_musicobject(mobj, changed, &ntag, cwin);
		update_track_current_playlist(&iter, changed, mobj, cwin);

		if (gtk_tree_path_compare(path, path_current) == 0) {
			update_musicobject(cwin->cstate->curr_mobj, changed, &ntag, cwin);
			if(cwin->cstate->state != ST_STOPPED)
				__update_current_song_info(cwin);
		}

		sfile = sanitize_string_sqlite3(mobj->file);
		location_id = find_location_db(sfile, cwin);
		if (location_id) {
			g_array_append_val(loc_arr, location_id);
			g_free(sfile);
			continue;
		}
		tfile = g_strdup(mobj->file);
		file_arr = g_array_append_val(file_arr, tfile);
		g_free(sfile);
	}

	tag_update(loc_arr, file_arr, changed, &ntag, cwin);

	if (changed && (loc_arr || file_arr))
		init_library_view(cwin);
exit:
	/* Cleanup */

	for (i=list; i != NULL; i = i->next) {
		path = i->data;
		gtk_tree_path_free(path);
	}
	gtk_tree_path_free(path_current);

	if (loc_arr)
		g_array_free(loc_arr, TRUE);
	if (file_arr) {
		gchar *elem = NULL;
		for (j = 0; j < file_arr->len; j++) {
			elem = g_array_index(file_arr, gchar *, j);
			g_free(elem);
		}
		g_array_free(file_arr, TRUE);
	}

	g_free(ntag.title);
	g_free(ntag.artist);
	g_free(ntag.album);
	g_free(ntag.genre);
	g_free(ntag.comment);
	g_list_free(list);
}

void refresh_tag_completion_entries(struct con_win *cwin)
{
	GtkTreeModel *artist_tag_model, *album_tag_model, *genre_tag_model;
	struct db_result result;
	gchar *query;
	gint i = 0;

	artist_tag_model = gtk_entry_completion_get_model(cwin->completion[0]);
	album_tag_model = gtk_entry_completion_get_model(cwin->completion[1]);
	genre_tag_model = gtk_entry_completion_get_model(cwin->completion[2]);

	gtk_list_store_clear(GTK_LIST_STORE(artist_tag_model));
	gtk_list_store_clear(GTK_LIST_STORE(album_tag_model));
	gtk_list_store_clear(GTK_LIST_STORE(genre_tag_model));

	query = g_strdup_printf("SELECT name FROM ARTIST;");
	exec_sqlite_query(query, cwin, &result);

	i = 0;
	for_each_result_row(result, i) {
		add_entry_tag_completion(result.resultp[i], artist_tag_model);

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE)) {
				sqlite3_free_table(result.resultp);
				return;
			}
		}
	}
	sqlite3_free_table(result.resultp);

	query = g_strdup_printf("SELECT name FROM ALBUM;");
	exec_sqlite_query(query, cwin, &result);

	i = 0;
	for_each_result_row(result, i) {
		add_entry_tag_completion(result.resultp[i], album_tag_model);

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE)) {
				sqlite3_free_table(result.resultp);
				return;
			}
		}
	}
	sqlite3_free_table(result.resultp);

	query = g_strdup_printf("SELECT name FROM GENRE;");
	exec_sqlite_query(query, cwin, &result);

	i = 0;
	for_each_result_row(result, i) {
		add_entry_tag_completion(result.resultp[i], genre_tag_model);

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE)) {
				sqlite3_free_table(result.resultp);
				return;
			}
		}
	}
	sqlite3_free_table(result.resultp);
}
