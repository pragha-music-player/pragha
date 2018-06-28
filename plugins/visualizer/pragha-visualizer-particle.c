/*************************************************************************/
/* Copyright (C) 2018 matias <mati86dl@gmail.com>                        */
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

#define SYNCRONIZED 1

//#define DRAW_DEBUG 1

#include "pragha-visualizer-particle.h"

const gdouble SMOOTHING = 0.5;
const gdouble SCALE_MIN = 5.0;
const gdouble SCALE_MAX =  80.0;
const gdouble SPEED_MIN = 0.2;
const gdouble SPEED_MAX = 1.0;
const gdouble ALPHA_MIN = 0.8;
const gdouble ALPHA_MAX = 0.9;
const gdouble SPIN_MIN = 0.001;
const gdouble SPIN_MAX = 0.005;
const gdouble SIZE_MIN = 0.5;
const gdouble SIZE_MAX = 1.25;

#ifdef DRAW_DEBUG
static gint tick_i = 0;
static gint draw_i = 0;
static gint update_i = 0;
#endif

const gchar *COLORS[] = {
	"#69D2E7",
	"#1B676B",
	"#BEF202",
	"#EBE54D",
	"#00CDAC",
	"#1693A5",
	"#F9D423",
	"#FF4E50",
	"#E7204E",
	"#0CCABA",
	"#FF006F"
};

struct _PraghaParticle {
	GObject        _parent;

	gdouble         x;
	gdouble         y;

	GdkRGBA         color;

	gdouble         level;
	gdouble         scale;
	gdouble         alpha;
	gdouble         speed;
	gdouble         size;
	gdouble         spin;
	gdouble         band;

	gdouble         smoothedScale;
	gdouble         smoothedAlpha;
	gdouble         decayScale;
	gdouble         decayAlpha;
	gdouble         rotation;
	gdouble         energy;
};

G_DEFINE_TYPE(PraghaParticle, pragha_particle, G_TYPE_OBJECT)

void
pragha_particle_reset (PraghaParticle *particle)
{
	particle->x = 0;
	particle->y = 0;

	particle->level = 1.0 + floor(g_random_double_range (0, 4));
	particle->scale = g_random_double_range (SCALE_MIN, SCALE_MAX);
	particle->alpha = g_random_double_range (ALPHA_MIN, ALPHA_MAX);
	particle->speed = g_random_double_range (SPEED_MIN, SPEED_MAX);
	gdk_rgba_parse (&particle->color, COLORS[g_random_int_range (0, G_N_ELEMENTS(COLORS))]);

	particle->size = g_random_double_range (SIZE_MIN, SIZE_MAX);
	particle->spin = g_random_double_range (SPIN_MIN, SPIN_MAX);

	if (g_random_double_range (0.0, 1.0) < 0.5)
		particle->spin *=-1.0;

	particle->smoothedScale = 0.0;
	particle->smoothedAlpha = 0.0;
	particle->decayScale = 0.0;
	particle->decayAlpha = 0.0;
	particle->rotation = g_random_double_range (0.0, 2*G_PI);
	particle->energy = 0.0;
}

void
pragha_particle_move_to (PraghaParticle *particle, gint x, gint y)
{
	particle->x = x;
	particle->y = y;
}

void
pragha_particle_move (PraghaParticle *particle, guint width, guint height)
{
	particle->rotation += particle->spin;
	particle->y -= particle->speed * particle->level;

	if (particle->y < (-particle->size * particle->level * particle->scale * 2))
	{
		pragha_particle_reset(particle);

		particle->x = g_random_int_range (0, width);
		particle->y = height + (particle->size * particle->scale * particle->level * 2);
	}
}

void
pragha_particle_set_energy (PraghaParticle *particle, gdouble energy)
{
	particle->energy = energy;
}

void
pragha_particle_draw (PraghaParticle *particle, cairo_t *cr)
{
	GdkRGBA *color;
	const gdouble power = exp (particle->energy);
	const gdouble scale = particle->scale * power;
	const gdouble alpha = particle->alpha * particle->energy * 2;

	particle->decayScale = MAX (particle->decayScale, scale);
	particle->decayAlpha = MAX (particle->decayAlpha, alpha);

	particle->smoothedScale += (particle->decayScale - particle->smoothedScale) * 0.3;
	particle->smoothedAlpha += (particle->decayAlpha - particle->smoothedAlpha) * 0.3;

	particle->decayScale *= 0.985;
	particle->decayAlpha *= 0.975;

	color = gdk_rgba_copy (&particle->color);
	color->alpha = particle->smoothedAlpha / particle->level;

	cairo_save (cr);

	cairo_translate (cr, particle->x + cos(particle->rotation * particle->speed) * 250, particle->y);
	cairo_rotate (cr, particle->rotation);
	cairo_scale (cr, particle->smoothedScale * particle->level, particle->smoothedScale * particle->level);

	cairo_move_to (cr, particle->size * 0.5, 0.0);
	cairo_line_to (cr, particle->size * -0.5, 0.0);

	cairo_set_line_width (cr, 1.0);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	gdk_cairo_set_source_rgba (cr, color);
	cairo_stroke (cr);

	cairo_restore (cr);

	gdk_rgba_free (color);
}

static void
pragha_particle_class_init (PraghaParticleClass *klass)
{
}

static void
pragha_particle_init (PraghaParticle *particle)
{
	pragha_particle_reset (particle);
}

PraghaParticle *
pragha_particle_new (void)
{
	PraghaParticle *particle = g_object_new (PRAGHA_TYPE_PARTICLE, NULL);
	return particle;
}

