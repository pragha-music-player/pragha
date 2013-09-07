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

#include <gudev/gudev.h>
#include <libmtp.h>
#include <stdlib.h>

#include "pragha.h"
#include "pragha-devices.h"
#include "pragha-devices-mtp.h"
#include "pragha-devices-block.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-debug.h"

static PraghaMusicobject *
new_musicobject_from_mtp (LIBMTP_track_t *track)
{
	PraghaMusicobject *mobj = NULL;
	gchar *uri = NULL;
	
	CDEBUG(DBG_MOBJ, "Creating new musicobject to MTP: %s", uri);

	uri = g_strdup_printf ("mtp://%i-%s", track->item_id, track->filename);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", uri,
	                     "file-type", FILE_MTP,
	                     NULL);
	if (track->title)
		pragha_musicobject_set_title (mobj, track->title);
	if (track->artist)
		pragha_musicobject_set_artist (mobj, track->artist);
	if (track->album)
		pragha_musicobject_set_album (mobj, track->album);
	if (track->genre)
		pragha_musicobject_set_genre (mobj, track->genre);
	if (track->duration)
		pragha_musicobject_set_length (mobj, track->duration/1000);
	if (track->tracknumber)
		pragha_musicobject_set_track_no (mobj, track->tracknumber);
	if (track->samplerate)
		pragha_musicobject_set_samplerate (mobj, track->samplerate);
	if (track->nochannels)
		pragha_musicobject_set_channels (mobj, track->nochannels);

	g_free(uri);

	return mobj;
}

void
pragha_devices_mtp_added (PraghaDevices *devices, GUdevDevice *device)
{
	LIBMTP_raw_device_t *device_list, *raw_device;
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_track_t *tracks, *track, *tmp;
	PraghaMusicobject *mobj = NULL;
	guint64 busnum = 0;
	guint64 devnum = 0;
	gint numdevs = 0;

	if (pragha_device_already_is_busy (devices))
		return;

	busnum = g_udev_device_get_property_as_uint64(device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64(device, "DEVNUM");

	/* Get devices.. */

	if (LIBMTP_Detect_Raw_Devices (&device_list, &numdevs) != LIBMTP_ERROR_NONE)
		return;

	if (device_list == NULL || numdevs == 0)
		return;

	raw_device = &device_list[0];

	if (raw_device->devnum != devnum && raw_device->bus_location != busnum)
		goto bad;

	/* Get mtp_device and load track list.. */

	mtp_device = LIBMTP_Open_Raw_Device(raw_device);
	tracks = LIBMTP_Get_Tracklisting_With_Callback (mtp_device, NULL, NULL);
	if (tracks) {
		track = tracks;
		while (track != NULL) {
			mobj = new_musicobject_from_mtp (track);
			pragha_playlist_append_single_song (pragha_device_get_aplication(devices)->cplaylist, mobj);

			tmp = track;
			track = track->next;
			LIBMTP_destroy_track_t(tmp);
		}
	}

	/* Device handled. */

	pragha_gudev_set_hook_device (devices, device, mtp_device, busnum, devnum);

	CDEBUG(DBG_INFO, "Hook a new MTP device, Bus: %ld, Dev: %ld", busnum, devnum);

bad:
	g_free(raw_device);
}

void
pragha_devices_prepare_mtp_temp_file (PraghaBackend *backend, gpointer user_data)
{
	PraghaMusicobject *mobj;
	const gchar *track_id;
	const gchar *file;
	gchar *tmp_uri = NULL;

	PraghaDevices *devices = user_data;

	if (pragha_device_already_is_idle (devices))
		return;

	mobj = pragha_backend_get_musicobject (backend);

	file = pragha_musicobject_get_file (mobj);

	if (g_str_has_prefix (file, "mtp://") == FALSE)
		return;

	track_id = file + strlen ("mtp://");

	tmp_uri = g_strdup_printf ("/tmp/%s", track_id);
	if (!LIBMTP_Get_Track_To_File (pragha_device_get_mtp_device(devices), atoi(track_id), tmp_uri, NULL, NULL)) {
		gchar *uri = g_filename_to_uri (tmp_uri, NULL, NULL);
		pragha_backend_set_tmp_uri (backend, uri);
		g_free(uri);
	}
}
