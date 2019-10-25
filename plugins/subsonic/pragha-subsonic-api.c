/*************************************************************************/
/* Copyright (C) 2019 matias <mati86dl@gmail.com>                        */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef HAVE_GRILO_NET3
#include <grilo-0.3/net/grl-net.h>
#endif
#ifdef HAVE_GRILO_NET2
#include <grilo-0.2/net/grl-net.h>
#endif

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "src/pragha-musicobject.h"
#include "src/pragha-utils.h"

#include "pragha-subsonic-api.h"


/*
 * Forward declarations
 */

static void
pragha_subsonic_api_get_album (PraghaSubsonicApi *subsonic,
                               const gchar       *album_id);

static void
pragha_subsonic_api_get_albums_queue (PraghaSubsonicApi *subsonic);


/*
 * PraghaSubsonicApi *
 */

struct _PraghaSubsonicApi {
	GObject      _parent;

	GrlNetWc     *glrnet;
	GCancellable *cancellable;

	gchar        *server;
	gchar        *username;
	gchar        *password;

	GQueue       *albums_queue;
	guint         albums_count;
	guint         albums_offset;

	guint         songs_count;
	GSList       *songs_list;

	gboolean      authenticated;
	gboolean      has_connection;
	gboolean      scanning;
};

enum {
	SIGNAL_AUTH_DONE,
	SIGNAL_PING_DONE,
	SIGNAL_SCAN_DONE,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaSubsonicApi, pragha_subsonic_api, G_TYPE_OBJECT)


/*
 *  Utils
 */

static void
pragha_subsonic_api_add_query_item (GString     *url,
                                    const gchar *parameter,
                                    const gchar *value)
{
	g_string_append_printf (url, "%s=%s&", parameter, value);
}

static GString *
pragha_subsonic_api_build_url (PraghaSubsonicApi *subsonic,
                               const gchar       *method)
{
	GString *url = g_string_new (subsonic->server);

	g_string_append_printf (url, "/rest/%s.view?", method);
	pragha_subsonic_api_add_query_item(url, "u", subsonic->username);
	pragha_subsonic_api_add_query_item(url, "p", subsonic->password);

	return url;
}

static gchar *
pragha_subsonic_api_close_url (GString *url)
{
	g_string_append_printf (url, "%s=%s&", "v", "1.11.0");
	g_string_append_printf (url, "%s=%s", "c", "Pragha");

	return g_string_free (url, FALSE);
}

static gchar *
pragha_subsonic_api_get_friendly_url (const gchar *server,
                                      const gchar *artist,
                                      const gchar *album,
                                      const gchar *title,
                                      const gchar *song_id)
{
	gchar *url = NULL;
	url = g_strdup_printf("%s/%s/%s/%s - %s", server,
		string_is_not_empty(artist) ? artist : _("Unknown Artist"),
		string_is_not_empty(album)  ? album  : _("Unknown Album"),
		song_id,
		string_is_not_empty(title)  ? title  : _("Unknown"));
	return url;
}

gchar *
pragha_subsonic_api_get_playback_url (PraghaSubsonicApi *subsonic,
                                      const gchar       *friendly_url)
{
	GString *url;
	gchar **url_split = NULL, *filename = NULL, *song_id = NULL;

	url_split = g_strsplit(friendly_url + strlen(subsonic->server), "/", -1);
	filename = g_strdup(url_split[3]);
	g_strfreev (url_split);

	url_split = g_strsplit(filename, " - ", -1);
	song_id = g_strdup(url_split[0]);
	g_strfreev (url_split);

	url = pragha_subsonic_api_build_url (subsonic, "stream");
	pragha_subsonic_api_add_query_item (url, "id", song_id);

	g_free (filename);
	g_free (song_id);

	return pragha_subsonic_api_close_url (url);
}

/*
 * Subsonic API handlers
 */

