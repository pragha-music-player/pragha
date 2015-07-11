/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_PREFERENCES_H
#define PRAGHA_PREFERENCES_H

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_PREFERENCES (pragha_preferences_get_type())
#define PRAGHA_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferences))
#define PRAGHA_PREFERENCES_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferences const))
#define PRAGHA_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PREFERENCES, PraghaPreferencesClass))
#define PRAGHA_IS_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PREFERENCES))
#define PRAGHA_IS_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PREFERENCES))
#define PRAGHA_PREFERENCES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferencesClass))

typedef struct _PraghaPreferences PraghaPreferences;
typedef struct _PraghaPreferencesClass PraghaPreferencesClass;
typedef struct _PraghaPreferencesPrivate PraghaPreferencesPrivate;

struct _PraghaPreferences
{
	GObject parent;

	/*< private >*/
	PraghaPreferencesPrivate *priv;
};

struct _PraghaPreferencesClass
{
	GObjectClass parent_class;
	void (*plugins_change) (PraghaPreferences *preferences, const gchar *key);
  void (*library_change) (PraghaPreferences *preferences);
  void (*need_restart) (PraghaPreferences *preferences);
};

/* Defines to key preferences. */

#define GROUP_GENERAL  "General"
#define KEY_INSTALLED_VERSION      "installed_version"
#define KEY_LAST_FOLDER            "last_folder"
#define KEY_ADD_RECURSIVELY_FILES  "add_recursively_files"
#define KEY_ALBUM_ART_PATTERN      "album_art_pattern"
#define KEY_TIMER_REMAINING_MODE   "timer_remaining_mode"
#define KEY_SHOW_ICON_TRAY         "show_icon_tray"
#define KEY_CLOSE_TO_TRAY          "close_to_tray"
#define KEY_INSTANT_SEARCH         "instant_filter"
#define KEY_APPROXIMATE_SEARCH     "aproximate_search"
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
#define KEY_LIBRARY_SCANNED        "library_scanned"
#define KEY_LIBRARY_VIEW_ORDER     "library_view_order"
#define KEY_LIBRARY_LAST_SCANNED   "library_last_scanned"
#define KEY_FUSE_FOLDERS           "library_fuse_folders"
#define KEY_SORT_BY_YEAR           "library_sort_by_year"

#define GROUP_AUDIO    "Audio"
#define KEY_AUDIO_SINK             "audio_sink"
#define KEY_AUDIO_DEVICE           "audio_device"
#define KEY_SOFTWARE_MIXER         "software_mixer"
#define KEY_SOFTWARE_VOLUME        "software_volume"
#define KEY_EQ_10_BANDS            "equealizer_10_bands"
#define KEY_EQ_PRESET              "equalizer_preset"

#define GROUP_WINDOW   "Window"
#define KEY_REMEMBER_STATE          "remember_window_state"
#define KEY_START_MODE              "start_mode"
#define KEY_WINDOW_SIZE            "window_size"
#define KEY_WINDOW_POSITION        "window_position"
#define KEY_SIDEBAR                "sidebar"
#define KEY_SIDEBAR_SIZE           "sidebar_size"
#define KEY_SECONDARY_SIDEBAR      "secondary_sidebar"
#define KEY_SECONDARY_SIDEBAR_SIZE "secondary_sidebar_size"
#define KEY_SHOW_ALBUM_ART         "show_album_art"
#define KEY_ALBUM_ART_SIZE         "album_art_size"
#define KEY_TOOLBAR_SIZE           "toolbar_icon_size"
#define KEY_STATUS_BAR             "status_bar"
#define KEY_GNOME_STYLE            "gnome_style"
#define KEY_CONTROLS_BELOW         "controls_below"
#define KEY_SHOW_MENUBAR           "show_menubar"

/* Some default preferences. */

#define DEFAULT_SIDEBAR_SIZE       200
#define DEFAULT_ALBUM_ART_SIZE     32

#define DEFAULT_SINK               "default"
#define ALSA_SINK                  "alsa"
#define OSS4_SINK                  "oss4"
#define OSS_SINK                   "oss"
#define PULSE_SINK                 "pulse"

#define ALSA_DEFAULT_DEVICE        "default"
#define OSS_DEFAULT_DEVICE         "/dev/dsp"

#define ALBUM_ART_NO_PATTERNS      6

/* Some useful macros. */

#define NORMAL_STATE               "normal"
#define FULLSCREEN_STATE           "fullscreen"
#define ICONIFIED_STATE            "iconified"


PraghaPreferences* pragha_preferences_get (void);
GType pragha_preferences_get_type (void) G_GNUC_CONST;

