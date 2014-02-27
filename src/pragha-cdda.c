/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
/* 									 */
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

#include "pragha-cdda.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha.h"

#if HAVE_LIBCDIO && HAVE_LIBCDIO_PARANOIA && HAVE_LIBCDDB

PraghaMusicobject *
new_musicobject_from_cdda(PraghaApplication *pragha,
                          cdrom_drive_t *cdda_drive,
                          cddb_disc_t *cddb_disc,
                          gint track_no)
{
	PraghaPreferences *preferences;
	PraghaMusicobject *mobj = NULL;
	gint channels, start, end;
	gchar *ntitle = NULL, *nfile = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from cdda: %d",
	       track_no);

	channels = cdio_get_track_channels(cdda_drive->p_cdio,
					   track_no);
	start = cdio_cddap_track_firstsector(cdda_drive, track_no);
	end = cdio_cddap_track_lastsector(cdda_drive, track_no);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT, NULL);

	preferences = pragha_application_get_preferences (pragha);
	if (pragha_preferences_get_use_cddb (preferences) && cddb_disc) {
		cddb_track_t *track;
		const gchar *title, *artist, *album, *genre;
		gint year;

		track = cddb_disc_get_track(cddb_disc, track_no - 1);
		if (track) {
			title = cddb_track_get_title(track);
			if (title)
				ntitle = g_strdup(title);

			artist = cddb_track_get_artist(track);
			if(artist)
				pragha_musicobject_set_artist(mobj, artist);

			album = cddb_disc_get_title(cddb_disc);
			if(album)
				pragha_musicobject_set_album(mobj, album);

			year = cddb_disc_get_year(cddb_disc);
			if(year)
				pragha_musicobject_set_year(mobj, year);

			genre = cddb_disc_get_genre(cddb_disc);
			if(genre)
				pragha_musicobject_set_genre(mobj, genre);
		}
	}

	nfile = g_strdup_printf("cdda://%d", track_no);
	pragha_musicobject_set_file(mobj, nfile);
	pragha_musicobject_set_file_type(mobj, FILE_CDDA);

	pragha_musicobject_set_track_no(mobj, track_no);

	if (!ntitle)
		ntitle = g_strdup_printf("Track %d", track_no);
	pragha_musicobject_set_title(mobj, ntitle);

	pragha_musicobject_set_length(mobj, (end - start) / CDIO_CD_FRAMES_PER_SEC);
	pragha_musicobject_set_channels(mobj, (channels > 0) ? channels : 0);

	g_free(nfile);
	g_free(ntitle);

	return mobj;
}

static gint cddb_add_tracks(cdrom_drive_t *cdda_drive, cddb_disc_t *cddb_disc)
{
	cddb_track_t *track;
	lba_t lba;
	gint num_tracks, first_track, i = 0;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return -1;

	first_track = cdio_get_first_track_num(cdda_drive->p_cdio);
	for (i = first_track; i <= num_tracks; i++) {
		track = cddb_track_new();
		if (!track)
			return -1;

		lba = cdio_get_track_lba(cdda_drive->p_cdio, i);
		if (lba == CDIO_INVALID_LBA)
			return -1;

		cddb_disc_add_track(cddb_disc, track);
		cddb_track_set_frame_offset(track, lba);
	}

	return 0;
}

static void add_audio_cd_tracks(PraghaApplication *pragha, cdrom_drive_t *cdda_drive, cddb_disc_t *cddb_disc)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj;

	gint num_tracks = 0, i = 0;
	GList *list = NULL;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return;

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda(pragha, cdda_drive, cddb_disc, i);
		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);

		pragha_process_gtk_events ();
	}
	if (list) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list(playlist, list);
		g_list_free (list);
	}
}

static cdrom_drive_t* find_audio_cd(PraghaApplication *pragha)
{
	cdrom_drive_t *drive = NULL;
	gchar **cdda_devices = NULL;
	PraghaPreferences *preferences;

	preferences = pragha_application_get_preferences (pragha);
	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device(preferences);

	if (!audio_cd_device) {
		cdda_devices = cdio_get_devices_with_cap(NULL, CDIO_FS_AUDIO,
							 FALSE);
		if (!cdda_devices || (cdda_devices && !*cdda_devices)) {
			g_warning("No Audio CD found");
			return NULL;
		}

		CDEBUG(DBG_INFO, "Trying Audio CD Device: %s", *cdda_devices);

		drive = cdio_cddap_identify(*cdda_devices, 0, NULL);
		if (!drive) {
			g_warning("Unable to identify Audio CD");
			goto exit;
		}
	} else {
		CDEBUG(DBG_INFO, "Trying Audio CD Device: %s",
		       audio_cd_device);

		drive = cdio_cddap_identify(audio_cd_device,
					    0, NULL);
		if (!drive) {
			g_warning("Unable to identify Audio CD");
			return NULL;
		}
	}
exit:
	if (cdda_devices)
		cdio_free_device_list(cdda_devices);

	return drive;
}

void pragha_application_append_audio_cd (PraghaApplication *pragha)
{
	lba_t lba;
	gint matches;
	cddb_disc_t *cddb_disc = NULL;
	cddb_conn_t *cddb_conn = NULL;
	PraghaPreferences *preferences;

	cdrom_drive_t *cdda_drive = find_audio_cd(pragha);
	if (!cdda_drive)
		return;

	if (cdio_cddap_open(cdda_drive)) {
		g_warning("Unable to open Audio CD");
		return;
	}

	preferences = pragha_application_get_preferences (pragha);
	if (pragha_preferences_get_use_cddb (preferences)) {
		cddb_conn = cddb_new ();
		if (!cddb_conn)
			goto add;

		cddb_disc = cddb_disc_new();
		if (!cddb_disc)
			goto add;

		lba = cdio_get_track_lba(cdda_drive->p_cdio,
					 CDIO_CDROM_LEADOUT_TRACK);
		if (lba == CDIO_INVALID_LBA)
			goto add;

		cddb_disc_set_length(cddb_disc, FRAMES_TO_SECONDS(lba));
		if (cddb_add_tracks(cdda_drive, cddb_disc) < 0)
			goto add;

		if (!cddb_disc_calc_discid(cddb_disc))
			goto add;

		cddb_disc_set_category(cddb_disc, CDDB_CAT_MISC);

		matches = cddb_query(cddb_conn, cddb_disc);
		if (matches == -1)
			goto add;

		if (!cddb_read(cddb_conn,
			       cddb_disc)) {
			cddb_error_print(cddb_errno(cddb_conn));
			goto add;
		}

		CDEBUG(DBG_INFO, "Successfully initialized CDDB");
		goto add;
	}

add:
	add_audio_cd_tracks(pragha, cdda_drive, cddb_disc);
	CDEBUG(DBG_INFO, "Successfully opened Audio CD device");

	if (cdda_drive)
		cdio_cddap_close(cdda_drive);
	if (cddb_disc)
		cddb_disc_destroy(cddb_disc);
	if (cddb_conn)
		cddb_destroy(cddb_conn);
}

void
pragha_cdda_free ()
{
	libcddb_shutdown ();
}
#endif

