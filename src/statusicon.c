/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
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

#include "consonance.h"

/* For want of a better place, this is here ... */

void show_osd(struct con_win *cwin)
{
	GError *error = NULL;
	NotifyNotification *osd;
	gchar *body, *length;
	gchar *eartist = NULL, *ealbum = NULL;

	/* Check if OSD is enabled in preferences */

	if (!cwin->cpref->show_osd)
		return;

	if (cwin->cstate->curr_mobj->tags->artist)
		eartist = g_markup_escape_text(cwin->cstate->curr_mobj->tags->artist,
				       strlen(cwin->cstate->curr_mobj->tags->artist));

	if (cwin->cstate->curr_mobj->tags->album)
		ealbum = g_markup_escape_text(cwin->cstate->curr_mobj->tags->album,
				      strlen(cwin->cstate->curr_mobj->tags->album));

	length = convert_length_str(cwin->cstate->curr_mobj->tags->length);
	body = g_strdup_printf("Artist: %s\nAlbum: %s\nLength: %s",
			       eartist,
			       ealbum,
			       length);

	/* Create notification instance */

	osd = notify_notification_new_with_status_icon(
		cwin->cstate->curr_mobj->tags->title,
		(const gchar *)body,
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

	g_free(length);
	g_free(body);
	g_free(eartist);
	g_free(ealbum);
	g_object_unref(G_OBJECT(osd));
}

void status_icon_tooltip_update(struct con_win *cwin)
{
	gchar *tooltip;

	tooltip = g_strdup_printf("%s by %s", cwin->cstate->curr_mobj->tags->title,
				  cwin->cstate->curr_mobj->tags->artist);
	gtk_status_icon_set_tooltip(GTK_STATUS_ICON(cwin->status_icon), tooltip);
	g_free(tooltip);
}

void unset_status_icon_tooltip(struct con_win *cwin)
{
	gtk_status_icon_set_tooltip(GTK_STATUS_ICON(cwin->status_icon),
				    PACKAGE_STRING);
}

void status_icon_activate(GtkStatusIcon *status_icon, struct con_win *cwin)
{
	if (GTK_WIDGET_VISIBLE(cwin->mainwindow)) {
		gtk_widget_hide(cwin->mainwindow);
	}
	else
		gtk_widget_show(cwin->mainwindow);
}

void status_icon_popup_menu(GtkStatusIcon *status_icon,
			    guint button,
			    guint activate_time,
			    struct con_win *cwin)
{
	GtkWidget *popup_menu;

	switch(button) {
	case 3:
		popup_menu = gtk_ui_manager_get_widget(cwin->systray_menu, "/popup");
		gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
			       button, activate_time);

		break;
	default:
		break;
	}
}	

void systray_play(GtkAction *action, struct con_win *cwin)
{
	play_track(cwin);
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
	exit_consonance(NULL, cwin);
}
