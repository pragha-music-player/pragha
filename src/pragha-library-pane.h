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

#ifndef PRAGHA_LIBRARY_PANE_H
#define PRAGHA_LIBRARY_PANE_H

#include <gtk/gtk.h>

/* pragha.h */
struct con_win;

typedef struct _PraghaLibraryPane PraghaLibraryPane;

/* Node types in library view */

enum node_type {
	NODE_CATEGORY,
	NODE_FOLDER,
	NODE_GENRE,
	NODE_ARTIST,
	NODE_ALBUM,
	NODE_TRACK,
	NODE_BASENAME,
	NODE_PLAYLIST,
	NODE_RADIO
};

/* Columns in Library view */

enum library_columns {
	L_PIXBUF,
	L_NODE_DATA,
	L_NODE_BOLD,
	L_NODE_TYPE,
	L_LOCATION_ID,
	L_MACH,
	L_VISIBILE,
	N_L_COLUMNS
};

/* Library Views */

enum library_style {
	FOLDERS,
	ARTIST,
	ALBUM,
	GENRE,
	ARTIST_ALBUM,
	GENRE_ARTIST,
	GENRE_ALBUM,
	GENRE_ARTIST_ALBUM,
	LAST_LIBRARY_STYLE
};

typedef enum {
	PRAGHA_RESPONSE_SKIP,
	PRAGHA_RESPONSE_SKIP_ALL,
	PRAGHA_RESPONSE_DELETE_ALL
} PraghaDeleteResponseType;

#define PRAGHA_BUTTON_SKIP       _("_Skip")
#define PRAGHA_BUTTON_SKIP_ALL   _("S_kip All")
#define PRAGHA_BUTTON_DELETE_ALL _("Delete _All")

/* Functions */

void expand_all_action(GtkAction *action, struct con_win *cwin);
void collapse_all_action(GtkAction *action, struct con_win *cwin);
void folders_library_tree(GtkAction *action, struct con_win *cwin);
void artist_library_tree(GtkAction *action, struct con_win *cwin);
void album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_library_tree(GtkAction *action, struct con_win *cwin);
void artist_album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_album_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_library_tree(GtkAction *action, struct con_win *cwin);
void genre_artist_album_library_tree(GtkAction *action, struct con_win *cwin);

void library_tree_add_to_playlist_action(GtkAction *action, struct con_win *cwin);
void library_tree_replace_playlist_action(GtkAction *action, struct con_win *cwin);
void library_tree_replace_and_play(GtkAction *action, struct con_win *cwin);
void playlist_tree_rename(GtkAction *action, struct con_win *cwin);
void playlist_tree_delete(GtkAction *action, struct con_win *cwin);
void playlist_tree_export(GtkAction *action, struct con_win *cwin);

void library_tree_edit_tags(GtkAction *action, struct con_win *cwin);
void library_tree_delete_hdd(GtkAction *action, struct con_win *cwin);
void library_tree_delete_db(GtkAction *action, struct con_win *cwin);

void
library_tree_row_activated_cb(GtkTreeView *library_tree,
                              GtkTreePath *path,
                              GtkTreeViewColumn *column,
                              struct con_win *cwin);
gboolean
library_tree_button_press_cb(GtkWidget *widget,
                             GdkEventButton *event,
                             struct con_win *cwin);
gboolean
library_tree_button_release_cb(GtkWidget *widget,
                               GdkEventButton *event,
                               struct con_win *cwin);
gboolean
library_page_right_click_cb(GtkWidget *widget,
                            GdkEventButton *event,
                            struct con_win *cwin);

void     simple_library_search_keyrelease         (struct con_win *cwin);
gboolean simple_library_search_keyrelease_handler (GtkEntry *entry, PraghaLibraryPane *clibrary);
gboolean simple_library_search_activate_handler   (GtkEntry *entry, PraghaLibraryPane *clibrary);
void     clear_library_search                     (PraghaLibraryPane *clibrary);

gboolean pragha_library_need_update    (PraghaLibraryPane *clibrary, gint changed);
void     library_pane_view_reload      (PraghaLibraryPane *clibrary);
void     pragha_library_pane_init_view (PraghaLibraryPane *clibrary, struct con_win *cwin);

GtkWidget         *pragha_library_pane_get_widget (PraghaLibraryPane *librarypane);
GtkUIManager      *pragha_library_pane_get_pane_context_menu(PraghaLibraryPane *clibrary);

void               pragha_library_pane_free       (PraghaLibraryPane *librarypane);
PraghaLibraryPane *pragha_library_pane_new        (struct con_win *cwin);

#endif /* PRAGHA_LIBRARY_PANE_H */