static void
pragha_subsonic_api_ping_done (GObject      *object,
                               GAsyncResult *res,
                               gpointer      user_data)
{
	GError *wc_error = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	gboolean has_connection = TRUE;
	gchar *content = NULL;
	SubsonicStatusCode code = S_GENERIC_OK;

	PraghaSubsonicApi *subsonic = PRAGHA_SUBSONIC_API (user_data);

	if (!grl_net_wc_request_finish (GRL_NET_WC (object),
	                                res,
	                                &content,
	                                NULL,
	                                &wc_error))
	{

		if (g_cancellable_is_cancelled (subsonic->cancellable)) {
			code = S_USER_CANCELLED;
			g_cancellable_reset (subsonic->cancellable);
		}
		else {
			code = S_GENERIC_ERROR;
			has_connection = FALSE;
			g_warning ("Failed to connect to subsonic server: %s", wc_error->message);
		}
	}

	if (content)
	{
		doc = xmlReadMemory (content, strlen(content), NULL, NULL,
		                     XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);

		node = xmlDocGetRootElement (doc);

		node = node->xmlChildrenNode;
		while(node)
		{
			if (!xmlStrcmp (node->name, (const xmlChar *) "error"))
			{
				code = atoi (xmlGetProp(node, "code"));
				has_connection = FALSE;
				g_warning("PING ERROR: %s %s", xmlGetProp(node, "code"), xmlGetProp(node, "message"));
			}
			node = node->next;
		}

		xmlFreeDoc (doc);
	}

	if (!subsonic->authenticated) {
		subsonic->authenticated = (code != S_GENERIC_OK);
		g_signal_emit (subsonic, signals[SIGNAL_AUTH_DONE], 0, code);
	}

	if (subsonic->has_connection != has_connection) {
		subsonic->has_connection = has_connection;
		g_signal_emit (subsonic, signals[SIGNAL_PING_DONE], 0, code);
	}
}

static void
pragha_subsonic_api_get_albums_done (GObject      *object,
                                     GAsyncResult *res,
                                     gpointer      user_data)
{
	GError *wc_error = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	guint albums_count = 0;
	gchar *content = NULL;
	gchar *album_id = NULL;
	SubsonicStatusCode code = S_GENERIC_OK;

	PraghaSubsonicApi *subsonic = PRAGHA_SUBSONIC_API (user_data);

	if (!grl_net_wc_request_finish (GRL_NET_WC (object),
	                                res,
	                                &content,
	                                NULL,
	                                &wc_error))
	{
		if (g_cancellable_is_cancelled (subsonic->cancellable)) {
			code = S_USER_CANCELLED;
			subsonic->scanning = FALSE;
			g_cancellable_reset (subsonic->cancellable);
		}
		else {
			code = S_GENERIC_ERROR;
			subsonic->scanning = FALSE;
			g_warning ("Failed to get albums from subsonic server: %s", wc_error->message);
		}
	}

	if (content)
	{
		doc = xmlReadMemory (content, strlen(content), NULL, NULL,
		                     XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);

		node = xmlDocGetRootElement (doc);

		node = node->xmlChildrenNode;
		if (node)
			node = node->xmlChildrenNode;

		while(node)
		{
			if (!xmlStrcmp (node->name, (const xmlChar *) "album"))
			{
				const gchar *id = xmlGetProp(node, "id");
				if (string_is_not_empty (id)) {
					g_queue_push_head (subsonic->albums_queue, g_strdup(id));
					albums_count++;
				}
			}
			else {
				g_critical ("Remove these warning: Unknown node: %s", node->name);
			}
			node = node->next;
		}

		xmlFreeDoc (doc);
	}

	if (code != S_GENERIC_OK) {
		g_warning ("Remove these warning: Subsonic scan finished due error or user interaction.. ");
		g_signal_emit (subsonic, signals[SIGNAL_SCAN_DONE], 0, code);
		return;
	}

	if (albums_count > 0) {
		subsonic->albums_count += albums_count;
		g_warning ("Remove these warning: Subsonic response %i albums...", subsonic->albums_count);

		pragha_subsonic_api_get_albums_queue (subsonic);
	}
	else if (albums_count == 0 && subsonic->albums_count > 0) {
		g_warning ("Remove these warning: Subsonic finish obtaining albums. Now look these songs.");

		album_id = g_queue_pop_head(subsonic->albums_queue);
		pragha_subsonic_api_get_album (subsonic, album_id);
	}
	else {
		g_warning ("Remove these warning: Subsonic dont reports any album...");

		subsonic->scanning = FALSE;
		g_signal_emit (subsonic, signals[SIGNAL_SCAN_DONE], 0, code);
	}
}

