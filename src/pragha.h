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
#include <glib/gstdio.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <gtk/gtk.h>
#include <tag_c.h>

#include "pragha-album-art.h"
#include "pragha-backend.h"
#include "pragha-database.h"
#include "pragha-glyr.h"
#include "pragha-musicobject.h"
#include "pragha-notify.h"
#include "pragha-preferences.h"
#include "pragha-preferences-dialog.h"
#include "pragha-playlist.h"
#include "pragha-library-pane.h"
#include "pragha-lastfm.h"
#include "pragha-scanner.h"
#include "pragha-sidebar.h"
#include "pragha-simple-async.h"
#include "pragha-statusbar.h"
#include "pragha-toolbar.h"
#include "gnome-media-keys.h"
#include "pragha-mpris.h"

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

enum dnd_target {
	TARGET_REF_LIBRARY,
	TARGET_URI_LIST,
	TARGET_PLAIN_TEXT
};

struct con_win {
	PraghaPlaylist *cplaylist;
	PraghaLibraryPane *clibrary;
	PraghaBackend *backend;
	PraghaDatabase *cdbase;
#ifdef HAVE_LIBGLYR
	PraghaGlyr *glyr;
#endif
	PraghaMpris2 *cmpris2;
	PraghaNotify *notify;
	PraghaScanner  *scanner;
	PraghaSidebar *sidebar;
	PraghaStatusbar *statusbar;
	PraghaToolbar *toolbar;
	PraghaPreferences *preferences;
	PreferencesWidgets *preferences_w;
	#ifdef HAVE_LIBCLASTFM
	struct con_lastfm *clastfm;
	#endif
	con_gnome_media_keys *cgnome_media_keys;
	GtkWidget *mainwindow;
	GdkPixbuf *pixbuf_app;
	GtkWidget *info_box;
	GtkWidget *paned;
	GtkStatusIcon *status_icon;
	GtkUIManager *bar_context_menu;
	GtkUIManager *systray_menu;
	gboolean unique_instance;
	gboolean first_run;
	guint related_timeout_id;
	DBusConnection *con_dbus;
};

/* Functions to access private members */

PraghaBackend *pragha_application_get_backend (struct con_win *cwin);
GtkWidget     *pragha_application_get_window  (struct con_win *cwin);

/* Info bar import music */

gboolean info_bar_import_music_will_be_useful(struct con_win *cwin);
GtkWidget* create_info_bar_import_music(struct con_win *cwin);
GtkWidget* create_info_bar_update_music(struct con_win *cwin);

/* Init */

gint init_options(struct con_win *cwin, int argc, char **argv);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* Close */

void pragha_application_quit (struct con_win *cwin);

#endif /* PRAGHA_H */
