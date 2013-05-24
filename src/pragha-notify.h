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

#ifndef PRAGHA_NOTIFY_H
#define PRAGHA_NOTIFY_H

#include <glib.h>

/* pragha.h */
struct con_win;

typedef struct PraghaNotify PraghaNotify;

gboolean       can_support_actions    (void);
PraghaNotify * pragha_notify_new      (struct con_win *cwin);
void           pragha_notify_show_osd (PraghaNotify *notify);
void           pragha_notify_free     (PraghaNotify *notify);

#endif /* PRAGHA_NOTIFY_H */
