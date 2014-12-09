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

#include <string.h>

#include <libpeas/peas-engine.h>

#include "pragha-plugins-view.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

/*
 * The Plugins Store
 */
static const GType ColumnTypes[] = {
	G_TYPE_BOOLEAN, /* Enabled */
	G_TYPE_BOOLEAN, /* Enabled Visible */
	G_TYPE_OBJECT,  /* GIcon Icon */
	G_TYPE_BOOLEAN, /* Icon Visible */
	G_TYPE_STRING,  /* Info */
	G_TYPE_BOOLEAN, /* Info Visible */
	/* To avoid having to unref it all the time */
	G_TYPE_POINTER  /* PeasPluginInfo */
};

G_STATIC_ASSERT (G_N_ELEMENTS (ColumnTypes) == PRAGHA_PLUGINS_STORE_N_COLUMNS);

struct _PraghaPluginsStorePrivate {
	PeasEngine *engine;
};

/* Properties */
enum {
	PROP_S_0,
	PROP_S_ENGINE,
	N_S_PROPERTIES
};

static GParamSpec *properties_s[N_S_PROPERTIES] = { NULL };

G_DEFINE_TYPE (PraghaPluginsStore, pragha_plugins_store, GTK_TYPE_LIST_STORE)

static void
update_plugin (PraghaPluginsStore *store,
               GtkTreeIter        *iter,
               PeasPluginInfo     *info)
{
	gboolean loaded;
	gboolean available;
	gboolean builtin;
	gchar *markup;
	const gchar *icon_name;
	GIcon *icon_gicon = NULL;

	loaded = peas_plugin_info_is_loaded (info);
	available = peas_plugin_info_is_available (info, NULL);
	builtin = peas_plugin_info_is_builtin (info);

	if (peas_plugin_info_get_description (info) == NULL) {
		markup = g_markup_printf_escaped ("<b>%s</b>",
		                                  peas_plugin_info_get_name (info));
	}
	else {
		markup = g_markup_printf_escaped ("<b>%s</b>\n%s",
		                                  peas_plugin_info_get_name (info),
		                                  peas_plugin_info_get_description (info));
    }

	if (!available) {
		icon_gicon = g_themed_icon_new ("dialog-error");
	}
	else {
		gchar *icon_path;

		icon_name = peas_plugin_info_get_icon_name (info);
		icon_path = g_build_filename (peas_plugin_info_get_data_dir (info),
		                              icon_name,
		                              NULL);

		/* Prevent warning for the common case that icon_path
		 * does not exist but warn when it is a directory
		 */
		if (g_file_test (icon_path, G_FILE_TEST_EXISTS)) {
			GFile *icon_file;

			icon_file = g_file_new_for_path (icon_path);
			icon_gicon = g_file_icon_new (icon_file);

			g_object_unref (icon_file);
		}
		else {
			gint i;
			GtkIconTheme *icon_theme;
			const gchar * const *names;
			gboolean found_icon = FALSE;

			icon_gicon = g_themed_icon_new_with_default_fallbacks (icon_name);

			icon_theme = gtk_icon_theme_get_default ();
			names = g_themed_icon_get_names (G_THEMED_ICON (icon_gicon));

			for (i = 0; !found_icon && i < g_strv_length ((gchar **) names); ++i)
				found_icon = gtk_icon_theme_has_icon (icon_theme, names[i]);

			if (!found_icon) {
				g_clear_object (&icon_gicon);
				icon_gicon = g_themed_icon_new ("libpeas-plugin");
			}
		}
		g_free (icon_path);
	}

	gtk_list_store_set (GTK_LIST_STORE (store), iter,
	                    PRAGHA_PLUGINS_STORE_ENABLED_COLUMN,        loaded,
	                    PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN,     !builtin && available,
	                    PRAGHA_PLUGINS_STORE_ICON_GICON_COLUMN,     icon_gicon,
	                    PRAGHA_PLUGINS_STORE_ICON_VISIBLE_COLUMN,   !available,
	                    PRAGHA_PLUGINS_STORE_INFO_COLUMN,           markup,
	                    PRAGHA_PLUGINS_STORE_INFO_SENSITIVE_COLUMN, available && (!builtin || loaded),
	                    PRAGHA_PLUGINS_STORE_PLUGIN_COLUMN,         info,
	                    -1);

	if (icon_gicon != NULL)
		g_object_unref (icon_gicon);

	g_free (markup);
}

