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

gboolean
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

void toogle_main_window (struct con_win *cwin, gboolean ignoreActivity)
{
	gint x = 0, y = 0;

	if (gtk_widget_get_visible (cwin->mainwindow)) {
		if (ignoreActivity || gtk_window_is_active (GTK_WINDOW(cwin->mainwindow))){
			gtk_window_get_position (GTK_WINDOW(cwin->mainwindow), &x, &y);
			gtk_widget_hide (GTK_WIDGET(cwin->mainwindow));
			gtk_window_move (GTK_WINDOW(cwin->mainwindow), x ,y);
		}
		else gtk_window_present (GTK_WINDOW(cwin->mainwindow));
	}
	else {
		gtk_widget_show (GTK_WIDGET(cwin->mainwindow));
	}
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

		g_list_foreach( caps, (GFunc)g_free, NULL );
		g_list_free( caps );
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

	if(cwin->cstate->curr_mobj->tags->title != NULL)
		summary = g_strdup(cwin->cstate->curr_mobj->tags->title);
	else
		summary = g_path_get_basename(cwin->cstate->curr_mobj->file);

	length = convert_length_str(cwin->cstate->curr_mobj->tags->length);

	body = g_markup_printf_escaped(_("by <b>%s</b> in <b>%s</b> <b>(%s)</b>"),
			(cwin->cstate->curr_mobj->tags->artist) ?
			cwin->cstate->curr_mobj->tags->artist : _("Unknown Artist"),
			(cwin->cstate->curr_mobj->tags->album) ?
			cwin->cstate->curr_mobj->tags->album : _("Unknown Album"),
			length);

	/* Create notification instance */
	#if NOTIFY_CHECK_VERSION (0, 7, 0)
	if (cwin->osd_notify == NULL) {
		cwin->osd_notify = notify_notification_new((const gchar *) summary, body, NULL);

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
		notify_notification_update (cwin->osd_notify, (const gchar *) summary, body, NULL);

		if(cwin->cpref->actions_in_osd == FALSE)
			notify_notification_clear_actions (cwin->osd_notify);
	}
	#else
	if(cwin->cpref->osd_in_systray && gtk_status_icon_is_embedded(GTK_STATUS_ICON(cwin->status_icon))) {
		cwin->osd_notify = notify_notification_new_with_status_icon((const gchar *) summary,
								body, NULL,
								GTK_STATUS_ICON(cwin->status_icon));
	}
	else {
		cwin->osd_notify = notify_notification_new((const gchar *) summary, body, NULL, NULL);
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

gboolean status_get_tooltip_cb (GtkWidget        *widget,
					gint              x,
					gint              y,
					gboolean          keyboard_mode,
					GtkTooltip       *tooltip,
					struct con_win *cwin) 
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

void
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

void systray_play_pause_action (GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_pause_resume(cwin);
}

void systray_stop_action(GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		backend_stop(NULL, cwin);
}

void systray_prev_action(GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_prev_track(cwin);
}

void systray_next_action(GtkAction *action, struct con_win *cwin)
{
	if(cwin->cgst->emitted_error == FALSE)
		play_next_track(cwin);
}

void systray_quit(GtkAction *action, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
}
