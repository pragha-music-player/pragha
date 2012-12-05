/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2011 matias <mati86dl@gmail.com>			 */
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

#include "pragha.h"

static gint cddb_add_tracks(struct con_win *cwin)
{
	cddb_track_t *track;
	lba_t lba;
	gint num_tracks, first_track, i = 0;

	num_tracks = cdio_cddap_tracks(cwin->cstate->cdda_drive);
	if (!num_tracks)
		return -1;

	first_track = cdio_get_first_track_num(cwin->cstate->cdda_drive->p_cdio);
	for (i = first_track; i <= num_tracks; i++) {
		track = cddb_track_new();
		if (!track)
			return -1;

		lba = cdio_get_track_lba(cwin->cstate->cdda_drive->p_cdio, i);
		if (lba == CDIO_INVALID_LBA)
			return -1;

		cddb_disc_add_track(cwin->cstate->cddb_disc, track);
		cddb_track_set_frame_offset(track, lba);
	}

	return 0;
}

static void add_audio_cd_tracks(struct con_win *cwin)
{
	PraghaMusicobject *mobj;
	gint num_tracks = 0, i = 0, prev_tracks = 0;

	num_tracks = cdio_cddap_tracks(cwin->cstate->cdda_drive);
	if (!num_tracks)
		return;

	prev_tracks = pragha_playlist_get_no_tracks(cwin->cplaylist);

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda(cwin, i);
		append_current_playlist(cwin->cplaylist, NULL, mobj);

		if (pragha_process_gtk_events ())
			return;
	}
	update_status_bar_playtime(cwin);
	select_numered_path_of_current_playlist(cwin->cplaylist, prev_tracks, TRUE);
}

static cdrom_drive_t* find_audio_cd(struct con_win *cwin)
{
	cdrom_drive_t *drive = NULL;
	gchar **cdda_devices = NULL;

	if (!cwin->cpref->audio_cd_device) {
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
		       cwin->cpref->audio_cd_device);

		drive = cdio_cddap_identify(cwin->cpref->audio_cd_device,
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

	/* Clean earlier CDDA state */
	if (cwin->cstate->cdda_drive) {
		cdio_cddap_close(cwin->cstate->cdda_drive);
		cwin->cstate->cdda_drive = NULL;
	}

	cwin->cstate->cdda_drive = find_audio_cd(cwin);
	if (!cwin->cstate->cdda_drive)
		return;

	if (cdio_cddap_open(cwin->cstate->cdda_drive)) {
		g_warning("Unable to open Audio CD");
		return;
	}

	if (cwin->cpref->use_cddb) {
		/* Clean earlier CDDB state */
		if (cwin->cstate->cddb_disc) {
			cddb_disc_destroy(cwin->cstate->cddb_disc);
			cwin->cstate->cddb_disc = NULL;
		}

		if (!cwin->cstate->cddb_conn) {
			cwin->cstate->cddb_conn = cddb_new();
			if (!cwin->cstate->cddb_conn)
				goto cddb_clean;
		}

		cwin->cstate->cddb_disc = cddb_disc_new();
		if (!cwin->cstate->cddb_disc)
			goto cddb_clean;

		lba = cdio_get_track_lba(cwin->cstate->cdda_drive->p_cdio,
					 CDIO_CDROM_LEADOUT_TRACK);
		if (lba == CDIO_INVALID_LBA)
			goto cddb_clean;

		cddb_disc_set_length(cwin->cstate->cddb_disc, FRAMES_TO_SECONDS(lba));
		if (cddb_add_tracks(cwin) < 0)
			goto cddb_clean;

		if (!cddb_disc_calc_discid(cwin->cstate->cddb_disc))
			goto cddb_clean;

		cddb_disc_set_category(cwin->cstate->cddb_disc, CDDB_CAT_MISC);

		matches = cddb_query(cwin->cstate->cddb_conn, cwin->cstate->cddb_disc);
		if (matches == -1)
			goto cddb_clean;

		if (!cddb_read(cwin->cstate->cddb_conn,
			       cwin->cstate->cddb_disc)) {
			cddb_error_print(cddb_errno(cwin->cstate->cddb_conn));
			goto cddb_clean;
		}

		CDEBUG(DBG_INFO, "Successfully initialized CDDB");
		goto add;
	}

cddb_clean:
	if (cwin->cstate->cddb_disc) {
		cddb_disc_destroy(cwin->cstate->cddb_disc);
		cwin->cstate->cddb_disc = NULL;
	}
	if (cwin->cstate->cddb_conn) {
		cddb_destroy(cwin->cstate->cddb_conn);
		cwin->cstate->cddb_conn = NULL;
	}
add:
	add_audio_cd_tracks(cwin);
	CDEBUG(DBG_INFO, "Successfully opened Audio CD device");
}
