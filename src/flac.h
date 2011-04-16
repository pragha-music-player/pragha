/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2007-2009 Jared Casper <jaredcasper@gmail.com>		 */
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

#ifndef FLAC_H
#define FLAC_H

#include <FLAC/stream_decoder.h>

#define FLAC_OUT_BUF_LEN 1152 * 16

struct con_flac_decoder {
	guint bits_per_sample;
	guint sample_rate;
	guint channels;
	gint displayed_seconds;
	guint64 current_sample;
	guint64 total_samples;
	guchar out_buf[FLAC_OUT_BUF_LEN];
	guint out_buf_len;
	FLAC__StreamDecoder *decoder;
	struct lastfm_track *ltrack;
};

void play_flac(struct con_win *cwin);

#endif /* FLAC_H */
