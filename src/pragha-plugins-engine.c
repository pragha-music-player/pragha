/*************************************************************************/
/* Copyright (C) 2013-2014 matias <mati86dl@gmail.com>                   */
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

#include "pragha-plugins-engine.h"

#include <libpeas/peas.h>

#include "pragha-utils.h"

struct _PraghaPluginsEngine {
	GObject           _parent;

	PraghaApplication *pragha;

	PeasEngine        *peas_engine;
	PeasExtensionSet  *peas_exten_set;
};

G_DEFINE_TYPE(PraghaPluginsEngine, pragha_plugins_engine, G_TYPE_OBJECT)

static void
on_extension_added (PeasExtensionSet  *set,
                    PeasPluginInfo    *info,
                    PeasExtension     *exten,
                    gpointer           data)
{
	peas_activatable_activate (PEAS_ACTIVATABLE (exten));
}

static void
on_extension_removed (PeasExtensionSet  *set,
                      PeasPluginInfo    *info,
                      PeasExtension     *exten,
                      gpointer           data)
{
	peas_activatable_deactivate (PEAS_ACTIVATABLE (exten));
}

void
pragha_plugins_engine_shutdown (PraghaPluginsEngine *engine)
{
	PraghaPreferences *preferences;
	gchar **loaded_plugins = NULL;

	CDEBUG(DBG_PLUGIN,"Plugins engine shutdown");

	g_signal_handlers_disconnect_by_func (engine->peas_exten_set, (GCallback) on_extension_added, engine);
	g_signal_handlers_disconnect_by_func (engine->peas_exten_set, (GCallback) on_extension_removed, engine);

	loaded_plugins = peas_engine_get_loaded_plugins (engine->peas_engine);
	if (loaded_plugins) {
		preferences = pragha_application_get_preferences (engine->pragha);
		pragha_preferences_set_string_list (preferences,
				                            "PLUGINS",
				                            "Activated",
				                            (const gchar * const*)loaded_plugins,
		                                     g_strv_length(loaded_plugins));

		g_strfreev(loaded_plugins);
	}
	peas_engine_set_loaded_plugins (engine->peas_engine, NULL);
}

void
pragha_plugins_engine_startup (PraghaPluginsEngine *engine)
{
	PraghaPreferences *preferences;
	gchar **loaded_plugins = NULL;
	const gchar *default_plugins[] = {"notify", "song-info", NULL};

	CDEBUG(DBG_PLUGIN,"Plugins engine startup");

	preferences = pragha_application_get_preferences (engine->pragha);

	if (string_is_not_empty (pragha_preferences_get_installed_version (preferences))) {
		loaded_plugins = pragha_preferences_get_string_list (preferences,
		                                                     "PLUGINS",
		                                                     "Activated",
		                                                     NULL);

		if (loaded_plugins) {
			peas_engine_set_loaded_plugins (engine->peas_engine, (const gchar **) loaded_plugins);
			g_strfreev(loaded_plugins);
		}
	}
	else {
		peas_engine_set_loaded_plugins (engine->peas_engine, (const gchar **) default_plugins);
	}
}

/*
 * PraghaPluginsEngine
 */
static void
pragha_plugins_engine_dispose (GObject *object)
{
	PraghaPluginsEngine *engine = PRAGHA_PLUGINS_ENGINE(object);

	CDEBUG(DBG_PLUGIN,"Dispose plugins engine");

	if (engine->peas_exten_set) {
	
		g_object_unref (engine->peas_exten_set);
		engine->peas_exten_set = NULL;
	}
	if (engine->peas_engine) {
		peas_engine_garbage_collect (engine->peas_engine);

		g_object_unref (engine->peas_engine);
		engine->peas_engine = NULL;
	}
	if (engine->pragha) {
		g_object_unref (engine->pragha);
		engine->pragha = NULL;
	}

	G_OBJECT_CLASS(pragha_plugins_engine_parent_class)->dispose(object);
}

static void
pragha_plugins_engine_class_init (PraghaPluginsEngineClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = pragha_plugins_engine_dispose;
}

static void
pragha_plugins_engine_init (PraghaPluginsEngine *engine)
{
	engine->peas_engine = peas_engine_get_default ();
}

PraghaPluginsEngine *
pragha_plugins_engine_new (PraghaApplication *pragha)
{
	PraghaPluginsEngine *engine;

	CDEBUG(DBG_PLUGIN,"Create new plugins engine");

	engine = g_object_new (PRAGHA_TYPE_PLUGINS_ENGINE, NULL);

	engine->pragha = g_object_ref(pragha);

	peas_engine_add_search_path (engine->peas_engine, LIBPLUGINDIR, USRPLUGINDIR);

	engine->peas_exten_set = peas_extension_set_new (engine->peas_engine,
	                                                 PEAS_TYPE_ACTIVATABLE,
	                                                 "object", pragha,
	                                                 NULL);

	g_signal_connect (engine->peas_exten_set, "extension-added",
	                  G_CALLBACK (on_extension_added), engine);
	g_signal_connect (engine->peas_exten_set, "extension-removed",
	                  G_CALLBACK (on_extension_removed), engine);

	return engine;
}
