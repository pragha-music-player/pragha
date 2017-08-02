/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2017 matias <mati86dl@gmail.com>                   */
/* Copyright (C) 2012-2013 Pavel Vasin                                   */
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
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-database.h"
#include "pragha-prepared-statement-private.h"

#include <sqlite3.h>

#include "pragha-musicobject-mgmt.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-utils.h"
#include "pragha-debug.h"

G_DEFINE_TYPE(PraghaDatabase, pragha_database, G_TYPE_OBJECT)

struct _PraghaDatabasePrivate
{
	sqlite3 *sqlitedb;
	GHashTable *statements_cache;
	gboolean successfully;
};

enum {
	SIGNAL_PLAYLISTS_CHANGED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

gboolean
pragha_database_exec_query (PraghaDatabase *database,
                            const gchar *query)
{
	gchar *err = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(PRAGHA_IS_DATABASE(database), FALSE);

	if (!query)
		return FALSE;

	CDEBUG(DBG_DB, "%s", query);

	sqlite3_exec(database->priv->sqlitedb, query, NULL, NULL, &err);

	if (err) {
		g_critical("SQL Err : %s",  err);
		g_critical("query   : %s", query);
		sqlite3_free(err);
		ret = FALSE;
	}
	else {
		ret = TRUE;
	}

	return ret;
}

static PraghaPreparedStatement *
new_statement (PraghaDatabase *database, const gchar *sql)
{
	PraghaDatabasePrivate *priv = database->priv;
	sqlite3_stmt *stmt;

	if (sqlite3_prepare_v2 (priv->sqlitedb, sql, -1, &stmt, NULL) != SQLITE_OK) {
		g_critical ("db: %s", pragha_database_get_last_error (database));
		return NULL;
	}

	return pragha_prepared_statement_new (stmt, database);
}

PraghaPreparedStatement *
pragha_database_create_statement (PraghaDatabase *database, const gchar *sql)
{
	PraghaDatabasePrivate *priv = database->priv;
	PraghaPreparedStatement *cached = g_hash_table_lookup (priv->statements_cache, sql);

	if (cached) {
		g_hash_table_steal (priv->statements_cache, sql);
		return cached;
	}

	return new_statement (database, sql);
}

void
pragha_database_release_statement (PraghaDatabase *database, PraghaPreparedStatement *statement)
{
	PraghaDatabasePrivate *priv = database->priv;
	gpointer sql = (gpointer) pragha_prepared_statement_get_sql (statement);

	pragha_prepared_statement_reset (statement);
	g_hash_table_replace (priv->statements_cache, sql, statement);
}

void
pragha_database_begin_transaction (PraghaDatabase *database)
{
	pragha_database_exec_query (database, "BEGIN TRANSACTION");
}

void
pragha_database_commit_transaction (PraghaDatabase *database)
{
	pragha_database_exec_query (database, "END TRANSACTION");
}

gint
pragha_database_find_location (PraghaDatabase *database, const gchar *location)
{
	gint location_id = 0;
	const gchar *sql = "SELECT id FROM LOCATION WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, location);
	if (pragha_prepared_statement_step (statement))
		location_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return location_id;
}

gint
pragha_database_find_provider (PraghaDatabase *database, const gchar *provider)
{
	PraghaPreparedStatement *statement;
	gint provider_id = 0;

	const gchar *sql = "SELECT id FROM PROVIDER WHERE name = ?";

	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, provider);
	if (pragha_prepared_statement_step (statement))
		provider_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return provider_id;
}

gint
pragha_database_find_provider_type (PraghaDatabase *database, const gchar *provider_type)
{
	PraghaPreparedStatement *statement;
	gint provider_type_id = 0;

	const gchar *sql = "SELECT id FROM PROVIDER_TYPE WHERE name = ?";

	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, provider_type);
	if (pragha_prepared_statement_step (statement))
		provider_type_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);

	return provider_type_id;
}

