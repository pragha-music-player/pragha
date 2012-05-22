/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
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

#include "cdda.h" //should be before config.h, libcdio issue

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBKEYBINDER
#include <keybinder.h>
#endif

#ifdef HAVE_LIBCLASTFM
#include <clastfm.h>
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
#include <sqlite3.h>
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

#include "gtkcellrendererbubble.h"
#include "xml_helper.h"

/* Some definitions to solve different versions of the libraries. */

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

#if (!GTK_CHECK_VERSION(2, 23, 0) || GTK_CHECK_VERSION(2, 90, 0)) && !GTK_CHECK_VERSION(2, 91, 1)
#define GtkComboBoxText GtkComboBox
#define GTK_COMBO_BOX_TEXT(X) GTK_COMBO_BOX(X)
#define gtk_combo_box_text_new gtk_combo_box_new_text
#define gtk_combo_box_text_append_text(X,Y) gtk_combo_box_append_text(X,Y)
#define gtk_combo_box_text_get_active_text(X) gtk_combo_box_get_active_text(X)
#endif

/* Some default preferences. */

#define MIN_WINDOW_WIDTH           640
#define MIN_WINDOW_HEIGHT          480
#define DEFAULT_SIDEBAR_SIZE       200
#define ALBUM_ART_SIZE             36
#define PROGRESS_BAR_WIDTH         300
#define COL_WIDTH_THRESH           30
#define DEFAULT_PLAYLIST_COL_WIDTH 200
#define OSD_TIMEOUT                5000
#define ALBUM_ART_PATTERN_LEN      1024
#define ALBUM_ART_NO_PATTERNS      6
#define LASTFM_UNAME_LEN           256
#define LASTFM_PASS_LEN            512
#define TAG_MAX_LEN                256
#define AUDIO_CD_DEVICE_ENTRY_LEN  32

#define P_TRACK_NO_STR      "#"
#define P_TNO_FULL_STR      N_("Track No")
#define P_TITLE_STR         N_("Title")
#define P_ARTIST_STR        N_("Artist")
#define P_ALBUM_STR         N_("Album")
#define P_GENRE_STR         N_("Genre")
#define P_BITRATE_STR       N_("Bitrate")
#define P_YEAR_STR          N_("Year")
#define P_COMMENT_STR       N_("Comment")
#define P_LENGTH_STR        N_("Length")
#define P_FILENAME_STR      N_("Filename")

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

#define DEFAULT_SINK "default"
#define ALSA_SINK    "alsa"
#define OSS4_SINK    "oss4"
#define OSS_SINK     "oss"
#define PULSE_SINK   "pulse"

#define DEFAULT_MIXER "default"
#define ALSA_MIXER    "alsa"
#define OSS_MIXER     "oss"
#define SOFT_MIXER    "software"

#define ALSA_DEFAULT_DEVICE "default"
#define OSS_DEFAULT_DEVICE  "/dev/dsp"

#define PROGRESS_BAR_TEXT           _("Scanning")
#define TRACK_PROGRESS_BAR_STOPPED  _("Stopped")
#define SAVE_PLAYLIST_STATE         "con_playlist"

#define LASTFM_API_KEY             "ecdc2d21dbfe1139b1f0da35daca9309"
#define LASTFM_SECRET              "f3498ce387f30eeae8ea1b1023afb32b"

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

#define MPRIS_NAME "org.mpris.MediaPlayer2.pragha"
#define MPRIS_PATH "/org/mpris/MediaPlayer2"

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

/* Defines to key preferences. */

#define GROUP_GENERAL  "General"
#define KEY_INSTALLED_VERSION      "installed_version"
#define KEY_LAST_FOLDER            "last_folder"
#define KEY_ADD_RECURSIVELY_FILES  "add_recursively_files"
#define KEY_ALBUM_ART_PATTERN      "album_art_pattern"
#define KEY_TIMER_REMAINING_MODE   "timer_remaining_mode"
#define KEY_CLOSE_TO_TRAY	   "close_to_tray"
#define KEY_SHOW_OSD               "show_osd"
#define KEY_OSD_IN_TRAY            "osd_in_tray"
#define KEY_SHOW_ALBUM_ART_OSD     "show_albumart_osd"
#define KEY_SHOW_ACTIONS_OSD       "show_action_osd"
#define KEY_INSTANT_FILTER         "instant_filter"
#define KEY_APROXIMATE_SEARCH      "aproximate_search"
#define KEY_USE_HINT               "use_hint"

#define GROUP_PLAYLIST "Playlist"
#define KEY_SAVE_PLAYLIST          "save_playlist"
#define KEY_CURRENT_REF		   "current_ref"
#define KEY_SHUFFLE                "shuffle"
#define KEY_REPEAT                 "repeat"
#define KEY_PLAYLIST_COLUMNS       "playlist_columns"
#define KEY_PLAYLIST_COLUMN_WIDTHS "playlist_column_widths"

#define GROUP_LIBRARY  "Library"
#define KEY_LIBRARY_DIR            "library_dir"
#define KEY_LIBRARY_DELETE         "library_delete"
#define KEY_LIBRARY_ADD            "library_add"
#define KEY_LIBRARY_TREE_NODES     "library_tree_nodes"
#define KEY_LIBRARY_VIEW_ORDER     "library_view_order"
#define KEY_LIBRARY_LAST_SCANNED   "library_last_scanned"
#define KEY_FUSE_FOLDERS	   "library_fuse_folders"
#define KEY_SORT_BY_YEAR	   "library_sort_by_year"

#define GROUP_AUDIO    "Audio"
#define KEY_AUDIO_SINK             "audio_sink"
#define KEY_AUDIO_DEVICE           "audio_device"
#define KEY_SOFTWARE_MIXER         "software_mixer"
#define KEY_SOFTWARE_VOLUME	   "software_volume"
#define KEY_AUDIO_CD_DEVICE        "audio_cd_device"
#define KEY_EQ_10_BANDS            "equealizer_10_bands"
#define KEY_EQ_PRESET              "equalizer_preset"

