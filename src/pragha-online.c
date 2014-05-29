/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#include <libsoup/soup.h>

#include "pragha-simple-async.h"
#include "pragha-utils.h"
#include "xml_helper.h"

#include "pragha-online.h"

typedef struct {
	PraghaApplication *pragha;
	XMLNode           *xml;
} PraghaCheckAsyncData;

static PraghaCheckAsyncData *
pragha_check_async_data_new (PraghaApplication *pragha, XMLNode *xml)
{
	PraghaCheckAsyncData *data;

	data = g_slice_new (PraghaCheckAsyncData);
	data->pragha = pragha;
	data->xml = xml;

	return data;
}

static void
pragha_check_async_data_free (PraghaCheckAsyncData *data)
{
	g_slice_free (PraghaCheckAsyncData, data);
}


static void
pragha_new_version_show_realise (GtkButton *button, PraghaApplication *pragha)
{
	XMLNode *xml = NULL, *xi;

	xml = g_object_get_data (G_OBJECT(button), "RESPONSE-XML");

	xi = xmlnode_get (xml, CCA{"rsp", "pragha", "url", NULL }, NULL, NULL);

	open_url (xi->content, pragha_application_get_window(pragha));

	gtk_widget_destroy (GTK_WIDGET(button));
}

static void
pragha_check_version_add_buuton (PraghaCheckAsyncData *data)
{
	PraghaToolbar *toolbar;
	GtkWidget *button;
	XMLNode *xi = NULL;
	gchar *tooltip = NULL;

	/* Add a new button to toolbar indicatin new version */

	button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
	gtk_button_set_image (GTK_BUTTON(button),
	                      gtk_image_new_from_icon_name("emblem-favorite", GTK_ICON_SIZE_MENU));

	xi = xmlnode_get (data->xml,
	                  CCA{"rsp", "pragha", "friendly-name", NULL },
	                  NULL, NULL);

	tooltip = g_strdup_printf(_("A new version is available: %s"), xi->content);
	gtk_widget_set_tooltip_text (GTK_WIDGET(button), tooltip);
	g_free (tooltip);

	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(pragha_new_version_show_realise), data->pragha);

	g_object_set_data_full (G_OBJECT(button), "RESPONSE-XML", data->xml, (GDestroyNotify)xmlnode_free);

	toolbar = pragha_application_get_toolbar (data->pragha);
	pragha_toolbar_add_extention_widget (toolbar, button);
	gtk_widget_show (button);
}

static gboolean
pragha_check_version_done (gpointer user_data)
{
	XMLNode *xi = NULL;

	PraghaCheckAsyncData *data = user_data;

	if (data == NULL)
		return FALSE;

	/* Compare current version */

	xi = xmlnode_get (data->xml,
	                  CCA{"rsp", "pragha", "curent-version", NULL },
	                  NULL, NULL);

	if (g_ascii_strcasecmp (xi->content, PACKAGE_VERSION) > 0) {
		pragha_check_version_add_buuton (data);
	}
	else {
		xmlnode_free (data->xml);
	}

	pragha_check_async_data_free (data);

	return FALSE;
}

static gpointer
pragha_check_version_async (gpointer data)
{
	SoupSession *session;
	SoupMessage *msg;
	XMLNode *xml = NULL;

	PraghaApplication *pragha = data;

	session = soup_session_sync_new ();
	msg = soup_message_new ("GET", "http://localhost/PRAGHA/api/pragha.version");
	soup_session_send_message (session, msg);

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		return NULL;

	xml = tinycxml_parse ((gchar *)msg->response_body->data);

	return pragha_check_async_data_new (pragha, xml);
}

void
pragha_check_online_version (PraghaApplication *pragha)
{
	pragha_async_launch (pragha_check_version_async,
	                     pragha_check_version_done,
	                     pragha);
}
