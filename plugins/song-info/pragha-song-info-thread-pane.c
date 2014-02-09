/*************************************************************************/
/* Copyright (C) 2011-2014 matias <mati86dl@gmail.com>                   */
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
	GCancellable         *cancellable;
	gulong                cancel_id;
	gchar                *filename;
	GlyrQuery             query;
	GlyrMemCache         *head;
} glyr_struct;


/*
 * Function to check if has the last
 */

static gboolean
glyr_finished_thread_is_current_song (PraghaSongInfoPlugin *plugin, const gchar *filename)
{
	PraghaApplication *pragha;
	PraghaBackend *backend;
	PraghaMusicobject *mobj;
	const gchar *current_filename = NULL;

	pragha = pragha_songinfo_plugin_get_application (plugin);

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return FALSE;

	mobj = pragha_backend_get_musicobject (backend);
	current_filename = pragha_musicobject_get_file (mobj);

	if (g_ascii_strcasecmp(filename, current_filename))
		return FALSE;

	return TRUE;
}

/*
 * Threads
 */

static void
glyr_finished_successfully_pane (glyr_struct *glyr_info)
{
	PraghaSonginfoPane *pane;

	switch (glyr_info->head->type) {
		case GLYR_TYPE_LYRICS:
			pane = pragha_songinfo_plugin_get_pane (glyr_info->plugin);
			pragha_songinfo_pane_set_text (pane, glyr_info->query.title, glyr_info->head->data, glyr_info->head->prov);
			break;
		case GLYR_TYPE_ARTIST_BIO:
			pane = pragha_songinfo_plugin_get_pane (glyr_info->plugin);
			pragha_songinfo_pane_set_text (pane,glyr_info->query.artist, glyr_info->head->data, glyr_info->head->prov);
			break;
		case GLYR_TYPE_COVERART:
		default:
			break;
	}
}

static void
glyr_finished_incorrectly_pane (glyr_struct *glyr_info)
{
	PraghaSonginfoPane *pane;

	switch (glyr_info->query.type) {
		case GLYR_GET_LYRICS:
			pane = pragha_songinfo_plugin_get_pane (glyr_info->plugin);
			pragha_songinfo_pane_set_text (pane, glyr_info->query.title, _("Lyrics not found."), "");
			break;
		case GLYR_GET_ARTIST_BIO:
			pane = pragha_songinfo_plugin_get_pane (glyr_info->plugin);
			pragha_songinfo_pane_set_text (pane, glyr_info->query.artist, _("Artist information not found."), "");
			break;
		case GLYR_GET_COVERART:
		default:
			break;
	}
}

static gboolean
glyr_finished_thread_update_pane (gpointer data)
{
	glyr_struct *glyr_info = data;

	if (g_cancellable_is_cancelled (glyr_info->cancellable))
		goto old_thread;

	if (!glyr_finished_thread_is_current_song(glyr_info->plugin, glyr_info->filename))
		goto old_thread;

	if (glyr_info->head != NULL)
		glyr_finished_successfully_pane (glyr_info);
	else
		glyr_finished_incorrectly_pane (glyr_info);

old_thread:
	g_cancellable_disconnect (glyr_info->cancellable, glyr_info->cancel_id);
	g_object_unref (glyr_info->cancellable);

	if (glyr_info->head != NULL)
		glyr_free_list (glyr_info->head);

	glyr_query_destroy (&glyr_info->query);
	g_free (glyr_info->filename);

	g_slice_free (glyr_struct, glyr_info);

	return FALSE;
}

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

static void
search_cancelled (GCancellable *cancellable, gpointer user_data)
{
	GlyrQuery *query = user_data;
	glyr_signal_exit (query);
}

GCancellable *
pragha_songinfo_plugin_get_info_to_pane (PraghaSongInfoPlugin *plugin,
                                         GLYR_GET_TYPE        type,
                                         const gchar          *artist,
                                         const gchar          *title,
                                         const gchar          *filename)
{
	PraghaSonginfoPane *pane;
	GlyrDatabase *cache_db;
	glyr_struct *glyr_info;

	glyr_info = g_slice_new0 (glyr_struct);

	glyr_query_init (&glyr_info->query);
	glyr_opt_type (&glyr_info->query, type);

	pane = pragha_songinfo_plugin_get_pane (plugin);

	switch (type) {
		case GLYR_GET_ARTIST_BIO:
			pragha_songinfo_pane_set_text (pane, artist, _("Searching..."), "");

			glyr_opt_artist(&glyr_info->query, artist);

			glyr_opt_lang (&glyr_info->query, "auto");
			glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
			break;
		case GLYR_GET_LYRICS:
			pragha_songinfo_pane_set_text (pane, title, _("Searching..."), "");

			glyr_opt_artist(&glyr_info->query, artist);
			glyr_opt_title(&glyr_info->query, title);
			break;
		default:
			break;
	}

	cache_db = pragha_songinfo_plugin_get_cache (plugin);

	glyr_opt_lookup_db (&glyr_info->query, cache_db);
	glyr_opt_db_autowrite (&glyr_info->query, TRUE);

	glyr_info->filename = g_strdup(filename);
	glyr_info->plugin = plugin;

	GCancellable *cancellable = g_cancellable_new ();
	glyr_info->cancellable = g_object_ref (cancellable);
	glyr_info->cancel_id = g_cancellable_connect (glyr_info->cancellable,
	                                              G_CALLBACK (search_cancelled),
	                                              &glyr_info->query,
	                                              NULL);

	pragha_async_launch (get_related_info_idle_func,
	                     glyr_finished_thread_update_pane,
	                     glyr_info);

	return cancellable;
}