#define GROUP_WINDOW   "Window"
#define KEY_REMEMBER_STATE	   "remember_window_state"
#define KEY_START_MODE		   "start_mode"
#define KEY_WINDOW_SIZE            "window_size"
#define KEY_WINDOW_POSITION        "window_position"
#define KEY_SIDEBAR_SIZE           "sidebar_size"
#define KEY_SIDEBAR                "sidebar"
#define KEY_SHOW_ALBUM_ART         "show_album_art"
#define KEY_ALBUM_ART_SIZE         "album_art_size"
#define KEY_STATUS_BAR             "status_bar"
#define KEY_CONTROLS_BELOW         "controls_below"

#define GROUP_SERVICES   "services"
#define KEY_LASTFM                 "lastfm"
#define KEY_LASTFM_USER            "lastfm_user"
#define KEY_LASTFM_PASS            "lastfm_pass"
#define KEY_GET_ALBUM_ART          "get_album_art"
#define KEY_USE_CDDB               "use_cddb"
#define KEY_ALLOW_MPRIS2           "allow_mpris2"

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

/* Current playlist movement */

enum playlist_action {
	PLAYLIST_CURR = 1,
	PLAYLIST_NEXT,
	PLAYLIST_PREV
};

/* Player state */

enum player_state {
	ST_PLAYING = 1,
	ST_STOPPED,
	ST_PAUSED
};

/* Node types in library view */

enum node_type {
	NODE_GENRE,
	NODE_ARTIST,
	NODE_ALBUM,
	NODE_TRACK,
	NODE_FOLDER,
	NODE_PLAYLIST,
	NODE_RADIO,
	NODE_BASENAME
};

/* Columns in Library view */

enum library_columns {
	L_PIXBUF,
	L_NODE_DATA,
	L_NODE_TYPE,
	L_LOCATION_ID,
	L_VISIBILE,
	N_L_COLUMNS
};

/* Columns in current playlist view */

enum curplaylist_columns {
	P_MOBJ_PTR,
	P_QUEUE,
	P_BUBBLE,
	P_STATUS_PIXBUF,
	P_TRACK_NO,
	P_TITLE,
	P_ARTIST,
	P_ALBUM,
	P_GENRE,
	P_BITRATE,
	P_YEAR,
	P_COMMENT,
	P_LENGTH,
	P_FILENAME,
	P_PLAYED,
	N_P_COLUMNS
};

/* DnD */

gboolean tree_selection_func_true(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data);

gboolean tree_selection_func_false(GtkTreeSelection *selection,
					       GtkTreeModel *model,
					       GtkTreePath *path,
					       gboolean path_currently_selected,
					       gpointer data);

enum dnd_target {
	TARGET_REF_LIBRARY,
	TARGET_URI_LIST,
	TARGET_PLAIN_TEXT
};

/* Library Views */

enum library_view {
	FOLDERS,
	ARTIST,
	ALBUM,
	GENRE,
	ARTIST_ALBUM,
	GENRE_ARTIST,
	GENRE_ALBUM,
	GENRE_ARTIST_ALBUM
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
	FILE_HTTP
};

/* Playlist type formats */

enum playlist_type {
	PL_FORMAT_UNKNOWN,
	PL_FORMAT_M3U,
	PL_FORMAT_PLS,
	PL_FORMAT_ASX,
	PL_FORMAT_XSPF
};

/* Track sources for last.fm submission */

enum track_source {
	USER_SOURCE,
	BROADCAST_SOURCE,
	RECO_SOURCE,
	LASTFM_SOURCE,
	UNKNOWN_SOURCE
};

struct tags {
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	gchar *comment;
	guint year;
	guint track_no;
	gint length;
	gint bitrate;
	gint channels;
	gint samplerate;
};

struct pixbuf {
	GdkPixbuf *pixbuf_app;
	GdkPixbuf *pixbuf_artist;
	GdkPixbuf *pixbuf_album;
	GdkPixbuf *pixbuf_track;
	GdkPixbuf *pixbuf_genre;
	GdkPixbuf *pixbuf_dir;
	GdkPixbuf *pixbuf_paused;
	GdkPixbuf *pixbuf_playing;
	GtkWidget *image_pause;
	GtkWidget *image_play;
};

#ifdef HAVE_LIBCLASTFM
struct lastfm_pref {
	gboolean lastfm_support;
	gchar *lastfm_user;
	gchar *lastfm_pass;
	GtkWidget *lastfm_w;
	GtkWidget *lastfm_uname_w;
	GtkWidget *lastfm_pass_w;
};
#endif

struct con_pref {
	enum library_view cur_library_view;
	gchar *installed_version;
	gchar *audio_sink;
	gchar *album_art_pattern;
	gchar *start_mode;
	gchar *audio_device;
	gchar *audio_cd_device;
	gint album_art_size;
	gint window_width;
	gint window_height;
	gint window_x;
	gint window_y;
	gint sidebar_size;
	GTimeVal last_rescan_time;
	GKeyFile *configrc_keyfile;
	gchar *configrc_file;
#ifdef HAVE_LIBGLYR
	gchar *cache_folder;
#endif
	gboolean add_recursively_files;
	gboolean show_album_art;
	gboolean show_osd;
	gboolean osd_in_systray;
	gboolean albumart_in_osd;
	gboolean actions_in_osd;
	gboolean timer_remaining_mode;
	gboolean shuffle;
	gboolean repeat;
	gboolean save_playlist;
	gboolean software_mixer;
#ifdef HAVE_LIBGLYR
	gboolean get_album_art;
#endif
	gboolean use_cddb;
	gboolean use_mpris2;
	gboolean close_to_tray;
	gboolean remember_window_state;
	gboolean lateral_panel;
	gboolean status_bar;
	gboolean controls_below;
	gboolean fuse_folders;
	gboolean sort_by_year;
	gboolean instant_filter;
	gboolean aproximate_search;
	gboolean use_hint;
	GSList *library_dir;
	GSList *playlist_columns;
	GSList *playlist_column_widths;
	GSList *library_tree_nodes;
	GSList *lib_delete;
	GSList *lib_add;

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
	GtkWidget *close_to_tray_w;
	GtkWidget *add_recursively_w;

