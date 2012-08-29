/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
/*									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

#ifdef HAVE_GUDEV

#include <gudev/gudev.h>

struct con_udev {
	struct con_win *cwin;
	GUdevDevice *device;
	guint64 bus_hooked;
	guint64 device_hooked;
};

const char * const gudev_subsystems[] = { "block", NULL };

/* Extentions copy of Thunar-volman code.
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-gio-extensions.c
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-gio-extensions.c */

gchar *
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

GVolume *
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

	g_message("Kind: %s, Identifier: %s", kind, identifier);

	volumes = g_volume_monitor_get_volumes (monitor);

	for (lp = volumes; volume == NULL && lp != NULL; lp = lp->next) {
		value = g_volume_get_identifier (lp->data, kind);
		if (value == NULL)
			continue;
		if (g_strcmp0 (value, identifier) == 0)
			volume = g_object_ref (lp->data);
		g_message("Get identifier: %s", value);
		g_free (value);
	}
	g_list_foreach (volumes, (GFunc)g_object_unref, NULL);
	g_list_free (volumes);

	return volume;
}

/* The next funtions allow to handle block devices.
 * The most of code is inspired in:
 * http://git.xfce.org/xfce/thunar-volman/tree/thunar-volman/tvm-block-device.c */

static void
pragha_block_device_mounted (GUdevDevice *device, GMount *mount, GError **error)
{
	const gchar *volume_name;
	gchar       *decoded_name;
	gchar       *message;
	GFile   *mount_point;
	gchar   *mount_path;

	g_return_if_fail (G_IS_MOUNT (mount));
	g_return_if_fail (error == NULL || *error == NULL);

	volume_name = g_udev_device_get_property (device, "ID_FS_LABEL_ENC");
	decoded_name = tvm_notify_decode (volume_name);
  
	if (decoded_name != NULL)
		message = g_strdup_printf (_("The volume \"%s\" was mounted automatically"), decoded_name);
	else
		message = g_strdup_printf (_("The inserted volume was mounted automatically"));

	g_message(message);
	mount_point = g_mount_get_root (mount);
	mount_path = g_file_get_path (mount_point);
	g_object_unref (mount_point);
	g_message(mount_path);

	g_free (decoded_name);
	g_free (message);
}

static void
pragha_block_device_mount_finish (GVolume *volume, GAsyncResult *result, GUdevDevice *device)
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
			pragha_block_device_mounted (device, mount, &error);
			g_object_unref (mount);
		}
		else {
			g_message("Unable to locate mount point");
		}
	}
	g_object_unref (volume);
}

gboolean
pragha_block_device_mount (gpointer data)
{
	GVolumeMonitor  *monitor;
	GMountOperation *mount_operation;
	GVolume         *volume;

	struct con_udev *cudev = data;

	monitor = g_volume_monitor_get ();

	/* determine the GVolume corresponding to the udev device */
	volume = tvm_g_volume_monitor_get_volume_for_kind (monitor,
							   G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE,
							   g_udev_device_get_device_file (cudev->device));
	g_object_unref (monitor);

	/* check if we have a volume */
	if (volume != NULL) {
		if (g_volume_can_mount (volume)) {
			/* try to mount the volume asynchronously */
			mount_operation = gtk_mount_operation_new (NULL);
			g_volume_mount (volume, G_MOUNT_MOUNT_NONE, mount_operation,
					NULL, (GAsyncReadyCallback) pragha_block_device_mount_finish, cudev->device);
			g_object_unref (mount_operation);
		}
		else {
			g_message("Unable to mount the device");
		}
	}
	else {
		g_message("Could not detect the volume corresponding to the device.");
	}

	return FALSE;
}

/* Functions that manage to "add" and "remove" devices events. */

static void
pragha_gudev_device_added(GUdevDevice *device, struct con_udev *cudev)
{
	const gchar *devtype;
	const gchar *id_type;
	const gchar *id_fs_usage;
	gboolean     is_cdrom;
	gboolean     is_partition;
	gboolean     is_volume;
	guint64      busnum = 0;
	guint64      devnum = 0;
	guint64      audio_tracks = 0;
	guint64      data_tracks = 0;

	/* collect general device information */
	devtype = g_udev_device_get_property (device, "DEVTYPE");
	id_type = g_udev_device_get_property (device, "ID_TYPE");
	id_fs_usage = g_udev_device_get_property (device, "ID_FS_USAGE");

	/* distinguish device types */
	is_cdrom = (g_strcmp0 (id_type, "cd") == 0);
	is_partition = (g_strcmp0 (devtype, "partition") == 0);
	is_volume = (g_strcmp0 (devtype, "disk") == 0) && (g_strcmp0 (id_fs_usage, "filesystem") == 0);

	if(is_cdrom) {
		/* silently ignore CD drives without media */
		if (g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA")) {
			audio_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");
			data_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_DATA");

			if (audio_tracks > 0) {
				gdk_threads_enter ();
				add_audio_cd(cudev->cwin);
				gdk_threads_leave ();
			}
		}
	}
	if (is_partition || is_volume || data_tracks) {
		if(cudev->bus_hooked == 0 &&
		   cudev->device_hooked == 0) {
			busnum = g_udev_device_get_property_as_uint64(device, "BUSNUM");
			devnum = g_udev_device_get_property_as_uint64(device, "DEVNUM");

			cudev->device = g_object_ref (device);
			cudev->bus_hooked = busnum;
			cudev->device_hooked = devnum;

			g_message("Device mountable added... . .\n");
			g_timeout_add_seconds(5, pragha_block_device_mount, cudev);
		}
	}
}

static void
pragha_gudev_device_removed(GUdevDevice *device, struct con_udev *cudev)
{
	guint64      busnum = 0;
	guint64      devnum = 0;

	busnum = g_udev_device_get_property_as_uint64(device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64(device, "DEVNUM");

	if(cudev->bus_hooked == busnum &&
	   cudev->device_hooked == devnum) {
		g_message("Device removed... . .\n");

		cudev->bus_hooked = 0;
		cudev->device_hooked = 0;
	}
}

/* Main devices function that listen udev events. */

static void
gudev_uevent_cb(GUdevClient *client, const char *action, GUdevDevice *device, struct con_udev *cudev)
{
	if (g_str_equal(action, "add")) {
		pragha_gudev_device_added(device, cudev);
	}
	else if (g_str_equal (action, "remove")) {
		pragha_gudev_device_removed(device, cudev);
	}
}

/* Init gudev subsysten, and listen events. */

gint
init_gudev_subsystem(struct con_win *cwin)
{
	GUdevClient *gudev_client;
	struct con_udev *cudev;

	cudev = g_slice_new0(struct con_udev);

	cudev->cwin = cwin;
	cudev->bus_hooked = 0;
	cudev->device_hooked = 0;

	gudev_client = g_udev_client_new(gudev_subsystems);
	g_signal_connect(gudev_client, "uevent", G_CALLBACK(gudev_uevent_cb), cudev);

	return 0;
}
#endif	