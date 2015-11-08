/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-dnd.h"

#include <stdlib.h>
#include <string.h>

#include "pragha-musicobject-mgmt.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-debug.h"

GList *
pragha_dnd_library_get_mobj_list (GtkSelectionData *data, PraghaDatabase *cdbase)
{
	gint n = 0, location_id = 0;
	gchar *name = NULL, *uri, **uris;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;

	CDEBUG(DBG_VERBOSE, "Dnd: Library");

	uris = g_uri_list_extract_uris ((const gchar *) gtk_selection_data_get_data (data));
	if (!uris) {
		g_warning("No selections to process in DnD");
		return list;
	}

	/* Dnd from the library, so will read everything from database. */

	pragha_database_begin_transaction (cdbase);

	/* Get the mobjs from the path of the library. */

	for (n = 0; uris[n] != NULL; n++) {
		uri = uris[n];
		if (g_str_has_prefix(uri, "Location:/")) {
			location_id = atoi(uri + strlen("Location:/"));
			mobj = new_musicobject_from_db (cdbase, location_id);
			if (G_LIKELY(mobj))
				list = g_list_prepend(list, mobj);
		}
		else if(g_str_has_prefix(uri, "Playlist:/")) {
			name = uri + strlen("Playlist:/");
			list = add_playlist_to_mobj_list (cdbase, name, list);
		}
		else if(g_str_has_prefix(uri, "Radio:/")) {
			name = uri + strlen("Radio:/");
			list = add_radio_to_mobj_list (cdbase, name, list);
		}
	}
	pragha_database_commit_transaction (cdbase);

	g_strfreev(uris);

	return g_list_reverse (list);
}

GList *
pragha_dnd_uri_list_get_mobj_list (GtkSelectionData *data)
{
	PraghaMusicobject *mobj = NULL;
	gchar **uris = NULL, *filename = NULL;
	GList *list = NULL;
	gint i = 0;

	CDEBUG(DBG_VERBOSE, "Target: URI_LIST");

	uris = gtk_selection_data_get_uris(data);

	if(uris){
		for(i = 0; uris[i] != NULL; i++) {
			filename = g_filename_from_uri(uris[i], NULL, NULL);
			if (g_file_test(filename, G_FILE_TEST_IS_DIR)){
				list = append_mobj_list_from_folder(list, filename);
			}
			else {
				mobj = new_musicobject_from_file(filename, NULL);
				if (G_LIKELY(mobj))
					list = g_list_prepend(list, mobj);
			}

			/* Have to give control to GTK periodically ... */
			pragha_process_gtk_events ();

			g_free(filename);
		}
		g_strfreev(uris);
	}

	return g_list_reverse (list);
}

GList *
pragha_dnd_plain_text_get_mobj_list (GtkSelectionData *data)
{
	PraghaMusicobject *mobj = NULL;
	gchar *filename = NULL;
	GList *list = NULL;

	CDEBUG(DBG_VERBOSE, "Target: PLAIN_TEXT");

	filename = (gchar*)gtk_selection_data_get_text(data);

	if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
		list = append_mobj_list_from_folder(list, filename);
	}
	else {
		mobj = new_musicobject_from_file(filename, NULL);
		if (G_LIKELY(mobj))
			list = g_list_prepend(list, mobj);

		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();
	}
	g_free(filename);

	return g_list_reverse (list);
}