	GtkWidget *show_osd_w;
	GtkWidget *osd_in_systray_w;
	GtkWidget *albumart_in_osd_w;
	GtkWidget *actions_in_osd_w;

#ifdef HAVE_LIBCLASTFM
	struct lastfm_pref lw;
#endif
#ifdef HAVE_LIBGLYR
	GtkWidget *get_album_art_w;
#endif
	GtkWidget *use_cddb_w;
	GtkWidget *use_mpris2_w;
};

struct musicobject {
	struct tags *tags;
	gchar *file;
	enum file_type file_type; /* FILE_MP3,... */
};

struct db_result {
	gchar **resultp;
	gint no_rows;
	gint no_columns;
};

/**
 * struct con_state - Pertains to the current state of the player
 *
 * @unique_instance: If current invocation of app is unique
 * @stop_scan: Flag to stop rescan process
 * @view_change: If library view change is in progress
 * @playlist_change: If current platlist change is in progress
 * @curr_mobj_clear: Clear curr_mobj flag
 * @state: State of the player { ST_STOPPED, ... }
 * @cmd: Thread Command {CMD_PLAYBACK_STOP, ... }
 * @seek_len: New seek length to pass to playback thread
 * @tracks_curr_playlist: Total no. of tracks in the current playlist
 * @unplayed_tracks: Total no. of tracks that haven't been played
 * @newsec: Arg for idle func invoked from playback thread
 * @seek_fraction: New seek fraction to pass to playback thread
 * @last_folder: Last polder used in file chooser
 * @filter_entry: Search entry for filtering library
 * @rand: To generate random numbers
 * @c_cond: Cond Between playback thread and main process
 * @rand_track_refs: List of references maintained in Shuffle mode
 * @curr_rand_ref: Currently playing track in Shuffle mode
 * @curr_seq_ref: Currently playing track in non-Shuffle mode
 * @cdda_drive: Global handle for the audio cd
 * @cddb_conn: libcddb connection
 * @curr_mobj: musicobject of currently playing track
 */

struct con_state {
	enum player_state state;
	gboolean dragging;
	gboolean unique_instance;
	gboolean stop_scan;
	gboolean view_change;
	gboolean playlist_change;
	gboolean curr_mobj_clear;
	gint seek_len;
	gint tracks_curr_playlist;
	gint unplayed_tracks;
	gint newsec;
	gint timeout_id;
	gdouble seek_fraction;
	gchar *last_folder;
	gchar *filter_entry;
	gchar *jump_filter;
	gchar *arturl;
	GRand *rand;
	GList *rand_track_refs;
	GSList *queue_track_refs;
	GtkTreeRowReference *curr_rand_ref;
	GtkTreeRowReference *curr_seq_ref;
	cdrom_drive_t *cdda_drive;
	cddb_conn_t *cddb_conn;
	cddb_disc_t *cddb_disc;
	struct musicobject *curr_mobj;
};

struct con_dbase {
	gchar *db_file;	/* Filename of the DB file (~/.pragha.db) */
	sqlite3 *db;	/* SQLITE3 handle of the opened DB */
#ifdef HAVE_LIBGLYR
	GlyrDatabase *cache_db;
#endif
};

struct con_gst {
	GstElement *pipeline;
	GstElement *audio_sink;
	GstElement *equalizer;
	int timer;
	gdouble curr_vol;
	gboolean emitted_error;
};

#ifdef HAVE_LIBCLASTFM
struct con_lastfm {
	LASTFM_SESSION *session_id;
	enum LASTFM_STATUS_CODES status;
	struct tags *ntags;
	time_t playback_started;
};
#endif

struct con_mpris2 {
	guint owner_id;
	GDBusNodeInfo *introspection_data;
	GDBusConnection *dbus_connection;
	GQuark interface_quarks[4];
	gboolean saved_playbackstatus;
	gboolean saved_shuffle;
	gchar *saved_title;
	gdouble volume;
	enum player_state state;
	GError **property_error;			/* for returning errors in propget/propput */
	GDBusMethodInvocation *method_invocation;	/* for returning errors during methods */
};

struct con_gnome_media_keys {
	guint watch_id;
	guint handler_id;
	GDBusProxy *proxy;
};

struct con_win {
	struct pixbuf *pixbuf;
	struct con_pref *cpref;
	struct con_state *cstate;
	struct con_dbase *cdbase;
	struct con_gst *cgst;
	#ifdef HAVE_LIBCLASTFM
	struct con_lastfm *clastfm;
	#endif
	struct con_mpris2 *cmpris2;
	struct con_gnome_media_keys *cgnome_media_keys;
	GtkWidget *mainwindow;
	GtkWidget *hbox_panel;
	GtkWidget *album_art_frame;
	GtkWidget *album_art;
	GtkWidget *track_progress_bar;
	GtkWidget *prev_button;
	GtkWidget *play_button;
	GtkWidget *stop_button;
	GtkWidget *next_button;
	GtkWidget *unfull_button;
	GtkWidget *shuffle_button;
	GtkWidget *repeat_button;
	GtkWidget *vol_button;
	GtkWidget *current_playlist;
	GtkWidget *status_bar;
	GtkWidget *search_entry;
	GtkWidget *browse_mode;
	GtkWidget *paned;
	GtkWidget *toggle_lib;
	GtkWidget *combo_order;
	GtkWidget *combo_order_label;
	GtkWidget *track_length_label;
	GtkWidget *track_time_label;
	GtkWidget *now_playing_label;
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *ntag_lastfm_button;
	#endif
	GtkWidget *library_tree;
	GtkWidget *jump_tree;
	GtkWidget *header_context_menu;
	GtkTreeStore *library_store;
	GtkStatusIcon *status_icon;
	NotifyNotification *osd_notify;
	GtkEntryCompletion *completion[3];
	GtkUIManager *bar_context_menu;
	GtkUIManager *cp_context_menu;
	GtkUIManager *cp_null_context_menu;
	GtkUIManager *playlist_tree_context_menu;
	GtkUIManager *library_tree_context_menu;
	GtkUIManager *library_page_context_menu;
	GtkUIManager *systray_menu;
	gint related_timeout_id;
	DBusConnection *con_dbus;
};

