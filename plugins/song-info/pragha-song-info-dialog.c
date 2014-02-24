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
#include <glib/gstdio.h>

#include "pragha-song-info-dialog.h"

#include "src/pragha-simple-widgets.h"

typedef struct {
	GtkWidget     *dialog;
	GtkTextBuffer *buffer;

	GlyrMemCache  *head;
	GlyrMemCache  *head_p;
} PraghaSongInfoDialog;

/* Use the download info on glyr thread and show a dialog. */

static void
pragha_text_info_dialog_clean (PraghaSongInfoDialog *dialog_s)
{
	glyr_free_list (dialog_s->head);
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

	gtk_text_buffer_set_text (dialog_s->buffer, head->data, -1);

	gtk_window_set_title (GTK_WINDOW(dialog_s->dialog), title);
	g_free (title);

	dialog_s->head_p = head;
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
			if (dialog_s->head_p->next) {
				pragha_text_info_dialog_set_info (dialog_s, dialog_s->head_p->next);
			}
			else {
				pragha_text_info_dialog_set_info (dialog_s, dialog_s->head);
			}
			break;
		case GTK_RESPONSE_OK:
		default:
			pragha_text_info_dialog_clean (dialog_s);
			gtk_widget_destroy (GTK_WIDGET(dialog));
			break;
	}
}

void
pragha_show_related_text_info_dialog (GtkWidget    *parent,
                                      const gchar  *title,
                                      GlyrMemCache *head)
{
	GtkWidget *dialog, *vbox, *header, *view, *scrolled;

	PraghaSongInfoDialog *dialog_s;
	dialog_s = g_slice_new0(PraghaSongInfoDialog);

	dialog_s->head = head;
	dialog_s->head_p = head;

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	dialog_s->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

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

	if (head->next)
		gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Next"), GTK_RESPONSE_YES);
	gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Ok"), GTK_RESPONSE_OK);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = sokoke_xfce_header_new (title, NULL);

	vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_box_pack_start (GTK_BOX(vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

	dialog_s->dialog = dialog;

	pragha_text_info_dialog_set_info (dialog_s, head);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(pragha_text_info_dialog_response), dialog_s);

	gtk_widget_show_all(dialog);
}
