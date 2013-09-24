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

#ifndef PRAGHA_KEYBINDER_H
#define PRAGHA_KEYBINDER_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBKEYBINDER

/* pragha.h */
struct con_win;

gboolean keybinder_will_be_useful ();
gboolean init_keybinder (struct con_win *cwin);
void keybinder_free ();

#endif

#endif /* PRAGHA_KEYBINDER_H */
