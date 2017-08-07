/*************************************************************************/
/* Copyright (C) 2010-2017 matias <mati86dl@gmail.com>                   */
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

#include "pragha-backend.h"
#include "pragha-playlist.h"

#include "pragha.h"

void pragha_playback_set_playlist_track   (PraghaPlaylist *playlist, PraghaMusicobject *mobj, PraghaApplication *pragha);

void pragha_playback_prev_track           (PraghaApplication *pragha);
void pragha_playback_play_pause_resume    (PraghaApplication *pragha);
void pragha_playback_stop                 (PraghaApplication *pragha);
void pragha_playback_next_track           (PraghaApplication *pragha);
void pragha_advance_playback              (PraghaApplication *pragha);

gboolean pragha_playback_can_go_prev      (PraghaApplication *pragha);
gboolean pragha_playback_can_go_next      (PraghaApplication *pragha);
gint     pragha_playback_get_no_tracks    (PraghaApplication *pragha);

void pragha_backend_finished_song         (PraghaBackend *backend, PraghaApplication *pragha);
void pragha_backend_tags_changed          (PraghaBackend *backend, gint changed, PraghaApplication *pragha);

void pragha_playback_show_current_album_art (GObject *object, PraghaApplication *pragha);
void pragha_playback_edit_current_track   (PraghaApplication *pragha);

void pragha_playback_seek_fraction (GObject *object, gdouble fraction, PraghaApplication *pragha);

#endif /* PRAGHA_PLAYBACK_H */
