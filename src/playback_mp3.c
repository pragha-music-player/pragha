/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "consonance.h"

/* Taken from minimad.c from libmad */

static signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static gint convert_samples(struct mad_pcm mpcm, guchar *out_buf)
{
	guint nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	guchar *ptr;

	nchannels = mpcm.channels;
	nsamples  = mpcm.length;
	left_ch   = mpcm.samples[0];
	right_ch  = mpcm.samples[1];

	ptr = out_buf;

	if (nchannels > 2) {
		g_critical("No Support for channels > 2");
		return -1;
	}

	while (nsamples--) {
		signed int sample;

		if (nchannels == 1) {
			sample = scale(*left_ch++);
			*ptr++ = (sample >> 0);
			*ptr++ = (sample >> 8);
			*ptr++ = (sample >> 0);
			*ptr++ = (sample >> 8);
		}
		else if (nchannels == 2) {
			sample = scale(*left_ch++);
			*ptr++ = (sample >> 0);
			*ptr++ = (sample >> 8);

			sample = scale(*right_ch++);
			*ptr++ = (sample >> 0);
			*ptr++ = (sample >> 8);
		}
	}

	return 0;
}

static gint fill_input_buffer(struct con_mad_decoder *mdec,
			      struct con_win *cwin,
			      guchar *read_start,
			      gsize read_size,
			      gsize *bytes_read)
{
	GError *err = NULL;
	gint ret = 0;

	ret = g_io_channel_read_chars(mdec->chan,
				      (gchar*)read_start,
				      read_size,
				      bytes_read,
				      &err);
	if (ret == G_IO_STATUS_ERROR) {
		g_critical("Unable to read chars");
		ret = -1;
	}
	else if (ret == G_IO_STATUS_EOF) {
		ret = -2;
	}

	return ret;
}

static void update_gui(struct con_mad_decoder *mdec, struct con_win *cwin)
{
	gint oldsec = 0, newsec = 0;
	mad_timer_t oldt;

	oldt = mdec->timer;
	mad_timer_add(&mdec->timer, mdec->mframe.header.duration);

	oldsec = mad_timer_count(oldt, MAD_UNITS_SECONDS);
	newsec = mad_timer_count(mdec->timer, MAD_UNITS_SECONDS);

	if ((newsec > oldsec) &&
	    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
		cwin->cstate->newsec = newsec;
		if (newsec > mdec->ltrack->play_duration)
			mdec->ltrack->play_duration = newsec;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
	}
}

static void update_vbr_seek_table(gint64 mmap_pos,
				  struct con_mad_decoder *mdec,
				  struct con_win *cwin)
{
	gint oldsec = 0, newsec = 0;
	gdouble pbar_pos = 0;

	if (mdec->vbr_seek_pos != 0)
		oldsec = mad_timer_count(g_array_index(mdec->vbr_time_pos,
						       mad_timer_t,
						       mdec->vbr_seek_pos-1),
					 MAD_UNITS_SECONDS);

	newsec = mad_timer_count(mdec->timer, MAD_UNITS_SECONDS);

	if ((newsec > oldsec) &&
	    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
		pbar_pos = newsec * (1/(gdouble)
				     cwin->cstate->curr_mobj->tags->length);
		mdec->vbr_mmap_pos = g_array_append_val(mdec->vbr_mmap_pos,
							mmap_pos);
		mdec->vbr_pbar_pos = g_array_append_val(mdec->vbr_pbar_pos,
							pbar_pos);
		mdec->vbr_time_pos = g_array_append_val(mdec->vbr_time_pos,
							mdec->timer);
		mdec->vbr_seek_pos++;
	}
}

/* Adapted from mpg321's 'scan' code */

static gint mad_get_type(struct con_mad_decoder *mdec, struct con_win *cwin)
{
	struct mad_stream stream;
	struct mad_header header;
	GError *err = NULL;
	guchar buf[(IN_BUF_LEN) * 2];
	gulong bitrate = 0;
	gint n_frames = 0, is_vbr = 0, type, ret;
	gsize bytes_read;

	memset(buf, '\0', (IN_BUF_LEN) * 2);

	ret = fill_input_buffer(mdec, cwin, buf, (IN_BUF_LEN) * 2, &bytes_read);
	if (ret < 0)
		return -1;

	mad_stream_init(&stream);
	mad_header_init(&header);
	mad_stream_buffer(&stream, buf, bytes_read);

	while (1) {
		if (mad_header_decode(&header, &stream)) {
			if (MAD_RECOVERABLE(stream.error))
				continue;
			else
				break;
		}

		/* Test the first n frames to see if this is a VBR file */
		if (!is_vbr && !(n_frames > 20)) {
			if (bitrate && header.bitrate != bitrate)
				is_vbr = 1;
			else
				bitrate = header.bitrate;
		}
        
		/* We have to assume it's not a VBR file if it hasn't already been
		   marked as one and we've checked n frames for
		   different bitrates */
		else if (!is_vbr)
			break;
	}

	if (!is_vbr) {
		type = CBR;
		mdec->no_samples = 32 * MAD_NSBSAMPLES(&header);
	}
	else if (is_vbr)
		type = VBR;

	mad_header_finish(&header);
	mad_stream_finish(&stream);

	/* Reset channel to the beginning */

	if (g_io_channel_seek_position(mdec->chan,
				       0,
				       G_SEEK_SET,
				       &err) != G_IO_STATUS_NORMAL)
		g_critical("Unable to seek to : %d", 0);

	return type;
}

