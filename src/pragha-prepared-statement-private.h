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

#ifndef PRAGHA_PREPARED_STATEMENT_PRIVATE_H
#define PRAGHA_PREPARED_STATEMENT_PRIVATE_H

#include <sqlite3.h>

#include "pragha-database.h"
#include "pragha-prepared-statement.h"

PraghaPreparedStatement* pragha_prepared_statement_new               (sqlite3_stmt *stmt, PraghaDatabase *database);
void                     pragha_prepared_statement_finalize          (PraghaPreparedStatement *statement);

#endif /* PRAGHA_PREPARED_STATEMENT_PRIVATE_H */
