/*************************************************************************/
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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
#include <glib/gstdio.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <gudev/gudev.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "plugins/pragha-plugin-macros.h"

#include "src/pragha-app-notification.h"
#include "src/pragha-database-provider.h"
#include "src/pragha-device-client.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-menubar.h"
#include "src/pragha-utils.h"
#include "src/pragha-simple-widgets.h"
#include "src/pragha-background-task-bar.h"
#include "src/pragha-background-task-widget.h"
#include "src/pragha-window.h"
#include "src/pragha-hig.h"
#include "src/pragha.h"

#include "pragha-mtp-musicobject.h"
#include "pragha-mtp-thread.h"
#include "pragha-mtp-thread-data.h"

#define PRAGHA_TYPE_MTP_PLUGIN         (pragha_mtp_plugin_get_type ())
#define PRAGHA_MTP_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPlugin))
#define PRAGHA_MTP_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPlugin))
#define PRAGHA_IS_MTP_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_MTP_PLUGIN))
#define PRAGHA_IS_MTP_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_MTP_PLUGIN))
#define PRAGHA_MTP_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPluginClass))

typedef struct _PraghaMtpPluginPrivate PraghaMtpPluginPrivate;

struct _PraghaMtpPluginPrivate {
	PraghaApplication  *pragha;

	PraghaDeviceClient *device_client;

	PraghaMtpThread    *device_thread;

	guint64             bus_hooked;
	guint64             device_hooked;

	GCancellable       *cancellable;

	gchar              *friend_name;
	gchar              *device_id;

	PraghaMtpThreadDownloadData *dw_data;

	GHashTable         *tracks_table;

	GtkActionGroup     *action_group_menu;
	guint               merge_id_menu;

	GtkActionGroup     *action_group_playlist;
	guint               merge_id_playlist;

	GtkWidget                  *ask_dialog;
	PraghaBackgroundTaskWidget *task_widget;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_MTP_PLUGIN,
                        PraghaMtpPlugin,
                        pragha_mtp_plugin)


/*
 * Forward declarations
 */
static void
pragha_mtp_plugin_append_menu_action (PraghaMtpPlugin *plugin);

static void
pragha_mtp_plugin_remove_menu_action (PraghaMtpPlugin *plugin);

static void
pragha_mtp_detected_ask_action (PraghaMtpPlugin *plugin);


/*
 * Menu Actions.
 */
static void pragha_mtp_action_send_to_device    (GtkAction *action, PraghaMtpPlugin *plugin);
static void pragha_mtp_action_disconnect_device (GtkAction *action, PraghaMtpPlugin *plugin);
static void pragha_mtp_action_show_device_info  (GtkAction *action, PraghaMtpPlugin *plugin);

static const GtkActionEntry mtp_sendto_actions [] = {
	{"Send to MTP", NULL, "Fake MTP device",
	 "", "Send to MTP", G_CALLBACK(pragha_mtp_action_send_to_device)},
};

static const gchar *mtp_sendto_xml = "<ui>					\
	<popup name=\"SelectionPopup\">						\
	<menu action=\"SendToMenu\">						\
		<placeholder name=\"pragha-sendto-placeholder\">		\
			<menuitem action=\"Send to MTP\"/>			\
			<separator/>						\
		</placeholder>							\
	</menu>									\
	</popup>								\
</ui>";

static const GtkActionEntry mtp_menu_actions [] = {
	{"MtpDevice", "multimedia-player", "Fake MTP device"},
	{"Disconnect device", NULL, N_("Disconnect device"),
	"", "Disconnect device", G_CALLBACK(pragha_mtp_action_disconnect_device)},
	{"Show device info", "dialog-information", N_("Show device info"),
	"", "Show device info", G_CALLBACK(pragha_mtp_action_show_device_info)},
};

static const gchar *mtp_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">						\
		<menu action=\"ToolsMenu\">					\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menu action=\"MtpDevice\">			\
					<menuitem action=\"Disconnect device\"/>	\
					<separator/>				\
					<menuitem action=\"Show device info\"/>	\
				</menu>						\
				<separator/>					\
			</placeholder>						\
		</menu>								\
	</menubar>								\
</ui>";

