/* gtkcellrendererbubble.h
 *
 * Copyright (C) 2009 - Christian Hergert
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GTK_CELL_RENDERER_BUBBLE_H__
#define __GTK_CELL_RENDERER_BUBBLE_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_CELL_RENDERER_BUBBLE (gtk_cell_renderer_bubble_get_type ())
G_DECLARE_FINAL_TYPE (GtkCellRendererBubble, gtk_cell_renderer_bubble, GTK, CELL_RENDERER_BUBBLE, GtkCellRendererText)

GType                  gtk_cell_renderer_bubble_get_type        (void) G_GNUC_CONST;

GtkCellRendererBubble* gtk_cell_renderer_bubble_new             (void);
gboolean               gtk_cell_renderer_bubble_get_show_bubble (GtkCellRendererBubble *cell);
void                   gtk_cell_renderer_bubble_set_show_bubble (GtkCellRendererBubble *cell,
                                                                 gboolean               show_bubble);

G_END_DECLS

#endif /* __GTK_CELL_RENDERER_BUBBLE_H__ */
