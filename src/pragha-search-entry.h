/*
 * Copyright (C) 2009-2013 matias <mati86dl@gmail.com>
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

#ifndef PRAGHA_SEARCH_ENTRY_H
#define PRAGHA_SEARCH_ENTRY_H

#include <glib.h>
#include <gtk/gtk.h>

#include "pragha-preferences.h"

G_BEGIN_DECLS

GtkWidget * pragha_search_entry_new (PraghaPreferences *preferences);

G_END_DECLS

#endif /* PRAGHA_SEARCH_ENTRY_H */
