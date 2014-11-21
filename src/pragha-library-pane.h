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

#define PRAGHA_TYPE_LIBRARY_PANE                  (pragha_library_pane_get_type ())
#define PRAGHA_LIBRARY_PANE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPane))
#define PRAGHA_IS_LIBRARY_PANE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_LIBRARY_PANE))
#define PRAGHA_LIBRARY_PANE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPaneClass))
#define PRAGHA_IS_LIBRARY_PANE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_LIBRARY_PANE))
#define PRAGHA_LIBRARY_PANE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_LIBRARY_PANE, PraghaLibraryPaneClass))

typedef struct _PraghaLibraryPane PraghaLibraryPane;

typedef struct {
	GtkBoxClass __parent__;
	void (*library_append_playlist) (PraghaLibraryPane *toolbar);
	void (*library_replace_playlist) (PraghaLibraryPane *toolbar);
	void (*library_replace_playlist_and_play) (PraghaLibraryPane *toolbar);
} PraghaLibraryPaneClass;

/* Library Views */

typedef enum {
	FOLDERS,
	ARTIST,
	ALBUM,
	GENRE,
	ARTIST_ALBUM,
	GENRE_ARTIST,
	GENRE_ALBUM,
	GENRE_ARTIST_ALBUM,
	LAST_LIBRARY_STYLE
} PraghaLibraryStyle;

/* Functions */

GList * pragha_library_pane_get_mobj_list (PraghaLibraryPane *library);

gboolean simple_library_search_activate_handler   (GtkEntry *entry, PraghaLibraryPane *clibrary);
void     clear_library_search                     (PraghaLibraryPane *clibrary);

gboolean pragha_library_need_update_view (PraghaPreferences *preferences, gint changed);
gboolean pragha_library_need_update    (PraghaLibraryPane *clibrary, gint changed);
void     library_pane_view_reload      (PraghaLibraryPane *clibrary);
void     pragha_library_pane_init_view (PraghaLibraryPane *clibrary);

GtkWidget         *pragha_library_pane_get_widget     (PraghaLibraryPane *librarypane);
GtkWidget         *pragha_library_pane_get_pane_title (PraghaLibraryPane *library);
GtkMenu           *pragha_library_pane_get_popup_menu (PraghaLibraryPane *library);

GtkUIManager      *pragha_library_pane_get_pane_context_menu(PraghaLibraryPane *clibrary);

PraghaLibraryPane *pragha_library_pane_new        (void);

#endif /* PRAGHA_LIBRARY_PANE_H */