/*
 * Gear Menu.
 */
static void
pragha_gmenu_mtp_disconnect_device_action (GSimpleAction *action,
                                           GVariant      *parameter,
                                           gpointer       user_data)
{
	pragha_mtp_action_disconnect_device (NULL, PRAGHA_MTP_PLUGIN(user_data));
}

static void
pragha_gmenu_mtp_show_device_info_action (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
	pragha_mtp_action_show_device_info (NULL, PRAGHA_MTP_PLUGIN(user_data));
}

static GActionEntry mtp_entries[] = {
	{ "mtp-disconnect", pragha_gmenu_mtp_disconnect_device_action, NULL, NULL, NULL },
	{ "mtp-info",       pragha_gmenu_mtp_show_device_info_action,  NULL, NULL, NULL }
};

static const gchar *mtp_menu_ui = \
	"<interface>" \
	"	<menu id='menubar'>" \
	"		<section>" \
	"			<section id='pragha-plugins-placeholder'>"	\
	"				<submenu id='mtp-sudmenu'>" \
	"					<attribute name='label' translatable='yes'>Unknown MTP device</attribute>" \
	"					<section>" \
	"						<item>" \
	"							<attribute name='label' translatable='yes'>Disconnect library</attribute>" \
	"							<attribute name='action'>win.mtp-disconnect</attribute>" \
	"						</item>" \
	"						<item>" \
	"							<attribute name='label' translatable='yes'>Show device info</attribute>" \
	"							<attribute name='action'>win.mtp-info</attribute>" \
	"						</item>" \
	"					</section>" \
	"				</submenu>" \
	"			</section>" \
	"		</section>" \
	"	</menu>" \
	"</interface>";


/*
 *  Some utils.
 */

static void
pragha_mtp_clear_hook_device (PraghaMtpPlugin *plugin)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	if (priv->bus_hooked)
		priv->bus_hooked = 0;

	if (priv->device_hooked)
		priv->device_hooked = 0;

	if (priv->device_id) {
		g_free (priv->device_id);
		priv->device_id = NULL;
	}

	if (priv->friend_name) {
		g_free (priv->friend_name);
		priv->friend_name = NULL;
	}
}


/*
 * MTP thread responses and show the results to the user.
 */

static gboolean
pragha_mtp_plugin_device_opened_idle (PraghaMtpThreadOpenedData *data)
{
	PraghaAppNotification *notification;
	const gchar *device_id = NULL, *friendly_name;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(pragha_mtp_thread_opened_data_get_user_data (data));
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	device_id = pragha_mtp_thread_opened_data_get_device_id (data);
	friendly_name = pragha_mtp_thread_opened_data_get_friendly_name (data);

	if (!device_id) {
		CDEBUG(DBG_INFO, "Mtp plugin fail to open device...");
		notification = pragha_app_notification_new (_("MTP Device"), _("Failed to open the MTP device. Try again."));
		pragha_app_notification_show (notification);
		pragha_mtp_thread_opened_data_free (data);
		pragha_mtp_clear_hook_device (plugin);

		return FALSE;
	}

	priv->device_id = g_strdup (device_id);
	priv->friend_name = g_strdup (friendly_name);

	pragha_mtp_detected_ask_action (plugin);

	pragha_mtp_thread_opened_data_free (data);

	return FALSE;
}

static gboolean
pragha_mtp_plugin_close_device_idle (gpointer user_data)
{
	PraghaDatabaseProvider *provider;
	PraghaMusicEnum *enum_map = NULL;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(user_data);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Remove music on playlist */

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_remove (enum_map, "MTP");
	g_object_unref (enum_map);

	/* Hide the provider but leave it as backup */

	provider = pragha_database_provider_get ();
	pragha_provider_set_visible (provider, priv->device_id, FALSE);
	pragha_provider_update_done (provider);
	g_object_unref (provider);

	/* Remove cache and clear the rest */

	pragha_mtp_plugin_remove_menu_action (plugin);
	pragha_mtp_clear_hook_device (plugin);

	return FALSE;
}

