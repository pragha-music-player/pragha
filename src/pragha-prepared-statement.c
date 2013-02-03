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

#include <sqlite3.h>
#include "pragha-database.h"
#include "pragha-prepared-statement.h"

struct PraghaPreparedStatement {
	sqlite3_stmt *stmt;
	PraghaDatabase *database;
};

PraghaPreparedStatement *
pragha_prepared_statement_new (gpointer stmt, gpointer database)
{
	PraghaPreparedStatement *statement = g_slice_new (PraghaPreparedStatement);
	statement->stmt = stmt;
	statement->database = g_object_ref (database);
	return statement;
}

void
pragha_prepared_statement_free (PraghaPreparedStatement *statement)
{
	sqlite3_finalize (statement->stmt);
	g_object_unref (statement->database);
	g_slice_free (PraghaPreparedStatement, statement);
}

static void
on_sqlite_error (PraghaPreparedStatement *statement)
{
	g_critical ("db: %s", pragha_database_get_last_error (statement->database));
}

void
pragha_prepared_statement_bind_string (PraghaPreparedStatement *statement, gint n, const gchar *value)
{
	if (sqlite3_bind_text (statement->stmt, n, value, -1, SQLITE_TRANSIENT) != SQLITE_OK)
		on_sqlite_error (statement);
}

void
pragha_prepared_statement_bind_int (PraghaPreparedStatement *statement, gint n, gint value)
{
	if (sqlite3_bind_int (statement->stmt, n, value) != SQLITE_OK)
		on_sqlite_error (statement);
}


gboolean
pragha_prepared_statement_step (PraghaPreparedStatement *statement)
{
	int error_code = sqlite3_step (statement->stmt);

	if (error_code != SQLITE_OK && error_code != SQLITE_ROW && error_code != SQLITE_DONE) {
		on_sqlite_error (statement);
	}

	return error_code == SQLITE_ROW;
}

gint
pragha_prepared_statement_get_int (PraghaPreparedStatement *statement, gint column)
{
	return sqlite3_column_int (statement->stmt, column);
}

const gchar *
pragha_prepared_statement_get_string (PraghaPreparedStatement *statement, gint column)
{
	return (const gchar *) sqlite3_column_text (statement->stmt, column);
}
