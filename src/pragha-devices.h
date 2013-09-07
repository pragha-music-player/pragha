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

#ifndef PRAGHA_DEVICES_H
#define PRAGHA_DEVICES_H

#include <gudev/gudev.h>
#include <libmtp.h>

/* pragha.h */
struct con_win;

typedef struct _PraghaDevices PraghaDevices;

enum
{
	PRAGHA_DEVICE_RESPONSE_NONE,
	PRAGHA_DEVICE_RESPONSE_PLAY,
	PRAGHA_DEVICE_RESPONSE_BROWSE,
};

enum
{
	PRAGHA_DEVICE_MOUNTABLE,
	PRAGHA_DEVICE_AUDIO_CD,
	PRAGHA_DEVICE_MTP,
	PRAGHA_DEVICE_UNKNOWN,
};

gboolean            pragha_device_already_is_busy   (PraghaDevices *devices);
gboolean            pragha_device_already_is_idle   (PraghaDevices *devices);

GUdevDevice        *pragha_device_get_udev_device   (PraghaDevices *devices);
LIBMTP_mtpdevice_t *pragha_device_get_mtp_device    (PraghaDevices *devices);
struct con_win     *pragha_device_get_aplication    (PraghaDevices *devices);

void                pragha_gudev_set_hook_device    (PraghaDevices *devices, GUdevDevice *device, LIBMTP_mtpdevice_t *mtp_device, guint64 busnum, guint64 devnum);
void                pragha_gudev_clear_hook_devices (PraghaDevices *devices);

gint
pragha_gudev_show_dialog (const gchar *title, const gchar *icon,
                          const gchar *primary_text, const gchar *secondary_text,
                          const gchar *first_button_text, gint first_button_response);

void           pragha_devices_append_playlist_action (PraghaDevices *devices, GtkActionGroup *action_group, const gchar *menu_xml);
void           pragha_devices_remove_playlist_action (PraghaDevices *devices);

void           pragha_devices_free (PraghaDevices *devices);
PraghaDevices *pragha_devices_new  (struct con_win *cwin);

#endif /* PRAGHA_DEVICES_H */
