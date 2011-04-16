/* gtkcellrendererbubble.c
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

#include "gtkcellrendererbubble.h"

struct _GtkCellRendererBubblePrivate
{
  gboolean show_bubble;
};

enum
{
  PROP_0,
  PROP_SHOW_BUBBLE,
};

G_DEFINE_TYPE (GtkCellRendererBubble,
               gtk_cell_renderer_bubble,
               GTK_TYPE_CELL_RENDERER_TEXT);

static void
gtk_cell_renderer_bubble_finalize (GObject *object)
{
  G_OBJECT_CLASS (gtk_cell_renderer_bubble_parent_class)->finalize (object);
}

gboolean
gtk_cell_renderer_bubble_get_show_bubble (GtkCellRendererBubble *cell)
{
  g_return_val_if_fail (GTK_IS_CELL_RENDERER_BUBBLE (cell), FALSE);
  return cell->priv->show_bubble;
}

void
gtk_cell_renderer_bubble_set_show_bubble (GtkCellRendererBubble *cell,
                                          gboolean               show_bubble)
{
  g_return_if_fail (GTK_IS_CELL_RENDERER_BUBBLE (cell));
  cell->priv->show_bubble = show_bubble;
}

static void
gtk_cell_renderer_bubble_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_SHOW_BUBBLE:
    g_value_set_boolean (value,
      gtk_cell_renderer_bubble_get_show_bubble (GTK_CELL_RENDERER_BUBBLE (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
gtk_cell_renderer_bubble_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  switch (property_id) {
  case PROP_SHOW_BUBBLE:
    gtk_cell_renderer_bubble_set_show_bubble (GTK_CELL_RENDERER_BUBBLE (object),
                                              g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
rounded_rectangle (cairo_t *cr,
                   gint     x,
                   gint     y,
                   gint     width,
                   gint     height,
                   gint     x_radius,
                   gint     y_radius)
{
  gint x1, x2;
  gint y1, y2;
  gint xr1, xr2;
  gint yr1, yr2;
  
  x1 = x;
  x2 = x1 + width;
  y1 = y;
  y2 = y1 + height;
  
  x_radius = MIN (x_radius, width / 2.0);
  y_radius = MIN (y_radius, width / 2.0);
  
  xr1 = x_radius;
  xr2 = x_radius / 2.0;
  yr1 = y_radius;
  yr2 = y_radius / 2.0;
  
  cairo_move_to    (cr, x1 + xr1, y1);
  cairo_line_to    (cr, x2 - xr1, y1);
  cairo_curve_to   (cr, x2 - xr2, y1, x2, y1 + yr2, x2, y1 + yr1);
  cairo_line_to    (cr, x2, y2 - yr1);
  cairo_curve_to   (cr, x2, y2 - yr2, x2 - xr2, y2, x2 - xr1, y2);
  cairo_line_to    (cr, x1 + xr1, y2);
  cairo_curve_to   (cr, x1 + xr2, y2, x1, y2 - yr2, x1, y2 - yr1);
  cairo_line_to    (cr, x1, y1 + yr1);
  cairo_curve_to   (cr, x1, y1 + yr2, x1 + xr2, y1, x1 + xr1, y1);
  cairo_close_path (cr);
}

static void
render (GtkCellRenderer      *cell,
        GdkDrawable          *window,
        GtkWidget            *widget,
        GdkRectangle         *background_area,
        GdkRectangle         *cell_area,
        GdkRectangle         *expose_area,
        GtkCellRendererState  flags)
{
  GtkCellRendererBubblePrivate *priv;
  cairo_t *cr;
  cairo_pattern_t *pattern;
  GdkColor *start;
  GdkColor *stop;
  
  g_return_if_fail (GTK_IS_CELL_RENDERER_BUBBLE (cell));
  
  priv = GTK_CELL_RENDERER_BUBBLE (cell)->priv;
  
  if (priv->show_bubble)
    {
      cr = gdk_cairo_create (GDK_DRAWABLE (window));
      g_assert (cr);
      
      start = &widget->style->light [GTK_STATE_SELECTED];
      stop = &widget->style->mid [GTK_STATE_SELECTED];
      
      pattern = cairo_pattern_create_linear (cell_area->x,
                                             cell_area->y,
                                             cell_area->x,
                                             cell_area->y + cell_area->height);
      
      cairo_pattern_add_color_stop_rgb (pattern, 0.3,
                                        start->red / (gdouble)G_MAXUINT16,
                                        start->green / (gdouble)G_MAXUINT16,
                                        start->blue / (gdouble)G_MAXUINT16);
      cairo_pattern_add_color_stop_rgb (pattern, 0.9,
                                        stop->red / (gdouble)G_MAXUINT16,
                                        stop->green / (gdouble)G_MAXUINT16,
                                        stop->blue / (gdouble)G_MAXUINT16);
      
      rounded_rectangle (cr,
                         cell_area->x, cell_area->y + 1,
                         cell_area->width, cell_area->height - 2,
                         cell_area->height / 2.5, cell_area->height / 2.5);
      
      cairo_set_source (cr, pattern);
      cairo_fill_preserve (cr);
      
      gdk_cairo_set_source_color (cr, &widget->style->dark [GTK_STATE_SELECTED]);
      cairo_set_line_width (cr, 1.0);
      cairo_stroke (cr);
      
      rounded_rectangle (cr,
                         cell_area->x + 1.0, cell_area->y + 2.0,
                         cell_area->width - 2.0, cell_area->height - 4.0,
                         cell_area->height / 2.5, cell_area->height / 2.5);
      gdk_cairo_set_source_color (cr, &widget->style->light [GTK_STATE_SELECTED]);
      cairo_stroke (cr);
      
      cairo_destroy (cr);
    }
  
  GTK_CELL_RENDERER_CLASS (gtk_cell_renderer_bubble_parent_class)->
    render (cell, window, widget, background_area, cell_area, expose_area, flags);
}

static void
gtk_cell_renderer_bubble_class_init (GtkCellRendererBubbleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);
  
  cell_class->render = render;
  
  object_class->finalize = gtk_cell_renderer_bubble_finalize;
  object_class->set_property = gtk_cell_renderer_bubble_set_property;
  object_class->get_property = gtk_cell_renderer_bubble_get_property;
  
  g_type_class_add_private (object_class, sizeof(GtkCellRendererBubblePrivate));
  
  g_object_class_install_property (object_class,
                                   PROP_SHOW_BUBBLE,
                                   g_param_spec_boolean ("show-bubble",
                                                         "Show Bubble",
                                                         "Show the bubble background",
                                                         TRUE,
                                                         G_PARAM_READWRITE));
}

static void
gtk_cell_renderer_bubble_init (GtkCellRendererBubble *cell)
{
  cell->priv = G_TYPE_INSTANCE_GET_PRIVATE (cell,
                                            GTK_TYPE_CELL_RENDERER_BUBBLE,
                                            GtkCellRendererBubblePrivate);
  cell->priv->show_bubble = TRUE;
  
  /* we need extra padding on the side */
  g_object_set (cell, "xpad", 3, "ypad", 3, NULL);
}

GtkCellRenderer*
gtk_cell_renderer_bubble_new ()
{
  return g_object_new (GTK_TYPE_CELL_RENDERER_BUBBLE, NULL);
}
