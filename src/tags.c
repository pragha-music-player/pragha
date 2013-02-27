/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
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
#include <taginfo/taginfo_c.h>
//#include <taginfo/taginfo.h>

gboolean
pragha_musicobject_set_tags_from_file(PraghaMusicobject *mobj, const gchar *file)
{
	gboolean ret = TRUE;
	TagInfo_Info *tfile = NULL;

	/* workaround for crash in taglib
	   https://github.com/taglib/taglib/issues/78 */
	if (!g_file_test (file, G_FILE_TEST_EXISTS)) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	tfile = taginfo_info_factory_make(file);
	if (!tfile) {
		g_warning("Unable to open file using taglib : %s", file);
		ret = FALSE;
		goto exit;
	}

	if (!taginfo_info_read(tfile)) {
		g_warning("Unable to locate tag in file %s", file);
		ret = FALSE;
		goto exit;
	}

	gchar *albumartist = taginfo_info_get_albumartist(tfile);
	g_object_set (mobj,
	              "title", taginfo_info_get_title(tfile),
	              "artist", taginfo_info_get_artist(tfile),
	              "album", taginfo_info_get_album(tfile),
	              "artist-album", albumartist ? albumartist : "",
	              "genre", taginfo_info_get_genre(tfile),
	              "comment", taginfo_info_get_genre(tfile),
	              "compilation", taginfo_info_get_is_compilation(tfile),
	              "year", taginfo_info_get_year(tfile),
	              "track-no", taginfo_info_get_tracknumber(tfile),
	              "length", taginfo_info_get_length(tfile),
	              "bitrate", taginfo_info_get_bitrate(tfile),
	              "channels", taginfo_info_get_channels(tfile),
	              "samplerate", taginfo_info_get_samplerate(tfile),
	              NULL);

	g_free(albumartist);

exit:
	taginfo_info_free(tfile);

	return ret;
}

gboolean
pragha_musicobject_save_tags_to_file(gchar *file, PraghaMusicobject *mobj, int changed)
{
	gboolean ret = TRUE;
	/*TagLib_File *tfile;
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
	taglib_file_free(tfile);*/

	return ret;
}

/* Show track properties dialog */

static void
pragha_track_properties_response(GtkDialog *dialog,
                                 gint response,
                                 gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
pragha_track_properties_dialog(PraghaMusicobject *mobj,
                               GtkWidget *parent)
{
	GtkWidget *dialog;
	GtkWidget *properties_table;
	GtkWidget *label_length, *label_bitrate, *label_channels, *label_samplerate, *label_folder, *label_filename;
	GtkWidget *info_length, *info_bitrate, *info_channels, *info_samplerate, *info_folder, *info_filename;

	gchar *length = NULL, *bitrate = NULL, *channels = NULL, *samplerate = NULL, *folder = NULL, *filename = NULL;

	if(!mobj)
		return;

	length = convert_length_str(pragha_musicobject_get_length(mobj));
	bitrate = g_strdup_printf("%d kbps", pragha_musicobject_get_bitrate(mobj));
	channels = g_strdup_printf("%d %s", pragha_musicobject_get_channels(mobj), _("Channels"));
	samplerate = g_strdup_printf("%d Hz", pragha_musicobject_get_samplerate(mobj));
	folder = get_display_filename(pragha_musicobject_get_file(mobj), TRUE);
	filename = get_display_name(mobj);

	/* Create table */

	properties_table = gtk_table_new(6, 2, FALSE);

	gtk_table_set_col_spacings(GTK_TABLE(properties_table), 5);
	gtk_table_set_row_spacings(GTK_TABLE(properties_table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(properties_table), 5);

	/* Create labels */

	label_length = gtk_label_new(_("Length"));
	label_bitrate = gtk_label_new(_("Bitrate"));
	label_channels = gtk_label_new(_("Channels"));
	label_samplerate = gtk_label_new(_("Samplerate"));
	label_folder = gtk_label_new(_("Folder"));
	label_filename = gtk_label_new(_("Filename"));

	gtk_misc_set_alignment(GTK_MISC (label_length), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_bitrate), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_channels), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_samplerate), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_folder), 1, 0);
	gtk_misc_set_alignment(GTK_MISC (label_filename), 1, 0);

	gtk_label_set_attribute_bold(GTK_LABEL(label_length));
	gtk_label_set_attribute_bold(GTK_LABEL(label_bitrate));
	gtk_label_set_attribute_bold(GTK_LABEL(label_channels));
	gtk_label_set_attribute_bold(GTK_LABEL(label_samplerate));
	gtk_label_set_attribute_bold(GTK_LABEL(label_folder));
	gtk_label_set_attribute_bold(GTK_LABEL(label_filename));

	/* Create info labels */

	info_length = gtk_label_new(length);
	info_bitrate = gtk_label_new(bitrate);
	info_channels = gtk_label_new(channels);
	info_samplerate = gtk_label_new(samplerate);
	info_folder = gtk_label_new(folder);
	info_filename = gtk_label_new(filename);

	gtk_misc_set_alignment(GTK_MISC (info_length), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_bitrate), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_channels), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_samplerate), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_folder), 0, 0);
	gtk_misc_set_alignment(GTK_MISC (info_filename), 0, 0);

	gtk_label_set_selectable(GTK_LABEL(info_length), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_bitrate), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_channels), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_samplerate), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_folder), TRUE);
	gtk_label_set_selectable(GTK_LABEL(info_filename), TRUE);

	/* Attach labels */

	gtk_table_attach(GTK_TABLE (properties_table), label_length,
			0, 1, 0, 1,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_length,
			1, 2, 0, 1,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_bitrate,
			0, 1, 1, 2,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_bitrate,
			1, 2, 1, 2,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_channels,
			0, 1, 2, 3,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_channels,
			1, 2, 2, 3,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_samplerate,
			0, 1, 3, 4,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_samplerate,
			1, 2, 3, 4,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_folder,
			0, 1, 4, 5,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_folder,
			1, 2, 4, 5,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (properties_table), label_filename,
			0, 1, 5, 6,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (properties_table), info_filename,
			1, 2, 5, 6,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	/* The main edit dialog */

	dialog = gtk_dialog_new_with_buttons(_("Details"),
					     GTK_WINDOW(parent),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), properties_table);

	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(pragha_track_properties_response), NULL);

	gtk_widget_show_all(dialog);

	g_free(length);
	g_free(bitrate);
	g_free(channels);
	g_free(samplerate);
	g_free(folder);
	g_free(filename);
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
                   gchar const *data)
{
	GtkWidget  *toplevel;
	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(entry));

	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gchar *uri = path_get_dir_as_uri (data);
		open_url(uri, toplevel);
		g_free (uri);
	}
}

