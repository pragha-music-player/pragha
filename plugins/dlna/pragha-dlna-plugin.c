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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif
#include <sys/types.h>
#include <ifaddrs.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <gio/gio.h>
#include <rygel-server.h>
#include <rygel-core.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "pragha-dlna-plugin.h"

#include "src/pragha.h"
#include "src/pragha-utils.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-playlist.h"
#include "src/pragha-database-provider.h"

#include "plugins/pragha-plugin-macros.h"

typedef struct _PraghaDlnaPluginPrivate PraghaDlnaPluginPrivate;

struct _PraghaDlnaPluginPrivate {
	PraghaApplication    *pragha;

    RygelMediaServer     *server;
    RygelSimpleContainer *container;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_DLNA_PLUGIN,
                        PraghaDlnaPlugin,
                        pragha_dlna_plugin)

static void
pragha_dlna_plugin_append_track (PraghaDlnaPlugin  *plugin,
                                 PraghaMusicobject *mobj,
                                 gint               id)
{
	RygelMusicItem *item = NULL;
	RygelMediaObject *object;
	gchar *uri = NULL, *u_title = NULL, *item_id = NULL, *content_type = NULL;
	const gchar *file = NULL, *title = NULL;
	gboolean uncertain;

	PraghaDlnaPluginPrivate *priv = plugin->priv;

	title = pragha_musicobject_get_title (mobj);
	u_title = string_is_not_empty(title) ? g_strdup(title) : get_display_name (mobj);

	item_id = g_strdup_printf ("%06d", id);
	item = rygel_music_item_new (item_id,
	                             RYGEL_MEDIA_CONTAINER(priv->container),
	                             u_title,
	                             RYGEL_MUSIC_ITEM_UPNP_CLASS);

	if (item != NULL) {
		rygel_music_item_set_artist (item, pragha_musicobject_get_artist(mobj));
		rygel_music_item_set_album (item, pragha_musicobject_get_album(mobj));
		rygel_music_item_set_track_number (item, pragha_musicobject_get_track_no(mobj));

		rygel_audio_item_set_duration (RYGEL_AUDIO_ITEM(item), (glong)pragha_musicobject_get_length(mobj));

		file = pragha_musicobject_get_file (mobj);
		content_type = g_content_type_guess (file, NULL, 0, &uncertain);
		rygel_media_item_set_mime_type (RYGEL_MEDIA_ITEM (item), content_type);
		g_free(content_type);

		uri = g_filename_to_uri (file, NULL, NULL);

		object = RYGEL_MEDIA_OBJECT (item);
		gee_collection_add (GEE_COLLECTION (object->uris), uri);
		g_free (uri);

		rygel_simple_container_add_child_item (priv->container, RYGEL_MEDIA_ITEM(item));
	}

	g_free(u_title);
	g_free(item_id);
}

static void
pragha_dlna_plugin_share_library (PraghaDlnaPlugin *plugin)
{
	PraghaDatabase *cdbase;
	PraghaMusicobject *mobj;
	gint i = 0;
	PraghaDlnaPluginPrivate *priv = plugin->priv;

	/* Query and insert entries */

	set_watch_cursor (pragha_application_get_window(priv->pragha));

	cdbase = pragha_application_get_database (priv->pragha);

	const gchar *sql = "SELECT id FROM LOCATION";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);

	while (pragha_prepared_statement_step (statement)) {
		gint location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db (cdbase, location_id);
		if (G_LIKELY(mobj)) {
			pragha_dlna_plugin_append_track (plugin, mobj, i++);
			g_object_unref (mobj);
		}
		pragha_process_gtk_events ();
	}
	pragha_prepared_statement_free (statement);

	remove_watch_cursor (pragha_application_get_window(priv->pragha));
}

static void
pragha_dlna_plugin_database_changed (PraghaDatabase   *cdbase,
                                     PraghaDlnaPlugin *plugin)
{
	PraghaDlnaPluginPrivate *priv = plugin->priv;

	if (TRUE)
		return;

	rygel_simple_container_clear (priv->container);
	pragha_dlna_plugin_share_library (plugin);
}

static void
pragha_dlna_plugin_share_playlist (PraghaDlnaPlugin *plugin)
{
	PraghaPlaylist *playlist;
	GList *list = NULL, *i;
	PraghaMusicobject *mobj;
	gint id = 0;

	PraghaDlnaPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);

	set_watch_cursor (pragha_application_get_window(priv->pragha));

	list = pragha_playlist_get_mobj_list (playlist);
	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;

		if (mobj == NULL)
			continue;

		if (pragha_musicobject_is_local_file(mobj))
			pragha_dlna_plugin_append_track (plugin, mobj, id++);

		pragha_process_gtk_events ();
	}

	remove_watch_cursor (pragha_application_get_window(priv->pragha));
}

static void
pragha_dlna_plugin_playlist_changed (PraghaPlaylist   *playlist,
                                     PraghaDlnaPlugin *plugin)
{
	PraghaDlnaPluginPrivate *priv = plugin->priv;

	if (FALSE)
		return;

	rygel_simple_container_clear (priv->container);
	pragha_dlna_plugin_share_playlist (plugin);
}


static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDatabaseProvider *provider;
	PraghaPlaylist *playlist;
    GError *error = NULL;
	struct ifaddrs *addrs,*tmp;

	PraghaDlnaPlugin *plugin = PRAGHA_DLNA_PLUGIN (activatable);

	PraghaDlnaPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "DLNA plugin %s", G_STRFUNC);

	rygel_media_engine_init (&error);
	if (error != NULL) {
		g_print ("Could not initialize media engine: %s\n", error->message);
		g_error_free (error);
	}

	priv->container = rygel_simple_container_new_root (_("Pragha Music Player"));

	priv->server = rygel_media_server_new (_("Pragha Music Player"),
	                                       RYGEL_MEDIA_CONTAINER(priv->container),
	                                       RYGEL_PLUGIN_CAPABILITIES_NONE);

	getifaddrs (&addrs);
	tmp = addrs;
	while (tmp) {
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET)
			rygel_media_device_add_interface (RYGEL_MEDIA_DEVICE (priv->server), tmp->ifa_name);
		tmp = tmp->ifa_next;
	}
	freeifaddrs (addrs);

	provider = pragha_database_provider_get ();
	g_signal_connect (provider, "UpdateDone",
	                  G_CALLBACK(pragha_dlna_plugin_database_changed), plugin);
	g_object_unref (provider);

	playlist = pragha_application_get_playlist (priv->pragha);
	g_signal_connect (playlist, "playlist-changed",
		              G_CALLBACK(pragha_dlna_plugin_playlist_changed), plugin);

	if (FALSE)
		pragha_dlna_plugin_share_library (plugin);
	else
		pragha_dlna_plugin_share_playlist (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDatabase *cdbase;
	PraghaPlaylist *playlist;

	PraghaDlnaPlugin *plugin = PRAGHA_DLNA_PLUGIN (activatable);

	PraghaDlnaPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "DLNA plugin %s", G_STRFUNC);

	cdbase = pragha_application_get_database (priv->pragha);
	g_signal_handlers_disconnect_by_func (cdbase,
	                                      pragha_dlna_plugin_database_changed,
	                                      plugin);

	playlist = pragha_application_get_playlist (priv->pragha);
	g_signal_handlers_disconnect_by_func (playlist,
	                                      pragha_dlna_plugin_playlist_changed,
	                                      plugin);
	                                      
	rygel_simple_container_clear (priv->container);

	g_object_unref (priv->container);
	g_object_unref (priv->server);
}