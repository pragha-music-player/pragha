/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2007-2009 Jared Casper <jaredcasper@gmail.com>		 */
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

#include "consonance.h"

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder,
						     const FLAC__Frame *frame,
						     const FLAC__int32 * const buffer[],
						     void *client_data)
{
	struct con_flac_decoder *fdec = (struct con_flac_decoder*)client_data;
	guint samp, chan, out_pos = 0;
	guint16 *out_buf16 = (guint16*)fdec->out_buf;
	guint8 *out_buf8 = (guint8*)fdec->out_buf;

	if (fdec->bits_per_sample == 8) {
		for (samp = out_pos = 0; samp < frame->header.blocksize; samp++) {
			for(chan = 0; chan < frame->header.channels; chan++) {
				out_buf8[out_pos++] = (guint8)buffer[chan][samp];
			}
		}
	} else {
		for (samp = out_pos = 0; samp < frame->header.blocksize; samp++) {
			for(chan = 0; chan < frame->header.channels; chan++) {
				out_buf16[out_pos++] = (guint16)buffer[chan][samp];
			}
		}
	}

	fdec->out_buf_len = frame->header.blocksize
		* frame->header.channels
		* (fdec->bits_per_sample / 8);
	fdec->current_sample += frame->header.blocksize;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void metadata_callback(const FLAC__StreamDecoder *decoder,
			      const FLAC__StreamMetadata *metadata,
			      void *client_data)
{
	struct con_flac_decoder* fdec = (struct con_flac_decoder*)client_data;
	const FLAC__StreamMetadata_StreamInfo *si = &(metadata->data.stream_info);

	switch (metadata->type) {
	case FLAC__METADATA_TYPE_STREAMINFO:
		fdec->bits_per_sample = si->bits_per_sample;
		fdec->sample_rate = si->sample_rate;
		fdec->channels = si->channels;
		fdec->total_samples = si->total_samples;
		break;
	default:
		break;
	}
}

static void error_callback(const FLAC__StreamDecoder *decoder,
			   FLAC__StreamDecoderErrorStatus status,
			   void *client_data)
{
	switch (status) {
	case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
		g_critical("FLAC decoder: lost sync");
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
		g_critical("FLAC decoder: bad header");
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
		g_critical("FLAC decoder: crc mismatch");
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
		g_critical("FLAC decoder: unparseable stream");
		break;
	default:
		g_critical("FLAC decoder: unknown error");
	}
}

static void update_gui(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	gint newsec = fdec->current_sample / fdec->sample_rate;

	if ((newsec != fdec->displayed_seconds) &&
	    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
		cwin->cstate->newsec = newsec;
		if (newsec > fdec->ltrack->play_duration)
			fdec->ltrack->play_duration = newsec;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
		fdec->displayed_seconds = newsec;
	}
}

static gint flac_init(struct con_flac_decoder* fdec, struct con_win *cwin)
{
	FLAC__StreamDecoderInitStatus init_status;

	fdec->decoder = FLAC__stream_decoder_new();
	if (fdec->decoder == NULL) {
		g_critical("Unable to allocate decoder");
		return -1;
	}

	init_status = FLAC__stream_decoder_init_file(fdec->decoder,
						     cwin->cstate->curr_mobj->file,
						     write_callback,
						     metadata_callback,
						     error_callback,
						     fdec);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		g_critical("Unable to initialize decoder: %s",
			   FLAC__StreamDecoderInitStatusString[init_status]);
		return -1;
	}

	return 0;
}

static gint flac_seek(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	gdouble seek_fraction;

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_fraction = cwin->cstate->seek_fraction;
	g_mutex_unlock(cwin->cstate->c_mutex);

	fdec->current_sample = fdec->total_samples * seek_fraction;

	FLAC__stream_decoder_seek_absolute(fdec->decoder, fdec->current_sample);
	update_gui(fdec, cwin);

	return 0;
}

static gint flac_decode(struct con_flac_decoder *fdec, struct con_win *cwin)
{
	enum thread_cmd cmd = 0;

	if (flac_init(fdec, cwin) < 0)
		goto exit;

	/* Decode loop */

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
			if (flac_seek(fdec, cwin) < 0)
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

		if (!FLAC__stream_decoder_process_single(fdec->decoder))
			break;

		if (FLAC__stream_decoder_get_state(fdec->decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
			break;

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)fdec->out_buf,
					  fdec->out_buf_len, cwin);

		ao_play(cwin->clibao->ao_dev, (gchar*)fdec->out_buf, fdec->out_buf_len);

		update_gui(fdec, cwin);

	} while (1);
exit:
	if (fdec->decoder)
		FLAC__stream_decoder_delete(fdec->decoder);

	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_flac(struct con_win *cwin)
{
	gint ret = 0;
	gboolean lastfm_f = FALSE;
	struct con_flac_decoder fdec;

	if (!cwin->cstate->curr_mobj->file)
		return;

	memset(&fdec, 0, sizeof(struct con_flac_decoder));

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

	lastfm_track_reset(cwin, &fdec.ltrack);
	lastfm_f = TRUE;

	/* Decode */

	ret = flac_decode(&fdec, cwin);
exit:
	playback_end_cleanup(cwin, fdec.ltrack, ret, lastfm_f);
}
