/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
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

#include <glib.h>

#ifdef HAVE_PARANOIA_NEW_INCLUDES
   #include <cdio/paranoia/cdda.h>
#else
   #include <cdio/cdda.h>
   #ifdef __CDIO_CONFIG_H__
      #include <cdio/cdio_unconfig.h>
   #endif
#endif
#include <cdio/cd_types.h>
#include <cddb/cddb.h>

/* pragha.h */
struct con_win;

void pragha_cdda_free ();
void add_audio_cd(struct con_win *cwin);

#endif /* CDDA_H */
