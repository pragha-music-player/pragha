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

/* Use the download info on glyr thread and show a dialog. */

static void
pragha_text_info_dialog_response (GtkDialog *dialog,
                                  gint       response,
                                  gpointer   data)
{
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

void
pragha_show_related_text_info_dialog (GtkWidget   *widget,
                                      const gchar *title_header,
                                      const gchar *subtitle_header,
                                      const gchar *text)
{
	PraghaHeader *header;
	GtkWidget *dialog, *vbox, *view, *scrolled;
	GtkTextBuffer *buffer;

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_set_text (buffer, text, -1);

	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrolled),
	                                     GTK_SHADOW_IN);

	gtk_container_set_border_width (GTK_CONTAINER (scrolled), 8);

	dialog = gtk_dialog_new_with_buttons (title_header,
	                                      GTK_WINDOW(widget),
	                                      GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Ok"), GTK_RESPONSE_OK,
	                                      NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	header = pragha_header_new ();
	pragha_header_set_title (header, subtitle_header);
	pragha_header_set_icon_name (header, "media-optical");

	vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(header), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(pragha_text_info_dialog_response), NULL);

	gtk_widget_show_all(dialog);
}