static gboolean
pragha_mtp_plugin_tracklist_progress_idle (PraghaMtpThreadProgressData *data)
{
	gchar *description = NULL;
	guint progress, total;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(pragha_mtp_thread_progress_data_get_user_data (data));
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	progress = pragha_mtp_thread_progress_data_get_progress (data);
	total = pragha_mtp_thread_progress_data_get_total (data);

	CDEBUG(DBG_PLUGIN, "Tracklist progress: %i of %i", progress, total);

	description = g_strdup_printf(_("%i files found in the %s device"), progress, priv->friend_name);
	pragha_background_task_widget_set_description (priv->task_widget, description);
	g_free(description);

	pragha_mtp_thread_progress_data_free(data);

	return FALSE;
}

static gboolean
pragha_mtp_plugin_tracklist_loaded_idle (PraghaMtpThreadTracklistData *data)
{
	PraghaBackgroundTaskBar *taskbar;
	PraghaDatabase *database;
	PraghaDatabaseProvider *provider;
	PraghaAppNotification *notification;
	PraghaMusicobject *mobj;
	GList *list, *l;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(pragha_mtp_thread_tracklist_data_get_user_data (data));
	PraghaMtpPluginPrivate *priv = plugin->priv;

	list = pragha_mtp_thread_tracklist_data_get_list (data);

	CDEBUG(DBG_PLUGIN, "Mtp plugin tracklist has %i tracks", g_list_length (list));

	/* Save to database
	 * TODO: Merge changes instead replace songs.
	 */

	database = pragha_database_get ();
	provider = pragha_database_provider_get ();
	if (pragha_database_find_provider (database, priv->device_id)) {
		pragha_provider_forget_songs (provider, priv->device_id);
	}
	else {
		pragha_provider_add_new (provider,
		                         priv->device_id,
		                         "MTP",
		                         priv->friend_name,
		                         "multimedia-player");
	}

	for (l = list; l != NULL; l = l->next) {
		mobj = PRAGHA_MUSICOBJECT(l->data);
		if (G_LIKELY(mobj))
			pragha_database_add_new_musicobject (database, mobj);
	}

	/* Inform to user */

	taskbar = pragha_background_task_bar_get ();
	pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
	g_object_unref(G_OBJECT(taskbar));

	notification = pragha_app_notification_new (priv->friend_name, _("You can interact with your MTP device"));
	pragha_app_notification_set_timeout (notification, 5);
	pragha_app_notification_show (notification);

	pragha_mtp_plugin_append_menu_action (plugin);

	pragha_provider_set_visible (provider, priv->device_id, TRUE);
	pragha_provider_update_done (provider);

	/* Clean */

	g_object_unref (database);
	g_object_unref (provider);

	pragha_mtp_thread_tracklist_data_free(data);

	return FALSE;
}

static gboolean
pragha_mtp_plugin_device_download_idle (PraghaMtpThreadDownloadData *data)
{
	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(pragha_mtp_thread_download_data_get_user_data (data));
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	priv->dw_data = data;

	return FALSE;
}

static gboolean
pragha_mtp_plugin_send_to_device_idle (PraghaMtpThreadUploadData *data)
{
	PraghaDatabase *database;
	PraghaDatabaseProvider *provider;
	PraghaAppNotification *notification;
	PraghaMusicobject *mobj;
	const gchar *error;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(pragha_mtp_thread_upload_data_get_user_data (data));
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	error = pragha_mtp_thread_upload_data_get_error (data);
	if (error) {
		notification = pragha_app_notification_new (priv->friend_name, _("Failed to send the song to the device."));
		pragha_app_notification_show (notification);
		pragha_mtp_thread_upload_data_free (data);
		return FALSE;
	}

	mobj = pragha_mtp_thread_upload_data_get_musicobject (data);
	if (G_LIKELY(mobj)) {
		database = pragha_database_get ();
		pragha_database_add_new_musicobject (database, mobj);
		g_object_unref(database);

		notification = pragha_app_notification_new (priv->friend_name, _("The song was uploaded to your device."));
		pragha_app_notification_set_timeout (notification, 5);
		pragha_app_notification_show (notification);

		provider = pragha_database_provider_get ();
		pragha_provider_update_done (provider);
		g_object_unref(provider);
	}

	pragha_mtp_thread_upload_data_free (data);

	return FALSE;
}

