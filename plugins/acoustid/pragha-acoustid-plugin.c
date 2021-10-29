/*************************************************************************/
/* Copyright (C) 2014-2019 matias <mati86dl@gmail.com>                   */
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

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <gst/gst.h>

#include <libsoup/soup.h>

#include <libpeas/peas.h>

#include "src/pragha.h"
#include "src/pragha-app-notification.h"
#include "src/pragha-menubar.h"
#include "src/pragha-playlist.h"
#include "src/pragha-playlists-mgmt.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-hig.h"
#include "src/pragha-utils.h"
#include "src/xml_helper.h"
#include "src/pragha-window.h"
#include "src/pragha-tagger.h"
#include "src/pragha-tags-dialog.h"
#include "src/pragha-background-task-bar.h"
#include "src/pragha-background-task-widget.h"

#include "plugins/pragha-plugin-macros.h"

#define PRAGHA_TYPE_ACOUSTID_PLUGIN         (pragha_acoustid_plugin_get_type ())
#define PRAGHA_ACOUSTID_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_ACOUSTID_PLUGIN, PraghaAcoustidPlugin))
#define PRAGHA_ACOUSTID_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_ACOUSTID_PLUGIN, PraghaAcoustidPlugin))
#define PRAGHA_IS_ACOUSTID_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_ACOUSTID_PLUGIN))
#define PRAGHA_IS_ACOUSTID_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_ACOUSTID_PLUGIN))
#define PRAGHA_ACOUSTID_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_ACOUSTID_PLUGIN, PraghaAcoustidPluginClass))

struct _PraghaAcoustidPluginPrivate {
	PraghaApplication          *pragha;

	PraghaMusicobject          *mobj;

	PraghaBackgroundTaskWidget *task_widget;

	GtkActionGroup             *action_group_main_menu;
	guint                       merge_id_main_menu;
};
typedef struct _PraghaAcoustidPluginPrivate PraghaAcoustidPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_ACOUSTID_PLUGIN,
                        PraghaAcoustidPlugin,
                        pragha_acoustid_plugin)

/*
 * Prototypes
 */
static void pragha_acoustid_get_metadata_dialog (PraghaAcoustidPlugin *plugin);

/*
 * Popups
 */
static void
pragha_acoustid_plugin_get_metadata_action (GtkAction *action, PraghaAcoustidPlugin *plugin)
{
	PraghaBackend *backend;

	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Get Metadata action");

	backend = pragha_application_get_backend (priv->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return;

	pragha_acoustid_get_metadata_dialog (plugin);
}

static const GtkActionEntry main_menu_actions [] = {
	{"Search metadata", NULL, N_("Search tags on AcoustID"),
	 "", "Search metadata", G_CALLBACK(pragha_acoustid_plugin_get_metadata_action)}
};

static const gchar *main_menu_xml = "<ui>						\
	<menubar name=\"Menubar\">									\
		<menu action=\"ToolsMenu\">								\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menuitem action=\"Search metadata\"/>			\
				<separator/>									\
			</placeholder>										\
		</menu>													\
	</menubar>													\
</ui>";

/*
 * Gear menu.
 */

static void
pragha_gmenu_search_metadata_action (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       user_data)
{
	pragha_acoustid_plugin_get_metadata_action (NULL, PRAGHA_ACOUSTID_PLUGIN(user_data));
}

/*
 * AcoustID Handlers
 */

static void
pragha_acoustid_dialog_response (GtkWidget            *dialog,
                                 gint                  response_id,
                                 PraghaAcoustidPlugin *plugin)
{
	PraghaBackend *backend;
	PraghaPlaylist *playlist;
	PraghaToolbar *toolbar;
	PraghaMusicobject *nmobj, *current_mobj;
	PraghaTagger *tagger;
	gint changed = 0;

	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog(nmobj, pragha_application_get_window(priv->pragha));
		return;
	}

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed(PRAGHA_TAGS_DIALOG(dialog));
		if (changed) {
			backend = pragha_application_get_backend (priv->pragha);

			nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));

			if (pragha_backend_get_state (backend) != ST_STOPPED) {
				current_mobj = pragha_backend_get_musicobject (backend);
				if (pragha_musicobject_compare (nmobj, current_mobj) == 0) {
					toolbar = pragha_application_get_toolbar (priv->pragha);

					/* Update public current song */
					pragha_update_musicobject_change_tag (current_mobj, changed, nmobj);

					/* Update current song on playlist */
					playlist = pragha_application_get_playlist (priv->pragha);
					pragha_playlist_update_current_track (playlist, changed, nmobj);

					pragha_toolbar_set_title(toolbar, current_mobj);
				}
			}

			if (G_LIKELY(pragha_musicobject_is_local_file (nmobj))) {
				tagger = pragha_tagger_new();
				pragha_tagger_add_file (tagger, pragha_musicobject_get_file(nmobj));
				pragha_tagger_set_changes (tagger, nmobj, changed);
				pragha_tagger_apply_changes (tagger);
				g_object_unref(tagger);
			}
		}
	}

	gtk_widget_destroy (dialog);
}