gint
pragha_database_find_mime_type (PraghaDatabase *database, const gchar *mime_type)
{
	gint mime_type_id = 0;
	const gchar *sql = "SELECT id FROM MIME_TYPE WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, mime_type);
	if (pragha_prepared_statement_step (statement))
		mime_type_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return mime_type_id;
}

gint
pragha_database_find_artist (PraghaDatabase *database, const gchar *artist)
{
	gint artist_id = 0;
	const gchar *sql = "SELECT id FROM ARTIST WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, artist);
	if (pragha_prepared_statement_step (statement))
		artist_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return artist_id;
}

gint
pragha_database_find_album (PraghaDatabase *database, const gchar *album)
{
	gint album_id = 0;
	const gchar *sql = "SELECT id FROM ALBUM WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, album);
	if (pragha_prepared_statement_step (statement))
		album_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return album_id;
}

gint
pragha_database_find_genre (PraghaDatabase *database, const gchar *genre)
{
	gint genre_id = 0;
	const gchar *sql = "SELECT id FROM GENRE WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, genre);
	if (pragha_prepared_statement_step (statement))
		genre_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return genre_id;
}

gint
pragha_database_find_comment (PraghaDatabase *database, const gchar *comment)
{
	gint comment_id = 0;
	const gchar *sql = "SELECT id FROM COMMENT WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, comment);
	if (pragha_prepared_statement_step (statement))
		comment_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return comment_id;
}

gint
pragha_database_find_year (PraghaDatabase *database, gint year)
{
	gint year_id = 0;
	const gchar *sql = "SELECT id FROM YEAR WHERE year = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, year);
	if (pragha_prepared_statement_step (statement))
		year_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return year_id;
}

gint
pragha_database_find_playlist (PraghaDatabase *database, const gchar *playlist)
{
	gint playlist_id = 0;
	const gchar *sql = "SELECT id FROM PLAYLIST WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, playlist);
	if (pragha_prepared_statement_step (statement))
		playlist_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return playlist_id;
}

gint
pragha_database_find_radio (PraghaDatabase *database, const gchar *radio)
{
	gint radio_id = 0;
	const gchar *sql = "SELECT id FROM RADIO WHERE name = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, radio);
	if (pragha_prepared_statement_step (statement))
		radio_id = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	return radio_id;
}

gint
pragha_database_add_new_location (PraghaDatabase *database, const gchar *location)
{
	const gchar *sql = "INSERT INTO LOCATION (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, location);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_location (database, location);
}

gint
pragha_database_add_new_provider_type (PraghaDatabase *database, const gchar *provider_type)
{
	PraghaPreparedStatement *statement;

	const gchar *sql = "INSERT INTO PROVIDER_TYPE (name) VALUES (?)";

	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, provider_type);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_provider_type (database, provider_type);
}

gint
pragha_database_add_new_mime_type (PraghaDatabase *database, const gchar *mime_type)
{
	const gchar *sql = "INSERT INTO MIME_TYPE (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, mime_type);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_mime_type (database, mime_type);
}

gint
pragha_database_add_new_artist (PraghaDatabase *database, const gchar *artist)
{
	const gchar *sql = "INSERT INTO ARTIST (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, artist);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_artist (database, artist);
}

gint
pragha_database_add_new_album (PraghaDatabase *database, const gchar *album)
{
	const gchar *sql = "INSERT INTO ALBUM (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, album);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_album (database, album);
}

gint
pragha_database_add_new_genre (PraghaDatabase *database, const gchar *genre)
{
	const gchar *sql = "INSERT INTO GENRE (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, genre);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_genre (database, genre);
}

gint
pragha_database_add_new_comment (PraghaDatabase *database, const gchar *comment)
{
	const gchar *sql = "INSERT INTO COMMENT (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, comment);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_comment (database, comment);
}

gint
pragha_database_add_new_year (PraghaDatabase *database, guint year)
{
	const gchar *sql = "INSERT INTO YEAR (year) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, year);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_year (database, year);
}

