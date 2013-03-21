/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_GLYR_H
#define PRAGHA_GLYR_H

#ifdef HAVE_LIBGLYR

#include <gtk/gtk.h>

/* pragha.h */
struct con_win;

G_BEGIN_DECLS

typedef struct _PraghaGlyr PraghaGlyr;

gchar * pragha_glyr_build_cached_art_path (PraghaGlyr *glyr, const gchar *artist, const gchar *album);
PraghaGlyr * pragha_glyr_new (struct con_win *cwin);
void pragha_glyr_free (PraghaGlyr *glyr);

G_END_DECLS

#endif

#endif /* PRAGHA_GLYR_H */