static void
pragha_acoustid_plugin_get_metadata_done (SoupSession *session,
                                          SoupMessage *msg,
                                          gpointer     user_data)
{
	GtkWidget *dialog;
	PraghaAppNotification *notification;
	PraghaBackgroundTaskBar *taskbar;
	XMLNode *xml = NULL, *xi;
	gchar *otitle = NULL, *oartist = NULL, *oalbum = NULL;
	gchar *ntitle = NULL, *nartist = NULL, *nalbum = NULL;
	gint prechanged = 0;

	PraghaAcoustidPlugin *plugin = user_data;
	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	taskbar = pragha_background_task_bar_get ();
	pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(taskbar));

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		notification = pragha_app_notification_new ("AcoustID", _("There was an error when searching your tags on AcoustID"));
		pragha_app_notification_show (notification);
		return;
	}

	g_object_get (priv->mobj,
	              "title", &otitle,
	              "artist", &oartist,
	              "album", &oalbum,
	              NULL);

	xml = tinycxml_parse ((gchar *)msg->response_body->data);

	xi = xmlnode_get (xml, CCA{"response", "results", "result", "recordings", "recording", "title", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content)) {
		ntitle = unescape_HTML (xi->content);
		if (g_strcmp0(otitle, ntitle)) {
			pragha_musicobject_set_title (priv->mobj, ntitle);
			prechanged |= TAG_TITLE_CHANGED;
		}
		g_free (ntitle);
	}

	xi = xmlnode_get (xml, CCA{"response", "results", "result", "recordings", "recording", "artists", "artist", "name", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content)) {
		nartist = unescape_HTML (xi->content);
		if (g_strcmp0(oartist, nartist)) {
			pragha_musicobject_set_artist (priv->mobj, nartist);
			prechanged |= TAG_ARTIST_CHANGED;
		}
		g_free (nartist);
	}

	xi = xmlnode_get (xml, CCA{"response", "results", "result", "recordings", "recording", "releasegroups", "releasegroup", "title", NULL }, NULL, NULL);
	if (xi && string_is_not_empty(xi->content)) {
		nalbum = unescape_HTML (xi->content);
		if (g_strcmp0(oalbum, nalbum)) {
			pragha_musicobject_set_album (priv->mobj, nalbum);
			prechanged |= TAG_ALBUM_CHANGED;
		}
		g_free (nalbum);
	}

	if (prechanged)	{
		dialog = pragha_tags_dialog_new ();
		gtk_window_set_transient_for (GTK_WINDOW(dialog),
			GTK_WINDOW(pragha_application_get_window(priv->pragha)));

		g_signal_connect (G_OBJECT (dialog), "response",
		                  G_CALLBACK (pragha_acoustid_dialog_response), plugin);

		pragha_tags_dialog_set_musicobject (PRAGHA_TAGS_DIALOG(dialog), priv->mobj);
		pragha_tags_dialog_set_changed (PRAGHA_TAGS_DIALOG(dialog), prechanged);

		gtk_widget_show (dialog);
	}
	else {
		notification = pragha_app_notification_new ("AcoustID", _("AcoustID not found any similar song"));
		pragha_app_notification_show (notification);
	}

	g_free (otitle);
	g_free (oartist);
	g_free (oalbum);

	g_object_unref (priv->mobj);
	xmlnode_free (xml);
}

static void
pragha_acoustid_plugin_get_metadata (PraghaAcoustidPlugin *plugin, gint duration, const gchar *fingerprint)
{
	SoupSession *session;
	SoupMessage *msg;
	gchar *query = NULL;

	query = g_strdup_printf ("http://api.acoustid.org/v2/lookup?client=%s&meta=%s&format=%s&duration=%d&fingerprint=%s",
	                         "yPvUXBmO", "recordings+releasegroups+compress", "xml", duration, fingerprint);

	session = soup_session_new ();

	msg = soup_message_new ("GET", query);
	soup_session_queue_message (session, msg,
	                            pragha_acoustid_plugin_get_metadata_done, plugin);

	g_free (query);
}

static void
error_cb (GstBus *bus, GstMessage *msg, void *data)
{
	GError *err;
	gchar *debug_info;

	/* Print error details on the screen */
	gst_message_parse_error (msg, &err, &debug_info);
	g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
	g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
	g_clear_error (&err);
	g_free (debug_info);
}