void
pragha_database_forget_location (PraghaDatabase *database, gint location_id)
{
	const gchar *sql;
	PraghaPreparedStatement *statement;

	sql = "DELETE FROM TRACK WHERE location = ?";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	sql = "DELETE FROM LOCATION WHERE id = ?";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_forget_track (PraghaDatabase *database, const gchar *file)
{
	gint location_id = pragha_database_find_location (database, file);

	if (!location_id) {
		g_warning ("File not present in DB: %s", file);
		return;
	}

	pragha_database_forget_location(database, location_id);
}

gint
pragha_database_add_new_playlist (PraghaDatabase *database, const gchar *playlist)
{
	const gchar *sql = "INSERT INTO PLAYLIST (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, playlist);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_playlist (database, playlist);
}

void
pragha_database_add_playlist_track (PraghaDatabase *database, gint playlist_id, const gchar *file)
{
	const gchar *sql = "INSERT INTO PLAYLIST_TRACKS (file, playlist) VALUES (?, ?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, file);
	pragha_prepared_statement_bind_int (statement, 2, playlist_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

gint
pragha_database_add_new_radio (PraghaDatabase *database, const gchar *radio)
{
	const gchar *sql = "INSERT INTO RADIO (name) VALUES (?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, radio);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	return pragha_database_find_radio (database, radio);
}

void
pragha_database_add_radio_track (PraghaDatabase *database, gint radio_id, const gchar *uri)
{
	const gchar *sql = "INSERT INTO RADIO_TRACKS (uri, radio) VALUES (?, ?)";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, uri);
	pragha_prepared_statement_bind_int (statement, 2, radio_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_update_playlist_name (PraghaDatabase *database, const gchar *old_name, const gchar *new_name)
{
	gint playlist_id = pragha_database_find_playlist (database, old_name);

	if (!playlist_id)
		return;

	const gchar *sql = "UPDATE PLAYLIST SET name = ? WHERE id = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, new_name);
	pragha_prepared_statement_bind_int (statement, 2, playlist_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_update_radio_name (PraghaDatabase *database, const gchar *old_name, const gchar *new_name)
{
	gint radio_id = pragha_database_find_radio (database, old_name);

	if (!radio_id)
		return;

	const gchar *sql = "UPDATE RADIO SET name = ? WHERE id = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, new_name);
	pragha_prepared_statement_bind_int (statement, 2, radio_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_delete_dir (PraghaDatabase *database, const gchar *dir_name)
{
	const gchar *sql;
	PraghaPreparedStatement *statement;
	gchar *mask = g_strconcat (dir_name, "%", NULL);

	/* Delete all tracks under the given dir */

	sql = "DELETE FROM TRACK WHERE location IN (SELECT id FROM LOCATION WHERE NAME LIKE ?)";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, mask);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete the location entries */

	sql = "DELETE FROM LOCATION WHERE name LIKE ?";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, mask);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete all entries from PLAYLIST_TRACKS which match given dir */

	sql = "DELETE FROM PLAYLIST_TRACKS WHERE file LIKE ?";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, mask);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Now flush unused artists, albums, genres, years */

	pragha_database_flush_stale_entries (database);

	g_free (mask);
}

gint
pragha_database_get_playlist_count (PraghaDatabase *database)
{
	gint n_playlists = 0;

	const gchar *sql = "SELECT COUNT() FROM PLAYLIST WHERE name != ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);
	if (pragha_prepared_statement_step (statement))
		n_playlists = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);

	return n_playlists;
}

void
pragha_database_flush_playlist (PraghaDatabase *database, gint playlist_id)
{
	const gchar *sql = "DELETE FROM PLAYLIST_TRACKS WHERE playlist = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, playlist_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_delete_playlist (PraghaDatabase *database, const gchar *playlist)
{
	gint playlist_id;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	playlist_id = pragha_database_find_playlist (database, playlist);

	if (!playlist_id) {
		g_warning("Playlist doesn't exist");
		return;
	}

	pragha_database_flush_playlist (database, playlist_id);

	const gchar *sql = "DELETE FROM PLAYLIST WHERE id = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, playlist_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_flush_radio (PraghaDatabase *database, gint radio_id)
{
	const gchar *sql = "DELETE FROM RADIO_TRACKS WHERE radio = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, radio_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_delete_radio (PraghaDatabase *database, const gchar *radio)
{
	gint radio_id;

	if (string_is_empty(radio)) {
		g_warning("Radio name is NULL");
		return;
	}

	radio_id = pragha_database_find_radio (database, radio);

	if (!radio_id) {
		g_warning("Radio doesn't exist");
		return;
	}

	pragha_database_flush_radio (database, radio_id);

	const gchar *sql = "DELETE FROM RADIO WHERE id = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, radio_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

static void
pragha_database_add_new_track (PraghaDatabase *database,
                               gint location_id,
                               gint provider_id,
                               gint mime_type_id,
                               gint artist_id,
                               gint album_id,
                               gint genre_id,
                               gint year_id,
                               gint comment_id,
                               guint track_no,
                               gint length,
                               gint channels,
                               gint bitrate,
                               gint samplerate,
                               const gchar *title)
{
	const gchar *sql = "INSERT INTO TRACK ("
				"location, "
				"provider, "
				"file_type, "
				"track_no, "
				"artist, "
				"album, "
				"genre, "
				"year, "
				"comment, "
				"bitrate, "
				"samplerate, "
				"length, "
				"channels, "
				"title) "
				"VALUES "
				"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	pragha_prepared_statement_bind_int (statement, 2, provider_id);
	pragha_prepared_statement_bind_int (statement, 3, mime_type_id);
	pragha_prepared_statement_bind_int (statement, 4, track_no);
	pragha_prepared_statement_bind_int (statement, 5, artist_id);
	pragha_prepared_statement_bind_int (statement, 6, album_id);
	pragha_prepared_statement_bind_int (statement, 7, genre_id);
	pragha_prepared_statement_bind_int (statement, 8, year_id);
	pragha_prepared_statement_bind_int (statement, 9, comment_id);
	pragha_prepared_statement_bind_int (statement, 10, bitrate);
	pragha_prepared_statement_bind_int (statement, 11, samplerate);
	pragha_prepared_statement_bind_int (statement, 12, length);
	pragha_prepared_statement_bind_int (statement, 13, channels);
	pragha_prepared_statement_bind_string (statement, 14, title);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_database_add_new_musicobject (PraghaDatabase *database, PraghaMusicobject *mobj)
{
	const gchar *file, *provider, *mime_type, *artist, *album, *genre, *comment;
	gint location_id = 0, provider_id = 0, mime_type_id = 0, artist_id = 0, album_id = 0, genre_id = 0, year_id = 0, comment_id;

	if (G_LIKELY(mobj)) {
		file = pragha_musicobject_get_file (mobj);
		provider = pragha_musicobject_get_provider (mobj);
		mime_type = pragha_musicobject_get_mime_type (mobj);
		artist = pragha_musicobject_get_artist (mobj);
		album = pragha_musicobject_get_album (mobj);
		genre = pragha_musicobject_get_genre (mobj);
		comment = pragha_musicobject_get_comment (mobj);

		/* If not have an associated provider not be stored in the database. */

		if ((provider_id = pragha_database_find_provider (database, provider)) == 0)
			return;

		/* Write location */

		if ((location_id = pragha_database_find_location (database, file)) == 0)
			location_id = pragha_database_add_new_location (database, file);

		/* Write mime_type */

		if ((mime_type_id = pragha_database_find_mime_type (database, mime_type)) == 0)
			mime_type_id = pragha_database_add_new_mime_type (database, mime_type);

		/* Write artist */

		if ((artist_id = pragha_database_find_artist (database, artist)) == 0)
			artist_id = pragha_database_add_new_artist (database, artist);

		/* Write album */

		if ((album_id = pragha_database_find_album (database, album)) == 0)
			album_id = pragha_database_add_new_album (database, album);

		/* Write genre */

		if ((genre_id = pragha_database_find_genre (database, genre)) == 0)
			genre_id = pragha_database_add_new_genre (database, genre);

		/* Write year */

		if ((year_id = pragha_database_find_year (database, pragha_musicobject_get_year (mobj))) == 0)
			year_id = pragha_database_add_new_year (database, pragha_musicobject_get_year (mobj));

		/* Write comment */

		if ((comment_id = pragha_database_find_comment (database, comment)) == 0)
			comment_id = pragha_database_add_new_comment (database, comment);

		/* Write track */

		pragha_database_add_new_track (database,
		                               location_id,
		                               provider_id,
		                               mime_type_id,
		                               artist_id,
		                               album_id,
		                               genre_id,
		                               year_id,
		                               comment_id,
		                               pragha_musicobject_get_track_no (mobj),
		                               pragha_musicobject_get_length (mobj),
		                               pragha_musicobject_get_channels (mobj),
		                               pragha_musicobject_get_bitrate (mobj),
		                               pragha_musicobject_get_samplerate (mobj),
		                               pragha_musicobject_get_title (mobj));
	}
}

gchar *
pragha_database_get_filename_from_location_id (PraghaDatabase *database, gint location_id)
{
	PraghaPreparedStatement *statement;
	const gchar *sql;
	gchar *file = NULL;

	sql = "SELECT name FROM LOCATION WHERE id = ?";
	statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);

	if (pragha_prepared_statement_step (statement))
		file = g_strdup (pragha_prepared_statement_get_string (statement, 0));
	else
		g_warning ("Unable to find filename for location id: %d", location_id);

	pragha_prepared_statement_free (statement);

	return file;
}

static void
pragha_database_update_track (PraghaDatabase *database,
                              gint location_id, gint changed,
                              gint track_no, const gchar *title,
                              gint artist_id, gint album_id, gint genre_id, gint year_id, gint comment_id)
{
	const gchar *sql;
	PraghaPreparedStatement *statement;

	if (changed & TAG_TNO_CHANGED) {
		sql = "UPDATE TRACK SET track_no = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, track_no);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_TITLE_CHANGED) {
		sql = "UPDATE TRACK SET title = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_string (statement, 1, title);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_ARTIST_CHANGED) {
		sql = "UPDATE TRACK SET artist = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, artist_id);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		sql = "UPDATE TRACK SET album = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, album_id);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_GENRE_CHANGED) {
		sql = "UPDATE TRACK SET genre = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, genre_id);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_YEAR_CHANGED) {
		sql = "UPDATE TRACK SET year = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, year_id);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
	if (changed & TAG_COMMENT_CHANGED) {
		sql = "UPDATE TRACK SET comment = ? WHERE LOCATION = ?";
		statement = pragha_database_create_statement (database, sql);
		pragha_prepared_statement_bind_int (statement, 1, comment_id);
		pragha_prepared_statement_bind_int (statement, 2, location_id);
		pragha_prepared_statement_step (statement);
		pragha_prepared_statement_free (statement);
	}
}

void
pragha_database_update_local_files_change_tag (PraghaDatabase *database, GArray *loc_arr, gint changed, PraghaMusicobject *mobj)
{
	gint track_no = 0, artist_id = 0, album_id = 0, genre_id = 0, year_id = 0, comment_id = 0;
	guint i = 0, elem = 0;

	if (!changed)
		return;

	if (!loc_arr)
		return;

	CDEBUG (DBG_VERBOSE, "Tags Changed: 0x%x", changed);

	if (changed & TAG_TNO_CHANGED) {
		track_no = pragha_musicobject_get_track_no (mobj);
	}
	if (changed & TAG_TITLE_CHANGED) {
	}
	if (changed & TAG_ARTIST_CHANGED) {
		const gchar *artist = pragha_musicobject_get_artist (mobj);
		artist_id = pragha_database_find_artist (database, artist);
		if (!artist_id)
			artist_id = pragha_database_add_new_artist (database, artist);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		const gchar *album = pragha_musicobject_get_album (mobj);
		album_id = pragha_database_find_album (database, album);
		if (!album_id)
			album_id = pragha_database_add_new_album (database, album);
	}
	if (changed & TAG_GENRE_CHANGED) {
		const gchar *genre = pragha_musicobject_get_genre (mobj);
		genre_id = pragha_database_find_genre (database, genre);
		if (!genre_id)
			genre_id = pragha_database_add_new_genre (database, genre);
	}
	if (changed & TAG_YEAR_CHANGED) {
		year_id = pragha_database_find_year (database, pragha_musicobject_get_year (mobj));
		if (!year_id)
			year_id = pragha_database_add_new_year (database, pragha_musicobject_get_year (mobj));
	}
	if (changed & TAG_COMMENT_CHANGED) {
		const gchar *comment = pragha_musicobject_get_comment (mobj);
		comment_id = pragha_database_find_comment (database, comment);
		if (!comment_id)
			comment_id = pragha_database_add_new_comment (database, comment);
	}

	pragha_database_begin_transaction (database);
	if (loc_arr) {
		elem = 0;
		for (i = 0; i < loc_arr->len; i++) {
			elem = g_array_index (loc_arr, gint, i);
			if (elem) {
				pragha_database_update_track (database,
				                              elem, changed,
				                              track_no,
				                              pragha_musicobject_get_title (mobj),
				                              artist_id,
				                              album_id,
				                              genre_id,
				                              year_id,
				                              comment_id);
			}
		}
	}
	pragha_database_commit_transaction (database);
}

gchar**
pragha_database_get_playlist_names (PraghaDatabase *database)
{
	GPtrArray *playlists = g_ptr_array_new ();

	const gchar *sql = "SELECT name FROM PLAYLIST WHERE name != ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		g_ptr_array_add (playlists, g_strdup (name));
	}

	pragha_prepared_statement_free (statement);

	if (playlists->len > 0) {
		g_ptr_array_add (playlists, NULL);
		return (gchar**) g_ptr_array_free (playlists, FALSE);
	} else {
		g_ptr_array_free (playlists, TRUE);
		return NULL;
	}
}

void
pragha_database_flush (PraghaDatabase *database)
{
	pragha_database_exec_query (database, "DELETE FROM TRACK");
	pragha_database_exec_query (database, "DELETE FROM LOCATION");
	pragha_database_exec_query (database, "DELETE FROM ARTIST");
	pragha_database_exec_query (database, "DELETE FROM ALBUM");
	pragha_database_exec_query (database, "DELETE FROM GENRE");
	pragha_database_exec_query (database, "DELETE FROM YEAR");
	pragha_database_exec_query (database, "DELETE FROM COMMENT");
}

void
pragha_database_flush_stale_entries (PraghaDatabase *database)
{
	pragha_database_exec_query (database, "DELETE FROM ARTIST WHERE id NOT IN (SELECT artist FROM TRACK);");
	pragha_database_exec_query (database, "DELETE FROM ALBUM WHERE id NOT IN (SELECT album FROM TRACK);");
	pragha_database_exec_query (database, "DELETE FROM GENRE WHERE id NOT IN (SELECT genre FROM TRACK);");
	pragha_database_exec_query (database, "DELETE FROM YEAR WHERE id NOT IN (SELECT year FROM TRACK);");
	pragha_database_exec_query (database, "DELETE FROM COMMENT WHERE id NOT IN (SELECT comment FROM TRACK);");
}

static gint
pragha_database_get_table_count (PraghaDatabase *database, const gchar *table)
{
	gchar *sql;
	gint count = 0;

	sql = g_strdup_printf ("SELECT COUNT() FROM %s;", table);
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	if (pragha_prepared_statement_step (statement))
		count = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);
	g_free (sql);

	return count;
}

gint
pragha_database_get_artist_count (PraghaDatabase *database)
{
	return pragha_database_get_table_count (database, "ARTIST");
}

gint
pragha_database_get_album_count (PraghaDatabase *database)
{
	return pragha_database_get_table_count (database, "ALBUM");
}

gint
pragha_database_get_track_count (PraghaDatabase *database)
{
	return pragha_database_get_table_count (database, "TRACK");
}

gboolean
pragha_database_init_schema (PraghaDatabase *database)
{
	gint i;

	const gchar *queries[] = {
		"PRAGMA user_version=140",

		"PRAGMA synchronous=OFF",

		"CREATE TABLE IF NOT EXISTS TRACK "
			"(location INT PRIMARY KEY,"
			"provider INT,"
			"track_no INT,"
			"artist INT,"
			"album INT,"
			"genre INT,"
			"year INT,"
			"comment INT,"
			"bitrate INT,"
			"length INT,"
			"channels INT,"
			"samplerate INT,"
			"file_type INT,"
			"title VARCHAR(255));",

		"CREATE TABLE IF NOT EXISTS LOCATION "
			"(id INTEGER PRIMARY KEY,"
			"name TEXT,"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS CACHE "
			"(id INTEGER PRIMARY KEY,"
			"name TEXT,"
			"size INT,"
			"playcount INT,"
			"timestamp INT,"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS PROVIDER "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"visible BOOLEAN NOT NULL CHECK (visible IN (0,1)),"
			"ignore BOOLEAN NOT NULL CHECK (ignore IN (0,1)),"
			"type INT,"
			"friendly_name TEXT,"
			"icon_name TEXT,"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS PROVIDER_TYPE "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS MIME_TYPE "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS ARTIST "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS ALBUM "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS GENRE "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS YEAR "
			"(id INTEGER PRIMARY KEY,"
			"year INT,"
			"UNIQUE(year));",

		"CREATE TABLE IF NOT EXISTS COMMENT "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS PLAYLIST_TRACKS "
			"(file TEXT,"
			"playlist INT);",

		"CREATE TABLE IF NOT EXISTS PLAYLIST "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));",

		"CREATE TABLE IF NOT EXISTS RADIO_TRACKS "
			"(uri TEXT,"
			"radio INT);",

		"CREATE TABLE IF NOT EXISTS RADIO "
			"(id INTEGER PRIMARY KEY,"
			"name VARCHAR(255),"
			"UNIQUE(name));"
	};

	for (i = 0; i < G_N_ELEMENTS(queries); i++) {
		if (!pragha_database_exec_query (database, queries[i]))
			return FALSE;
	}

	return TRUE;
}

/**
 * pragha_database_change_playlists_done:
 *
 */
void
pragha_database_change_playlists_done(PraghaDatabase *database)
{
	g_return_if_fail(PRAGHA_IS_DATABASE(database));

	g_signal_emit (database, signals[SIGNAL_PLAYLISTS_CHANGED], 0);
}

/**
 * pragha_database_compatibilize_version:
 *
 */
void
pragha_database_compatibilize_version (PraghaDatabase *database)
{
	/*gint i;

	const gchar *mime_types[] = {
		"audio/x-wav",
		"audio/mpeg",
		"audio/x-flac",
		"audio/ogg",
		"audio/x-ms-wma",
		"audio/x-m4a",
		"audio/ape",
		"audio/x-mod"
	};

	const gchar *sql_queries[] = {
		"UPDATE TRACK SET file_type = file_type + 1",
		"ALTER TABLE TRACK ADD COLUMN provider INT"
	};

	for (i = 0; i < G_N_ELEMENTS(mime_types); i++) {
		if (!pragha_database_find_mime_type (database, mime_types[i]))
			pragha_database_add_new_mime_type (database, mime_types[i]);
	}

	for (i = 0; i < G_N_ELEMENTS(sql_queries); i++) {
		pragha_database_exec_query (database, sql_queries[i]);
	}*/
}

gint
pragha_database_get_version (PraghaDatabase *database)
{
	gint version = 0;

	const gchar *sql = "PRAGMA user_version";
	PraghaPreparedStatement *statement = pragha_database_create_statement (database, sql);
	if (pragha_prepared_statement_step (statement))
		version = pragha_prepared_statement_get_int (statement, 0);
	pragha_prepared_statement_free (statement);

	return version;
}

/**
 * pragha_database_start_successfully:
 *
 */
gboolean
pragha_database_start_successfully (PraghaDatabase *database)
{
	g_return_val_if_fail(PRAGHA_IS_DATABASE(database), FALSE);

	return database->priv->successfully;
}

const gchar *
pragha_database_get_last_error (PraghaDatabase *database)
{
	return sqlite3_errmsg (database->priv->sqlitedb);
}

static void
pragha_database_print_stats (PraghaDatabase *database)
{
	PraghaDatabasePrivate *priv = database->priv;
	int current = 0, high = 0;

	sqlite3_db_status (priv->sqlitedb, SQLITE_DBSTATUS_STMT_USED, &current, &high, 0);
	gchar *formatted = g_format_size_full (current, G_FORMAT_SIZE_IEC_UNITS);
	CDEBUG (DBG_DB, "statements in cache: %i, mem used: %s",
			g_hash_table_size (priv->statements_cache),
			formatted);
	g_free (formatted);
}

static void
pragha_database_finalize (GObject *object)
{
	PraghaDatabase *database = PRAGHA_DATABASE(object);
	PraghaDatabasePrivate *priv = database->priv;

	pragha_database_print_stats (database);

	g_hash_table_destroy (priv->statements_cache);

	sqlite3_close(priv->sqlitedb);

	G_OBJECT_CLASS(pragha_database_parent_class)->finalize(object);
}

static void
pragha_database_class_init (PraghaDatabaseClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_database_finalize;

	signals[SIGNAL_PLAYLISTS_CHANGED] = g_signal_new ("PlaylistsChanged",
	                                                  G_TYPE_FROM_CLASS (object_class),
	                                                  G_SIGNAL_RUN_LAST,
	                                                  G_STRUCT_OFFSET (PraghaDatabaseClass, playlists_change),
	                                                  NULL, NULL,
	                                                  g_cclosure_marshal_VOID__VOID,
	                                                  G_TYPE_NONE, 0);

	g_type_class_add_private(object_class, sizeof(PraghaDatabasePrivate));
}

static void
pragha_database_init (PraghaDatabase *database)
{
	gint ret;
	gchar *database_file;
	const gchar *home;

	database->priv = G_TYPE_INSTANCE_GET_PRIVATE(database,
	                                             PRAGHA_TYPE_DATABASE,
	                                             PraghaDatabasePrivate);

	PraghaDatabasePrivate *priv = database->priv;

	priv->statements_cache = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
	                                       (GDestroyNotify) pragha_prepared_statement_finalize);

	home = g_get_user_config_dir();
	database_file = g_build_path(G_DIR_SEPARATOR_S, home, "/pragha/pragha.db", NULL);

	priv->successfully = FALSE;

	/* Create the database file */

	ret = sqlite3_open(database_file, &priv->sqlitedb);
	if (ret) {
		g_critical("Unable to open/create DATABASE file : %s", database_file);
		g_free(database_file);
		return;
	}
	g_free(database_file);

	if (!pragha_database_init_schema (database))
		return;

	priv->successfully = TRUE;
}

/**
 * pragha_database_get:
 *
 * Queries the global #PraghaDatabase instance, which is shared
 * by all modules. The function automatically takes a reference
 * for the caller, so you'll need to call g_object_unref() when
 * you're done with it.
 *
 * Return value: the global #PraghaDatabase instance.
 **/
PraghaDatabase*
pragha_database_get (void)
{
   static PraghaDatabase *database = NULL;

   if (G_UNLIKELY (database == NULL)) {
      database = g_object_new(PRAGHA_TYPE_DATABASE, NULL);
      g_object_add_weak_pointer(G_OBJECT (database),
                                (gpointer) &database);
   }
   else {
      g_object_ref (G_OBJECT (database));
   }

   return database;
}
