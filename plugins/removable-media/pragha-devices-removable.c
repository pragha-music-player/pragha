/*************************************************************************/
/* Copyright (C) 2009-2014 matias <mati86dl@gmail.com>                   */
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

#include <gudev/gudev.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "plugins/pragha-plugin-macros.h"

#include "plugins/devices/pragha-devices-plugin.h"
#include "plugins/devices/pragha-device-client.h"

#include "src/pragha-database-provider.h"
#include "src/pragha-playback.h"
#include "src/pragha-utils.h"
#include "src/pragha.h"

#define PRAGHA_TYPE_REMOVABLE_PLUGIN         (pragha_removable_plugin_get_type ())
#define PRAGHA_REMOVABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_REMOVABLE_PLUGIN, PraghaRemovablePlugin))
#define PRAGHA_REMOVABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_REMOVABLE_PLUGIN, PraghaRemovablePlugin))
#define PRAGHA_IS_REMOVABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_REMOVABLE_PLUGIN))
#define PRAGHA_IS_REMOVABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_REMOVABLE_PLUGIN))
#define PRAGHA_REMOVABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_REMOVABLE_PLUGIN, PraghaRemovablePluginClass))

typedef struct _PraghaRemovablePluginPrivate PraghaRemovablePluginPrivate;

struct _PraghaRemovablePluginPrivate {
	PraghaApplication  *pragha;

	/* Gudev devie */
	guint64             bus_hooked;
	guint64             device_hooked;
	GUdevDevice        *u_device;

	/* Gio Volume */
	GVolume            *volume;

	/* Mount point. */
	gchar              *mount_path;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_REMOVABLE_PLUGIN,
                        PraghaRemovablePlugin,
                        pragha_removable_plugin)

static void
pragha_removable_clear_hook_device (PraghaRemovablePlugin *plugin)
{
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	priv->bus_hooked = 0;
	priv->device_hooked = 0;	

	if (priv->u_device) {
		g_object_unref (priv->u_device);
		priv->u_device = NULL;
	}
	if (priv->volume) {
		g_object_unref (priv->volume);
		priv->volume = NULL;
	}
	if (priv->mount_path) {
		g_free(priv->mount_path);
		priv->mount_path = NULL;
	}
}

static void
pragha_block_device_add_to_library (PraghaRemovablePlugin *plugin, GMount *mount)
{
	PraghaDatabaseProvider *provider;
	PraghaScanner *scanner;
	GSList *provider_list = NULL;
	GFile       *mount_point;
	gchar       *mount_path, *name;

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	mount_point = g_mount_get_root (mount);
	mount_path = g_file_get_path (mount_point);

	provider = pragha_database_provider_get ();
	provider_list = pragha_provider_get_list_by_type (provider, "local");

	if (pragha_string_list_is_not_present (provider_list, mount_path))
	{
		name = g_mount_get_name (mount);

		pragha_provider_add_new (provider,
		                         mount_path,
		                         "local",
		                         name,
		                         "media-removable");

		g_free (name);
	}
	g_slist_free_full (provider_list, g_free);

	priv->mount_path = g_strdup(mount_path);

	scanner = pragha_application_get_scanner (priv->pragha);
	pragha_scanner_update_library (scanner);

	g_object_unref (provider);
	g_object_unref (mount_point);
	g_free (mount_path);
}

static void
pragha_removable_drop_device_from_library (PraghaRemovablePlugin *plugin)
{
	PraghaDatabaseProvider *provider;
	PraghaScanner *scanner;
	GSList *provider_list = NULL;

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	provider = pragha_database_provider_get ();
	provider_list = pragha_provider_get_list_by_type (provider, "local");

	if (pragha_string_list_is_present (provider_list, priv->mount_path))
	{
		pragha_provider_remove (provider,
		                        priv->mount_path),

		scanner = pragha_application_get_scanner (priv->pragha);
		pragha_scanner_update_library (scanner);
	}

	g_slist_free_full (provider_list, g_free);
	g_object_unref (provider);
}