extern gulong switch_cb_id;
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

/*Open file function*/

void handle_selected_file(gpointer data, gpointer udata);

/* Convenience macros */

#define for_each_result_row(result, i)					\
	for (i=result.no_columns; i<((result.no_rows+1)*(result.no_columns)); \
	     i+=result.no_columns)

#define SCALE_UP_VOL(vol) volume_convert(vol, cwin->cmixer->min_vol,	\
					 cwin->cmixer->max_vol, 0, 100)
#define SCALE_DOWN_VOL(vol) volume_convert(vol, 0, 100, cwin->cmixer->min_vol, \
					   cwin->cmixer->max_vol)

/* Debugging */

#define CDEBUG(_lvl, _fmt, ...)			\
	if (G_UNLIKELY(_lvl <= debug_level))	\
		g_debug(_fmt, ##__VA_ARGS__);

/* Menu actions */
void update_menubar_playback_state (struct con_win *cwin);
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
void search_library_action(GtkAction *action, struct con_win *cwin);
void search_playlist_action(GtkAction *action, struct con_win *cwin);
void shuffle_action(GtkToggleAction *action, struct con_win *cwin);
void repeat_action(GtkToggleAction *action, struct con_win *cwin);
void pref_action(GtkAction *action, struct con_win *cwin);
void fullscreen_action (GtkAction *action, struct con_win *cwin);
void library_pane_action (GtkAction *action, struct con_win *cwin);
void playlists_pane_action (GtkAction *action, struct con_win *cwin);
void status_bar_action (GtkAction *action, struct con_win *cwin);
void show_controls_below_action (GtkAction *action, struct con_win *cwin);
void jump_to_playing_song_action (GtkAction *action, struct con_win *cwin);
void show_equalizer_action(GtkAction *action, struct con_win *cwin);
void rescan_library_action(GtkAction *action, struct con_win *cwin);
void update_library_action(GtkAction *action, struct con_win *cwin);
void statistics_action(GtkAction *action, struct con_win *cwin);
void home_action(GtkAction *action, struct con_win *cwin);
void community_action(GtkAction *action, struct con_win *cwin);
void wiki_action(GtkAction *action, struct con_win *cwin);
void translate_action(GtkAction *action, struct con_win *cwin);
void about_action(GtkAction *action, struct con_win *cwin);

void rescan_library_handler(struct con_win *cwin);

/* Panel actions */

gboolean update_current_song_info(gpointer data);
void __update_progress_song_info(struct con_win *cwin, gint length);
void __update_current_song_info(struct con_win *cwin);
void unset_current_song_info(struct con_win *cwin);
gboolean update_track_progress_bar(gpointer data);
void __update_track_progress_bar(struct con_win *cwin, gint length);
void unset_track_progress_bar(struct con_win *cwin);
void timer_remaining_mode_change(GtkWidget *w, GdkEventButton* event, struct con_win *cwin);
void edit_tags_playing_event(GtkWidget *w, GdkEventButton* event, struct con_win *cwin);
void track_progress_change_cb(GtkWidget *widget, GdkEventButton *event,struct con_win *cwin);
gboolean album_art_frame_press_callback (GtkWidget *event_box, GdkEventButton *event, struct con_win *cwin);
void update_album_art(struct musicobject *mobj, struct con_win *cwin);
void unset_album_art(struct con_win *cwin);
gboolean panel_button_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin);
void unfull_button_handler(GtkButton *button, struct con_win *cwin);
void shuffle_button_handler(GtkToggleButton *button, struct con_win *cwin);
void repeat_button_handler(GtkToggleButton *button, struct con_win *cwin);
void play_button_handler(GtkButton *button, struct con_win *cwin);
void stop_button_handler(GtkButton *button, struct con_win *cwin);
void prev_button_handler(GtkButton *button, struct con_win *cwin);
void next_button_handler(GtkButton *button, struct con_win *cwin);
void vol_button_handler(GtkScaleButton *button, gdouble value, struct con_win *cwin);
void update_panel_playback_state(struct con_win *cwin);
void album_art_toggle_state(struct con_win *cwin);
void resize_album_art_frame(struct con_win *cwin);
void toggled_cb(GtkToggleButton *toggle, struct con_win *cwin);

/* File tree functions */

void __non_recur_add(gchar *dir_name, gboolean init, struct con_win *cwin);
void __recur_add(gchar *dir_name, struct con_win *cwin);
GList *append_mobj_list_from_folder(GList *list, gchar *dir_name, struct con_win *cwin);

/* Musicobject functions */

struct musicobject* new_musicobject_from_file(gchar *file);
struct musicobject* new_musicobject_from_db(gint location_id, struct con_win *cwin);
struct musicobject* new_musicobject_from_cdda(struct con_win *cwin, gint track_no);
struct musicobject* new_musicobject_from_location(gchar *uri, const gchar *name, struct con_win *cwin);
void update_musicobject(struct musicobject *mobj, gint changed, struct tags *ntag, struct con_win *cwin);
void init_tag_struct(struct tags *mtags);
void free_tag_struct(struct tags *mtags);
void delete_musicobject(struct musicobject *mobj);
void test_delete_musicobject(struct musicobject *mobj, struct con_win *cwin);

