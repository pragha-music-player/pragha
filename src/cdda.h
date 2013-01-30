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

#include <glib.h>

#ifdef HAVE_PARANOIA_NEW_INCLUDES
   #include <cdio/paranoia/cdda.h>
   #include <cdio/paranoia/cd_types.h>
   #ifdef __CDIO_CONFIG_H__
      #include <cdio/paranoia/cdio_unconfig.h>
   #endif
#else
   #include <cdio/cdda.h>
   #include <cdio/cd_types.h>
   #ifdef __CDIO_CONFIG_H__
      #include <cdio/cdio_unconfig.h>
   #endif
#endif

#ifdef __CDIO_CONFIG_H__
#include <cdio/cdio_unconfig.h>
#endif

#endif /* CDDA_H */