/*
 * Generic api to accessing other preferences.
 */

gboolean
pragha_preferences_get_boolean (PraghaPreferences *preferences,
                                const gchar       *group_name,
                                const gchar       *key);
void
pragha_preferences_set_boolean (PraghaPreferences *preferences,
                                const gchar       *group_name,
                                const gchar       *key,
                                gboolean           sbool);

gint *
pragha_preferences_get_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gsize *length);
void
pragha_preferences_set_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gint list[],
                                     gsize length);

gdouble *
pragha_preferences_get_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key);
void
pragha_preferences_set_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gdouble list[],
                                    gsize length);

gchar *
pragha_preferences_get_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key);

void
pragha_preferences_set_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key,
                               const gchar *string);

gchar **
pragha_preferences_get_string_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gsize *length);
void
pragha_preferences_set_string_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    const gchar * const list[],
                                    gsize length);

GSList *
pragha_preferences_get_filename_list (PraghaPreferences *preferences,
                                      const gchar *group_name,
                                      const gchar *key);
void
pragha_preferences_set_filename_list (PraghaPreferences *preferences,
                                      const gchar *group_name,
                                      const gchar *key,
                                      GSList *list);

/**
 * pragha_preferences_remove_key:
 *
 */
void
pragha_preferences_remove_key (PraghaPreferences *preferences,
                               const gchar       *group_name,
                               const gchar       *key);

gboolean
pragha_preferences_has_group (PraghaPreferences *preferences,
                              const gchar       *group_name);

void
pragha_preferences_remove_group (PraghaPreferences *preferences,
                                 const gchar       *group_name);

/*
 * Specific plugin api.
 */

void
pragha_preferences_plugin_changed (PraghaPreferences *preferences,
                                   const gchar       *key);

gchar *
pragha_preferences_get_plugin_group_name (PraghaPreferences *preferences,
                                          const gchar       *plugin_name);

/*
 * Public api.
 */

void
pragha_preferences_need_restart (PraghaPreferences *preferences);

GSList *
pragha_preferences_get_library_list (PraghaPreferences *preferences);
void
pragha_preferences_set_library_list (PraghaPreferences *preferences,
                                     GSList *list);

const gchar *
pragha_preferences_get_installed_version (PraghaPreferences *preferences);

void
pragha_preferences_set_approximate_search (PraghaPreferences *prefernces,
                                           gboolean approximate_search);
gboolean
pragha_preferences_get_approximate_search (PraghaPreferences *preferences);

void
pragha_preferences_set_instant_search (PraghaPreferences *preferences,
                                       gboolean instant_search);
gboolean
pragha_preferences_get_instant_search (PraghaPreferences *preferences);

void
pragha_preferences_set_library_style (PraghaPreferences *preferences,
                                      gint library_style);
gint
pragha_preferences_get_library_style (PraghaPreferences *preferences);

void
pragha_preferences_set_sort_by_year (PraghaPreferences *preferences,
                                     gboolean sort_by_year);
gboolean
pragha_preferences_get_sort_by_year (PraghaPreferences *preferences);

void
pragha_preferences_set_fuse_folders (PraghaPreferences *preferences,
                                     gboolean fuse_folders);
gboolean
pragha_preferences_get_fuse_folders (PraghaPreferences *preferences);

void
pragha_preferences_set_shuffle (PraghaPreferences *preferences,
                                gboolean shuffle);
gboolean
pragha_preferences_get_shuffle (PraghaPreferences *preferences);

void
pragha_preferences_set_repeat (PraghaPreferences *preferences,
                               gboolean repeat);
gboolean
pragha_preferences_get_repeat (PraghaPreferences *preferences);

void
pragha_preferences_set_use_hint (PraghaPreferences *preferences,
                                 gboolean use_hint);
gboolean
pragha_preferences_get_use_hint (PraghaPreferences *preferences);

void
pragha_preferences_set_restore_playlist (PraghaPreferences *preferences,
                                         gboolean restore_playlist);
gboolean
pragha_preferences_get_restore_playlist (PraghaPreferences *preferences);

const gchar *
pragha_preferences_get_audio_sink (PraghaPreferences *preferences);

void
pragha_preferences_set_audio_sink (PraghaPreferences *preferences,
                                   const gchar *audio_sink);

const gchar *
pragha_preferences_get_audio_device (PraghaPreferences *preferences);

void
pragha_preferences_set_audio_device (PraghaPreferences *preferences,
                                     const gchar *audio_device);

gboolean
pragha_preferences_get_software_mixer (PraghaPreferences *preferences);