static gboolean
pragha_mtp_action_show_device_info_idle (PraghaMtpThreadStatsData *data)
{
	PraghaAppNotification *notification;
	PraghaHeader *header;
	GtkWidget *dialog, *table, *label;
	gchar *storage_size = NULL, *storage_free = NULL, *storage_string = NULL;
	guint64 storage_free_space, storage_capacity;
	guint8 current_battery_level = 0, maximum_battery_level = 0;
	gchar *battery_string;
	guint row = 0;
	const gchar *error;

	PraghaMtpPlugin *plugin = pragha_mtp_thread_stats_data_get_user_data (data);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	error = pragha_mtp_thread_stats_data_get_error (data);
	if (error) {
		CDEBUG(DBG_INFO, "Mtp plugin get stats hass some error: %s", error);
		notification = pragha_app_notification_new (priv->friend_name, _("We could not get the device information."));
		pragha_app_notification_show (notification);
		pragha_mtp_thread_stats_data_free (data);
		return FALSE;
	}

	dialog = gtk_dialog_new_with_buttons (priv->friend_name,
	                                      GTK_WINDOW(pragha_application_get_window (priv->pragha)),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Ok"), GTK_RESPONSE_OK,
	                                      NULL);

	header = pragha_header_new ();
	pragha_header_set_title (header, priv->friend_name);
	pragha_header_set_icon_name (header, "multimedia-player");

	table = pragha_hig_workarea_table_new ();

	if (pragha_mtp_thread_stats_data_get_first_storage_description(data)) {
		pragha_hig_workarea_table_add_section_title (table, &row,
			pragha_mtp_thread_stats_data_get_first_storage_description(data));

		storage_free_space = pragha_mtp_thread_stats_data_get_first_storage_free_space (data);
		storage_capacity = pragha_mtp_thread_stats_data_get_first_storage_capacity (data);

		storage_free = g_format_size (storage_free_space);
		storage_size = g_format_size (storage_capacity);

		storage_string = g_strdup_printf (_("%s free of %s (%d%% used)"),
		                                  storage_free, storage_size,
		                                  (gint) ((storage_capacity - storage_free_space) * 100 / storage_capacity));

		label = gtk_label_new_with_mnemonic (storage_string);

		pragha_hig_workarea_table_add_wide_control (table, &row, label);

		g_free (storage_free);
		g_free (storage_size);
		g_free (storage_string);
	}

	if (pragha_mtp_thread_stats_data_get_second_storage_description(data)) {
		pragha_hig_workarea_table_add_section_title (table, &row,
			pragha_mtp_thread_stats_data_get_second_storage_description(data));

		storage_free_space = pragha_mtp_thread_stats_data_get_second_storage_free_space (data);
		storage_capacity = pragha_mtp_thread_stats_data_get_second_storage_capacity (data);

		storage_free = g_format_size (storage_free_space);
		storage_size = g_format_size (storage_capacity);

		storage_string = g_strdup_printf (_("%s free of %s (%d%% used)"),
		                                  storage_free, storage_size,
		                                  (gint) ((storage_capacity - storage_free_space) * 100 / storage_capacity));

		label = gtk_label_new_with_mnemonic (storage_string);

		pragha_hig_workarea_table_add_wide_control (table, &row, label);

		g_free (storage_free);
		g_free (storage_size);
		g_free (storage_string);
	}

	if (pragha_mtp_thread_stats_data_get_maximun_battery_level (data)) {
		pragha_hig_workarea_table_add_section_title (table, &row, _("Battery level"));

		current_battery_level = pragha_mtp_thread_stats_data_get_current_battery_level (data);
		maximum_battery_level = pragha_mtp_thread_stats_data_get_maximun_battery_level (data);

		battery_string = g_strdup_printf (_("%d%%"), (int) ((float)current_battery_level / (float)maximum_battery_level * 100));

		label = gtk_label_new_with_mnemonic (battery_string);

		pragha_hig_workarea_table_add_wide_control (table, &row, label);

		g_free (battery_string);
	}

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), GTK_WIDGET(header), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), table, TRUE, TRUE, 0);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show_all (dialog);

	pragha_mtp_thread_stats_data_free (data);

	return FALSE;
}


