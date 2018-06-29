/*************************************************************************/
/* Copyright (C) 2018 matias <mati86dl@gmail.com>                        */
/*                                                       */
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
#include <gdk/gdkx.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "pragha-visualizer-plugin.h"

#include "src/pragha.h"
#include "src/pragha-menubar.h"
#include "src/pragha-playback.h"
#include "src/pragha-window.h"

#include "plugins/pragha-plugin-macros.h"

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_VISUALIZER_PLUGIN,
                        PraghaVisualizerPlugin,
                        pragha_visualizer_plugin)


/*
 * Menubar Prototypes
 */

static void
visualizer_action (GtkAction *action, PraghaVisualizerPlugin *plugin)
{
	GtkWidget *main_stack;
	gboolean visualizer;

	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	main_stack = pragha_application_get_main_stack (priv->pragha);

	visualizer = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
	if(visualizer) {
		gtk_stack_set_visible_child_name (GTK_STACK(main_stack), "visualizer");
	}
	else {
		gtk_stack_set_visible_child_name (GTK_STACK(main_stack), "playlist");
	}

	/* Sink gear menu and menubar */
	g_simple_action_set_state (priv->gear_action, g_variant_new_boolean (visualizer));
}

static const
GtkToggleActionEntry main_menu_actions [] = {
	{"Visualizer", NULL, N_("_Visualizer"),
	 "<Control>V", "Switch between playlist and visualizer", G_CALLBACK(visualizer_action),
	FALSE}
};

static const
gchar *main_menu_xml = "<ui>						\
	<menubar name=\"Menubar\">							\
		<menu action=\"ViewMenu\">						\
			<placeholder name=\"pragha-view-placeholder\">			\
				<menuitem action=\"Visualizer\"/>			\
			</placeholder>							\
		</menu>									\
	</menubar>									\
</ui>";


static void
pragha_gmenu_visualizer (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
	GtkAction *gtkaction;

	PraghaVisualizerPlugin *plugin = user_data;
	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	/* Sink gear menu and menubar. Logic there. */
	gtkaction = gtk_action_group_get_action (priv->action_group_main_menu, "Visualizer");
	gtk_action_activate (gtkaction);
}


/*
 *  Visualizer plugin.
 */
static void
pragha_visualizer_plugin_update_spectrum (PraghaBackend *backend, gpointer value, gpointer user_data)
{
	GValue *magnitudes = value;

	PraghaVisualizerPlugin *plugin = user_data;
	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	pragha_visualizer_set_magnitudes (priv->visualizer, magnitudes);
}

static void
pragha_visualizer_plugin_append_menues (PraghaVisualizerPlugin *plugin)
{
	GMenuItem *item;
	GSimpleAction *action;

	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	/*
	 * Menubar
	 */
	priv->action_group_main_menu = gtk_action_group_new ("PraghaVisualizerMainMenuActions");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_toggle_actions (priv->action_group_main_menu,
	                                     main_menu_actions,
	                                     G_N_ELEMENTS (main_menu_actions),
	                                     plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);

	/*
	 * Gear Menu
	 */

	action = g_simple_action_new_stateful("visualizer", NULL, g_variant_new_boolean(FALSE));
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_gmenu_visualizer), plugin);

	item = g_menu_item_new (_("Show Visualizer"), "win.visualizer");
	pragha_menubar_append_action (priv->pragha, "pragha-view-placeholder", action, item);
	g_object_unref (item);

	priv->gear_action = action;
}

static void
pragha_visualizer_plugin_remove_menues (PraghaVisualizerPlugin *plugin)
{
	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	if (!priv->merge_id_main_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);

	priv->merge_id_main_menu = 0;

	pragha_menubar_remove_action (priv->pragha, "pragha-view-placeholder", "visualizer");
}


static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	GtkWidget *main_stack;

	PraghaVisualizerPlugin *plugin = PRAGHA_VISUALIZER_PLUGIN (activatable);
	PraghaVisualizerPluginPrivate *priv = plugin->priv;
	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "Visualizer plugin %s", G_STRFUNC);

	priv->visualizer = pragha_visualizer_new ();

	main_stack = pragha_application_get_main_stack (priv->pragha);
	gtk_stack_add_named (GTK_STACK(main_stack), GTK_WIDGET(priv->visualizer), "visualizer");

	pragha_visualizer_plugin_append_menues (plugin);

	/* Connect signals */
	backend = pragha_application_get_backend (priv->pragha);
	pragha_backend_enable_spectrum (backend);
	g_signal_connect (backend, "spectrum",
	                 G_CALLBACK(pragha_visualizer_plugin_update_spectrum), plugin);

	gtk_widget_show_all (GTK_WIDGET(priv->visualizer));
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	GtkWidget *main_stack;

	PraghaVisualizerPlugin *plugin = PRAGHA_VISUALIZER_PLUGIN (activatable);
	PraghaVisualizerPluginPrivate *priv = plugin->priv;

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	CDEBUG(DBG_PLUGIN, "Visualizer plugin %s", G_STRFUNC);

	/* Disconnect signals */
	backend = pragha_application_get_backend (priv->pragha);
	pragha_backend_disable_spectrum (backend);
	g_signal_handlers_disconnect_by_func (backend,
	                                      pragha_visualizer_plugin_update_spectrum, plugin);

	pragha_visualizer_plugin_remove_menues (plugin);

	/* Free Memory */

	main_stack = pragha_application_get_main_stack (priv->pragha);
	gtk_container_remove (GTK_CONTAINER(main_stack), GTK_WIDGET(priv->visualizer));
}
