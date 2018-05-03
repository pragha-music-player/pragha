/*************************************************************************/
/* Copyright (C) 2011-2018 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_SONGINFO_PANE_H
#define PRAGHA_SONGINFO_PANE_H

#include <gtk/gtk.h>
#include <glib-object.h>

#include "src/pragha-musicobject.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_SONGINFO_PANE            (pragha_songinfo_pane_get_type ())
#define PRAGHA_SONGINFO_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SONGINFO_PANE, PraghaSonginfoPane))
#define PRAGHA_IS_SONGINFO_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SONGINFO_PANE))
#define PRAGHA_SONGINFO_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_SONGINFO_PANE, PraghaSonginfoPaneClass))
#define PRAGHA_IS_SONGINFO_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_SONGINFO_PANE))
#define PRAGHA_SONGINFO_PANE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_SONGINFO_PANE, PraghaSonginfoPaneClass))

typedef struct _PraghaSonginfoPane PraghaSonginfoPane;

typedef struct {
	GtkScrolledWindowClass __parent__;
	void (*type_changed)   (PraghaSonginfoPane *pane);
	void (*append)         (PraghaSonginfoPane *pane, PraghaMusicobject *mobj);
	void (*append_all)     (PraghaSonginfoPane *pane);
} PraghaSonginfoPaneClass;

GtkWidget *         pragha_songinfo_pane_row_new               (PraghaMusicobject *mobj);

void                pragha_songinfo_pane_set_title             (PraghaSonginfoPane *pane,
                                                                const gchar        *title);

void                pragha_songinfo_pane_set_text              (PraghaSonginfoPane *pane,
                                                                const gchar        *text,
                                                                const gchar        *provider);

void                pragha_songinfo_pane_append_song_row       (PraghaSonginfoPane *pane,
                                                                GtkWidget          *row);

void                pragha_songinfo_pane_clear_text            (PraghaSonginfoPane *pane);
void                pragha_songinfo_pane_clear_list            (PraghaSonginfoPane *pane);

GList              *pragha_songinfo_get_mobj_list              (PraghaSonginfoPane *pane);

GtkWidget          *pragha_songinfo_pane_get_pane_title        (PraghaSonginfoPane *pane);
GtkMenu            *pragha_songinfo_pane_get_popup_menu        (PraghaSonginfoPane *pane);
GtkUIManager       *pragha_songinfo_pane_get_pane_context_menu (PraghaSonginfoPane *pane);
GLYR_GET_TYPE       pragha_songinfo_pane_get_default_view      (PraghaSonginfoPane *pane);

PraghaSonginfoPane *pragha_songinfo_pane_new                   (void);

G_END_DECLS

#endif /* PRAGHA_SONGINFO_H */
