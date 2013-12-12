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

#include "pragha-tags-dialog.h"
#include "pragha-hig.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha.h"

static void     pragha_tags_dialog_dispose            (GObject *object);
static void     pragha_tags_dialog_finalize           (GObject *object);

static void     pragha_tag_entry_change               (GtkEntry *entry, GtkCheckButton *check);
static void     pragha_tag_entry_clear_pressed        (GtkEntry *entry, gint position, GdkEventButton *event);
static void     pragha_tag_entry_directory_pressed    (GtkEntry *entry, gint position, GdkEventButton *event, gpointer user_data);
static gboolean pragha_tag_entry_select_text_on_click (GtkWidget *widget, GdkEvent  *event, gpointer user_data);
static void     pragha_file_entry_populate_menu       (GtkEntry *entry, GtkMenu *menu, gpointer user_data);

GtkEntryCompletion *pragha_tags_get_entry_completion_from_table (const gchar *table);

struct _PraghaTagsDialogClass {
	GtkDialogClass __parent__;
};

struct _PraghaTagsDialog {
	GtkDialog __parent__;

	GtkWidget         *title_entry;
	GtkWidget         *artist_entry;
	GtkWidget         *album_entry;
	GtkWidget         *genre_entry;
	GtkWidget         *track_no_entry;
	GtkWidget         *year_entry;
	GtkWidget         *comment_entry;
	GtkWidget         *file_entry;

	GtkWidget         *title_check_change;
	GtkWidget         *artist_check_change;
	GtkWidget         *album_check_change;
	GtkWidget         *genre_check_change;
	GtkWidget         *track_no_check_change;
	GtkWidget         *year_check_change;
	GtkWidget         *comment_check_change;

	PraghaMusicobject *mobj;
};

G_DEFINE_TYPE (PraghaTagsDialog, pragha_tags_dialog, GTK_TYPE_DIALOG);

static void
pragha_tags_dialog_class_init (PraghaTagsDialogClass *klass)
{
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = pragha_tags_dialog_dispose;
  gobject_class->finalize = pragha_tags_dialog_finalize;
}

