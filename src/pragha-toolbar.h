/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2018 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_TOOLBAR_H
#define PRAGHA_TOOLBAR_H

#include <gtk/gtk.h>

#include "pragha-musicobject.h"
#include "pragha-album-art.h"
#include "pragha-backend.h"

#define PRAGHA_TYPE_TOOLBAR                  (pragha_toolbar_get_type ())
#define PRAGHA_TOOLBAR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TOOLBAR, PraghaToolbar))
#define PRAGHA_IS_TOOLBAR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TOOLBAR))
#define PRAGHA_TOOLBAR_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TOOLBAR, PraghaToolbarClass))
#define PRAGHA_IS_TOOLBAR_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TOOLBAR))
#define PRAGHA_TOOLBAR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TOOLBAR, PraghaToolbarClass))

typedef struct _PraghaToolbar PraghaToolbar;

typedef struct {
	GtkHeaderBarClass __parent__;

	void (*prev) (PraghaToolbar *toolbar);
	void (*play) (PraghaToolbar *toolbar);
	void (*stop) (PraghaToolbar *toolbar);
	void (*next) (PraghaToolbar *toolbar);
	void (*album_art_activated) (PraghaToolbar *toolbar);
	void (*track_info_activated) (PraghaToolbar *toolbar);
	void (*track_progress_activated) (PraghaToolbar *toolbar, gdouble fraction);
	void (*favorite_toggle) (PraghaToolbar *toolbar);
	void (*unfull) (PraghaToolbar *toolbar);
} PraghaToolbarClass;

void pragha_toolbar_set_title               (PraghaToolbar *toolbar, PraghaMusicobject *mobj);
void pragha_toolbar_set_image_album_art     (PraghaToolbar *toolbar, const gchar *uri);
void pragha_toolbar_set_favorite_icon       (PraghaToolbar *toolbar, gboolean love);
void pragha_toolbar_update_progress         (PraghaToolbar *toolbar, gint length, gint progress);

void pragha_toolbar_update_buffering_cb      (PraghaBackend *backend, gint percent, gpointer user_data);
void pragha_toolbar_update_playback_progress (PraghaBackend *backend, gpointer user_data);
void pragha_toolbar_playback_state_cb        (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data);
void pragha_toolbar_show_ramaning_time_cb    (PraghaToolbar *toolbar, GParamSpec *pspec, gpointer user_data);

gboolean pragha_toolbar_window_state_event   (GtkWidget *widget, GdkEventWindowState *event, PraghaToolbar *toolbar);

void     pragha_toolbar_set_style            (PraghaToolbar *toolbar, gboolean gnome_style);
void     pragha_toolbar_add_extention_widget (PraghaToolbar *toolbar, GtkWidget *widget);
void     pragha_toolbar_add_extra_button     (PraghaToolbar *toolbar, GtkWidget *widget);

const gchar    *pragha_toolbar_get_progress_text (PraghaToolbar *toolbar);
const gchar    *pragha_toolbar_get_length_text   (PraghaToolbar *toolbar);

GtkWidget      *pragha_toolbar_get_song_box      (PraghaToolbar *toolbar);
PraghaAlbumArt *pragha_toolbar_get_album_art     (PraghaToolbar *toolbar);

PraghaToolbar *pragha_toolbar_new        (void);

#endif /* PRAGHA_TOOLBAR_H */
