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

static void pragha_mtp_action_send_to_device (GtkAction *action, PraghaDevices *devices);
static void pragha_mtp_action_append_songs   (GtkAction *action, PraghaDevices *devices);

static const GtkActionEntry mtp_sendto_actions [] = {
	{"Send to MTP", "multimedia-player", "Fake MTP device",
	 "", "Send to MTP", G_CALLBACK(pragha_mtp_action_send_to_device)},
};

static const gchar *mtp_sendto_xml = "<ui>					\
	<popup name=\"SelectionPopup\">		   				\
	<menu action=\"SendToMenu\">						\
		<placeholder name=\"pragha-sendto-placeholder\">			\
			<menuitem action=\"Send to MTP\"/>				\
			<separator/>						\
		</placeholder>							\
	</menu>									\
	</popup>				    				\
</ui>";

static const GtkActionEntry mtp_menu_actions [] = {
	{"MtpDevice", "multimedia-player", "Fake MTP device"},
	{"Add the library", GTK_STOCK_ADD, N_("_Add the library"),
	"", "Add all the library", G_CALLBACK(pragha_mtp_action_append_songs)},
};

static const gchar *mtp_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">						\
		<menu action=\"ToolsMenu\">					\
			<placeholder name=\"pragha-plugins-placeholder\">		\
				<menu action=\"MtpDevice\">				\
					<menuitem action=\"Add the library\"/>		\
				</menu>							\
				<separator/>						\
			</placeholder>						\
		</menu>								\
	</menubar>								\
</ui>";


LIBMTP_track_t *
get_mtp_track_from_musicobject (LIBMTP_mtpdevice_t *mtp_device, PraghaMusicobject *mobj)
{
	LIBMTP_track_t *tr;
	LIBMTP_filetype_t filetype;
	gchar *filename;
	struct stat sbuf;

	switch (pragha_musicobject_get_file_type(mobj)) {
		case FILE_WAV:
			filetype = LIBMTP_FILETYPE_WAV;
			break;
		case FILE_MP3:
			filetype = LIBMTP_FILETYPE_MP3;
			break;
		case FILE_FLAC:
			filetype = LIBMTP_FILETYPE_FLAC;
			break;
		case FILE_OGGVORBIS:
			filetype = LIBMTP_FILETYPE_OGG;
			break;
		case FILE_ASF:
			filetype = LIBMTP_FILETYPE_WMA;
			break;
		case FILE_MP4:
			filetype = LIBMTP_FILETYPE_MP4;
			break;
		case FILE_APE:
		case FILE_CDDA:
		case FILE_HTTP:
		case FILE_MTP:
		default:
			filetype = LIBMTP_FILETYPE_UNKNOWN;
			break;
	}

	if (filetype == LIBMTP_FILETYPE_UNKNOWN)
		return NULL;

	filename = g_strdup(pragha_musicobject_get_file(mobj));
	if (g_stat(filename, &sbuf) == -1) {
		g_free(filename);
		return NULL;
	}

	tr = LIBMTP_new_track_t();

	/* Minimun data. */

	tr->filesize = (uint64_t) sbuf.st_size;
	tr->filename = get_display_name(mobj);
	tr->filetype = filetype;

	/* Metadata. */

    tr->title = g_strdup(pragha_musicobject_get_title(mobj));
    tr->artist = g_strdup(pragha_musicobject_get_artist(mobj));
    tr->album = g_strdup(pragha_musicobject_get_album(mobj));
    tr->duration = (1000 * pragha_musicobject_get_length(mobj));
    tr->genre = g_strdup(pragha_musicobject_get_genre (mobj));
    tr->date = g_strdup_printf("%d", pragha_musicobject_get_year (mobj));

	/* Storage data. */

	tr->parent_id = mtp_device->default_music_folder;
	tr->storage_id = 0;

    g_free(filename);

    return tr;
}