/* Tag functions */

gboolean get_wav_info(gchar *file, struct tags *tags);
gboolean get_mp3_info(gchar *file, struct tags *tags);
gboolean get_flac_info(gchar *file, struct tags *tags);
gboolean get_ogg_info(gchar *file, struct tags *tags);
#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
gboolean get_asf_info(gchar *file, struct tags *tags);
#endif
#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
gboolean get_mp4_info(gchar *file, struct tags *tags);
#endif
#ifdef HAVE_TAGLIB_1_7
gboolean get_ape_info(gchar *file, struct tags *tags);
#endif
gboolean save_tags_to_file(gchar *file, struct tags *tags,
			   int changed, struct con_win *cwin);
void tag_update(GArray *loc_arr, GArray *file_arr, gint changed, struct tags *ntag,
		struct con_win *cwin);
gint tag_edit_dialog(struct tags *otag, gint prechanged, struct tags *ntag, gchar *file,
		     struct con_win *cwin);
void refresh_tag_completion_entries(struct con_win *cwin);
void edit_tags_current_playlist(GtkAction *action, struct con_win *cwin);


/* Library manipulation functions */

void library_tree_row_activated_cb(GtkTreeView *library_tree,
				   GtkTreePath *path,
				   GtkTreeViewColumn *column,
				   struct con_win *cwin);
gboolean library_tree_button_press_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin);
gboolean library_tree_button_release_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin);
gboolean library_page_right_click_cb(GtkWidget *widget,
				     GdkEventButton *event,
				     struct con_win *cwin);
gboolean dnd_library_tree_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin);
gboolean dnd_library_tree_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin);
void dnd_library_tree_get(GtkWidget *widget,
			  GdkDragContext *context,
			  GtkSelectionData *data,
			  enum dnd_target info,
			  guint time,
			  struct con_win *cwin);
void simple_library_search_keyrelease(struct con_win *cwin);
gboolean simple_library_search_keyrelease_handler(GtkEntry *entry, struct con_win *cwin);
gboolean simple_library_search_activate_handler(GtkEntry *entry, struct con_win *cwin);
void clear_library_search(struct con_win *cwin);
void folders_library_tree(GtkAction *action, struct con_win *cwin);
void artist_library_tree(GtkAction *action, struct con_win *cwin);
void album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_library_tree(GtkAction *action, struct con_win *cwin);
void artist_album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_album_library_tree(GtkAction *action, struct con_win *cwin);
void library_tree_replace_playlist(GtkAction *action, struct con_win *cwin);
void library_tree_replace_and_play(GtkAction *action, struct con_win *cwin);
void library_tree_add_to_playlist(struct con_win *cwin);
void library_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin);
void library_tree_edit_tags(GtkAction *action, struct con_win *cwin);
void library_tree_delete_db(GtkAction *action, struct con_win *cwin);
void library_tree_delete_hdd(GtkAction *action, struct con_win *cwin);
void init_library_view(struct con_win *cwin);

/* DB (Sqlite) Functions */

gint add_new_artist_db(gchar *artist, struct con_win *cwin);
gint add_new_album_db(gchar *album, struct con_win *cwin);
gint add_new_genre_db(gchar *genre, struct con_win *cwin);
gint add_new_year_db(guint year, struct con_win *cwin);
gint add_new_comment_db(gchar *comment, struct con_win *cwin);
gint add_new_location_db(gchar *location, struct con_win *cwin);
void add_track_playlist_db(gchar *file, gint playlist_id, struct con_win *cwin);
void add_track_radio_db(gchar *uri, gint radio_id, struct con_win *cwin);
gint find_artist_db(const gchar *artist, struct con_win *cwin);
gint find_album_db(const gchar *album, struct con_win *cwin);
gint find_genre_db(const gchar *genre, struct con_win *cwin);
gint find_year_db(gint year, struct con_win *cwin);
gint find_comment_db(const gchar *comment, struct con_win *cwin);
gint find_location_db(const gchar *location, struct con_win *cwin);
gint find_playlist_db(const gchar *playlist, struct con_win *cwin);
gint find_radio_db(const gchar *radio, struct con_win *cwin);
void delete_location_db(gint location_id, struct con_win *cwin);
gint delete_location_hdd(gint location_id, struct con_win *cwin);
void update_track_db(gint location_id, gint changed,
		     gint track_no, gchar *title,
		     gint artist_id, gint album_id, gint genre_id, gint year_id, gint comment_id,
		     struct con_win *cwin);
void update_playlist_name_db(const gchar *oplaylist, gchar *nplaylist, struct con_win *cwin);
gint add_new_playlist_db(const gchar *playlist, struct con_win *cwin);
gchar** get_playlist_names_db(struct con_win *cwin);
gint get_playlist_count_db(struct con_win *cwin);
void update_radio_name_db(const gchar *oradio, gchar *nradio, struct con_win *cwin);
gint add_new_radio_db(const gchar *radio, struct con_win *cwin);
gchar** get_radio_names_db(struct con_win *cwin);
gint get_radio_count_db(struct con_win *cwin);
gint get_tracklist_count_db(struct con_win *cwin);
void delete_playlist_db(gchar *playlist, struct con_win *cwin);
void flush_playlist_db(gint playlist_id, struct con_win *cwin);
void delete_radio_db(gchar *radio, struct con_win *cwin);
void flush_radio_db(gint radio_id, struct con_win *cwin);
void flush_stale_entries_db(struct con_win *cwin);
void flush_db(struct con_win *cwin);
gboolean fraction_update(GtkWidget *pbar);
void rescan_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
void update_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
void delete_db(gchar *dir_name, gint no_files, GtkWidget *pbar,
	       gint call_recur, struct con_win *cwin);
