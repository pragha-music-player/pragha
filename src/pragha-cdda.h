/*
 * Copyright (C) 2013 Pavel Vasin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PRAGHA_CDDA_H
#define PRAGHA_CDDA_H

#include <glib.h>

/* pragha.h */
struct con_win;

G_BEGIN_DECLS

void pragha_cdda_add  (struct con_win *cwin);
void pragha_cdda_free ();

G_END_DECLS

#endif /* PRAGHA_CDDA_H */
