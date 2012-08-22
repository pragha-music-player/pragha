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
can_support_actions ()
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

void
show_osd (struct con_win *cwin)
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

gint
init_notify (struct con_win *cwin)
{
	if (cwin->cpref->show_osd) {
		if (!notify_init(PACKAGE_NAME))
			return -1;
	}

	return 0;
}

void
notify_free ()
{
	if (notify_is_initted())
		notify_uninit();
}
