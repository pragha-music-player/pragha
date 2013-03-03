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

#ifdef HAVE_LIBCLASTFM

#include <clastfm.h>
#include "pragha-simple-async.h"
#include "pragha.h"

#define LASTFM_API_KEY             "ecdc2d21dbfe1139b1f0da35daca9309"
#define LASTFM_SECRET              "f3498ce387f30eeae8ea1b1023afb32b"

struct con_lastfm {
	LASTFM_SESSION *session_id;
	enum LASTFM_STATUS_CODES status;
	time_t playback_started;
	PraghaMusicobject *nmobj;
	PRAGHA_MUTEX (nmobj_mutex);
};

enum LASTFM_QUERY_TYPE {
	LASTFM_NONE = 0,
	LASTFM_GET_SIMILAR,
	LASTFM_GET_LOVED
};

typedef struct {
	GList *list;
	guint query_type;
	guint query_count;
	struct con_win *cwin;
} AddMusicObjectListData;

void update_menubar_lastfm_state (struct con_win *cwin);
void edit_tags_corrected_by_lastfm(GtkButton *button, struct con_win *cwin);
void lastfm_get_similar_current_playlist_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_current_playlist_love_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_current_playlist_unlove_action (GtkAction *action, struct con_win *cwin);
void lastfm_add_favorites_action (GtkAction *action, struct con_win *cwin);
void lastfm_get_similar_action (GtkAction *action, struct con_win *cwin);
void lastfm_import_xspf_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_love_action(GtkAction *action, struct con_win *cwin);
void lastfm_track_unlove_action (GtkAction *action, struct con_win *cwin);
void lastfm_now_playing_handler (struct con_win *cwin);
gint just_init_lastfm (struct con_win *cwin);
gint init_lastfm(struct con_win *cwin);
void lastfm_free(struct con_lastfm *clastfm);

#endif

#endif /* PRAGHA_LASTFM_H */
