/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-statusicon.h"
#include "pragha-playback.h"
#include "pragha-menubar.h"
#include "pragha.h"

struct _PraghaStatusIcon {
	GtkStatusIcon __parent__;

	PraghaApplication *pragha;

	GtkUIManager   *ui_manager;
};

G_DEFINE_TYPE(PraghaStatusIcon, pragha_status_icon, GTK_TYPE_STATUS_ICON)

static void systray_about_action        (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_open_file_action    (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_add_audio_cd_action (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_add_location_action (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_play_pause_action   (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_stop_action         (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_prev_action         (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_next_action         (GtkAction *action, PraghaStatusIcon *status_icon);
static void systray_quit                (GtkAction *action, PraghaStatusIcon *status_icon);

static const gchar *systray_menu_xml =
	"<ui>						\
	<popup>						\
		<menuitem action=\"About\"/>		\
		<separator/>				\
		<menuitem action=\"Add files\"/>	\
		<menuitem action=\"Add Audio CD\"/>	\
		<menuitem action=\"Add location\"/>	\
		<separator/>				\
		<menuitem action=\"Prev\"/>		\
		<menuitem action=\"Play_Pause\"/>	\
		<menuitem action=\"Stop\"/>		\
		<menuitem action=\"Next\"/>		\
		<separator/>				\
		<menuitem action=\"Edit tags\"/>	\
		<separator/>				\
		<menuitem action=\"Quit\"/>		\
	</popup>					\
	</ui>";

static const GtkActionEntry systray_menu_aentries[] = {
	{"About", GTK_STOCK_ABOUT, N_("About"),
	 "", NULL, G_CALLBACK(systray_about_action)},
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 "", NULL, G_CALLBACK(systray_open_file_action)},
	{"Add Audio CD", GTK_STOCK_CDROM, N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(systray_add_audio_cd_action)},
	{"Add location", GTK_STOCK_NETWORK, N_("Add _location"),
	 "", "Add a no local stream", G_CALLBACK(systray_add_location_action)},
	{"Prev", GTK_STOCK_MEDIA_PREVIOUS, N_("Prev Track"),
	 "", "Prev Track", G_CALLBACK(systray_prev_action)},
	{"Play_Pause", GTK_STOCK_MEDIA_PLAY, N_("Play / Pause"),
	 "", "Play / Pause", G_CALLBACK(systray_play_pause_action)},
	{"Stop", GTK_STOCK_MEDIA_STOP, N_("Stop"),
	 "", "Stop", G_CALLBACK(systray_stop_action)},
	{"Next", GTK_STOCK_MEDIA_NEXT, N_("Next Track"),
	 "", "Next Track", G_CALLBACK(systray_next_action)},
	{"Edit tags", GTK_STOCK_EDIT, N_("Edit track information"),
	 "", "Edit information of current track", G_CALLBACK(edit_tags_playing_action)},
	{"Quit", GTK_STOCK_QUIT, N_("_Quit"),
	 "", "Quit", G_CALLBACK(systray_quit)}
};

static gboolean
status_icon_clicked (GtkWidget *widget, GdkEventButton *event, PraghaStatusIcon *status_icon)
{
	GtkWidget *popup_menu;

	switch (event->button)
	{
		case 1:
			pragha_window_toggle_state (status_icon->pragha, FALSE);
			break;
		case 2:
			pragha_playback_play_pause_resume (status_icon->pragha);
			break;
		case 3:
			popup_menu = gtk_ui_manager_get_widget(status_icon->ui_manager, "/popup");
			gtk_menu_popup (GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
			                event->button, gtk_get_current_event_time ());
		default: break;
	}
	
	return TRUE;
}

static gboolean
status_get_tooltip_cb (GtkWidget        *widget,
                       gint              x,
                       gint              y,
                       gboolean          keyboard_mode,
                       GtkTooltip       *tooltip,
                       PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaMusicobject *mobj;
	gchar *markup_text;

	toolbar = pragha_application_get_toolbar (status_icon->pragha);

	backend = pragha_application_get_backend (status_icon->pragha);
	if (pragha_backend_get_state (backend) == ST_STOPPED)
		markup_text = g_strdup_printf("%s", _("<b>Not playing</b>"));
	else {
		mobj = pragha_backend_get_musicobject (backend);

		markup_text = g_markup_printf_escaped ("<b>%s</b>: %s\n<b>%s</b>: %s\n<b>%s</b>: %s\n<b>%s</b>: %s / %s",
		                                       _("Title"), pragha_musicobject_get_title (mobj),
		                                       _("Artist"), pragha_musicobject_get_artist (mobj),
		                                       _("Album"), pragha_musicobject_get_album (mobj),
		                                       _("Length"), pragha_toolbar_get_progress_text (toolbar),
		                                       pragha_toolbar_get_length_text (toolbar));
	}

	gtk_tooltip_set_markup (tooltip, markup_text);
	gtk_tooltip_set_icon (tooltip,
		pragha_album_art_get_pixbuf(pragha_toolbar_get_album_art(toolbar)));

	g_free(markup_text);

	return TRUE;
}

static void
systray_volume_scroll (GtkWidget *widget, GdkEventScroll *event, PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend;

	if (event->type != GDK_SCROLL)
		return;

	backend = pragha_application_get_backend (status_icon->pragha);

	switch (event->direction){
		case GDK_SCROLL_UP:
			pragha_backend_set_delta_volume (backend, +0.02);
			break;
		case GDK_SCROLL_DOWN:
			pragha_backend_set_delta_volume (backend, -0.02);
			break;
		default:
			break;
	}
}

static void
systray_about_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	about_action (action, status_icon->pragha);
}

static void
systray_open_file_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	open_file_action (action, status_icon->pragha);
}

static void
systray_add_audio_cd_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	add_audio_cd_action (action, status_icon->pragha);
}

static void
systray_add_location_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	add_location_action (action, status_icon->pragha);
}

static void
systray_play_pause_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend = pragha_application_get_backend (status_icon->pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_play_pause_resume(status_icon->pragha);
}

static void
systray_stop_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend = pragha_application_get_backend (status_icon->pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_stop(status_icon->pragha);
}

static void
systray_prev_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend = pragha_application_get_backend (status_icon->pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_prev_track(status_icon->pragha);
}

static void
systray_next_action (GtkAction *action, PraghaStatusIcon *status_icon)
{
	PraghaBackend *backend = pragha_application_get_backend (status_icon->pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_next_track(status_icon->pragha);
}

static void
systray_quit (GtkAction *action, PraghaStatusIcon *status_icon)
{
	pragha_application_quit (status_icon->pragha);
}

static void
pragha_status_icon_update_state (PraghaBackend *backend, GParamSpec *pspec, PraghaStatusIcon *status_icon)
{
	GtkAction *action;
	enum player_state state = 0;

	state = pragha_backend_get_state (backend);

	gboolean playing = (state != ST_STOPPED);

	action = gtk_ui_manager_get_action (status_icon->ui_manager, "/popup/Prev");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action (status_icon->ui_manager, "/popup/Stop");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action (status_icon->ui_manager, "/popup/Next");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);

	action = gtk_ui_manager_get_action (status_icon->ui_manager, "/popup/Edit tags");
	gtk_action_set_sensitive (GTK_ACTION (action), playing);
}

static void
pragha_status_icon_set_application (PraghaStatusIcon *status_icon, PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	GtkActionGroup *actions;
	GdkPixbuf *pixbuf_app;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	status_icon->pragha = pragha;

	pixbuf_app = pragha_application_get_pixbuf_app(pragha);
	if (pixbuf_app)
		gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON(status_icon), pixbuf_app);

	actions = gtk_action_group_new ("Systray Actions");
	gtk_action_group_set_translation_domain (actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (actions,
	                              systray_menu_aentries,
	                              G_N_ELEMENTS(systray_menu_aentries),
	                              (gpointer)status_icon);
	gtk_ui_manager_insert_action_group (status_icon->ui_manager, actions, 0);

	preferences = pragha_application_get_preferences (pragha);
	g_object_bind_property (preferences, "show-status-icon",
	                        status_icon, "visible", binding_flags);

	g_signal_connect (pragha_application_get_backend (pragha), "notify::state",
	                  G_CALLBACK (pragha_status_icon_update_state), status_icon);

	g_object_unref (actions);
}

static void
pragha_status_icon_dispose (GObject *object)
{
	PraghaStatusIcon *status_icon = PRAGHA_STATUS_ICON(object);

	if (status_icon->ui_manager) {
		g_object_unref (status_icon->ui_manager);
		status_icon->ui_manager = NULL;
	}

	(*G_OBJECT_CLASS (pragha_status_icon_parent_class)->dispose) (object);
}

static void
pragha_status_icon_class_init (PraghaStatusIconClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = pragha_status_icon_dispose;
}

static void
pragha_status_icon_init (PraghaStatusIcon *status_icon)
{
	GError *error = NULL;

	status_icon->ui_manager = gtk_ui_manager_new();
	if (!gtk_ui_manager_add_ui_from_string (status_icon->ui_manager, systray_menu_xml, -1, &error))
		g_critical("Unable to create systray menu, err : %s", error->message);

	g_signal_connect (status_icon, "button-press-event",
	                  G_CALLBACK (status_icon_clicked), status_icon);
	g_signal_connect (status_icon, "scroll_event",
	                  G_CALLBACK(systray_volume_scroll), status_icon);

	g_object_set (G_OBJECT(status_icon), "has-tooltip", TRUE, NULL);
	g_signal_connect (G_OBJECT(status_icon), "query-tooltip",
	                  G_CALLBACK(status_get_tooltip_cb), status_icon);
}

PraghaStatusIcon *
pragha_status_icon_new (PraghaApplication *pragha)
{
	PraghaStatusIcon *status_icon;

	status_icon = g_object_new (PRAGHA_TYPE_STATUS_ICON, NULL);

	pragha_status_icon_set_application (status_icon, pragha);

	return status_icon;
}
