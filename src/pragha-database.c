/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>			 */
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

#include "pragha-database.h"
#include "pragha.h"

G_DEFINE_TYPE(PraghaDatabase, pragha_database, G_TYPE_OBJECT)

struct _PraghaDatabasePrivate
{
	sqlite3 *sqlitedb;
	gboolean successfully;
};

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

gboolean
pragha_database_exec_sqlite_query(PraghaDatabase *database,
                                  gchar *query,
                                  struct db_result *result)
{
	gchar *err = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(PRAGHA_IS_DATABASE(database), FALSE);

	if (!query)
		return FALSE;

	CDEBUG(DBG_INFO, "%s", query);

	/* Caller doesn't expect any result */

	if (!result) {
		ret = pragha_database_exec_query(database, query);
	}

	/* Caller expects result */

	else {
		sqlite3_get_table(database->priv->sqlitedb, query,
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

static void
pragha_database_finalize (GObject *object)
{
	PraghaDatabase *database = PRAGHA_DATABASE(object);
	PraghaDatabasePrivate *priv = database->priv;

	sqlite3_close(priv->sqlitedb);

	G_OBJECT_CLASS(pragha_database_parent_class)->finalize(object);
}

static void
pragha_database_class_init (PraghaDatabaseClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_database_finalize;
	g_type_class_add_private(object_class, sizeof(PraghaDatabasePrivate));
}

static void
pragha_database_init (PraghaDatabase *database)
{
	gint ret, i;
	gchar *database_file;
	const gchar *home;

	database->priv = G_TYPE_INSTANCE_GET_PRIVATE(database,
	                                             PRAGHA_TYPE_DATABASE,
	                                             PraghaDatabasePrivate);

	PraghaDatabasePrivate *priv = database->priv;

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

	const gchar *queries[] = {
		"PRAGMA synchronous=OFF",

		"CREATE TABLE IF NOT EXISTS TRACK "
			"(location INT PRIMARY KEY,"
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
			return;
	}

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
