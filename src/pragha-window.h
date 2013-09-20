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

#ifndef PRAGHA_WINDOW_H
#define PRAGHA_WINDOW_H

#include <gtk/gtk.h>
#include "pragha-backend.h"
#include "pragha-toolbar.h"

/* pragha.h */
struct con_win;

gboolean pragha_close_window        (GtkWidget *widget, GdkEvent *event, struct con_win *cwin);
void     pragha_destroy_window      (GtkWidget *widget, struct con_win *cwin);
void     pragha_window_toggle_state (struct con_win *cwin, gboolean ignoreActivity);

void gui_backend_error_show_dialog_cb (PraghaBackend *backend, const GError *error, gpointer user_data);
void gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin);

void       pragha_window_unfullscreen          (GObject *object, struct con_win *cwin);

void       pragha_window_add_widget_to_infobox (struct con_win *cwin, GtkWidget *widget);

gint           pragha_menubar_append_plugin_action (struct con_win *cwin, GtkActionGroup *action_group, const gchar *menu_xml);
void           pragha_menubar_remove_plugin_action (struct con_win *cwin, GtkActionGroup *action_group, gint merge_id);

GtkAction     *pragha_window_get_menu_action     (struct con_win *cwin, const gchar *path);
GtkUIManager  *pragha_window_get_menu_ui_manager (struct con_win *cwin);

GtkWidget     *pragha_window_get_menubar       (struct con_win *cwin);
GtkWidget     *pragha_window_get_mainwindow    (struct con_win *cwin);
PraghaToolbar *pragha_window_get_toolbar       (struct con_win *cwin);
GdkPixbuf     *pragha_window_get_pixbuf_app    (struct con_win *cwin);

void          pragha_window_free (struct con_win *cwin);
void          pragha_window_new  (struct con_win *cwin);

#endif /* PRAGHA_WINDOW_H */