/*
 * Copyright (C) 2013 Pavel Vasin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pragha.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"

#include <cddb/cddb.h>

#if HAVE_GSTREAMER_PBUTILS

#include <gst/audio/gstaudiocdsrc.h>
#include <gst/tag/tag.h>
#include <gst/pbutils/pbutils.h>

static PraghaMusicobject *
build_musicobject (const GstTocEntry *entry)
{
	PraghaMusicobject *mobj = NULL;
	const GstTagList *tags = gst_toc_entry_get_tags (entry);

	gchar *uri, *title;
	guint track_number = 0;
	guint64 duration = 0;

	gst_tag_list_get_uint (tags, GST_TAG_TRACK_NUMBER, &track_number);
	gst_tag_list_get_uint64 (tags, GST_TAG_DURATION, &duration);
	uri = g_strdup_printf ("cdda://%d", track_number);
	title = g_strdup_printf ("Track %d", track_number);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", uri,
	                     "file-type", FILE_CDDA,
	                     "track-no", track_number,
	                     "length", GST_TIME_AS_SECONDS (duration),
	                     "title", title,
	                     NULL);

	g_free (uri);
	g_free (title);

	return mobj;
}

static void
append_cddb_data (PraghaMusicobject *mobj, cddb_disc_t *disc)
{
	guint year;
	const gchar *title, *artist, *album, *genre;

	guint track_no = pragha_musicobject_get_track_no (mobj);
	cddb_track_t *track = cddb_disc_get_track (disc, track_no - 1);

	if (!track)
		return;

	title = cddb_track_get_title (track);
	if (title)
		pragha_musicobject_set_title (mobj, title);

	artist = cddb_track_get_artist (track);
	if (artist)
		pragha_musicobject_set_artist (mobj, artist);

	album = cddb_disc_get_title (disc);
	if (album)
		pragha_musicobject_set_album (mobj, album);

	year = cddb_disc_get_year (disc);
	if (year)
		pragha_musicobject_set_year (mobj, year);

	genre = cddb_disc_get_genre (disc);
	if (genre)
		pragha_musicobject_set_genre (mobj, genre);
}

static guint
get_disc_id (GstDiscovererInfo *info)
{
	guint disc_id = 0;

	const GstTagList *tags = gst_discoverer_info_get_tags (info);

	gchar *disc_id_string = NULL;
	gst_tag_list_get_string (tags, GST_TAG_CDDA_CDDB_DISCID, &disc_id_string);

	if (disc_id_string) {
		sscanf (disc_id_string, "%x", &disc_id);
		g_free (disc_id_string);
	}

	return disc_id;
}

static cddb_disc_t *
get_cddb_disc (GstDiscovererInfo *info)
{
	guint disc_id = get_disc_id (info);

	cddb_conn_t *conn = cddb_new ();

	cddb_disc_t *disc = cddb_disc_new ();
	cddb_disc_set_category (disc, CDDB_CAT_MISC);
	cddb_disc_set_discid (disc, disc_id);

	int success = cddb_read (conn, disc);
	if (!success) {
		cddb_error_print (cddb_errno (conn));
		cddb_disc_destroy (disc);
		disc = NULL;
	}

	cddb_destroy (conn);

	return disc;
}

static void
source_setup_cb (GstDiscoverer *discoverer, GstElement *source, gpointer user_data)
{
	g_return_if_fail (GST_IS_AUDIO_CD_SRC (source));

	PraghaPreferences *preferences = user_data;
	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device (preferences);

	if (string_is_not_empty (audio_cd_device)) {
		g_object_set (source, "device", audio_cd_device, NULL);
	}
}

void
pragha_cdda_add (struct con_win *cwin)
{
	GList *i, *tracks = NULL;
	GError *error = NULL;
	GstDiscoverer *discoverer = NULL;
	GstDiscovererInfo *info = NULL;
	const GstClockTime timeout = 10 * GST_SECOND;

	discoverer = gst_discoverer_new (timeout, &error);

	if (error)
		goto exit;

	g_signal_connect (discoverer, "source-setup", G_CALLBACK (source_setup_cb), cwin->preferences);

	info = gst_discoverer_discover_uri (discoverer, "cdda://", &error);

	if (error)
		goto exit;

	const GstToc *toc = gst_discoverer_info_get_toc (info);
	GList *entries = gst_toc_get_entries (toc);

	for (i = entries; i; i = i->next) {
		PraghaMusicobject *mobj = build_musicobject (i->data);
		tracks = g_list_prepend (tracks, mobj);
	}

	tracks = g_list_reverse (tracks);

	if (pragha_preferences_get_use_cddb (cwin->preferences)) {
		cddb_disc_t *disc = get_cddb_disc (info);
		if (disc) {
			g_list_foreach (tracks, (GFunc)append_cddb_data, disc);
			cddb_disc_destroy (disc);
		}
	}

	pragha_playlist_append_mobj_list (cwin->cplaylist, tracks);

exit:
	if (error) {
		g_warning ("cdda error: %s", error->message);
		g_error_free (error);
	}
	if (info)
		g_object_unref (info);
	if (discoverer)
		g_object_unref (discoverer);
	g_list_free (tracks);
}

void
pragha_cdda_free ()
{
	libcddb_shutdown ();
}

#endif /* HAVE_GSTREAMER_PBUTILS */
