/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */


#include "config.h"

#include "pragha-progressbar.h"

struct _PraghaProgressBarPrivate
{
	gdouble        fraction;
};

enum {
	PROP_0,
	PROP_FRACTION,
};

static void pragha_progress_bar_set_property         (GObject        *object,
                                                      guint           prop_id,
                                                      const GValue   *value,
                                                      GParamSpec     *pspec);
static void pragha_progress_bar_get_property         (GObject        *object,
                                                      guint           prop_id,
                                                      GValue         *value,
                                                      GParamSpec     *pspec);
static void pragha_progress_bar_size_allocate        (GtkWidget      *widget,
                                                      GtkAllocation  *allocation);
static void pragha_progress_bar_get_preferred_width  (GtkWidget      *widget,
                                                      gint           *minimum,
                                                      gint           *natural);
static void pragha_progress_bar_get_preferred_height (GtkWidget      *widget,
                                                      gint           *minimum,
                                                      gint           *natural);

static gboolean pragha_progress_bar_draw             (GtkWidget      *widget,
                                                      cairo_t        *cr);
static void     pragha_progress_bar_finalize         (GObject        *object);

G_DEFINE_TYPE_WITH_PRIVATE (PraghaProgressBar, pragha_progress_bar, GTK_TYPE_WIDGET)

static void
pragha_progress_bar_class_init (PraghaProgressBarClass *class)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;

	gobject_class = G_OBJECT_CLASS (class);
	widget_class = (GtkWidgetClass *) class;

	gobject_class->set_property = pragha_progress_bar_set_property;
	gobject_class->get_property = pragha_progress_bar_get_property;
	gobject_class->finalize = pragha_progress_bar_finalize;

	widget_class->draw = pragha_progress_bar_draw;
	widget_class->size_allocate = pragha_progress_bar_size_allocate;
	widget_class->get_preferred_width = pragha_progress_bar_get_preferred_width;
	widget_class->get_preferred_height = pragha_progress_bar_get_preferred_height;

	g_object_class_install_property (gobject_class,
	                                 PROP_FRACTION,
	                                 g_param_spec_double ("fraction",
                                                          "Fraction",
	                                                      "The fraction of total work that has been completed",
	                                                      0.0, 1.0, 0.0,
	                                                      G_PARAM_READWRITE));
}

static void
pragha_progress_bar_init (PraghaProgressBar *pbar)
{
	GtkStyleContext *context;
	PraghaProgressBarPrivate *priv;

	pbar->priv = pragha_progress_bar_get_instance_private (pbar);
	priv = pbar->priv;

	priv->fraction = 0.0;

	gtk_widget_set_has_window (GTK_WIDGET (pbar), FALSE);
	context = gtk_widget_get_style_context (GTK_WIDGET (pbar));
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_HORIZONTAL);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_TROUGH);
}

