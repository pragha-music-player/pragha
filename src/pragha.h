/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

#if HAVE_CONFIG_H
#include <config.h>
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
#include <ao/ao.h>
#include <curl/curl.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <cddb/cddb.h>
/*#include <libintl.h>*/

#include "mp3.h"
#include "wav.h"
#include "flac.h"
#include "oggvorbis.h"
#include "cdda.h"
#include "mod.h"

#include "gtkcellrendererbubble.h"
#include "keybinder.h"

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
#define OSS_SINK     "oss"

#define DEFAULT_MIXER "default"
#define ALSA_MIXER    "alsa"
#define OSS_MIXER     "oss"
#define SOFT_MIXER    "software"

#define ALSA_DEFAULT_DEVICE "default"
#define OSS_DEFAULT_DEVICE  "/dev/dsp"

#define PROGRESS_BAR_TEXT           _("Scanning")
#define TRACK_PROGRESS_BAR_STOPPED  _("Stopped")
#define SAVE_PLAYLIST_STATE         "con_playlist"

#define LASTFM_URL                 "http://post.audioscrobbler.com"
#define LASTFM_PORT                80
#define LASTFM_CONN_TIMEOUT        20 /* seconds */
#define LASTFM_OPER_TIMEOUT        60 /* seconds */
#define LASTFM_SUBMISSION_PROTOCOL "1.2.1"
#define LASTFM_CLIENT_ID           "cns"
#define LASTFM_CLIENT_VERSION      ".0.4.1"
#define LASTFM_MIN_PLAYTIME        30  /* seconds */
#define LASTFM_MIN_DURATION        240 /* seconds */

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
#define DBUS_SIG_SHOW_OSD "show_osd"
#define DBUS_SIG_ADD_FILE "add_files"

#define DBUS_METHOD_CURRENT_STATE "curent_state"

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

#define GROUP_AUDIO    "Audio"
#define KEY_AUDIO_SINK             "audio_sink"
#define KEY_AUDIO_ALSA_DEVICE      "audio_alsa_device"
#define KEY_AUDIO_OSS_DEVICE       "audio_oss_device"
#define KEY_SOFTWARE_MIXER         "software_mixer"
#define KEY_SOFTWARE_VOLUME	   "software_volume"
#define KEY_AUDIO_CD_DEVICE        "audio_cd_device"

#define GROUP_WINDOW   "Window"
#define KEY_REMEMBER_STATE	   "remember_window_state"
#define KEY_START_MODE		   "start_mode"
#define KEY_WINDOW_SIZE            "window_size"
#define KEY_SIDEBAR_SIZE           "sidebar_size"
#define KEY_SIDEBAR                "sidebar"
#define KEY_SHOW_ALBUM_ART         "show_album_art"
#define KEY_ALBUM_ART_SIZE         "album_art_size"
#define KEY_STATUS_BAR		   "status_bar"

#define GROUP_SERVICES   "services"
#define KEY_LASTFM                 "lastfm"
#define KEY_LASTFM_USER            "lastfm_user"
#define KEY_LASTFM_PASS            "lastfm_pass"
#define KEY_USE_CDDB               "use_cddb"

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
	DBG_INFO = 1,
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

/* Thread commands */

enum thread_cmd {
	CMD_PLAYBACK_STOP = 1,
	CMD_PLAYBACK_PAUSE,
	CMD_PLAYBACK_RESUME,
	CMD_PLAYBACK_SEEK
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

/* Columns in Playlist view */

enum playlist_columns {
	P_PIXBUF,
	P_PLAYLIST,
	N_PL_COLUMNS
};

/* Columns in current playlist view */

enum curplaylist_columns {
	P_MOBJ_PTR,
	P_QUEUE,
	P_BUBBLE,
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
	TARGET_LOCATION_ID,
	TARGET_PLAYLIST,
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
	SAVE_COMPLETE,
	SAVE_SELECTED
};

/* File Type */

enum file_type {
	FILE_WAV,
	FILE_MP3,
	FILE_FLAC,
	FILE_OGGVORBIS,
	FILE_MODPLUG,
	FILE_CDDA
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
	GtkWidget *image_pause;
	GtkWidget *image_play;
};

struct lastfm_pref {
	gboolean lastfm_support;
	gchar *lastfm_user;
	gchar *lastfm_pass;
	GtkWidget *lastfm_w;
	GtkWidget *lastfm_uname_w;
	GtkWidget *lastfm_pass_w;
};

struct con_pref {
	enum library_view cur_library_view;
	gchar *installed_version;
	gchar *audio_sink;
	gchar *album_art_pattern;
	gchar *start_mode;
	gchar *sidebar_pane;
	gchar *audio_alsa_device;
	gchar *audio_oss_device;
	gchar *audio_cd_device;
	gint album_art_size;
	gint window_width;
	gint window_height;
	gint sidebar_size;
	GTimeVal last_rescan_time;
	GKeyFile *configrc_keyfile;
	gchar *configrc_file;
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
	gboolean use_cddb;
	gboolean close_to_tray;
	gboolean remember_window_state;
	gboolean status_bar;
	gboolean fuse_folders;
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

