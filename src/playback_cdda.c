/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

	struct musicobject *mobj;
	gint num_tracks = 0, i = 0;
	gint first_track;

	num_tracks = cdio_cddap_tracks(cwin->cstate->cdda_drive);
	if (!num_tracks)
		return;

	first_track = cdio_get_first_track_num(cwin->cstate->cdda_drive->p_cdio);
	clear_current_playlist(NULL, cwin);

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda(cwin, i);
		append_current_playlist(mobj, cwin);

		while(gtk_events_pending()) {
			if (gtk_main_iteration_do(FALSE))
				return;
		}
	}
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

static void update_gui(struct con_cdda_decoder *cdec, struct con_win *cwin)
{
	gint newsec;

	newsec = (cdec->cur - cdec->start) / CDIO_CD_FRAMES_PER_SEC;

	if ((newsec <= cwin->cstate->curr_mobj->tags->length) &&
	    (newsec != cdec->displayed_seconds)) {
		cwin->cstate->newsec = newsec;
		if (newsec > cdec->ltrack->play_duration)
			cdec->ltrack->play_duration = newsec;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
		cdec->displayed_seconds = newsec;
	}
}

static gint cdda_seek(struct con_cdda_decoder *cdec, struct con_win *cwin)
{
	gint seek_len;

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_len = cwin->cstate->seek_len;
	g_mutex_unlock(cwin->cstate->c_mutex);

	if (seek_len > cwin->cstate->curr_mobj->tags->length)
		return -1;

	cdec->cur = cdec->start + (seek_len * CDIO_CD_FRAMES_PER_SEC);
	update_gui(cdec, cwin);

	return 0;
}

static gint cdda_decode(struct con_cdda_decoder *cdec, struct con_win *cwin)
{
	gint ret = 0;
	enum thread_cmd cmd = 0;

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
			if (cdda_seek(cdec, cwin) < 0)
				g_critical("Unable to seek");
			break;
		default:
			break;
		}

		/* Reset command */

		if (cmd)
			cmd = reset_thread_command(cwin, cmd);
		if (cmd == CMD_PLAYBACK_STOP)
			goto exit;

		if (cdio_get_media_changed(cwin->cstate->cdda_drive->p_cdio)) {
			g_warning("CDDA ejected");
			cdio_cddap_close(cwin->cstate->cdda_drive);
			cwin->cstate->cdda_drive = NULL;
			if (cwin->cstate->cddb_disc) {
				cddb_disc_destroy(cwin->cstate->cddb_disc);
				cwin->cstate->cddb_disc = NULL;
			}
			if (cwin->cstate->cddb_conn) {
				cddb_destroy(cwin->cstate->cddb_conn);
				cwin->cstate->cddb_conn = NULL;
				libcddb_shutdown();
			}
			break;
		}

		if (cdec->cur >= cdec->end)
			break;

		/* Decode */

		memset(cdec->cdda_buf, 0, CDIO_CD_FRAMESIZE_RAW);
		ret = cdio_cddap_read(cwin->cstate->cdda_drive, cdec->cdda_buf,
				      cdec->cur, 1);

		if (ret < 0)
			break;

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)cdec->cdda_buf,
					  CDIO_CD_FRAMESIZE_RAW, cwin);

		ao_play(cwin->clibao->ao_dev, (gchar*)cdec->cdda_buf,
			CDIO_CD_FRAMESIZE_RAW);

		update_gui(cdec, cwin);
		cdec->cur++;

	} while (1);
exit:
	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_cdda(struct con_win *cwin)
{
	struct con_cdda_decoder cdec;
	gint ret = 0;
	gboolean lastfm_f = FALSE;

	if (!cwin->cstate->curr_mobj->file || !cwin->cstate->cdda_drive) {
		g_warning("CDDA not initialized");
		return;
	}

	/* Open audio device */

	if (open_audio_device(44100,
			      cwin->cstate->curr_mobj->tags->channels,
			      FALSE,
			      cwin) == -1) {
		g_warning("Unable to play file: %s",
			  cwin->cstate->curr_mobj->file);
		goto exit;
	}

	CDEBUG(DBG_INFO, "Playing : %s", cwin->cstate->curr_mobj->file);

	memset(&cdec, sizeof(struct con_cdda_decoder), 0);
	cdec.start = cdio_cddap_track_firstsector(cwin->cstate->cdda_drive,
					  cwin->cstate->curr_mobj->tags->track_no);
	cdec.end = cdio_cddap_track_lastsector(cwin->cstate->cdda_drive,
				       cwin->cstate->curr_mobj->tags->track_no);
	cdec.cur = cdec.start;
	lastfm_track_reset(cwin, &cdec.ltrack);
	lastfm_f = TRUE;

	ret = cdda_decode(&cdec, cwin);
exit:
	playback_end_cleanup(cwin, cdec.ltrack, ret, lastfm_f);
}

void play_audio_cd(struct con_win *cwin)
{
	gint disc_len = 0;
	lba_t lba;

	/* Stop playback first */
	stop_playback(cwin);

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

		disc_len = lba / CDIO_CD_FRAMES_PER_SEC;
		cddb_disc_set_length(cwin->cstate->cddb_disc, disc_len);
		if (cddb_add_tracks(cwin) < 0)
			goto cddb_clean;

		if (!cddb_disc_calc_discid(cwin->cstate->cddb_disc))
			goto cddb_clean;

		cddb_disc_set_category(cwin->cstate->cddb_disc, CDDB_CAT_MISC);

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
	CDEBUG(DBG_INFO, "Succesfully opened Audio CD device");

	play_first_current_playlist(cwin);
}
