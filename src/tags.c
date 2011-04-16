/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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
	gchar *query = NULL, *stitle = NULL, *sartist = NULL;
	gchar *salbum = NULL, *sgenre = NULL, *file = NULL;
	gint i = 0, artist_id = 0, album_id = 0, genre_id = 0, year_id = 0;
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
}

/* Layout idea: Easytag */

gint tag_edit_dialog(struct tags *otag, struct tags *ntag,
		     struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *label_tno, *label_title, *label_artist;
	GtkWidget *label_album, *label_genre, *label_year;
	GtkWidget *chk_tno, *chk_title, *chk_artist, *chk_album;
	GtkWidget *chk_genre, *chk_year;
	GtkWidget *entry_tno, *entry_title, *entry_artist;
	GtkWidget *entry_album, *entry_genre, *entry_year;
	GtkWidget *vbox_label, *vbox_entry, *vbox_chk;
	GtkWidget *hbox_all;
	gint result, changed = 0;
	gchar *year = NULL, *tno = NULL;

	/* Create labels */
	label_tno = gtk_label_new(_("Track No"));
	label_title = gtk_label_new(_("Title"));
	label_artist = gtk_label_new(_("Artist"));
	label_album = gtk_label_new(_("Album"));
	label_genre = gtk_label_new(_("Genre"));
	label_year = gtk_label_new(_("Year"));

	/* Create entry fields */

	entry_tno = gtk_entry_new();
	entry_title = gtk_entry_new();
	entry_artist = gtk_entry_new();
	entry_album = gtk_entry_new();
	entry_genre = gtk_entry_new();
	entry_year = gtk_entry_new();

	gtk_entry_set_max_length(GTK_ENTRY(entry_tno), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_title), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_artist), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_album), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_genre), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_year), TAG_MAX_LEN);

	gtk_entry_set_completion(GTK_ENTRY(entry_artist), cwin->completion[0]);
	gtk_entry_set_completion(GTK_ENTRY(entry_album), cwin->completion[1]);
	gtk_entry_set_completion(GTK_ENTRY(entry_genre), cwin->completion[2]);

	g_signal_connect(G_OBJECT(entry_tno), "key-press-event",
			 G_CALLBACK(entry_validate_cb), cwin);
	g_signal_connect(G_OBJECT(entry_year), "key-press-event",
			 G_CALLBACK(entry_validate_cb), cwin);

	/* Create checkboxes */

	chk_tno = gtk_check_button_new();
	chk_title = gtk_check_button_new();
	chk_artist = gtk_check_button_new();
	chk_album = gtk_check_button_new();
	chk_genre = gtk_check_button_new();
	chk_year = gtk_check_button_new();

	/* Create hboxes */

	vbox_label = gtk_vbox_new(TRUE, 2);
	vbox_entry = gtk_vbox_new(TRUE, 2);
	vbox_chk = gtk_vbox_new(TRUE, 2);

	/* Fill vboxes */

	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_tno,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_title,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_artist,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_album,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_genre,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_label),
			   label_year,
			   FALSE,
			   FALSE,
			   2);

	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_tno,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_title,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_artist,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_album,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_genre,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_entry),
			   entry_year,
			   FALSE,
			   FALSE,
			   2);

	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_tno,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_title,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_artist,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_album,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_genre,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(vbox_chk),
			   chk_year,
			   FALSE,
			   FALSE,
			   2);

	/* Add all elements to a hbox */

	hbox_all = gtk_hbox_new(FALSE, 2);

	gtk_box_pack_start(GTK_BOX(hbox_all),
			   vbox_label,
			   FALSE,
			   FALSE,
			   2);
	gtk_box_pack_start(GTK_BOX(hbox_all),
			   vbox_entry,
			   TRUE,
			   TRUE,
			   2);
	gtk_box_pack_start(GTK_BOX(hbox_all),
			   vbox_chk,
			   FALSE,
			   FALSE,
			   2);

	/* The main edit dialog */

	dialog = gtk_dialog_new_with_buttons("Edit tags",
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     GTK_STOCK_CANCEL,
					     GTK_RESPONSE_CANCEL,
					     NULL);

	/* Add to the dialog's main vbox */

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox_all);

	/* Fill in initial entries */

	if (otag->track_no) {
		tno = g_strdup_printf("%d", otag->track_no);
		gtk_entry_set_text(GTK_ENTRY(entry_tno), tno);
	}
	if (otag->title)
		gtk_entry_set_text(GTK_ENTRY(entry_title), otag->title);
	if (otag->artist)
		gtk_entry_set_text(GTK_ENTRY(entry_artist), otag->artist);
	if (otag->album)
		gtk_entry_set_text(GTK_ENTRY(entry_album), otag->album);
	if (otag->genre)
		gtk_entry_set_text(GTK_ENTRY(entry_genre), otag->genre);
	if (otag->year) {
		year = g_strdup_printf("%d", otag->year);
		gtk_entry_set_text(GTK_ENTRY(entry_year), year);
	}

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case GTK_RESPONSE_OK:
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_tno))) {
			ntag->track_no =
				atoi(gtk_entry_get_text(GTK_ENTRY(entry_tno)));
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
				atoi(gtk_entry_get_text(GTK_ENTRY(entry_year)));
			changed |= TAG_YEAR_CHANGED;
		}
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}

	g_free(tno);
	g_free(year);
	gtk_widget_destroy(dialog);

	return changed;
}

/* Edit tags for selected track(s) */

void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin)
{
	struct tags otag, ntag;
	struct musicobject *mobj = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
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
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (!sel)
		return;

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
	}

	/* Prompt the user for tag changes */

	changed = tag_edit_dialog(&otag, &ntag, cwin);
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

	/* NB: Optimize this to just refresh the changed tracks */

	if (changed && (loc_arr || file_arr))
		remove_current_playlist(NULL, cwin);

	if (changed && loc_arr) {
		gint elem = 0;
		for (j = 0; j < loc_arr->len; j++) {
			elem = g_array_index(loc_arr, gint, j);
			if (elem) {
				mobj = new_musicobject_from_db(elem, cwin);
				if (!mobj)
					g_critical("Invalid location ID");
				else
					append_current_playlist(mobj, cwin);
			}
		}
	}

	if (changed && file_arr) {
		gchar *elem = NULL;
		for (j = 0; j < file_arr->len; j++) {
			elem = g_array_index(file_arr, gchar *, j);
			if (elem) {
				mobj = new_musicobject_from_file(elem);
				if (!mobj)
					g_critical("Invalid File");
				else
					append_current_playlist(mobj, cwin);
			}
		}
	}

	if (changed && (loc_arr || file_arr))
		init_library_view(cwin);
exit:
	/* Cleanup */

	for (i=list; i != NULL; i = i->next) {
		path = i->data;
		gtk_tree_path_free(path);
	}

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
