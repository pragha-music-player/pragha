/*************************************************************************/
/* Copyright (C) 2009-2015 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "plugins/pragha-plugin-macros.h"

#include "pragha-device-client.h"
#include "pragha-devices-plugin.h"

#include "src/pragha-utils.h"
#include "src/pragha.h"

#define PRAGHA_TYPE_DEVICES_PLUGIN         (pragha_devices_plugin_get_type ())
#define PRAGHA_DEVICES_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPlugin))
#define PRAGHA_DEVICES_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPlugin))
#define PRAGHA_IS_DEVICES_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_DEVICES_PLUGIN))
#define PRAGHA_IS_DEVICES_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_DEVICES_PLUGIN))
#define PRAGHA_DEVICES_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPluginClass))

struct _PraghaDevicesPluginPrivate {
	PraghaApplication  *pragha;

	PraghaDeviceClient *device_client;
};
typedef struct _PraghaDevicesPluginPrivate PraghaDevicesPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_DEVICES_PLUGIN,
                        PraghaDevicesPlugin,
                        pragha_devices_plugin)

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Devices plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	priv->device_client = pragha_device_client_get ();
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Devices plugin %s", G_STRFUNC);

	g_object_unref (priv->device_client);

	priv->pragha = NULL;
}