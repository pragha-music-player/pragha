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

#ifndef PRAGHA_MENU_H
#define PRAGHA_MENU_H

#include <gtk/gtk.h>
#include "pragha-backend.h"

/* pragha.h */
typedef struct _PraghaApplication PraghaApplication;

void pragha_menubar_update_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data);

/* Playback */

void prev_action(GtkAction *action, PraghaApplication *pragha);
void play_pause_action(GtkAction *action, PraghaApplication *pragha);
void stop_action(GtkAction *action, PraghaApplication *pragha);
void next_action (GtkAction *action, PraghaApplication *pragha);
void add_audio_cd(PraghaApplication *pragha);
void quit_action(GtkAction *action, PraghaApplication *pragha);

/* Playlist */

void open_file_action(GtkAction *action, PraghaApplication *pragha);
void add_audio_cd_action(GtkAction *action, PraghaApplication *pragha);
void add_location_action(GtkAction *action, PraghaApplication *pragha);
void add_libary_action(GtkAction *action, PraghaApplication *pragha);
void search_playlist_action(GtkAction *action, PraghaApplication *pragha);

/* View */

void fullscreen_action (GtkAction *action, PraghaApplication *pragha);
void playlists_pane_action (GtkAction *action, PraghaApplication *pragha);
void show_controls_below_action (GtkAction *action, PraghaApplication *pragha);
void jump_to_playing_song_action (GtkAction *action, PraghaApplication *pragha);

/* Tools */

void show_equalizer_action(GtkAction *action, PraghaApplication *pragha);
void rescan_library_action(GtkAction *action, PraghaApplication *pragha);
void update_library_action(GtkAction *action, PraghaApplication *pragha);
void statistics_action(GtkAction *action, PraghaApplication *pragha);
void pref_action(GtkAction *action, PraghaApplication *pragha);

/* Help */

void home_action(GtkAction *action, PraghaApplication *pragha);
void community_action(GtkAction *action, PraghaApplication *pragha);
void wiki_action(GtkAction *action, PraghaApplication *pragha);
void translate_action(GtkAction *action, PraghaApplication *pragha);
void about_action(GtkAction *action, PraghaApplication *pragha);

/* Others */

void expand_all_action(GtkAction *action, PraghaApplication *pragha);
void collapse_all_action(GtkAction *action, PraghaApplication *pragha);

void pragha_menubar_connect_signals (GtkUIManager *menu_ui_manager, PraghaApplication *pragha);

GtkUIManager* pragha_menubar_new (void);

#endif /* PRAGHA_MENU_H */