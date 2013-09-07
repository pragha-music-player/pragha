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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <gudev/gudev.h>
#include <stdlib.h>

#include "pragha.h"
#include "pragha-devices.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-debug.h"

/*
 * Some functions to mount block devices.
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

/* The next funtions allow to handle block devicess.
 * The most of code is inspired in:
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-block-devices.c */

static void
pragha_block_device_mounted (GUdevDevice *device, GMount *mount, GError **error)
{
	const gchar *volume_name;
	gchar       *decoded_name;
	gchar       *message;
	GFile       *mount_point;
	gchar       *mount_path;
	gint         response;

	g_return_if_fail (G_IS_MOUNT (mount));
	g_return_if_fail (error == NULL || *error == NULL);

	volume_name = g_udev_device_get_property (device, "ID_FS_LABEL_ENC");
	decoded_name = tvm_notify_decode (volume_name);
  
	if (decoded_name != NULL)
		message = g_strdup_printf (_("The volume \"%s\" was mounted automatically"), decoded_name);
	else
		message = g_strdup_printf (_("The inserted volume was mounted automatically"));

	response = pragha_gudev_show_dialog (_("Audio/Data CD"), "gnome-fs-executable",
	                                     message, NULL,
	                                     _("Open folder"), PRAGHA_DEVICE_RESPONSE_BROWSE);
	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_BROWSE:
			mount_point = g_mount_get_root (mount);
			mount_path = g_file_get_path (mount_point);

			open_url (mount_path, NULL);

			g_object_unref (mount_point);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}

	g_free (decoded_name);
	g_free (message);
}

static void
pragha_block_device_mount_finish (GVolume *volume, GAsyncResult *result, PraghaDevices *devices)
{
	GMount *mount;
	GError *error = NULL;
  
	g_return_if_fail (G_IS_VOLUME (volume));
	g_return_if_fail (G_IS_ASYNC_RESULT (result));

	/* finish mounting the volume */
	if (g_volume_mount_finish (volume, result, &error)) {
		/* get the moint point of the volume */
		mount = g_volume_get_mount (volume);

		if (mount != NULL) {
			pragha_block_device_mounted (pragha_device_get_udev_device(devices), mount, &error);
			g_object_unref (mount);
		}
	}

	if(error != NULL) {
		// TODO: Check G_IO_ERROR_ALREADY_MOUNTED and ignore it..
		pragha_gudev_clear_hook_devices(devices);
	}

	g_object_unref (volume);
}

static gboolean
pragha_block_device_mount (gpointer data)
{
	GVolumeMonitor  *monitor;
	GMountOperation *mount_operation;
	GVolume         *volume;

	PraghaDevices *devices = data;

	monitor = g_volume_monitor_get ();

	/* determine the GVolume corresponding to the udev devices */
	volume = tvm_g_volume_monitor_get_volume_for_kind (monitor,
	                                                   G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE,
	                                                   g_udev_device_get_device_file (pragha_device_get_udev_device(devices)));
	g_object_unref (monitor);

	/* check if we have a volume */
	if (volume != NULL) {
		if (g_volume_can_mount (volume)) {
			/* try to mount the volume asynchronously */
			mount_operation = gtk_mount_operation_new (NULL);
			g_volume_mount (volume, G_MOUNT_MOUNT_NONE, mount_operation,
			                NULL, (GAsyncReadyCallback) pragha_block_device_mount_finish,
			                devices);
			g_object_unref (mount_operation);
		}
	}

	return FALSE;
}

void
pragha_devices_moutable_added (PraghaDevices *devices, GUdevDevice *device)
{
	guint64 busnum = 0;
	guint64 devnum = 0;

	if (pragha_device_already_is_busy (devices))
		return;

	busnum = g_udev_device_get_property_as_uint64(device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64(device, "DEVNUM");

	pragha_gudev_set_hook_device (devices, device, NULL, busnum, devnum);

	CDEBUG(DBG_INFO, "Hook a new mountable device, Bus: %ld, Dev: %ld", busnum, devnum);

	/*
	 * HACK: We're listening udev. Then wait 5 seconds, to ensure that GVolume also detects the device.
	 */
	g_timeout_add_seconds(5, pragha_block_device_mount, devices);
}