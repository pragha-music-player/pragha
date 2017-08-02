/*************************************************************************/
/* Copyright (C) 2017 matias <mati86dl@gmail.com>                        */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
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

#include "pragha-database.h"
#include "pragha-song-cache.h"

#include <glib/gstdio.h>
#include <gio/gio.h>

struct _PraghaSongCache {
	GObject        _parent;
	PraghaDatabase *cdbase;
	gchar          *cache_dir;
	gchar          *template_filename;
};

G_DEFINE_TYPE(PraghaSongCache, pragha_song_cache, G_TYPE_OBJECT)

static void
pragha_song_cache_finalize (GObject *object)
{
	PraghaSongCache *cache = PRAGHA_SONG_CACHE(object);

	g_free (cache->cache_dir);

	G_OBJECT_CLASS(pragha_song_cache_parent_class)->finalize(object);
}

static void
pragha_song_cache_dispose (GObject *object)
{
	PraghaSongCache *cache = PRAGHA_SONG_CACHE(object);

	if (cache->cdbase) {
		g_object_unref (cache->cdbase);
		cache->cdbase = NULL;
	}
	G_OBJECT_CLASS(pragha_song_cache_parent_class)->dispose(object);
}

static void
pragha_song_cache_class_init (PraghaSongCacheClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_song_cache_finalize;
	object_class->dispose = pragha_song_cache_dispose;
}

static void
pragha_song_cache_init (PraghaSongCache *cache)
{
	cache->cdbase = pragha_database_get ();
	cache->cache_dir = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (), "pragha", "songs", NULL);
	g_mkdir_with_parents (cache->cache_dir, S_IRWXU);
}

PraghaSongCache *
pragha_song_cache_get (void)
{
	static PraghaSongCache *cache = NULL;

	if (G_UNLIKELY (cache == NULL)) {
		cache = g_object_new (PRAGHA_TYPE_SONG_CACHE, NULL);
		g_object_add_weak_pointer (G_OBJECT (cache),
		                          (gpointer) &cache);
	}
	else {
		g_object_ref (G_OBJECT(cache));
	}

	return cache;
}

const gchar *
pragha_song_cache_get_template (PraghaSongCache *cache)
{
	return cache->template_filename;
}

void
pragha_song_cache_put_location (PraghaSongCache *cache, const gchar *location, const gchar *filename)
{
	PraghaPreparedStatement *statement;
	GFile *file, *destination;
	gchar *dest_filename = NULL, *file_basename = NULL;
	gint size = 0, timestamp = 0;
	gint location_id = 0;
	struct stat sbuf;

	location_id = pragha_database_find_location (cache->cdbase, location);

	/* FIXME: Gstreamer 'download' even if it is a local file */
	statement = pragha_database_create_statement (cache->cdbase, "SELECT name FROM CACHE WHERE id = ?");
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	if (pragha_prepared_statement_step (statement)) {
		pragha_prepared_statement_free (statement);
		return;
	}

	/* TODO: Do it async... */
	file = g_file_new_for_path (filename);
	file_basename = g_file_get_basename(file);
	dest_filename = g_strdup_printf ("%s%s%s", cache->cache_dir, G_DIR_SEPARATOR_S, file_basename);
	destination = g_file_new_for_path (dest_filename);
	if (g_file_copy (file, destination, G_FILE_COPY_NONE, NULL, NULL, NULL, NULL)) {
		if (g_stat(dest_filename, &sbuf) == 0) {
			timestamp = sbuf.st_mtime;
			size = sbuf.st_size;
		}

		statement = pragha_database_create_statement (cache->cdbase, "INSERT INTO CACHE (id, name, size, playcount, timestamp) VALUES (?, ?, ?, ?, ?)");
		pragha_prepared_statement_bind_int (statement, 1, location_id);
		pragha_prepared_statement_bind_string (statement, 2, file_basename);
		pragha_prepared_statement_bind_int (statement, 3, size);
		pragha_prepared_statement_bind_int (statement, 4, 1);
		pragha_prepared_statement_bind_int (statement, 5, timestamp);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}

	g_free (file_basename);
	g_free (dest_filename);
}

gchar *
pragha_song_cache_get_from_location (PraghaSongCache *cache, const gchar *location)
{
	PraghaPreparedStatement *statement;
	gchar *basename = NULL, *filename = NULL;
	gint location_id = 0;

	location_id = pragha_database_find_location (cache->cdbase, location);

	statement = pragha_database_create_statement (cache->cdbase, "SELECT name FROM CACHE WHERE id = ?");
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	if (pragha_prepared_statement_step (statement))
		basename = g_strdup (pragha_prepared_statement_get_string (statement, 0));
	pragha_prepared_statement_free (statement);

	if (basename != NULL) {
		filename = g_strdup_printf ("%s%s%s", cache->cache_dir, G_DIR_SEPARATOR_S, basename);
		if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			statement = pragha_database_create_statement (cache->cdbase, "UPDATE CACHE SET playcount = playcount + 1 WHERE id = ?");
			pragha_prepared_statement_bind_int (statement, 1, location_id);
			pragha_prepared_statement_step (statement);
			pragha_prepared_statement_free (statement);
		}
		else {
			statement = pragha_database_create_statement (cache->cdbase, "DELETE FROM CACHE WHERE id = ?");
			pragha_prepared_statement_bind_int (statement, 1, location_id);
			pragha_prepared_statement_step (statement);
			pragha_prepared_statement_free (statement);
			g_free (filename);
			filename = NULL;
		}
		g_free (basename);
	}

	return filename;
}

