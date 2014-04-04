/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2011 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_DEBUG_H
#define PRAGHA_DEBUG_H

#include <glib.h>

enum debug_level {
	DBG_BACKEND = 1,
	DBG_INFO,
	DBG_PLUGIN,
	DBG_MOBJ,
	DBG_DB,
	DBG_VERBOSE,
};

extern gint debug_level;

#define CDEBUG(_lvl, _fmt, ...)			\
	if (G_UNLIKELY(_lvl <= debug_level))	\
		g_debug(_fmt, ##__VA_ARGS__);

void
pragha_log_to_file (const gchar* log_domain,
                    GLogLevelFlags log_level,
                    const gchar* message,
                    gpointer user_data);

#endif /* PRAGHA_DEBUG_H */