static void
plugin_loaded_toggled_cb (PeasEngine                *engine,
                          PeasPluginInfo            *info,
                          PraghaPluginsStore *store)
{
	GtkTreeIter iter;

	if (pragha_plugins_store_get_iter_from_plugin (store, &iter, info))
		update_plugin (store, &iter, info);
}

static gint
model_name_sort_func (PraghaPluginsStore *store,
                      GtkTreeIter        *iter1,
                      GtkTreeIter        *iter2,
                      gpointer            user_data)
{
	PeasPluginInfo *info1;
	PeasPluginInfo *info2;

	info1 = pragha_plugins_store_get_plugin (store, iter1);
	info2 = pragha_plugins_store_get_plugin (store, iter2);

	return g_utf8_collate (peas_plugin_info_get_name (info1),
	                       peas_plugin_info_get_name (info2));
}

static void
pragha_plugins_store_init (PraghaPluginsStore *store)
{
	store->priv = G_TYPE_INSTANCE_GET_PRIVATE (store,
	                                           PRAGHA_TYPE_PLUGINS_STORE,
	                                           PraghaPluginsStorePrivate);

	gtk_list_store_set_column_types (GTK_LIST_STORE (store),
	                                 PRAGHA_PLUGINS_STORE_N_COLUMNS,
	                                (GType *) ColumnTypes);

	/* Sort on the plugin names */
	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (store),
	                                         (GtkTreeIterCompareFunc) model_name_sort_func,
	                                         NULL, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
	                                      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
	                                      GTK_SORT_ASCENDING);
}