static gint cbr_seek(gint seek_len, struct con_mad_decoder *mdec, struct con_win *cwin)
{
	GError *err = NULL;
	gint seek_pos = 0, ret = 0, frame_size, frame_res;
	gsize bytes_read = 0;

	frame_size = (mdec->no_samples * mdec->mframe.header.bitrate) /
		(8 *mdec->mframe.header.samplerate);
	frame_res = mdec->mframe.header.samplerate / mdec->no_samples;
	seek_pos = seek_len * frame_res * frame_size;

	if (g_io_channel_seek_position(mdec->chan,
				       seek_pos,
				       G_SEEK_SET,
				       &err) != G_IO_STATUS_NORMAL) {
		g_critical("Unable to seek to : %d", seek_pos);
		return -1;
	}

	memset(mdec->in_buf, '\0', IN_BUF_LEN);

	ret = fill_input_buffer(mdec, cwin, mdec->in_buf, IN_BUF_LEN, &bytes_read);
	if (ret < 0)
		return -1;

	/* As Rob Leslie says here:
	   http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
	*/

	mad_stream_buffer(&mdec->mstream, mdec->in_buf, bytes_read);
	mad_frame_mute(&mdec->mframe);
	mad_synth_mute(&mdec->msynth);
	mad_frame_decode(&mdec->mframe, &mdec->mstream);
	mad_frame_decode(&mdec->mframe, &mdec->mstream);
	mad_synth_frame(&mdec->msynth, &mdec->mframe);

	cwin->cstate->newsec = seek_len;
	if (seek_len > mdec->ltrack->play_duration)
		mdec->ltrack->play_duration = seek_len;
	g_idle_add(update_track_progress_bar, cwin);
	g_idle_add(update_current_song_info, cwin);

	mad_timer_set(&mdec->timer, cwin->cstate->seek_len, 0, 0);

	return 0;
}

static gint frac_from_table(gdouble seek_fraction, struct con_mad_decoder *mdec)
{
	gdouble elem = 0;
	gint i = 0;

	do {
		elem = g_array_index(mdec->vbr_pbar_pos, gdouble, i);
		if (elem >= seek_fraction)
			break;
	} while (++i < mdec->vbr_seek_pos);

	return i;
}

/* Basic VBR seek */

static gint vbr_seek(gdouble seek_fraction,
		     struct con_mad_decoder *mdec,
		     struct con_win *cwin)
{
	gint tab_pos = 0, b_time = 0, oldsec = 0, newsec = 0;
	gint64 b_mmap_pos = 0, cur_mmap_pos = 0;
	gsize bytes_read = 0;
	gdouble old_fraction = 0, frac_unit = 0;
	guchar *read_start = NULL;

