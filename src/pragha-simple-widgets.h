/*************************************************************************/
/* Copyright (C) 2009-2018 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_SIMPLE_WIDGETS_H
#define PRAGHA_SIMPLE_WIDGETS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _PraghaHeader        PraghaHeader;
typedef struct _PraghaTrackProgress PraghaTrackProgress;
typedef struct _PraghaContainer     PraghaContainer;
typedef struct _PraghaToolbarButton PraghaToolbarButton;
typedef struct _PraghaToggleButton  PraghaToggleButton;

PraghaHeader *pragha_header_new (void);
void
pragha_header_set_icon_name (PraghaHeader *header,
                             const gchar  *icon_name);
void
pragha_header_set_title (PraghaHeader *header,
                         const gchar  *title);
void
pragha_header_set_subtitle (PraghaHeader *header,
                            const gchar  *subtitle);


void pragha_toolbar_button_set_icon_name (PraghaToolbarButton *button, const gchar *icon_name);
void pragha_toolbar_button_set_icon_size (PraghaToolbarButton *button, GtkIconSize  icon_size);
PraghaToolbarButton *pragha_toolbar_button_new (const gchar *icon_name);

void pragha_toggle_button_set_icon_name (PraghaToggleButton *button, const gchar *icon_name);
void pragha_toggle_button_set_icon_size (PraghaToggleButton *button, GtkIconSize  icon_size);
PraghaToggleButton *pragha_toggle_button_new (const gchar *icon_name);


G_END_DECLS

#endif /* PRAGHA_SIMPLE_WIDGETS_H */
