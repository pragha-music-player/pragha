/*************************************************************************/
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <keybinder.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "pragha-keybinder-plugin.h"

#include "src/pragha.h"
#include "src/pragha-playback.h"

static void peas_activatable_iface_init     (PeasActivatableInterface    *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PraghaKeybinderPlugin,
                                pragha_keybinder_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
	PROP_0,
	PROP_OBJECT
};


static void
keybind_prev_handler (const char *keystring, gpointer data)
{
	PraghaBackend *backend;
	PraghaApplication *pragha = data;

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_prev_track(pragha);
}

static void
keybind_play_handler (const char *keystring, gpointer data)
{
	PraghaBackend *backend;
	PraghaApplication *pragha = data;

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_play_pause_resume(pragha);
}

static void
keybind_stop_handler (const char *keystring, gpointer data)
{
	PraghaBackend *backend;
	PraghaApplication *pragha = data;

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_stop(pragha);
}

static void
keybind_next_handler (const char *keystring, gpointer data)
{
	PraghaBackend *backend;
	PraghaApplication *pragha = data;

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_next_track(pragha);
}

static void
keybind_media_handler (const char *keystring, gpointer data)
{
	PraghaApplication *pragha = data;

	pragha_window_toggle_state (pragha, FALSE);
}

static void
pragha_keybinder_plugin_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
	PraghaKeybinderPlugin *plugin = PRAGHA_KEYBINDER_PLUGIN (object);

	switch (prop_id) {
		case PROP_OBJECT:
			plugin->pragha = g_value_get_object (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_keybinder_plugin_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
	PraghaKeybinderPlugin *plugin = PRAGHA_KEYBINDER_PLUGIN (object);

	switch (prop_id) {
		case PROP_OBJECT:
			g_value_set_object (value, plugin->pragha);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_keybinder_plugin_init (PraghaKeybinderPlugin *plugin)
{
	g_debug ("%s", G_STRFUNC);
}

static void
pragha_keybinder_plugin_finalize (GObject *object)
{
	PraghaKeybinderPlugin *plugin = PRAGHA_KEYBINDER_PLUGIN (object);

	g_debug ("%s", G_STRFUNC);

	G_OBJECT_CLASS (pragha_keybinder_plugin_parent_class)->finalize (object);
}

static void
pragha_keybinder_plugin_activate (PeasActivatable *activatable)
{
	PraghaKeybinderPlugin *plugin = PRAGHA_KEYBINDER_PLUGIN (activatable);

	keybinder_init ();

	g_debug ("%s", G_STRFUNC);

	keybinder_bind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler, plugin->pragha);
	keybinder_bind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler, plugin->pragha);
	keybinder_bind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler, plugin->pragha);
	keybinder_bind("XF86AudioNext", (KeybinderHandler) keybind_next_handler, plugin->pragha);
	keybinder_bind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler, plugin->pragha);
}

static void
pragha_keybinder_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaKeybinderPlugin *plugin = PRAGHA_KEYBINDER_PLUGIN (activatable);

	g_debug ("%s", G_STRFUNC);

	keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
	keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
	keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
	keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
	keybinder_unbind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler);
}

static void
pragha_keybinder_plugin_class_init (PraghaKeybinderPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = pragha_keybinder_plugin_set_property;
	object_class->get_property = pragha_keybinder_plugin_get_property;
	object_class->finalize = pragha_keybinder_plugin_finalize;

	g_object_class_override_property (object_class, PROP_OBJECT, "object");
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
	iface->activate = pragha_keybinder_plugin_activate;
	iface->deactivate = pragha_keybinder_plugin_deactivate;
}

static void
pragha_keybinder_plugin_class_finalize (PraghaKeybinderPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	pragha_keybinder_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            PEAS_TYPE_ACTIVATABLE,
	                                            PRAGHA_TYPE_KEYBINDER_PLUGIN);
}
