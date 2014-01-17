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

#include "pragha-devices-plugin.h"
#include "pragha-devices-block.h"
#include "pragha-devices-mtp.h"
#include "pragha-devices-cd.h"

#include "src/pragha-playback.h"
#include "src/pragha-utils.h"
#include "src/pragha.h"

struct _PraghaDevicesPluginPrivate {
	PraghaApplication  *pragha;

	GUdevClient        *gudev_client;
	GUdevDevice        *device;
	LIBMTP_mtpdevice_t *mtp_device;

	guint64             bus_hooked;
	guint64             device_hooked;
	PraghaDeviceType    hooked_type;

	GHashTable         *tracks_table;

	GtkActionGroup     *action_group_menu;
	guint               merge_id_menu;

	GtkActionGroup     *action_group_playlist;
	guint               merge_id_playlist;
};

PRAGHA_PLUGIN_REGISTER_PRIVATE_CODE (PRAGHA_TYPE_DEVICES_PLUGIN,
                                     PraghaDevicesPlugin,
                                     pragha_devices_plugin)

static const gchar * gudev_subsystems[] =
{
	"block",
	"usb",
	NULL,
};

void
pragha_device_cache_append_tracks (PraghaDevicesPlugin *plugin)
{
	PraghaPlaylist *playlist;
	GHashTableIter iter;
	gpointer key, value;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;

	PraghaDevicesPluginPrivate *priv = plugin->priv;

	g_hash_table_iter_init (&iter, priv->tracks_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		mobj = value;
		if (G_LIKELY(mobj)) {
			list = g_list_append (list, mobj);
			g_object_ref (mobj);
		}
		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();
	}

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_append_mobj_list (playlist, list);
	g_list_free(list);
}

void
pragha_device_cache_clear (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	g_hash_table_remove_all (priv->tracks_table);
}

void
pragha_device_cache_insert_track (PraghaDevicesPlugin *plugin, PraghaMusicobject *mobj)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	g_hash_table_insert (priv->tracks_table,
	                     g_strdup(file),
	                     mobj);
}

/**/

gboolean
pragha_device_already_is_busy (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	return (priv->hooked_type != PRAGHA_DEVICE_NONE);
}

gboolean
pragha_device_already_is_idle (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	return (priv->hooked_type == PRAGHA_DEVICE_NONE);
}

GUdevDevice *
pragha_device_get_udev_device (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	return priv->device;
}

LIBMTP_mtpdevice_t *
pragha_device_get_mtp_device (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	return priv->mtp_device;
}

PraghaApplication *
pragha_device_get_application (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	return priv->pragha;
}

/*
 * Generic dialog to get some action to responce to udev events.
 */

gint
pragha_gudev_show_dialog (GtkWidget *parent, const gchar *title, const gchar *icon,
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
pragha_gudev_clear_hook_devices (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_INFO, "Clear hooked device, Bus: %ld, Dev: %ld", priv->bus_hooked, priv->device_hooked);

	if (priv->device) {
		g_object_unref (priv->device);
		priv->device = NULL;
	}
	if (priv->mtp_device) {
		LIBMTP_Release_Device(priv->mtp_device);
		priv->mtp_device = NULL;
	}

	pragha_device_cache_clear (plugin);

	priv->hooked_type   = PRAGHA_DEVICE_NONE;
	priv->bus_hooked    = 0;
	priv->device_hooked = 0;
}

void
pragha_gudev_set_hook_device (PraghaDevicesPlugin *plugin,
                              PraghaDeviceType     device_type,
                              GUdevDevice         *device,
                              LIBMTP_mtpdevice_t  *mtp_device,
                              guint64              busnum,
                              guint64              devnum)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	priv->hooked_type   = device_type;
	priv->device        = g_object_ref (device);
	priv->mtp_device    = mtp_device;
	priv->bus_hooked    = busnum;
	priv->device_hooked = devnum;
}

/* Functions that manage to "add" "change" and "remove" devices events. */

