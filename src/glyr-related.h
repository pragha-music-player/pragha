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

#ifndef PRAGHA_GLYR_RELATED_H
#define PRAGHA_GLYR_RELATED_H

#include <gtk/gtk.h>

/* pragha.h */
struct con_win;

#ifdef HAVE_LIBGLYR

void related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_action(GtkAction *action, struct con_win *cwin);
void related_get_lyric_action(GtkAction *action, struct con_win *cwin);

int init_glyr_related (struct con_win *cwin);
void glyr_related_free (struct con_win *cwin);

#endif

#endif /* PRAGHA_GLYR_RELATED_H */