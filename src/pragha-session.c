/*************************************************************************/
/* Copyright (C) 2009-2018 matias <mati86dl@gmail.com>                   */
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

#include "pragha-session.h"

#ifdef HAVE_LIBXFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#endif

#include "pragha.h"

#ifdef HAVE_LIBXFCE4UI
void
pragha_init_session_support(PraghaApplication *pragha)
{
	XfceSMClient *client;
	GError *error = NULL;

	client =  xfce_sm_client_get ();
	xfce_sm_client_set_priority (client, XFCE_SM_CLIENT_PRIORITY_DEFAULT);
	xfce_sm_client_set_restart_style (client, XFCE_SM_CLIENT_RESTART_NORMAL);
	xfce_sm_client_set_desktop_file(client, DESKTOPENTRY);

	g_signal_connect_swapped (G_OBJECT (client), "quit",
	                          G_CALLBACK (pragha_application_quit), pragha);

	if(!xfce_sm_client_connect (client, &error)) {
		g_warning ("Failed to connect to session manager: %s", error->message);
		g_error_free (error);

		// As fallback register DBUS session..
		g_warning ("As fallback try to use dbus session manager");
		g_object_set (GTK_APPLICATION(pragha), "register-session", TRUE, NULL);
	}
}
#else
void
pragha_init_session_support(PraghaApplication *pragha)
{
	GtkWidget *window;
	gchar *role;

	window = pragha_application_get_window (pragha);

	/* set a unique role on each window (for session management) */
	role = g_strdup_printf ("Pragha-%p-%d-%d", window, (gint) getpid (), (gint) time (NULL));
	gtk_window_set_role (GTK_WINDOW (window), role);
	g_free (role);

	g_object_set (GTK_APPLICATION(pragha), "register-session", TRUE, NULL);
}
#endif