	GtkWidget *library_view_w;
	GtkWidget *fuse_folders_w;

	GtkWidget *window_state_combo_w;
	GtkWidget *restore_playlist_w;
	GtkWidget *close_to_tray_w;
	GtkWidget *add_recursively_w;
	GtkWidget *album_art_w;
	GtkWidget *album_art_size_w;
	GtkWidget *album_art_pattern_w;

	GtkWidget *show_osd_w;
	GtkWidget *osd_in_systray_w;
	GtkWidget *albumart_in_osd_w;
	GtkWidget *actions_in_osd_w;

	struct lastfm_pref lw;
	GtkWidget *use_cddb_w;
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

struct lastfm_track {
	struct con_win *cwin;
	gint play_duration;
	GTimeVal start_time;
	enum track_source tsource;
	struct tags tags;
};

/**
 * struct con_state - Pertains to the current state of the player
 *
 * @unique_instance: If current invocation of app is unique
 * @stop_scan: Flag to stop rescan process
 * @view_change: If library view change is in progress
 * @curr_mobj_clear: Clear curr_mobj flag
 * @advance_track: Advance to next track - set by playback thread
 * @state: State of the player { ST_STOPPED, ... }
 * @cmd: Thread Command {CMD_PLAYBACK_STOP, ... }
 * @seek_len: New seek length to pass to playback thread
 * @tracks_curr_playlist: Total no. of tracks in the current playlist
 * @unplayed_tracks: Total no. of tracks that haven't been played
 * @newsec: Arg for idle func invoked from playback thread
 * @lastfm_hard_failure: If more than 3, should do a handshake again
 * @seek_fraction: New seek fraction to pass to playback thread
 * @last_folder: Last polder used in file chooser
 * @filter_entry: Search entry for filtering library
 * @rand: To generate random numbers
 * @c_thread: Playback thread
 * @c_mutex: Mutex between playback thread and main process
 * @l_mutex: Mutex for last.fm submission thread
 * @c_cond: Cond Between playback thread and main process
 * @rand_track_refs: List of references maintained in Shuffle mode
 * @curr_rand_ref: Currently playing track in Shuffle mode
 * @curr_seq_ref: Currently playing track in non-Shuffle mode
 * @cdda_drive: Global handle for the audio cd
 * @cddb_conn: libcddb connection
 * @curr_mobj: musicobject of currently playing track
 */

struct con_state {
	enum thread_cmd cmd;
	enum player_state state;
	gboolean dragging;
	gboolean unique_instance;
	gboolean audio_init;
	gboolean stop_scan;
	gboolean view_change;
	gboolean curr_mobj_clear;
	gboolean advance_track;
	gint seek_len;
	gint tracks_curr_playlist;
	gint unplayed_tracks;
	gint newsec;
	gint lastfm_hard_failure;
	gint timeout_id;
	gdouble seek_fraction;
	gchar *last_folder;
	gchar *filter_entry;
	GRand *rand;
	GThread *c_thread;
	GMutex *c_mutex;
	GMutex *l_mutex;
	GCond *c_cond;
	GList *rand_track_refs;
	GSList *queue_track_refs;
	GtkTreeRowReference *curr_rand_ref;
	GtkTreeRowReference *curr_seq_ref;
	cdrom_drive_t *cdda_drive;
	cddb_conn_t *cddb_conn;
	cddb_disc_t *cddb_disc;
	struct musicobject *curr_mobj;
};

struct con_mixer {
	gchar *mixer_elem;
	glong min_vol;
	glong max_vol;
	glong curr_vol;
	void (*set_volume)(struct con_win *);
	void (*inc_volume)(struct con_win *);
	void (*dec_volume)(struct con_win *);
	gint (*init_mixer)(struct con_win *);
	void (*deinit_mixer)(struct con_win *);
	gint (*mute_mixer)(struct con_win *);
};

struct con_dbase {
	gchar *db_file;	/* Filename of the DB file (~/.pragha.db) */
	sqlite3 *db;	/* SQLITE3 handle of the opened DB */
};

struct con_libao {
	gint ao_driver_id;
	ao_device *ao_dev;
	ao_option *ao_opts;
	ao_sample_format format;
};

#define LASTFM_NOT_CONNECTED 1<<1
#define LASTFM_CONNECTED     1<<2

struct con_lastfm {
	CURL *curl_handle;
	gint state; /* LASTFM_* */
	gchar *session_id;
	gchar *submission_url;
};

struct con_win {
	struct pixbuf *pixbuf;
	struct con_pref *cpref;
	struct con_state *cstate;
	struct con_dbase *cdbase;
	struct con_mixer *cmixer;
	struct con_libao *clibao;
	struct con_lastfm *clastfm;
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
	GtkWidget *toggle_playlists;
	GtkWidget *combo_order;
	GtkWidget *combo_order_label;
	GtkWidget *track_length_label;
	GtkWidget *track_time_label;
	GtkWidget *now_playing_label;
	GtkWidget *library_tree;
	GtkWidget *playlist_tree;
	GtkWidget *header_context_menu;
	GtkTreeStore *library_store;
	GOptionContext *cmd_context;
	GtkStatusIcon *status_icon;
	GtkEntryCompletion *completion[3];
	GtkUIManager *bar_context_menu;
	GtkUIManager *cp_context_menu;
	GtkUIManager *playlist_tree_context_menu;
	GtkUIManager *library_tree_context_menu;
	GtkUIManager *library_page_context_menu;
	GtkUIManager *systray_menu;
	DBusConnection *con_dbus;
};
extern gulong switch_cb_id;
extern gint debug_level;
extern const gchar *mime_mpeg[];
extern const gchar *mime_wav[];
extern const gchar *mime_flac[];
extern const gchar *mime_ogg[];
extern const gchar *mime_image[];

/*Open file function*/

void handle_selected_file(gpointer data, gpointer udata);

/* Conversion routine from alsaplayer */

static inline long volume_convert(glong val, long omin, long omax,
				  long nmin, long nmax) {
        float orange = omax - omin, nrange = nmax - nmin;

        if (orange == 0)
                return 0;
        return ((nrange * (val - omin)) + (orange / 2)) / orange + nmin;
}

/* Init last.fm track status */

static inline void lastfm_track_reset(struct con_win *cwin,
				      struct lastfm_track **ltrack)
{
	*ltrack = g_slice_new0(struct lastfm_track);
	(*ltrack)->cwin = cwin;
	g_get_current_time(&(*ltrack)->start_time);
	(*ltrack)->tsource = USER_SOURCE;
	memcpy(&(*ltrack)->tags, cwin->cstate->curr_mobj->tags,
	       sizeof(struct tags));
}

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
	if (_lvl <= debug_level)		\
		g_debug(_fmt, ##__VA_ARGS__);

/* Menu actions */
void open_file_action(GtkAction *action, struct con_win *cwin);
void play_audio_cd_action(GtkAction *action, struct con_win *cwin);
void prev_action(GtkAction *action, struct con_win *cwin);
void play_pause_action(GtkAction *action, struct con_win *cwin);
void stop_action(GtkAction *action, struct con_win *cwin);
void next_action (GtkAction *action, struct con_win *cwin);
void play_audio_cd(struct con_win *cwin);
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
void jump_to_playing_song_action (GtkAction *action, struct con_win *cwin);
void rescan_library_action(GtkAction *action, struct con_win *cwin);
void update_library_action(GtkAction *action, struct con_win *cwin);
void add_all_action(GtkAction *action, struct con_win *cwin);
void statistics_action(GtkAction *action, struct con_win *cwin);
void lyric_action(GtkAction *action, struct con_win *cwin);
void home_action(GtkAction *action, struct con_win *cwin);
void community_action(GtkAction *action, struct con_win *cwin);
void wiki_action(GtkAction *action, struct con_win *cwin);
void translate_action(GtkAction *action, struct con_win *cwin);
void about_action(GtkAction *action, struct con_win *cwin);

void rescan_library_handler(struct con_win *cwin);

/* Global Hotkeys handlers */
void keybind_prev_handler (const char *keystring, gpointer data);
void keybind_play_handler (const char *keystring, gpointer data);
void keybind_stop_handler (const char *keystring, gpointer data);
void keybind_next_handler (const char *keystring, gpointer data);
void keybind_media_handler (const char *keystring, gpointer data);

/* Panel actions */
gboolean update_current_song_info(gpointer data);
void __update_progress_song_info(struct con_win *cwin, gint length);
void __update_current_song_info(struct con_win *cwin);
void unset_current_song_info(struct con_win *cwin);
gboolean update_track_progress_bar(gpointer data);
void __update_track_progress_bar(struct con_win *cwin, gint length);
void unset_track_progress_bar(struct con_win *cwin);
void timer_remaining_mode_change(GtkWidget *w, GdkEventButton* event, struct con_win *cwin);
void track_progress_change_cb(GtkWidget *widget,
			      GdkEventButton *event,
			      struct con_win *cwin);
void update_album_art(struct musicobject *mobj, struct con_win *cwin);
void unset_album_art(struct con_win *cwin);
void unfull_button_handler(GtkButton *button, struct con_win *cwin);
void shuffle_button_handler(GtkToggleButton *button, struct con_win *cwin);
void repeat_button_handler(GtkToggleButton *button, struct con_win *cwin);
void play_button_handler(GtkButton *button, struct con_win *cwin);
void stop_button_handler(GtkButton *button, struct con_win *cwin);
void prev_button_handler(GtkButton *button, struct con_win *cwin);
void next_button_handler(GtkButton *button, struct con_win *cwin);
void jump_to_playing_song_handler(GtkButton *button, struct con_win *cwin);
void vol_button_handler(GtkScaleButton *button, gdouble value,
			struct con_win *cwin);
void play_button_toggle_state(struct con_win *cwin);
void album_art_toggle_state(struct con_win *cwin);
void resize_album_art_frame(struct con_win *cwin);
void toggled_cb(GtkToggleButton *toggle, struct con_win *cwin);

/* File tree functions */

void __non_recur_add(gchar *dir_name, gboolean init, struct con_win *cwin);
void __recur_add(gchar *dir_name, struct con_win *cwin);

/* Musicobject functions */

struct musicobject* new_musicobject_from_file(gchar *file);
struct musicobject* new_musicobject_from_db(gint location_id,
					    struct con_win *cwin);
struct musicobject* new_musicobject_from_cdda(struct con_win *cwin,
					      gint track_no);
void update_musicobject(struct musicobject *mobj, gint changed, struct tags *ntag, struct con_win *cwin);
void delete_musicobject(struct musicobject *mobj);
void test_delete_musicobject(struct musicobject *mobj, struct con_win *cwin);

/* Tag functions */

gboolean get_wav_info(gchar *file, struct tags *tags);
gboolean get_mp3_info(gchar *file, struct tags *tags);
gboolean get_flac_info(gchar *file, struct tags *tags);
gboolean get_ogg_info(gchar *file, struct tags *tags);
gboolean get_mod_info(gchar *file, struct tags *tags);
gboolean save_tags_to_file(gchar *file, struct tags *tags,
			   int changed, struct con_win *cwin);
void tag_update(GArray *loc_arr, GArray *file_arr, gint changed, struct tags *ntag,
		struct con_win *cwin);
gint tag_edit_dialog(struct tags *otag, struct tags *ntag, gchar *file,
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
gboolean simple_library_search_keyrelease_handler(GtkEntry *entry,
						  struct con_win *cwin);
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
gint find_artist_db(const gchar *artist, struct con_win *cwin);
gint find_album_db(const gchar *album, struct con_win *cwin);
gint find_genre_db(const gchar *genre, struct con_win *cwin);
gint find_year_db(gint year, struct con_win *cwin);
gint find_comment_db(const gchar *comment, struct con_win *cwin);
gint find_location_db(const gchar *location, struct con_win *cwin);
gint find_playlist_db(const gchar *playlist, struct con_win *cwin);
void delete_location_db(gint location_id, struct con_win *cwin);
gint delete_location_hdd(gint location_id, struct con_win *cwin);
void update_track_db(gint location_id, gint changed,
		     gint track_no, gchar *title,
		     gint artist_id, gint album_id, gint genre_id, gint year_id, gint comment_id,
		     struct con_win *cwin);
gint add_new_playlist_db(const gchar *playlist, struct con_win *cwin);
gchar** get_playlist_names_db(struct con_win *cwin);
void delete_playlist_db(gchar *playlist, struct con_win *cwin);
void flush_playlist_db(gint playlist_id, struct con_win *cwin);
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
void playlist_tree_row_activated_cb(GtkTreeView *playlist_tree,
				    GtkTreePath *path,
				    GtkTreeViewColumn *column,
				    struct con_win *cwin);
gboolean playlist_tree_button_press_cb(GtkWidget *widget,
				      GdkEventButton *event,
				      struct con_win *cwin);
gboolean playlist_tree_button_release_cb(GtkWidget *widget,
				      GdkEventButton *event,
				      struct con_win *cwin);
void playlist_tree_replace_playlist(GtkAction *action, struct con_win *cwin);
void playlist_tree_add_to_playlist(struct con_win *cwin);
void playlist_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin);
void playlist_tree_delete(GtkAction *action, struct con_win *cwin);
void playlist_tree_export(GtkAction *action, struct con_win *cwi);
void open_m3u_playlist(gchar *file, struct con_win *cwin);
gboolean dnd_playlist_tree_begin(GtkWidget *widget,
				    GdkDragContext *context,
				    struct con_win *cwin);
void dnd_playlist_tree_get(GtkWidget *widget,
			   GdkDragContext *context,
			   GtkSelectionData *data,
			   guint info,
			   guint time,
			   struct con_win *cwin);
void save_playlist(gint playlist_id, enum playlist_mgmt type,
		   struct con_win *cwin);
void new_playlist(const gchar *playlist, enum playlist_mgmt type,
		  struct con_win *cwin);
void append_playlist(const gchar *playlist, gint type, struct con_win *cwin);
void init_playlist_view(struct con_win *cwin);

/* Current playlist */

void update_current_state(GThread *thread,
			  GtkTreePath *path,
			  enum playlist_action action,
			  struct con_win *cwin);
struct musicobject* current_playlist_mobj_at_path(GtkTreePath *path,
						  struct con_win *cwin);
void reset_rand_track_refs(GtkTreeRowReference *ref, struct con_win *cwin);
void current_playlist_clear_dirty_all(struct con_win *cwin);
GtkTreePath* current_playlist_get_selection(struct con_win *cwin);
GtkTreePath* current_playlist_get_next(struct con_win *cwin);
GtkTreePath* current_playlist_get_prev(struct con_win *cwin);
GtkTreePath* current_playlist_get_actual(struct con_win *cwin);
GtkTreePath* get_next_queue_track(struct con_win *cwin);
gchar* get_ref_current_track(struct con_win *cwin);
void init_current_playlist_columns(struct con_win *cwin);
void requeue_track_refs (struct con_win *cwin);
void enqueue_current_playlist(GtkAction *action, struct con_win *cwin);
void queue_current_playlist(GtkAction *action, struct con_win *cwin);
int current_playlist_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin);
void remove_current_playlist(GtkAction *action, struct con_win *cwin);
void crop_current_playlist(GtkAction *action, struct con_win *cwin);
void edit_tags_playing_action(GtkAction *action, struct con_win *cwin);
void track_properties(struct musicobject *mobj, struct con_win *cwin);
void clear_current_playlist(GtkAction *action, struct con_win *cwin);
void update_track_current_playlist(GtkTreeIter *iter, gint changed, struct musicobject *mobj, struct con_win *cwin);
void insert_current_playlist(struct musicobject *mobj, gboolean drop_after, GtkTreeIter *pos, struct con_win *cwin);
void append_current_playlist(struct musicobject *mobj, struct con_win *cwin);
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
gboolean current_playlist_search_compare(GtkTreeModel *model,
					 gint column,
					 const gchar *key,
					 GtkTreeIter *iter,
					 gpointer data);
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

/* Threads */

GThread* start_playback(struct musicobject *mobj, struct con_win *cwin);
void pause_playback(struct con_win *cwin);
void resume_playback(struct con_win *cwin);
void stop_playback(struct con_win *cwin);
void seek_playback(struct con_win *cwin, gint seek_len, gdouble seek_fraction);
gint process_thread_command(struct con_win *cwin);
gint reset_thread_command(struct con_win *cwin, gint cmd);
void playback_end_cleanup(struct con_win *cwin, struct lastfm_track *ltrack,
			  gint ret, gboolean lastfm_f);
void lastfm_init_thread(struct con_win *cwin);

/* Audio functions */

void set_alsa_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_oss_mixer(struct con_win *cwin, gchar *mixer_elem);
void set_soft_mixer(struct con_win *cwin);
void soft_volume_apply(gchar *buffer, gint buflen, struct con_win *cwin);
GSList* alsa_pcm_devices(struct con_win *cwin);
gint open_audio_device(gint samplerate, gint channels,
		       gboolean resume, struct con_win *cwin);

/* Systray functions */

void show_osd(struct con_win *cwin);
gboolean status_icon_clicked (GtkWidget *widget, GdkEventButton *event, struct con_win *cwin);
gboolean status_get_tooltip_cb (GtkWidget *widget, gint x, gint y, gboolean keyboard_mode,GtkTooltip *tooltip, struct con_win *cwin);
void create_status_icon (struct con_win *cwin);
void systray_display_popup_menu (struct con_win *cwin);
void systray_play(GtkAction *action, struct con_win *cwin);
void systray_stop(GtkAction *action, struct con_win *cwin);
void systray_pause(GtkAction *action, struct con_win *cwin);
void systray_prev(GtkAction *action, struct con_win *cwin);
void systray_next(GtkAction *action, struct con_win *cwin);
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
gboolean cmd_current_state(const gchar *opt_name, const gchar *val,
			   struct con_win *cwin, GError **error);
gboolean cmd_add_file(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error);

/* D-BUS functions */

DBusHandlerResult dbus_filter_handler(DBusConnection *conn,
				      DBusMessage *msg,
				      gpointer data);
void dbus_send_signal(const gchar *signal, struct con_win *cwin);

/* Utilities */
gboolean is_playable_file(const gchar *file);
gboolean is_dir_and_accessible(gchar *dir, struct con_win *cwin);
gint dir_file_count(gchar *dir_name, gint call_recur);
gchar* sanitize_string_sqlite3(gchar *str);
enum file_type get_file_type(gchar *file);
gboolean is_image_file(gchar *file);
gchar* convert_length_str(gint length);
gboolean is_present_str_list(const gchar *str, GSList *list);
GSList* delete_from_str_list(const gchar *str, GSList *list);
gchar* get_display_filename(const gchar *filename, gboolean get_folder);
void free_str_list(GSList *list);
gint compare_utf8_str(gchar *str1, gchar *str2);
gboolean validate_album_art_pattern(const gchar *pattern);
gboolean is_m3u_playlist(gchar *file);
void open_url( struct con_win *cwin, const gchar *url);
void menu_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data);
gboolean is_incompatible_upgrade(struct con_win *cwin);

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

/* Last.fm */

gint lastfm_handshake(struct con_win *cwin);
gint lastfm_submit(struct con_win *cwin, gchar *title, gchar *artist,
		   gchar *album, gint track_no, gint track_len,
		   GTimeVal start_time, enum track_source source);

/* Init */

gint init_dbus(struct con_win *cwin);
gint init_dbus_handlers(struct con_win *cwin);
gint init_options(struct con_win *cwin, int argc, char **argv);
gint init_config(struct con_win *cwin);
gint init_musicdbase(struct con_win *cwin);
gint init_audio(struct con_win *cwin);
gint init_threads(struct con_win *cwin);
gint init_notify(struct con_win *cwin);
gint init_keybinder(struct con_win *cwin);
gint init_lastfm(struct con_win *cwin);
void init_state(struct con_win *cwin);
void init_tag_completion(struct con_win *cwin);
void init_gui(gint argc, gchar **argv, struct con_win *cwin);

/* Others */

void common_cleanup(struct con_win *cwin);
void exit_pragha(GtkWidget *widget, struct con_win *cwin);

void toogle_main_window(struct con_win *cwin);
void systray_volume_scroll (GtkWidget *widget, GdkEventScroll *event, struct con_win *cwin);
GtkUIManager* create_systray_menu(struct con_win *cwin);

#endif /* PRAGHA_H */