	frac_unit = 1 / (gdouble)cwin->cstate->curr_mobj->tags->length;
	old_fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(
						     cwin->track_progress_bar));

	/* Seek backward */

	if (seek_fraction < old_fraction) {
		tab_pos = frac_from_table(seek_fraction, mdec);
		b_mmap_pos = g_array_index(mdec->vbr_mmap_pos, gint64, tab_pos);
		b_time = mad_timer_count(g_array_index(mdec->vbr_time_pos,
						       mad_timer_t,
						       tab_pos),
					 MAD_UNITS_SECONDS);
		read_start = (guchar*)g_mapped_file_get_contents(mdec->map) +
			b_mmap_pos;
		bytes_read = g_mapped_file_get_length(mdec->map) - b_mmap_pos;

		mad_stream_buffer(&mdec->mstream, read_start, bytes_read);
		mad_frame_mute(&mdec->mframe);
		mad_synth_mute(&mdec->msynth);
		mad_frame_decode(&mdec->mframe, &mdec->mstream);
		mad_frame_decode(&mdec->mframe, &mdec->mstream);
		mad_synth_frame(&mdec->msynth, &mdec->mframe);

		cwin->cstate->newsec = b_time;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);

		mad_timer_set(&mdec->timer, b_time, 0, 0);

		return 0;
	}

	/* Seek forward */

	mad_header_init(&mdec->mheader);

	while (1)
	{
		if (old_fraction >= seek_fraction)
			break;

		if (mad_header_decode(&mdec->mheader, &mdec->mstream)) {
			if (MAD_RECOVERABLE(mdec->mstream.error)) {
				g_warning("%s", mad_stream_errorstr(
						  &mdec->mstream));
				continue;
			}
			else {
				if (mdec->mstream.error == MAD_ERROR_BUFLEN) {
					g_critical("%s", mad_stream_errorstr(
							   &mdec->mstream));
					break;
				}
				else {
					g_critical("%s", mad_stream_errorstr(
							   &mdec->mstream));
					return -1;
				}
			}
		}

		oldsec = mad_timer_count(mdec->timer, MAD_UNITS_SECONDS);
		mad_timer_add(&mdec->timer, mdec->mheader.duration);
		newsec = mad_timer_count(mdec->timer, MAD_UNITS_SECONDS);

		if ((newsec > oldsec) &&
		    (newsec <= cwin->cstate->curr_mobj->tags->length)) {
			cwin->cstate->newsec = newsec;
			if (newsec > mdec->ltrack->play_duration)
				mdec->ltrack->play_duration = newsec;
			g_idle_add(update_track_progress_bar, cwin);
			g_idle_add(update_current_song_info, cwin);
			old_fraction += frac_unit;
		}

		cur_mmap_pos = mdec->mstream.ptr.byte - mdec->mstream.buffer;
		update_vbr_seek_table(cur_mmap_pos, mdec, cwin);
	}

	mad_header_finish(&mdec->mheader);

	return 0;
}

static gint mad_seek(struct con_mad_decoder *mdec, struct con_win *cwin)
{
	gint seek_len, ret = 0;
	gdouble seek_fraction = 0;

	g_mutex_lock(cwin->cstate->c_mutex);
	seek_len = cwin->cstate->seek_len;
	seek_fraction = cwin->cstate->seek_fraction;
	g_mutex_unlock(cwin->cstate->c_mutex);

	if (mdec->mp3_type == CBR) {
		CDEBUG(DBG_INFO, "MP3 CBR Seek");
		ret = cbr_seek(seek_len, mdec, cwin);
	}
	else {
		CDEBUG(DBG_INFO, "MP3 VBR Seek");
		ret = vbr_seek(seek_fraction, mdec, cwin);
	}

	return ret;
}

static gint mad_decode(struct con_mad_decoder *mdec, struct con_win *cwin)
{
	enum thread_cmd cmd = 0;
	gboolean vbr_ret = FALSE;
	gint rem = 0, read_size = 0, ret = 0;
	gint64 cur_mmap_pos = 0;
	gsize bytes_read = 0;
	guchar *read_start = NULL;
	gchar *map_start = NULL;

	mad_stream_init(&mdec->mstream);
	mad_frame_init(&mdec->mframe);
	mad_synth_init(&mdec->msynth);
	mad_timer_reset(&mdec->timer);

	/*Set initial values for mad decoder */

	memset(mdec->in_buf, '\0', IN_BUF_LEN);
	memset(mdec->out_buf, '\0', OUT_BUF_LEN);

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
			if (mad_seek(mdec, cwin) < 0)
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

		if ((mdec->mstream.buffer == NULL) ||
		    (mdec->mstream.error == MAD_ERROR_BUFLEN)) {
			if (vbr_ret)
				break;

			if (mdec->mstream.next_frame) {
				rem = mdec->mstream.bufend -
					mdec->mstream.next_frame;
				memmove(mdec->in_buf, mdec->mstream.next_frame,
					rem);
				read_start = mdec->in_buf + rem;
				read_size = IN_BUF_LEN - rem;
			}
			else {
				rem = 0;
				read_start = mdec->in_buf;
				read_size = IN_BUF_LEN;
			}

			if (mdec->mp3_type == CBR) {
				ret = fill_input_buffer(mdec, cwin, read_start,
							read_size, &bytes_read);
				if (ret < 0)
					break;

				mad_stream_buffer(&mdec->mstream,
						  mdec->in_buf,
						  bytes_read + rem);
				mdec->mstream.error = 0;

			}
			else if (mdec->mp3_type == VBR) {
				map_start = g_mapped_file_get_contents(mdec->map);
				bytes_read = g_mapped_file_get_length(mdec->map);
				mad_stream_buffer(&mdec->mstream,
						  (guchar*)map_start,
						  bytes_read);
				mdec->mstream.error = 0;
				vbr_ret = TRUE;
			}
		}

		if (mad_frame_decode(&mdec->mframe, &mdec->mstream)) {
			if (MAD_RECOVERABLE(mdec->mstream.error)) {
				continue;
			}
			else {
				if (mdec->mstream.error == MAD_ERROR_BUFLEN)
					continue;
				else {
					g_critical("%s", mad_stream_errorstr(
							   &mdec->mstream));
					break;
				}
			}
		}

		mad_synth_frame(&mdec->msynth, &mdec->mframe);

		/* Get PCM samples */

		memset(mdec->out_buf, '\0', OUT_BUF_LEN);
		if (convert_samples(mdec->msynth.pcm, mdec->out_buf) < 0)
			g_critical("Unable to convert samples");

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)mdec->out_buf,
					  mdec->msynth.pcm.length*4, cwin);
		/* Play */

		if (!ao_play(cwin->clibao->ao_dev,
			     (gchar *)mdec->out_buf,
			     mdec->msynth.pcm.length*4))
			g_critical("libao output error");

		update_gui(mdec, cwin);

		if (mdec->mp3_type == VBR) {
			cur_mmap_pos = mdec->mstream.ptr.byte -
				mdec->mstream.buffer;
			update_vbr_seek_table(cur_mmap_pos, mdec, cwin);
		}
	} while (1);

