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

#include <libnotify/notify.h>

#include "pragha-notify.h"
#include "pragha-playback.h"
#include "pragha-utils.h"
#include "pragha.h"

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

struct PraghaNotify {
	NotifyNotification *osd_notify;
	struct con_win *cwin;
};

static void
notify_closed_cb (NotifyNotification *osd,
                  PraghaNotify *notify)
{
	g_object_unref (G_OBJECT(osd));

	if (notify->osd_notify == osd) {
		notify->osd_notify = NULL;
	}
}

static void
notify_Prev_Callback (NotifyNotification *osd,
                      const char *action,
                      PraghaNotify *notify)
{
	PraghaBackend *backend;

	g_assert (action != NULL);

	struct con_win *cwin = notify->cwin;

	backend = pragha_application_get_backend (cwin);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_prev_track(cwin);
}

static void
notify_Next_Callback (NotifyNotification *osd,
                      const char *action,
                      PraghaNotify *notify)
{
	PraghaBackend *backend;

	g_assert (action != NULL);

	struct con_win *cwin = notify->cwin;

	backend = pragha_application_get_backend (cwin);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_next_track(cwin);
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
pragha_notify_show_osd (PraghaNotify *notify)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaMusicobject *mobj = NULL;
	GError *error = NULL;
	gchar *summary, *body, *slength;

	struct con_win *cwin = notify->cwin;

	/* Check if OSD is enabled in preferences */
	if (!pragha_preferences_get_show_osd(cwin->preferences) || gtk_window_is_active(GTK_WINDOW (pragha_application_get_window(cwin))))
		return;

	backend = pragha_application_get_backend (cwin);

	mobj = pragha_backend_get_musicobject (backend);
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

	if (notify->osd_notify == NULL) {
		#if NOTIFY_CHECK_VERSION (0, 7, 1)
		notify->osd_notify = notify_notification_new(summary, body, NULL);
		#else
		notify->osd_notify = notify_notification_new(summary, body, NULL, NULL);
		#endif

		if(can_support_actions() &&
		   pragha_preferences_get_actions_in_osd (cwin->preferences) == TRUE) {
			notify_notification_add_action(
				notify->osd_notify, "media-skip-backward", _("Prev Track"),
				NOTIFY_ACTION_CALLBACK(notify_Prev_Callback), notify,
				NULL);
			notify_notification_add_action(
				notify->osd_notify, "media-skip-forward", _("Next Track"),
				NOTIFY_ACTION_CALLBACK(notify_Next_Callback), notify,
				NULL);
		}
		notify_notification_set_hint (notify->osd_notify, "transient", g_variant_new_boolean (TRUE));
		g_signal_connect (notify->osd_notify, "closed", G_CALLBACK (notify_closed_cb), notify);
	}
	else {
		notify_notification_update (notify->osd_notify, summary, body, NULL);

		if(pragha_preferences_get_actions_in_osd (cwin->preferences) == FALSE)
			notify_notification_clear_actions (notify->osd_notify);
	}

	notify_notification_set_timeout (notify->osd_notify, OSD_TIMEOUT);

	/* Add album art if set */
	toolbar = pragha_application_get_toolbar (cwin);
	notify_notification_set_icon_from_pixbuf (notify->osd_notify,
		pragha_album_art_get_pixbuf (pragha_toolbar_get_album_art(toolbar)));

	/* Show OSD */
	if (!notify_notification_show (notify->osd_notify, &error)) {
		g_warning("Unable to show OSD notification: %s", error->message);
		g_error_free (error);
	}

	/* Cleanup */

	g_free(summary);
	g_free(body);
	g_free(slength);
}

PraghaNotify *
pragha_notify_new (struct con_win *cwin)
{
	if (!notify_init (PACKAGE_NAME))
		return NULL;

	PraghaNotify *notify = g_slice_new (PraghaNotify);

	notify->osd_notify = NULL;
	notify->cwin = cwin;

	return notify;
}

void
pragha_notify_free (PraghaNotify *notify)
{
	if (notify->osd_notify)
		g_object_unref (notify->osd_notify);

	g_slice_free (PraghaNotify, notify);

	notify_uninit ();
}