static void
popup_menu_open_folder (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *toplevel;
	const gchar *file;
	gchar *uri;

	entry_file = g_object_get_data (storage, "entry_file");
	file = gtk_entry_get_text (GTK_ENTRY(entry_file));
	uri = path_get_dir_as_uri (file);

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(entry_file));

	open_url(uri, toplevel);
	g_free (uri);
}

static void
popup_menu_selection_to_title (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *entry_title;
	gint start_sel, end_sel;
	gchar *clip = NULL;

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), &start_sel, &end_sel))
		return;
	clip = gtk_editable_get_chars (GTK_EDITABLE(entry_file), start_sel, end_sel);

	entry_title = g_object_get_data (storage, "entry_title");
	gtk_entry_set_text(GTK_ENTRY(entry_title), clip);

	gtk_widget_grab_focus(GTK_WIDGET(entry_title));

	g_free(clip);
}

static void
popup_menu_selection_to_artist (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *entry_artist;
	gint start_sel, end_sel;
	gchar *clip = NULL;

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), &start_sel, &end_sel))
		return;
	clip = gtk_editable_get_chars (GTK_EDITABLE(entry_file), start_sel, end_sel);

	entry_artist = g_object_get_data (storage, "entry_artist");
	gtk_entry_set_text (GTK_ENTRY(entry_artist), clip);

	gtk_widget_grab_focus(GTK_WIDGET(entry_artist));

	g_free (clip);
}

static void
popup_menu_selection_to_album (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *entry_album;
	gint start_sel, end_sel;
	gchar *clip = NULL;

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), &start_sel, &end_sel))
		return;
	clip = gtk_editable_get_chars (GTK_EDITABLE(entry_file), start_sel, end_sel);

	entry_album = g_object_get_data (storage, "entry_album");
	gtk_entry_set_text (GTK_ENTRY(entry_album), clip);

	gtk_widget_grab_focus(GTK_WIDGET(entry_album));

	g_free (clip);
}

static void
popup_menu_selection_to_genre (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *entry_genre;
	gint start_sel, end_sel;
	gchar *clip = NULL;

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), &start_sel, &end_sel))
		return;
	clip = gtk_editable_get_chars (GTK_EDITABLE(entry_file), start_sel, end_sel);

	entry_genre = g_object_get_data (storage, "entry_genre");
	gtk_entry_set_text (GTK_ENTRY(entry_genre), clip);

	gtk_widget_grab_focus(GTK_WIDGET(entry_genre));

	g_free(clip);
}

