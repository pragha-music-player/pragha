/*************************************************************************/
/* Copyright (C) 2011-2014 matias <mati86dl@gmail.com>                   */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <glyr/glyr.h>
#include <glyr/cache.h>

#include <glib/gstdio.h>

#include "pragha-song-info-dialog.h"

#include "src/pragha-simple-widgets.h"

struct _PraghaSongInfoDialog {
	GtkWidget     *dialog;
	GtkWidget     *text_view;
	GtkTextBuffer *buffer;

	GlyrDatabase  *cache_db;
	GlyrQuery     *query;

	GlyrMemCache  *head;
	GlyrMemCache  *head_p;
};

/* Use the download info on glyr thread and show a dialog. */

static void
pragha_text_info_dialog_clean (PraghaSongInfoDialog *dialog_s)
{
	glyr_free_list (dialog_s->head);
	glyr_query_destroy (dialog_s->query);

	g_slice_free(PraghaSongInfoDialog, dialog_s);
}

static void
pragha_text_info_dialog_set_info (PraghaSongInfoDialog *dialog_s, GlyrMemCache *head)
{
	gchar *title = NULL;

	switch (head->type) {
		case GLYR_TYPE_LYRICS:
			title =  g_strdup_printf(_("Lyrics thanks to %s"), head->prov);
			break;
		case GLYR_TYPE_ARTIST_BIO:
			title = g_strdup_printf(_("Artist info thanks to %s"), head->prov);
			break;
		default:
			break;
	}

	if (g_ascii_strcasecmp(head->prov, _("Edited")) == 0) {
		gtk_text_view_set_editable (GTK_TEXT_VIEW(dialog_s->text_view), TRUE);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(dialog_s->text_view), TRUE);
	}
	else {
		gtk_text_view_set_editable (GTK_TEXT_VIEW(dialog_s->text_view), FALSE);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(dialog_s->text_view), FALSE);
	}

	gtk_text_buffer_set_text (dialog_s->buffer, head->data, -1);

	gtk_window_set_title (GTK_WINDOW(dialog_s->dialog), title);
	g_free (title);

	dialog_s->head_p = head;
}

static void
pragha_song_info_save_cache (PraghaSongInfoDialog *dialog_s)
{
	GlyrMemCache *tmp_head;

	tmp_head = dialog_s->head_p->next;
	dialog_s->head_p->next = NULL;

	/* Removes all previous entries */
	glyr_opt_number (dialog_s->query, 0);
	glyr_db_delete (dialog_s->cache_db, dialog_s->query);

	/* Insert only the new info */
	glyr_db_insert (dialog_s->cache_db, dialog_s->query, dialog_s->head_p);

	dialog_s->head_p->next = tmp_head;
}

static void
pragha_text_info_dialog_edit_info (PraghaSongInfoDialog *dialog_s)
{
	GlyrMemCache *dup, *tmp_head;
	dup = glyr_cache_copy (dialog_s->head_p);
	glyr_cache_set_prov (dup, _("Edited"));

	tmp_head = dialog_s->head_p->next;
	dialog_s->head_p->next = dup;

	dup->prev = dialog_s->head_p;
	dup->next = tmp_head;

	pragha_text_info_dialog_set_info (dialog_s, dup);
}

static void
pragha_song_info_update_edited_cache (PraghaSongInfoDialog *dialog_s)
{
	GtkTextIter start, end;
	const gchar *text = NULL;

	gtk_text_buffer_get_start_iter (dialog_s->buffer, &start);
	gtk_text_buffer_get_end_iter (dialog_s->buffer, &end);

	text = gtk_text_buffer_get_text (dialog_s->buffer, &start, &end, FALSE);

	glyr_cache_set_data (dialog_s->head_p, g_strdup(text), -1);
}

static void
pragha_text_info_dialog_show_next (PraghaSongInfoDialog *dialog_s)
{
	if (g_ascii_strcasecmp (dialog_s->head_p->prov, _("Edited")) == 0)
		pragha_song_info_update_edited_cache(dialog_s);

	if (dialog_s->head_p->next) {
		pragha_text_info_dialog_set_info (dialog_s, dialog_s->head_p->next);
	}
	else {
		pragha_text_info_dialog_set_info (dialog_s, dialog_s->head);
	}
}

static void
pragha_text_info_dialog_response (GtkDialog *dialog,
                                  gint       response,
                                  gpointer   data)
{
	PraghaSongInfoDialog *dialog_s = data;

	switch (response)
	{
		case GTK_RESPONSE_YES:
			pragha_text_info_dialog_show_next (dialog_s);
			break;
		case GTK_RESPONSE_HELP:
			pragha_text_info_dialog_edit_info (dialog_s);
			break;
		case GTK_RESPONSE_OK:
			pragha_song_info_save_cache (dialog_s);
		default:
			pragha_text_info_dialog_clean (dialog_s);
			gtk_widget_destroy (GTK_WIDGET(dialog));
			break;
	}
}

void
pragha_song_info_dialog_show (PraghaSongInfoDialog *dialog,
                              GlyrQuery            *query,
                              GlyrMemCache         *head)
{
	if (head->next)
		gtk_dialog_add_button (GTK_DIALOG (dialog->dialog),
		                       _("_Next"), GTK_RESPONSE_YES);

	dialog->query = query;
	dialog->head = head;
	dialog->head_p = head;

	pragha_text_info_dialog_set_info (dialog, head);

	gtk_widget_show_all (dialog->dialog);
}

PraghaSongInfoDialog *
pragha_song_info_dialog_new (GtkWidget    *parent,
                             const gchar  *title,
                             GlyrDatabase *cache_db)
{
	GtkWidget *dialog, *vbox, *header, *view, *scrolled;

	PraghaSongInfoDialog *dialog_s;
	dialog_s = g_slice_new0(PraghaSongInfoDialog);

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrolled),
	                                     GTK_SHADOW_IN);

	gtk_container_set_border_width (GTK_CONTAINER (scrolled), 8);

	dialog = gtk_dialog_new ();

	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);

	gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Ok"), GTK_RESPONSE_OK);
	gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Edit"), GTK_RESPONSE_HELP);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (title, NULL);

	vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_box_pack_start (GTK_BOX(vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

	dialog_s->dialog = dialog;
	dialog_s->text_view = view;
	dialog_s->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	dialog_s->cache_db = cache_db;

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(pragha_text_info_dialog_response), dialog_s);

	return dialog_s;
}