/*
 * Some functions to mount block removable.
 */

/* Decode the ID_FS_LABEL_ENC of block device.
 * Extentions copy of Thunar-volman code.
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-gio-extensions.c */

static GVolume *
tvm_g_volume_monitor_get_volume_for_kind (GVolumeMonitor *monitor,
                                          const gchar    *kind,
                                          const gchar    *identifier)
{
	GVolume *volume = NULL;
	GList   *volumes;
	GList   *lp;
	gchar   *value;

	g_return_val_if_fail (G_IS_VOLUME_MONITOR (monitor), NULL);
	g_return_val_if_fail (kind != NULL && *kind != '\0', NULL);
	g_return_val_if_fail (identifier != NULL && *identifier != '\0', NULL);

	volumes = g_volume_monitor_get_volumes (monitor);

	for (lp = volumes; volume == NULL && lp != NULL; lp = lp->next) {
		value = g_volume_get_identifier (lp->data, kind);
		if (value == NULL)
			continue;
		if (g_strcmp0 (value, identifier) == 0)
			volume = g_object_ref (lp->data);
		g_free (value);
	}
	g_list_foreach (volumes, (GFunc)g_object_unref, NULL);
	g_list_free (volumes);

	return volume;
}

static void
pragha_block_device_mount_finish (GVolume *volume, GAsyncResult *result, PraghaRemovablePlugin *plugin)
{
	GtkWidget *dialog;
	GMount    *mount;
	GError    *error = NULL;
	gchar     *name = NULL, *primary = NULL;
  
	g_return_if_fail (G_IS_VOLUME (volume));
	g_return_if_fail (G_IS_ASYNC_RESULT (result));

	/* finish mounting the volume */
	if (!g_volume_mount_finish (volume, result, &error)) {
		if (error->code != G_IO_ERROR_FAILED_HANDLED &&
		    error->code != G_IO_ERROR_ALREADY_MOUNTED) {
			name = g_volume_get_name (G_VOLUME (volume));
			primary = g_strdup_printf (_("Unable to access \"%s\""), name);
			g_free (name);

			dialog = pragha_gudev_dialog_new (NULL, _("Removable Device"), "media-removable",
			                                  primary, error->message,
			                                  NULL, PRAGHA_DEVICE_RESPONSE_NONE);
			g_signal_connect (dialog, "response",
			                  G_CALLBACK (gtk_widget_destroy), NULL);

			gtk_widget_show_all (dialog);

			g_free (primary);
		}
		g_error_free (error);
	}

	/* get the moint point of the volume */
	mount = g_volume_get_mount (volume);
	if (mount != NULL) {
		pragha_block_device_add_to_library (plugin, mount);
		g_object_unref (mount);
	}
	g_object_unref (volume);
}

static void
pragha_block_device_mount_device (PraghaRemovablePlugin *plugin)
{
	GMountOperation *mount_operation;

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	/* try to mount the volume asynchronously */
	mount_operation = gtk_mount_operation_new (NULL);
	g_volume_mount (priv->volume, G_MOUNT_MOUNT_NONE, mount_operation,
	                NULL, (GAsyncReadyCallback) pragha_block_device_mount_finish,
	                plugin);
	g_object_unref (mount_operation);
}

static void
pragha_block_device_detected_response (GtkWidget *dialog,
                                       gint       response,
                                       gpointer   user_data)
{
	PraghaRemovablePlugin *plugin = user_data;

	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_BROWSE:
			pragha_block_device_mount_device (plugin);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
			pragha_removable_clear_hook_device (plugin);
		default:
			break;
	}
	gtk_widget_destroy (dialog);
}