void
pragha_subsonic_api_get_albums_queue (PraghaSubsonicApi *subsonic)
{
	GString *url;
	gchar *urlc = NULL, *offsetc = NULL;

	subsonic->albums_offset += 250;
	offsetc = g_strdup_printf("%i",  subsonic->albums_offset);

	url = pragha_subsonic_api_build_url (subsonic, "getAlbumList2");
	pragha_subsonic_api_add_query_item (url, "type", "alphabeticalByName");
	pragha_subsonic_api_add_query_item (url, "size", "250");
	pragha_subsonic_api_add_query_item (url, "offset", offsetc);
	urlc = pragha_subsonic_api_close_url (url);

	g_warning ("Remove these warning: Albums url: %s", urlc);

	grl_net_wc_request_async (subsonic->glrnet,
	                          urlc,
	                          subsonic->cancellable,
	                          pragha_subsonic_api_get_albums_done,
	                          subsonic);

	g_free (offsetc);
	g_free(urlc);
}


static void
pragha_subsonic_api_get_album_done (GObject      *object,
                                    GAsyncResult *res,
                                    gpointer      user_data)
{
	PraghaMusicobject *mobj = NULL;
	GError *wc_error = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	guint songs_count = 0;
	gchar *content = NULL;
	gchar *album = NULL, *artist = NULL, *genre = NULL;
	gchar *url = NULL, *song_id = NULL, *title = NULL, *contentType = NULL;
	guint year = 0, track_no = 0, duration = 0;
	gchar *album_id = NULL;
	SubsonicStatusCode code = S_GENERIC_OK;


	PraghaSubsonicApi *subsonic = PRAGHA_SUBSONIC_API (user_data);

	if (!grl_net_wc_request_finish (GRL_NET_WC (object),
	                                res,
	                                &content,
	                                NULL,
	                                &wc_error))
	{
		if (g_cancellable_is_cancelled (subsonic->cancellable)) {
			code = S_USER_CANCELLED;
			subsonic->scanning = FALSE;
			g_cancellable_reset (subsonic->cancellable);
		}
		else {
			code = S_GENERIC_ERROR;
			subsonic->scanning = FALSE;
			g_warning ("Failed to get album from subsonic server: %s", wc_error->message);
		}
	}

	if (content)
	{
		doc = xmlReadMemory (content, strlen(content), NULL, NULL,
		                     XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);

		node = xmlDocGetRootElement (doc);

		node = node->xmlChildrenNode;

		album = xmlGetProp(node, "album");
		artist = xmlGetProp(node, "artist");
		year = xmlHasProp(node, "year") ? atoi(xmlGetProp(node, "year")) : 0;
		genre = xmlGetProp(node, "genre");

		node = node->xmlChildrenNode;
		while(node)
		{
			if (!xmlStrcmp (node->name, (const xmlChar *) "song"))
			{
				song_id = xmlGetProp(node, "id");
				title = xmlGetProp(node, "title");
				contentType = xmlGetProp(node, "contentType");
				track_no = xmlHasProp(node, "track") ? atoi(xmlGetProp(node, "track")) : 0;
				duration = xmlHasProp(node, "duration") ? atoi(xmlGetProp(node, "duration")) : 0;

				url = pragha_subsonic_api_get_friendly_url (subsonic->server, artist, album, title, song_id);

				mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
				                     "file", url,
				                     "source", FILE_HTTP,
				                     "provider", subsonic->server,
				                     "mime-type", contentType,
				                     "track-no", track_no,
				                     "title", title != NULL ? title : "",
				                     "artist", artist != NULL ? artist : "",
				                     "album", album != NULL ? album : "",
				                     "year", year,
				                     "genre", genre != NULL ? genre : "",
				                     "length", duration,
				                     NULL);

				subsonic->songs_list = g_slist_prepend (subsonic->songs_list, mobj);
				songs_count++;
			}
			else {
				g_warning ("Remove these warning: Unknown album node: %s", node->name);
			}
			node = node->next;
		}

		xmlFreeDoc (doc);
	}

	if (code != S_GENERIC_OK) {
		g_warning ("Remove these warning: Subsonic scan finished due error or user interaction.. ");
		g_signal_emit (subsonic, signals[SIGNAL_SCAN_DONE], 0, code);
		return;
	}

	subsonic->songs_count += songs_count;
	g_warning ("Remove these warning: Subsonic response %i songs...", subsonic->songs_count);

	if (album_id = g_queue_pop_head(subsonic->albums_queue)) {
		g_warning ("Remove these warning: Queue new album to look: %s", album_id);
		pragha_subsonic_api_get_album (subsonic, album_id);
	}
	else {
		g_warning ("Remove these warning: Subsonic import finished.");

		subsonic->scanning = FALSE;
		g_signal_emit (subsonic, signals[SIGNAL_SCAN_DONE], 0, code);
	}
}

