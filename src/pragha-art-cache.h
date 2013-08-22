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

G_BEGIN_DECLS

typedef struct _PraghaArtCache PraghaArtCache;

PraghaArtCache * pragha_art_cache_new      ();
void             pragha_art_cache_free     (PraghaArtCache *cache);
gchar *          pragha_art_cache_get      (PraghaArtCache *cache, const gchar *artist, const gchar *album);
gboolean         pragha_art_cache_contains (PraghaArtCache *cache, const gchar *artist, const gchar *album);
void             pragha_art_cache_put      (PraghaArtCache *cache, const gchar *artist, const gchar *album, gconstpointer data, gsize size);

G_END_DECLS

#endif /* PRAGHA_ART_CACHE_H */
