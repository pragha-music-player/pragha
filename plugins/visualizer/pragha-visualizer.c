/*************************************************************************/
/* Copyright (C) 2018-2021 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
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

/*
 * This code is completely based on the Koel audio visualization..
 * https://github.com/koel/core/blob/master/js/utils/visualizer.js
 *
 * Just Thanks!.
 */

#include <gmodule.h>
#include <math.h>

#include "src/pragha-backend.h"

#include "pragha-visualizer-particle.h"
#include "pragha-visualizer.h"


#define PRAGHA_TYPE_VISUALIZER (pragha_visualizer_get_type())
#define PRAGHA_VISUALIZER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_VISUALIZER, PraghaVisualizer))
#define PRAGHA_VISUALIZER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_VISUALIZER, PraghaVisualizer const))
#define PRAGHA_VISUALIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_VISUALIZER, PraghaVisualizerClass))
#define PRAGHA_IS_VISUALIZER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_VISUALIZER))
#define PRAGHA_IS_VISUALIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_VISUALIZER))
#define PRAGHA_VISUALIZER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_VISUALIZER, PraghaVisualizerClass))

struct _PraghaVisualizerClass {
	 GtkBoxClass     parent_class;
};
struct _PraghaVisualizer {
	GtkBox         _parent;

	GtkWidget      *drawing_area;
	GList          *particles;

	guint           width;
	guint           height;

	guint           tick_id;
};

G_DEFINE_TYPE(PraghaVisualizer, pragha_visualizer, GTK_TYPE_BOX)

void
pragha_visualizer_set_magnitudes (PraghaVisualizer *visualizer, GValue *magnitudes)
{
	PraghaParticle *particle = NULL;
	const GValue *mag = NULL;
	GList *l = NULL;
	guint i = 0;
	gdouble dmag = 0.0;

	if (!gtk_widget_is_visible (GTK_WIDGET(visualizer)))
		return;

	for (l = visualizer->particles, i = 0 ; l != NULL ; l = l->next, i++)
	{
		particle = l->data;
		mag = gst_value_list_get_value (magnitudes, i);
		if (mag != NULL)
			dmag = 80.0 + g_value_get_float (mag);
		else
			dmag = 0.0;

		pragha_particle_set_energy (particle, dmag/128);
//		pragha_particle_move (particle, visualizer->width, visualizer->height);
	}

	gtk_widget_queue_draw (GTK_WIDGET(visualizer->drawing_area));
}

void
pragha_visualizer_stop (PraghaVisualizer *visualizer)
{
	PraghaParticle *particle = NULL;
	GList *l = NULL;
	for (l = visualizer->particles ; l != NULL ; l = l->next)
	{
		particle = l->data;
		pragha_particle_set_energy (particle, 0.0);
	}
}

static gboolean
pragha_visualizer_widget_draw (GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	PraghaParticle *particle = NULL;
	GList *l = NULL;

	PraghaVisualizer *visualizer = PRAGHA_VISUALIZER (user_data);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_set_tolerance (cr, 1.0);
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_FAST);

	cairo_rectangle (cr, 0, 0, visualizer->width, visualizer->height);
	cairo_fill (cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);

	for (l = visualizer->particles ; l != NULL ; l = l->next)
	{
		particle = l->data;
		pragha_particle_move (particle, visualizer->width, visualizer->height);
		pragha_particle_draw (particle, cr);
	}

	return TRUE;
}

static gboolean
pragha_visualizer_drawing_tick (gpointer user_data)
{
	GtkWidget *widget = GTK_WIDGET (user_data);

	if (gtk_widget_is_visible (widget))
		gtk_widget_queue_draw(widget);

	return G_SOURCE_CONTINUE;
}

static void
pragha_visualizer_size_allocate (GtkWidget *widget, GdkRectangle *allocation, gpointer user_data)
{
	PraghaParticle *particle = NULL;
	GList *l = NULL;
	gint x = 0, y = 0;

	PraghaVisualizer *visualizer = PRAGHA_VISUALIZER (user_data);

	visualizer->width = allocation->width;
	visualizer->height = allocation->height;

	for (l = visualizer->particles ; l != NULL ; l = l->next)
	{
		particle = l->data;
		x = g_random_int_range (1, visualizer->width);
		y = g_random_int_range (1, visualizer->height);
		pragha_particle_reset (particle);
		pragha_particle_move_to (particle, x, y);
	}
}

static void
pragha_visualizer_dispose (GObject *object)
{
	PraghaVisualizer *visualizer = PRAGHA_VISUALIZER (object);

	if (visualizer->tick_id) {
		g_source_remove (visualizer->tick_id);
		visualizer->tick_id = 0;
	}

	if (visualizer->particles) {
		g_list_free_full (visualizer->particles, g_object_unref);
		visualizer->particles = NULL;
	}
	G_OBJECT_CLASS (pragha_visualizer_parent_class)->dispose (object);
}

static void
pragha_visualizer_class_init (PraghaVisualizerClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->dispose = pragha_visualizer_dispose;
}

static void
pragha_visualizer_init (PraghaVisualizer *visualizer)
{
	GtkWidget *drawing_area;
	PraghaParticle *particle = NULL;
	gint i = 0;

	drawing_area = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawing_area, 640, 480);
	gtk_widget_set_hexpand (drawing_area, TRUE);

	g_signal_connect (drawing_area, "size-allocate",
	                  G_CALLBACK(pragha_visualizer_size_allocate), visualizer);
	g_signal_connect (G_OBJECT (drawing_area), "draw",
	                  G_CALLBACK (pragha_visualizer_widget_draw), visualizer);

	visualizer->tick_id = g_timeout_add (11, pragha_visualizer_drawing_tick, drawing_area);

	for (i = 0 ; i < 128 ; i++) {
		particle = pragha_particle_new ();
		pragha_particle_set_energy (particle, g_random_double_range (0, i));
		visualizer->particles = g_list_append (visualizer->particles, particle);
	}

	visualizer->drawing_area = drawing_area;
	gtk_widget_set_visible (drawing_area, TRUE);

	gtk_container_add(GTK_CONTAINER(visualizer), drawing_area);
}

PraghaVisualizer *
pragha_visualizer_new (void)
{
	return g_object_new (PRAGHA_TYPE_VISUALIZER, NULL);
}
