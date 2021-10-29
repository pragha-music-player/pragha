/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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
#include "pragha.h"

gboolean pragha_close_window        (GtkWidget *widget, GdkEvent *event, PraghaApplication *pragha);
void     pragha_destroy_window      (GtkWidget *widget, PraghaApplication *pragha);
void     pragha_window_toggle_state (PraghaApplication *pragha, gboolean ignoreActivity);

void     pragha_window_show_backend_error_dialog (PraghaApplication *pragha);

void     gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, PraghaApplication *pragha);

void     pragha_window_unfullscreen          (GObject *object, PraghaApplication *pragha);

void     pragha_window_add_widget_to_infobox (PraghaApplication *pragha, GtkWidget *widget);

void     pragha_init_gui_state (PraghaApplication *pragha);

void     pragha_window_save_settings (PraghaApplication *pragha);

void     pragha_window_new  (PraghaApplication *pragha);

#endif /* PRAGHA_WINDOW_H */