static void
pragha_tags_dialog_init (PraghaTagsDialog *dialog)
{
	GtkWidget *tag_table;
	GtkWidget *label_title, *label_artist, *label_album, *label_genre, *label_tno, *label_year, *label_comment, *label_file;
	GtkWidget *chk_title, *chk_artist, *chk_album, *chk_genre, *chk_tno, *chk_year, *chk_comment;
	GtkWidget *entry_title, *entry_artist, *entry_album, *entry_genre,  *entry_tno, *entry_year, *entry_comment, *entry_file;
	GtkWidget *hbox_title, *hbox_artist, *hbox_album, *hbox_genre, *hbox_tno, *hbox_year, *hbox_comment;
	GtkWidget *hbox_spins, *comment_view_scroll, *chk_alignment;
	GtkEntryCompletion *completion;

	/* Set dialog properties */

	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit tags"));
	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 300);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

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

	entry_file = gtk_entry_new();

	gtk_entry_set_max_length(GTK_ENTRY(entry_title), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_artist), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_album), TAG_MAX_LEN);
	gtk_entry_set_max_length(GTK_ENTRY(entry_genre), TAG_MAX_LEN);

	completion = pragha_tags_get_entry_completion_from_table ("ARTIST");
	gtk_entry_set_completion(GTK_ENTRY(entry_artist), completion);
	g_object_unref (completion);

	completion = pragha_tags_get_entry_completion_from_table ("ALBUM");
	gtk_entry_set_completion(GTK_ENTRY(entry_album), completion);
	g_object_unref (completion);

	completion = pragha_tags_get_entry_completion_from_table ("GENRE");
	gtk_entry_set_completion(GTK_ENTRY(entry_genre), completion);
	g_object_unref (completion);

	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_title), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_artist), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_album), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_genre), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_file), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_DIRECTORY);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(entry_file), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_JUMP_TO);

	gtk_editable_set_editable (GTK_EDITABLE(entry_file), FALSE);

	/* Create checkboxes. */

	chk_title = gtk_check_button_new();
	chk_artist = gtk_check_button_new();
	chk_album = gtk_check_button_new();
	chk_genre = gtk_check_button_new();
	chk_year = gtk_check_button_new();
	chk_tno = gtk_check_button_new();
	chk_comment = gtk_check_button_new();

	/* Connect signals. */

	g_signal_connect(G_OBJECT(entry_title),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_title);
	g_signal_connect(G_OBJECT(entry_artist),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_artist);
	g_signal_connect(G_OBJECT(entry_album),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_album);
	g_signal_connect(G_OBJECT(entry_genre),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_genre);
	g_signal_connect(G_OBJECT(entry_tno),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_tno);
	g_signal_connect(G_OBJECT(entry_year),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_year);
	g_signal_connect(G_OBJECT(gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry_comment))),
	                 "changed",
	                 G_CALLBACK(pragha_tag_entry_change), chk_comment);

	g_signal_connect(G_OBJECT(entry_title),
	                 "icon-press",
	                 G_CALLBACK(pragha_tag_entry_clear_pressed), NULL);
	g_signal_connect(G_OBJECT(entry_artist),
	                 "icon-press",
	                 G_CALLBACK(pragha_tag_entry_clear_pressed), NULL);
	g_signal_connect(G_OBJECT(entry_album),
	                 "icon-press",
	                 G_CALLBACK(pragha_tag_entry_clear_pressed), NULL);
	g_signal_connect(G_OBJECT(entry_genre),
	                 "icon-press",
	                 G_CALLBACK(pragha_tag_entry_clear_pressed), NULL);
	g_signal_connect(G_OBJECT(entry_file),
	                 "icon-press",
	                 G_CALLBACK(pragha_tag_entry_directory_pressed), dialog);

	g_signal_connect(G_OBJECT(entry_tno),
	                 "button-release-event",
	                 G_CALLBACK(pragha_tag_entry_select_text_on_click), NULL);
	g_signal_connect(G_OBJECT(entry_year),
	                 "button-release-event",
	                 G_CALLBACK(pragha_tag_entry_select_text_on_click), NULL);

	g_signal_connect (G_OBJECT(entry_file),
	                  "populate-popup",
	                  G_CALLBACK (pragha_file_entry_populate_menu), dialog);

	/* Create boxs and package all. */

	hbox_title = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_artist = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_album = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_genre = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_year = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_tno = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_comment = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	hbox_spins = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

	/* Create hobxs(ENTRY CHECHK) and attach in table */

	gtk_box_pack_start(GTK_BOX(hbox_title), entry_title,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_title), chk_title,
	                   FALSE, FALSE, 0);

	gtk_table_attach(GTK_TABLE (tag_table), label_title,
	                 0, 1, 0, 1,
	                 GTK_FILL, GTK_SHRINK,
	                 0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_title,
	                 1, 2, 0, 1,
	                 GTK_FILL|GTK_EXPAND, GTK_SHRINK,
	                 0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_artist), entry_artist,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_artist), chk_artist,
	                   FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE (tag_table), label_artist,
	                 0, 1, 1, 2,
	                 GTK_FILL, GTK_SHRINK,
	                 0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_artist,
	                 1, 2, 1, 2,
	                 GTK_FILL|GTK_EXPAND, GTK_SHRINK,
	                 0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_album), entry_album,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_album), chk_album,
	                   FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE (tag_table), label_album,
	                 0, 1, 2, 3,
	                 GTK_FILL, GTK_SHRINK,
	                 0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_album,
	                 1, 2, 2, 3,
	                 GTK_FILL|GTK_EXPAND, GTK_SHRINK,
	                 0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_genre), entry_genre,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_genre), chk_genre,
	                   FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE (tag_table), label_genre,
	                 0, 1, 3, 4,
	                 GTK_FILL, GTK_SHRINK,
	                 0, 0);
	gtk_table_attach(GTK_TABLE (tag_table), hbox_genre,
	                 1, 2, 3, 4,
	                 GTK_FILL|GTK_EXPAND, GTK_SHRINK,
	                 0, 0);

	gtk_box_pack_start(GTK_BOX(hbox_tno), entry_tno,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_tno), chk_tno,
	                   FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox_year), label_year,
	                   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox_year), entry_year,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_year), chk_year,
	                   FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox_spins), hbox_tno,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_spins), hbox_year,
	                   TRUE, TRUE, 0);

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
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(comment_view_scroll),
	                                    GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(comment_view_scroll), entry_comment);

	chk_alignment = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(chk_alignment), chk_comment);

	gtk_box_pack_start(GTK_BOX(hbox_comment), comment_view_scroll,
	                   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_comment), chk_alignment,
	                   FALSE, FALSE, 0);
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

	/* Save changes when press enter. */
	
	gtk_entry_set_activates_default (GTK_ENTRY(entry_title), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_artist), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_album), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_genre), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_tno), TRUE);
	gtk_entry_set_activates_default (GTK_ENTRY(entry_year), TRUE);

	/* Storage widgets and its to dialog. */

	dialog->title_entry = entry_title;
	dialog->artist_entry = entry_artist;
	dialog->album_entry = entry_album;
	dialog->genre_entry = entry_genre;
	dialog->track_no_entry = entry_tno;
	dialog->year_entry = entry_year;
	dialog->comment_entry = entry_comment;
	dialog->file_entry = entry_file;

	dialog->title_check_change = chk_title;
	dialog->artist_check_change = chk_artist;
	dialog->album_check_change = chk_album;
	dialog->genre_check_change = chk_genre;
	dialog->track_no_check_change = chk_tno;
	dialog->year_check_change = chk_year;
	dialog->comment_check_change = chk_comment;

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), tag_table, TRUE, TRUE, 0);
	gtk_widget_show_all(tag_table);

	/* Init flags */
	dialog->mobj = pragha_musicobject_new();
}

