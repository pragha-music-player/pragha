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

#ifndef PRAGHA_MUSICOBJECT_MGMT_H
#define PRAGHA_MUSICOBJECT_MGMT_H

#include "pragha.h"

PraghaMusicobject* new_musicobject_from_file(const gchar *file);
PraghaMusicobject* new_musicobject_from_db(PraghaDatabase *cdbase, gint location_id);
PraghaMusicobject* new_musicobject_from_cdda(struct con_win *cwin, gint track_no);
PraghaMusicobject* new_musicobject_from_location(const gchar *uri, const gchar *name);
void pragha_update_musicobject_change_tag(PraghaMusicobject *mobj, gint changed, PraghaMusicobject *nmobj);

#endif /* PRAGHA_MUSICOBJECT_MGMT_H */