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

#ifndef PRAGHA_PREPARED_STATEMENT_H
#define PRAGHA_PREPARED_STATEMENT_H

#include <glib.h>

G_BEGIN_DECLS

struct PraghaPreparedStatement;
typedef struct PraghaPreparedStatement PraghaPreparedStatement;

PraghaPreparedStatement* pragha_prepared_statement_new               (gpointer stmt, gpointer database);
void                     pragha_prepared_statement_free              (PraghaPreparedStatement *statement);
void                     pragha_prepared_statement_bind_string       (PraghaPreparedStatement *statement, gint n, const gchar *value);
void                     pragha_prepared_statement_bind_int          (PraghaPreparedStatement *statement, gint n, gint value);
gboolean                 pragha_prepared_statement_step              (PraghaPreparedStatement *statement);
gint                     pragha_prepared_statement_get_int           (PraghaPreparedStatement *statement, gint column);
const gchar *            pragha_prepared_statement_get_string        (PraghaPreparedStatement *statement, gint column);

G_END_DECLS

#endif /* PRAGHA_PREPARED_STATEMENT_H */