gint init_dbase_schema(struct con_win *cwin);
gint drop_dbase_schema(struct con_win *cwin);
gboolean exec_sqlite_query(gchar *query, struct con_win *cwin,
			   struct db_result *result);

/* Playlist mgmt functions */

void add_playlist_current_playlist(gchar *playlist, struct con_win *cwin);
GList *prepend_playlist_to_mobj_list(gchar *playlist, GList *list, struct con_win *cwin);
void add_playlist_current_playlist_on_model(GtkTreeModel *model, gchar *playlist, struct con_win *cwin);
void add_radio_current_playlist(gchar *playlist, struct con_win *cwin);
GList *prepend_radio_to_mobj_list(gchar *playlist, GList *list, struct con_win *cwin);
void add_radio_current_playlist_on_model(GtkTreeModel *model, gchar *radio, struct con_win *cwin);
void playlist_tree_replace_playlist(GtkAction *action, struct con_win *cwin);
void playlist_tree_replace_and_play(GtkAction *action, struct con_win *cwin);
void playlist_tree_add_to_playlist(struct con_win *cwin);
void playlist_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin);
void playlist_tree_rename(GtkAction *action, struct con_win *cwin);
void playlist_tree_delete(GtkAction *action, struct con_win *cwin);
void export_playlist (gint choice, struct con_win *cwin);
void playlist_tree_export(GtkAction *action, struct con_win *cwi);
GSList *pragha_pl_parser_parse_from_file_by_extension (const gchar *filename);
GSList *pragha_totem_pl_parser_parse_from_uri(const gchar *uri);
void pragha_pl_parser_open_from_file_by_extension(gchar *file, struct con_win *cwin);
gboolean dnd_playlist_tree_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin);
void dnd_playlist_tree_get(GtkWidget *widget,
			   GdkDragContext *context,
			   GtkSelectionData *data,
			   guint info,
			   guint time,
			   struct con_win *cwin);
void append_files_to_playlist(GSList *list, gint playlist_id, struct con_win *cwin);
void save_playlist(gint playlist_id, enum playlist_mgmt type,
		   struct con_win *cwin);
void new_playlist(const gchar *playlist, enum playlist_mgmt type,
		  struct con_win *cwin);
void append_playlist(const gchar *playlist, gint type, struct con_win *cwin);
void new_radio (gchar *uri, gchar *name, struct con_win *cwin);
void complete_add_to_playlist_submenu (struct con_win *cwin);
void complete_save_playlist_submenu (struct con_win *cwin);
void complete_main_save_playlist_submenu (struct con_win *cwin);
void complete_main_add_to_playlist_submenu (struct con_win *cwin);

/* Current playlist */

void jump_to_path_on_current_playlist(GtkTreePath *path, struct con_win *cwin);
void select_last_path_of_current_playlist(struct con_win *cwin);
void update_pixbuf_state_on_path(GtkTreePath *path, GError *error, struct con_win *cwin);
void update_status_bar(struct con_win *cwin);
void update_current_state(GtkTreePath *path,
			  enum playlist_action action,
			  struct con_win *cwin);
struct musicobject* current_playlist_mobj_at_path(GtkTreePath *path,
						  struct con_win *cwin);
GtkTreePath* current_playlist_path_at_mobj(struct musicobject *mobj,
						struct con_win *cwin);
void reset_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin);
void current_playlist_clear_dirty_all(struct con_win *cwin);
GtkTreePath* current_playlist_get_selection(struct con_win *cwin);
GtkTreePath* current_playlist_get_next(struct con_win *cwin);
GtkTreePath* current_playlist_get_prev(struct con_win *cwin);
GtkTreePath* current_playlist_get_actual(struct con_win *cwin);
GtkTreePath* get_first_random_track(struct con_win *cwin);
GtkTreePath* get_next_queue_track(struct con_win *cwin);
gchar* get_ref_current_track(struct con_win *cwin);
void init_current_playlist_columns(struct con_win *cwin);
void requeue_track_refs (struct con_win *cwin);
void dequeue_current_playlist(GtkAction *action, struct con_win *cwin);
void queue_current_playlist(GtkAction *action, struct con_win *cwin);
void toggle_queue_selected_current_playlist (struct con_win *cwin);
int current_playlist_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin);
void remove_from_playlist(GtkAction *action, struct con_win *cwin);
void crop_current_playlist(GtkAction *action, struct con_win *cwin);
void edit_tags_playing_action(GtkAction *action, struct con_win *cwin);
void track_properties(struct musicobject *mobj, struct con_win *cwin);
void clear_current_playlist(GtkAction *action, struct con_win *cwin);
void update_track_current_playlist(GtkTreeIter *iter, gint changed, struct musicobject *mobj, struct con_win *cwin);
void insert_current_playlist(struct musicobject *mobj, GtkTreeViewDropPosition droppos, GtkTreeIter *pos, struct con_win *cwin);
void append_current_playlist(struct musicobject *mobj, struct con_win *cwin);
void append_current_playlist_ex(struct musicobject *mobj, struct con_win *cwin, GtkTreePath **path);
void append_current_playlist_on_model(GtkTreeModel *model, struct musicobject *mobj, struct con_win *cwin);
void clear_sort_current_playlist(GtkAction *action, struct con_win *cwin);
void save_selected_playlist(GtkAction *action, struct con_win *cwin);
void save_current_playlist(GtkAction *action, struct con_win *cwin);
void play_first_current_playlist(struct con_win *cwin);
void play_prev_track(struct con_win *cwin);
void play_next_track(struct con_win *cwin);
void play_track(struct con_win *cwin);
void pause_resume_track(struct con_win *cwin);
void play_pause_resume(struct con_win *cwin);
void shuffle_button(struct con_win *cwin);
void jump_to_playing_song(struct con_win *cwin);
void current_playlist_row_activated_cb(GtkTreeView *current_playlist,
				       GtkTreePath *path,
				       GtkTreeViewColumn *column,
				       struct con_win *cwin);
