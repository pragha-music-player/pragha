/*************************************************************************/
/* Copyright (C) 2014-2019 matias <mati86dl@gmail.com>                   */
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
#include <stdlib.h>

#include "pragha-device-client.h"

struct _PraghaDeviceClient {
	GObject             _parent;

	GUdevClient        *gudev_client;
};

enum {
	SIGNAL_DEVICE_ADDED,
	SIGNAL_DEVICE_REMOVED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaDeviceClient, pragha_device_client, G_TYPE_OBJECT)

static const
gchar * gudev_subsystems[] =
{
	"block",
	"usb",
	NULL,
};

/*
 * Publics functions.
 */

GtkWidget *
pragha_gudev_dialog_new (GtkWidget   *parent,
                         const gchar *title,
                         const gchar *icon,
                         const gchar *primary_text,
                         const gchar *secondary_text,
                         const gchar *first_button_text,
                         gint         first_button_response)
{
	GtkWidget *dialog;
	GtkWidget *image;

	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_QUESTION,
	                                 GTK_BUTTONS_NONE,
	                                 NULL);

	if (title != NULL)
		gtk_window_set_title (GTK_WINDOW (dialog), title);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), primary_text);

	gtk_dialog_add_button (GTK_DIALOG (dialog), _("Ignore"), PRAGHA_DEVICE_RESPONSE_NONE);
	if (first_button_text != NULL)
		gtk_dialog_add_button (GTK_DIALOG (dialog), first_button_text, first_button_response);

	if(icon != NULL) {
		image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG (dialog), image);
G_GNUC_END_IGNORE_DEPRECATIONS
	}
	if (secondary_text != NULL)
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (dialog), "%s", secondary_text);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), PRAGHA_DEVICE_RESPONSE_NONE);

	return dialog;
}

gint
pragha_gudev_get_property_as_int (GUdevDevice *device,
                                  const gchar *property,
                                  gint         base)
{
	const char *strvalue;

	strvalue = g_udev_device_get_property (device, property);
	if (strvalue == NULL) {
		return 0;
	}

	return strtol (strvalue, NULL, base);
}

/* Identify devices.*/

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
		if (g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA")) {
			audio_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");
			data_tracks = g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_DATA");

			if (audio_tracks > 0)
				return PRAGHA_DEVICE_AUDIO_CD;
		}
		else
			return PRAGHA_DEVICE_EMPTY_AUDIO_CD;
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

/* Functions that manage to "add" "change" and "remove" devices events. */

static void
pragha_gudev_device_added (PraghaDeviceClient *client, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type != PRAGHA_DEVICE_UNKNOWN)
		g_signal_emit (client, signals[SIGNAL_DEVICE_ADDED], 0, device_type, device);
}

static void
pragha_gudev_device_changed (PraghaDeviceClient *client, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type == PRAGHA_DEVICE_AUDIO_CD) {
		g_signal_emit (client, signals[SIGNAL_DEVICE_ADDED], 0, device_type, device);
	}
	else if (device_type == PRAGHA_DEVICE_EMPTY_AUDIO_CD) {
		g_signal_emit (client, signals[SIGNAL_DEVICE_REMOVED], 0, PRAGHA_DEVICE_AUDIO_CD, device);
	}
}

static void
pragha_gudev_device_removed (PraghaDeviceClient *client, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type != PRAGHA_DEVICE_UNKNOWN)
		g_signal_emit (client, signals[SIGNAL_DEVICE_REMOVED], 0, device_type, device);
}

static void
gudev_uevent_cb (GUdevClient *uclient, const char *action, GUdevDevice *device, PraghaDeviceClient *client)
{
	if (g_str_equal(action, "add")) {
		pragha_gudev_device_added (client, device);
	}
	else if (g_str_equal(action, "change")) {
		pragha_gudev_device_changed (client, device);
	}
	else if (g_str_equal (action, "remove")) {
		pragha_gudev_device_removed (client, device);
	}
}

/* Pragha device client object */

static void
pragha_device_client_dispose (GObject *object)
{
	PraghaDeviceClient *client = PRAGHA_DEVICE_CLIENT (object);

	if (client->gudev_client) {
		g_object_unref (client->gudev_client);
		client->gudev_client = NULL;
	}

	(*G_OBJECT_CLASS (pragha_device_client_parent_class)->dispose) (object);
}

static void
pragha_device_client_class_init (PraghaDeviceClientClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = pragha_device_client_dispose;

	signals[SIGNAL_DEVICE_ADDED] =
		g_signal_new ("device-added",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDeviceClientClass, device_added),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT_POINTER,
		              G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_POINTER);
	signals[SIGNAL_DEVICE_REMOVED] =
		g_signal_new ("device-removed",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDeviceClientClass, device_removed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT_POINTER,
		              G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_POINTER);
}

static void
pragha_device_client_init (PraghaDeviceClient *device_client)
{
	device_client->gudev_client = g_udev_client_new(gudev_subsystems);

	g_signal_connect (device_client->gudev_client, "uevent",
	                  G_CALLBACK(gudev_uevent_cb), device_client);

}

PraghaDeviceClient *
pragha_device_client_get (void)
{
	static PraghaDeviceClient *device_client = NULL;

	if (G_UNLIKELY (device_client == NULL)) {
		device_client = g_object_new (PRAGHA_TYPE_DEVICE_CLIENT, NULL);
		g_object_add_weak_pointer (G_OBJECT (device_client),
		                          (gpointer) &device_client);
	}
	else {
		g_object_ref (G_OBJECT(device_client));
	}

	return device_client;
}
