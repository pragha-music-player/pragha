/*************************************************************************/
/* Copyright (C) 2012-2014 matias <mati86dl@gmail.com>                   */
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

#include <glib/gstdio.h>
#include <gudev/gudev.h>
#include <libmtp.h>
#include <stdlib.h>

#include "src/pragha-file-utils.h"
#include "src/pragha-utils.h"
#include "src/pragha-simple-widgets.h"
#include "src/pragha-hig.h"
#include "src/pragha-debug.h"
#include "src/pragha.h"

#include "pragha-devices-plugin.h"
#include "pragha-devices-mtp.h"
#include "pragha-devices-block.h"

static void pragha_mtp_action_send_to_device   (GtkAction *action, PraghaDevicesPlugin *plugin);
static void pragha_mtp_action_append_songs     (GtkAction *action, PraghaDevicesPlugin *plugin);
static void pragha_mtp_action_show_device_info (GtkAction *action, PraghaDevicesPlugin *plugin);

static PraghaMusicobject *new_musicobject_from_mtp (LIBMTP_track_t *track);

static const GtkActionEntry mtp_sendto_actions [] = {
	{"Send to MTP", "multimedia-player", "Fake MTP device",
	 "", "Send to MTP", G_CALLBACK(pragha_mtp_action_send_to_device)},
};

static const gchar *mtp_sendto_xml = "<ui>					\
	<popup name=\"SelectionPopup\">						\
	<menu action=\"SendToMenu\">						\
		<placeholder name=\"pragha-sendto-placeholder\">		\
			<menuitem action=\"Send to MTP\"/>			\
			<separator/>						\
		</placeholder>							\
	</menu>									\
	</popup>								\
</ui>";

static const GtkActionEntry mtp_menu_actions [] = {
	{"MtpDevice", "multimedia-player", "Fake MTP device"},
	{"Add MTP library", "list-add", N_("_Add the library"),
	"", "Add all the library", G_CALLBACK(pragha_mtp_action_append_songs)},
	{"Show device info", "dialog-information", N_("Show device info"),
	"", "Show device info", G_CALLBACK(pragha_mtp_action_show_device_info)},
};

static const gchar *mtp_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">						\
		<menu action=\"ToolsMenu\">					\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menu action=\"MtpDevice\">			\
					<menuitem action=\"Add MTP library\"/>	\
					<separator/>				\
					<menuitem action=\"Show device info\"/>	\
				</menu>						\
				<separator/>					\
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
pragha_mtp_action_send_to_device (GtkAction *action, PraghaDevicesPlugin *plugin)
{
	PraghaApplication *pragha = NULL;
	PraghaPlaylist *playlist;
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_track_t *mtp_track;
	LIBMTP_error_t *stack;
	PraghaMusicobject *mobj = NULL;
	const gchar *file;
	gint ret;

	pragha = pragha_device_get_application (plugin);

	playlist = pragha_application_get_playlist (pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	if (!mobj)
		return;

	mtp_device = pragha_device_get_mtp_device (plugin);

	file = pragha_musicobject_get_file (mobj);
	mtp_track = get_mtp_track_from_musicobject(mtp_device, mobj);

	ret = LIBMTP_Send_Track_From_File (mtp_device, file, mtp_track, NULL, NULL);

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
		mobj = new_musicobject_from_mtp (mtp_track);
		if (G_LIKELY(mobj))
			pragha_device_cache_insert_track (plugin, mobj);

		CDEBUG(DBG_INFO, "Added %s to MTP device", file);
	}

	LIBMTP_destroy_track_t(mtp_track);
}
static void
pragha_mtp_action_append_songs (GtkAction *action, PraghaDevicesPlugin *plugin)
{
	pragha_device_cache_append_tracks (plugin);
}

static void
pragha_mtp_action_show_device_info (GtkAction *action, PraghaDevicesPlugin *plugin)
{
	PraghaApplication *pragha = NULL;
	GtkWidget *dialog, *header, *table, *label;
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_devicestorage_t *storage;
	gchar *friend_label = NULL;
	gchar *storage_size = NULL;
	gchar *storage_free = NULL;
	gchar *storage_string = NULL;
	guint row = 0;

	mtp_device = pragha_device_get_mtp_device (plugin);
	pragha = pragha_device_get_application (plugin);

	friend_label = LIBMTP_Get_Friendlyname (mtp_device);

	dialog = gtk_dialog_new_with_buttons (friend_label,
	                                      GTK_WINDOW(pragha_application_get_window (pragha)),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Ok"), GTK_RESPONSE_OK,
	                                      NULL);

	header = sokoke_xfce_header_new (friend_label, "multimedia-player");

	table = pragha_hig_workarea_table_new ();

	LIBMTP_Get_Storage (mtp_device, LIBMTP_STORAGE_SORTBY_FREESPACE);
	for (storage = mtp_device->storage; storage != 0; storage = storage->next) {
		pragha_hig_workarea_table_add_section_title (table, &row, storage->StorageDescription);

		storage_free = g_format_size (storage->FreeSpaceInBytes);
		storage_size = g_format_size (storage->MaxCapacity);

		storage_string = g_strdup_printf (_("%s free of %s (%d%% used)"),
		                                  storage_free, storage_size,
		                                  (gint) ((storage->MaxCapacity - storage->FreeSpaceInBytes) * 100 / storage->MaxCapacity));

		label = gtk_label_new_with_mnemonic (storage_string);

		pragha_hig_workarea_table_add_wide_control (table, &row, label);

		g_free (storage_free);
		g_free (storage_size);
		g_free (storage_string);
	}

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), table, TRUE, TRUE, 0);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show_all (dialog);

	g_free (friend_label);
}

