/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>                        */
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

#ifndef PRAGHA_TAGGER_H
#define PRAGHA_TAGGER_H

#include <glib-object.h>
#include "pragha-musicobject.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_TAGGER (pragha_tagger_get_type())
#define PRAGHA_TAGGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TAGGER, PraghaTagger))
#define PRAGHA_TAGGER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TAGGER, PraghaTagger const))
#define PRAGHA_TAGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TAGGER, PraghaTaggerClass))
#define PRAGHA_IS_TAGGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TAGGER))
#define PRAGHA_IS_TAGGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TAGGER))
#define PRAGHA_TAGGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TAGGER, PraghaTaggerClass))

typedef struct _PraghaTagger PraghaTagger;
typedef struct _PraghaTaggerClass PraghaTaggerClass;
typedef struct _PraghaTaggerPrivate PraghaTaggerPrivate;

struct _PraghaTagger
{
	GObject parent;

	/*< private >*/
	PraghaTaggerPrivate *priv;
};

struct _PraghaTaggerClass
{
	GObjectClass parent_class;
};

void pragha_tagger_set_changes     (PraghaTagger *tagger, PraghaMusicobject *mobj, gint changed);
void pragha_tagger_add_file        (PraghaTagger *tagger, const gchar *file);
void pragha_tagger_add_location_id (PraghaTagger *tagger, gint location_id);
void pragha_tagger_apply_changes   (PraghaTagger *tagger);

PraghaTagger *pragha_tagger_new (void);

GType pragha_tagger_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* PRAGHA_TAGGER_H */
