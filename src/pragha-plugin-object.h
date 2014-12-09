/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_PLUGIN_OBJECT_H
#define PRAGHA_PLUGIN_OBJECT_H

#include "pragha.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_PLUGIN_OBJECT            (pragha_plugin_object_get_type())
#define PRAGHA_PLUGIN_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PLUGIN_OBJECT, PraghaPluginObject))
#define PRAGHA_PLUGIN_OBJECT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PLUGIN_OBJECT, PraghaPluginObject const))
#define PRAGHA_PLUGIN_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_PLUGIN_OBJECT, PraghaPluginObjectClass))
#define PRAGHA_IS_PLUGIN_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PLUGIN_OBJECT))
#define PRAGHA_IS_PLUGIN_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_PLUGIN_OBJECT))
#define PRAGHA_PLUGIN_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_PLUGIN_OBJECT, PraghaPluginObjectClass))

typedef struct _PraghaPluginObject PraghaPluginObject;
typedef struct _PraghaPluginObjectClass PraghaPluginObjectClass;
typedef struct _PraghaPluginObjectPrivate PraghaPluginObjectPrivate;

struct _PraghaPluginObject
{
	GObject parent;

	/*< private >*/
	PraghaPluginObjectPrivate *priv;
};

struct _PraghaPluginObjectClass
{
	GObjectClass parent_class;
};

/*
 * Playback:
 */

void
pragha_plugin_object_playback_prev              (PraghaPluginObject *object);
void
pragha_plugin_object_playback_play_pause_resume (PraghaPluginObject *object);
void
pragha_plugin_object_playback_play              (PraghaPluginObject *object);
void
pragha_plugin_object_playback_pause             (PraghaPluginObject *object);
void
pragha_plugin_object_playback_resume            (PraghaPluginObject *object);
void
pragha_plugin_object_playback_stop              (PraghaPluginObject *object);
void
pragha_plugin_object_playback_next              (PraghaPluginObject *object);

void
pragha_plugin_object_playback_toggle_repeat     (PraghaPluginObject *object);
void
pragha_plugin_object_playback_set_repeat        (PraghaPluginObject *object,
                                                 gboolean            repeat);
gboolean
pragha_plugin_object_playback_get_repeat        (PraghaPluginObject *object);

void
pragha_plugin_object_playback_toggle_shuffle    (PraghaPluginObject *object);
void
pragha_plugin_object_playback_set_shuffle       (PraghaPluginObject *object,
                                                 gboolean            shuffle);
gboolean
pragha_plugin_object_playback_get_shuffle       (PraghaPluginObject *object);

/*
 * PraghaPluginObject
 */

PraghaPluginObject *
pragha_plugin_object_get (PraghaApplication *pragha);

PraghaApplication *
pragha_plugin_object_get_pragha (PraghaPluginObject *object);

GType pragha_plugin_object_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* PRAGHA_PLUGIN_OBJECT_H */
