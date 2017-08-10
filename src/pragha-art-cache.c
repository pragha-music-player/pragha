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

#include <glib/gstdio.h>

#include "pragha-utils.h"

struct _PraghaArtCache {
	GObject _parent;
	gchar   *cache_dir;
};

enum {
	SIGNAL_CACHE_CHANGED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaArtCache, pragha_art_cache, G_TYPE_OBJECT)

static void
pragha_art_cache_finalize (GObject *object)
{
	PraghaArtCache *cache = PRAGHA_ART_CACHE(object);

	g_free (cache->cache_dir);

	G_OBJECT_CLASS(pragha_art_cache_parent_class)->finalize(object);
}

static void
pragha_art_cache_class_init (PraghaArtCacheClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_art_cache_finalize;

	signals[SIGNAL_CACHE_CHANGED] =
		g_signal_new ("cache-changed",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaArtCacheClass, cache_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
}

static void
pragha_art_cache_init (PraghaArtCache *cache)
{
	cache->cache_dir = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (), "pragha", "art", NULL);
	g_mkdir_with_parents (cache->cache_dir, S_IRWXU);
}

PraghaArtCache *
pragha_art_cache_get (void)
{
	static PraghaArtCache *cache = NULL;

	if (G_UNLIKELY (cache == NULL)) {
		cache = g_object_new (PRAGHA_TYPE_ART_CACHE, NULL);
		g_object_add_weak_pointer (G_OBJECT (cache),
		                          (gpointer) &cache);
	}
	else {
		g_object_ref (G_OBJECT(cache));
	}

	return cache;
}

static gchar *
pragha_art_cache_build_path (PraghaArtCache *cache, const gchar *artist, const gchar *album)
{
	gchar *artist_escaped = pragha_escape_slashes (artist);
	gchar *album_escaped = pragha_escape_slashes (album);
	gchar *result = g_strdup_printf ("%s%salbum-%s-%s.jpeg", cache->cache_dir, G_DIR_SEPARATOR_S, artist_escaped, album_escaped);
	g_free (album_escaped);
	g_free (artist_escaped);
	return result;
}

gchar *
pragha_art_cache_get_uri (PraghaArtCache *cache, const gchar *artist, const gchar *album)
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
	gchar *path = pragha_art_cache_get_uri (cache, artist, album);

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

	g_signal_emit (cache, signals[SIGNAL_CACHE_CHANGED], 0);

	g_free (path);
	g_object_unref (pixbuf);
}
