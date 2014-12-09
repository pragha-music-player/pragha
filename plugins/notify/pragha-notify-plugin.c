/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2014 matias <mati86dl@gmail.com>                   */
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

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include <libnotify/notify.h>

#include "plugins/pragha-plugin-macros.h"

#include "src/pragha.h"
#include "src/pragha-plugin-object.h"
#include "src/pragha-hig.h"
#include "src/pragha-playback.h"
#include "src/pragha-utils.h"
#include "src/pragha-preferences-dialog.h"

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

#define PRAGHA_TYPE_NOTIFY_PLUGIN         (pragha_notify_plugin_get_type ())
#define PRAGHA_NOTIFY_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_NOTIFY_PLUGIN, PraghaNotifyPlugin))
#define PRAGHA_NOTIFY_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_NOTIFY_PLUGIN, PraghaNotifyPlugin))
#define PRAGHA_IS_NOTIFY_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_NOTIFY_PLUGIN))
#define PRAGHA_IS_NOTIFY_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_NOTIFY_PLUGIN))
#define PRAGHA_NOTIFY_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_NOTIFY_PLUGIN, PraghaNotifyPluginClass))

typedef struct {
	PraghaPluginObject *object;
	GtkWidget          *setting_widget;

	NotifyNotification *notify;

	GtkWidget          *album_art_in_osd_w;
	GtkWidget          *actions_in_osd_w;
	gboolean            album_art_in_osd;
	gboolean            actions_in_osd;
} PraghaNotifyPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_NOTIFY_PLUGIN,
                        PraghaNotifyPlugin,
                        pragha_notify_plugin)

static gboolean
can_support_actions (void)
{
	static gboolean supported;
	static gboolean have_checked = FALSE;

	if( !have_checked ){
		GList * c;
		GList * caps = notify_get_server_caps( );

		have_checked = TRUE;

		for( c=caps; c && !supported; c=c->next )
			supported = !strcmp( "actions", (char*)c->data );

		g_list_free_full( caps, g_free );
	}

	return supported;
}

static void
notify_closed_cb (NotifyNotification *osd,
                  PraghaNotifyPlugin *plugin)
{
	g_object_unref (G_OBJECT(osd));

	if (plugin->priv->notify == osd) {
		plugin->priv->notify = NULL;
	}
}

static void
notify_Prev_Callback (NotifyNotification *osd,
                      const char *action,
                      PraghaNotifyPlugin *plugin)
{
	PraghaBackend *backend;

	g_assert (action != NULL);

	PraghaApplication *pragha = pragha_plugin_object_get_pragha(plugin->priv->object);

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_prev_track(pragha);
}

static void
notify_Next_Callback (NotifyNotification *osd,
                      const char         *action,
                      PraghaNotifyPlugin *plugin)
{
	PraghaBackend *backend;

	g_assert (action != NULL);

	PraghaApplication *pragha = pragha_plugin_object_get_pragha(plugin->priv->object);

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_next_track(pragha);
}

void
pragha_notify_plugin_show_new_track (PraghaPlaylist     *playlist,
                                     PraghaMusicobject  *mobj,
                                     PraghaNotifyPlugin *plugin)
{
	PraghaNotifyPluginPrivate *priv = NULL;
	PraghaToolbar *toolbar;
	gchar *summary, *body, *slength;
	GError *error = NULL;

	priv = plugin->priv;

	if (NULL == mobj)
		return;

	if (gtk_window_is_active(GTK_WINDOW (pragha_application_get_window(pragha_plugin_object_get_pragha(priv->object)))))
		return;

	const gchar *file = pragha_musicobject_get_file (mobj);
	const gchar *title = pragha_musicobject_get_title (mobj);
	const gchar *artist = pragha_musicobject_get_artist (mobj);
	const gchar *album = pragha_musicobject_get_album (mobj);
	gint length = pragha_musicobject_get_length (mobj);

	if(string_is_not_empty(title))
		summary = g_strdup(title);
	else
		summary = g_path_get_basename(file);

	slength = convert_length_str(length);

	body = g_markup_printf_escaped(_("by <b>%s</b> in <b>%s</b> <b>(%s)</b>"),
	                               string_is_not_empty(artist) ? artist : _("Unknown Artist"),
	                               string_is_not_empty(album) ? album : _("Unknown Album"),
	                               slength);

	/* Create notification instance */

	if (priv->notify == NULL) {
		#if NOTIFY_CHECK_VERSION (0, 7, 1)
		priv->notify = notify_notification_new(summary, body, NULL);
		#else
		priv->notify = notify_notification_new(summary, body, NULL, NULL);
		#endif

		if (can_support_actions() && priv->actions_in_osd) {
			notify_notification_add_action(
				priv->notify, "media-skip-backward", _("Previous track"),
				NOTIFY_ACTION_CALLBACK(notify_Prev_Callback), plugin,
				NULL);
			notify_notification_add_action(
				priv->notify, "media-skip-forward", _("Next track"),
				NOTIFY_ACTION_CALLBACK(notify_Next_Callback), plugin,
				NULL);
		}
		notify_notification_set_hint (priv->notify, "transient", g_variant_new_boolean (TRUE));
		g_signal_connect (priv->notify, "closed", G_CALLBACK (notify_closed_cb), plugin);
	}
	else {
		notify_notification_update (priv->notify, summary, body, NULL);

		if (!priv->actions_in_osd)
			notify_notification_clear_actions (priv->notify);
	}

	/* Add album art if set */
	if (priv->album_art_in_osd) {
		toolbar = pragha_application_get_toolbar (pragha_plugin_object_get_pragha(priv->object));
		notify_notification_set_icon_from_pixbuf (priv->notify,
			pragha_album_art_get_pixbuf (pragha_toolbar_get_album_art(toolbar)));
	}

	/* Show OSD */
	if (!notify_notification_show (priv->notify, &error)) {
		g_warning("Unable to show OSD notification: %s", error->message);
		g_error_free (error);
	}

	/* Cleanup */

	g_free(summary);
	g_free(body);
	g_free(slength);
}

