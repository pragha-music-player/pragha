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

#include "pragha-tagger.h"
#include "pragha-musicobject.h"
#include "pragha-database.h"
#include "pragha-tags-mgmt.h"

G_DEFINE_TYPE(PraghaTagger, pragha_tagger, G_TYPE_OBJECT)

struct _PraghaTaggerPrivate
{
	PraghaMusicobject *mobj;
	gint               changed;

	GArray            *loc_arr;
	GPtrArray         *file_arr;

	PraghaDatabase    *cdbase;
};

void
pragha_tagger_set_changes(PraghaTagger *tagger, PraghaMusicobject *mobj, gint changed)
{
	PraghaTaggerPrivate *priv = tagger->priv;

	priv->mobj = pragha_musicobject_dup(mobj);
	priv->changed = changed;
}

void
pragha_tagger_add_file(PraghaTagger *tagger, const gchar *file)
{
	gint location_id = 0;
	PraghaTaggerPrivate *priv = tagger->priv;

	location_id = pragha_database_find_location(priv->cdbase, file);
	if (G_LIKELY(location_id))
		g_array_append_val(priv->loc_arr, location_id);

	g_ptr_array_add(priv->file_arr, g_strdup(file));
}

void
pragha_tagger_apply_changes(PraghaTagger *tagger)
{
	PraghaTaggerPrivate *priv = tagger->priv;

	pragha_database_update_local_files_change_tag(priv->cdbase, priv->loc_arr, priv->changed, priv->mobj);
	/* FIXME: Port it to preferences.
	 * if(pragha_library_need_update(cwin->clibrary, changed))
		pragha_database_change_tracks_done(priv->cdbase);
	}*/
	pragha_update_local_files_change_tag(priv->file_arr, priv->changed, priv->mobj);
}

static void
pragha_tagger_finalize (GObject *object)
{
	PraghaTagger *tagger = PRAGHA_TAGGER(object);
	PraghaTaggerPrivate *priv = tagger->priv;

	g_object_unref(priv->mobj);
	g_array_free(priv->loc_arr, TRUE);
	g_ptr_array_free(priv->file_arr, TRUE);

	g_object_unref(priv->cdbase);

	G_OBJECT_CLASS(pragha_tagger_parent_class)->finalize(object);
}

static void
pragha_tagger_class_init (PraghaTaggerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_tagger_finalize;

	g_type_class_add_private(object_class, sizeof(PraghaTaggerPrivate));
}

static void
pragha_tagger_init (PraghaTagger *tagger)
{
	tagger->priv = G_TYPE_INSTANCE_GET_PRIVATE(tagger,
	                                           PRAGHA_TYPE_TAGGER,
	                                           PraghaTaggerPrivate);

	PraghaTaggerPrivate *priv = tagger->priv;

	priv->mobj = NULL;
	priv->changed = 0;

	priv->loc_arr = g_array_new(TRUE, TRUE, sizeof(gint));
	priv->file_arr = g_ptr_array_new_with_free_func(g_free);

	priv->cdbase = pragha_database_get();
}

/**
 * pragha_tagger_new:
 *
 * Return value: a new #PraghaTagger instance.
 **/
PraghaTagger*
pragha_tagger_new (void)
{
	PraghaTagger *tagger = NULL;

	tagger = g_object_new(PRAGHA_TYPE_TAGGER, NULL);

	return tagger;
}