gboolean current_playlist_button_press_cb(GtkWidget *widget,
					 GdkEventButton *event,
					 struct con_win *cwin);
gboolean current_playlist_button_release_cb(GtkWidget *widget,
					    GdkEventButton *event,
					    struct con_win *cwin);
gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       struct con_win *cwin);
gboolean dnd_current_playlist_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin);
void drag_current_playlist_get_data (GtkWidget *widget,
				    GdkDragContext *context,
				    GtkSelectionData *selection_data,
				    guint target_type,
				    guint time,
				    struct con_win *cwin);
gboolean dnd_current_playlist_drop(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   guint time,
				   struct con_win *cwin);
void dnd_current_playlist_received(GtkWidget *widget,
				   GdkDragContext *context,
				   gint x,
				   gint y,
				   GtkSelectionData *data,
				   enum dnd_target info,
				   guint time,
				   struct con_win *cwin);
void save_current_playlist_state(struct con_win *cwin);
void init_current_playlist_view(struct con_win *cwin);
void playlist_track_column_change_cb(GtkCheckMenuItem *item,
				     struct con_win *cwin);
void playlist_title_column_change_cb(GtkCheckMenuItem *item,
				     struct con_win *cwin);
void playlist_artist_column_change_cb(GtkCheckMenuItem *item,
				      struct con_win *cwin);
void playlist_album_column_change_cb(GtkCheckMenuItem *item,
				     struct con_win *cwin);
void playlist_genre_column_change_cb(GtkCheckMenuItem *item,
				     struct con_win *cwin);
void playlist_bitrate_column_change_cb(GtkCheckMenuItem *item,
				       struct con_win *cwin);
void playlist_year_column_change_cb(GtkCheckMenuItem *item,
				    struct con_win *cwin);
void playlist_length_column_change_cb(GtkCheckMenuItem *item,
				      struct con_win *cwin);
void playlist_comment_column_change_cb(GtkCheckMenuItem *item,
				     struct con_win *cwin);
void playlist_filename_column_change_cb(GtkCheckMenuItem *item,
					struct con_win *cwin);
void clear_sort_current_playlist_cb(GtkMenuItem *item,
					struct con_win *cwin);
gint compare_track_no(GtkTreeModel *model, GtkTreeIter *a,
		      GtkTreeIter *b, gpointer data);
gint compare_bitrate(GtkTreeModel *model, GtkTreeIter *a,
		     GtkTreeIter *b, gpointer data);
gint compare_year(GtkTreeModel *model, GtkTreeIter *a,
		  GtkTreeIter *b, gpointer data);
gint compare_length(GtkTreeModel *model, GtkTreeIter *a,
		    GtkTreeIter *b, gpointer data);

/* Preferences */

void save_preferences(struct con_win *cwin);
void preferences_dialog(struct con_win *cwin);
void free_library_dir(struct con_win *cwin);
void free_library_add_dir(struct con_win *cwin);
void free_library_delete_dir(struct con_win *cwin);
void free_playlist_columns(struct con_win *cwin);
void free_library_tree_nodes(struct con_win *cwin);

/* Gstreamer */

void backend_seek (guint64 seek, struct con_win *cwin);
gint64 backend_get_current_length(struct con_win *cwin);
gint64 backend_get_current_position(struct con_win *cwin);
void backend_set_soft_volume(struct con_win *cwin);
gdouble backend_get_volume (struct con_win *cwin);
gboolean update_volume_notify_cb (struct con_win *cwin);
void backend_set_volume (gdouble volume, struct con_win *cwin);
void backend_update_volume (struct con_win *cwin);
gboolean backend_is_playing(struct con_win *cwin);
gboolean backend_is_paused(struct con_win *cwin);
void backend_pause (struct con_win *cwin);
void backend_resume (struct con_win *cwin);
void backend_play (struct con_win *cwin);
void backend_stop (GError *error, struct con_win *cwin);
void backend_start(struct musicobject *mobj, struct con_win *cwin);
void backend_quit (struct con_win *cwin);
gint backend_init(struct con_win *cwin);

/* Audio functions */

void set_alsa_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_oss_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_soft_mixer(struct con_win *cwin);
void soft_volume_apply(gchar *buffer, gint buflen, struct con_win *cwin);
GSList* alsa_pcm_devices(struct con_win *cwin);
gint open_audio_device(gint samplerate, gint channels,
		       gboolean resume, struct con_win *cwin);

/* Systray functions */

gboolean can_support_actions(void);
void show_osd(struct con_win *cwin);
gboolean status_icon_clicked (GtkWidget *widget, GdkEventButton *event, struct con_win *cwin);
gboolean status_get_tooltip_cb (GtkWidget *widget, gint x, gint y, gboolean keyboard_mode,GtkTooltip *tooltip, struct con_win *cwin);
void create_status_icon (struct con_win *cwin);
void systray_display_popup_menu (struct con_win *cwin);
void systray_play_pause_action(GtkAction *action, struct con_win *cwin);
void systray_stop_action(GtkAction *action, struct con_win *cwin);
void systray_prev_action(GtkAction *action, struct con_win *cwin);
void systray_next_action(GtkAction *action, struct con_win *cwin);
void systray_quit(GtkAction *action, struct con_win *cwin);

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

/* MPRIS functions */

gint mpris_init(struct con_win *cwin);
void mpris_update_seeked(struct con_win *cwin, gint position);
gboolean handle_path_request(struct con_win *cwin, const gchar *dbus_path, struct musicobject **mobj, GtkTreePath **tree_path);
void mpris_update_any(struct con_win *cwin);
void mpris_update_metadata_changed(struct con_win *cwin);
void mpris_update_mobj_remove(struct con_win *cwin, struct musicobject *mobj);
void mpris_update_mobj_added(struct con_win *cwin, struct musicobject *mobj, GtkTreeIter *iter);
void mpris_update_mobj_changed(struct con_win *cwin, struct musicobject *mobj, gint bitmask);
void mpris_update_tracklist_replaced(struct con_win *cwin);
void mpris_close(struct con_win *cwin);
void mpris_cleanup(struct con_win *cwin);

