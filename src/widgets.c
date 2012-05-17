/*************************************************************************/
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
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

/* Generic search entry, valid to library tree, and jump dialog. */

static void
pragha_search_bar_icon_pressed_cb (GtkEntry       *entry,
					     gint            position,
					     GdkEventButton *event,
					     struct con_win *cwin)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gtk_entry_set_text (entry, "");
		gtk_widget_grab_focus(GTK_WIDGET(entry));

		if (!cwin->cpref->instant_filter)
			g_signal_emit_by_name (G_OBJECT(entry),
								 "activate", cwin);
	}
}

GtkWidget* pragha_search_entry_new(struct con_win *cwin)
{
	GtkWidget *search_entry;

	search_entry = gtk_entry_new ();

	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

	gtk_entry_set_icon_sensitive (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, FALSE);

	/* Signal handlers */

	g_signal_connect (G_OBJECT(search_entry), "icon-press",
			G_CALLBACK (pragha_search_bar_icon_pressed_cb), cwin);

	return search_entry;
}

/* Create a new haeder widget to use in preferences.
 * Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan. */

gpointer sokoke_xfce_header_new(const gchar* header, const gchar *icon, struct con_win *cwin)
{
	GtkWidget* entry;
	GtkWidget* xfce_heading;
	GtkWidget* hbox;
	GtkWidget* vbox;
	GtkWidget* image;
	GtkWidget* label;
	GtkWidget* separator;
	gchar* markup;

	entry = gtk_entry_new();
	xfce_heading = gtk_event_box_new();

	gtk_widget_modify_bg(xfce_heading,
				GTK_STATE_NORMAL,
				&gtk_widget_get_style(entry)->base[GTK_STATE_NORMAL]);

	hbox = gtk_hbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

        if (icon)
            image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
        else
            image = gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_DIALOG);

	label = gtk_label_new(NULL);
	gtk_widget_modify_fg(label,
				GTK_STATE_NORMAL,
				&gtk_widget_get_style(entry)->text[GTK_STATE_NORMAL]);
        markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", header);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	g_free(markup);
	gtk_widget_destroy (entry);

	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(xfce_heading), hbox);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), xfce_heading, FALSE, FALSE, 0);

	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

	return vbox;
}
