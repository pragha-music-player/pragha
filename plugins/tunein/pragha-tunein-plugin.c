/*************************************************************************/
/* Copyright (C) 2014-2016 matias <mati86dl@gmail.com>                   */
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
#include "src/pragha-menubar.h"
#include "src/pragha-playlist.h"
#include "src/pragha-playlists-mgmt.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-hig.h"
#include "src/pragha-utils.h"
#include "src/pragha-window.h"
#include "src/pragha-background-task-widget.h"
#include "src/xml_helper.h"

#include "plugins/pragha-plugin-macros.h"

#define PRAGHA_TYPE_TUNEIN_PLUGIN         (pragha_tunein_plugin_get_type ())
#define PRAGHA_TUNEIN_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPlugin))
#define PRAGHA_TUNEIN_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPlugin))
#define PRAGHA_IS_TUNEIN_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_TUNEIN_PLUGIN))
#define PRAGHA_IS_TUNEIN_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_TUNEIN_PLUGIN))
#define PRAGHA_TUNEIN_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_TUNEIN_PLUGIN, PraghaTuneinPluginClass))

struct _PraghaTuneinPluginPrivate {
	PraghaApplication          *pragha;

	PraghaBackgroundTaskWidget *task_widget;
	GtkWidget                  *name_entry;
	GtkActionGroup             *action_group_main_menu;
	guint                       merge_id_main_menu;
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
pragha_tunein_plugin_get_radio_action (GtkAction *action, PraghaTuneinPlugin *plugin)
{
	pragha_tunein_get_radio_dialog (plugin);
}

static void
pragha_gmenu_tunein_plugin_get_radio_action (GSimpleAction *action,
                                             GVariant      *parameter,
                                             gpointer       user_data)
{
	pragha_tunein_get_radio_dialog (PRAGHA_TUNEIN_PLUGIN(user_data));
}

static const GtkActionEntry main_menu_actions [] = {
	{"Search tunein", NULL, N_("Search radio on TuneIn"),
	 "", "Search tunein", G_CALLBACK(pragha_tunein_plugin_get_radio_action)}
};

static const gchar *main_menu_xml = "<ui>						\
	<menubar name=\"Menubar\">									\
		<menu action=\"ToolsMenu\">								\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menuitem action=\"Search tunein\"/>			\
				<separator/>									\
			</placeholder>										\
		</menu>													\
	</menubar>													\
</ui>";

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
	PraghaPlaylist *playlist;
	PraghaStatusbar *statusbar;
	PraghaDatabase *cdbase;
	PraghaMusicobject *mobj = NULL;
	XMLNode *xml = NULL, *xi;
	const gchar *type = NULL, *name = NULL, *url = NULL;
	gchar *uri_parsed, *name_fixed = NULL;

	PraghaTuneinPlugin *plugin = user_data;
	PraghaTuneinPluginPrivate *priv = plugin->priv;

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_remove_task_widget (statusbar, GTK_WIDGET(priv->task_widget));
	g_object_unref (statusbar);

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		statusbar = pragha_statusbar_get ();
		pragha_statusbar_set_misc_text (statusbar, _("There was an error when searching radio on TuneIn"));
		g_object_unref (statusbar);
		return;
	}

	xml = tinycxml_parse ((gchar *)msg->response_body->data);
	xi = xmlnode_get (xml, CCA{"opml", "body", "outline", NULL }, NULL, NULL);

	type = tunein_helper_get_atribute (xi, "type");
	if (g_ascii_strcasecmp(type, "audio") != 0) {
		statusbar = pragha_statusbar_get ();
		pragha_statusbar_set_misc_text (statusbar, _("There was an error when searching radio on TuneIn"));
		g_object_unref (statusbar);
		xmlnode_free(xml);
		return;
	}

	name = tunein_helper_get_atribute (xi, "text");
	url = tunein_helper_get_atribute (xi, "URL");

	if (string_is_empty(name) || string_is_empty(url)) {
		statusbar = pragha_statusbar_get ();
		pragha_statusbar_set_misc_text (statusbar, _("There was an error when searching radio on TuneIn"));
		g_object_unref (statusbar);
		xmlnode_free(xml);
		return;
	}

	name_fixed = unescape_HTML (name);
	uri_parsed = pragha_pl_get_first_playlist_item (url);

	mobj = new_musicobject_from_location (uri_parsed, name_fixed);

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_append_single_song (playlist, mobj);
	new_radio (playlist, uri_parsed, name_fixed);

	cdbase = pragha_application_get_database (priv->pragha);
	pragha_database_change_playlists_done (cdbase);

	xmlnode_free(xml);

	g_free (name_fixed);
	g_free (uri_parsed);
}