/* Utilities */

gchar *e2_utf8_ndup (const gchar *str, glong num);
gsize levenshtein_strcmp(const gchar * s, const gchar * t);
gsize levenshtein_safe_strcmp(const gchar * s, const gchar * t);
gchar *g_strstr_lv (gchar *haystack, gchar *needle, gsize lv_distance);
gboolean nm_is_online ();
gboolean already_in_current_playlist(struct musicobject *mobj, struct con_win *cwin);
gint append_track_with_artist_and_title(gchar *artist, gchar *title, struct con_win *cwin);
struct musicobject *get_selected_musicobject(struct con_win *cwin);
void set_watch_cursor_on_thread(struct con_win *cwin);
void remove_watch_cursor_on_thread(gchar *message, struct con_win *cwin);
void set_status_message (gchar *message, struct con_win *cwin);
void gtk_label_set_attribute_bold(GtkLabel *label);
GdkPixbuf *vgdk_pixbuf_new_from_memory (char *data, size_t size);
gboolean is_playable_file(const gchar *file);
gboolean is_dir_and_accessible(gchar *dir, struct con_win *cwin);
gint dir_file_count(gchar *dir_name, gint call_recur);
gchar* sanitize_string_sqlite3(gchar *str);
enum file_type get_file_type(gchar *file);
gchar* get_mime_type(const gchar *file);
enum playlist_type pragha_pl_parser_guess_format_from_extension (const gchar *filename);
gboolean is_image_file(const gchar *file);
gchar* convert_length_str(gint length);
gboolean is_present_str_list(const gchar *str, GSList *list);
GSList* delete_from_str_list(const gchar *str, GSList *list);
gchar* get_display_filename(const gchar *filename, gboolean get_folder);
void free_str_list(GSList *list);
gint compare_utf8_str(gchar *str1, gchar *str2);
gboolean validate_album_art_pattern(const gchar *pattern);
void open_url( struct con_win *cwin, const gchar *url);
void menu_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data);
gboolean is_incompatible_upgrade(struct con_win *cwin);

/* Some widgets. */

gpointer sokoke_xfce_header_new (const gchar *header, const gchar *icon, struct con_win *cwin);
GtkWidget* pragha_search_entry_new(struct con_win *cwin);

/* GUI */

GtkUIManager* create_menu(struct con_win *cwin);
GtkWidget* create_main_region(struct con_win *cwin);
GtkWidget* create_panel(struct con_win *cwin);
GtkWidget* create_playing_box(struct con_win *cwin);
GtkWidget* create_paned_region(struct con_win *cwin);
GtkWidget* create_status_bar(struct con_win *cwin);
GtkWidget* create_search_bar(struct con_win *cwin);
GtkWidget* create_combo_order(struct con_win *cwin);
void create_status_icon(struct con_win *cwin);
gboolean dialog_audio_init(gpointer data);
gboolean exit_gui(GtkWidget *widget, GdkEvent *event, struct con_win *cwin);

/* Init */

gint init_dbus(struct con_win *cwin);
gint init_dbus_handlers(struct con_win *cwin);
gint init_options(struct con_win *cwin, int argc, char **argv);
gint init_config(struct con_win *cwin);
gint init_musicdbase(struct con_win *cwin);
gint init_audio(struct con_win *cwin);
gint init_threads(struct con_win *cwin);
gint init_notify(struct con_win *cwin);
gint init_first_state(struct con_win *cwin);
void init_tag_completion(struct con_win *cwin);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* gnome media keys */

gint init_gnome_media_keys(struct con_win *cwin);
void cleanup_gnome_media_keys(struct con_win *cwin);

/* keybinder */

gint init_keybinder(struct con_win *cwin);
void cleanup_keybinder(struct con_win *cwin);

/* Lastfm Helper */

void update_menubar_lastfm_state (struct con_win *cwin);
void edit_tags_corrected_by_lastfm(GtkButton *button, struct con_win *cwin);
void lastfm_get_similar_current_playlist_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_current_playlist_love_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_current_playlist_unlove_action (GtkAction *action, struct con_win *cwin);
void lastfm_add_favorites_action (GtkAction *action, struct con_win *cwin);
void lastfm_get_similar_action (GtkAction *action, struct con_win *cwin);
void lastfm_import_xspf_action (GtkAction *action, struct con_win *cwin);
void lastfm_track_love_action(GtkAction *action, struct con_win *cwin);
void lastfm_track_unlove_action (GtkAction *action, struct con_win *cwin);
void lastfm_now_playing_handler (struct con_win *cwin);
gint just_init_lastfm (struct con_win *cwin);
gint init_lastfm_idle(struct con_win *cwin);

/* Related info helpers */

void related_get_lyric_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_current_playlist_action(GtkAction *action, struct con_win *cwin);
void related_get_artist_info_action(GtkAction *action, struct con_win *cwin);
void related_get_lyric_action(GtkAction *action, struct con_win *cwin);

int uninit_glyr_related (struct con_win *cwin);
int init_glyr_related (struct con_win *cwin);

void update_related_state (struct con_win *cwin);

/* Others */

void dialog_jump_to_track (struct con_win *cwin);

void common_cleanup(struct con_win *cwin);
void exit_pragha(GtkWidget *widget, struct con_win *cwin);

void toogle_main_window(struct con_win *cwin, gboolean ignoreActivity);
void systray_volume_scroll (GtkWidget *widget, GdkEventScroll *event, struct con_win *cwin);
GtkUIManager* create_systray_menu(struct con_win *cwin);

#endif /* PRAGHA_H */
