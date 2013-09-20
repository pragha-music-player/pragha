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
struct con_win;

void pragha_menubar_update_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data);

void open_file_action(GtkAction *action, struct con_win *cwin);
void add_audio_cd_action(GtkAction *action, struct con_win *cwin);
void add_location_action(GtkAction *action, struct con_win *cwin);
void prev_action(GtkAction *action, struct con_win *cwin);
void play_pause_action(GtkAction *action, struct con_win *cwin);
void stop_action(GtkAction *action, struct con_win *cwin);
void next_action (GtkAction *action, struct con_win *cwin);
void add_audio_cd(struct con_win *cwin);
void quit_action(GtkAction *action, struct con_win *cwin);
void expand_all_action(GtkAction *action, struct con_win *cwin);
void collapse_all_action(GtkAction *action, struct con_win *cwin);
void search_playlist_action(GtkAction *action, struct con_win *cwin);
void pref_action(GtkAction *action, struct con_win *cwin);
void fullscreen_action (GtkAction *action, struct con_win *cwin);
void playlists_pane_action (GtkAction *action, struct con_win *cwin);
void show_controls_below_action (GtkAction *action, struct con_win *cwin);
void jump_to_playing_song_action (GtkAction *action, struct con_win *cwin);
void show_equalizer_action(GtkAction *action, struct con_win *cwin);
void rescan_library_action(GtkAction *action, struct con_win *cwin);
void update_library_action(GtkAction *action, struct con_win *cwin);
void add_libary_action(GtkAction *action, struct con_win *cwin);
void statistics_action(GtkAction *action, struct con_win *cwin);
void home_action(GtkAction *action, struct con_win *cwin);
void community_action(GtkAction *action, struct con_win *cwin);
void wiki_action(GtkAction *action, struct con_win *cwin);
void translate_action(GtkAction *action, struct con_win *cwin);
void about_action(GtkAction *action, struct con_win *cwin);

void pragha_menubar_connect_signals (GtkUIManager *menu_ui_manager, struct con_win *cwin);

GtkUIManager* pragha_menubar_new (void);

#endif /* PRAGHA_MENU_H */