void
pragha_preferences_set_software_mixer (PraghaPreferences *preferences,
                                       gboolean software_mixer);

gdouble
pragha_preferences_get_software_volume (PraghaPreferences *preferences);

void
pragha_preferences_set_software_volume (PraghaPreferences *preferences,
                                        gdouble software_volume);

gboolean
pragha_preferences_get_lateral_panel (PraghaPreferences *preferences);

void
pragha_preferences_set_lateral_panel (PraghaPreferences *preferences,
                                      gboolean lateral_panel);

gboolean
pragha_preferences_get_secondary_lateral_panel (PraghaPreferences *preferences);

void
pragha_preferences_set_secondary_lateral_panel (PraghaPreferences *preferences,
                                                gboolean secondary_lateral_panel);

gboolean
pragha_preferences_get_show_album_art (PraghaPreferences *preferences);

void
pragha_preferences_set_show_album_art (PraghaPreferences *preferences,
                                       gboolean show_album_art);

gint
pragha_preferences_get_album_art_size (PraghaPreferences *preferences);

void
pragha_preferences_set_album_art_size (PraghaPreferences *preferences,
                                       gint album_art_size);

const gchar *
pragha_preferences_get_album_art_pattern (PraghaPreferences *preferences);

void
pragha_preferences_set_album_art_pattern (PraghaPreferences *preferences,
                                          const gchar *album_art_pattern);

GtkIconSize
pragha_preferences_get_toolbar_size (PraghaPreferences *preferences);

void
pragha_preferences_set_toolbar_size (PraghaPreferences *preferences,
                                     GtkIconSize        toolbar_size);

gboolean
pragha_preferences_get_show_status_bar (PraghaPreferences *preferences);

void
pragha_preferences_set_show_status_bar (PraghaPreferences *preferences,
                                       gboolean show_status_bar);

gboolean
pragha_preferences_get_show_status_icon (PraghaPreferences *preferences);

void
pragha_preferences_set_show_status_icon (PraghaPreferences *preferences,
                                         gboolean show_status_icon);

gboolean
pragha_preferences_get_show_menubar (PraghaPreferences *preferences);

void
pragha_preferences_set_show_menubar (PraghaPreferences *preferences,
                                     gboolean           show_menubar);

gboolean
pragha_preferences_get_gnome_style (PraghaPreferences *preferences);

void
pragha_preferences_set_gnome_style (PraghaPreferences *preferences,
                                    gboolean           gnome_style);

gboolean
pragha_preferences_get_controls_below (PraghaPreferences *preferences);

void
pragha_preferences_set_controls_below (PraghaPreferences *preferences,
                                       gboolean controls_below);

gboolean
pragha_preferences_get_remember_state (PraghaPreferences *preferences);

void
pragha_preferences_set_remember_state (PraghaPreferences *preferences,
                                       gboolean remember_state);

gint
pragha_preferences_get_sidebar_size (PraghaPreferences *preferences);

void
pragha_preferences_set_sidebar_size (PraghaPreferences *preferences,
                                     gint sidebar_size);

gint
pragha_preferences_get_secondary_sidebar_size (PraghaPreferences *preferences);

void
pragha_preferences_set_secondary_sidebar_size (PraghaPreferences *preferences,
                                               gint secondary_sidebar_size);

const gchar *
pragha_preferences_get_start_mode (PraghaPreferences *preferences);

void
pragha_preferences_set_start_mode (PraghaPreferences *preferences,
                                   const gchar *start_mode);

const gchar *
pragha_preferences_get_last_folder (PraghaPreferences *preferences);

void
pragha_preferences_set_last_folder (PraghaPreferences *preferences,
                                    const gchar *last_folder);

gboolean
pragha_preferences_get_add_recursively (PraghaPreferences *preferences);

void
pragha_preferences_set_add_recursively(PraghaPreferences *preferences,
                                       gboolean add_recursively);

gboolean
pragha_preferences_get_timer_remaining_mode (PraghaPreferences *preferences);

void
pragha_preferences_set_timer_remaining_mode(PraghaPreferences *preferences,
                                            gboolean add_recursively);

gboolean
pragha_preferences_get_hide_instead_close (PraghaPreferences *preferences);

void
pragha_preferences_set_hide_instead_close (PraghaPreferences *preferences,
                                           gboolean hide_instead_close);

gboolean
pragha_preferences_get_lock_library (PraghaPreferences *preferences);

void
pragha_preferences_set_lock_library (PraghaPreferences *preferences,
                                     gboolean           lock_library);

G_END_DECLS

#endif /* PRAGHA_PREFERENCES_H */