void
pragha_tags_dialog_set_changed(PraghaTagsDialog *dialog, gint changed)
{
	if(changed & TAG_TNO_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->track_no_check_change), TRUE);
	if(changed & TAG_TITLE_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->title_check_change), TRUE);
	if(changed & TAG_ARTIST_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->artist_check_change), TRUE);
	if(changed & TAG_ALBUM_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->album_check_change), TRUE);
	if(changed & TAG_GENRE_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->genre_check_change), TRUE);
	if(changed & TAG_YEAR_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->year_check_change), TRUE);
	if(changed & TAG_COMMENT_CHANGED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->comment_check_change), TRUE);
}

gint
pragha_tags_dialog_get_changed(PraghaTagsDialog *dialog)
{
	gint changed = 0;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->track_no_check_change)))
		changed |= TAG_TNO_CHANGED;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->title_check_change)))
		changed |= TAG_TITLE_CHANGED;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->artist_check_change)))
		changed |= TAG_ARTIST_CHANGED;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->album_check_change)))
		changed |= TAG_ALBUM_CHANGED;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->genre_check_change)))
		changed |= TAG_GENRE_CHANGED;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->year_check_change)))
		changed |= TAG_YEAR_CHANGED;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->comment_check_change)))
		changed |= TAG_COMMENT_CHANGED;

	return changed;
}

