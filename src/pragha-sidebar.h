/*************************************************************************/
/* Copyright (C) 2013-2019 matias <mati86dl@gmail.com>                   */
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

#define PRAGHA_TYPE_SIDEBAR            (pragha_sidebar_get_type ())
#define PRAGHA_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SIDEBAR, PraghaSidebar))
#define PRAGHA_IS_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SIDEBAR))
#define PRAGHA_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_SIDEBAR, PraghaSidebarClass))
#define PRAGHA_IS_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_SIDEBAR))
#define PRAGHA_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_SIDEBAR, PraghaSidebarClass))

typedef struct _PraghaSidebar PraghaSidebar;

typedef struct {
	GtkScrolledWindowClass    __parent__;
	void (*children_changed) (PraghaSidebar *sidebar);
} PraghaSidebarClass;

void
pragha_sidebar_attach_plugin (PraghaSidebar *sidebar,
                              GtkWidget     *widget,
                              GtkWidget     *title,
                              GtkWidget     *popover);

void
pragha_sidebar_remove_plugin (PraghaSidebar *sidebar,
                              GtkWidget     *widget);

gint
pragha_sidebar_get_n_panes (PraghaSidebar *sidebar);

void
pragha_sidebar_style_position (PraghaSidebar  *sidebar,
                               GtkPositionType position);

PraghaSidebar *pragha_sidebar_new (void);

#endif /* PRAGHA_SIDEBAR_H */