static void
pragha_plugins_store_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	PraghaPluginsStore *store = PRAGHA_PLUGINS_STORE (object);

	switch (prop_id) {
		case PROP_S_ENGINE:
			store->priv->engine = g_value_get_object (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_plugins_store_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	PraghaPluginsStore *store = PRAGHA_PLUGINS_STORE (object);

	switch (prop_id) {
		case PROP_S_ENGINE:
			g_value_set_object (value, store->priv->engine);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_plugins_store_constructed (GObject *object)
{
	PraghaPluginsStore *store = PRAGHA_PLUGINS_STORE (object);

	if (store->priv->engine == NULL)
		store->priv->engine = peas_engine_get_default ();

	g_object_ref (store->priv->engine);

	g_signal_connect_after (store->priv->engine,
	                        "load-plugin",
	                        G_CALLBACK (plugin_loaded_toggled_cb),
	                        store);
	g_signal_connect_after (store->priv->engine,
	                        "unload-plugin",
	                        G_CALLBACK (plugin_loaded_toggled_cb),
	                        store);

	pragha_plugins_store_reload (store);

	G_OBJECT_CLASS (pragha_plugins_store_parent_class)->constructed (object);
}

static void
pragha_plugins_store_dispose (GObject *object)
{
	PraghaPluginsStore *store = PRAGHA_PLUGINS_STORE (object);

	if (store->priv->engine != NULL) {
		g_signal_handlers_disconnect_by_func (store->priv->engine,
		                                      plugin_loaded_toggled_cb,
		                                      store);
		g_clear_object (&store->priv->engine);
	}

	G_OBJECT_CLASS (pragha_plugins_store_parent_class)->dispose (object);
}

static void
pragha_plugins_store_class_init (PraghaPluginsStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = pragha_plugins_store_set_property;
	object_class->get_property = pragha_plugins_store_get_property;
	object_class->constructed = pragha_plugins_store_constructed;
	object_class->dispose = pragha_plugins_store_dispose;

	properties_s[PROP_S_ENGINE] =
		g_param_spec_object ("engine",
		                     "engine",
		                     "The PeasEngine this store is attached to",
		                     PEAS_TYPE_ENGINE,
		                     G_PARAM_READWRITE |
		                     G_PARAM_CONSTRUCT_ONLY |
		                     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_S_PROPERTIES, properties_s);
	g_type_class_add_private (object_class, sizeof (PraghaPluginsStorePrivate));
}

PraghaPluginsStore  *
pragha_plugins_store_new (PeasEngine *engine)
{
	g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

	return PRAGHA_PLUGINS_STORE (g_object_new (PRAGHA_TYPE_PLUGINS_STORE,
	                                           "engine", engine,
	                                           NULL));
}

void
pragha_plugins_store_reload (PraghaPluginsStore *store)
{
	GtkListStore *list_store;
	const GList *plugins;
	GtkTreeIter iter;

	g_return_if_fail (PRAGHA_IS_PLUGINS_STORE (store));

	list_store = GTK_LIST_STORE (store);

	gtk_list_store_clear (list_store);

	plugins = peas_engine_get_plugin_list (store->priv->engine);

	while (plugins != NULL) {
		PeasPluginInfo *info;

		info = PEAS_PLUGIN_INFO (plugins->data);

		if (!peas_plugin_info_is_hidden (info)) {
			gtk_list_store_append (list_store, &iter);
			update_plugin (store, &iter, info);
		}

		plugins = plugins->next;
	}
}

void
pragha_plugins_store_set_enabled (PraghaPluginsStore *store,
                                  GtkTreeIter        *iter,
                                  gboolean            enabled)
{
	PeasPluginInfo *info;
	gboolean success = TRUE;

	g_return_if_fail (PRAGHA_IS_PLUGINS_STORE (store));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (pragha_plugins_store_can_enable (store, iter));

	info = pragha_plugins_store_get_plugin (store, iter);
	g_return_if_fail (info != NULL);

	if (enabled) {
		/* load the plugin */
		if (!peas_engine_load_plugin (store->priv->engine, info))
			success = FALSE;
	}
	else {
		/* unload the plugin */
		if (!peas_engine_unload_plugin (store->priv->engine, info))
			success = FALSE;
	}

	if (success)
		update_plugin (store, iter, info);
}

gboolean
pragha_plugins_store_get_enabled (PraghaPluginsStore *store,
                                  GtkTreeIter        *iter)
{
	GValue value = G_VALUE_INIT;
	gboolean enabled;

	g_return_val_if_fail (PRAGHA_IS_PLUGINS_STORE (store), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	gtk_tree_model_get_value (GTK_TREE_MODEL (store), iter,
	                          PRAGHA_PLUGINS_STORE_ENABLED_COLUMN, &value);

	g_return_val_if_fail (G_VALUE_HOLDS_BOOLEAN (&value), FALSE);
	enabled = g_value_get_boolean (&value);

	g_value_unset (&value);

	return enabled;
}

void
pragha_plugins_store_set_all_enabled (PraghaPluginsStore *store,
                                      gboolean            enabled)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	g_return_if_fail (PRAGHA_IS_PLUGINS_STORE (store));

	model = GTK_TREE_MODEL (store);

	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;

	do {
		if (pragha_plugins_store_can_enable (store, &iter))
			pragha_plugins_store_set_enabled (store, &iter, enabled);
	} while (gtk_tree_model_iter_next (model, &iter));
}

void
pragha_plugins_store_toggle_enabled (PraghaPluginsStore *store,
                                     GtkTreeIter        *iter)
{
	gboolean enabled;

	g_return_if_fail (PRAGHA_IS_PLUGINS_STORE (store));
	g_return_if_fail (iter != NULL);

	enabled = pragha_plugins_store_get_enabled (store, iter);

	pragha_plugins_store_set_enabled (store, iter, !enabled);
}

gboolean
pragha_plugins_store_can_enable (PraghaPluginsStore *store,
                                 GtkTreeIter        *iter)
{
	GValue value = G_VALUE_INIT;
	gboolean can_enable;

	g_return_val_if_fail (PRAGHA_IS_PLUGINS_STORE (store), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	gtk_tree_model_get_value (GTK_TREE_MODEL (store), iter,
	                          PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN, &value);

	g_return_val_if_fail (G_VALUE_HOLDS_BOOLEAN (&value), FALSE);
	can_enable = g_value_get_boolean (&value);

	g_value_unset (&value);

	return can_enable;
}

PeasPluginInfo *
pragha_plugins_store_get_plugin (PraghaPluginsStore *store,
                                 GtkTreeIter        *iter)
{
	GValue value = G_VALUE_INIT;
	PeasPluginInfo *info;

	g_return_val_if_fail (PRAGHA_IS_PLUGINS_STORE (store), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	gtk_tree_model_get_value (GTK_TREE_MODEL (store), iter,
	                          PRAGHA_PLUGINS_STORE_PLUGIN_COLUMN, &value);

	g_return_val_if_fail (G_VALUE_HOLDS_POINTER (&value), NULL);
	info = g_value_get_pointer (&value);

	g_value_unset (&value);

	return info;
}

gboolean
pragha_plugins_store_get_iter_from_plugin (PraghaPluginsStore   *store,
                                           GtkTreeIter          *iter,
                                           const PeasPluginInfo *info)
{
	GtkTreeModel *model = GTK_TREE_MODEL (store);
	gboolean found = FALSE;

	g_return_val_if_fail (PRAGHA_IS_PLUGINS_STORE (store), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (info != NULL, FALSE);

	if (gtk_tree_model_get_iter_first (model, iter)) {
		PeasPluginInfo *current_info;

		do {
			current_info = pragha_plugins_store_get_plugin (store, iter);
			found = (info == current_info);
		} while (!found && gtk_tree_model_iter_next (model, iter));
	}

	return found;
}


/*
 * PraghaManager
 */

struct _PraghaPluginsViewPrivate {
	PeasEngine *engine;

	PraghaPluginsStore *store;

	GtkWidget *popup_menu;

	guint show_builtin : 1;
};

/* Properties */
enum {
	PROP_V_0,
	PROP_V_ENGINE,
	PROP_V_SHOW_BUILTIN,
	N_V_PROPERTIES
};

/* Signals */
enum {
	POPULATE_POPUP,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];
static GParamSpec *properties_v[N_V_PROPERTIES] = { NULL };

G_DEFINE_TYPE (PraghaPluginsView, pragha_plugins_view, GTK_TYPE_TREE_VIEW)

static void
convert_iter_to_child_iter (PraghaPluginsView *view,
                            GtkTreeIter       *iter)
{
	if (!view->priv->show_builtin) {
		GtkTreeModel *model;
		GtkTreeIter child_iter;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

		gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER (model),
																												&child_iter, iter);

		*iter = child_iter;
	}
}

static gboolean
convert_child_iter_to_iter (PraghaPluginsView *view,
                            GtkTreeIter       *child_iter)
{
	gboolean success = TRUE;

	if (!view->priv->show_builtin) {
		GtkTreeModel *model;
		GtkTreeIter iter;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

		success = gtk_tree_model_filter_convert_child_iter_to_iter (GTK_TREE_MODEL_FILTER (model),
		                                                            &iter, child_iter);

		if (success)
			*child_iter = iter;
	}

	return success;
}

static void
plugin_list_changed_cb (PeasEngine        *engine,
                        GParamSpec        *pspec,
                        PraghaPluginsView *view)
{
	PeasPluginInfo *info;

	info = pragha_plugins_view_get_selected_plugin (view);

	pragha_plugins_store_reload (view->priv->store);

	if (info != NULL)
		pragha_plugins_view_set_selected_plugin (view, info);
}

static gboolean
filter_builtins_visible (PraghaPluginsStore *store,
                         GtkTreeIter        *iter,
                         PraghaPluginsView  *view)
{
	PeasPluginInfo *info;

	/* We never filter showing builtins */
	g_assert (view->priv->show_builtin == FALSE);

	info = pragha_plugins_store_get_plugin (store, iter);

	if (info == NULL)
		return FALSE;

	return !peas_plugin_info_is_builtin (info);
}

static void
enabled_toggled_cb (GtkCellRendererToggle *cell,
                    gchar                 *path_str,
                    PraghaPluginsView     *view)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));
	path = gtk_tree_path_new_from_string (path_str);

	if (gtk_tree_model_get_iter (model, &iter, path)) {
		convert_iter_to_child_iter (view, &iter);
		pragha_plugins_store_toggle_enabled (view->priv->store, &iter);
	}

	gtk_tree_path_free (path);
}

/* Callback used as the interactive search comparison function */
static gboolean
name_search_cb (GtkTreeModel      *model,
                gint               column,
                const gchar       *key,
                GtkTreeIter       *iter,
                PraghaPluginsView *view)
{
	GtkTreeIter child_iter = *iter;
	PeasPluginInfo *info;
	gchar *normalized_string;
	gchar *normalized_key;
	gchar *case_normalized_string;
	gchar *case_normalized_key;
	gint key_len;
	gboolean retval;

	convert_iter_to_child_iter (view, &child_iter);
	info = pragha_plugins_store_get_plugin (view->priv->store, &child_iter);

	if (info == NULL)
		return FALSE;

	normalized_string = g_utf8_normalize (peas_plugin_info_get_name (info), -1, G_NORMALIZE_ALL);
	normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);
	case_normalized_string = g_utf8_casefold (normalized_string, -1);
	case_normalized_key = g_utf8_casefold (normalized_key, -1);

	key_len = strlen (case_normalized_key);

	/* Oddly enough, this callback must return whether to stop the search
	 * because we found a match, not whether we actually matched.
	 */
	retval = strncmp (case_normalized_key, case_normalized_string, key_len) != 0;

	g_free (normalized_key);
	g_free (normalized_string);
	g_free (case_normalized_key);
	g_free (case_normalized_string);

	return retval;
}

static void
enabled_menu_cb (GtkMenu           *menu,
                 PraghaPluginsView *view)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

	g_return_if_fail (gtk_tree_selection_get_selected (selection, NULL, &iter));

	convert_iter_to_child_iter (view, &iter);

	pragha_plugins_store_toggle_enabled (view->priv->store, &iter);
}