static void
pragha_gudev_device_added (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	PraghaDeviceType device_type = PRAGHA_DEVICE_UNKNOWN;

	device_type = pragha_gudev_get_device_type (device);

	switch (device_type) {
		case PRAGHA_DEVICE_MOUNTABLE:
			pragha_devices_moutable_added (plugin, device);
			break;
		case PRAGHA_DEVICE_AUDIO_CD:
			pragha_devices_audio_cd_added (plugin);
			break;
		case PRAGHA_DEVICE_MTP:
			pragha_devices_mtp_added (plugin, device);
		case PRAGHA_DEVICE_UNKNOWN:
		case PRAGHA_DEVICE_NONE:
		default:
			break;
	}
}

static void
pragha_gudev_device_changed (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	gint device_type = 0;

	device_type = pragha_gudev_get_device_type (device);

	if (device_type == PRAGHA_DEVICE_AUDIO_CD)
		pragha_devices_audio_cd_added (plugin);
}

static void
pragha_gudev_device_removed (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	guint64 busnum = 0;
	guint64 devnum = 0;

	PraghaDevicesPluginPrivate *priv = plugin->priv;

	if (priv->hooked_type == PRAGHA_DEVICE_NONE)
		return;

	busnum = g_udev_device_get_property_as_uint64 (device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64 (device, "DEVNUM");

	if (priv->bus_hooked == busnum &&
	    priv->device_hooked == devnum) {
		pragha_gudev_clear_hook_devices (plugin);

		pragha_devices_remove_playlist_action (plugin);
	}
}

/* Main devices functions that listen udev events. */

static void
gudev_uevent_cb(GUdevClient *client, const char *action, GUdevDevice *device, PraghaDevicesPlugin *plugin)
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

/* Add send to menu option.. */

void
pragha_devices_append_playlist_action (PraghaDevicesPlugin *plugin, GtkActionGroup *action_group, const gchar *menu_xml)
{
	PraghaPlaylist *playlist;

	PraghaDevicesPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);

	priv->action_group_playlist = action_group;
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                priv->action_group_playlist,
	                                                                menu_xml);
}

void
pragha_devices_remove_playlist_action (PraghaDevicesPlugin *plugin)
{
	PraghaPlaylist *playlist;

	PraghaDevicesPluginPrivate *priv = plugin->priv;

	if (!priv->merge_id_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_menu,
	                                     priv->merge_id_menu);

	priv->merge_id_menu = 0;

	if (!priv->merge_id_playlist)
		return;

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);

	priv->merge_id_playlist = 0;
}

void
pragha_devices_plugin_append_menu_action (PraghaDevicesPlugin *plugin, GtkActionGroup *action_group, const gchar *menu_xml)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	priv->action_group_menu = action_group;
	priv->merge_id_menu     = pragha_menubar_append_plugin_action (priv->pragha,
	                                                               priv->action_group_menu,
	                                                               menu_xml);
}

void
pragha_devices_plugin_remove_menu_action (PraghaDevicesPlugin *plugin)
{
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	if (!priv->merge_id_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_menu,
	                                     priv->merge_id_menu);

	priv->merge_id_menu = 0;
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	g_debug ("%s", G_STRFUNC);

	priv->tracks_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	priv->gudev_client = g_udev_client_new(gudev_subsystems);

	LIBMTP_Init ();

	g_signal_connect (priv->gudev_client, "uevent",
	                  G_CALLBACK(gudev_uevent_cb), plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDevicesPlugin *plugin = PRAGHA_DEVICES_PLUGIN (activatable);
	PraghaDevicesPluginPrivate *priv = plugin->priv;

	g_debug ("%s", G_STRFUNC);

	if (priv->hooked_type != PRAGHA_DEVICE_NONE)
		pragha_gudev_clear_hook_devices (plugin);

	g_hash_table_destroy (priv->tracks_table);

	g_signal_handlers_disconnect_by_func (priv->gudev_client,
	                                      gudev_uevent_cb, plugin);

	g_object_unref (priv->gudev_client);

	priv->pragha = NULL;
}