static void
pragha_progress_bar_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
	PraghaProgressBar *pbar;
	pbar = PRAGHA_PROGRESS_BAR (object);

	switch (prop_id)
	{
		case PROP_FRACTION:
			pragha_progress_bar_set_fraction (pbar, g_value_get_double (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_progress_bar_get_property (GObject      *object,
                                  guint         prop_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
	PraghaProgressBar *pbar = PRAGHA_PROGRESS_BAR (object);
	PraghaProgressBarPrivate* priv = pbar->priv;

	switch (prop_id)
	{
		case PROP_FRACTION:
			g_value_set_double (value, priv->fraction);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

GtkWidget*
pragha_progress_bar_new (void)
{
	GtkWidget *pbar;

	pbar = g_object_new (PRAGHA_TYPE_PROGRESS_BAR, NULL);

	return pbar;
}

static void
pragha_progress_bar_finalize (GObject *object)
{
	//PraghaProgressBar *pbar = PRAGHA_PROGRESS_BAR (object);
	G_OBJECT_CLASS (pragha_progress_bar_parent_class)->finalize (object);
}

static void
pragha_progress_bar_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
	GTK_WIDGET_CLASS (pragha_progress_bar_parent_class)->size_allocate (widget, allocation);
	//gtk_widget_set_clip (widget, NULL);
}

static void
pragha_progress_bar_get_preferred_width (GtkWidget *widget,
                                         gint      *minimum,
                                         gint      *natural)
{
	if (minimum)
		*minimum = 140;
	if (natural)
		*natural = 140;
}

static void
pragha_progress_bar_get_preferred_height (GtkWidget *widget,
                                          gint      *minimum,
                                          gint      *natural)
{
	if (minimum)
		*minimum = 12;
	if (natural)
		*natural = 12;
}

static void
pragha_progress_bar_paint_continuous (PraghaProgressBar *pbar,
                                      cairo_t           *cr,
                                      gint               amount,
                                      int                x,
                                      int                y,
                                      int                width,
                                      int                height)
{
	GtkStyleContext *context;
	GtkStateFlags state;
	GtkBorder padding;
	GtkWidget *widget = GTK_WIDGET (pbar);
	GdkRectangle area;

	if (amount <= 0)
		return;

	context = gtk_widget_get_style_context (widget);
	state = gtk_widget_get_state_flags (widget);
	gtk_style_context_get_padding (context, state, &padding);

	gtk_style_context_save (context);
	gtk_style_context_remove_class (context, GTK_STYLE_CLASS_TROUGH);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_PROGRESSBAR);

	area.width = amount;
	area.height = height - padding.top - padding.bottom;
	area.y = y + padding.top;
	area.x = x + padding.left;

	gtk_style_context_add_class (context, GTK_STYLE_CLASS_LEFT);
	if (pragha_progress_bar_get_fraction (pbar) == 1.0)
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_RIGHT);

	gtk_render_background (context, cr, area.x, area.y, area.width, area.height);
	gtk_render_frame (context, cr, area.x, area.y, area.width, area.height);

	gtk_style_context_restore (context);
}

static gboolean
pragha_progress_bar_draw (GtkWidget      *widget,
                          cairo_t        *cr)
{
	PraghaProgressBar *pbar = PRAGHA_PROGRESS_BAR (widget);
	GtkStyleContext *context;
	GtkStateFlags state;
	GtkBorder padding;
	gint width, height;
	gint bar_width, bar_height;

	context = gtk_widget_get_style_context (widget);
	state = gtk_widget_get_state_flags (widget);
	gtk_style_context_get_padding (context, state, &padding);

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);

	bar_height = height;
	bar_width = width;

	gtk_style_context_save (context);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_TROUGH);

	gtk_render_background (context, cr, width - bar_width, height - bar_height, bar_width, bar_height);
	gtk_render_frame (context, cr, width - bar_width, height - bar_height, bar_width, bar_height);

	gtk_style_context_restore (context);

	gint amount;
	gint space;

	space = bar_width - padding.left - padding.right;
	amount = space * pragha_progress_bar_get_fraction (pbar);

	pragha_progress_bar_paint_continuous (pbar, cr, amount,
	                                      width - bar_width, height - bar_height,
	                                      bar_width, bar_height);

	return FALSE;
}

void
pragha_progress_bar_set_fraction (PraghaProgressBar *pbar,
                                  gdouble            fraction)
{
	PraghaProgressBarPrivate* priv;

	g_return_if_fail (PRAGHA_IS_PROGRESS_BAR (pbar));

	priv = pbar->priv;

	priv->fraction = CLAMP (fraction, 0.0, 1.0);
	gtk_widget_queue_draw (GTK_WIDGET (pbar));

	g_object_notify (G_OBJECT (pbar), "fraction");
}

gdouble
pragha_progress_bar_get_fraction (PraghaProgressBar *pbar)
{
	g_return_val_if_fail (PRAGHA_IS_PROGRESS_BAR (pbar), 0);

	return pbar->priv->fraction;
}