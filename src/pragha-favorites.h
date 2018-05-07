/*************************************************************************/
/* Copyright (C) 2018 matias <mati86dl@gmail.com>                        */
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

#ifndef PRAGHA_FAVORITES_H
#define PRAGHA_FAVORITES_H

#include <glib.h>
#include <glib-object.h>

#include "pragha-musicobject.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_FAVORITES (pragha_favorites_get_type())
#define PRAGHA_FAVORITES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_FAVORITES, PraghaFavorites))
#define PRAGHA_FAVORITES_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_FAVORITES, PraghaFavorites const))
#define PRAGHA_FAVORITES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_FAVORITES, PraghaFavoritesClass))
#define PRAGHA_IS_FAVORITES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_FAVORITES))
#define PRAGHA_IS_FAVORITES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_FAVORITES))
#define PRAGHA_FAVORITES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_FAVORITES, PraghaFavoritesClass))

typedef struct _PraghaFavorites PraghaFavorites;
typedef struct _PraghaFavoritesClass PraghaFavoritesClass;

struct _PraghaFavoritesClass
{
	GObjectClass parent_class;
	void (*song_added)    (PraghaFavorites *favorites, PraghaMusicobject *mobj);
	void (*song_removed)  (PraghaFavorites *favorites, PraghaMusicobject *mobj);
};

PraghaFavorites *
pragha_favorites_get      (void);

void             pragha_favorites_put_song      (PraghaFavorites *favorites, PraghaMusicobject *mobj);
void             pragha_favorites_remove_song   (PraghaFavorites *favorites, PraghaMusicobject *mobj);
gboolean         pragha_favorites_contains_song (PraghaFavorites *favorites, PraghaMusicobject *mobj);

G_END_DECLS

#endif /* PRAGHA_FAVORITES_H */