static void
enable_all_menu_cb (GtkMenu           *menu,
                    PraghaPluginsView *view)
{
	pragha_plugins_store_set_all_enabled (view->priv->store, TRUE);
}

static void
disable_all_menu_cb (GtkMenu           *menu,
                     PraghaPluginsView *view)
{
	pragha_plugins_store_set_all_enabled (view->priv->store, FALSE);
}

static GtkWidget *
create_popup_menu (PraghaPluginsView *view)
{
	PeasPluginInfo *info;
	GtkWidget *menu;
	GtkWidget *item;

	info = pragha_plugins_view_get_selected_plugin (view);

	if (info == NULL)
		return NULL;

	menu = gtk_menu_new ();

	item = gtk_check_menu_item_new_with_mnemonic (_("_Enabled"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
	                                peas_plugin_info_is_loaded (info));
	g_signal_connect (item, "toggled", G_CALLBACK (enabled_menu_cb), view);
	gtk_widget_set_sensitive (item, peas_plugin_info_is_available (info, NULL) &&
	                                !peas_plugin_info_is_builtin (info));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("E_nable All"));
	g_signal_connect (item, "activate", G_CALLBACK (enable_all_menu_cb), view);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_menu_item_new_with_mnemonic (_("_Disable All"));
	g_signal_connect (item, "activate", G_CALLBACK (disable_all_menu_cb), view);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	g_signal_emit (view, signals[POPULATE_POPUP], 0, menu);

	gtk_widget_show_all (menu);

	return menu;
}

