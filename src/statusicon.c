/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

static void systray_play_pause_action(GtkAction *action, struct con_win *cwin);
static void systray_stop_action(GtkAction *action, struct con_win *cwin);
static void systray_prev_action(GtkAction *action, struct con_win *cwin);
static void systray_next_action(GtkAction *action, struct con_win *cwin);
static void systray_quit(GtkAction *action, struct con_win *cwin);

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
	 "", NULL, G_CALLBACK(about_action)},
	{"Add files", GTK_STOCK_OPEN, N_("_Add files"),
	 "", NULL, G_CALLBACK(open_file_action)},
	{"Add Audio CD", GTK_STOCK_CDROM, N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(add_audio_cd_action)},
	{"Add location", GTK_STOCK_NETWORK, N_("Add _location"),
	 "", "Add a no local stream", G_CALLBACK(add_location_action)},
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
status_icon_clicked (GtkWidget *widget, GdkEventButton *event, struct con_win *cwin)
{
	GtkWidget *popup_menu;
	switch (event->button)
	{
		case 1: toogle_main_window (cwin, FALSE);
			break;
		case 2:	play_pause_resume(cwin);
			break;
		case 3:
			popup_menu = gtk_ui_manager_get_widget(cwin->systray_menu, "/popup");
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				       event->button, gtk_get_current_event_time ());
		default: break;
	}
	
	return TRUE;
}

static void
notify_closed_cb (NotifyNotification *osd,
			struct con_win *cwin)
{
	g_object_unref (G_OBJECT(osd));

	if (cwin->osd_notify == osd) {
		cwin->osd_notify = NULL;
	}
}

static void
notify_Prev_Callback (NotifyNotification *osd,
                const char *action,
                struct con_win *cwin)
{
	g_assert (action != NULL);
	g_assert (strcmp (action, "media-prev") == 0);

	if(cwin->cgst->emitted_error == FALSE)
		play_prev_track(cwin);
}

static void
notify_Next_Callback (NotifyNotification *osd,
                const char *action,
                struct con_win *cwin)
{
	g_assert (action != NULL);
	g_assert (strcmp (action, "media-next") == 0);

	if(cwin->cgst->emitted_error == FALSE)
		play_next_track(cwin);
}


gboolean
can_support_actions( void )
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

void show_osd(struct con_win *cwin)
{
	GError *error = NULL;
	gchar *summary, *body, *length;

	/* Check if OSD is enabled in preferences */
	if (!cwin->cpref->show_osd || gtk_window_is_active(GTK_WINDOW (cwin->mainwindow)))
		return;

	if(g_utf8_strlen(cwin->cstate->curr_mobj->tags->title, 4))
		summary = g_strdup(cwin->cstate->curr_mobj->tags->title);
	else
		summary = g_path_get_basename(cwin->cstate->curr_mobj->file);

	length = convert_length_str(cwin->cstate->curr_mobj->tags->length);

	body = g_markup_printf_escaped(_("by <b>%s</b> in <b>%s</b> <b>(%s)</b>"),
			(cwin->cstate->curr_mobj->tags->artist && strlen(cwin->cstate->curr_mobj->tags->artist)) ?
			cwin->cstate->curr_mobj->tags->artist : _("Unknown Artist"),
			(cwin->cstate->curr_mobj->tags->album && strlen(cwin->cstate->curr_mobj->tags->album)) ?
			cwin->cstate->curr_mobj->tags->album : _("Unknown Album"),
			length);

	/* Create notification instance */
	#if NOTIFY_CHECK_VERSION (0, 7, 0)
	if (cwin->osd_notify == NULL) {
		cwin->osd_notify = notify_notification_new(summary, body, NULL);

		if(can_support_actions() &&
		   cwin->cpref->actions_in_osd == TRUE) {
			notify_notification_add_action(
				cwin->osd_notify, "media-prev", _("Prev Track"),
				NOTIFY_ACTION_CALLBACK(notify_Prev_Callback), cwin,
				NULL);
			notify_notification_add_action(
				cwin->osd_notify, "media-next", _("Next Track" ),
				NOTIFY_ACTION_CALLBACK(notify_Next_Callback), cwin,
				NULL);
		}
		notify_notification_set_hint (cwin->osd_notify, "transient", g_variant_new_boolean (TRUE));
		g_signal_connect(cwin->osd_notify, "closed", G_CALLBACK (notify_closed_cb), cwin);
	}
	else {
		notify_notification_update (cwin->osd_notify, summary, body, NULL);

		if(cwin->cpref->actions_in_osd == FALSE)
			notify_notification_clear_actions (cwin->osd_notify);
	}
	#else
	if(cwin->cpref->osd_in_systray && gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))) {
		cwin->osd_notify = notify_notification_new_with_status_icon(summary,
								body, NULL,
								GTK_STATUS_ICON(cwin->status_icon));
	}
	else {
		cwin->osd_notify = notify_notification_new(summary, body, NULL, NULL);
	}
	#endif

	notify_notification_set_timeout(cwin->osd_notify, OSD_TIMEOUT);

	/* Add album art if set */
	if (cwin->cpref->show_album_art && cwin->album_art && cwin->cpref->albumart_in_osd &&
	    (gtk_image_get_storage_type(GTK_IMAGE(cwin->album_art)) == GTK_IMAGE_PIXBUF))
			notify_notification_set_icon_from_pixbuf(cwin->osd_notify, gtk_image_get_pixbuf(GTK_IMAGE(cwin->album_art)));

	/* Show OSD */
	if (!notify_notification_show(cwin->osd_notify, &error)) {
		g_warning("Unable to show OSD notification: %s", error->message);
		g_error_free (error);
	}

	/* Cleanup */

	g_free(summary);
	g_free(body);
	g_free(length);
}

