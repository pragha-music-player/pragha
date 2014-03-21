/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#ifndef PRAGHA_MUSIC_ENUM_H
#define PRAGHA_MUSIC_ENUM_H

#include <glib.h>

#define MAX_ENUM_SIZE 10

G_BEGIN_DECLS

const gchar * pragha_music_enum_map_get_name (gint enum_code);

gint          pragha_music_enum_map_get      (const gchar *name);
gint          pragha_music_enum_map_remove   (const gchar *name);

void          pragha_music_enum_map_free     (void);
void          pragha_music_enum_map_init     (gint min_enum, gint max_enum);

void          test_pragha_music_enum_map     (void);

G_END_DECLS

#endif /* PRAGHA_MUSIC_ENUM_H */
