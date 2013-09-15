/*
 * Copyright (C) 2009-2013 matias <mati86dl@gmail.com>
 * Copyright (C) 2013 Pavel Vasin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-search-entry.h"
#include "pragha-utils.h"

/* Generic search entry, valid to library tree, and jump dialog. */

static void
seach_entry_populate_popup (GtkEntry *entry, PraghaPreferences *preferences)
{
	GtkWidget *popup_menu, *item;
	const GBindingFlags binding_flags = G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	popup_menu = gtk_menu_new ();

	/* Instant search. */

	item = gtk_check_menu_item_new_with_label (_("Refine the search while writing"));
	gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu), item);
	g_object_bind_property (preferences, "instant-search", item, "active", binding_flags);
	gtk_widget_show (item);

	/* Aproximate search. */

	item = gtk_check_menu_item_new_with_label (_("Search approximate words"));
	gtk_menu_shell_append (GTK_MENU_SHELL (popup_menu), item);
	g_object_bind_property (preferences, "approximate-searches", item, "active", binding_flags);
	gtk_widget_show (item);

	gtk_menu_attach_to_widget(GTK_MENU(popup_menu), GTK_WIDGET(entry), NULL);

	gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL,
			(GtkMenuPositionFunc) menu_position, entry,
			0, gtk_get_current_event_time());
}

static void
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

GtkWidget *
pragha_search_entry_new (PraghaPreferences *preferences)
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
