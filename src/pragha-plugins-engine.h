/*************************************************************************/
/* Copyright (C) 2013-2015 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_PLUGINS_ENGINE_H
#define PRAGHA_PLUGINS_ENGINE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_PLUGINS_ENGINE (pragha_plugins_engine_get_type())
#define PRAGHA_PLUGINS_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PLUGINS_ENGINE, PraghaPluginsEngine))
#define PRAGHA_PLUGINS_ENGINE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PLUGINS_ENGINE, PraghaPluginsEngine const))
#define PRAGHA_PLUGINS_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PLUGINS_ENGINE, PraghaPluginsEngineClass))
#define PRAGHA_IS_PLUGINS_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PLUGINS_ENGINE))
#define PRAGHA_IS_PLUGINS_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PLUGINS_ENGINE))
#define PRAGHA_PLUGINS_ENGINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PLUGINS_ENGINE, PraghaPluginsEngineClass))

typedef struct _PraghaPluginsEngine PraghaPluginsEngine;
typedef struct _PraghaPluginsEngineClass PraghaPluginsEngineClass;

struct _PraghaPluginsEngineClass
{
	GObjectClass parent_class;
};

gboolean             pragha_plugins_is_shutdown     (PraghaPluginsEngine *engine);

void                 pragha_plugins_engine_shutdown (PraghaPluginsEngine *engine);
void                 pragha_plugins_engine_startup  (PraghaPluginsEngine *engine);

PraghaPluginsEngine *pragha_plugins_engine_new      (GObject             *object);

G_END_DECLS

#endif /* PRAGHA_PLUGINS_ENGINE_H */
