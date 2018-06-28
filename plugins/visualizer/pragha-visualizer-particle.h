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

#ifndef PRAGHA_VISUALIZER_PARTICLE_H
#define PRAGHA_VISUALIZER_PARTICLE_H

#include <gtk/gtk.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_PARTICLE (pragha_particle_get_type())
#define PRAGHA_PARTICLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PARTICLE, PraghaParticle))
#define PRAGHA_PARTICLE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PARTICLE, PraghaParticle const))
#define PRAGHA_PARTICLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PARTICLE, PraghaParticleClass))
#define PRAGHA_IS_PARTICLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PARTICLE))
#define PRAGHA_IS_PARTICLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PARTICLE))
#define PRAGHA_PARTICLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PARTICLE, PraghaParticleClass))

typedef struct _PraghaParticle PraghaParticle;
typedef struct _PraghaParticleClass PraghaParticleClass;

struct _PraghaParticleClass
{
	GObjectClass parent_class;
};

void
pragha_particle_reset (PraghaParticle *particle);

void
pragha_particle_move_to (PraghaParticle *particle, gint x, gint y);

void
pragha_particle_move (PraghaParticle *particle, guint width, guint height);

void
pragha_particle_set_energy (PraghaParticle *particle, gdouble energy);

void
pragha_particle_draw (PraghaParticle *particle, cairo_t *cr);

PraghaParticle *
pragha_particle_new (void);

G_END_DECLS

#endif /* PRAGHA_VISUALIZER_PARTICLE_H */
