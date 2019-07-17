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

#ifndef PRAGHA_DEVICE_CLIENT_H
#define PRAGHA_DEVICE_CLIENT_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <gudev/gudev.h>

G_BEGIN_DECLS

/* Device types */

typedef enum {
	PRAGHA_DEVICE_NONE = 0,
	PRAGHA_DEVICE_MOUNTABLE,
	PRAGHA_DEVICE_AUDIO_CD,
	PRAGHA_DEVICE_EMPTY_AUDIO_CD,
	PRAGHA_DEVICE_MTP,
	PRAGHA_DEVICE_UNKNOWN
} PraghaDeviceType;

#define PRAGHA_TYPE_DEVICE_CLIENT (pragha_device_client_get_type())
#define PRAGHA_DEVICE_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_DEVICE_CLIENT, PraghaDeviceClient))
#define PRAGHA_DEVICE_CLIENT_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_DEVICE_CLIENT, PraghaDeviceClient const))
#define PRAGHA_DEVICE_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_DEVICE_CLIENT, PraghaDeviceClientClass))
#define PRAGHA_IS_DEVICE_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_DEVICE_CLIENT))
#define PRAGHA_IS_DEVICE_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_DEVICE_CLIENT))
#define PRAGHA_DEVICE_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_DEVICE_CLIENT, PraghaDeviceClientClass))

typedef struct _PraghaDeviceClient PraghaDeviceClient;
typedef struct _PraghaDeviceClientClass PraghaDeviceClientClass;

struct _PraghaDeviceClientClass
{
	GObjectClass parent_class;
	void (*device_added)   (PraghaDeviceClient *device_client,
	                        PraghaDeviceType    device_type,
	                        GUdevDevice        *u_device);
	void (*device_removed) (PraghaDeviceClient *device_client,
	                        PraghaDeviceType    device_type,
	                        GUdevDevice        *u_device);
};

/* Dialog when add device */

enum
{
	PRAGHA_DEVICE_RESPONSE_NONE,
	PRAGHA_DEVICE_RESPONSE_PLAY,
	PRAGHA_DEVICE_RESPONSE_BROWSE,
};

GtkWidget *
pragha_gudev_dialog_new (GtkWidget   *parent,
                         const gchar *title,
                         const gchar *icon,
                         const gchar *primary_text,
                         const gchar *secondary_text,
                         const gchar *first_button_text,
                         gint         first_button_response);

gint
pragha_gudev_get_property_as_int (GUdevDevice *device,
                                  const gchar *property,
                                  gint         base);

/* Create a new instance of PraghaDeviceClient* */

PraghaDeviceClient *pragha_device_client_get          (void);

G_END_DECLS

#endif /* PRAGHA_DEVICE_CLIENT_H */