static void
popup_menu_selection_to_comment (GtkMenuItem *menuitem, gpointer storage)
{
	GtkWidget *entry_file, *entry_comment;
	GtkTextBuffer *buffer;
	gint start_sel, end_sel;
	gchar *clip = NULL;

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), &start_sel, &end_sel))
		return;
	clip = gtk_editable_get_chars (GTK_EDITABLE(entry_file), start_sel, end_sel);

	entry_comment = g_object_get_data (storage, "entry_comment");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry_comment));

	gtk_text_buffer_set_text (buffer, clip, -1);

	gtk_widget_grab_focus(GTK_WIDGET(entry_comment));

	g_free(clip);
}

static void
file_entry_populate_popup (GtkEntry *entry, GtkMenu *menu, gpointer storage)
{
	GtkWidget *submenu, *item;
	GtkWidget *entry_file;

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	item = gtk_menu_item_new_with_mnemonic (_("Selection to"));
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

	item = gtk_menu_item_new_with_label (_("Title"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_selection_to_title), storage);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Artist"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_selection_to_artist), storage);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Album"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_selection_to_album), storage);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Genre"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_selection_to_genre), storage);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Comment"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_selection_to_comment), storage);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);

	gtk_widget_show_all (submenu);

	entry_file = g_object_get_data (storage, "entry_file");
	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry_file), NULL, NULL))
		gtk_widget_set_sensitive (submenu, FALSE);

	item = gtk_menu_item_new_with_mnemonic (_("Open folder"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (popup_menu_open_folder), storage);
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);
}

static gboolean
select_all_on_click(GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   user_data)
{
	gtk_editable_select_region(GTK_EDITABLE(widget), 0, -1);

	return FALSE;
}

