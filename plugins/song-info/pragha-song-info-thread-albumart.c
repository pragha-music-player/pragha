/*************************************************************************/
/* Copyright (C) 2011-2018 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glyr/glyr.h>

#include "pragha-song-info-plugin.h"
#include "pragha-song-info-pane.h"

#include "src/pragha-simple-async.h"

typedef struct {
	PraghaSongInfoPlugin *plugin;
	GlyrQuery             query;
	GlyrMemCache         *head;
} glyr_struct;

/* Save the downloaded album art in cache, and updates the gui.*/

static void
glyr_finished_successfully (glyr_struct *glyr_info)
{
	PraghaApplication *pragha;
	PraghaArtCache *art_cache;
	const gchar *artist = NULL, *album = NULL;

	pragha = pragha_songinfo_plugin_get_application (glyr_info->plugin);

	artist = glyr_info->query.artist;
	album = glyr_info->query.album;

	art_cache = pragha_application_get_art_cache (pragha);

	if (glyr_info->head->data)
		pragha_art_cache_put_album (art_cache, artist, album, glyr_info->head->data, glyr_info->head->size);

	glyr_free_list(glyr_info->head);
}

/*
 * Final threads
 */

static gboolean
glyr_finished_thread_update (gpointer data)
{
	glyr_struct *glyr_info = data;

	if (glyr_info->head != NULL)
		glyr_finished_successfully (glyr_info);

	glyr_query_destroy (&glyr_info->query);
	g_slice_free (glyr_struct, glyr_info);

	return FALSE;
}

/* Get artist bio or lyric on a thread. */

static gpointer
get_related_info_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	head = glyr_get (&glyr_info->query, &error, NULL);

	glyr_info->head = head;

	return glyr_info;
}

void
pragha_songinfo_plugin_get_album_art (PraghaSongInfoPlugin *plugin,
                                      const gchar          *artist,
                                      const gchar          *album)
{
	glyr_struct *glyr_info;

	CDEBUG(DBG_INFO, "Get album art handler");

	glyr_info = g_slice_new0 (glyr_struct);

	pragha_songinfo_plugin_init_glyr_query(&glyr_info->query);

	glyr_opt_type (&glyr_info->query, GLYR_GET_COVERART);
	glyr_opt_from (&glyr_info->query, "lastfm;musicbrainz");

	glyr_opt_artist (&glyr_info->query, artist);
	glyr_opt_album (&glyr_info->query, album);

	glyr_info->plugin = plugin;

	pragha_async_launch (get_related_info_idle_func,
	                     glyr_finished_thread_update,
	                     glyr_info);
}