static void
pragha_tunein_plugin_get_radio (PraghaTuneinPlugin *plugin, const gchar *field)
{
	PraghaStatusbar *statusbar;
	SoupSession *session;
	SoupMessage *msg;
	gchar *escaped_field = NULL, *query = NULL;

	PraghaTuneinPluginPrivate *priv = plugin->priv;

	statusbar = pragha_statusbar_get ();
	priv->task_widget = pragha_background_task_widget_new (_("Searching radio on TuneIn"),
	                                                       "edit-find",
	                                                       -1,
	                                                       NULL);
	g_object_ref (G_OBJECT(priv->task_widget));
	pragha_statusbar_add_task_widget (statusbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(statusbar));

	escaped_field = g_uri_escape_string (field, NULL, TRUE);
	query = g_strdup_printf ("%s%s", "http://opml.radiotime.com/Search.aspx?query=", escaped_field);

	session = soup_session_new ();

	msg = soup_message_new ("GET", query);
	soup_session_queue_message (session, msg,
	                            pragha_tunein_plugin_get_radio_done, plugin);

	g_free (escaped_field);
	g_free (query);
}

/*
 * TuneIn dialog
 */

static void
pragha_tunein_dialog_response (GtkWidget          *dialog,
                               gint                response_id,
                               PraghaTuneinPlugin *plugin)
{
	PraghaTuneinPluginPrivate *priv = plugin->priv;

	switch (response_id) {
		case GTK_RESPONSE_ACCEPT:
			pragha_tunein_plugin_get_radio (plugin, gtk_entry_get_text(GTK_ENTRY(priv->name_entry)));
			break;
		case GTK_RESPONSE_CANCEL:
		default:
			break;
	}

	priv->name_entry = NULL;
	gtk_widget_destroy (dialog);
}

static void
pragha_tunein_get_radio_dialog (PraghaTuneinPlugin *plugin)
{
	GtkWidget *dialog, *parent;
	GtkWidget *table, *entry;
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
	priv->name_entry = entry;

	pragha_hig_workarea_table_add_wide_control (table, &row, entry);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_tunein_dialog_response), plugin);
	gtk_widget_show_all (dialog);
}

/*
 * TuneIn plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GMenuItem *item;
	GSimpleAction *action;

	PraghaTuneinPlugin *plugin = PRAGHA_TUNEIN_PLUGIN (activatable);

	PraghaTuneinPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "TuneIn plugin %s", G_STRFUNC);

	/* Attach main menu */

	priv->action_group_main_menu = gtk_action_group_new ("PraghaTuneinPlugin");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	/* Gear Menu */

	action = g_simple_action_new ("search-tunein", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_gmenu_tunein_plugin_get_radio_action), plugin);

	item = g_menu_item_new (_("Search radio on TuneIn"), "win.search-tunein");
	pragha_menubar_append_action (priv->pragha, "pragha-plugins-placeholder", action, item);
	g_object_unref (item);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaTuneinPlugin *plugin = PRAGHA_TUNEIN_PLUGIN (activatable);
	PraghaTuneinPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "TuneIn plugin %s", G_STRFUNC);

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	pragha_menubar_remove_action (priv->pragha, "pragha-plugins-placeholder", "search-tunein");
}
