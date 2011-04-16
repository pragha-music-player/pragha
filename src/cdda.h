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

#ifndef CDDA_H
#define CDDA_H

#include <cdio/cdda.h>
#include <cdio/cd_types.h>

struct con_cdda_decoder {
	gchar cdda_buf[CDIO_CD_FRAMESIZE_RAW];
	struct lastfm_track *ltrack;
	gint displayed_seconds;
	gint start;
	gint cur;
	gint end;
};

void play_cdda(struct con_win *cwin);

#endif /* CDDA_H */
