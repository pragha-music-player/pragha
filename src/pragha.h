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

#ifdef HAVE_LIBGLYR
#include <glyr/glyr.h>
#include <glyr/cache.h>
#endif

//#include <stdio.h>
//#include <stdlib.h>
//#include <errno.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
//#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
//#include <gio/gio.h>
//#include <dbus/dbus.h>
//#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libnotify/notify.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

/* Some definitions to solve different versions of the libraries. */

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

#include "pragha-album-art.h"
#include "pragha-backend.h"
#include "pragha-database.h"
#include "pragha-musicobject.h"
#include "pragha-preferences.h"
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

#define SAVE_PLAYLIST_STATE         "con_playlist"

#define WAIT_UPDATE 5

#define TAG_TNO_CHANGED		1<<0
#define TAG_TITLE_CHANGED	1<<1
#define TAG_ARTIST_CHANGED	1<<2
#define TAG_ALBUM_CHANGED	1<<3
#define TAG_GENRE_CHANGED	1<<4
#define TAG_YEAR_CHANGED	1<<5
#define TAG_COMMENT_CHANGED	1<<6

enum debug_level {
	DBG_BACKEND = 1,
	DBG_INFO,
	DBG_LASTFM,
	DBG_MPRIS,
	DBG_MOBJ,
	DBG_DB,
	DBG_VERBOSE,
};

enum dnd_target {
	TARGET_REF_LIBRARY,
	TARGET_URI_LIST,
	TARGET_PLAIN_TEXT
};

/* Playlist management */

enum playlist_mgmt {
	NEW_PLAYLIST,
	APPEND_PLAYLIST,
	EXPORT_PLAYLIST,
	SAVE_COMPLETE,
	SAVE_SELECTED
};

typedef struct {
	GtkWidget *audio_device_w;
	GtkWidget *audio_cd_device_w;
	GtkWidget *audio_sink_combo_w;
	GtkWidget *soft_mixer_w;

	GtkWidget *use_hint_w;
	GtkWidget *album_art_w;
	GtkWidget *album_art_size_w;
	GtkWidget *album_art_pattern_w;

	GtkWidget *library_view_w;
	GtkWidget *fuse_folders_w;
	GtkWidget *sort_by_year_w;

	GtkWidget *instant_filter_w;
	GtkWidget *aproximate_search_w;
	GtkWidget *window_state_combo_w;
	GtkWidget *restore_playlist_w;
	GtkWidget *show_icon_tray_w;
	GtkWidget *close_to_tray_w;
	GtkWidget *add_recursively_w;

	GtkWidget *show_osd_w;
#if !NOTIFY_CHECK_VERSION (0, 7, 0)
	GtkWidget *osd_in_systray_w;
#endif
	GtkWidget *albumart_in_osd_w;
	GtkWidget *actions_in_osd_w;

#ifdef HAVE_LIBCLASTFM
	GtkWidget *lastfm_w;
	GtkWidget *lastfm_uname_w;
	GtkWidget *lastfm_pass_w;
#endif
#ifdef HAVE_LIBGLYR
	GtkWidget *get_album_art_w;
#endif
	GtkWidget *use_cddb_w;
	GtkWidget *use_mpris2_w;
}PreferencesWidgets;

struct con_pref {
	gchar *installed_version;
	gchar *album_art_pattern;
	gchar *start_mode;
	gint window_width;
	gint window_height;
	gint window_x;
	gint window_y;
	GKeyFile *configrc_keyfile;
#ifdef HAVE_LIBGLYR
	gchar *cache_folder;
#endif
	gboolean show_osd;
	gboolean osd_in_systray;
	gboolean albumart_in_osd;
	gboolean actions_in_osd;
#ifdef HAVE_LIBGLYR
	gboolean get_album_art;
#endif
	gboolean use_cddb;
	gboolean use_mpris2;
	gboolean show_icon_tray;
	gboolean close_to_tray;
	gboolean remember_window_state;
	gboolean controls_below;
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
	PRAGHA_MUTEX (curr_mobj_mutex);
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
	struct con_gnome_media_keys *cgnome_media_keys;
	GtkWidget *mainwindow;
	GdkPixbuf *pixbuf_app;
	GtkWidget *info_box;
	GtkWidget *paned;
	GtkStatusIcon *status_icon;
	NotifyNotification *osd_notify;
	GtkEntryCompletion *completion[3];
	GtkUIManager *bar_context_menu;
	GtkUIManager *systray_menu;
#ifdef HAVE_LIBGLYR
	GlyrDatabase *cache_db;
#endif
	guint related_timeout_id;
	DBusConnection *con_dbus;
};

/* Debugging */

extern gint debug_level;

#define CDEBUG(_lvl, _fmt, ...)			\
	if (G_UNLIKELY(_lvl <= debug_level))	\
		g_debug(_fmt, ##__VA_ARGS__);

/* Musicobject functions */

PraghaMusicobject* new_musicobject_from_file(const gchar *file);
PraghaMusicobject* new_musicobject_from_db(PraghaDatabase *cdbase, gint location_id);
PraghaMusicobject* new_musicobject_from_cdda(struct con_win *cwin, gint track_no);
PraghaMusicobject* new_musicobject_from_location(const gchar *uri, const gchar *name);
void pragha_update_musicobject_change_tag(PraghaMusicobject *mobj, gint changed, PraghaMusicobject *nmobj);

/* Tag functions */

gboolean pragha_musicobject_set_tags_from_file(PraghaMusicobject *mobj, const gchar *file);
gboolean pragha_musicobject_save_tags_to_file(gchar *file, PraghaMusicobject *mobj, int changed);
gboolean confirm_tno_multiple_tracks(gint tno, GtkWidget *parent);
gboolean confirm_title_multiple_tracks(const gchar *title, GtkWidget *parent);
void pragha_update_local_files_change_tag(GPtrArray *file_arr, gint changed, PraghaMusicobject *mobj);
void refresh_tag_completion_entries(struct con_win *cwin);
void copy_tags_selection_current_playlist(PraghaMusicobject *omobj, gint changes, struct con_win *cwin);
void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin);

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
void init_tag_completion(struct con_win *cwin);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* Related info helpers */

void related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_action(GtkAction *action, struct con_win *cwin);
void related_get_lyric_action(GtkAction *action, struct con_win *cwin);

int init_glyr_related (struct con_win *cwin);
void glyr_related_free (struct con_win *cwin);

/* Others */

void exit_pragha(GtkWidget *widget, struct con_win *cwin);

void toogle_main_window(struct con_win *cwin, gboolean ignoreActivity);

#endif /* PRAGHA_H */
