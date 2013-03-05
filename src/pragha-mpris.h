/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
/* Copyright (C) 2011      hakan  <smultimeter@gmail.com>                */
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

#ifndef PRAGHA_MPRIS_H
#define PRAGHA_MPRIS_H

#include "pragha.h"

typedef struct _PraghaMpris2 PraghaMpris2;

#define MPRIS_NAME "org.mpris.MediaPlayer2.pragha"
#define MPRIS_PATH "/org/mpris/MediaPlayer2"

gint mpris_init(struct con_win *cwin);
void mpris_update_any(struct con_win *cwin);
void mpris_update_metadata_changed(struct con_win *cwin);
void mpris_update_mobj_remove(struct con_win *cwin, PraghaMusicobject *mobj);
void mpris_update_mobj_added(struct con_win *cwin, PraghaMusicobject *mobj, GtkTreeIter *iter);
void mpris_update_mobj_changed(struct con_win *cwin, PraghaMusicobject *mobj, gint bitmask);
void mpris_update_tracklist_replaced(struct con_win *cwin);
void mpris_close(PraghaMpris2 *cmpris2);
void mpris_free(PraghaMpris2 *cmpris2);

PraghaMpris2 *pragha_mpris_new();

#endif /* PRAGHA_MPRIS_H */