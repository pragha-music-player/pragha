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

GtkWidget* create_main_region(struct con_win *cwin);
GtkWidget* create_info_box(struct con_win *cwin);
GtkWidget* create_paned_region(struct con_win *cwin);
GtkWidget* create_search_bar(struct con_win *cwin);
GtkWidget* create_combo_order(struct con_win *cwin);
gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin);
void mainwindow_add_widget_to_info_box(struct con_win *cwin, GtkWidget *widget);
void gui_free(struct con_win *cwin);
void gui_backend_error_show_dialog_cb (PraghaBackend *backend, const GError *error, gpointer user_data);
void gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin);


#endif /* PRAGHA_WINDOW_H */