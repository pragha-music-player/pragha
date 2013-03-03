/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#include "pragha.h"

#ifndef PRAGHA_TOOLBAR_H
#define PRAGHA_TOOLBAR_H

typedef struct {
	GtkWidget *widget;
	PraghaAlbumArt *albumart;
	GtkWidget *track_progress_bar;
	GtkToolItem *prev_button;
	GtkToolItem *play_button;
	GtkToolItem *stop_button;
	GtkToolItem *next_button;
	GtkToolItem *unfull_button;
	GtkWidget *vol_button;
	GtkWidget *track_length_label;
	GtkWidget *track_time_label;
	GtkWidget *now_playing_label;
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *ntag_lastfm_button;
	#endif
} PraghaToolbar;

void __update_progress_song_info(struct con_win *cwin, gint length);
void __update_current_song_info(struct con_win *cwin);

void update_album_art(PraghaMusicobject *mobj, struct con_win *cwin);

GtkWidget     *pragha_toolbar_get_widget (PraghaToolbar *toolbar);
void           pragha_toolbar_free       (PraghaToolbar *toolbar);
PraghaToolbar *pragha_toolbar_new        (struct con_win *cwin);

#endif /* PRAGHA_TOOLBAR_H */