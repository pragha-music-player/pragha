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
#include <glib-object.h>

G_BEGIN_DECLS

#define MAX_ENUM_SIZE 10

#define PRAGHA_TYPE_MUSIC_ENUM (pragha_music_enum_get_type())
#define PRAGHA_MUSIC_ENUM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_MUSIC_ENUM, PraghaMusicEnum))
#define PRAGHA_MUSIC_ENUM_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_MUSIC_ENUM, PraghaMusicEnum const))
#define PRAGHA_MUSIC_ENUM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_MUSIC_ENUM, PraghaMusicEnumClass))
#define PRAGHA_IS_MUSIC_ENUM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_MUSIC_ENUM))
#define PRAGHA_IS_MUSIC_ENUM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_MUSIC_ENUM))
#define PRAGHA_MUSIC_ENUM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_MUSIC_ENUM, PraghaMusicEnumClass))

typedef struct _PraghaMusicEnum PraghaMusicEnum;
typedef struct _PraghaMusicEnumClass PraghaMusicEnumClass;

struct _PraghaMusicEnumClass
{
	GObjectClass parent_class;
	void (*enum_removed)    (PraghaMusicEnum *enum_map, gint enum_removed);
};

PraghaMusicEnum *pragha_music_enum_get          (void);

const gchar     *pragha_music_enum_map_get_name (PraghaMusicEnum *enum_map, gint enum_code);
gint             pragha_music_enum_map_get      (PraghaMusicEnum *enum_map, const gchar *name);
gint             pragha_music_enum_map_remove   (PraghaMusicEnum *enum_map, const gchar *name);

G_END_DECLS

#endif /* PRAGHA_MUSIC_ENUM_H */
