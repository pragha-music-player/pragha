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

#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"
#include "pragha.h"

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

static void add_audio_cd_tracks(struct con_win *cwin, cdrom_drive_t *cdda_drive, cddb_disc_t *cddb_disc)
{
	PraghaMusicobject *mobj;
	gint num_tracks = 0, i = 0;
	GList *list = NULL;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return;

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda(cwin, cdda_drive, cddb_disc, i);
		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);

		if (pragha_process_gtk_events ())
			return;
	}

	pragha_playlist_append_mobj_list(cwin->cplaylist, list);
	g_list_free(list);
}

static cdrom_drive_t* find_audio_cd(struct con_win *cwin)
{
	cdrom_drive_t *drive = NULL;
	gchar **cdda_devices = NULL;
	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device(cwin->preferences);

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

void add_audio_cd(struct con_win *cwin)
{
	lba_t lba;
	gint matches;
	cddb_disc_t *cddb_disc = NULL;
	cddb_conn_t *cddb_conn = NULL;

	cdrom_drive_t *cdda_drive = find_audio_cd(cwin);
	if (!cdda_drive)
		return;

	if (cdio_cddap_open(cdda_drive)) {
		g_warning("Unable to open Audio CD");
		return;
	}

	if (pragha_preferences_get_use_cddb(cwin->preferences)) {
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
	add_audio_cd_tracks(cwin, cdda_drive, cddb_disc);
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
