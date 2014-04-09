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

#ifndef __PRAGHA_DEVICES_PLUGIN_H__
#define __PRAGHA_DEVICES_PLUGIN_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

#include <gudev/gudev.h>
#include <libmtp.h>

#include "plugins/pragha-plugin-macros.h"

#include "src/pragha.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_DEVICES_PLUGIN         (pragha_devices_plugin_get_type ())
#define PRAGHA_DEVICES_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPlugin))
#define PRAGHA_DEVICES_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPlugin))
#define PRAGHA_IS_DEVICES_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_DEVICES_PLUGIN))
#define PRAGHA_IS_DEVICES_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_DEVICES_PLUGIN))
#define PRAGHA_DEVICES_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_DEVICES_PLUGIN, PraghaDevicesPluginClass))

typedef struct _PraghaDevicesPluginPrivate PraghaDevicesPluginPrivate;

PRAGHA_PLUGIN_REGISTER_PUBLIC_HEADER (PRAGHA_TYPE_DEVICES_PLUGIN,
                                      PraghaDevicesPlugin,
                                      pragha_devices_plugin)

enum
{
	PRAGHA_DEVICE_RESPONSE_NONE,
	PRAGHA_DEVICE_RESPONSE_PLAY,
	PRAGHA_DEVICE_RESPONSE_BROWSE,
};

gint
pragha_gudev_show_dialog (GtkWidget *parent, const gchar *title, const gchar *icon,
                          const gchar *primary_text, const gchar *secondary_text,
                          const gchar *first_button_text, gint first_button_response);

typedef enum {
	PRAGHA_DEVICE_NONE = 0,
	PRAGHA_DEVICE_MOUNTABLE,
	PRAGHA_DEVICE_AUDIO_CD,
	PRAGHA_DEVICE_MTP,
	PRAGHA_DEVICE_UNKNOWN
} PraghaDeviceType;

void                pragha_device_cache_append_tracks        (PraghaDevicesPlugin *plugin);
void                pragha_device_cache_clear                (PraghaDevicesPlugin *plugin);
void                pragha_device_cache_insert_track         (PraghaDevicesPlugin *plugin, PraghaMusicobject *mobj);

gboolean            pragha_device_already_is_busy            (PraghaDevicesPlugin *plugin);
gboolean            pragha_device_already_is_idle            (PraghaDevicesPlugin *plugin);

GUdevDevice        *pragha_device_get_udev_device            (PraghaDevicesPlugin *plugin);
LIBMTP_mtpdevice_t *pragha_device_get_mtp_device             (PraghaDevicesPlugin *plugin);
PraghaApplication  *pragha_device_get_application            (PraghaDevicesPlugin *plugin);

void                pragha_gudev_set_hook_device             (PraghaDevicesPlugin *plugin, PraghaDeviceType device_type, GUdevDevice *device, LIBMTP_mtpdevice_t *mtp_device, guint64 busnum, guint64 devnum);
void                pragha_gudev_clear_hook_devices          (PraghaDevicesPlugin *plugin);

void                pragha_devices_append_playlist_action    (PraghaDevicesPlugin *plugin, GtkActionGroup *action_group, const gchar *menu_xml);
void                pragha_devices_remove_playlist_action    (PraghaDevicesPlugin *plugin);
void                pragha_devices_plugin_append_menu_action (PraghaDevicesPlugin *plugin, GtkActionGroup *action_group, const gchar *menu_xml);
void                pragha_devices_remove_menu_action        (PraghaDevicesPlugin *plugin);

G_END_DECLS

#endif /* __PRAGHA_DEVICES_PLUGIN_H__ */
