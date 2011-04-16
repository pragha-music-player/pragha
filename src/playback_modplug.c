/*************************************************************************/
/* Copyright (C) 2008-2009 blub <woolf.linux@bumiller.com>               */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

#include "pragha.h"

ModPlug_Settings modplug_settings;

static void update_gui(struct con_modplug_decoder *mdec,
		       struct con_win *cwin)
{
	GTimeVal newtime;
	gint diff;

	g_get_current_time(&newtime);
	diff = newtime.tv_sec - mdec->timeval.tv_sec;
	if (diff < mdec->length) {
		cwin->cstate->newsec = diff;
		if (diff > mdec->ltrack->play_duration)
			mdec->ltrack->play_duration = diff;
		g_idle_add(update_track_progress_bar, cwin);
		g_idle_add(update_current_song_info, cwin);
	}
}

static gint modfile_decode(struct con_modplug_decoder *mdec, struct con_win *cwin)
{
	gint ret = 0, play_size = 0;
	enum thread_cmd cmd = 0;

	g_get_current_time(&mdec->timeval);

	do {
		/* Get and process command */

		cmd = process_thread_command(cwin);
		switch(cmd) {
		case CMD_PLAYBACK_STOP:
			goto exit;
		case CMD_PLAYBACK_SEEK:
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

		ret = ModPlug_Read(mdec->mf, mdec->buf, MODBUF_LEN);
		if(ret <= 0)
			break;

		if (cwin->cpref->software_mixer)
			soft_volume_apply((gchar *)mdec->buf,
					  ret, cwin);

		if (!ao_play(cwin->clibao->ao_dev, (gchar*)mdec->buf, ret))
			g_critical("libao output error");

		++play_size;

		update_gui(mdec, cwin);

	} while (1);
exit:
	/* Cleanup */

	if (cmd == CMD_PLAYBACK_STOP)
		return -1;
	else
		return 0;
}

void play_modplug(struct con_win *cwin)
{
	gint ret = 0;
	gboolean lastfm_f = FALSE;
	struct con_modplug_decoder mdec;

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

	if (cwin->cstate->curr_mobj->tags->channels > 2) {
		g_critical("No support for non mono/stereo files");
		goto exit;
	}

	memset(&mdec, 0, sizeof(mdec));

	/* Open the file */

	if(!g_file_get_contents(cwin->cstate->curr_mobj->file,
				&mdec.data, &mdec.length, NULL))
	{
		g_critical("Unable to open file : %s",
			   cwin->cstate->curr_mobj->file);
		return;
	}

	mdec.mf = ModPlug_Load((const void*)mdec.data, (int)mdec.length);

	if(!mdec.mf) {
		g_critical("ModPlug_Load failed for %s",
			   cwin->cstate->curr_mobj->file);
		goto exit;
	}

	ModPlug_GetSettings(&modplug_settings);
	modplug_settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING;
	modplug_settings.mChannels = cwin->cstate->curr_mobj->tags->channels;
	modplug_settings.mBits = 16;
	modplug_settings.mFrequency = cwin->cstate->curr_mobj->tags->samplerate; /* 44100 */
	modplug_settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
	ModPlug_SetSettings(&modplug_settings);

	lastfm_track_reset(cwin, &mdec.ltrack);
	lastfm_f = TRUE;

	/* Decode */

	ret = modfile_decode(&mdec, cwin);

	/* Close and Cleanup */
	
	ModPlug_Unload(mdec.mf);
exit:
	g_free(mdec.data);
	
exit1:
	playback_end_cleanup(cwin, mdec.ltrack, ret, lastfm_f);
}
