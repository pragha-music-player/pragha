/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

#include "pragha.h"

static void add_new_track_db(gint location_id,
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
			     gint file_type,
			     gchar *title,
			     struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("INSERT INTO TRACK ("
				"location, "
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
				"file_type, "
				"title) "
				"VALUES "
				"('%d', '%d', '%d', '%d', '%d', '%d', '%d', "
				"'%d', '%d', '%d', %d, '%d', '%s')",
				location_id,
				track_no,
				artist_id,
				album_id,
				genre_id,
				year_id,
				comment_id,
				bitrate,
				samplerate,
				length,
				channels,
				file_type,
				title);
	exec_sqlite_query(query, cwin, NULL);
}

static void add_entry_db(gchar *file, struct con_win *cwin)
{
	struct musicobject *mobj;
	gchar *sfile, *stitle, *sartist, *salbum, *sgenre, *scomment;
	gint location_id = 0, artist_id = 0, album_id = 0, genre_id = 0, year_id = 0, comment_id;

	mobj = new_musicobject_from_file(file);
	if (mobj) {
		sfile = sanitize_string_sqlite3(file);
		stitle = sanitize_string_sqlite3(mobj->tags->title);
		sartist = sanitize_string_sqlite3(mobj->tags->artist);
		salbum = sanitize_string_sqlite3(mobj->tags->album);
		sgenre = sanitize_string_sqlite3(mobj->tags->genre);
		scomment = sanitize_string_sqlite3(mobj->tags->comment);

		/* Write location */

		if ((location_id = find_location_db(sfile, cwin)) == 0)
			location_id = add_new_location_db(sfile, cwin);

		/* Write artist */

		if ((artist_id = find_artist_db(sartist, cwin)) == 0)
			artist_id = add_new_artist_db(sartist, cwin);

		/* Write album */

		if ((album_id = find_album_db(salbum, cwin)) == 0)
			album_id = add_new_album_db(salbum, cwin);

		/* Write genre */

		if ((genre_id = find_genre_db(sgenre, cwin)) == 0)
			genre_id = add_new_genre_db(sgenre, cwin);

		/* Write year */

		if ((year_id = find_year_db(mobj->tags->year, cwin)) == 0)
			year_id = add_new_year_db(mobj->tags->year, cwin);

		/* Write comment */

		if ((comment_id = find_comment_db(scomment, cwin)) == 0)
			comment_id = add_new_comment_db(scomment, cwin);

		/* Write track */

		add_new_track_db(location_id,
				 artist_id,
				 album_id,
				 genre_id,
				 year_id,
				 comment_id,
				 mobj->tags->track_no,
				 mobj->tags->length,
				 mobj->tags->channels,
				 mobj->tags->bitrate,
				 mobj->tags->samplerate,
				 mobj->file_type,
				 stitle,
				 cwin);

		g_free(sfile);
		g_free(stitle);
		g_free(sartist);
		g_free(salbum);
		g_free(sgenre);
		g_free(scomment);

		delete_musicobject(mobj);
	}
}

static void delete_track_db(gchar *file, struct con_win *cwin)
{
	gchar *query, *sfile;
	gint location_id;
	struct db_result result;

	sfile = sanitize_string_sqlite3(file);

	query = g_strdup_printf("SELECT id FROM LOCATION WHERE name = '%s';", sfile);
	exec_sqlite_query(query, cwin, &result);
	if (!result.no_rows) {
		g_warning("File not present in DB: %s", sfile);
		goto bad;
	}

	location_id = atoi(result.resultp[result.no_columns]);
	query = g_strdup_printf("DELETE FROM TRACK WHERE location = %d;", location_id);
	exec_sqlite_query(query, cwin, NULL);
bad:
	g_free(sfile);
}

/**************/
/* Public API */
/**************/

/* NB: All of the add_* functions require sanitized strings */

gint add_new_artist_db(gchar *artist, struct con_win *cwin)
{
	gchar *query;
	gint artist_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO ARTIST (name) VALUES ('%s')",
				artist);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM ARTIST WHERE name = '%s'",
				artist);
	if (exec_sqlite_query(query, cwin, &result)) {
		artist_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return artist_id;
}

