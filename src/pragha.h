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
#include "cdda.h" // Should be before config.h, libcdio 0.83 issue
#endif

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBGLYR
#include <glyr/glyr.h>
#include <glyr/cache.h>
#endif

#ifdef HAVE_LIBXFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#endif

#ifdef HAVE_PLPARSER
#include <totem-pl-parser.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libnotify/notify.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <tag_c.h>
#include <taglib_config.h>
#include <cddb/cddb.h>
#include <gst/gst.h>

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
#include "mpris.h"
#include "pragha-preferences.h"
#include "pragha-playlist.h"
#include "pragha-library-pane.h"
#include "pragha-lastfm.h"
#include "pragha-scanner.h"
#include "pragha-sidebar.h"
#include "pragha-simple-async.h"
#include "pragha-statusbar.h"
#include "pragha-toolbar.h"
#include "pragha-utils.h"
#include "gnome-media-keys.h"

#include "gtkcellrendererbubble.h"

#include "xml_helper.h"

/* With libcio 0.83 should be before config.h. libcdio issue */
#ifdef HAVE_PARANOIA_NEW_INCLUDES
#include "cdda.h"
#endif

/* Some default preferences. */

#define MIN_WINDOW_WIDTH           (gdk_screen_width() * 3 / 4)
#define MIN_WINDOW_HEIGHT          (gdk_screen_height() * 3 / 4)
#define PROGRESS_BAR_WIDTH         300
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH ((MIN_WINDOW_WIDTH - DEFAULT_SIDEBAR_SIZE) / 4)
#define OSD_TIMEOUT                5000
#define ALBUM_ART_PATTERN_LEN      1024
#define ALBUM_ART_NO_PATTERNS      6
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

#define DBUS_PATH      "/org/pragha/DBus"
#define DBUS_NAME      "org.pragha.DBus"
#define DBUS_INTERFACE "org.pragha.DBus"

#define DBUS_SIG_PLAY     "play"
#define DBUS_SIG_STOP     "stop"
#define DBUS_SIG_PAUSE    "pause"
#define DBUS_SIG_NEXT     "next"
#define DBUS_SIG_PREV     "prev"
#define DBUS_SIG_SHUFFLE  "shuffle"
#define DBUS_SIG_REPEAT   "repeat"
#define DBUS_SIG_INC_VOL  "inc_vol"
#define DBUS_SIG_DEC_VOL  "dec_vol"
#define DBUS_SIG_TOGGLE_VIEW      "toggle_view"
#define DBUS_SIG_SHOW_OSD "show_osd"
#define DBUS_SIG_ADD_FILE "add_files"

#define DBUS_METHOD_CURRENT_STATE "curent_state"
#define DBUS_EVENT_UPDATE_STATE   "update_state"

#if !GLIB_CHECK_VERSION(2,32,0)
/* Defines to get network manager status. */

#define NM_DBUS_SERVICE		"org.freedesktop.NetworkManager"
#define NM_DBUS_PATH		"/org/freedesktop/NetworkManager"
#define NM_DBUS_INTERFACE	"org.freedesktop.NetworkManager"

typedef enum {
        NM_STATE_UNKNOWN          = 0,
        NM_STATE_ASLEEP           = 10,
        NM_STATE_DISCONNECTED     = 20,
        NM_STATE_DISCONNECTING    = 30,
        NM_STATE_CONNECTING       = 40,
        NM_STATE_CONNECTED_LOCAL  = 50,
        NM_STATE_CONNECTED_SITE   = 60,
        NM_STATE_CONNECTED_GLOBAL = 70
} NMState;
#endif

#define TAG_TNO_CHANGED		1<<0
#define TAG_TITLE_CHANGED	1<<1
#define TAG_ARTIST_CHANGED	1<<2
#define TAG_ALBUM_CHANGED	1<<3
#define TAG_GENRE_CHANGED	1<<4
#define TAG_YEAR_CHANGED	1<<5
#define TAG_COMMENT_CHANGED	1<<6

#define PRAGHA_BUTTON_SKIP       _("_Skip")
#define PRAGHA_BUTTON_SKIP_ALL   _("S_kip All")
#define PRAGHA_BUTTON_DELETE_ALL _("Delete _All")

typedef enum {
	PRAGHA_RESPONSE_SKIP,
	PRAGHA_RESPONSE_SKIP_ALL,
	PRAGHA_RESPONSE_DELETE_ALL
} PraghaDeleteResponseType;

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

/* File Type */

enum file_type {
	FILE_WAV,
	FILE_MP3,
	FILE_FLAC,
	FILE_OGGVORBIS,
#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
	FILE_ASF,
#endif
#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
	FILE_MP4,
#endif
#ifdef HAVE_TAGLIB_1_7
	FILE_APE,
#endif
	FILE_CDDA,
	FILE_HTTP,
	LAST_FILE_TYPE
};