static void
pragha_subsonic_api_get_album (PraghaSubsonicApi *subsonic,
                               const gchar       *album_id)
{
	GString *url;
	gchar *urlc = NULL;

	url = pragha_subsonic_api_build_url (subsonic, "getAlbum");
	pragha_subsonic_api_add_query_item (url, "id", album_id);
	urlc = pragha_subsonic_api_close_url (url);

	g_warning ("Remove these warning: Album url: %s", urlc);

	grl_net_wc_request_async (subsonic->glrnet,
	                          urlc,
	                          subsonic->cancellable,
	                          pragha_subsonic_api_get_album_done,
	                          subsonic);

	g_free(urlc);
}


/*
 *  Public api.
 */

void
pragha_subsonic_api_authentication (PraghaSubsonicApi *subsonic,
                                    const gchar       *server,
                                    const gchar       *username,
                                    const gchar       *password)
{
	/* Save credentials */

	g_free (subsonic->server);
	g_free (subsonic->username);
	g_free(subsonic->password);

	subsonic->server = g_strdup (server);
	subsonic->username = g_strdup (username);
	subsonic->password = g_strdup (password);

	/* Ping to check connection */

	pragha_subsonic_api_ping_server (subsonic);
}

void
pragha_subsonic_api_deauthentication (PraghaSubsonicApi *subsonic)
{
	if (subsonic->scanning)
		g_cancellable_cancel (subsonic->cancellable);

	subsonic->authenticated = FALSE;

	if (subsonic->server) {
		g_free(subsonic->server);
		subsonic->server = NULL;
	}
	if (subsonic->username) {
		g_free(subsonic->username);
		subsonic->username = NULL;
	}
	if (subsonic->password) {
		g_free(subsonic->password);
		subsonic->password = NULL;
	}
}

void
pragha_subsonic_api_ping_server (PraghaSubsonicApi *subsonic)
{
	GString *url;
	gchar *urlc = NULL;

	url = pragha_subsonic_api_build_url (subsonic, "ping");
	urlc = pragha_subsonic_api_close_url (url);
	grl_net_wc_request_async (subsonic->glrnet,
	                          urlc,
	                          subsonic->cancellable,
	                          pragha_subsonic_api_ping_done,
	                          subsonic);

	g_warning ("Remove these warning: Ping url: %s", urlc);

	g_free(urlc);
}

void
pragha_subsonic_api_scan_server (PraghaSubsonicApi *subsonic)
{
	if (subsonic->scanning)
		return;

	subsonic->scanning = TRUE;

	subsonic->albums_count = 0;
	subsonic->albums_offset = 0;
	subsonic->songs_count = 0;

	pragha_subsonic_api_get_albums_queue (subsonic);
}

