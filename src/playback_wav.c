/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

static void update_gui(gint played,
		       struct con_wav_decoder *wdec,
		       struct con_win *cwin)
{
	wdec->frames_played += played;

	if (wdec->frame_size == 0)
		return;

	if (wdec->frames_played / wdec->frame_size == wdec->seconds_played)
		return;

	cwin->cstate->newsec = wdec->frames_played / wdec->frame_size;
	if (cwin->cstate->newsec > wdec->ltrack->play_duration)
		wdec->ltrack->play_duration = cwin->cstate->newsec;
	wdec->seconds_played = wdec->frames_played / wdec->frame_size;
	g_idle_add(update_track_progress_bar, cwin);
	g_idle_add(update_current_song_info, cwin);
}

static gint sndfile_seek(struct con_wav_decoder *wdec, struct con_win *cwin)
{
	gint seek_len = 0;
	sf_count_t seek_frame = 0;

	if (wdec->sinfo.seekable == 0) {
		g_warning("File is not seekable");
		return -1;
	}

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_len = cwin->cstate->seek_len;
	g_mutex_unlock(cwin->cstate->c_mutex);

	seek_frame = wdec->frame_size * seek_len;

	if (wdec->sinfo.channels == 2)
		seek_frame /= 2;

	if (sf_seek(wdec->sfile, seek_frame, SEEK_SET) == -1) {
		g_warning("Inavlid seek len: %d", seek_len);
		return -1;
	}

	if (wdec->sinfo.channels == 2)
		seek_frame *= 2;

	update_gui((seek_frame - wdec->frames_played), wdec, cwin);

	return 0;
}

static gint sndfile_decode(struct con_wav_decoder *wdec, struct con_win *cwin)
{
	gint ret = 0, iter = 0, play_size = 0;
	enum thread_cmd cmd = 0;

	do {
		/* Read */

		memset(wdec->buf, '\0', BUF_LEN);
		ret = sf_read_short(wdec->sfile, wdec->buf, BUF_LEN);

		if (ret <= 0)
			break;

		iter = 0;

		while (iter < ret) {
			/* Get and process command */

			cmd = process_thread_command(cwin);
			switch(cmd) {
			case CMD_PLAYBACK_STOP:
				goto exit;
			case CMD_PLAYBACK_SEEK:
				if (sndfile_seek(wdec, cwin) < 0)
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

			/* Play */

			if ((ret - iter) >= 2048)
				play_size = 2048;
			else
				play_size = (ret - iter);

			if (cwin->cpref->software_mixer)
				soft_volume_apply((gchar *)wdec->buf + (iter * 2),
						  play_size * 2, cwin);

			if (!ao_play(cwin->clibao->ao_dev,
				     (gchar*)wdec->buf + (iter * 2),
				     play_size * 2))
				g_critical("libao output error");

			iter += play_size;

			update_gui(play_size, wdec, cwin);
		}

		if (cmd)
			continue;
		if (ret < BUF_LEN)
			break;
	} while (1);
exit:
	/* Cleanup */

	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_wav(struct con_win *cwin)
{
	struct con_wav_decoder wdec;
	gint ret = 0;
	gboolean lastfm_f = FALSE;

	if (!cwin->cstate->curr_mobj->file)
		return;

	/* Open audio device */

	if (open_audio_device(cwin->cstate->curr_mobj->tags->samplerate,
			      cwin->cstate->curr_mobj->tags->channels,
			      FALSE,
			      cwin) == -1) {
		g_warning("Unable to play file: %s",
			  cwin->cstate->curr_mobj->file);
		goto exit1;
	}

	CDEBUG(DBG_INFO, "Playing : %s", cwin->cstate->curr_mobj->file);

	memset(&wdec, 0, sizeof(wdec));

	/* Open the file */

	wdec.fd = g_open(cwin->cstate->curr_mobj->file, O_RDONLY, 0);
	if (wdec.fd == -1) {
		g_critical("Unable to open file : %s",
			   cwin->cstate->curr_mobj->file);
		return;
	}

	wdec.sfile = sf_open_fd(wdec.fd, SFM_READ, &wdec.sinfo, 0);
	if (!wdec.sfile) {
		g_critical("Unable to open sndfile : %s",
			   cwin->cstate->curr_mobj->file);
		goto exit;
	}

	if (wdec.sinfo.channels > 2) {
		g_critical("Non mono/stereo channels not supported : %s",
			   cwin->cstate->curr_mobj->file);
		goto exit;
	}

	lastfm_track_reset(cwin, &wdec.ltrack);
	lastfm_f = TRUE;

	/* Decode */

	if (cwin->cstate->curr_mobj->tags->length)
		wdec.frame_size = wdec.sinfo.frames / cwin->cstate->curr_mobj->tags->length;
	if (wdec.sinfo.channels == 2)
		wdec.frame_size *= 2;

	ret = sndfile_decode(&wdec, cwin);

	/* Cleanup and return */

	sf_close(wdec.sfile);
exit:
	close(wdec.fd);
exit1:
	playback_end_cleanup(cwin, wdec.ltrack, ret, lastfm_f);
}
