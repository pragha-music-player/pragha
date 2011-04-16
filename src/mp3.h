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

#ifndef MP3_H
#define MP3_H

#include <mad.h>

#define IN_BUF_LEN 40000 + MAD_BUFFER_GUARD
#define OUT_BUF_LEN 1152 * 4

/* MP3 Type */

enum {
	CBR,
	VBR
};

struct con_win;

struct con_mad_decoder {
	gint fd;			/* File Descriptor of the currently playing track */
	gint no_samples;		/* Number of samples in a frame */
	gint vbr_seek_pos;		/* Current umber of elements in any of the vbr_* arrays */
	gint mp3_type;			/* CBR / VBR */
	guchar in_buf[IN_BUF_LEN];	/* i/p buffer */
	guchar out_buf[OUT_BUF_LEN];	/* o/p buffer */
	mad_timer_t timer;		/* libmad timer */
	GArray *vbr_mmap_pos;		/* Mmap position entries for easier backwards seek of VBR tracks */
	GArray *vbr_pbar_pos;		/* Progress bar position entries for easier .... */
	GArray *vbr_time_pos;		/* Time positions for easier ... */
	GIOChannel *chan;		/* IO Channel to read from fd ( used for CBR) */
	GMappedFile *map;		/* Mmaped file ( used for VBR) */
	struct lastfm_track *ltrack;	/* Track details to be submitted to last.fm */
	struct mad_stream mstream;	/* libmad stream */
	struct mad_frame mframe;	/* libmad frame */
	struct mad_header mheader;	/* libmad header */
	struct mad_synth msynth;	/* libmad PCM synth */
};

/* MP3 Playback */

void play_mp3(struct con_win *cwin);

#endif /* MP3_H */
