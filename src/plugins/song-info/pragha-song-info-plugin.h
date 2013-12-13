/*************************************************************************/
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

#ifndef __PRAGHA_SONG_INFO_PLUGIN_H__
#define __PRAGHA_SONG_INFO_PLUGIN_H__

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif


#include <gtk/gtk.h>
#include <libpeas/peas.h>

#include <glyr/cache.h>

#include "../../pragha.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_SONG_INFO_PLUGIN         (pragha_song_info_plugin_get_type ())
#define PRAGHA_SONG_INFO_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_SONG_INFO_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_IS_SONG_INFO_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_IS_SONG_INFO_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_SONG_INFO_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPluginClass))

typedef struct _PraghaSongInfoPlugin       PraghaSongInfoPlugin;
typedef struct _PraghaSongInfoPluginClass  PraghaSongInfoPluginClass;

struct _PraghaSongInfoPlugin {
	PeasExtensionBase parent_instance;

	PraghaApplication *pragha;

	GlyrDatabase      *cache_db;

	gboolean           download_album_art;

	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;

	GtkActionGroup    *action_group_playlist;
	guint              merge_id_playlist;
};

struct _PraghaSongInfoPluginClass {
	PeasExtensionBaseClass parent_class;
};

GType                 pragha_song_info_plugin_get_type        (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                     (PeasObjectModule *module);

G_END_DECLS

#endif /* __PRAGHA_SONG_INFO_PLUGIN_H__ */
