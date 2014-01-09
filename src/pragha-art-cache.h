/*
 * Copyright (C) 2013 Pavel Vasin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PRAGHA_ART_CACHE_H
#define PRAGHA_ART_CACHE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_ART_CACHE (pragha_art_cache_get_type())
#define PRAGHA_ART_CACHE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_ART_CACHE, PraghaArtCache))
#define PRAGHA_ART_CACHE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_ART_CACHE, PraghaArtCache const))
#define PRAGHA_ART_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_ART_CACHE, PraghaArtCacheClass))
#define PRAGHA_IS_ART_CACHE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_ART_CACHE))
#define PRAGHA_IS_ART_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_ART_CACHE))
#define PRAGHA_ART_CACHE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_ART_CACHE, PraghaArtCacheClass))

typedef struct _PraghaArtCache PraghaArtCache;
typedef struct _PraghaArtCacheClass PraghaArtCacheClass;

struct _PraghaArtCacheClass
{
	GObjectClass parent_class;
	void (*cache_changed)    (PraghaArtCache *cache);
};

PraghaArtCache * pragha_art_cache_get      (void);

gchar *          pragha_art_cache_get_uri  (PraghaArtCache *cache, const gchar *artist, const gchar *album);
gboolean         pragha_art_cache_contains (PraghaArtCache *cache, const gchar *artist, const gchar *album);
void             pragha_art_cache_put      (PraghaArtCache *cache, const gchar *artist, const gchar *album, gconstpointer data, gsize size);

G_END_DECLS

#endif /* PRAGHA_ART_CACHE_H */
