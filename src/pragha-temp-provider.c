/*************************************************************************/
/* Copyright (C) 2018 matias <mati86dl@gmail.com>                        */
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

#include "pragha-temp-provider.h"

#include "pragha-utils.h"
#include "pragha-database.h"
#include "pragha-musicobject-mgmt.h"

struct _PraghaTempProvider {
	GObject        _parent;

	PraghaDatabase *database;

	gchar          *name;

	GHashTable     *db_table;
	GHashTable     *nw_table;
	GHashTable     *up_table;
	GHashTable     *rm_table;
};

G_DEFINE_TYPE(PraghaTempProvider, pragha_temp_provider, G_TYPE_OBJECT)

/*
 * Helpers
 */
static void
pragha_temp_provider_add_track_db (gpointer key,
                                   gpointer value,
                                   gpointer user_data)
{
	PraghaMusicobject *mobj = value;
	PraghaDatabase *database = user_data;
	pragha_database_add_new_musicobject (database, mobj);
}

static void
pragha_temp_provider_forget_track_db (gpointer key,
                                      gpointer value,
                                      gpointer user_data)
{
	const gchar *file = key;
	PraghaDatabase *database = user_data;

	pragha_database_forget_track (database, file);
}

/*
 * Public api
 */
void
pragha_temp_provider_save_database (PraghaTempProvider *provider)
{
	/* Remove old. */
	g_hash_table_foreach (provider->rm_table,
	                      pragha_temp_provider_forget_track_db,
	                      provider->database);

	/* Add song with changes. */
	g_hash_table_foreach (provider->up_table,
	                      pragha_temp_provider_add_track_db,
	                      provider->database);

	/* Add new songs. */
	g_hash_table_foreach (provider->nw_table,
	                      pragha_temp_provider_add_track_db,
	                      provider->database);

	/* Songs without changes remain there.. */
}

void
pragha_temp_provider_insert_track (PraghaTempProvider *provider,
                                   PraghaMusicobject  *mobj)
{
	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	g_hash_table_insert (provider->nw_table, g_strdup(file), mobj);
}

void
pragha_temp_provider_delete_track (PraghaTempProvider *provider,
                                   PraghaMusicobject  *mobj)
{
	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	if (g_hash_table_remove (provider->db_table, file))
		g_hash_table_insert (provider->rm_table, g_strdup(file), mobj);
}

void
pragha_temp_provider_replace_track (PraghaTempProvider *provider,
                                    PraghaMusicobject  *mobj)
{
	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	if (g_hash_table_remove (provider->db_table, file)) {
		g_hash_table_insert (provider->nw_table, g_strdup(file), mobj);
	}
}

void
pragha_temp_provider_foreach_purge (PraghaTempProvider *provider,
                                    ProviderCheckFunc  *check_func,
                                    gpointer            user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, provider->db_table);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		if (!(* check_func) (key, value, user_data))
		{
			gchar *file = key;
			PraghaMusicobject *mobj = PRAGHA_MUSICOBJECT(value);
			g_hash_table_insert (provider->rm_table, g_strdup(file), g_object_ref(mobj));
			g_hash_table_iter_remove(&iter);
		}
	}
}


/*
 *
 */
static void
pragha_temp_provider_fill_cache (PraghaTempProvider *provider)
{
	PraghaDatabase *database;
	PraghaPreparedStatement *statement;
	PraghaMusicobject *mobj = NULL;
	gint location_id = 0;

	database = pragha_database_get();

	const gchar *sql = "SELECT location FROM TRACK WHERE provider = ?";
	statement = pragha_database_create_statement (database, sql);

	pragha_prepared_statement_bind_int (statement, 1,
		pragha_database_find_provider (database, provider->name));

	while (pragha_prepared_statement_step (statement)) {
		location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db(database, location_id);
		if (G_LIKELY(mobj)) {
			g_hash_table_insert(provider->db_table,
			                    g_strdup(pragha_musicobject_get_file(mobj)),
			                    mobj);
		}
		pragha_prepared_statement_free (statement);
	}
	g_object_unref(database);
}

static void
pragha_temp_provider_dispose (GObject *object)
{
	PraghaTempProvider *provider = PRAGHA_TEMP_PROVIDER(object);

	if (provider->database) {
		g_object_unref (provider->database);
		provider->database = NULL;
	}

	if (provider->db_table) {
		g_hash_table_destroy (provider->db_table);
		provider->db_table = NULL;
	}

	if (provider->rm_table) {
		g_hash_table_destroy (provider->rm_table);
		provider->rm_table = NULL;
	}

	if (provider->nw_table) {
		g_hash_table_destroy (provider->nw_table);
		provider->nw_table = NULL;
	}

	G_OBJECT_CLASS(pragha_temp_provider_parent_class)->finalize(object);
}

static void
pragha_temp_provider_finalize (GObject *object)
{
	PraghaTempProvider *provider = PRAGHA_TEMP_PROVIDER(object);

	g_free (provider->name);

	G_OBJECT_CLASS(pragha_temp_provider_parent_class)->finalize(object);
}

static void
pragha_temp_provider_class_init (PraghaTempProviderClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = pragha_temp_provider_dispose;
	object_class->finalize = pragha_temp_provider_finalize;
}

static void
pragha_temp_provider_init (PraghaTempProvider *provider)
{
	provider->database = pragha_database_get();

	provider->db_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	provider->rm_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	provider->nw_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);
}

PraghaTempProvider *
pragha_temp_provider_new (const gchar *name)
{
	PraghaTempProvider *provider = NULL;

	provider = g_object_new (PRAGHA_TYPE_TEMP_PROVIDER, NULL);

	provider->name = g_strdup (name);

	pragha_temp_provider_fill_cache (provider);

	return provider;
}