void
pragha_tags_dialog_set_musicobject(PraghaTagsDialog *dialog, PraghaMusicobject *mobj)
{
	const gchar *title, *artist, *album, *genre, *comment, *file;
	gint track_no, year;
	GtkTextBuffer *buffer;

	g_object_unref(dialog->mobj);
	dialog->mobj = pragha_musicobject_dup (mobj);

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);
	album = pragha_musicobject_get_album(mobj);
	genre = pragha_musicobject_get_genre(mobj);
	track_no = pragha_musicobject_get_track_no(mobj);
	year = pragha_musicobject_get_year(mobj);
	comment = pragha_musicobject_get_comment(mobj);
	file = pragha_musicobject_get_file(mobj);

	/* Block changed signal, and force text. */

	g_signal_handlers_block_by_func(G_OBJECT(dialog->title_entry), pragha_tag_entry_change, dialog->title_check_change);
	gtk_entry_set_text(GTK_ENTRY(dialog->title_entry), title);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->title_entry), pragha_tag_entry_change, dialog->title_check_change);

	g_signal_handlers_block_by_func(G_OBJECT(dialog->artist_entry), pragha_tag_entry_change, dialog->artist_check_change);
	gtk_entry_set_text(GTK_ENTRY(dialog->artist_entry), artist);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->artist_entry), pragha_tag_entry_change, dialog->artist_check_change);

	g_signal_handlers_block_by_func(G_OBJECT(dialog->album_entry), pragha_tag_entry_change, dialog->album_check_change);
	gtk_entry_set_text(GTK_ENTRY(dialog->album_entry), album);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->album_entry), pragha_tag_entry_change, dialog->album_check_change);

	g_signal_handlers_block_by_func(G_OBJECT(dialog->genre_entry), pragha_tag_entry_change, dialog->genre_check_change);
	gtk_entry_set_text(GTK_ENTRY(dialog->genre_entry), genre);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->genre_entry), pragha_tag_entry_change, dialog->genre_check_change);

	g_signal_handlers_block_by_func(G_OBJECT(dialog->track_no_entry), pragha_tag_entry_change, dialog->track_no_check_change);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->track_no_entry), track_no);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->track_no_entry), pragha_tag_entry_change, dialog->track_no_check_change);

	g_signal_handlers_block_by_func(G_OBJECT(dialog->year_entry), pragha_tag_entry_change, dialog->year_check_change);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->year_entry), year);
	g_signal_handlers_unblock_by_func(G_OBJECT(dialog->year_entry), pragha_tag_entry_change, dialog->year_check_change);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->comment_entry));
	g_signal_handlers_block_by_func(G_OBJECT(buffer), pragha_tag_entry_change, dialog->comment_check_change);
	gtk_text_buffer_set_text (buffer, comment, -1);
	g_signal_handlers_unblock_by_func(G_OBJECT(buffer), pragha_tag_entry_change, dialog->comment_check_change);

	if (string_is_empty(file))
		gtk_widget_set_sensitive(GTK_WIDGET(dialog->file_entry), FALSE);
	else {
		gtk_entry_set_text(GTK_ENTRY(dialog->file_entry), file);
		gtk_editable_set_position(GTK_EDITABLE(dialog->file_entry), g_utf8_strlen(file, -1));
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Details"), GTK_RESPONSE_HELP);
	}
}

PraghaMusicobject *
pragha_tags_dialog_get_musicobject(PraghaTagsDialog *dialog)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->track_no_check_change)))
		pragha_musicobject_set_track_no(dialog->mobj,
		                                gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(dialog->track_no_entry)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->title_check_change)))
		pragha_musicobject_set_title(dialog->mobj,
		                             gtk_entry_get_text (GTK_ENTRY(dialog->title_entry)));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->artist_check_change)))
		pragha_musicobject_set_artist(dialog->mobj,
		                              gtk_entry_get_text (GTK_ENTRY(dialog->artist_entry)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->album_check_change)))
		pragha_musicobject_set_album(dialog->mobj,
		                             gtk_entry_get_text (GTK_ENTRY(dialog->album_entry)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->genre_check_change)))
		pragha_musicobject_set_genre(dialog->mobj,
		                             gtk_entry_get_text (GTK_ENTRY(dialog->genre_entry)));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->year_check_change)))
		pragha_musicobject_set_year(dialog->mobj,
		                            gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(dialog->year_entry)));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->comment_check_change))) {
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->comment_entry));
		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		pragha_musicobject_set_comment(dialog->mobj,
		                               gtk_text_buffer_get_text (buffer, &start, &end, FALSE));
	}

	return dialog->mobj;
}

