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

#ifndef PRAGHA_UTILS_H
#define PRAGHA_UTILS_H

#define string_is_empty(s) (!(s) || !(s)[0])
#define string_is_not_empty(s) (s && (s)[0])

void pragha_log_to_file (const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data);

gchar *e2_utf8_ndup (const gchar *str, glong num);
gsize levenshtein_strcmp(const gchar * s, const gchar * t);
gsize levenshtein_safe_strcmp(const gchar * s, const gchar * t);
gchar *g_strstr_lv (gchar *haystack, gchar *needle, gsize lv_distance);
gchar *pragha_strstr_lv(gchar *haystack, gchar *needle, PraghaPreferences *preferences);


#if !GLIB_CHECK_VERSION(2,32,0)
gboolean nm_is_online ();
#endif

gboolean already_in_current_playlist(PraghaMusicobject *mobj, struct con_win *cwin);
GList *prepend_song_with_artist_and_title_to_mobj_list(const gchar *artist, const gchar *title, GList *list, struct con_win *cwin);

void set_watch_cursor (GtkWidget *widget);
void remove_watch_cursor (GtkWidget *widget);

GdkPixbuf *vgdk_pixbuf_new_from_memory (const char *data, size_t size);
enum file_type get_file_type(const gchar *file);
enum playlist_type pragha_pl_parser_guess_format_from_extension (const gchar *filename);
gboolean is_image_file(const gchar *file);
gchar* convert_length_str(gint length);
gboolean is_present_str_list(const gchar *str, GSList *list);
GSList* delete_from_str_list(const gchar *str, GSList *list);
gchar * path_get_dir_as_uri (const gchar *path);
gchar* get_display_filename(const gchar *filename, gboolean get_folder);
gchar* get_display_name(PraghaMusicobject *mobj);
void free_str_list(GSList *list);
gint compare_utf8_str(const gchar *str1, const gchar *str2);
gboolean validate_album_art_pattern(const gchar *pattern);
gboolean pragha_process_gtk_events ();
void open_url(const gchar *url, GtkWidget *parent);
void menu_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data);
gboolean is_incompatible_upgrade(struct con_win *cwin);

#endif /* PRAGHA_UTILS_H */