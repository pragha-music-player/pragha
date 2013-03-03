/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
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

#ifndef PRAGHA_HIG_ASYNC_H
#define PRAGHA_HIG_ASYNC_H

#include "pragha.h"

typedef struct {
	GtkWidget *filter_view;
	GtkTreeModel *filter_model;
	gchar *filter_string;
	guint timeout_id;
	PraghaPlaylist *cplaylist;
	PraghaPreferences *preferences;
} PraghaFilterDialog;

void
pragha_filter_dialog (struct con_win *cwin);

#endif /* PRAGHA_SIMPLE_ASYNC_H */