static void
pragha_tags_dialog_dispose (GObject *object)
{
	PraghaTagsDialog *dialog = PRAGHA_TAGS_DIALOG (object);

	if(dialog->mobj) {
		g_object_unref(dialog->mobj);
		dialog->mobj = NULL;
	}

	(*G_OBJECT_CLASS (pragha_tags_dialog_parent_class)->dispose) (object);
}

static void
pragha_tags_dialog_finalize (GObject *object)
{
	//PraghaTagsDialog *dialog = PRAGHA_TAGS_DIALOG (object);

	/*
	 * Need free dialog->loc_arr or dialog->rlist?
	 */

	(*G_OBJECT_CLASS (pragha_tags_dialog_parent_class)->finalize) (object);
}

GtkWidget *
pragha_tags_dialog_new (void)
{
  return g_object_new (PRAGHA_TYPE_TAGS_DIALOG, NULL);
}


/*
 * Track properties dialog
 */

static void
pragha_track_properties_response(GtkDialog *dialog,
                                 gint response,
                                 gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
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

	gint channels_n = pragha_musicobject_get_channels(mobj);

	length = convert_length_str(pragha_musicobject_get_length(mobj));
	bitrate = g_strdup_printf("%d kbps", pragha_musicobject_get_bitrate(mobj));
	channels = g_strdup_printf("%d %s", channels_n, ngettext("channel", "channels", channels_n));
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
					     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
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

/*
 * Track tags editing dialog.
 */

static void
pragha_tag_entry_change (GtkEntry *entry, GtkCheckButton *check)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
}

static void
pragha_tag_entry_clear_pressed (GtkEntry       *entry,
                                gint            position,
                                GdkEventButton *event)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gtk_entry_set_text (entry, "");
		gtk_widget_grab_focus(GTK_WIDGET(entry));
	}
}

static void
pragha_tag_entry_directory_pressed (GtkEntry       *entry,
                                    gint            position,
                                    GdkEventButton *event,
                                    gpointer        user_data)
{
	PraghaMusicobject *mobj;
	GtkWidget *toplevel;

	PraghaTagsDialog *dialog = user_data;

	if (position == GTK_ENTRY_ICON_SECONDARY) {
		mobj = pragha_tags_dialog_get_musicobject(dialog);
		toplevel = gtk_widget_get_toplevel(GTK_WIDGET(entry));

		gchar *uri = path_get_dir_as_uri (pragha_musicobject_get_file(mobj));
		open_url(uri, toplevel);
		g_free (uri);
	}
}

static gboolean
pragha_tag_entry_select_text_on_click (GtkWidget *widget,
                                       GdkEvent  *event,
                                       gpointer   user_data)
{
	gtk_editable_select_region(GTK_EDITABLE(widget), 0, -1);

	return FALSE;
}

gchar *
pragha_tag_entry_get_selected_text(GtkWidget *entry)
{
	gint start_sel, end_sel;

	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(entry), &start_sel, &end_sel))
		return NULL;

	return gtk_editable_get_chars (GTK_EDITABLE(entry), start_sel, end_sel);
}

static void
pragha_tag_entry_set_text(GtkWidget *entry, const gchar *text)
{
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	gtk_widget_grab_focus(GTK_WIDGET(entry));
}

static void
pragha_file_entry_open_folder (GtkMenuItem *menuitem, PraghaTagsDialog *dialog)
{
	GtkWidget *toplevel;
	const gchar *file;
	gchar *uri;

	file = gtk_entry_get_text (GTK_ENTRY(dialog->file_entry));
	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(dialog->file_entry));

	uri = path_get_dir_as_uri (file);
	open_url(uri, toplevel);
	g_free (uri);
}