static void
pragha_playlist_append_mtp_action (PraghaDevicesPlugin *plugin)
{
	GtkActionGroup *action_group;
	GtkAction *action;
	gchar *friend_label = NULL;

	friend_label = LIBMTP_Get_Friendlyname (pragha_device_get_mtp_device(plugin));

	/* Menubar tools. */

	action_group = gtk_action_group_new ("PraghaMenubarMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_menu_actions,
	                              G_N_ELEMENTS (mtp_menu_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "MtpDevice");
	gtk_action_set_label(GTK_ACTION(action), friend_label);

	pragha_devices_plugin_append_menu_action (plugin, action_group, mtp_menu_xml);

	/* Playlist sendto */

	action_group = gtk_action_group_new ("PraghaPlaylistMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_sendto_actions,
	                              G_N_ELEMENTS (mtp_sendto_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "Send to MTP");

	friend_label = LIBMTP_Get_Friendlyname (pragha_device_get_mtp_device(plugin));
	gtk_action_set_label(GTK_ACTION(action), friend_label);

	pragha_devices_append_playlist_action (plugin, action_group, mtp_sendto_xml);

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
	                     "file-type", FILE_DEVICE_MTP,
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
	pragha_process_gtk_events ();

	return 0;
}

void
pragha_device_mtp_cache_tracks (PraghaDevicesPlugin *plugin)
{
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_track_t *tracks, *track, *tmp;
	PraghaMusicobject *mobj = NULL;

	mtp_device = pragha_device_get_mtp_device (plugin);
	tracks = LIBMTP_Get_Tracklisting_With_Callback (mtp_device, progressfunc, NULL); // Slow!.
	if (tracks) {
		track = tracks;
		while (track != NULL) {
			mobj = new_musicobject_from_mtp (track);
			if (G_LIKELY(mobj))
				pragha_device_cache_insert_track (plugin, mobj);

			tmp = track;
			track = track->next;
			LIBMTP_destroy_track_t(tmp);

			/* Have to give control to GTK periodically ... */
			pragha_process_gtk_events ();
		}
	}
}

void
pragha_device_mtp_append_tracks (PraghaDevicesPlugin *plugin)
{
	if (pragha_device_already_is_idle (plugin))
		return;

	pragha_device_cache_append_tracks (plugin);
}

void
pragha_devices_add_detected_device (PraghaDevicesPlugin *plugin)
{
	gint response;
	response = pragha_gudev_show_dialog (NULL, _("MTP Device"), "multimedia-player",
	                                     _("Was inserted an MTP Device"), NULL,
	                                     _("Append songs of device"), PRAGHA_DEVICE_RESPONSE_PLAY);
	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_device_mtp_append_tracks (plugin);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}
}

void
pragha_devices_mtp_added (PraghaDevicesPlugin *plugin, GUdevDevice *device)
{
	LIBMTP_raw_device_t *device_list, *raw_device;
	LIBMTP_mtpdevice_t *mtp_device;
	guint64 busnum = 0;
	guint64 devnum = 0;
	gint numdevs = 0;

	if (pragha_device_already_is_busy (plugin))
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

	/* Get device and reorder by free space. */

	mtp_device = LIBMTP_Open_Raw_Device(raw_device); // Slow!.
	LIBMTP_Get_Storage (mtp_device, LIBMTP_STORAGE_SORTBY_FREESPACE);

	/* Device handled. */

	pragha_gudev_set_hook_device (plugin, PRAGHA_DEVICE_MTP, device, mtp_device, busnum, devnum);

	/* Cache song of device. */
	pragha_device_mtp_cache_tracks (plugin); // Slow!.

	/* Add action to menubar and playlist. */
	pragha_playlist_append_mtp_action (plugin);

	/* Show dialog to append songs. */
	pragha_devices_add_detected_device (plugin);

	CDEBUG(DBG_INFO, "Hook a new MTP device, Bus: %ld, Dev: %ld", busnum, devnum);

bad:
	g_free(raw_device);
}