/*
 * User actions.
 */

static void
pragha_mtp_detected_ask_action_response (GtkWidget *dialog,
                                         gint       response,
                                         gpointer   user_data)
{
	PraghaBackgroundTaskBar *taskbar;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Destroy first due cache track is slow in main thread */
	gtk_widget_destroy (dialog);

	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			taskbar = pragha_background_task_bar_get ();
			pragha_background_task_bar_prepend_widget (taskbar, GTK_WIDGET(priv->task_widget));
			g_object_unref(G_OBJECT(taskbar));

			pragha_mtp_thread_get_track_list (priv->device_thread,
			                                  G_SOURCE_FUNC (pragha_mtp_plugin_tracklist_loaded_idle),
			                                  G_SOURCE_FUNC (pragha_mtp_plugin_tracklist_progress_idle),
			                                  plugin);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			pragha_mtp_clear_hook_device (plugin);
			break;
	}
}

static void
pragha_mtp_detected_ask_action (PraghaMtpPlugin *plugin)
{
	GtkWidget *dialog;
	gchar *message = NULL;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	message = g_strdup_printf (_("Do you want to manage the “%s” device with Pragha?"), priv->friend_name);

	dialog = pragha_gudev_dialog_new (pragha_application_get_window (priv->pragha),
	                                  _("MTP Device"), "multimedia-player",
	                                  _("An MTP device was detected"), message,
	                                  _("Manage device"), PRAGHA_DEVICE_RESPONSE_PLAY);

	g_free(message);

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_mtp_detected_ask_action_response), plugin);

	priv->ask_dialog = dialog;

	gtk_widget_show_all (dialog);
}

static void
pragha_mtp_action_send_to_device (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	playlist = pragha_application_get_playlist (priv->pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	if (!mobj)
		return;

	pragha_mtp_thread_upload_track (priv->device_thread,
	                                mobj,
	                                G_SOURCE_FUNC (pragha_mtp_plugin_send_to_device_idle),
	                                plugin);
}

static void
pragha_mtp_action_disconnect_device (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	pragha_mtp_thread_close_device (priv->device_thread,
	                                pragha_mtp_plugin_close_device_idle,
	                                plugin);
}

static void
pragha_mtp_action_show_device_info (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	pragha_mtp_thread_get_stats (priv->device_thread,
	                             G_SOURCE_FUNC (pragha_mtp_action_show_device_info_idle),
	                             plugin);
}

/*
 *  PraghaBackend signals to playback.
 */
static void
pragha_mtp_plugin_clean_source (PraghaBackend *backend, gpointer user_data)
{
	PraghaMusicobject *mobj;
	gchar *tmp_filename = NULL;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_mtp_file (mobj))
		return;

	tmp_filename = pragha_mtp_plugin_get_temp_filename(mobj);
	g_unlink (tmp_filename);
	g_free (tmp_filename);
}

