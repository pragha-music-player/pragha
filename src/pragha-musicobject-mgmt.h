/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2018 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_MUSICOBJECT_MGMT_H
#define PRAGHA_MUSICOBJECT_MGMT_H

#include "pragha-musicobject.h"
#include "pragha-database.h"

/* Flags to control tags changed. */

#define TAG_TNO_CHANGED     1<<0
#define TAG_TITLE_CHANGED   1<<1
#define TAG_ARTIST_CHANGED  1<<2
#define TAG_ALBUM_CHANGED   1<<3
#define TAG_GENRE_CHANGED   1<<4
#define TAG_YEAR_CHANGED    1<<5
#define TAG_COMMENT_CHANGED 1<<6

PraghaMusicobject *
new_musicobject_from_file                 (const gchar *file,
                                           const gchar *provider);

PraghaMusicobject *
new_musicobject_from_db                   (PraghaDatabase *cdbase,
                                           gint location_id);

PraghaMusicobject *
new_musicobject_from_location             (const gchar *uri,
                                           const gchar *name);

PraghaMusicobject *
pragha_database_get_artist_and_title_song (PraghaDatabase *cdbase,
                                           const gchar    *artist,
                                           const gchar    *title);

void
pragha_update_musicobject_change_tag      (PraghaMusicobject *mobj,
                                           gint               changed,
                                           PraghaMusicobject *nmobj);

#endif /* PRAGHA_MUSICOBJECT_MGMT_H */