static gboolean
pragha_block_device_detected (gpointer data)
{
	GtkWidget      *dialog;
	GVolumeMonitor *monitor;
	GVolume        *volume;
	gchar *name = NULL, *primary = NULL;

	PraghaRemovablePlugin *plugin = data;
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	/* determine the GVolume corresponding to the udev removable */
	monitor = g_volume_monitor_get ();
	volume = tvm_g_volume_monitor_get_volume_for_kind (monitor,
	                                                   G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE,
	                                                   g_udev_device_get_device_file (priv->u_device));
	g_object_unref (monitor);

	/* check if we have a volume */
	priv->volume = volume;
	if (volume == NULL || !g_volume_can_mount (volume)) {
		pragha_removable_clear_hook_device (plugin);
		return FALSE;
	}

	name = g_volume_get_name (G_VOLUME (volume));
	primary = g_strdup_printf (_("Want to manage \"%s\" volume?"), name);
	g_free (name);

	dialog = pragha_gudev_dialog_new (NULL, _("Removable Device"), "media-removable",
	                                  primary, NULL,
	                                  _("_Update library"), PRAGHA_DEVICE_RESPONSE_BROWSE);

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_block_device_detected_response), plugin);

	gtk_widget_show_all (dialog);

	g_free (primary);

	return FALSE;
}

static void
pragha_removable_plugin_device_added (PraghaDeviceClient *device_client,
                                      PraghaDeviceType    device_type,
                                      GUdevDevice        *u_device,
                                      gpointer            user_data)
{
	PraghaRemovablePlugin *plugin = user_data;
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	if (device_type != PRAGHA_DEVICE_MOUNTABLE)
		return;

	priv->bus_hooked = g_udev_device_get_property_as_uint64 (u_device, "BUSNUM");
	priv->device_hooked = g_udev_device_get_property_as_uint64 (u_device, "DEVNUM");
	priv->u_device = g_object_ref (u_device);
	priv->volume = NULL;

	/*
	 * HACK: We're listening udev. Then wait 2 seconds, to ensure that GVolume also detects the device.
	 */
	g_timeout_add_seconds(2, pragha_block_device_detected, plugin);
}

void
pragha_removable_plugin_device_removed (PraghaDeviceClient *device_client,
                                        PraghaDeviceType    device_type,
                                        GUdevDevice        *u_device,
                                        gpointer            user_data)
{
	guint64 busnum = 0;
	guint64 devnum = 0;

	PraghaRemovablePlugin *plugin = user_data;
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	if (!priv->u_device || !priv->mount_path)
		return;

	if (device_type != PRAGHA_DEVICE_MOUNTABLE)
		return;

	busnum = g_udev_device_get_property_as_uint64(u_device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64(u_device, "DEVNUM");

	if (busnum == priv->bus_hooked && devnum == priv->device_hooked) {
		pragha_removable_drop_device_from_library (plugin);
		pragha_removable_clear_hook_device (plugin);
	}
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDeviceClient *device_client;
	PraghaRemovablePlugin *plugin = PRAGHA_REMOVABLE_PLUGIN (activatable);
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Removable plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	device_client = pragha_device_client_get();
	g_signal_connect (G_OBJECT(device_client), "device-added",
	                  G_CALLBACK(pragha_removable_plugin_device_added), plugin);
	g_signal_connect (G_OBJECT(device_client), "device-removed",
	                  G_CALLBACK(pragha_removable_plugin_device_removed), plugin);
	g_object_unref (device_client);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDeviceClient *device_client;
	PraghaRemovablePlugin *plugin = PRAGHA_REMOVABLE_PLUGIN (activatable);
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Removable plugin %s", G_STRFUNC);

	device_client = pragha_device_client_get();
	g_signal_handlers_disconnect_by_func (device_client,
	                                      pragha_removable_plugin_device_added,
	                                      plugin);
	g_signal_handlers_disconnect_by_func (device_client,
	                                      pragha_removable_plugin_device_removed,
	                                      plugin);
	g_object_unref (device_client);

	priv->pragha = NULL;
}