static gboolean
pragha_acoustid_get_fingerprint (const gchar *filename, gchar **fingerprint)
{
	GstElement *pipeline, *chromaprint;
	GstBus *bus;
	GstMessage *msg;
	gchar *uri, *pipestring = NULL;

	uri = g_filename_to_uri(filename, NULL, NULL);
	pipestring = g_strdup_printf("uridecodebin uri=%s ! audioconvert ! chromaprint name=chromaprint0 ! fakesink", uri);
	g_free (uri);

	pipeline = gst_parse_launch (pipestring, NULL);

	bus = gst_element_get_bus (pipeline);
	g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, NULL);

	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
	if (msg != NULL)
		gst_message_unref (msg);
	gst_object_unref (bus);

	gst_element_set_state (pipeline, GST_STATE_NULL);

	chromaprint = gst_bin_get_by_name (GST_BIN(pipeline), "chromaprint0");
	g_object_get (chromaprint, "fingerprint", fingerprint, NULL);

	gst_object_unref (pipeline);
	g_free (pipestring);

	return TRUE;
}

/*
 * AcoustID dialog
 */
static void
pragha_acoustid_get_metadata_dialog (PraghaAcoustidPlugin *plugin)
{
	PraghaAppNotification *notification;
	PraghaBackend *backend = NULL;
	PraghaMusicobject *mobj = NULL;
	PraghaBackgroundTaskBar *taskbar;
	const gchar *file = NULL;
	gchar *fingerprint = NULL;
	gint duration = 0;

	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	backend = pragha_application_get_backend (priv->pragha);
	mobj = pragha_backend_get_musicobject (backend);

	priv->mobj = pragha_musicobject_dup (mobj);

	file = pragha_musicobject_get_file (mobj);
	duration = pragha_musicobject_get_length (mobj);

	priv->task_widget = pragha_background_task_widget_new (_("Searching tags on AcoustID"),
	                                                      "edit-find",
	                                                      0,
	                                                      NULL);
	g_object_unref (G_OBJECT(priv->task_widget));

	taskbar = pragha_background_task_bar_get ();
	pragha_background_task_bar_prepend_widget (taskbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(taskbar));

	if (pragha_acoustid_get_fingerprint (file, &fingerprint))
		pragha_acoustid_plugin_get_metadata (plugin, duration, fingerprint);
	else {
		taskbar = pragha_background_task_bar_get ();
		pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
		g_object_unref(G_OBJECT(taskbar));
		priv->task_widget = NULL;

		notification = pragha_app_notification_new ("AcoustID", _("There was an error when searching your tags on AcoustID"));
		pragha_app_notification_show (notification);
	}

	g_free (fingerprint);
}

static void
backend_changed_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	GtkWindow *window;
	GtkAction *action;
	PraghaBackendState state = ST_STOPPED;

	PraghaAcoustidPlugin *plugin = user_data;
	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	state = pragha_backend_get_state (backend);

	action = gtk_action_group_get_action (priv->action_group_main_menu, "Search metadata");
	gtk_action_set_sensitive (action, state != ST_STOPPED);

	window = GTK_WINDOW(pragha_application_get_window(priv->pragha));
	pragha_menubar_set_enable_action (window, "search-metadata", state != ST_STOPPED);
}

/*
 * AcoustID plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GMenuItem *item;
	GSimpleAction *action;

	PraghaAcoustidPlugin *plugin = PRAGHA_ACOUSTID_PLUGIN (activatable);

	PraghaAcoustidPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "AcustId plugin %s", G_STRFUNC);

	/* Attach main menu */

	priv->action_group_main_menu = pragha_menubar_plugin_action_new ("PraghaAcoustidPlugin",
	                                                                 main_menu_actions,
	                                                                 G_N_ELEMENTS (main_menu_actions),
	                                                                 NULL,
	                                                                 0,
	                                                                 plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);
	/* Gear Menu */

	action = g_simple_action_new ("search-metadata", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_gmenu_search_metadata_action), plugin);

	item = g_menu_item_new (_("Search tags on AcoustID"), "win.search-metadata");
	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);
	g_object_unref (item);

	/* Connect playback signals */

	g_signal_connect (pragha_application_get_backend (priv->pragha), "notify::state",
	                  G_CALLBACK (backend_changed_state_cb), plugin);
	backend_changed_state_cb (pragha_application_get_backend (priv->pragha), NULL, plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaAcoustidPlugin *plugin = PRAGHA_ACOUSTID_PLUGIN (activatable);
	PraghaAcoustidPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "AcustID plugin %s", G_STRFUNC);

	/* Disconnect playback signals */

	g_signal_handlers_disconnect_by_func (pragha_application_get_backend (priv->pragha),
	                                      backend_changed_state_cb, plugin);

	/* Remove menu actions */

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	pragha_menubar_remove_action (priv->pragha, "pragha-plugins-placeholder", "search-metadata");
}
