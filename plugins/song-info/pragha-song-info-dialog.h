/*************************************************************************/
/* Copyright (C) 2011-2014 matias <mati86dl@gmail.com>                   */
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

#ifndef __PRAGHA_SONG_INFO_DIALOG_H__
#define __PRAGHA_SONG_INFO_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

void
pragha_show_related_text_info_dialog (GtkWidget   *widget,
                                      const gchar *title_header,
                                      const gchar *subtitle_header,
                                      const gchar *text);
G_END_DECLS

#endif /* __PRAGHA_SONG_INFO_DIALOG_H__ */
