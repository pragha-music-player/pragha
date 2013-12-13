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

#ifndef PRAGHA_H
#define PRAGHA_H

#ifndef HAVE_PARANOIA_NEW_INCLUDES
#include "pragha-cdda.h" // Should be before config.h, libcdio 0.83 issue
#endif

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>

#include "pragha-album-art.h"
#include "pragha-art-cache.h"
#include "pragha-backend.h"
#include "pragha-database.h"
#include "pragha-notify.h"
#include "pragha-preferences.h"
#include "pragha-playlist.h"
#include "pragha-library-pane.h"
#include "pragha-lastfm.h"
#include "pragha-toolbar.h"
#include "pragha-scanner.h"
#include "pragha-sidebar.h"
#include "pragha-statusbar.h"
#include "pragha-statusicon.h"
#include "pragha-window.h"
#include "pragha-debug.h"

/* With libcio 0.83 should be before config.h. libcdio issue */
#ifdef HAVE_PARANOIA_NEW_INCLUDES
#include "pragha-cdda.h"
#endif

/* Some default preferences. */

#define MIN_WINDOW_WIDTH           (gdk_screen_width() * 3 / 4)
#define MIN_WINDOW_HEIGHT          (gdk_screen_height() * 3 / 4)
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH ((MIN_WINDOW_WIDTH - DEFAULT_SIDEBAR_SIZE) / 4)

#define WAIT_UPDATE 5

typedef struct _PraghaApplication PraghaApplication;

/* Functions to access private members */

PraghaPreferences *pragha_application_get_preferences     (PraghaApplication *pragha);
PraghaDatabase    *pragha_application_get_database        (PraghaApplication *pragha);
PraghaArtCache    *pragha_application_get_art_cache       (PraghaApplication *pragha);

PraghaBackend     *pragha_application_get_backend         (PraghaApplication *pragha);

PraghaScanner     *pragha_application_get_scanner         (PraghaApplication *pragha);

GtkWidget         *pragha_application_get_window          (PraghaApplication *pragha);
GdkPixbuf         *pragha_application_get_pixbuf_app      (PraghaApplication *pragha);
PraghaPlaylist    *pragha_application_get_playlist        (PraghaApplication *pragha);
PraghaLibraryPane *pragha_application_get_library         (PraghaApplication *pragha);
PraghaToolbar     *pragha_application_get_toolbar         (PraghaApplication *pragha);
PraghaSidebar     *pragha_application_get_sidebar         (PraghaApplication *pragha);
PraghaStatusbar   *pragha_application_get_statusbar       (PraghaApplication *pragha);
PraghaStatusIcon  *pragha_application_get_status_icon     (PraghaApplication *pragha);

GtkUIManager      *pragha_application_get_menu_ui_manager    (PraghaApplication *pragha);
GtkAction         *pragha_application_get_menu_action        (PraghaApplication *pragha, const gchar *path);
GtkWidget         *pragha_application_get_menu_action_widget (PraghaApplication *pragha, const gchar *path);
GtkWidget         *pragha_application_get_menubar            (PraghaApplication *pragha);
GtkWidget         *pragha_application_get_infobox_container  (PraghaApplication *pragha);
GtkWidget         *pragha_application_get_pane               (PraghaApplication *pragha);

PeasEngine        *pragha_application_get_peas_engine        (PraghaApplication *pragha);

#ifdef HAVE_LIBCLASTFM
PraghaLastfm      *pragha_application_get_lastfm             (PraghaApplication *pragha);
#endif

PraghaNotify      *pragha_application_get_notify             (PraghaApplication *pragha);
void               pragha_application_set_notify             (PraghaApplication *pragha, PraghaNotify *notify);

gboolean           pragha_application_is_first_run           (PraghaApplication *pragha);

gint handle_command_line (PraghaApplication *pragha, GApplicationCommandLine *command_line, gint argc, gchar **args);

/* Info bar import music */

gboolean info_bar_import_music_will_be_useful(PraghaApplication *pragha);
GtkWidget* create_info_bar_import_music(PraghaApplication *pragha);
GtkWidget* create_info_bar_update_music(PraghaApplication *pragha);

/* Pragha app */

#define PRAGHA_TYPE_APPLICATION            (pragha_application_get_type ())
#define PRAGHA_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PRAGHA_TYPE_APPLICATION, PraghaApplication))
#define PRAGHA_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PRAGHA_TYPE_APPLICATION, PraghaApplicationClass))
#define PRAGHA_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PRAGHA_TYPE_APPLICATION))
#define PRAGHA_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PRAGHA_TYPE_APPLICATION))
#define PRAGHA_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PRAGHA_TYPE_APPLICATION, PraghaApplicationClass))

typedef struct {
	GApplicationClass parent_class;
} PraghaApplicationClass;

void pragha_application_quit (PraghaApplication *pragha);

#endif /* PRAGHA_H */
