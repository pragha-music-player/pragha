/*************************************************************************/
/* Copyright (C) 2012-2019 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_ALBUM_ART_H
#define PRAGHA_ALBUM_ART_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_ALBUM_ART (pragha_album_art_get_type ())
G_DECLARE_FINAL_TYPE (PraghaAlbumArt, pragha_album_art, PRAGHA, ALBUM_ART, GtkImage)


PraghaAlbumArt *pragha_album_art_new (void);

const gchar *
pragha_album_art_get_path (PraghaAlbumArt *albumart);
void
pragha_album_art_set_path (PraghaAlbumArt *albumart,
                           const char *path);

guint
pragha_album_art_get_size (PraghaAlbumArt *albumart);
void
pragha_album_art_set_size (PraghaAlbumArt *albumart,
                           guint size);

void
pragha_album_art_set_pixbuf (PraghaAlbumArt *albumart,
                             GdkPixbuf *pixbuf);
GdkPixbuf *
pragha_album_art_get_pixbuf (PraghaAlbumArt *albumart);

G_END_DECLS

#endif /* PRAGHA_ALBUM_ART_H */
