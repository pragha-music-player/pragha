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

#include <stdlib.h>

#include "pragha.h"
#include "pragha-devices.h"
#include "pragha-devices-block.h"
#include "pragha-devices-cd.h"
#include "pragha-devices-mtp.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-debug.h"

struct _PraghaDevices {
	struct con_win *cwin;

	GUdevClient *gudev_client;
	GUdevDevice *device;

	LIBMTP_mtpdevice_t *mtp_device;
	guint64 bus_hooked;
	guint64 device_hooked;

	GHashTable        *tracks_table;

	GtkActionGroup *action_group_playlist;
	guint merge_id_playlist;

};

static const gchar * gudev_subsystems[] =
{
	"block",
	"usb",
	NULL,
};

/* */

void
pragha_device_cache_append_tracks (PraghaDevices *devices)
{
	GHashTableIter iter;
	gpointer key, value;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;

	g_hash_table_iter_init (&iter, devices->tracks_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		mobj = value;
		if (G_LIKELY(mobj)) {
			list = g_list_append (list, mobj);
			g_object_ref (mobj);
		}

		/* Have to give control to GTK periodically ... */
		if (pragha_process_gtk_events ())
			return;
	}

	pragha_playlist_append_mobj_list (pragha_device_get_aplication(devices)->cplaylist, list);
	g_list_free(list);
}

void
pragha_device_cache_clear (PraghaDevices *devices)
{
	g_hash_table_remove_all (devices->tracks_table);
}

void
pragha_device_cache_insert_track (PraghaDevices *devices, PraghaMusicobject *mobj)
{
	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	g_hash_table_insert (devices->tracks_table,
	                     g_strdup(file),
	                     mobj);
}

/**/

gboolean
pragha_device_already_is_busy (PraghaDevices *devices)
{
	if (devices->bus_hooked != 0 &&
	    devices->device_hooked != 0)
		return TRUE;

	return FALSE;
}

gboolean
pragha_device_already_is_idle (PraghaDevices *devices)
{
	if (devices->bus_hooked == 0 &&
	    devices->device_hooked == 0)
		return TRUE;

	return FALSE;
}

GUdevDevice *
pragha_device_get_udev_device (PraghaDevices *devices)
{
	return devices->device;
}

LIBMTP_mtpdevice_t *
pragha_device_get_mtp_device (PraghaDevices *devices)
{
	return devices->mtp_device;
}

struct con_win *
pragha_device_get_aplication (PraghaDevices *devices)
{
	return devices->cwin;
}

/*
 * Generic dialog to get some action to responce to udev events.
 */

gint
pragha_gudev_show_dialog (const gchar *title, const gchar *icon,
                          const gchar *primary_text, const gchar *secondary_text,
                          const gchar *first_button_text, gint first_button_response)
{
	GtkWidget *dialog;
	GtkWidget *image;
	gint response = PRAGHA_DEVICE_RESPONSE_NONE;

	dialog = gtk_message_dialog_new (NULL,
	                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_QUESTION,
	                                 GTK_BUTTONS_NONE,
	                                 NULL);

	if (title != NULL)
		gtk_window_set_title (GTK_WINDOW (dialog), title);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), primary_text);

	gtk_dialog_add_button (GTK_DIALOG (dialog), _("Ignore"), PRAGHA_DEVICE_RESPONSE_NONE);
	gtk_dialog_add_button (GTK_DIALOG (dialog), first_button_text, first_button_response);

	if(icon != NULL) {
		image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
		gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG (dialog), image);
	}
	if (secondary_text != NULL)
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (dialog), secondary_text);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), PRAGHA_DEVICE_RESPONSE_NONE);

	gtk_widget_show_all(dialog);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return response;
}

/*
 * Check the type of device that listens udev to act accordingly.
 */

static gint
pragha_gudev_get_device_type (GUdevDevice *device)
{
	const gchar *devtype;
	const gchar *id_type;
	const gchar *id_fs_usage;
	gboolean     is_cdrom;
	gboolean     is_partition;
	gboolean     is_volume;
	guint64      audio_tracks = 0;
	guint64      data_tracks = 0;
	guint64      is_mtpdevice = 0;

	/* collect general devices information */

	id_type = g_udev_device_get_property (device, "ID_TYPE");

	is_cdrom = (g_strcmp0 (id_type, "cd") == 0);
	if (is_cdrom) {
		/* silently ignore CD drives without media */
		if (g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA")) {

			audio_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");
			data_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_DATA");

			if (audio_tracks > 0)
				return PRAGHA_DEVICE_AUDIO_CD;
		}
	}

	devtype = g_udev_device_get_property (device, "DEVTYPE");
	id_fs_usage = g_udev_device_get_property (device, "ID_FS_USAGE");

	is_partition = (g_strcmp0 (devtype, "partition") == 0);
	is_volume = (g_strcmp0 (devtype, "disk") == 0) && (g_strcmp0 (id_fs_usage, "filesystem") == 0);

	if (is_partition || is_volume || data_tracks)
		return PRAGHA_DEVICE_MOUNTABLE;

	is_mtpdevice = g_udev_device_get_property_as_uint64 (device, "ID_MTP_DEVICE");
	if (is_mtpdevice)
		return PRAGHA_DEVICE_MTP;

	return PRAGHA_DEVICE_UNKNOWN;
}

