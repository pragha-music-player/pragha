/*************************************************************************/
/* Copyright (C) 2010-2012 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_PLAYBACK_H
#define PRAGHA_PLAYBACK_H

#include "pragha.h"

void pragha_playback_notificate_new_track (PraghaBackend *backend, GParamSpec *pspec, struct con_win *cwin);

void pragha_playback_prev_track           (struct con_win *cwin);
void pragha_playback_play_pause_resume    (struct con_win *cwin);
void pragha_playback_stop                 (struct con_win *cwin);
void pragha_playback_next_track           (struct con_win *cwin);
void pragha_advance_playback              (struct con_win *cwin);

#endif /* PRAGHA_PLAYBACK_H */