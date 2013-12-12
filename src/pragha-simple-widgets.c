/*************************************************************************/
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-simple-widgets.h"

/* Create a new haeder widget to use in preferences.
 * Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan. */

gpointer sokoke_xfce_header_new(const gchar* header, const gchar *icon)
{
	GtkWidget* xfce_heading;
	GtkWidget* hbox;
	GtkWidget* vbox;
	GtkWidget* image;
	GtkWidget* label;
	GtkWidget* separator;
	gchar* markup;

	xfce_heading = gtk_event_box_new();

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	if (icon)
		image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
	else
		image = gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_DIALOG);

	label = gtk_label_new(NULL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", header);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_misc_set_alignment (GTK_MISC(label), 0, 0.5);
	g_free(markup);

	gtk_style_context_add_class (gtk_widget_get_style_context (xfce_heading),
	                             GTK_STYLE_CLASS_ENTRY);

	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(xfce_heading), hbox);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (vbox), xfce_heading, FALSE, FALSE, 0);

	separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

	return vbox;
}