static void
pragha_mtp_plugin_prepare_source (PraghaBackend *backend, gpointer user_data)
{
	PraghaAppNotification *notification;
	PraghaMusicobject *mobj;
	gchar *tmp_filename = NULL, *uri = NULL;
	const gchar *error_msg = NULL;
	gint track_id;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_mtp_file (mobj))
		return;

	tmp_filename = pragha_mtp_plugin_get_temp_filename(mobj);
	track_id = pragha_mtp_plugin_get_track_id(mobj);

	pragha_mtp_thread_download_track (priv->device_thread,
	                                  track_id,
	                                  tmp_filename,
	                                  G_SOURCE_FUNC (pragha_mtp_plugin_device_download_idle),
	                                  NULL,
	                                  plugin);

	CDEBUG(DBG_PLUGIN, "Mtp plugin waiting until download track done.");

	// FIXME: Maybe an ugly hack to syncronize download.
	while (priv->dw_data == NULL) {
		pragha_process_gtk_events ();
	}

	error_msg = pragha_mtp_thread_download_data_get_error(priv->dw_data);
	if (error_msg) {
		CDEBUG(DBG_INFO, "Mtp plugin download track with some error: %s", error_msg);
		notification = pragha_app_notification_new (priv->friend_name, _("Failed to download the song from device."));
		pragha_app_notification_show (notification);
	}
	else {
		CDEBUG(DBG_INFO, "Mtp plugin download done. Try to reproduce it: %s", tmp_filename);
		uri = g_filename_to_uri (tmp_filename, NULL, NULL);
		pragha_backend_set_playback_uri (backend, uri);
		g_free(uri);
	}

	pragha_mtp_thread_download_data_free(priv->dw_data);
	g_free (tmp_filename);

	// Download and playback done.
	priv->dw_data = NULL;
}


/*
 * PraghaDevices signals to handle device conections
 */
static void
pragha_mtp_plugin_device_added (PraghaDeviceClient *device_client,
                                PraghaDeviceType    device_type,
                                GUdevDevice        *u_device,
                                gpointer            user_data)
{
	PraghaAppNotification *notification;
	LIBMTP_raw_device_t *raw_devices, *raw_device = NULL;
	gint busnum = 0, devnum = 0, num_raw_devices = 0, i = 0;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	if (priv->device_hooked  != 0)
		return;

	if (device_type != PRAGHA_DEVICE_MTP)
		return;

	/* Get devices.. */

	busnum = g_udev_device_get_property_as_int (u_device, "BUSNUM");
	devnum = pragha_gudev_get_property_as_int (u_device, "DEVNUM", 10);

	notification = pragha_app_notification_new (_("MTP Device"), _("An MTP device was connected"));
	pragha_app_notification_set_timeout (notification, 5);
	pragha_app_notification_show (notification);

	/* Partial hook device */

	priv->bus_hooked = busnum;
	priv->device_hooked = devnum;

	/* Open device */

	pragha_mtp_thread_open_device (priv->device_thread,
	                               devnum, busnum,
	                               G_SOURCE_FUNC (pragha_mtp_plugin_device_opened_idle),
	                               plugin);
}

void
pragha_mtp_plugin_device_removed (PraghaDeviceClient *device_client,
                                  PraghaDeviceType    device_type,
                                  GUdevDevice        *u_device,
                                  gpointer            user_data)
{
	PraghaAppNotification *notification;
	guint64 busnum = 0;
	guint64 devnum = 0;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN(user_data);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	busnum = g_udev_device_get_property_as_uint64(u_device, "BUSNUM");
	devnum = pragha_gudev_get_property_as_int (u_device, "DEVNUM", 10);

	if (busnum == priv->bus_hooked && devnum == priv->device_hooked)
	{
		notification = pragha_app_notification_new (priv->friend_name, _("The device was disconnected."));
		pragha_app_notification_show (notification);

		if (priv->ask_dialog) {
			gtk_widget_destroy (priv->ask_dialog);
			priv->ask_dialog = NULL;
		}

		pragha_mtp_thread_close_device (priv->device_thread,
		                                pragha_mtp_plugin_close_device_idle,
		                                plugin);
	}
}


/*
 *  PraghaMtpPlugin implementation.
 */