static void
pragha_file_entry_selection_to_title (GtkMenuItem *menuitem,PraghaTagsDialog *dialog)
{
	gchar *text = pragha_tag_entry_get_selected_text(dialog->file_entry);
	if(text) {
		pragha_tag_entry_set_text(dialog->title_entry, text);
		g_free(text);
	}
}

static void
pragha_file_entry_selection_to_artist (GtkMenuItem *menuitem, PraghaTagsDialog *dialog)
{
	gchar *text = pragha_tag_entry_get_selected_text(dialog->file_entry);
	if(text) {
		pragha_tag_entry_set_text(dialog->artist_entry, text);
		g_free(text);
	}
}

static void
pragha_file_entry_selection_to_album (GtkMenuItem *menuitem, PraghaTagsDialog *dialog)
{
	gchar *text = pragha_tag_entry_get_selected_text(dialog->file_entry);
	if(text) {
		pragha_tag_entry_set_text(dialog->album_entry, text);
		g_free(text);
	}
}

static void
pragha_file_entry_selection_to_genre (GtkMenuItem *menuitem, PraghaTagsDialog *dialog)
{
	gchar *text = pragha_tag_entry_get_selected_text(dialog->file_entry);
	if(text) {
		pragha_tag_entry_set_text(dialog->genre_entry, text);
		g_free(text);
	}
}

static void
pragha_file_entry_selection_to_comment (GtkMenuItem *menuitem, PraghaTagsDialog *dialog)
{
	GtkTextBuffer *buffer;

	gchar *text = pragha_tag_entry_get_selected_text(dialog->file_entry);
	if(text) {
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->comment_entry));
		gtk_text_buffer_set_text (buffer, text, -1);
		g_free(text);
	}
}

static void
pragha_file_entry_populate_menu (GtkEntry *entry, GtkMenu *menu, gpointer user_data)
{
	GtkWidget *submenu, *item;

	PraghaTagsDialog *dialog = user_data;

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	item = gtk_menu_item_new_with_mnemonic (_("Selection to"));
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);

	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

	item = gtk_menu_item_new_with_label (_("Title"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_selection_to_title), dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Artist"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_selection_to_artist), dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Album"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_selection_to_album), dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Genre"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_selection_to_genre), dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
	item = gtk_menu_item_new_with_label (_("Comment"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_selection_to_comment), dialog);
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);

	gtk_widget_show_all (submenu);

	if (!gtk_editable_get_selection_bounds (GTK_EDITABLE(dialog->file_entry), NULL, NULL))
		gtk_widget_set_sensitive (submenu, FALSE);

	item = gtk_menu_item_new_with_mnemonic (_("Open folder"));
	g_signal_connect (G_OBJECT (item),
	                  "activate",
	                  G_CALLBACK (pragha_file_entry_open_folder), dialog);
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
	gtk_widget_show (item);
}

GtkEntryCompletion *
pragha_tags_get_entry_completion_from_table(const gchar *table)
{
	PraghaDatabase *cdbase;
	PraghaPreparedStatement *statement;
	gchar *sql;
	GtkEntryCompletion *completion;
	GtkListStore *model;
	GtkTreeIter iter;

	cdbase = pragha_database_get ();

	model = gtk_list_store_new(1, G_TYPE_STRING);

	sql = g_strdup_printf("SELECT name FROM %s ORDER BY name DESC", table);
	statement = pragha_database_create_statement (cdbase, sql);
	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		gtk_list_store_insert_with_values (GTK_LIST_STORE(model), &iter, 0, 0, name, -1);
	}
	pragha_prepared_statement_free (statement);
	g_object_unref(G_OBJECT(cdbase));
	g_free(sql);

	completion = gtk_entry_completion_new();
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(model));
	gtk_entry_completion_set_text_column(completion, 0);
	g_object_unref(model);

	return completion;
}