gint add_new_album_db(gchar *album, struct con_win *cwin)
{
	gchar *query;
	gint album_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO ALBUM (name) VALUES ('%s')",
				album);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM ALBUM WHERE name = '%s'",
				album);
	if (exec_sqlite_query(query, cwin, &result)) {
		album_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return album_id;
}

gint add_new_genre_db(gchar *genre, struct con_win *cwin)
{
	gchar *query;
	gint genre_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO GENRE (name) VALUES ('%s')",
				genre);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM GENRE WHERE name = '%s'",
				genre);
	if (exec_sqlite_query(query, cwin, &result)) {
		genre_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return genre_id;
}

gint add_new_year_db(guint year, struct con_win *cwin)
{
	gchar *query;
	gint year_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO YEAR (year) VALUES ('%d')",
				year);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM YEAR WHERE year = '%d'",
				year);
	if (exec_sqlite_query(query, cwin, &result)) {
		year_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return year_id;
}

gint add_new_comment_db(gchar *comment, struct con_win *cwin)
{
	gchar *query;
	gint comment_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO COMMENT (name) VALUES ('%s')",
				comment);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM COMMENT WHERE name = '%s'",
				comment);
	if (exec_sqlite_query(query, cwin, &result)) {
		comment_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return comment_id;
}

gint add_new_location_db(gchar *location, struct con_win *cwin)
{
	gchar *query;
	gint location_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO LOCATION (name) VALUES ('%s')",
				location);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM LOCATION WHERE name = '%s'",
				location);
	if (exec_sqlite_query(query, cwin, &result)) {
		location_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return location_id;
}

void add_track_playlist_db(gchar *file, gint playlist_id, struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("INSERT INTO PLAYLIST_TRACKS (file, playlist) "
				"VALUES ('%s', %d);",
				file,
				playlist_id);
	exec_sqlite_query(query, cwin, NULL);
}

/* NB: All of the find_* functions require sanitized strings. */

gint find_artist_db(const gchar *artist, struct con_win *cwin)
{
	gint artist_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM ARTIST WHERE name = '%s';", artist);
	if (exec_sqlite_query(query, cwin, &result)) {
		if(result.no_rows)
			artist_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return artist_id;
}

gint find_album_db(const gchar *album, struct con_win *cwin)
{
	gint album_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM ALBUM WHERE name = '%s';", album);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_rows)
			album_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return album_id;
}

gint find_genre_db(const gchar *genre, struct con_win *cwin)
{
	gint genre_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM GENRE WHERE name = '%s';", genre);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_rows)
			genre_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return genre_id;
}

gint find_year_db(gint year, struct con_win *cwin)
{
	gint year_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM YEAR WHERE year = '%d';", year);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_rows)
			year_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return year_id;
}

gint find_comment_db(const gchar *comment, struct con_win *cwin)
{
	gint comment_id = 0;
	gchar *query;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM COMMENT WHERE name = '%s';", comment);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_rows)
			comment_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return comment_id;
}

gint find_location_db(const gchar *location, struct con_win *cwin)
{
	gchar *query;
	gint location_id = 0;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM LOCATION WHERE name = '%s'",
				location);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_columns)
			location_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return location_id;
}

gint find_playlist_db(const gchar *playlist, struct con_win *cwin)
{
	gchar *query;
	gint playlist_id = 0;
	struct db_result result;

	query = g_strdup_printf("SELECT id FROM PLAYLIST WHERE name = '%s'",
				playlist);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_columns)
			playlist_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return playlist_id;
}
void delete_location_db(gint location_id, struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("DELETE FROM TRACK WHERE location = %d;", location_id);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM LOCATION WHERE id = %d;", location_id);
	exec_sqlite_query(query, cwin, NULL);
}

gint delete_location_hdd(gint location_id, struct con_win *cwin)
{
	gint ret = 0;
	gchar *query, *file;
	struct db_result result;

	query = g_strdup_printf("SELECT name FROM LOCATION WHERE id = %d;", location_id);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_columns) {
			file = result.resultp[result.no_columns];
			ret = g_unlink(file);
			if (ret != 0)
				g_warning("%s", strerror(ret));
			else
				CDEBUG(DBG_VERBOSE, "Deleted file: %s", file);
		}
		sqlite3_free_table(result.resultp);
	} else {
		g_warning("Unable to find filename for location id: %d", location_id);
		ret = -1;
	}

	return ret;
}

