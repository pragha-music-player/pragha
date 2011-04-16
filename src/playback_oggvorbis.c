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

static void update_gui(struct con_vorbis_decoder *vdec, struct con_win *cwin)
{
	gint newsec = ov_time_tell(&vdec->vf);

	if ((newsec != vdec->played_seconds) &&
	    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
		cwin->cstate->newsec = newsec;
		if (newsec > vdec->ltrack->play_duration)
			vdec->ltrack->play_duration = newsec;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
		vdec->played_seconds = newsec;
	}
}

static gint vorbis_seek(struct con_vorbis_decoder *vdec, struct con_win *cwin)
{
	gdouble seek_fraction;

	if (!ov_seekable(&vdec->vf))
		return -1;

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_fraction = cwin->cstate->seek_fraction;
	g_mutex_unlock(cwin->cstate->c_mutex);

	ov_pcm_seek(&vdec->vf, vdec->tot_samples * seek_fraction);
	update_gui(vdec, cwin);

	return 0;
}

static gint vorbis_decode(struct con_vorbis_decoder *vdec, struct con_win *cwin)
{
	gint bitstream;
	gint ret = 0;
	enum thread_cmd cmd = 0;

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
			if (vorbis_seek(vdec, cwin) < 0)
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

		/* Decode */

		memset(vdec->buf, '\0', OUTBUF_LEN);
		ret = ov_read(&vdec->vf,
			      vdec->buf,
			      OUTBUF_LEN,
			      (G_BYTE_ORDER == G_LITTLE_ENDIAN) ? 0 : 1,
			      2, 1,
			      &bitstream);

		if (ret <= 0)
			break;

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)vdec->buf,
					  ret, cwin);

		ao_play(cwin->clibao->ao_dev, (gchar*)vdec->buf, ret);

		update_gui(vdec, cwin);

	} while (1);
exit:
	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_oggvorbis(struct con_win *cwin)
{
	struct con_vorbis_decoder vdec;
	gint ret = 0, err = 0;
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
		goto exit;
	}

	CDEBUG(DBG_INFO, "Playing : %s", cwin->cstate->curr_mobj->file);

	if (cwin->cstate->curr_mobj->tags->channels > 2) {
		g_critical("No support for non mono/stereo files");
		goto exit;
	}

	memset(&vdec, 0, sizeof(vdec));

	/* Open the file */

	if ((err = ov_fopen(cwin->cstate->curr_mobj->file, &vdec.vf)) < 0) {
		g_critical("Unable to initialize vorbis decoder: %d", err);
		goto exit;
	}
	vdec.tot_samples = ov_pcm_total(&vdec.vf, -1);

	lastfm_track_reset(cwin, &vdec.ltrack);
	lastfm_f = TRUE;

	/* Decode */

	ret = vorbis_decode(&vdec, cwin);

	/* Close and cleanup */

	ov_clear(&vdec.vf);
exit:
	playback_end_cleanup(cwin, vdec.ltrack, ret, lastfm_f);

}
