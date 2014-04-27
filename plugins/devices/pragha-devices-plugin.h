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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <gudev/gudev.h>

G_BEGIN_DECLS

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

void
pragha_devices_plugin_connect_signals    (GCallback added_callback,
                                          GCallback removed_callback,
                                          gpointer  user_data);

void
pragha_devices_plugin_disconnect_signals (GCallback added_callback,
                                          GCallback removed_callback,
                                          gpointer  user_data);

G_END_DECLS

#endif /* __PRAGHA_DEVICES_PLUGIN_H__ */