static void
popup_menu_detach (PraghaPluginsView *view,
                   GtkMenu           *menu)
{
	view->priv->popup_menu = NULL;
}

static void
menu_position_under_tree_view (GtkMenu     *menu,
                               gint        *x,
                               gint        *y,
                               gboolean    *push_in,
                               GtkTreeView *tree_view)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GdkWindow *window;

	selection = gtk_tree_view_get_selection (tree_view);

	window = gtk_widget_get_window (GTK_WIDGET (tree_view));
	gdk_window_get_origin (window, x, y);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		GtkTreeModel *model;
		GtkTreePath *path;
		GdkRectangle rect;

		model = gtk_tree_view_get_model (tree_view);
		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_get_cell_area (tree_view,
		                             path,
		                             gtk_tree_view_get_column (tree_view, 0), /* FIXME 0 for RTL ? */
		                             &rect);
		gtk_tree_path_free (path);

		*x += rect.x;
		*y += rect.y + rect.height;

		if (gtk_widget_get_direction (GTK_WIDGET (tree_view)) == GTK_TEXT_DIR_RTL) {
			GtkRequisition requisition;
			gtk_widget_get_preferred_size (GTK_WIDGET (menu), &requisition, NULL);
			*x += rect.width - requisition.width;
		}
	}
	else {
		GtkAllocation allocation;
		gtk_widget_get_allocation (GTK_WIDGET (tree_view), &allocation);

		*x += allocation.x;
		*y += allocation.y;

		if (gtk_widget_get_direction (GTK_WIDGET (tree_view)) == GTK_TEXT_DIR_RTL) {
			GtkRequisition requisition;
			gtk_widget_get_preferred_size (GTK_WIDGET (menu), &requisition, NULL);

			*x += allocation.width - requisition.width;
		}
	}
	*push_in = TRUE;
}

