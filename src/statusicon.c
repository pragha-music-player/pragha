/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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
		case 1: if (GTK_WIDGET_VISIBLE(cwin->mainwindow))
				toogle_main_window (cwin, TRUE);
			else
				toogle_main_window (cwin, FALSE);
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

void toogle_main_window(struct con_win *cwin, gboolean present)
{
GtkWindow * window = GTK_WINDOW( cwin->mainwindow );
static int  x = 0, y = 0;

	if (present) {
		gtk_window_get_position( window, &x, &y );
		gtk_widget_hide(GTK_WIDGET(window));
	}
	else{
		gtk_window_set_skip_taskbar_hint( window , FALSE );
		if( x != 0 && y != 0 )
			gtk_window_move( window , x, y );
		gtk_widget_show( GTK_WIDGET( window ) );
		gtk_window_deiconify( window );
		gtk_window_present( window );
	}
}

/* For want of a better place, this is here ... */

void show_osd(struct con_win *cwin)
{
	GError *error = NULL;
	NotifyNotification *osd;
	gchar *summary, *length, *str;

	/* Check if OSD is enabled in preferences */

	if (!cwin->cpref->show_osd || gtk_window_is_active(GTK_WINDOW (cwin->mainwindow)))
		return;

	if( g_utf8_strlen(cwin->cstate->curr_mobj->tags->title, -1))
		str = g_strdup(cwin->cstate->curr_mobj->tags->title);
	else
		str = g_strdup(g_path_get_basename(cwin->cstate->curr_mobj->file));

	length = convert_length_str(cwin->cstate->curr_mobj->tags->length);

	summary = g_markup_printf_escaped("%s: %s\n%s: %s\n%s: %s\n%s: %s",
 			_("Title"), str,
 			_("Artist"), cwin->cstate->curr_mobj->tags->artist,
 			_("Album"), cwin->cstate->curr_mobj->tags->album,
			_("Length"), convert_length_str(cwin->cstate->curr_mobj->tags->length));

	/* Create notification instance */

	osd = notify_notification_new_with_status_icon((const gchar *) summary,
					NULL,
					NULL,
					GTK_STATUS_ICON(cwin->status_icon));
	notify_notification_set_timeout(osd, OSD_TIMEOUT);

	/* Add album art if set */

	if (cwin->cpref->show_album_art &&
	    cwin->album_art &&
	    (gtk_image_get_storage_type(GTK_IMAGE(
				cwin->album_art)) == GTK_IMAGE_PIXBUF))
		notify_notification_set_icon_from_pixbuf(osd,
				 gtk_image_get_pixbuf(GTK_IMAGE(cwin->album_art)));

	/* Show OSD */

	if (!notify_notification_show(osd, &error)) {
		g_warning("Unable to show OSD notification");
	}

	/* Cleanup */

	g_free(summary);
	g_free(length);
	g_free(str);
	g_object_unref(G_OBJECT(osd));
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
			convert_length_str(cwin->cstate->curr_mobj->tags->length));
	}
	gtk_tooltip_set_markup (tooltip, markup_text);
	g_free(markup_text);

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
			cwin->cmixer->inc_volume(cwin);
			break;
		case GDK_SCROLL_DOWN:
			cwin->cmixer->dec_volume(cwin);
			break;
		default:
			return;
	}
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button),
						cwin->cmixer->curr_vol);
	return;
}

void systray_play(GtkAction *action, struct con_win *cwin)
{
	play_pause_resume( cwin);
}

void systray_stop(GtkAction *action, struct con_win *cwin)
{
	stop_playback(cwin);
}

void systray_pause(GtkAction *action, struct con_win *cwin)
{
	pause_resume_track(cwin);
}

void systray_prev(GtkAction *action, struct con_win *cwin)
{
	play_prev_track(cwin);
}

void systray_next(GtkAction *action, struct con_win *cwin)
{
	play_next_track(cwin);
}

void systray_quit(GtkAction *action, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
}
