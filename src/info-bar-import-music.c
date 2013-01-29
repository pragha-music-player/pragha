/*
 * Copyright (C) 2012 Pavel Vasin
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

#include "pragha.h"
#include "pragha-scanner.h"

gboolean info_bar_import_music_will_be_useful(struct con_win *cwin)
{
	return cwin->cstate->first_run && g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
}

static void info_bar_response_cb(GtkInfoBar *info_bar, gint response_id, gpointer user_data)
{
	GSList *library_dir = NULL;

	struct con_win *cwin = user_data;
	const gchar *dir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);

	gtk_widget_destroy(GTK_WIDGET(info_bar));

	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_YES:
			pragha_preferences_remove_key(cwin->preferences,
			                              GROUP_LIBRARY,
			                              KEY_LIBRARY_SCANNED);

			library_dir = g_slist_append(library_dir, g_strdup(dir));
			pragha_preferences_set_filename_list(cwin->preferences,
				                             GROUP_LIBRARY,
				                             KEY_LIBRARY_DIR,
				                             library_dir);
			free_str_list(library_dir);

			pragha_scanner_scan_library(cwin->mainwindow);
			break;
		default:
			g_warn_if_reached();
	}
}

GtkWidget * create_info_bar_import_music(struct con_win *cwin)
{
	const gchar *dir = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);

	GtkWidget *info_bar = gtk_info_bar_new();
	GtkWidget *action_area = gtk_info_bar_get_action_area(GTK_INFO_BAR (info_bar));
	GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR(info_bar));

	gtk_orientable_set_orientation(GTK_ORIENTABLE(action_area), GTK_ORIENTATION_HORIZONTAL);

	//GtkInfoBar has undocumented behavior for GTK_RESPONSE_CANCEL
	gtk_info_bar_add_button(GTK_INFO_BAR(info_bar), GTK_STOCK_NO, GTK_RESPONSE_CANCEL);
	gtk_info_bar_add_button(GTK_INFO_BAR(info_bar), GTK_STOCK_YES, GTK_RESPONSE_YES);

	gchar *content = g_strdup_printf(_("Would you like to import %s to library?"), dir);

	GtkWidget *label = gtk_label_new(content);
	gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 0);

	g_signal_connect(info_bar, "response", G_CALLBACK(info_bar_response_cb), cwin);

	gtk_widget_show_all(info_bar);

	g_free(content);

	return info_bar;
}