exit:
	mad_synth_finish(&mdec->msynth);
	mad_frame_finish(&mdec->mframe);
	mad_stream_finish(&mdec->mstream);

	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_mp3(struct con_win *cwin)
{
	GError *err = NULL;
	struct con_mad_decoder mdec;
	gint ret = 0;
	gboolean lastfm_f = FALSE;

	if (!cwin->cstate->curr_mobj->file)
		return;

	/* Open audio device */

	if (open_audio_device(cwin->cstate->curr_mobj->tags->samplerate,
			      2, FALSE, cwin) == -1) {
		g_warning("Unable to play file: %s",
			  cwin->cstate->curr_mobj->file);
		goto exit1;
	}

	CDEBUG(DBG_INFO, "Playing : %s", cwin->cstate->curr_mobj->file);

	/* Open the file */

	mdec.fd = g_open(cwin->cstate->curr_mobj->file, O_RDONLY, 0);
	if (mdec.fd == -1) {
		g_critical("Unable to open file : %s",
			   cwin->cstate->curr_mobj->file);
		goto exit1;
	}

	mdec.chan = g_io_channel_unix_new(mdec.fd);
	if (g_io_channel_set_encoding(mdec.chan, NULL, &err) !=
	    G_IO_STATUS_NORMAL) {
		g_critical("Unable to set IO Channel encoding");
		g_error_free(err);
		err = NULL;
		goto exit;
	}

	/* Find out if the file is CBR or VBR */

	mdec.mp3_type = mad_get_type(&mdec, cwin);

	/* Preparation for VBR */

	if (mdec.mp3_type == VBR) {
		mdec.vbr_seek_pos = 0;
		mdec.vbr_mmap_pos = g_array_new(FALSE, TRUE, sizeof(gint64));
		mdec.vbr_pbar_pos = g_array_new(FALSE, TRUE, sizeof(gdouble));
		mdec.vbr_time_pos = g_array_new(FALSE, TRUE, sizeof(mad_timer_t));

		mdec.map = g_mapped_file_new(cwin->cstate->curr_mobj->file,
					     FALSE, &err);
		if (!mdec.map) {
			g_critical("MMAP Error : %s", err->message);
			g_error_free(err);
			err = NULL;
			goto exit;
		}
		CDEBUG(DBG_INFO, "MMapped file: %s",
		       cwin->cstate->curr_mobj->file);
	}

	lastfm_track_reset(cwin, &mdec.ltrack);
	lastfm_f = TRUE;

	/* Decode */

	ret = mad_decode(&mdec, cwin);

	/* Close and cleanup */
exit:
	if (g_io_channel_shutdown(mdec.chan, FALSE, &err) != G_IO_STATUS_NORMAL) {
		g_critical("IO Shutdown error : %s", err->message);
		g_error_free(err);
		err = NULL;
	}
	g_io_channel_unref(mdec.chan);
	close(mdec.fd);

	if (mdec.mp3_type == VBR) {
		g_array_free(mdec.vbr_mmap_pos, TRUE);
		g_array_free(mdec.vbr_pbar_pos, TRUE);
		g_array_free(mdec.vbr_time_pos, TRUE);

		if (mdec.map) {
			CDEBUG(DBG_INFO, "Freeing mmaped file: %s",
			       cwin->cstate->curr_mobj->file);
			g_mapped_file_free(mdec.map);
		}
	}
exit1:
	playback_end_cleanup(cwin, mdec.ltrack, ret, lastfm_f);
}
