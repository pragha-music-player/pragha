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

#include <glib/gi18n.h>
#include "pragha-album-art.h"

G_DEFINE_TYPE(PraghaAlbumArt, pragha_album_art, G_TYPE_OBJECT)

struct _PraghaAlbumArtPrivate
{
   gchar *uri;
};

enum
{
   PROP_0,
   PROP_URI,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

PraghaAlbumArt *
pragha_album_art_new (void)
{
   return g_object_new(PRAGHA_TYPE_ALBUM_ART, NULL);
}

/**
 * album_art_get_uri:
 *
 */
const gchar *
pragha_album_art_get_uri (PraghaAlbumArt *albumart)
{
   g_return_val_if_fail(PRAGHA_IS_ALBUM_ART(albumart), NULL);
   return albumart->priv->uri;
}

/**
 * album_art_set_uri:
 *
 */
void
pragha_album_art_set_uri (PraghaAlbumArt *albumart,
                          const gchar *uri)
{
   PraghaAlbumArtPrivate *priv;

   g_return_if_fail(PRAGHA_IS_ALBUM_ART(albumart));

   priv = albumart->priv;

   g_free(priv->uri);
   priv->uri = g_strdup(uri);
   g_object_notify_by_pspec(G_OBJECT(albumart), gParamSpecs[PROP_URI]);
}

static void
pragha_album_art_finalize (GObject *object)
{
   PraghaAlbumArtPrivate *priv;

   priv = PRAGHA_ALBUM_ART(object)->priv;

   g_free(priv->uri);

   G_OBJECT_CLASS(pragha_album_art_parent_class)->finalize(object);
}

static void
pragha_album_art_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
   PraghaAlbumArt *albumart = PRAGHA_ALBUM_ART(object);

   switch (prop_id) {
   case PROP_URI:
      g_value_set_string(value, pragha_album_art_get_uri(albumart));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
pragha_album_art_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
   PraghaAlbumArt *albumart = PRAGHA_ALBUM_ART(object);

   switch (prop_id) {
   case PROP_URI:
      pragha_album_art_set_uri(albumart, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
pragha_album_art_class_init (PraghaAlbumArtClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = pragha_album_art_finalize;
   object_class->get_property = pragha_album_art_get_property;
   object_class->set_property = pragha_album_art_set_property;
   g_type_class_add_private(object_class, sizeof(PraghaAlbumArtPrivate));

   /**
    * PraghaAlbumArt:uri:
    *
    */
   gParamSpecs[PROP_URI] =
      g_param_spec_string("uri",
                          _("Uri"),
                          _("The album art uri"),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_URI,
                                   gParamSpecs[PROP_URI]);
}

static void
pragha_album_art_init (PraghaAlbumArt *albumart)
{
   albumart->priv = G_TYPE_INSTANCE_GET_PRIVATE(albumart,
                                               PRAGHA_TYPE_ALBUM_ART,
                                               PraghaAlbumArtPrivate);
}