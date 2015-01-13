/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2015 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_APPLICATION_H
#define PRAGHA_APPLICATION_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include "pragha-album-art.h"
#include "pragha-art-cache.h"
#include "pragha-backend.h"
#include "pragha-database.h"
#include "pragha-preferences.h"
#include "pragha-preferences-dialog.h"
#include "pragha-playlist.h"
#include "pragha-library-pane.h"
#include "pragha-toolbar.h"
#include "pragha-scanner.h"
#include "pragha-sidebar.h"
#include "pragha-statusbar.h"
#include "pragha-statusicon.h"
#include "pragha-debug.h"

#ifdef HAVE_LIBPEAS
#include "pragha-plugins-engine.h"
#endif

G_BEGIN_DECLS

/* Some default preferences. */

#define MIN_WINDOW_WIDTH           (gdk_screen_width() * 3 / 4)
#define MIN_WINDOW_HEIGHT          (gdk_screen_height() * 3 / 4)
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH ((MIN_WINDOW_WIDTH - DEFAULT_SIDEBAR_SIZE) / 4)

typedef struct _PraghaApplication PraghaApplication;

void               pragha_application_open_files          (PraghaApplication *pragha);
void               pragha_application_add_location        (PraghaApplication *pragha);
void               pragha_application_append_entery_libary(PraghaApplication *pragha);
void               pragha_application_about_dialog        (PraghaApplication *pragha);

/* Functions to access private members */

PraghaPreferences *pragha_application_get_preferences     (PraghaApplication *pragha);
PraghaDatabase    *pragha_application_get_database        (PraghaApplication *pragha);
PraghaArtCache    *pragha_application_get_art_cache       (PraghaApplication *pragha);

PraghaBackend     *pragha_application_get_backend         (PraghaApplication *pragha);

#ifdef HAVE_LIBPEAS
PraghaPluginsEngine *pragha_application_get_plugins_engine (PraghaApplication *pragha);
#endif

PraghaScanner     *pragha_application_get_scanner         (PraghaApplication *pragha);

GtkWidget         *pragha_application_get_window          (PraghaApplication *pragha);
GdkPixbuf         *pragha_application_get_pixbuf_app      (PraghaApplication *pragha);
PraghaPlaylist    *pragha_application_get_playlist        (PraghaApplication *pragha);
PraghaLibraryPane *pragha_application_get_library         (PraghaApplication *pragha);
PreferencesDialog *pragha_application_get_preferences_dialog (PraghaApplication *pragha);

PraghaToolbar     *pragha_application_get_toolbar         (PraghaApplication *pragha);
PraghaSidebar     *pragha_application_get_first_sidebar   (PraghaApplication *pragha);
PraghaSidebar     *pragha_application_get_second_sidebar  (PraghaApplication *pragha);
PraghaStatusbar   *pragha_application_get_statusbar       (PraghaApplication *pragha);
PraghaStatusIcon  *pragha_application_get_status_icon     (PraghaApplication *pragha);

GtkBuilder        *pragha_application_get_menu_ui            (PraghaApplication *pragha);
GtkUIManager      *pragha_application_get_menu_ui_manager    (PraghaApplication *pragha);
GtkAction         *pragha_application_get_menu_action        (PraghaApplication *pragha, const gchar *path);
GtkWidget         *pragha_application_get_menu_action_widget (PraghaApplication *pragha, const gchar *path);
GtkWidget         *pragha_application_get_menubar            (PraghaApplication *pragha);
GtkWidget         *pragha_application_get_infobox_container  (PraghaApplication *pragha);
GtkWidget         *pragha_application_get_first_pane         (PraghaApplication *pragha);
GtkWidget         *pragha_application_get_second_pane        (PraghaApplication *pragha);

gboolean           pragha_application_is_first_run           (PraghaApplication *pragha);

gint handle_command_line (PraghaApplication *pragha, GApplicationCommandLine *command_line, gint argc, gchar **args);

/* Info bar import music */

gboolean info_bar_import_music_will_be_useful(PraghaApplication *pragha);
GtkWidget* create_info_bar_import_music(PraghaApplication *pragha);
GtkWidget* create_info_bar_update_music(PraghaApplication *pragha);
GtkWidget *pragha_info_bar_need_restart (PraghaApplication *pragha);

/* Pragha app */

#define PRAGHA_TYPE_APPLICATION            (pragha_application_get_type ())
#define PRAGHA_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PRAGHA_TYPE_APPLICATION, PraghaApplication))
#define PRAGHA_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PRAGHA_TYPE_APPLICATION, PraghaApplicationClass))
#define PRAGHA_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PRAGHA_TYPE_APPLICATION))
#define PRAGHA_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PRAGHA_TYPE_APPLICATION))
#define PRAGHA_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PRAGHA_TYPE_APPLICATION, PraghaApplicationClass))

typedef struct {
	GtkApplicationClass parent_class;
} PraghaApplicationClass;

void pragha_application_quit (PraghaApplication *pragha);

G_END_DECLS

#endif /* PRAGHA_APPLICATION_H */
