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

#define GTK_CELL_RENDERER_BUBBLE(obj)             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),             \
  GTK_TYPE_CELL_RENDERER_BUBBLE,                  \
  GtkCellRendererBubble))

#define GTK_CELL_RENDERER_BUBBLE_CONST(obj)       \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),             \
  GTK_TYPE_CELL_RENDERER_BUBBLE,                  \
  GtkCellRendererBubble const))

#define GTK_CELL_RENDERER_BUBBLE_CLASS(klass)     \
  (G_TYPE_CHECK_CLASS_CAST ((klass),              \
  GTK_TYPE_CELL_RENDERER_BUBBLE,                  \
  GtkCellRendererBubbleClass))

#define GTK_IS_CELL_RENDERER_BUBBLE(obj)          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),             \
  GTK_TYPE_CELL_RENDERER_BUBBLE))

#define GTK_IS_CELL_RENDERER_BUBBLE_CLASS(klass)  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),              \
  GTK_TYPE_CELL_RENDERER_BUBBLE))

#define GTK_CELL_RENDERER_BUBBLE_GET_CLASS(obj)   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),              \
  GTK_TYPE_CELL_RENDERER_BUBBLE,                  \
  GtkCellRendererBubbleClass))

typedef struct _GtkCellRendererBubble         GtkCellRendererBubble;
typedef struct _GtkCellRendererBubbleClass    GtkCellRendererBubbleClass;
typedef struct _GtkCellRendererBubblePrivate  GtkCellRendererBubblePrivate;

struct _GtkCellRendererBubble
{
  GtkCellRendererText parent;
  
  GtkCellRendererBubblePrivate *priv;
};

struct _GtkCellRendererBubbleClass
{
  GtkCellRendererTextClass parent_class;
};

GType            gtk_cell_renderer_bubble_get_type        (void) G_GNUC_CONST;
GtkCellRenderer* gtk_cell_renderer_bubble_new             (void);
gboolean         gtk_cell_renderer_bubble_get_show_bubble (GtkCellRendererBubble *cell);
void             gtk_cell_renderer_bubble_set_show_bubble (GtkCellRendererBubble *cell,
                                                           gboolean               show_bubble);

G_END_DECLS

#endif /* __GTK_CELL_RENDERER_BUBBLE_H__ */
