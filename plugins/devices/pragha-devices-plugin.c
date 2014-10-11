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
	GUdevClient        *gudev_client;
};
typedef struct _PraghaDevicesPluginPrivate PraghaDevicesPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_DEVICES_PLUGIN,
                        PraghaDevicesPlugin,
                        pragha_devices_plugin)

static const gchar * gudev_subsystems[] =
{
	"block",
	"usb",
	NULL,
};


/*
 * Publics functions.
 */
GtkWidget *
pragha_gudev_dialog_new (GtkWidget *parent, const gchar *title, const gchar *icon,
                         const gchar *primary_text, const gchar *secondary_text,
                         const gchar *first_button_text, gint first_button_response)
{
	GtkWidget *dialog;
	GtkWidget *image;

	dialog = gtk_message_dialog_new (NULL,
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
		gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG (dialog), image);
	}
	if (secondary_text != NULL)
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (dialog), "%s", secondary_text);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), PRAGHA_DEVICE_RESPONSE_NONE);

	return dialog;
}

void
pragha_devices_plugin_connect_signals (GCallback added_callback,
                                       GCallback removed_callback,
                                       gpointer  user_data)
{
	PraghaDeviceClient *device_client;
	device_client = pragha_device_client_get();

	g_signal_connect (G_OBJECT(device_client), "device-added",
	                  G_CALLBACK(added_callback), user_data);
	g_signal_connect (G_OBJECT(device_client), "device-removed",
	                  G_CALLBACK(removed_callback), user_data);

	g_object_unref (device_client);
}

void
pragha_devices_plugin_disconnect_signals (GCallback added_callback,
                                          GCallback removed_callback,
                                          gpointer  user_data)
{
	PraghaDeviceClient *device_client;
	device_client = pragha_device_client_get();

	g_signal_handlers_disconnect_by_func (device_client,
	                                      added_callback,
	                                      user_data);
	g_signal_handlers_disconnect_by_func (device_client,
	                                      removed_callback,
	                                      user_data);

	g_object_unref (device_client);
}

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

/* Functions that manage to "add" "change" and "remove" devices events. */

static void
pragha_gudev_device_added (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type != PRAGHA_DEVICE_UNKNOWN)
		pragha_device_client_device_added (priv->device_client, device_type, device);
}

static void
pragha_gudev_device_changed (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type == PRAGHA_DEVICE_AUDIO_CD)
		pragha_device_client_device_added (priv->device_client, device_type, device);
}

static void
pragha_gudev_device_removed (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	device_type = pragha_gudev_get_device_type (device);
	if (device_type != PRAGHA_DEVICE_UNKNOWN)
		pragha_device_client_device_removed (priv->device_client, device_type, device);
}

/* Main devices functions that listen udev events. */

static void
gudev_uevent_cb (GUdevClient *client, const char *action, GUdevDevice *device, PraghaDevicesPlugin *plugin)
{
	if (g_str_equal(action, "add")) {
		pragha_gudev_device_added (plugin, device);
	}
	else if (g_str_equal(action, "change")) {
		pragha_gudev_device_changed (plugin, device);
	}
	else if (g_str_equal (action, "remove")) {
		pragha_gudev_device_removed (plugin, device);
	}
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Devices plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	priv->device_client = pragha_device_client_get ();

	priv->gudev_client = g_udev_client_new(gudev_subsystems);
	g_signal_connect (priv->gudev_client, "uevent",
	                  G_CALLBACK(gudev_uevent_cb), plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Devices plugin %s", G_STRFUNC);

	g_signal_handlers_disconnect_by_func (priv->gudev_client,
	                                      gudev_uevent_cb, plugin);
	g_object_unref (priv->gudev_client);

	g_object_unref (priv->device_client);

	priv->pragha = NULL;
}