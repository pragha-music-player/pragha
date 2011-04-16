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

#ifndef WAV_H
#define WAV_H

#include <sndfile.h>

#define BUF_LEN 40000

struct con_wav_decoder {
	gint fd;		/* File Descriptor of the currently playing track */
	gshort buf[BUF_LEN];	/* i/p buffer */
	gint frame_size;	/* Frame size */
	gint frames_played;	/* Frames played so far */
	gint seconds_played;	/* Seconds played so far */
	SNDFILE *sfile;		/* sndfile file handle */
	SF_INFO sinfo;		/* sndfile info structure */
	struct lastfm_track *ltrack;
};

/* WAV Playback */

void play_wav(struct con_win *cwin);

#endif /* WAV_H */