static gboolean
show_popup_menu (GtkTreeView       *tree_view,
                 PraghaPluginsView *view,
                 GdkEventButton    *event)
{
	if (view->priv->popup_menu)
		gtk_widget_destroy (view->priv->popup_menu);

	view->priv->popup_menu = create_popup_menu (view);

	if (view->priv->popup_menu == NULL)
		return FALSE;

	gtk_menu_attach_to_widget (GTK_MENU (view->priv->popup_menu),
	                           GTK_WIDGET (view),
	                           (GtkMenuDetachFunc) popup_menu_detach);

	if (event != NULL) {
		gtk_menu_popup (GTK_MENU (view->priv->popup_menu), NULL, NULL,
		                NULL, NULL, event->button, event->time);
	}
	else {
		gtk_menu_popup (GTK_MENU (view->priv->popup_menu), NULL, NULL,
		                (GtkMenuPositionFunc) menu_position_under_tree_view,
		                view, 0, gtk_get_current_event_time ());

		gtk_menu_shell_select_first (GTK_MENU_SHELL (view->priv->popup_menu),
		                             FALSE);
	}

	return TRUE;
}

static void
plugin_icon_data_func (GtkTreeViewColumn *column,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *model,
                       GtkTreeIter       *iter)
{
	GIcon *icon_gicon;

	gtk_tree_model_get (model, iter,
	                    PRAGHA_PLUGINS_STORE_ICON_GICON_COLUMN, &icon_gicon,
	                    -1);

	g_object_set (cell, "gicon", icon_gicon, NULL);
	g_object_unref (icon_gicon);
}

static void
pragha_plugins_view_init (PraghaPluginsView *view)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;

	view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
	                                          PRAGHA_TYPE_PLUGINS_VIEW,
	                                          PraghaPluginsViewPrivate);

	gtk_widget_set_has_tooltip (GTK_WIDGET (view), TRUE);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

	/* first column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Enabled"));
	gtk_tree_view_column_set_resizable (column, FALSE);

	cell = gtk_cell_renderer_toggle_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	g_object_set (cell, "xpad", 6, NULL);
	gtk_tree_view_column_set_attributes (column, cell,
	                                     "active", PRAGHA_PLUGINS_STORE_ENABLED_COLUMN,
	                                     "activatable", PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN,
	                                     "sensitive", PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN,
	                                     "visible", PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN,
	                                     NULL);
	g_signal_connect (cell,
	                  "toggled",
	                  G_CALLBACK (enabled_toggled_cb),
	                  view);

	gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

	/* second column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Plugin"));
	gtk_tree_view_column_set_resizable (column, FALSE);

	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	g_object_set (cell, "stock-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
	                                         (GtkTreeCellDataFunc) plugin_icon_data_func,
	                                         NULL, NULL);

	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_set_attributes (column, cell,
	                                     "sensitive", PRAGHA_PLUGINS_STORE_INFO_SENSITIVE_COLUMN,
	                                     "markup", PRAGHA_PLUGINS_STORE_INFO_COLUMN,
	                                     NULL);

	gtk_tree_view_column_set_spacing (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

	/* Enable search for our non-string column */
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (view),
	                                 PRAGHA_PLUGINS_STORE_PLUGIN_COLUMN);
	gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (view),
	                                     (GtkTreeViewSearchEqualFunc) name_search_cb,
	                                     view,
	                                     NULL);
}

static gboolean
pragha_plugins_view_button_press_event (GtkWidget      *tree_view,
                                        GdkEventButton *event)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (tree_view);
	GtkWidgetClass *widget_class;
	gboolean handled;

	widget_class = GTK_WIDGET_CLASS (pragha_plugins_view_parent_class);

	/* The selection must by updated */
	handled = widget_class->button_press_event (tree_view, event);

	if (event->type != GDK_BUTTON_PRESS || event->button != 3 || !handled)
		return handled;

	return show_popup_menu (GTK_TREE_VIEW (tree_view), view, event);
}

