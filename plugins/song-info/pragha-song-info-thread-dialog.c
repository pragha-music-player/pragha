/*************************************************************************/
/* Copyright (C) 2011-2019 matias <mati86dl@gmail.com>                   */
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
#include "pragha-song-info-dialog.h"
#include "pragha-song-info-pane.h"

#include "src/pragha-app-notification.h"
#include "src/pragha-simple-async.h"
#include "src/pragha-utils.h"

typedef struct {
	PraghaSongInfoPlugin *plugin;
	GlyrQuery             query;
	GlyrMemCache         *head;
} glyr_struct;

static void
glyr_finished_successfully (glyr_struct *glyr_info)
{
	PraghaApplication *pragha;
	GtkWidget *window;
	gchar *title_header = NULL, *subtitle_header = NULL;

	pragha = pragha_songinfo_plugin_get_application (glyr_info->plugin);

	switch (glyr_info->head->type) {
		case GLYR_TYPE_LYRICS:
			window = pragha_application_get_window (pragha);
			title_header =  g_strdup_printf(_("Lyrics thanks to %s"), glyr_info->head->prov);
			subtitle_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), glyr_info->query.title, glyr_info->query.artist);

			pragha_show_related_text_info_dialog (window, title_header, subtitle_header, glyr_info->head->data);
			break;
		case GLYR_TYPE_ARTIST_BIO:
			window = pragha_application_get_window (pragha);
			title_header =  g_strdup_printf(_("Artist info"));
			subtitle_header = g_strdup_printf(_("%s <small><span weight=\"light\">thanks to</span></small> %s"), glyr_info->query.artist, glyr_info->head->prov);

			pragha_show_related_text_info_dialog (window, title_header, subtitle_header, glyr_info->head->data);
			break;
		case GLYR_TYPE_COVERART:
		default:
			break;
	}

	g_free(title_header);
	g_free(subtitle_header);

	glyr_free_list(glyr_info->head);
}

static void
glyr_finished_incorrectly(glyr_struct *glyr_info)
{
	PraghaAppNotification *notification;

	switch (glyr_info->query.type) {
		case GLYR_GET_LYRICS:
			notification = pragha_app_notification_new (_("Lyrics"), _("Lyrics not found."));
			pragha_app_notification_show (notification);
			break;
		case GLYR_GET_ARTIST_BIO:
			notification = pragha_app_notification_new (_("Artist info"), _("Artist information not found."));
			pragha_app_notification_show (notification);
			break;
		case GLYR_GET_COVERART:
		default:
			break;
	}
}

/*
 * Final threads
 */

static gboolean
glyr_finished_thread_update (gpointer data)
{
	PraghaApplication *pragha;
	GtkWidget *window;

	glyr_struct *glyr_info = data;

	pragha = pragha_songinfo_plugin_get_application (glyr_info->plugin);
	window = pragha_application_get_window (pragha);
	remove_watch_cursor (window);

	if(glyr_info->head != NULL)
		glyr_finished_successfully (glyr_info);
	else
		glyr_finished_incorrectly (glyr_info);

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
pragha_songinfo_plugin_get_info_to_dialog (PraghaSongInfoPlugin *plugin,
                                           GLYR_GET_TYPE        type,
                                           const gchar          *artist,
                                           const gchar          *title)
{
	PraghaApplication *pragha;
	GtkWidget *window;
	glyr_struct *glyr_info;

	glyr_info = g_slice_new0 (glyr_struct);

	pragha_songinfo_plugin_init_glyr_query (&glyr_info->query);
	glyr_opt_type (&glyr_info->query, type);

	switch (type) {
		case GLYR_GET_ARTIST_BIO:
			glyr_opt_artist(&glyr_info->query, artist);

			glyr_opt_lang (&glyr_info->query, "auto");
			glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
			break;
		case GLYR_GET_LYRICS:
			glyr_opt_artist(&glyr_info->query, artist);
			glyr_opt_title(&glyr_info->query, title);
			break;
		default:
			break;
	}

	glyr_info->plugin = plugin;

	pragha = pragha_songinfo_plugin_get_application (plugin);
	window = pragha_application_get_window (pragha);
	set_watch_cursor (window);

	pragha_async_launch (get_related_info_idle_func,
	                     glyr_finished_thread_update,
	                     glyr_info);
}