static void
pragha_notify_preferences_dialog_response (GtkDialog          *dialog,
                                           gint                response_id,
                                           PraghaNotifyPlugin *plugin)
{
	PraghaPreferences *preferences;
	gchar *plugin_group = NULL;

	PraghaNotifyPluginPrivate *priv = plugin->priv;

	switch(response_id) {
		case GTK_RESPONSE_CANCEL:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->album_art_in_osd_w),
			                              priv->album_art_in_osd);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->actions_in_osd_w),
			                              priv->actions_in_osd);
			break;
		case GTK_RESPONSE_OK:
			priv->album_art_in_osd =
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->album_art_in_osd_w));
			priv->actions_in_osd =
				gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->actions_in_osd_w));

			preferences = pragha_preferences_get ();
			plugin_group = pragha_preferences_get_plugin_group_name(preferences, "notify");

			pragha_preferences_set_boolean (preferences,
			                                plugin_group, "album_art_in_osd",
			                                priv->album_art_in_osd);
			pragha_preferences_set_boolean (preferences,
			                                plugin_group, "actions_in_osd",
			                                priv->actions_in_osd);

			g_object_unref (preferences);
			g_free (plugin_group);
			break;
		default:
			break;
	}
}

static void
pragha_notify_plugin_append_setting (PraghaNotifyPlugin *plugin)
{
	PreferencesDialog *dialog;
	GtkWidget *table, *albumart_in_osd, *actions_in_osd;
	guint row = 0;

	PraghaNotifyPluginPrivate *priv = plugin->priv;

	table = pragha_hig_workarea_table_new ();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Notifications"));

	albumart_in_osd = gtk_check_button_new_with_label(_("Show Album art in notifications"));
	pragha_hig_workarea_table_add_wide_control(table, &row, albumart_in_osd);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(albumart_in_osd), priv->album_art_in_osd);

	actions_in_osd = gtk_check_button_new_with_label(_("Add actions to change track in notifications"));
	pragha_hig_workarea_table_add_wide_control(table, &row, actions_in_osd);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(actions_in_osd), priv->actions_in_osd);

	if (!can_support_actions())
		gtk_widget_set_sensitive (actions_in_osd, FALSE);

	priv->setting_widget = table;
	priv->album_art_in_osd_w = albumart_in_osd;
	priv->actions_in_osd_w = actions_in_osd;

	dialog = pragha_application_get_preferences_dialog (pragha_plugin_object_get_pragha(priv->object));
	pragha_preferences_append_desktop_setting (dialog, table, FALSE);

	/* Configure handler and settings */
	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_notify_preferences_dialog_response),
	                                           plugin);
}

static void
pragha_notify_plugin_remove_setting (PraghaNotifyPlugin *plugin)
{
	PreferencesDialog *dialog;
	PraghaNotifyPluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (pragha_plugin_object_get_pragha(priv->object));

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_notify_preferences_dialog_response),
	                                              plugin);
	pragha_preferences_remove_desktop_setting (dialog, priv->setting_widget);
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaPreferences *preferences;
	PraghaPlaylist *playlist;
	gchar *plugin_group = NULL;

	PraghaNotifyPlugin *plugin = PRAGHA_NOTIFY_PLUGIN (activatable);
	PraghaNotifyPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Notify plugin %s", G_STRFUNC);

	priv->object = g_object_get_data (G_OBJECT (plugin), "object");

	notify_init (PACKAGE_NAME);

	preferences = pragha_application_get_preferences (pragha_plugin_object_get_pragha(priv->object));
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "notify");
	priv->actions_in_osd =
		pragha_preferences_get_boolean (preferences,
		                                plugin_group,
		                                "actions_in_osd");
	priv->album_art_in_osd =
		pragha_preferences_get_boolean (preferences,
		                                plugin_group,
		                                "album_art_in_osd");

	playlist = pragha_application_get_playlist (pragha_plugin_object_get_pragha(priv->object));
	g_signal_connect (playlist, "playlist-set-track",
	                  G_CALLBACK(pragha_notify_plugin_show_new_track), plugin);

	pragha_notify_plugin_append_setting (plugin);

	g_free (plugin_group);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaPlaylist *playlist;

	PraghaNotifyPlugin *plugin = PRAGHA_NOTIFY_PLUGIN (activatable);
	PraghaNotifyPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Notify plugin %s", G_STRFUNC);

	notify_uninit ();

	playlist = pragha_application_get_playlist (pragha_plugin_object_get_pragha(priv->object));
	g_signal_handlers_disconnect_by_func (playlist,
	                                      pragha_notify_plugin_show_new_track,
	                                      plugin);

	pragha_notify_plugin_remove_setting (plugin);

	priv->object = NULL;
}
