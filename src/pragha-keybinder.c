/*************************************************************************/
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>                   */
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

#include <keybinder.h>
#include "pragha-playback.h"
#include "pragha-tags-dialog.h"
#include "pragha.h"

static void keybind_prev_handler (const char *keystring, gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_backend_emitted_error (cwin->backend) == FALSE)
		pragha_playback_prev_track(cwin);
}

static void keybind_play_handler (const char *keystring, gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_backend_emitted_error (cwin->backend) == FALSE)
		pragha_playback_play_pause_resume(cwin);
}

static void keybind_stop_handler (const char *keystring, gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_backend_emitted_error (cwin->backend) == FALSE)
		pragha_playback_stop(cwin);
}

static void keybind_next_handler (const char *keystring, gpointer data)
{
	struct con_win *cwin = data;

	if (pragha_backend_emitted_error (cwin->backend) == FALSE)
		pragha_playback_next_track(cwin);
}

static void keybind_media_handler (const char *keystring, gpointer data)
{
	struct con_win *cwin = data;

	toogle_main_window (cwin, FALSE);
}

gint init_keybinder(struct con_win *cwin)
{
	keybinder_init ();

	keybinder_bind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler, cwin);
	keybinder_bind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler, cwin);
	keybinder_bind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler, cwin);
	keybinder_bind("XF86AudioNext", (KeybinderHandler) keybind_next_handler, cwin);
	keybinder_bind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler, cwin);

	return 0;
}

void keybinder_free()
{
	keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
	keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
	keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
	keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
	keybinder_unbind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler);
}