/* Arg. title has to be sanitized */

void update_track_db(gint location_id, gint changed,
		     gint track_no, gchar *title,
		     gint artist_id, gint album_id, gint genre_id, gint year_id, gint comment_id,
		     struct con_win *cwin)
{
	gchar *query = NULL;

	if (changed & TAG_TNO_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET track_no = '%d' "
					"WHERE LOCATION = '%d';",
					track_no, location_id);
		exec_sqlite_query(query, cwin, NULL);

	}
	if (changed & TAG_TITLE_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET title = '%s' "
					"WHERE LOCATION = '%d';",
					title, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
	if (changed & TAG_ARTIST_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET artist = '%d' "
					"WHERE LOCATION = '%d';",
					artist_id, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
	if (changed & TAG_ALBUM_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET album = '%d' "
					"WHERE LOCATION = '%d';",
					album_id, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
	if (changed & TAG_GENRE_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET genre = '%d' "
					"WHERE LOCATION = '%d';",
					genre_id, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
	if (changed & TAG_YEAR_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET year = '%d' "
					"WHERE LOCATION = '%d';",
					year_id, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
	if (changed & TAG_COMMENT_CHANGED) {
		query = g_strdup_printf("UPDATE TRACK SET comment = '%d' "
					"WHERE LOCATION = '%d';",
					comment_id, location_id);
		exec_sqlite_query(query, cwin, NULL);
	}
}

/* 'playlist' has to be a sanitized string */

gint add_new_playlist_db(const gchar *playlist, struct con_win *cwin)
{
	gchar *query;
	gint playlist_id = 0;
	struct db_result result;

	query = g_strdup_printf("INSERT INTO PLAYLIST (name) VALUES ('%s')",
				playlist);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("SELECT id FROM PLAYLIST WHERE name = '%s'",
				playlist);
	if (exec_sqlite_query(query, cwin, &result)) {
		playlist_id = atoi(result.resultp[result.no_columns]);
		sqlite3_free_table(result.resultp);
	}

	return playlist_id;
}

/* Get the names of all the playlists stored in the DB.
   Returned NULL terminated array of strings that has to freed by caller. */

gchar** get_playlist_names_db(struct con_win *cwin)
{
	gchar *query;
	struct db_result result;
	gchar **playlists = NULL;
	gint i, j=0;

	query = g_strdup_printf("SELECT NAME FROM PLAYLIST WHERE NAME != \"%s\";",
				SAVE_PLAYLIST_STATE);
	if (exec_sqlite_query(query, cwin, &result)) {
		if (result.no_rows) {
			playlists = g_malloc0((result.no_rows+1) * sizeof(gchar *));
			for_each_result_row(result, i) {
				playlists[j] = g_strdup(result.resultp[i]);
				j++;
			}
			playlists[j] = NULL;
		}
		sqlite3_free_table(result.resultp);
	}

	return playlists;
}

/* 'playlist' has to be a sanitized string */

void delete_playlist_db(gchar *playlist, struct con_win *cwin)
{
	gint playlist_id;
	gchar *query;

	if (!playlist || !strlen(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	playlist_id = find_playlist_db(playlist, cwin);

	if (!playlist_id) {
		g_warning("Playlist doesn't exist");
		return;
	}

	query = g_strdup_printf("DELETE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d;",
				playlist_id);
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM PLAYLIST WHERE ID=%d;",
				playlist_id);
	exec_sqlite_query(query, cwin, NULL);
}

/* Flushes all the tracks in a given playlist */

void flush_playlist_db(gint playlist_id, struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("DELETE FROM PLAYLIST_TRACKS WHERE PLAYLIST=%d;",
				playlist_id);
	exec_sqlite_query(query, cwin, NULL);
}

void flush_db(struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("DELETE FROM TRACK");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM LOCATION");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM ARTIST");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM ALBUM");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM GENRE");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM YEAR");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM COMMENT");
	exec_sqlite_query(query, cwin, NULL);
}

/* Flush unused artists, albums, genres, years */

void flush_stale_entries_db(struct con_win *cwin)
{
	gchar *query;

	query = g_strdup_printf("DELETE FROM ARTIST WHERE id NOT IN "
				"(SELECT artist FROM TRACK);");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM ALBUM WHERE id NOT IN "
				"(SELECT album FROM TRACK);");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM GENRE WHERE id NOT IN "
				"(SELECT genre FROM TRACK);");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM YEAR WHERE id NOT IN "
				"(SELECT year FROM TRACK);");
	exec_sqlite_query(query, cwin, NULL);

	query = g_strdup_printf("DELETE FROM COMMENT WHERE id NOT IN "
				"(SELECT comment FROM TRACK);");
	exec_sqlite_query(query, cwin, NULL);
}

