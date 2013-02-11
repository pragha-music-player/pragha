/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#ifndef PRAGHA_ALBUM_ART_H
#define PRAGHA_ALBUM_ART_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_ALBUM_ART (pragha_album_art_get_type())
#define PRAGHA_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_ALBUM_ART, PraghaAlbumArt))
#define PRAGHA_ALBUM_ART_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_ALBUM_ART, PraghaAlbumArt const))
#define PRAGHA_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_ALBUM_ART, PraghaAlbumArtClass))
#define PRAGHA_IS_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_ALBUM_ART))
#define PRAGHA_IS_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_ALBUM_ART))
#define PRAGHA_ALBUM_ART_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_ALBUM_ART, PraghaAlbumArtClass))


typedef struct _PraghaAlbumArt PraghaAlbumArt;
typedef struct _PraghaAlbumArtClass PraghaAlbumArtClass;
typedef struct _PraghaAlbumArtPrivate PraghaAlbumArtPrivate;

struct _PraghaAlbumArt
{
   GtkImage parent;

   /*< private >*/
   PraghaAlbumArtPrivate *priv;
};

struct _PraghaAlbumArtClass
{
   GtkImageClass parent_class;
};

PraghaAlbumArt *pragha_album_art_new (void);
GType pragha_album_art_get_type (void) G_GNUC_CONST;
const gchar *pragha_album_art_get_path (PraghaAlbumArt *albumart);
void pragha_album_art_set_path (PraghaAlbumArt *albumart,
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
