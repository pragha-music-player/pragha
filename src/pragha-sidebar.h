/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>                        */
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

#ifndef PRAGHA_SIDEBAR_H
#define PRAGHA_SIDEBAR_H

#include <gtk/gtk.h>

typedef struct _PraghaSidebar PraghaSidebar;

void           pragha_sidebar_header_set_text (PraghaSidebar *sidebar, const gchar *text);
void           pragha_sidebar_attach_menu     (PraghaSidebar *sidebar, GtkMenu *menu);
void           pragha_sidebar_add_pane        (PraghaSidebar *sidebar, GtkWidget *widget);
GtkWidget     *pragha_sidebar_get_widget      (PraghaSidebar *sidebar);

void           pragha_sidebar_free            (PraghaSidebar *sidebar);
PraghaSidebar *pragha_sidebar_new             (void);

#endif /* PRAGHA_SIDEBAR_H */