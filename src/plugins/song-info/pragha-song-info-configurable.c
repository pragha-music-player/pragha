/*
 * peasdemo-hello-world-configurable.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009-2010 Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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

#include "pragha-song-info-configurable.h"
#include "pragha-song-info-plugin.h"
#include "../../pragha-hig.h"

static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PraghaSongInfoConfigurable,
                                pragha_song_info_configurable,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init))

static void
toggle_download_album_art (GtkToggleButton *button, PraghaSongInfoPlugin *plugin)
{
	plugin->download_album_art = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
}

static void
pragha_song_info_configurable_init (PraghaSongInfoConfigurable *plugin)
{
	g_debug ("%s", G_STRFUNC);
}

static GtkWidget *
pragha_song_info_configurable_create_configure_widget (PeasGtkConfigurable *configurable)
{
	GtkWidget *table;
	GtkWidget *download_album_art;
	guint row = 0;

	PraghaSongInfoPlugin *plugin = PRAGHA_SONG_INFO_PLUGIN (configurable);

	table = pragha_hig_workarea_table_new ();

	download_album_art = gtk_check_button_new_with_label (_("Download the album art while playing their songs."));
	pragha_hig_workarea_table_add_wide_control (table, &row, download_album_art);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(download_album_art), plugin->download_album_art);

	g_signal_connect (G_OBJECT(download_album_art), "toggled",
	                  G_CALLBACK(toggle_download_album_art), plugin);

	return table;
}

static void
pragha_song_info_configurable_class_init (PraghaSongInfoConfigurableClass *klass)
{
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = pragha_song_info_configurable_create_configure_widget;
}

static void
pragha_song_info_configurable_class_finalize (PraghaSongInfoConfigurableClass *klass)
{
}

void
pragha_song_info_configurable_register (GTypeModule *module)
{
	pragha_song_info_configurable_register_type (module);
}