static gboolean
status_get_tooltip_cb (GtkWidget        *widget,
                       gint              x,
                       gint              y,
                       gboolean          keyboard_mode,
                       GtkTooltip       *tooltip,
                       struct con_win   *cwin)
{
	gchar *markup_text;

	if (cwin->cstate->state == ST_STOPPED)
		markup_text = g_strdup_printf("%s", _("<b>Not playing</b>"));
	else {
		markup_text = g_markup_printf_escaped("<b>%s</b>: %s\n<b>%s</b>: %s\n<b>%s</b>: %s\n<b>%s</b>: %s / %s",
			_("Title"), cwin->cstate->curr_mobj->tags->title,
			_("Artist"), cwin->cstate->curr_mobj->tags->artist,
			_("Album"), cwin->cstate->curr_mobj->tags->album,
			_("Length"), gtk_label_get_text (GTK_LABEL(cwin->track_time_label)),
			gtk_label_get_text (GTK_LABEL(cwin->track_length_label)));
	}
	gtk_tooltip_set_markup (tooltip, markup_text);
	g_free(markup_text);

	if (cwin->cpref->show_album_art && cwin->album_art &&
	    (gtk_image_get_storage_type(GTK_IMAGE(cwin->album_art)) == GTK_IMAGE_PIXBUF))
		gtk_tooltip_set_icon (tooltip, gtk_image_get_pixbuf(GTK_IMAGE(cwin->album_art)));

	return TRUE;
}

static void
systray_volume_scroll (GtkWidget *widget, GdkEventScroll *event, struct con_win *cwin)
{
	if (event->type != GDK_SCROLL)
		return;

	switch (event->direction){
		case GDK_SCROLL_UP:
			cwin->cgst->curr_vol += 0.02;
			break;
		case GDK_SCROLL_DOWN:
			cwin->cgst->curr_vol -= 0.02;
			break;
		default:
			return;
	}

	cwin->cgst->curr_vol = CLAMP (cwin->cgst->curr_vol, 0.0, 1.0);

	backend_update_volume(cwin);

	return;
}

static void
systray_play_pause_action (GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_pause_resume(cwin);
}

static void
systray_stop_action (GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		backend_stop(NULL, cwin);
}

static void
systray_prev_action (GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_prev_track(cwin);
}

static void
systray_next_action (GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_next_track(cwin);
}

static void
systray_quit (GtkAction *action, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
}

static GtkUIManager*
create_systray_menu (struct con_win *cwin)
{
	GtkUIManager *menu = NULL;
	GtkActionGroup *actions;
	GError *error = NULL;

	actions = gtk_action_group_new("Systray Actions");
	menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(menu, systray_menu_xml, -1, &error)) {
		g_critical("Unable to create systray menu, err : %s",
			   error->message);
	}

	gtk_action_group_add_actions(actions,
				     systray_menu_aentries,
				     G_N_ELEMENTS(systray_menu_aentries),
				     (gpointer)cwin);
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow),
				   gtk_ui_manager_get_accel_group(menu));
	gtk_ui_manager_insert_action_group(menu, actions, 0);

	return menu;
}

void create_status_icon (struct con_win *cwin)
{
	GtkStatusIcon *status_icon;
	GtkUIManager *systray_menu;

	if (cwin->pixbuf->pixbuf_app)
		status_icon = gtk_status_icon_new_from_pixbuf(cwin->pixbuf->pixbuf_app);
	else
		status_icon = gtk_status_icon_new_from_stock(GTK_STOCK_NEW);

	g_signal_connect (status_icon, "button-press-event", G_CALLBACK (status_icon_clicked), cwin);
	g_signal_connect (status_icon, "scroll_event", G_CALLBACK(systray_volume_scroll), cwin);

	g_object_set (G_OBJECT(status_icon), "has-tooltip", TRUE, NULL);
	g_signal_connect(G_OBJECT(status_icon), "query-tooltip",
			G_CALLBACK(status_get_tooltip_cb),
			cwin);

	gtk_status_icon_set_visible(status_icon, cwin->cpref->show_icon_tray);

	/* Systray right click menu */

	systray_menu = create_systray_menu(cwin);

	/* Store reference */

	cwin->status_icon = status_icon;
	cwin->systray_menu = systray_menu;
}
