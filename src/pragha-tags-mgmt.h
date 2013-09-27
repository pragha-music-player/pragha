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

#ifndef PRAGHA_TAGS_MGMT_H
#define PRAGHA_TAGS_MGMT_H

#include <gtk/gtk.h>
#include "pragha-musicobject.h"

/* pragha.h */
typedef struct _PraghaApplication PraghaApplication;

gboolean pragha_musicobject_set_tags_from_file(PraghaMusicobject *mobj, const gchar *file);
gboolean pragha_musicobject_save_tags_to_file(gchar *file, PraghaMusicobject *mobj, int changed);
gboolean confirm_tno_multiple_tracks(gint tno, GtkWidget *parent);
gboolean confirm_title_multiple_tracks(const gchar *title, GtkWidget *parent);
void pragha_update_local_files_change_tag(GPtrArray *file_arr, gint changed, PraghaMusicobject *mobj);

#endif /* PRAGHA_TAGS_MGMT_H */