static gboolean
pragha_plugins_view_popup_menu (GtkWidget *tree_view)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (tree_view);

	return show_popup_menu (GTK_TREE_VIEW (tree_view), view, NULL);
}

static gboolean
pragha_plugins_view_query_tooltip (GtkWidget  *widget,
                                   gint        x,
                                   gint        y,
                                   gboolean    keyboard_mode,
                                   GtkTooltip *tooltip)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (widget);
	gboolean is_row;
	GtkTreeIter iter;
	PeasPluginInfo *info;
	gchar *message;
	GError *error = NULL;

	is_row = gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget),
	                                            &x, &y, keyboard_mode,
	                                            NULL, NULL, &iter);

	if (!is_row)
		return FALSE;

	convert_iter_to_child_iter (view, &iter);

	info = pragha_plugins_store_get_plugin (view->priv->store, &iter);

	if (peas_plugin_info_is_available (info, &error))
		return FALSE;

	message = g_markup_printf_escaped (_("<b>The plugin '%s' could not be "
	                                     "loaded</b>\nAn error occurred: %s"),
	                                     peas_plugin_info_get_name (info),
	                                     error->message);
	g_error_free (error);

	gtk_tooltip_set_markup (tooltip, message);

	g_free (message);

	return TRUE;
}

static void
pragha_plugins_view_row_activated (GtkTreeView       *tree_view,
                                   GtkTreePath       *path,
                                   GtkTreeViewColumn *column)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (tree_view);
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter (gtk_tree_view_get_model (tree_view), &iter, path))
		return;

	convert_iter_to_child_iter (view, &iter);

	if (pragha_plugins_store_can_enable (view->priv->store, &iter))
		pragha_plugins_store_toggle_enabled (view->priv->store, &iter);
}

static void
pragha_plugins_view_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (object);

	switch (prop_id)
		{
		case PROP_V_ENGINE:
			view->priv->engine = g_value_get_object (value);
			break;
		case PROP_V_SHOW_BUILTIN:
			pragha_plugins_view_set_show_builtin (view,
			                                      g_value_get_boolean (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
		}
}

static void
pragha_plugins_view_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (object);

	switch (prop_id)
		{
		case PROP_V_ENGINE:
			g_value_set_object (value, view->priv->engine);
			break;
		case PROP_V_SHOW_BUILTIN:
			g_value_set_boolean (value,
			                     pragha_plugins_view_get_show_builtin (view));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
		}
}

static void
pragha_plugins_view_constructed (GObject *object)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (object);

	if (view->priv->engine == NULL)
		view->priv->engine = peas_engine_get_default ();

	g_object_ref (view->priv->engine);

	view->priv->store = pragha_plugins_store_new (view->priv->engine);

	/* Properly set the model */
	view->priv->show_builtin = TRUE;
	pragha_plugins_view_set_show_builtin (view, FALSE);

	g_signal_connect (view->priv->engine,
	                  "notify::plugin-list",
	                  G_CALLBACK (plugin_list_changed_cb),
	                  view);

	G_OBJECT_CLASS (pragha_plugins_view_parent_class)->constructed (object);
}

static void
pragha_plugins_view_dispose (GObject *object)
{
	PraghaPluginsView *view = PRAGHA_PLUGINS_VIEW (object);

	if (view->priv->popup_menu != NULL) {
		gtk_widget_destroy (view->priv->popup_menu);
		view->priv->popup_menu = NULL;
	}

	if (view->priv->engine != NULL) {
		g_signal_handlers_disconnect_by_func (view->priv->engine,
		                                      plugin_list_changed_cb,
		                                      view);
		g_clear_object (&view->priv->engine);
	}

	g_clear_object (&view->priv->store);

	G_OBJECT_CLASS (pragha_plugins_view_parent_class)->dispose (object);
}

