/*************************************************************************/
/* Copyright (C) 2010-2012 matias <mati86dl@gmail.com>			 */
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

#include "pragha.h"

void
pragha_playback_notificate_new_track(PraghaBackend *backend, gint state, struct con_win *cwin)
{
	if(state != ST_PLAYING)
		return;

	/* New song playback. */
	if(cwin->cstate->update_playlist_action != PLAYLIST_NONE) {
		/* Update current song info */
		__update_current_song_info(cwin);
		__update_progress_song_info(cwin, 0);

		/* Update and jump in current playlist */
		update_current_playlist_view_new_track(cwin);

		/* Update album art */
		update_album_art(cwin->cstate->curr_mobj, cwin);

		/* Show osd, and inform new album art. */
		show_osd(cwin);
		mpris_update_metadata_changed(cwin);
		cwin->cstate->update_playlist_action = PLAYLIST_NONE;
	}
}