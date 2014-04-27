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

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "src/pragha-utils.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-debug.h"

#include "pragha-mtp-musicobject.h"

LIBMTP_track_t *
mtp_track_new_from_pragha_musicobject (LIBMTP_mtpdevice_t *mtp_device, PraghaMusicobject *mobj)
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

PraghaMusicobject *
pragha_musicobject_new_from_mtp_track (LIBMTP_track_t *track)
{
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicobject *mobj = NULL;
	gchar *uri = NULL;
	
	uri = g_strdup_printf ("mtp://%i-%s", track->item_id, track->filename);

	CDEBUG(DBG_MOBJ, "Creating new musicobject to MTP: %s", uri);

	enum_map = pragha_music_enum_get();
	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", uri,
	                     "file-type", pragha_music_enum_map_get(enum_map, "FILE_MTP"),
	                     NULL);
	g_object_unref (enum_map);

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

gint
pragha_mtp_plugin_get_track_id (PraghaMusicobject *mobj)
{
	const gchar *track_id, *file;

	file = pragha_musicobject_get_file (mobj);
	track_id = file + strlen ("mtp://");

	return atoi(track_id);
}

gchar *
pragha_mtp_plugin_get_temp_filename (PraghaMusicobject *mobj)
{
	const gchar *track_id, *file;

	file = pragha_musicobject_get_file (mobj);
	track_id = file + strlen ("mtp://");

	return g_strdup_printf ("/tmp/%s", track_id);
}

gboolean
pragha_musicobject_is_mtp_file (PraghaMusicobject *mobj)
{
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicType file_type = FILE_NONE;

	enum_map = pragha_music_enum_get ();
	file_type = pragha_music_enum_map_get(enum_map, "FILE_MTP");
	g_object_unref (enum_map);

	return (file_type == pragha_musicobject_get_file_type (mobj));
}
