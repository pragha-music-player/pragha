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

#ifndef OGGVORBIS_H
#define OGGVORBIS_H

#include <vorbis/vorbisfile.h>

#define OUTBUF_LEN 4096

struct con_vorbis_decoder {
	gchar buf[OUTBUF_LEN];
	gint played_seconds;
	gint64 tot_samples;
	OggVorbis_File vf;
	struct lastfm_track *ltrack;
};

void play_oggvorbis(struct con_win *cwin);

#endif /* OGGVORBIS_H */
