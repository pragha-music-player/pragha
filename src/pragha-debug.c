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

#include "pragha-debug.h"

#include <stdio.h>

/* Function to save debug on file. */

void
pragha_log_to_file (const gchar* log_domain,
                    GLogLevelFlags log_level,
                    const gchar* message,
                    gpointer user_data)
{
	FILE* logfile = fopen ((const char*)user_data, "a");
	gchar* level_name = "";

	switch (log_level)
	{
	/* skip irrelevant flags */
	case G_LOG_LEVEL_MASK:
	case G_LOG_FLAG_FATAL:
	case G_LOG_FLAG_RECURSION:
	case G_LOG_LEVEL_ERROR:
		level_name = "ERROR";
		break;
	case G_LOG_LEVEL_CRITICAL:
		level_name = "CRITICAL";
		break;
	case G_LOG_LEVEL_WARNING:
		level_name = "WARNING";
		break;
	case G_LOG_LEVEL_MESSAGE:
		level_name = "MESSAGE";
		break;
	case G_LOG_LEVEL_INFO:
		level_name = "INFO";
		break;
	case G_LOG_LEVEL_DEBUG:
		level_name = "DEBUG";
		break;
	}

	fprintf (logfile, "%s %s: %s\n",
	log_domain ? log_domain : "Pragha", level_name, message);
	fclose (logfile);
}