static void
pragha_mtp_action_send_to_device (GtkAction *action, PraghaDevices *devices)
{
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_track_t *mtp_track;
	LIBMTP_error_t *stack;
	PraghaMusicobject *mobj = NULL;
	const gchar *file;
	gint ret;

	struct con_win *cwin = pragha_device_get_aplication (devices);

	mobj = pragha_playlist_get_selected_musicobject (cwin->cplaylist);
	if (!mobj)
		return;

	mtp_device = pragha_device_get_mtp_device(devices);

	file = pragha_musicobject_get_file (mobj);
	mtp_track = get_mtp_track_from_musicobject(mtp_device, mobj);

	ret = LIBMTP_Send_Track_From_File (mtp_device, file, mtp_track, NULL, NULL);
	LIBMTP_destroy_track_t(mtp_track);

	if (ret != 0) {
		stack = LIBMTP_Get_Errorstack (mtp_device);
		CDEBUG(DBG_INFO, "unable to send track: %s", stack->error_text);

		if (stack->errornumber == LIBMTP_ERROR_STORAGE_FULL) {
			CDEBUG(DBG_INFO, "No space left on MTP device");
		}
		else {
			CDEBUG(DBG_INFO, "Unable to send file to MTP device: %s", file);
		}

		LIBMTP_Dump_Errorstack(mtp_device);
		LIBMTP_Clear_Errorstack(mtp_device);
	}
	else {
		CDEBUG(DBG_INFO, "Added %s to MTP device", file);
	}
}
static void
pragha_mtp_action_append_songs (GtkAction *action, PraghaDevices *devices)
{
	pragha_device_cache_append_tracks (devices);
}

static void
pragha_playlist_append_mtp_action (PraghaDevices *devices)
{
	GtkActionGroup *action_group;
	GtkAction *action;
	gchar *friend_label = NULL;

	friend_label = LIBMTP_Get_Friendlyname (pragha_device_get_mtp_device(devices));

	/* Menubar tools. */

	action_group = gtk_action_group_new ("PraghaMenubarMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_menu_actions,
	                              G_N_ELEMENTS (mtp_menu_actions),
	                              devices);

	action = gtk_action_group_get_action (action_group, "MtpDevice");
	gtk_action_set_label(GTK_ACTION(action), friend_label);

	pragha_devices_append_menu_action (devices, action_group, mtp_menu_xml);

	/* Playlist sendto */

	action_group = gtk_action_group_new ("PraghaPlaylistMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_sendto_actions,
	                              G_N_ELEMENTS (mtp_sendto_actions),
	                              devices);

	action = gtk_action_group_get_action (action_group, "Send to MTP");

	friend_label = LIBMTP_Get_Friendlyname (pragha_device_get_mtp_device(devices));
	gtk_action_set_label(GTK_ACTION(action), friend_label);

	pragha_devices_append_playlist_action (devices, action_group, mtp_sendto_xml);

	g_free(friend_label);
}

static PraghaMusicobject *
new_musicobject_from_mtp (LIBMTP_track_t *track)
{
	PraghaMusicobject *mobj = NULL;
	gchar *uri = NULL;
	
	uri = g_strdup_printf ("mtp://%i-%s", track->item_id, track->filename);

	CDEBUG(DBG_MOBJ, "Creating new musicobject to MTP: %s", uri);

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

int progressfunc (uint64_t const sent, uint64_t const total, void const *const data)
{
	/* Have to give control to GTK periodically ... */
	if (pragha_process_gtk_events ())
		return -1;

	return 0;
}

void
pragha_device_mtp_append_tracks (PraghaDevices *devices)
{
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_track_t *tracks, *track, *tmp;
	PraghaMusicobject *mobj = NULL;

	if (pragha_device_already_is_idle (devices))
		return;

	mtp_device = pragha_device_get_mtp_device (devices);

	tracks = LIBMTP_Get_Tracklisting_With_Callback (mtp_device, NULL, NULL);
	if (tracks) {
		track = tracks;
		while (track != NULL) {
			mobj = new_musicobject_from_mtp (track);
			if (G_LIKELY(mobj))
				pragha_device_cache_insert_track (devices, mobj);

			tmp = track;
			track = track->next;
			LIBMTP_destroy_track_t(tmp);

			/* Have to give control to GTK periodically ... */
			if (pragha_process_gtk_events ())
				return;
		}
	}

	pragha_device_cache_append_tracks (devices);
}

void
pragha_devices_add_detected_device (PraghaDevices *devices)
{
	gint response;
	response = pragha_gudev_show_dialog (_("MTP Device"), "gnome-dev-cdrom-audio",
	                                     _("Was inserted an MTP Device"), NULL,
	                                     _("Append songs of device"), PRAGHA_DEVICE_RESPONSE_PLAY);
	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_device_mtp_append_tracks (devices);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}
}

void
pragha_devices_mtp_added (PraghaDevices *devices, GUdevDevice *device)
{
	LIBMTP_raw_device_t *device_list, *raw_device;
	LIBMTP_mtpdevice_t *mtp_device;
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

	/* Device handled. */

	mtp_device = LIBMTP_Open_Raw_Device(raw_device);

	pragha_gudev_set_hook_device (devices, device, mtp_device, busnum, devnum);

	pragha_playlist_append_mtp_action (devices);

	pragha_devices_add_detected_device (devices);

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