gboolean fraction_update(GtkWidget *pbar)
{
	static gdouble fraction = 0.0;
	gint files_scanned = 0;
	gint no_files;

	no_files = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pbar), "no_files"));
	files_scanned = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pbar), "files_scanned"));

	if(files_scanned > 0)
		fraction = (gdouble)files_scanned / (gdouble)no_files;

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), fraction);

	return TRUE;
}

void rescan_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin)
{
	static gint files_scanned = 0;
	gint progress_timeout = 0;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file;
	GError *error = NULL;

	/* Reinitialize static variables if called from rescan_library_action */

	if (call_recur)
		files_scanned = 0;

	if (cwin->cstate->stop_scan)
		goto exit;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		goto exit;
	}

	if(progress_timeout == 0) {
		g_object_set_data(G_OBJECT(pbar), "no_files", GINT_TO_POINTER(no_files));
		g_object_set_data(G_OBJECT(pbar), "files_scanned", GINT_TO_POINTER(files_scanned));
		progress_timeout = g_timeout_add_seconds(3, (GSourceFunc)fraction_update, pbar);
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if (cwin->cstate->stop_scan)
			goto exit;
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			rescan_db(ab_file, no_files, pbar, 0, cwin);
		else {
			files_scanned++;
			add_entry_db(ab_file, cwin);
		}
		/* Have to give control to GTK periodically ... */

		while(gtk_events_pending())
			gtk_main_iteration();

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
exit:
	if(progress_timeout != 0) {
		g_source_remove(progress_timeout);
		progress_timeout = 0;
	}
}

void update_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin)
{
	static gint files_scanned = 0;
	gint progress_timeout = 0;
	GDir *dir;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL, *s_ab_file = NULL;
	GError *error = NULL;
	struct stat sbuf;

	/* Reinitialize static variables if called from rescan_library_action */

	if (call_recur)
		files_scanned = 0;

	if (cwin->cstate->stop_scan)
		goto exit;

	dir = g_dir_open(dir_name, 0, &error);
	if (!dir) {
		g_critical("Unable to open library : %s", dir_name);
		goto exit;
	}

	if(progress_timeout == 0) {
		g_object_set_data(G_OBJECT(pbar), "no_files", GINT_TO_POINTER(no_files));
		g_object_set_data(G_OBJECT(pbar), "files_scanned", GINT_TO_POINTER(files_scanned));
		progress_timeout = g_timeout_add_seconds(3, (GSourceFunc)fraction_update, pbar);
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		if (cwin->cstate->stop_scan)
			goto exit;
		ab_file = g_strconcat(dir_name, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_DIR))
			update_db(ab_file, no_files, pbar, 0, cwin);
		else {
			files_scanned++;
			s_ab_file = sanitize_string_sqlite3(ab_file);
			if (!find_location_db(s_ab_file, cwin)) {
				add_entry_db(ab_file,cwin);
			} else {
				g_stat(ab_file, &sbuf);
				if (sbuf.st_mtime >
				    cwin->cpref->last_rescan_time.tv_sec) {
					if (find_location_db(s_ab_file, cwin))
						delete_track_db(ab_file, cwin);
					add_entry_db(ab_file,cwin);
				}
			}
			g_free(s_ab_file);
		}

		/* Have to give control to GTK periodically ... */

		while(gtk_events_pending())
			gtk_main_iteration();

		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);
exit:
	if(progress_timeout != 0) {
		g_source_remove(progress_timeout);
		progress_timeout = 0;
	}
}

/* Delete all tracks falling under the given directory.
   Also, flush the database of unused albums, artists, etc. */

