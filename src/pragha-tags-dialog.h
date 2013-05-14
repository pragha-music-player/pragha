/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2011 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_TAGS_DIALOG_H
#define PRAGHA_TAGS_DIALOG_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include "pragha-musicobject.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_TAGS_DIALOG            (pragha_tags_dialog_get_type ())
#define PRAGHA_TAGS_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TAGS_DIALOG, PraghaTagsDialog))
#define PRAGHA_TAGS_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TAGS_DIALOG, PraghaTagsDialogClass))
#define PRAGHA_IS_TAGS_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TAGS_DIALOG))
#define PRAGHA_IS_TAGS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHAL_TYPE_TAGS_DIALOG))
#define PRAGHA_TAGS_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TAGS_DIALOG, PraghaTagsDialogClass))

typedef struct _PraghaTagsDialogClass PraghaTagsDialogClass;
typedef struct _PraghaTagsDialog      PraghaTagsDialog;

GType              pragha_tags_dialog_get_type        (void) G_GNUC_CONST;

PraghaMusicobject *pragha_tags_dialog_get_musicobject (PraghaTagsDialog *dialog);
void               pragha_tags_dialog_set_musicobject (PraghaTagsDialog *dialog, PraghaMusicobject *mobj);

void               pragha_tags_dialog_set_changed     (PraghaTagsDialog *dialog, gint changed);
gint               pragha_tags_dialog_get_changed     (PraghaTagsDialog *dialog);

GtkWidget          *pragha_tags_dialog_new            (void);

void                pragha_track_properties_dialog    (PraghaMusicobject *mobj, GtkWidget *parent);

G_END_DECLS

#endif /* PRAGHA_TAGS_DIALOG_H */