static void
pragha_mtp_plugin_append_menu_action (PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;
	GtkActionGroup *action_group;
	GtkAction *action;
	GActionMap *map;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Menubar tools. */

	action_group = gtk_action_group_new ("PraghaMenubarMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_menu_actions,
	                              G_N_ELEMENTS (mtp_menu_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "MtpDevice");
	gtk_action_set_label(GTK_ACTION(action), priv->friend_name);

	priv->merge_id_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                           action_group,
	                                                           mtp_menu_xml);
	priv->action_group_menu = action_group;

	/* Gear Menu */

	pragha_menubar_append_submenu (priv->pragha, "pragha-plugins-placeholder",
	                               mtp_menu_ui,
	                               "mtp-sudmenu",
	                               priv->friend_name,
	                               plugin);

	map = G_ACTION_MAP (pragha_application_get_window(priv->pragha));
	g_action_map_add_action_entries (G_ACTION_MAP (map),
	                                 mtp_entries, G_N_ELEMENTS(mtp_entries),
	                                 plugin);

	/* Playlist sendto */

	action_group = gtk_action_group_new ("PraghaPlaylistMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_sendto_actions,
	                              G_N_ELEMENTS (mtp_sendto_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "Send to MTP");
	gtk_action_set_label(GTK_ACTION(action), priv->friend_name);

	playlist = pragha_application_get_playlist (priv->pragha);
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                action_group,
	                                                                mtp_sendto_xml);
	priv->action_group_playlist = action_group;
}

static void
pragha_mtp_plugin_remove_menu_action (PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	if (!priv->merge_id_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_menu,
	                                     priv->merge_id_menu);
	priv->merge_id_menu = 0;

	if (!priv->merge_id_playlist)
		return;

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);
	priv->merge_id_playlist = 0;

	pragha_menubar_remove_by_id (priv->pragha,
	                             "pragha-plugins-placeholder",
	                             "mtp-sudmenu");
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaBackend *backend;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN (activatable);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	priv->tracks_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	priv->device_thread = pragha_mtp_thread_new ();

	/* New Task widget */

	priv->cancellable = g_cancellable_new ();

	priv->task_widget = pragha_background_task_widget_new (_("Searching files to analyze"),
	                                                       "multimedia-player",
	                                                       0,
	                                                       priv->cancellable);
	g_object_ref (G_OBJECT(priv->task_widget));

	/* Connect signals */

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_mtp_plugin_prepare_source), plugin);
	g_signal_connect (backend, "clean-source",
	                  G_CALLBACK(pragha_mtp_plugin_clean_source), plugin);

	priv->device_client = pragha_device_client_get();
	g_signal_connect (G_OBJECT(priv->device_client), "device-added",
	                  G_CALLBACK(pragha_mtp_plugin_device_added), plugin);
	g_signal_connect (G_OBJECT(priv->device_client), "device-removed",
	                  G_CALLBACK(pragha_mtp_plugin_device_removed), plugin);

}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDatabaseProvider *provider;
	PraghaBackend *backend;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN (activatable);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Remove provider if user disable the plugin or hide it */

	provider = pragha_database_provider_get ();
	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha)))
	{
		if (priv->device_id)
		{
			pragha_provider_remove (provider,
			                        priv->device_id);
			pragha_provider_update_done (provider);
		}
	}
	else
	{
		if (priv->device_id)
		{
			pragha_provider_set_visible (provider, priv->device_id, FALSE);
		}
	}
	g_object_unref (provider);

	g_object_unref (priv->device_thread);

	/* Remove cache and clear the rest */

	pragha_mtp_plugin_remove_menu_action (plugin);
	pragha_mtp_clear_hook_device (plugin);

	g_hash_table_destroy (priv->tracks_table);
	g_object_unref(priv->cancellable);

	/* Disconnect signals */

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_handlers_disconnect_by_func (backend, pragha_mtp_plugin_prepare_source, plugin);
	g_signal_handlers_disconnect_by_func (backend, pragha_mtp_plugin_clean_source, plugin);

	g_signal_handlers_disconnect_by_func (priv->device_client,
	                                      pragha_mtp_plugin_device_added,
	                                      plugin);
	g_signal_handlers_disconnect_by_func (priv->device_client,
	                                      pragha_mtp_plugin_device_removed,
	                                      plugin);
	g_object_unref (priv->device_client);

	priv->pragha = NULL;
}
