/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
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
#include <libnotify/notify.h>
#include <gtk/gtk.h>

#include "pragha-album-art.h"
#include "pragha-backend.h"
#include "pragha-database.h"
#include "pragha-glyr.h"
#include "pragha-musicobject.h"
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
#define PROGRESS_BAR_WIDTH         300
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH ((MIN_WINDOW_WIDTH - DEFAULT_SIDEBAR_SIZE) / 4)
#define OSD_TIMEOUT                5000
#define ALBUM_ART_PATTERN_LEN      1024
#define LASTFM_UNAME_LEN           256
#define LASTFM_PASS_LEN            512
#define TAG_MAX_LEN                256
#define AUDIO_CD_DEVICE_ENTRY_LEN  32

/* These are not seen in the playlist columns */
/* Used for library view and preferences */

#define P_FOLDER_STR        "Folder"	/* Containing folder */
#define P_BASENAME_STR      "Basename"	/* Base name of the file */

#define NORMAL_STATE		"normal"
#define FULLSCREEN_STATE	"fullscreen"
#define ICONIFIED_STATE		"iconified"

#define PANE_LIBRARY		"library"
#define PANE_PLAYLISTS		"playlists"
#define PANE_NONE		"none"

#define MIN_DATABASE_VERSION	"0.8.0"

#define WAIT_UPDATE 5

enum dnd_target {
	TARGET_REF_LIBRARY,
	TARGET_URI_LIST,
	TARGET_PLAIN_TEXT
};

struct con_pref {
	gchar *installed_version;
	gchar *start_mode;
	gint window_width;
	gint window_height;
	gint window_x;
	gint window_y;
	GKeyFile *configrc_keyfile;
#ifdef HAVE_LIBCLASTFM
	gboolean lastfm_support;
	gchar *lastfm_user;
	gchar *lastfm_pass;
#endif
};

/**
 * struct con_state - Pertains to the current state of the player
 *
 * @unique_instance: If current invocation of app is unique
 * @view_change: If library view change is in progress
 * @last_folder: Last polder used in file chooser
 * @filter_entry: Search entry for filtering library
 * @cdda_drive: Global handle for the audio cd
 * @cddb_conn: libcddb connection
 * @curr_mobj: musicobject of currently playing track
 */

struct con_state {
	gboolean unique_instance;
	gboolean first_run;
	gchar *last_folder;
	cdrom_drive_t *cdda_drive;
	cddb_conn_t *cddb_conn;
	cddb_disc_t *cddb_disc;
	PraghaMusicobject *curr_mobj;
};

struct con_win {
	struct con_pref *cpref;
	struct con_state *cstate;
	PraghaPlaylist *cplaylist;
	PraghaLibraryPane *clibrary;
	PraghaBackend *backend;
	PraghaDatabase *cdbase;
	PraghaScanner  *scanner;
	PraghaSidebar *sidebar;
	PraghaStatusbar *statusbar;
	PraghaToolbar *toolbar;
	PraghaPreferences *preferences;
	PreferencesWidgets *preferences_w;
	#ifdef HAVE_LIBCLASTFM
	struct con_lastfm *clastfm;
	#endif
	PraghaMpris2 *cmpris2;
	con_gnome_media_keys *cgnome_media_keys;
	GtkWidget *mainwindow;
	GdkPixbuf *pixbuf_app;
	GtkWidget *info_box;
	GtkWidget *paned;
	GtkStatusIcon *status_icon;
	NotifyNotification *osd_notify;
	GtkUIManager *bar_context_menu;
	GtkUIManager *systray_menu;
#ifdef HAVE_LIBGLYR
	PraghaGlyr *glyr;
#endif
	guint related_timeout_id;
	DBusConnection *con_dbus;
};

/* Info bar import music */

gboolean info_bar_import_music_will_be_useful(struct con_win *cwin);
GtkWidget* create_info_bar_import_music(struct con_win *cwin);
GtkWidget* create_info_bar_update_music(struct con_win *cwin);

/* Init */

gint init_dbus(struct con_win *cwin);
gint init_options(struct con_win *cwin, int argc, char **argv);
gint init_taglib(struct con_win *cwin);
gint init_config(struct con_win *cwin);
gint init_audio(struct con_win *cwin);
gint init_threads(struct con_win *cwin);
gint init_first_state(struct con_win *cwin);
void state_free(struct con_state *cstate);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* Others */

void exit_pragha(GtkWidget *widget, struct con_win *cwin);

void toogle_main_window(struct con_win *cwin, gboolean ignoreActivity);

#endif /* PRAGHA_H */