void
pragha_subsonic_api_cancel (PraghaSubsonicApi *subsonic)
{
	if (!g_cancellable_is_cancelled (subsonic->cancellable))
		g_cancellable_cancel (subsonic->cancellable);

	if (subsonic->scanning == TRUE) {
		while (g_cancellable_is_cancelled(subsonic->cancellable)) {
			// When canceling always resets it.
			pragha_process_gtk_events ();
		}

		g_queue_clear_full (subsonic->albums_queue,
		                    (GDestroyNotify) g_free);

		g_slist_free_full (subsonic->songs_list,
		                   (GDestroyNotify) g_object_unref);

		subsonic->albums_count = 0;
		subsonic->albums_offset = 0;
		subsonic->songs_count = 0;
	}
}

gboolean
pragha_subsonic_api_is_authtenticated (PraghaSubsonicApi *subsonic)
{
	return subsonic->authenticated;
}

gboolean
pragha_subsonic_api_is_connected (PraghaSubsonicApi *subsonic)
{
	return subsonic->has_connection;
}

gboolean
pragha_subsonic_api_is_scanning (PraghaSubsonicApi *subsonic)
{
	return subsonic->scanning;
}


GCancellable *
pragha_subsonic_get_cancellable (PraghaSubsonicApi *subsonic)
{
	return subsonic->cancellable;
}

GSList *
pragha_subsonic_api_get_songs_list (PraghaSubsonicApi *subsonic)
{
	if (subsonic->scanning == TRUE)
		return NULL;

	return subsonic->songs_list;
}

guint
pragha_subsonic_api_get_songs_count (PraghaSubsonicApi *subsonic)
{
	if (subsonic->scanning == TRUE)
		return 0;

	return subsonic->songs_count;
}

guint
pragha_subsonic_api_get_albums_count (PraghaSubsonicApi *subsonic)
{
	if (subsonic->scanning == TRUE)
		return 0;

	return subsonic->albums_count;
}


/*
 * PraghaSubsonicApi
 */
static void
pragha_subsonic_api_finalize (GObject *object)
{
	PraghaSubsonicApi *subsonic = PRAGHA_SUBSONIC_API(object);

	if (subsonic->scanning == TRUE)
		pragha_subsonic_api_cancel (subsonic);

	g_queue_free_full (subsonic->albums_queue,
	                   (GDestroyNotify) g_free);

	g_slist_free_full (subsonic->songs_list,
	                   (GDestroyNotify) g_object_unref);

	g_object_unref(subsonic->cancellable);

	g_free (subsonic->server);
	g_free (subsonic->username);
	g_free (subsonic->password);

	G_OBJECT_CLASS(pragha_subsonic_api_parent_class)->finalize(object);
}

static void
pragha_subsonic_api_class_init (PraghaSubsonicApiClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_subsonic_api_finalize;

	signals[SIGNAL_AUTH_DONE] =
		g_signal_new ("authenticated",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSubsonicApiClass, authenticated),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);
	signals[SIGNAL_PING_DONE] =
		g_signal_new ("pong",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSubsonicApiClass, pong),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);
	signals[SIGNAL_SCAN_DONE] =
		g_signal_new ("scan-finished",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSubsonicApiClass, scan_finished),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
pragha_subsonic_api_init (PraghaSubsonicApi *subsonic)
{
	subsonic->server = NULL;
	subsonic->username = NULL;
	subsonic->password = NULL;

	subsonic->glrnet = grl_net_wc_new ();
	//grl_net_wc_set_throttling (subsonic->glrnet, 1);
	subsonic->cancellable = g_cancellable_new ();

	subsonic->albums_offset = 0;
	subsonic->albums_queue = g_queue_new ();
	subsonic->albums_count = 0;

	subsonic->songs_count = 0;
	subsonic->songs_list = NULL;

	subsonic->authenticated = FALSE;
	subsonic->has_connection = FALSE;
	subsonic->scanning = FALSE;
}

PraghaSubsonicApi *
pragha_subsonic_api_new (void)
{
	return PRAGHA_SUBSONIC_API(g_object_new (PRAGHA_TYPE_SUBSONIC_API, NULL));
}