static void
pragha_plugins_view_class_init (PraghaPluginsViewClass *klass)
{
	GType the_type = G_TYPE_FROM_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkTreeViewClass *tree_view_class = GTK_TREE_VIEW_CLASS (klass);

	object_class->set_property = pragha_plugins_view_set_property;
	object_class->get_property = pragha_plugins_view_get_property;
	object_class->constructed = pragha_plugins_view_constructed;
	object_class->dispose = pragha_plugins_view_dispose;

	widget_class->button_press_event = pragha_plugins_view_button_press_event;
	widget_class->popup_menu = pragha_plugins_view_popup_menu;
	widget_class->query_tooltip = pragha_plugins_view_query_tooltip;

	tree_view_class->row_activated = pragha_plugins_view_row_activated;

	properties_v[PROP_V_ENGINE] =
		g_param_spec_object ("engine",
		                     "engine",
		                     "The PeasEngine this view is attached to",
		                     PEAS_TYPE_ENGINE,
		                     G_PARAM_READWRITE |
		                     G_PARAM_CONSTRUCT_ONLY |
		                     G_PARAM_STATIC_STRINGS);

	properties_v[PROP_V_SHOW_BUILTIN] =
		g_param_spec_boolean ("show-builtin",
		                      "show-builtin",
		                      "If builtin plugins should be shown",
		                      FALSE,
		                      G_PARAM_READWRITE |
		                      G_PARAM_STATIC_STRINGS);

	signals[POPULATE_POPUP] =
		g_signal_new ("populate-popup",
		              the_type,
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaPluginsViewClass, populate_popup),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE,
		              1,
		              GTK_TYPE_MENU);

	g_object_class_install_properties (object_class, N_V_PROPERTIES, properties_v);

	g_type_class_add_private (object_class, sizeof (PraghaPluginsViewPrivate));
}

GtkWidget *
pragha_plugins_view_new (PeasEngine *engine)
{
	g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);

	return GTK_WIDGET (g_object_new (PRAGHA_TYPE_PLUGINS_VIEW,
	                                 "engine", engine,
	                                 NULL));
}

void
pragha_plugins_view_set_show_builtin (PraghaPluginsView *view,
                                      gboolean           show_builtin)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean iter_set;

	g_return_if_fail (PRAGHA_PLUGINS_VIEW (view));

	show_builtin = (show_builtin != FALSE);

	if (view->priv->show_builtin == show_builtin)
		return;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

	/* We must get the selected iter before setting if builtin
	   plugins should be shown so the proper model is set */
	iter_set = gtk_tree_selection_get_selected (selection, NULL, &iter);

	if (iter_set)
		convert_iter_to_child_iter (view, &iter);

	view->priv->show_builtin = show_builtin;

	if (show_builtin) {
		gtk_tree_view_set_model (GTK_TREE_VIEW (view),
		                         GTK_TREE_MODEL (view->priv->store));
    }
	else {
		GtkTreeModel *model;

		model = gtk_tree_model_filter_new (GTK_TREE_MODEL (view->priv->store), NULL);
		gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (model),
		                                        (GtkTreeModelFilterVisibleFunc) filter_builtins_visible,
		                                        view,
		                                        NULL);

		gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

		g_object_unref (model);
	}

	if (iter_set && convert_child_iter_to_iter (view, &iter))
		gtk_tree_selection_select_iter (selection, &iter);

	g_object_notify_by_pspec (G_OBJECT (view),
	                          properties_v[PROP_V_SHOW_BUILTIN]);
}

gboolean
pragha_plugins_view_get_show_builtin (PraghaPluginsView *view)
{
	g_return_val_if_fail (PRAGHA_IS_PLUGINS_VIEW (view), FALSE);

	return view->priv->show_builtin;
}

void
pragha_plugins_view_set_selected_plugin (PraghaPluginsView *view,
                                         PeasPluginInfo    *info)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	g_return_if_fail (PRAGHA_IS_PLUGINS_VIEW (view));
	g_return_if_fail (info != NULL);

	g_return_if_fail (pragha_plugins_store_get_iter_from_plugin (view->priv->store, &iter, info));

	if (!convert_child_iter_to_iter (view, &iter))
		return;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
	gtk_tree_selection_select_iter (selection, &iter);
}

PeasPluginInfo *
pragha_plugins_view_get_selected_plugin (PraghaPluginsView *view)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	PeasPluginInfo *info = NULL;

	g_return_val_if_fail (PRAGHA_IS_PLUGINS_VIEW (view), NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
	if (selection != NULL && gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		convert_iter_to_child_iter (view, &iter);
		info = pragha_plugins_store_get_plugin (view->priv->store, &iter);
	}

	return info;
}