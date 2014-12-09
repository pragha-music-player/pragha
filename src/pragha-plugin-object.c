/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-plugin-object.h"

#include <glib.h>
#include <glib/gstdio.h>

#include "src/pragha-playback.h"

G_DEFINE_TYPE(PraghaPluginObject, pragha_plugin_object, G_TYPE_OBJECT)

struct _PraghaPluginObjectPrivate
{
	PraghaApplication *pragha;
};

enum
{
	PROP_0,
	PROP_PRAGHA,
	LAST_PROP
};
static GParamSpec *gParamSpecs[LAST_PROP];

/*
 * Playback:
 */

void
pragha_plugin_object_playback_prev (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_prev_track (priv->pragha);
}

void
pragha_plugin_object_playback_play_pause_resume (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_play_pause_resume(priv->pragha);
}

void
pragha_plugin_object_playback_play (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_play (priv->pragha);
}


void
pragha_plugin_object_playback_pause (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_pause (priv->pragha);
}


void
pragha_plugin_object_playback_stop (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_stop (priv->pragha);
}

void
pragha_plugin_object_playback_next (PraghaPluginObject *object)
{
	PraghaBackend *backend;

	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	backend = pragha_application_get_backend (priv->pragha);
	if (!pragha_backend_emitted_error (backend))
		pragha_playback_next_track (priv->pragha);
}

void
pragha_plugin_object_playback_toggle_repeat (PraghaPluginObject *object)
{
	PraghaPreferences *preferences;
	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);
	pragha_preferences_set_repeat (preferences,
		!pragha_preferences_get_repeat (preferences));
}

void
pragha_plugin_object_playback_set_repeat (PraghaPluginObject *object,
                                          gboolean            repeat)
{
	PraghaPreferences *preferences;
	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);
	pragha_preferences_set_repeat (preferences, repeat);
}

gboolean
pragha_plugin_object_playback_get_repeat (PraghaPluginObject *object)
{
	PraghaPreferences *preferences;
	g_return_val_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object), FALSE);

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);

	return pragha_preferences_get_repeat (preferences);
}

void
pragha_plugin_object_playback_toggle_shuffle (PraghaPluginObject *object)
{
	PraghaPreferences *preferences;
	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);
	pragha_preferences_set_shuffle (preferences,
		!pragha_preferences_get_shuffle (preferences));
}

void
pragha_plugin_object_playback_set_shuffle (PraghaPluginObject *object,
                                           gboolean            shuffle)
{
	PraghaPreferences *preferences;
	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);
	pragha_preferences_set_shuffle (preferences, shuffle);
}

gboolean
pragha_plugin_object_playback_get_shuffle (PraghaPluginObject *object)
{
	PraghaPreferences *preferences;
	g_return_if_fail(PRAGHA_IS_PLUGIN_OBJECT(object));

	PraghaPluginObjectPrivate *priv = object->priv;

	preferences = pragha_application_get_preferences (priv->pragha);

	return pragha_preferences_get_shuffle (preferences);
}


/*
 *
 */
PraghaApplication *
pragha_plugin_object_get_pragha (PraghaPluginObject *object)
{
	PraghaPluginObjectPrivate *priv = object->priv;

	return priv->pragha;
}

static void
pragha_plugin_object_set_pragha (PraghaPluginObject *object,
                                 PraghaApplication  *pragha)
{
	PraghaPluginObjectPrivate *priv = object->priv;
	priv->pragha = pragha;
}

static void
pragha_plugin_object_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	PraghaPluginObject *pobject = PRAGHA_PLUGIN_OBJECT(object);

	switch (prop_id) {
		case PROP_PRAGHA:
			g_value_set_pointer (value, pragha_plugin_object_get_pragha(pobject));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
pragha_plugin_object_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	PraghaPluginObject *pobject = PRAGHA_PLUGIN_OBJECT(object);

	switch (prop_id) {
		case PROP_PRAGHA:
			pragha_plugin_object_set_pragha (pobject, g_value_get_pointer (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
pragha_plugin_object_class_init (PraghaPluginObjectClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->get_property = pragha_plugin_object_get_property;
	object_class->set_property = pragha_plugin_object_set_property;

	g_type_class_add_private(object_class, sizeof(PraghaPluginObjectPrivate));

	/**
	  * PraghaPluginObject:pragha:
	  *
	  */
	gParamSpecs[PROP_PRAGHA] =
		g_param_spec_pointer("pragha",
		                     "Pragha",
		                     "Pragha Object",
		                     G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(object_class, LAST_PROP, gParamSpecs);
}

static void
pragha_plugin_object_init (PraghaPluginObject *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,
	                                           PRAGHA_TYPE_PLUGIN_OBJECT,
	                                           PraghaPluginObjectPrivate);
}

/**
 * pragha_plugin_object_get:
 *
 * Queries the global #PraghaPluginObject instance, which is shared
 * by all modules. The function automatically takes a reference
 * for the caller, so you'll need to call g_object_unref() when
 * you're done with it.
 *
 * Return value: the global #PraghaPluginObject instance.
 **/
PraghaPluginObject*
pragha_plugin_object_get (PraghaApplication *pragha)
{
	static PraghaPluginObject *object = NULL;

	if (G_UNLIKELY (object == NULL)) {
		CDEBUG(DBG_INFO, "Creating a new PraghaPluginObject instance");

		object = g_object_new(PRAGHA_TYPE_PLUGIN_OBJECT, "pragha", pragha, NULL);
		g_object_add_weak_pointer(G_OBJECT (object),
		                          (gpointer) &object);
	}
	else {
		g_object_ref (G_OBJECT (object));
	}

	return object;
}
