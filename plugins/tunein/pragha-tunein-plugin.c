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

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libsoup/soup.h>

#include <libpeas/peas.h>

#include "src/pragha.h"
#include "src/pragha-playlist.h"
#include "src/pragha-playlists-mgmt.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-hig.h"
#include "src/pragha-menubar.h"
#include "src/pragha-utils.h"
#include "src/xml_helper.h"

#include "plugins/pragha-plugin-macros.h"

#define PRAGHA_TYPE_TUNEIN_PLUGIN         (pragha_tunein_plugin_get_type ())
#define PRAGHA_TUNEIN_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPlugin))
#define PRAGHA_TUNEIN_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPlugin))
#define PRAGHA_IS_TUNEIN_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_TUNEIN_PLUGIN))
#define PRAGHA_IS_TUNEIN_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_TUNEIN_PLUGIN))
#define PRAGHA_TUNEIN_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPluginClass))

struct _PraghaTuneinPluginPrivate {
	PraghaApplication *pragha;

	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;
};
typedef struct _PraghaTuneinPluginPrivate PraghaTuneinPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_TUNEIN_PLUGIN,
                        PraghaTuneinPlugin,
                        pragha_tunein_plugin)

/*
 * Prototypes
 */
static void pragha_tunein_get_radio_dialog        (PraghaTuneinPlugin *plugin);

/*
 * Popups
 */

static void
pragha_tunein_plugin_get_radio_action (GAction            *action,
                                       GVariant           *variant,
                                       PraghaTuneinPlugin *plugin)
{
	pragha_tunein_get_radio_dialog (plugin);
}

/*
 * TuneIn Handlers
 */
static const gchar *
tunein_helper_get_atribute (XMLNode *xml, const gchar *atribute)
{
	XMLNode *xi;

	xi = xmlnode_get (xml,CCA {"outline", NULL}, atribute, NULL);

	if (xi)
		return xi->content;

	return NULL;
}

static void
pragha_tunein_plugin_get_radio_done (SoupSession *session,
                                     SoupMessage *msg,
                                     gpointer     user_data)
{
	GtkWidget *window;
	PraghaPlaylist *playlist;
	PraghaDatabase *cdbase;
	PraghaMusicobject *mobj = NULL;
	XMLNode *xml = NULL, *xi;
	const gchar *name = NULL, *url = NULL;
	gchar *name_fixed = NULL;

	PraghaTuneinPlugin *plugin = user_data;
	PraghaTuneinPluginPrivate *priv = plugin->priv;

	window = pragha_application_get_window (priv->pragha);
	remove_watch_cursor (window);

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		return;

	xml = tinycxml_parse ((gchar *)msg->response_body->data);
	xi = xmlnode_get (xml, CCA{"opml", "body", "outline", NULL }, NULL, NULL);

	name = tunein_helper_get_atribute (xi, "text");
	url = tunein_helper_get_atribute (xi, "URL");

	if (string_is_empty(name) || string_is_empty(url)) {
		xmlnode_free(xml);
		return;
	}

	name_fixed = unescape_HTML (name);
	mobj = new_musicobject_from_location (url, name_fixed);

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_append_single_song (playlist, mobj);
	new_radio (playlist, url, name_fixed);

	cdbase = pragha_application_get_database (priv->pragha);
	pragha_database_change_playlists_done (cdbase);

	xmlnode_free(xml);
	g_free (name_fixed);
}

static void
pragha_tunein_plugin_get_radio (PraghaTuneinPlugin *plugin, const gchar *field)
{
	GtkWidget *window;
	SoupSession *session;
	SoupMessage *msg;
	gchar *query = NULL;

	PraghaTuneinPluginPrivate *priv = plugin->priv;

	window = pragha_application_get_window (priv->pragha);
	set_watch_cursor (window);

	query = g_strdup_printf ("%s%s", "http://opml.radiotime.com/Search.aspx?query=", field);

	session = soup_session_sync_new ();

	msg = soup_message_new ("GET", query);
	soup_session_queue_message (session, msg,
	                            pragha_tunein_plugin_get_radio_done, plugin);

	g_free (query);
}

/*
 * TuneIn dialog
 */
static void
pragha_tunein_get_radio_dialog (PraghaTuneinPlugin *plugin)
{
	GtkWidget *dialog, *parent;
	GtkWidget *table, *entry;
	gint result;
	guint row = 0;

	PraghaTuneinPluginPrivate *priv = plugin->priv;

	parent = pragha_application_get_window (priv->pragha);
	dialog = gtk_dialog_new_with_buttons (_("Search in TuneIn"),
	                                      GTK_WINDOW(parent),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Ok"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title (table, &row, _("Search in TuneIn"));

	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(entry), 255);
	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);

	pragha_hig_workarea_table_add_wide_control (table, &row, entry);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		pragha_tunein_plugin_get_radio(plugin, gtk_entry_get_text(GTK_ENTRY(entry)));
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}
	gtk_widget_destroy (dialog);
}

/*
 * TuneIn plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GSimpleAction *action;
	GMenuItem *item;
	PraghaTuneinPlugin *plugin = PRAGHA_TUNEIN_PLUGIN (activatable);

	g_debug ("%s", G_STRFUNC);

	PraghaTuneinPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	/* Attach main menu */
	action = g_simple_action_new ("tunein_get_radio", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_tunein_plugin_get_radio_action), plugin);
	item = g_menu_item_new (_("Search radio on TuneIn"), "win.tunein_get_radio");
	g_menu_item_set_icon (item, g_themed_icon_new ("audio-input-microphone"));

	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaTuneinPlugin *plugin = PRAGHA_TUNEIN_PLUGIN (activatable);
	PraghaTuneinPluginPrivate *priv = plugin->priv;

	g_debug ("%s", G_STRFUNC);

	pragha_menubar_remove_action(priv->pragha, "pragha-plugins-placeholder", "tunein_get_radio");
}