gint tag_edit_dialog(PraghaMusicobject *omobj, gint prechanged, PraghaMusicobject *nmobj, struct con_win *cwin)
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
	gpointer storage;

	const gchar *otitle, *oartist, *oalbum, *ogenre, *ocomment, *ofile;
	gint otrack_no, oyear, result, changed = 0;

	/* Create table */

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

	gtk_label_set_attribute_bold(GTK_LABEL(label_title));
	gtk_label_set_attribute_bold(GTK_LABEL(label_artist));
	gtk_label_set_attribute_bold(GTK_LABEL(label_album));
	gtk_label_set_attribute_bold(GTK_LABEL(label_genre));
	gtk_label_set_attribute_bold(GTK_LABEL(label_tno));
	gtk_label_set_attribute_bold(GTK_LABEL(label_year));
	gtk_label_set_attribute_bold(GTK_LABEL(label_comment));
	gtk_label_set_attribute_bold(GTK_LABEL(label_file));

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

	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_file), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_DIRECTORY);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_file), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_JUMP_TO);

	gtk_editable_set_editable (GTK_EDITABLE(entry_file), FALSE);

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

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 300);

	/* Add to the dialog's main vbox */

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), tag_table, TRUE, TRUE, 0);

	/* Fill in initial entries */

	otitle = pragha_musicobject_get_title(omobj);
	oartist = pragha_musicobject_get_artist(omobj);
	oalbum = pragha_musicobject_get_album(omobj);
	ogenre = pragha_musicobject_get_genre(omobj);
	otrack_no = pragha_musicobject_get_track_no(omobj);
	oyear = pragha_musicobject_get_year(omobj);
	ocomment = pragha_musicobject_get_comment(omobj);
	ofile = pragha_musicobject_get_file(omobj);

	if (otitle)
		gtk_entry_set_text(GTK_ENTRY(entry_title), otitle);
	if (oartist)
		gtk_entry_set_text(GTK_ENTRY(entry_artist), oartist);
	if (oalbum)
		gtk_entry_set_text(GTK_ENTRY(entry_album), oalbum);
	if (ogenre)
		gtk_entry_set_text(GTK_ENTRY(entry_genre), ogenre);
	if (otrack_no > 0)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry_tno), otrack_no);
	if (oyear > 0)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry_year), oyear);
	if (ocomment)
		gtk_text_buffer_set_text (buffer, ocomment, -1);
	if (ofile) {
		gtk_entry_set_text(GTK_ENTRY(entry_file), ofile);
		gtk_editable_set_position(GTK_EDITABLE(entry_file), g_utf8_strlen(ofile, -1));
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Details"), GTK_RESPONSE_HELP);
	}
	else
		gtk_widget_set_sensitive(GTK_WIDGET(entry_file), FALSE);

	if(prechanged & TAG_TNO_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_tno), TRUE);
	if(prechanged & TAG_TITLE_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_title), TRUE);
	if(prechanged & TAG_ARTIST_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_artist), TRUE);
	if(prechanged & TAG_ALBUM_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_album), TRUE);
	if(prechanged & TAG_GENRE_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_genre), TRUE);
	if(prechanged & TAG_YEAR_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_year), TRUE);
	if(prechanged & TAG_COMMENT_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_comment), TRUE);

	/* Connect to check the save changes when change the entry. */

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

	g_signal_connect(G_OBJECT(entry_tno),
			  "button-release-event",
			 G_CALLBACK(select_all_on_click),
			 NULL);
	g_signal_connect(G_OBJECT(entry_year),
			  "button-release-event",
			 G_CALLBACK(select_all_on_click),
			 NULL);

	/* Save changes when press enter. */
	
	gtk_entry_set_activates_default (GTK_ENTRY(entry_title), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_artist), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_album), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_genre), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_tno), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_year), TRUE);

	/* Connect to set clear check when click the icon. */

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
	if(ofile) {
		g_signal_connect (G_OBJECT(entry_file),
				"icon-press",
				G_CALLBACK (directory_pressed),
				(gpointer)ofile);
	}

	/* Genereate storage of gtk_entry and cwin,
	 *  and add popup menu to copy selection to tags. */

	storage = g_object_new(G_TYPE_OBJECT, NULL);
	g_object_set_data(storage, "entry_title", entry_title);
	g_object_set_data(storage, "entry_artist", entry_artist);
	g_object_set_data(storage, "entry_album", entry_album);
	g_object_set_data(storage, "entry_genre", entry_genre);
	g_object_set_data(storage, "entry_comment", entry_comment);
	g_object_set_data(storage, "entry_file", entry_file);

	g_signal_connect (G_OBJECT(entry_file),
			"populate-popup",
			G_CALLBACK (file_entry_populate_popup),
			storage);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_widget_show_all(dialog);

	while ((result = gtk_dialog_run (GTK_DIALOG (dialog))) &&
		(result != GTK_RESPONSE_CANCEL) &&
		(result != GTK_RESPONSE_OK) &&
		(result != GTK_RESPONSE_DELETE_EVENT)) {

		if(result == GTK_RESPONSE_HELP){
			pragha_track_properties_dialog(omobj, cwin->mainwindow);
		}
	}

	switch (result)
	{
	case GTK_RESPONSE_OK:
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_tno))) {
			pragha_musicobject_set_track_no(nmobj, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(entry_tno)));
			changed |= TAG_TNO_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_title))) {
			pragha_musicobject_set_title(nmobj,
				gtk_entry_get_text(GTK_ENTRY(entry_title)));
			changed |= TAG_TITLE_CHANGED;
		}
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_artist))) {
			pragha_musicobject_set_artist(nmobj,
				gtk_entry_get_text(GTK_ENTRY(entry_artist)));
			changed |= TAG_ARTIST_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_album))) {
			pragha_musicobject_set_album(nmobj,
				gtk_entry_get_text(GTK_ENTRY(entry_album)));
			changed |= TAG_ALBUM_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_genre))) {
			pragha_musicobject_set_genre(nmobj,
				gtk_entry_get_text(GTK_ENTRY(entry_genre)));
			changed |= TAG_GENRE_CHANGED;
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_year))) {
			pragha_musicobject_set_year(nmobj,
				gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(entry_year)));
			changed |= TAG_YEAR_CHANGED;
		}
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk_comment))) {
			gtk_text_buffer_get_start_iter (buffer, &start);
			gtk_text_buffer_get_end_iter (buffer, &end);
			pragha_musicobject_set_comment(nmobj,
				gtk_text_buffer_get_text (buffer, &start, &end, FALSE));
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

	g_object_unref(storage);

	return changed;
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
	GList *rlist, *mlist, *tlist;

	clear_sort_current_playlist_cb(NULL, cwin->cplaylist);

	pragha_playlist_set_changing(cwin->cplaylist, TRUE);
	rlist = pragha_playlist_get_selection_ref_list(cwin->cplaylist);
	mlist = pragha_playlist_get_selection_mobj_list(cwin->cplaylist);

	/* Update all mobj selected minus which provide the information. */
	mlist = g_list_remove (mlist, omobj);
	pragha_update_mobj_list_change_tag(mlist, changed, omobj);

	/* Update the view. */
	pragha_playlist_update_ref_list_change_tag(cwin->cplaylist, rlist, changed);
	pragha_playlist_set_changing(cwin->cplaylist, FALSE);

	/* If change current song, update gui and mpris. */
	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	tlist = g_list_find (mlist, cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);
	if(tlist) {
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
	GList *rlist, *mlist, *tlist;
	gint sel = 0, changed = 0;

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
	pragha_playlist_update_ref_list_change_tag(cwin->cplaylist, rlist, changed);

	/* If change current song, update gui and mpris. */
	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	tlist = g_list_find (mlist, cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);
	if(tlist) {
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