void delete_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin)
{
	gchar *query, *sdir_name;

	sdir_name = sanitize_string_sqlite3(dir_name);

	/* Delete all tracks under the given dir */

	query = g_strdup_printf("DELETE FROM TRACK WHERE location IN "
				"(SELECT id FROM LOCATION WHERE NAME LIKE '%s%%');",
				sdir_name);
	exec_sqlite_query(query, cwin, NULL);

	/* Delete the location entries */

	query = g_strdup_printf("DELETE FROM LOCATION WHERE name LIKE '%s%%';",
				sdir_name);
	exec_sqlite_query(query, cwin, NULL);

	/* Delete all entries from PLAYLIST_TRACKS which match given dir */

	query = g_strdup_printf("DELETE FROM PLAYLIST_TRACKS WHERE file LIKE '%s%%';",
				sdir_name);
	exec_sqlite_query(query, cwin, NULL);

	/* Now flush unused artists, albums, genres, years */

	flush_stale_entries_db(cwin);

	g_free(sdir_name);
}

gint init_dbase_schema(struct con_win *cwin)
{
	gchar *query;

	/* Create 'TRACKS' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS TRACK "
				"(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);",
				"location INT PRIMARY KEY",
				"track_no INT",
				"artist INT",
				"album INT",
				"genre INT",
				"year INT",
				"comment INT",
				"bitrate INT",
				"length INT",
				"channels INT",
				"samplerate INT",
				"file_type INT",
				"title VARCHAR(255)");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;

	/* Create 'LOCATION' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS LOCATION "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name TEXT");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;


	/* Create 'ARTIST' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS ARTIST "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name VARCHAR(255)");
	if (!exec_sqlite_query(query,cwin,  NULL))
		return -1;


	/* Create 'ALBUM' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS ALBUM "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name VARCHAR(255)");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;

	/* Create 'GENRE' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS GENRE "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name VARCHAR(255)");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;


	/* Create 'YEAR' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS YEAR "
				"(%s, %s, UNIQUE(year));",
				"id INTEGER PRIMARY KEY",
				"year INT");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;

	/* Create 'COMMENT' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS COMMENT "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name VARCHAR(255)");
	if (!exec_sqlite_query(query,cwin,  NULL))
		return -1;

	/* Create 'PLAYLIST_TRACKS' table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS PLAYLIST_TRACKS "
				"(%s, %s);",
				"file TEXT",
				"playlist INT");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;

	/* Create 'PLAYLIST table */

	query = g_strdup_printf("CREATE TABLE IF NOT EXISTS PLAYLIST "
				"(%s, %s, UNIQUE(name));",
				"id INTEGER PRIMARY KEY",
				"name VARCHAR(255)");
	if (!exec_sqlite_query(query, cwin, NULL))
		return -1;

	return 0;
}

gint drop_dbase_schema(struct con_win *cwin)
{
	gint ret = 0;
	gchar *query;

	query = g_strdup_printf("DROP TABLE ALBUM");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE ARTIST");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE GENRE");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE LOCATION");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE TRACK");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE YEAR");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	query = g_strdup_printf("DROP TABLE COMMENT");
	if (!exec_sqlite_query(query, cwin, NULL))
		ret = -1;

	return ret;
}

gboolean exec_sqlite_query(gchar *query, struct con_win *cwin,
			   struct db_result *result)
{
	gchar *err = NULL;
	gboolean ret = FALSE;

	if (!query)
		return FALSE;

	CDEBUG(DBG_DB, "%s", query);

	/* Caller doesn't expect any result */

	if (!result) {
		sqlite3_exec(cwin->cdbase->db, query, NULL, NULL, &err);
		if (err) {
			g_critical("SQL Err : %s",  err);
			g_critical("query   : %s", query);
			ret = FALSE;
		} else {
			ret = TRUE;
		}
		sqlite3_free(err);
	}

	/* Caller expects result */

	else {
		sqlite3_get_table(cwin->cdbase->db, query,
				  &result->resultp,
				  &result->no_rows,
				  &result->no_columns,
				  &err);
		if (err) {
			g_critical("SQL Err : %s",  err);
			g_critical("query   : %s", query);
			ret = FALSE;
		}
		else {
			ret = TRUE;
		}
		sqlite3_free(err);
	}

	/* Free the query here, don't free in the callsite ! */

	g_free(query);

	return ret;
}
