/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_LASTFM_H
#define PRAGHA_LASTFM_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBCLASTFM

#include <clastfm.h>

#include "pragha-preferences.h"

/* pragha.h */
struct con_win;

typedef struct _PraghaLastfm PraghaLastfm;

void          pragha_lastfm_set_password (PraghaPreferences *preferences, const gchar *pass);
const gchar  *pragha_lastfm_get_password (PraghaPreferences *preferences);

gint          pragha_lastfm_connect      (PraghaLastfm *clastfm);
void          pragha_lastfm_disconnect   (PraghaLastfm *clastfm);

PraghaLastfm *pragha_lastfm_new          (struct con_win *cwin);
void          pragha_lastfm_free         (PraghaLastfm *clastfm);

#endif

#endif /* PRAGHA_LASTFM_H */
