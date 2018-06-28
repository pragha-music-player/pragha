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

#ifndef PRAGHA_VISUALIZER_H
#define PRAGHA_VISUALIZER_H

#include <gtk/gtk.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _PraghaVisualizer PraghaVisualizer;
typedef struct _PraghaVisualizerClass PraghaVisualizerClass;

void
pragha_visualizer_set_magnitudes (PraghaVisualizer *visualizer, GValue *magnitudes);

PraghaVisualizer *
pragha_visualizer_new (void);

G_END_DECLS

#endif /* PRAGHA_VISUALIZER_H */
