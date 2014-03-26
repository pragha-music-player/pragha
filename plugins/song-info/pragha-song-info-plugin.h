/*************************************************************************/
/* Copyright (C) 2011-2014 matias <mati86dl@gmail.com>                   */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef PRAGHA_SONGINFO_PLUGIN_H
#define PRAGHA_SONGINFO_PLUGIN_H

#include <gtk/gtk.h>
#include <glib-object.h>

#include <glyr/cache.h>
#include "pragha-song-info-pane.h"

#include "src/pragha.h"

#include "plugins/pragha-plugin-macros.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_SONG_INFO_PLUGIN         (pragha_song_info_plugin_get_type ())
#define PRAGHA_SONG_INFO_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_SONG_INFO_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPlugin))
#define PRAGHA_IS_SONG_INFO_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_IS_SONG_INFO_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_SONG_INFO_PLUGIN))
#define PRAGHA_SONG_INFO_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_SONG_INFO_PLUGIN, PraghaSongInfoPluginClass))

typedef struct _PraghaSongInfoPluginPrivate PraghaSongInfoPluginPrivate;

PRAGHA_PLUGIN_REGISTER_PUBLIC_HEADER (PRAGHA_TYPE_SONG_INFO_PLUGIN,
                                      PraghaSongInfoPlugin,
                                      pragha_song_info_plugin)

PraghaApplication  *pragha_songinfo_plugin_get_application (PraghaSongInfoPlugin *plugin);
GlyrDatabase       *pragha_songinfo_plugin_get_cache       (PraghaSongInfoPlugin *plugin);
PraghaSonginfoPane *pragha_songinfo_plugin_get_pane        (PraghaSongInfoPlugin *plugin);

G_END_DECLS

#endif /* PRAGHA_SONGINFO_PLUGIN_H */
