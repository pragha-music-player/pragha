/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#include "pragha-device-client.h"

struct _PraghaDeviceClient {
	GObject             _parent;
};

enum {
	SIGNAL_DEVICE_ADDED,
	SIGNAL_DEVICE_REMOVED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaDeviceClient, pragha_device_client, G_TYPE_OBJECT)

void
pragha_device_client_device_added (PraghaDeviceClient *device_client,
                                   PraghaDeviceType    device_type,
                                   GUdevDevice        *u_device)
{
	g_signal_emit (device_client, signals[SIGNAL_DEVICE_ADDED], 0, device_type, u_device);
}

void
pragha_device_client_device_removed (PraghaDeviceClient *device_client,
                                     PraghaDeviceType    device_type,
                                     GUdevDevice        *u_device)
{
	g_signal_emit (device_client, signals[SIGNAL_DEVICE_REMOVED], 0, device_type, u_device);
}

static void
pragha_device_client_class_init (PraghaDeviceClientClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);

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