/* Playlist type formats */

enum playlist_type {
	PL_FORMAT_UNKNOWN,
	PL_FORMAT_M3U,
	PL_FORMAT_PLS,
	PL_FORMAT_ASX,
	PL_FORMAT_XSPF
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

extern gint debug_level;
extern const gchar *mime_mpeg[];
extern const gchar *mime_wav[];
extern const gchar *mime_flac[];
extern const gchar *mime_ogg[];
#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
extern const gchar *mime_asf[];
#endif
#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
extern const gchar *mime_mp4[];
#endif
#ifdef HAVE_TAGLIB_1_7
extern const gchar *mime_ape[];
#endif

extern const gchar *mime_image[];

#ifdef HAVE_PLPARSER
extern const gchar *mime_playlist[];
extern const gchar *mime_dual[];
#endif

/* Debugging */

#define CDEBUG(_lvl, _fmt, ...)			\
	if (G_UNLIKELY(_lvl <= debug_level))	\
		g_debug(_fmt, ##__VA_ARGS__);

/* Menu */

void open_file_action(GtkAction *action, struct con_win *cwin);
void add_audio_cd_action(GtkAction *action, struct con_win *cwin);
void add_location_action(GtkAction *action, struct con_win *cwin);
void prev_action(GtkAction *action, struct con_win *cwin);
void play_pause_action(GtkAction *action, struct con_win *cwin);
void stop_action(GtkAction *action, struct con_win *cwin);
void next_action (GtkAction *action, struct con_win *cwin);
void add_audio_cd(struct con_win *cwin);
void quit_action(GtkAction *action, struct con_win *cwin);
void expand_all_action(GtkAction *action, struct con_win *cwin);
void collapse_all_action(GtkAction *action, struct con_win *cwin);
void search_playlist_action(GtkAction *action, struct con_win *cwin);
void pref_action(GtkAction *action, struct con_win *cwin);
void fullscreen_action (GtkAction *action, struct con_win *cwin);
void playlists_pane_action (GtkAction *action, struct con_win *cwin);
void status_bar_action (GtkAction *action, struct con_win *cwin);
void show_controls_below_action (GtkAction *action, struct con_win *cwin);
void jump_to_playing_song_action (GtkAction *action, struct con_win *cwin);
void show_equalizer_action(GtkAction *action, struct con_win *cwin);
void rescan_library_action(GtkAction *action, struct con_win *cwin);
void update_library_action(GtkAction *action, struct con_win *cwin);
void add_libary_action(GtkAction *action, struct con_win *cwin);
void statistics_action(GtkAction *action, struct con_win *cwin);
void home_action(GtkAction *action, struct con_win *cwin);
void community_action(GtkAction *action, struct con_win *cwin);
void wiki_action(GtkAction *action, struct con_win *cwin);
void translate_action(GtkAction *action, struct con_win *cwin);
void about_action(GtkAction *action, struct con_win *cwin);
GtkUIManager* create_menu(struct con_win *cwin);

/* File utils functions */

gboolean is_playable_file(const gchar *file);
gboolean is_dir_and_accessible(const gchar *dir);
gint pragha_get_dir_count(const gchar *dir_name, GCancellable *cancellable);
gint dir_file_count(const gchar *dir_name, gint call_recur);
GList *append_mobj_list_from_folder(GList *list, gchar *dir_name);
GList *append_mobj_list_from_unknown_filename(GList *list, gchar *filename);

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
gint tag_edit_dialog(PraghaMusicobject *omobj, gint prechanged, PraghaMusicobject *nmobj, struct con_win *cwin);
void refresh_tag_completion_entries(struct con_win *cwin);
void copy_tags_selection_current_playlist(PraghaMusicobject *omobj, gint changes, struct con_win *cwin);
void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin);

/* Playlist mgmt functions */

gchar *get_playlist_name(enum playlist_mgmt type, GtkWidget *parent);
void add_playlist_current_playlist(gchar *playlist, struct con_win *cwin);
GList * add_playlist_to_mobj_list(PraghaDatabase *cdbase, const gchar *playlist, GList *list);
GList *add_radio_to_mobj_list(PraghaDatabase *cdbase, const gchar *playlist, GList *list);
gboolean delete_existing_item_dialog(const gchar *item, GtkWidget *parent);
gchar* rename_playlist_dialog(const gchar *oplaylist, GtkWidget *parent);
GIOChannel *create_m3u_playlist(gchar *file);
gint save_m3u_playlist(GIOChannel *chan, gchar *playlist, gchar *filename, PraghaDatabase *cdbase);
gchar *playlist_export_dialog_get_filename(const gchar *prefix, GtkWidget *parent);
void export_playlist (PraghaPlaylist* cplaylist, gint choice);
GList *
pragha_pl_parser_append_mobj_list_by_extension (GList *mlist, const gchar *file);
GSList *pragha_pl_parser_parse_from_file_by_extension (const gchar *filename);
GSList *pragha_totem_pl_parser_parse_from_uri(const gchar *uri);
void pragha_pl_parser_open_from_file_by_extension(const gchar *file, struct con_win *cwin);
void
save_playlist(PraghaPlaylist* cplaylist,
              gint playlist_id,
              enum playlist_mgmt type);
void
new_playlist(PraghaPlaylist* cplaylist,
             const gchar *playlist,
             enum playlist_mgmt type);
void append_playlist(PraghaPlaylist* cplaylist, const gchar *playlist, gint type);
void new_radio (PraghaPlaylist* cplaylist, const gchar *uri, const gchar *name);
void update_playlist_changes_on_menu(struct con_win *cwin);

/* Preferences */

void save_preferences(struct con_win *cwin);
void preferences_dialog(struct con_win *cwin);
void preferences_free(struct con_pref *cpref);

/* Systray functions */

void create_status_icon (struct con_win *cwin);
void systray_display_popup_menu (struct con_win *cwin);

void about_widget(struct con_win *cwin);

/* Command line functions */

gboolean cmd_version(const gchar *opt_name, const gchar *val,
		     struct con_win *cwin, GError **error);
gboolean cmd_play(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_stop(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_pause(const gchar *opt_name, const gchar *val,
		   struct con_win *cwin, GError **error);
gboolean cmd_prev(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_next(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error);
gboolean cmd_shuffle(const gchar *opt_name, const gchar *val,
		     struct con_win *cwin, GError **error);
gboolean cmd_repeat(const gchar *opt_name, const gchar *val,
		    struct con_win *cwin, GError **error);
gboolean cmd_inc_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error);
gboolean cmd_dec_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error);
gboolean cmd_show_osd(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);
gboolean cmd_toggle_view(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);
gboolean cmd_current_state(const gchar *opt_name, const gchar *val,
			   struct con_win *cwin, GError **error);
gboolean cmd_add_file(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);

/* D-BUS functions */

DBusHandlerResult dbus_filter_handler(DBusConnection *conn,
				      DBusMessage *msg,
				      gpointer data);
void dbus_send_signal(const gchar *signal, struct con_win *cwin);
void dbus_handlers_free(struct con_win *cwin);

/* Some widgets. */

gpointer sokoke_xfce_header_new (const gchar *header, const gchar *icon);
GtkWidget* pragha_search_entry_new(PraghaPreferences *preferences);

/* GUI */

GtkWidget* create_main_region(struct con_win *cwin);
GtkWidget* create_info_box(struct con_win *cwin);
GtkWidget* create_paned_region(struct con_win *cwin);
GtkWidget* create_search_bar(struct con_win *cwin);
GtkWidget* create_combo_order(struct con_win *cwin);
void create_status_icon(struct con_win *cwin);
gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin);
void mainwindow_add_widget_to_info_box(struct con_win *cwin, GtkWidget *widget);
void gui_free(struct con_win *cwin);
void gui_backend_error_show_dialog_cb (PraghaBackend *backend, const GError *error, gpointer user_data);
void gui_backend_error_update_current_playlist_cb (PraghaBackend *backend, const GError *error, struct con_win *cwin);

/* Info bar import music */

gboolean info_bar_import_music_will_be_useful(struct con_win *cwin);
GtkWidget* create_info_bar_import_music(struct con_win *cwin);
GtkWidget* create_info_bar_update_music(struct con_win *cwin);

/* Init */

gint init_dbus(struct con_win *cwin);
gint init_dbus_handlers(struct con_win *cwin);
gint init_options(struct con_win *cwin, int argc, char **argv);
gint init_taglib(struct con_win *cwin);
gint init_config(struct con_win *cwin);
gint init_audio(struct con_win *cwin);
gint init_threads(struct con_win *cwin);
gint init_first_state(struct con_win *cwin);
void state_free(struct con_state *cstate);
void init_tag_completion(struct con_win *cwin);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* notify */

gboolean can_support_actions(void);
void show_osd(struct con_win *cwin);
gint init_notify(struct con_win *cwin);
void notify_free();

/* Related info helpers */

void related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_action(GtkAction *action, struct con_win *cwin);
void related_get_lyric_action(GtkAction *action, struct con_win *cwin);

int init_glyr_related (struct con_win *cwin);
void glyr_related_free (struct con_win *cwin);

/* Others */

void pragha_filter_dialog (struct con_win *cwin);

void exit_pragha(GtkWidget *widget, struct con_win *cwin);

void toogle_main_window(struct con_win *cwin, gboolean ignoreActivity);

#endif /* PRAGHA_H */
