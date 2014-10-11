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

	guint64             bus_hooked;
	guint64             device_hooked;
	GUdevDevice        *u_device;
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
	if (priv->mount_path) {
		g_free(priv->mount_path);
		priv->mount_path = NULL;
	}
}

static void
pragha_block_device_add_to_library (PraghaRemovablePlugin *plugin, GMount *mount)
{
	PraghaPreferences *preferences;
	PraghaScanner *scanner;
	GSList *library_dir = NULL;
	GFile       *mount_point;
	gchar       *mount_path;

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	mount_point = g_mount_get_root (mount);
	mount_path = g_file_get_path (mount_point);

	preferences = pragha_application_get_preferences (priv->pragha);

	library_dir =
		pragha_preferences_get_filename_list (preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR);

	if (!is_present_str_list (mount_path, library_dir)) {
		library_dir = g_slist_append (library_dir, g_strdup(mount_path));

		pragha_preferences_set_filename_list (preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR,
		                                      library_dir);
	}
	priv->mount_path = g_strdup(mount_path);

	scanner = pragha_application_get_scanner (priv->pragha);
	pragha_scanner_update_library (scanner);

	g_object_unref (mount_point);
	g_free (mount_path);
}

static void
pragha_removable_drop_device_from_library (PraghaRemovablePlugin *plugin)
{
	PraghaPreferences *preferences;
	PraghaScanner *scanner;
	GSList *library_dir = NULL;

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	preferences = pragha_application_get_preferences (priv->pragha);

	library_dir =
		pragha_preferences_get_filename_list (preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR);

	if (is_present_str_list (priv->mount_path, library_dir)) {
		library_dir = delete_from_str_list (priv->mount_path, library_dir);

		pragha_preferences_set_filename_list (preferences,
		                                      GROUP_LIBRARY,
		                                      KEY_LIBRARY_DIR,
		                                      library_dir);

		scanner = pragha_application_get_scanner (priv->pragha);
		pragha_scanner_update_library (scanner);
	}
}

/*
 * Some functions to mount block removable.
 */

/* Decode the ID_FS_LABEL_ENC of block device.
 * Extentions copy of Thunar-volman code.
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-gio-extensions.c */

static gchar *
tvm_notify_decode (const gchar *str)
{
	GString     *string;
	const gchar *p;
	gchar       *result;
	gchar        decoded_c;

	if (str == NULL)
		return NULL;

	if (!g_utf8_validate (str, -1, NULL))
		return NULL;

	string = g_string_new (NULL);

	for (p = str; p != NULL && *p != '\0'; ++p) {
		if (*p == '\\' && p[1] == 'x' && g_ascii_isalnum (p[2]) && g_ascii_isalnum (p[3])) {
			decoded_c = (g_ascii_xdigit_value (p[2]) << 4) | g_ascii_xdigit_value (p[3]);
			g_string_append_c (string, decoded_c);
			p = p + 3;
		}
		else
			g_string_append_c (string, *p);
	}

	result = string->str;
	g_string_free (string, FALSE);

	return result;
}

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

/* The next funtions allow to handle block removables.
 * The most of code is inspired in:
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-block-removable.c */

static void
pragha_block_device_mounted (PraghaRemovablePlugin *plugin, GUdevDevice *device, GMount *mount, GError **error)
{
	const gchar *volume_name;
	gchar       *decoded_name;
	gchar       *message;
	gint         response;

	g_return_if_fail (G_IS_MOUNT (mount));
	g_return_if_fail (error == NULL || *error == NULL);

	volume_name = g_udev_device_get_property (device, "ID_FS_LABEL_ENC");
	decoded_name = tvm_notify_decode (volume_name);
  
	if (decoded_name != NULL)
		message = g_strdup_printf (_("The volume \"%s\" was mounted automatically"), decoded_name);
	else
		message = g_strdup_printf (_("The inserted volume was mounted automatically"));

	response = pragha_gudev_show_dialog (NULL, _("Renovable Device"), "media-removable",
	                                     message, NULL,
	                                     _("_Update library"), PRAGHA_DEVICE_RESPONSE_BROWSE);
	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_BROWSE:
			pragha_block_device_add_to_library (plugin, mount);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}

	g_free (decoded_name);
	g_free (message);
}

static void
pragha_block_device_mount_finish (GVolume *volume, GAsyncResult *result, PraghaRemovablePlugin *plugin)
{
	GMount *mount;
	GError *error = NULL;
	gchar *name = NULL, *primary = NULL;
  
	g_return_if_fail (G_IS_VOLUME (volume));
	g_return_if_fail (G_IS_ASYNC_RESULT (result));

	PraghaRemovablePluginPrivate *priv = plugin->priv;

	/* finish mounting the volume */
	if (!g_volume_mount_finish (volume, result, &error)) {
		if (error->code != G_IO_ERROR_FAILED_HANDLED &&
		    error->code != G_IO_ERROR_ALREADY_MOUNTED) {
			name = g_volume_get_name (G_VOLUME (volume));
			primary = g_strdup_printf (_("Unable to access \"%s\""), name);
			g_free (name);

			pragha_gudev_show_dialog (NULL, _("Renovable Device"), "media-removable",
			                          primary, error->message,
			                          NULL, PRAGHA_DEVICE_RESPONSE_NONE);

			g_free (primary);
		}
		g_error_free (error);
	}

	/* get the moint point of the volume */
	mount = g_volume_get_mount (volume);
	if (mount != NULL) {
		pragha_block_device_mounted (plugin, priv->u_device, mount, &error);
		g_object_unref (mount);
	}
	g_object_unref (volume);
}

static gboolean
pragha_block_device_mount (gpointer data)
{
	GVolumeMonitor  *monitor;
	GMountOperation *mount_operation;
	GVolume         *volume;

	PraghaRemovablePlugin *plugin = data;
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	monitor = g_volume_monitor_get ();

	/* determine the GVolume corresponding to the udev removable */
	volume = tvm_g_volume_monitor_get_volume_for_kind (monitor,
	                                                   G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE,
	                                                   g_udev_device_get_device_file (priv->u_device));
	g_object_unref (monitor);

	/* check if we have a volume */
	if (volume != NULL) {
		if (g_volume_can_mount (volume)) {
			/* try to mount the volume asynchronously */
			mount_operation = gtk_mount_operation_new (NULL);
			g_volume_mount (volume, G_MOUNT_MOUNT_NONE, mount_operation,
			                NULL, (GAsyncReadyCallback) pragha_block_device_mount_finish,
			                plugin);
			g_object_unref (mount_operation);
		}
	}

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

	/*
	 * HACK: We're listening udev. Then wait 2 seconds, to ensure that GVolume also detects the device.
	 */
	g_timeout_add_seconds(2, pragha_block_device_mount, plugin);
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
	PraghaRemovablePlugin *plugin = PRAGHA_REMOVABLE_PLUGIN (activatable);
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Removable plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	pragha_devices_plugin_connect_signals (G_CALLBACK(pragha_removable_plugin_device_added),
	                                       G_CALLBACK(pragha_removable_plugin_device_removed),
	                                       plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaRemovablePlugin *plugin = PRAGHA_REMOVABLE_PLUGIN (activatable);
	PraghaRemovablePluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Removable plugin %s", G_STRFUNC);

	pragha_devices_plugin_disconnect_signals (G_CALLBACK(pragha_removable_plugin_device_added),
	                                          G_CALLBACK(pragha_removable_plugin_device_removed),
	                                          plugin);

	priv->pragha = NULL;
}