void
pragha_gudev_clear_hook_devices (PraghaDevices *devices)
{
	CDEBUG(DBG_INFO, "Clear hooked device, Bus: %ld, Dev: %ld", devices->bus_hooked, devices->device_hooked);

	if (devices->device) {
		g_object_unref (devices->device);
		devices->device = NULL;
	}
	if (devices->mtp_device) {
		LIBMTP_Release_Device(devices->mtp_device);
		devices->mtp_device = NULL;
	}

	pragha_device_cache_clear (devices);

	devices->bus_hooked = 0;
	devices->device_hooked = 0;
}

void
pragha_gudev_set_hook_device (PraghaDevices *devices, GUdevDevice *device, LIBMTP_mtpdevice_t *mtp_device, guint64 busnum, guint64 devnum)
{
	devices->device = g_object_ref (device);
	devices->mtp_device = mtp_device;
	devices->bus_hooked = busnum;
	devices->device_hooked = devnum;
}

/* Functions that manage to "add" "change" and "remove" devices events. */

static void
pragha_gudev_device_added (PraghaDevices *devices, GUdevDevice *device)
{
	gint device_type = 0;

	device_type = pragha_gudev_get_device_type (device);

	switch (device_type) {
		case PRAGHA_DEVICE_MOUNTABLE:
			pragha_devices_moutable_added (devices, device);
			break;
		case PRAGHA_DEVICE_AUDIO_CD:
			pragha_devices_audio_cd_added (devices);
			break;
		case PRAGHA_DEVICE_MTP:
			pragha_devices_mtp_added (devices, device);
		case PRAGHA_DEVICE_UNKNOWN:
		default:
			break;
	}
}

static void
pragha_gudev_device_changed (PraghaDevices *devices, GUdevDevice *device)
{
	gint device_type = 0;

	device_type = pragha_gudev_get_device_type (device);

	if (device_type == PRAGHA_DEVICE_AUDIO_CD)
		pragha_devices_audio_cd_added (devices);
}

static void
pragha_gudev_device_removed (PraghaDevices *devices, GUdevDevice *device)
{
	guint64 busnum = 0;
	guint64 devnum = 0;

	if (devices->bus_hooked == 0 &&
	    devices->device_hooked == 0)
		return;

	busnum = g_udev_device_get_property_as_uint64 (device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64 (device, "DEVNUM");

	if (devices->bus_hooked == busnum &&
	    devices->device_hooked == devnum) {
		pragha_gudev_clear_hook_devices (devices);

		pragha_devices_remove_playlist_action (devices);
	}
}

/* Main devices functions that listen udev events. */

static void
gudev_uevent_cb(GUdevClient *client, const char *action, GUdevDevice *device, PraghaDevices *devices)
{
	if (g_str_equal(action, "add")) {
		pragha_gudev_device_added (devices, device);
	}
	else if (g_str_equal(action, "change")) {
		pragha_gudev_device_changed (devices, device);
	}
	else if (g_str_equal (action, "remove")) {
		pragha_gudev_device_removed (devices, device);
	}
}

/* Add send to menu option.. */

void
pragha_devices_append_playlist_action (PraghaDevices *devices, GtkActionGroup *action_group, const gchar *menu_xml)
{
	devices->action_group_playlist = action_group;
	devices->merge_id_playlist = pragha_playlist_append_plugin_action (devices->cwin->cplaylist,
	                                                                   devices->action_group_playlist,
	                                                                   menu_xml);
}

void
pragha_devices_remove_playlist_action (PraghaDevices *devices)
{
	if (!devices->merge_id_playlist)
		return;

	pragha_playlist_remove_plugin_action (devices->cwin->cplaylist,
	                                      devices->action_group_playlist,
	                                      devices->merge_id_playlist);

	devices->merge_id_playlist = 0;
}

/* Init gudev subsysten, and listen events. */

void
pragha_devices_free(PraghaDevices *devices)
{
	if(devices->bus_hooked != 0 &&
	   devices->device_hooked != 0)
		pragha_gudev_clear_hook_devices (devices);

	g_hash_table_destroy(devices->tracks_table);

	g_object_unref(devices->gudev_client);

	g_slice_free (PraghaDevices, devices);
}

PraghaDevices *
pragha_devices_new (struct con_win *cwin)
{
	PraghaDevices *devices;

	devices = g_slice_new0(PraghaDevices);

	devices->cwin = cwin;
	devices->bus_hooked = 0;
	devices->device_hooked = 0;

	devices->device = NULL;
	devices->mtp_device = NULL;

	devices->tracks_table = g_hash_table_new_full (g_str_hash,
	                                               g_str_equal,
	                                               g_free,
	                                               g_object_unref);

	devices->gudev_client = g_udev_client_new(gudev_subsystems);

	LIBMTP_Init();

	g_signal_connect (devices->gudev_client, "uevent", G_CALLBACK(gudev_uevent_cb), devices);

	g_signal_connect (cwin->backend, "prepare-source", G_CALLBACK(pragha_devices_prepare_mtp_temp_file), devices);

	return devices;
}
