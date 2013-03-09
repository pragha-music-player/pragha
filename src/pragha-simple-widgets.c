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
#include "pragha-utils.h"

/* Generic search entry, valid to library tree, and jump dialog. */

void
search_entry_instant_option_toggled(GtkCheckMenuItem *item, PraghaPreferences *preferences)
{
	gboolean instant_search;

	instant_search = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));

	pragha_preferences_set_instant_search (preferences, instant_search);
}

void
search_entry_approximate_option_toggled(GtkCheckMenuItem *item, PraghaPreferences *preferences)
{
	gboolean approximate_search;
	approximate_search = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));

	pragha_preferences_set_approximate_search (preferences, approximate_search);
}

static void
seach_entry_populate_popup (GtkEntry *entry, PraghaPreferences *preferences)
{
	GtkWidget *popup_menu, *item;
	gboolean instant_search, approximate_search;

	popup_menu = gtk_menu_new ();

	/* Instant search. */

	item = gtk_check_menu_item_new_with_label (_("Refine the search while writing"));
	gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu), item);

	instant_search = pragha_preferences_get_instant_search(preferences);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), instant_search);
	g_signal_connect (G_OBJECT (item), "toggled",
				G_CALLBACK (search_entry_instant_option_toggled), preferences);
	gtk_widget_show (item);

	/* Aproximate search. */

	item = gtk_check_menu_item_new_with_label (_("Search approximate words"));
	gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu), item);

	approximate_search = pragha_preferences_get_approximate_search(preferences);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), approximate_search);
	g_signal_connect (G_OBJECT (item), "toggled",
				G_CALLBACK (search_entry_approximate_option_toggled), preferences);
	gtk_widget_show (item);

	gtk_menu_attach_to_widget(GTK_MENU(popup_menu), GTK_WIDGET(entry), NULL);

	gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL,
			(GtkMenuPositionFunc) menu_position, entry,
			0, gtk_get_current_event_time());
}

void
pragha_search_bar_icon_pressed_cb (GtkEntry       *entry,
                                   gint            position,
                                   GdkEventButton *event,
                                   PraghaPreferences *preferences)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		gtk_entry_set_text (entry, "");
		gtk_widget_grab_focus(GTK_WIDGET(entry));

		if (!pragha_preferences_get_instant_search(preferences))
			g_signal_emit_by_name(G_OBJECT(entry), "activate");
	} else {
		seach_entry_populate_popup(entry, preferences);
	}
}

GtkWidget* pragha_search_entry_new(PraghaPreferences *preferences)
{
	GtkWidget *search_entry;

	search_entry = gtk_entry_new ();

	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);

	gtk_entry_set_icon_sensitive (GTK_ENTRY(search_entry), GTK_ENTRY_ICON_SECONDARY, FALSE);

	/* Signal handlers */

	g_signal_connect (G_OBJECT(search_entry), "icon-press",
			G_CALLBACK (pragha_search_bar_icon_pressed_cb), preferences);

	return search_entry;
}

/* Create a new haeder widget to use in preferences.
 * Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan. */

gpointer sokoke_xfce_header_new(const gchar* header, const gchar *icon)
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
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_widget_modify_fg(label,
				GTK_STATE_NORMAL,
				&gtk_widget_get_style(entry)->text[GTK_STATE_NORMAL]);
        markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", header);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_misc_set_alignment (GTK_MISC(label), 0, 0.5);
	g_free(markup);

	gtk_widget_destroy (entry);

	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(xfce_heading), hbox);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), xfce_heading, FALSE, FALSE, 0);

	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

	return vbox;
}
