/*************************************************************************/
/* Copyright (C) 2018 matias <mati86dl@gmail.com>                        */
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

#ifndef __PRAGHA_VISUALIZER_PLUGIN_H__
#define __PRAGHA_VISUALIZER_PLUGIN_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

#include "src/pragha.h"

#include "pragha-visualizer.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_VISUALIZER_PLUGIN         (pragha_visualizer_plugin_get_type ())
#define PRAGHA_VISUALIZER_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_VISUALIZER_PLUGIN, PraghaVisualizerPlugin))
#define PRAGHA_VISUALIZER_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_VISUALIZER_PLUGIN, PraghaVisualizerPlugin))
#define PRAGHA_IS_VISUALIZER_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_VISUALIZER_PLUGIN))
#define PRAGHA_IS_VISUALIZER_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_VISUALIZER_PLUGIN))
#define PRAGHA_VISUALIZER_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_VISUALIZER_PLUGIN, PraghaVisualizerPluginClass))

typedef struct _PraghaVisualizerPluginPrivate PraghaVisualizerPluginPrivate;

struct _PraghaVisualizerPluginPrivate {
	PraghaApplication *pragha;

  PraghaVisualizer  *visualizer;

	/* Menu options */
	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;
  GSimpleAction     *gear_action;
};

GType                 pragha_visualizer_plugin_get_type        (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __PRAGHA_VISUALIZER_PLUGIN_H__ */
