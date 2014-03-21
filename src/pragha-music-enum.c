/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#include "pragha-music-enum.h"

struct _PraghaMusicEnumItem {
    gchar *name;
    gint   code;
};
typedef struct _PraghaMusicEnumItem PraghaMusicEnumItem;

PraghaMusicEnumItem enum_map[MAX_ENUM_SIZE];
gint enum_size = 0;

const gchar *
pragha_music_enum_map_get_name (gint enum_code)
{
	return enum_map[enum_code].name;
}

gint
pragha_music_enum_map_get (const gchar *name)
{
	gint i = 0;

	/* First check if exist */
	for (i = 0; i <= enum_size; i++) {
		if (enum_map[i].name == NULL)
			continue;
		if (g_ascii_strcasecmp(name, enum_map[i].name) == 0)
			return enum_map[i].code;
	}
	/* Add a new enum */
	for (i = 0; i <= enum_size; i++) {
		if (enum_map[i].name == NULL) {
			enum_map[i].name = g_strdup(name);
			return enum_map[i].code;
		}
	}
	return -1;
}

gint
pragha_music_enum_map_remove (const gchar *name)
{
	gint i = 0;

	for (i = 0; i <= enum_size; i++) {
		if (enum_map[i].name == NULL)
			continue;
		if (g_ascii_strcasecmp (name, enum_map[i].name) == 0) {
			g_free (enum_map[i].name);
			enum_map[i].name = NULL;

			return enum_map[i].code;
		}
	}
	return -1;
}

void
pragha_music_enum_map_free (void)
{
	gint i = 0;
	for (i = 0; i <= enum_size; i++) {
		if (enum_map[i].name == NULL)
			continue;
		g_free (enum_map[i].name);
	}
}

void
pragha_music_enum_map_init (gint min_enum, gint max_enum)
{
	gint i = 0, code = 0;

	/* Set size */
	enum_size = max_enum - min_enum;
	if (enum_size > MAX_ENUM_SIZE)
		enum_size = MAX_ENUM_SIZE;

	for (i = 0, code = min_enum; i <= enum_size; i++, code++) {
		enum_map[i].name = NULL;
		if (i <= enum_size)
			enum_map[i].code = code;
		else
			enum_map[i].code = -1;
	}
}

static void
pragha_music_enum_map_print (void)
{
	gint i = 0;

	for (i = 0; i <= enum_size; i++)
		g_print("NAME: %s, ENUM: %d\n", enum_map[i].name, enum_map[i].code);
}

void
test_pragha_music_enum_map(void)
{
	gint ret = 0;

	g_print("MUSIC ENUM TEST\n");
	g_print("INIT 1 to 5\n");
	pragha_music_enum_map_init (1, 5);

	g_print("Add CDDA\n");
	ret = pragha_music_enum_map_get ("Cdda");
	g_print("RETURN %d\n\n", ret);

	g_print("Add MTP\n");
	ret = pragha_music_enum_map_get ("Mtp");
	g_print("RETURN %d\n\n", ret);

	g_print("Add TEST\n");
	ret = pragha_music_enum_map_get ("TEST");
	g_print("RETURN %d\n\n", ret);

	g_print("Add DVD\n");
	ret = pragha_music_enum_map_get ("DVD");
	g_print("RETURN %d\n\n", ret);

	g_print("Add MTP?\n");
	ret = pragha_music_enum_map_get ("Mtp");
	g_print("RETURN %d\n\n", ret);

	g_print("Current enum map\n");
	pragha_music_enum_map_print ();

	g_print("Remove TEST?\n");
	ret = pragha_music_enum_map_remove ("TEST");
	g_print("RETURN %d\n\n", ret);

	g_print("Current enum map\n");
	pragha_music_enum_map_print ();

	g_print("Remove TEST?\n");
	ret = pragha_music_enum_map_remove ("TEST");
	g_print("RETURN %d\n\n", ret);

	g_print("Current enum map\n");
	pragha_music_enum_map_print ();

	g_print("Add GLY?\n");
	ret = pragha_music_enum_map_get ("Gly");
	g_print("RETURN %d\n\n", ret);

	g_print("Add TXT?\n");
	ret = pragha_music_enum_map_get ("Txt");
	g_print("RETURN %d\n\n", ret);

	g_print("Current enum map\n");
	pragha_music_enum_map_print ();

	g_print("Add TXT2?\n");
	ret = pragha_music_enum_map_get ("Txt2");
	g_print("RETURN %d\n\n", ret);

	g_print("Current enum map\n");
	pragha_music_enum_map_print ();
}
