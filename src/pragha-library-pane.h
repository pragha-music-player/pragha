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
#include "pragha-preferences.h"

/* pragha.h */
struct con_win;

#define PRAGHA_TYPE_LIBRARY_PANE                  (pragha_library_pane_get_type ())
#define PRAGHA_LIBRARY_PANE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPane))
#define PRAGHA_IS_LIBRARY_PANE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_LIBRARY_PANE))
#define PRAGHA_LIBRARY_PANE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPaneClass))
#define PRAGHA_IS_LIBRARY_PANE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_LIBRARY_PANE))
#define PRAGHA_LIBRARY_PANE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPaneClass))

typedef struct _PraghaLibraryPane PraghaLibraryPane;

typedef struct {
	GtkVBoxClass __parent__;
	void (*library_append_playlist) (PraghaLibraryPane *toolbar);
	void (*library_replace_playlist) (PraghaLibraryPane *toolbar);
	void (*library_replace_playlist_and_play) (PraghaLibraryPane *toolbar);
} PraghaLibraryPaneClass;

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

GList * pragha_library_pane_get_mobj_list (PraghaLibraryPane *library);

gboolean
library_page_right_click_cb(GtkWidget *widget,
                            GdkEventButton *event,
                            struct con_win *cwin);

void     simple_library_search_keyrelease         (struct con_win *cwin);
gboolean simple_library_search_keyrelease_handler (GtkEntry *entry, PraghaLibraryPane *clibrary);
gboolean simple_library_search_activate_handler   (GtkEntry *entry, PraghaLibraryPane *clibrary);
void     clear_library_search                     (PraghaLibraryPane *clibrary);

gboolean pragha_library_need_update_view (PraghaPreferences *preferences, gint changed);
gboolean pragha_library_need_update    (PraghaLibraryPane *clibrary, gint changed);
void     library_pane_view_reload      (PraghaLibraryPane *clibrary);
void     pragha_library_pane_init_view (PraghaLibraryPane *clibrary, struct con_win *cwin);

GtkWidget         *pragha_library_pane_get_widget (PraghaLibraryPane *librarypane);
GtkUIManager      *pragha_library_pane_get_pane_context_menu(PraghaLibraryPane *clibrary);

PraghaLibraryPane *pragha_library_pane_new        (void);

#endif /* PRAGHA_LIBRARY_PANE_H */