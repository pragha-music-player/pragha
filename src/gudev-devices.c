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

const char * const gudev_subsystems[] = { "block", NULL };

static void
uevent_cb(GUdevClient *client, const char *action, GUdevDevice *device, struct con_win *cwin)
{
	const gchar *id_type;
	const gchar *media_state;
	gboolean     is_cdrom;
	guint64      audio_tracks;

	/* collect general device information */
	id_type = g_udev_device_get_property (device, "ID_TYPE");

	/* distinguish device types */

	is_cdrom = (g_strcmp0 (id_type, "cd") == 0);

	if(is_cdrom) {
		/* silently ignore CD drives without media */
		if (g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA")) {
			/* collect CD information */

			media_state = g_udev_device_get_property (device, "ID_CDROM_MEDIA_STATE");
			audio_tracks =  g_udev_device_get_property_as_uint64 (device, "ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");

			/* check if we have a blank CD/DVD here */

			if (g_strcmp0 (media_state, "blank") != 0 && audio_tracks > 0) {
				gdk_threads_enter ();
				add_audio_cd(cwin);
				gdk_threads_leave ();
			}
		}
	}
}

gint
init_gudev_subsystem(struct con_win *cwin)
{
	GUdevClient *gudev_client;

	gudev_client = g_udev_client_new(gudev_subsystems);
	g_signal_connect(gudev_client, "uevent", G_CALLBACK(uevent_cb), cwin);

	return 0;
}
#endif
