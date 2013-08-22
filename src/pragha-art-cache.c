/*
 * Copyright (C) 2011-2013 matias <mati86dl@gmail.com>
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

#include "pragha-art-cache.h"
#include "pragha-utils.h"

struct _PraghaArtCache {
	gchar *cache_dir;
};

PraghaArtCache *
pragha_art_cache_new ()
{
	PraghaArtCache *cache = g_slice_new (PraghaArtCache);
	cache->cache_dir = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (), "pragha", NULL);
	return cache;
}

void
pragha_art_cache_free (PraghaArtCache *cache)
{
	g_free (cache->cache_dir);
	g_slice_free (PraghaArtCache, cache);
}

static gchar *
pragha_art_cache_build_path (PraghaArtCache *cache, const gchar *artist, const gchar *album)
{
	return g_strdup_printf ("%s/album-%s-%s.jpeg", cache->cache_dir, artist, album);
}

gchar *
pragha_art_cache_get (PraghaArtCache *cache, const gchar *artist, const gchar *album)
{
	gchar *path = pragha_art_cache_build_path (cache, artist, album);

	if (g_file_test (path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == FALSE) {
		g_free (path);
		return NULL;
	}

	return path;
}

gboolean
pragha_art_cache_contains (PraghaArtCache *cache, const gchar *artist, const gchar *album)
{
	gchar *path = pragha_art_cache_get (cache, artist, album);

	if (path) {
		g_free (path);
		return TRUE;
	}

	return FALSE;
}

void
pragha_art_cache_put (PraghaArtCache *cache, const gchar *artist, const gchar *album, gconstpointer data, gsize size)
{
	GError *error = NULL;

	GdkPixbuf *pixbuf = pragha_gdk_pixbuf_new_from_memory (data, size);
	if (!pixbuf)
		return;

	gchar *path = pragha_art_cache_build_path (cache, artist, album);

	gdk_pixbuf_save (pixbuf, path, "jpeg", &error, "quality", "100", NULL);

	if (error) {
		g_warning ("Failed to save albumart file %s: %s\n", path, error->message);
		g_error_free (error);
	}

	g_free (path);
	g_object_unref (pixbuf);
}
