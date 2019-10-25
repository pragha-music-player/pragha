/*************************************************************************/
/* Copyright (C) 2018-2019 matias <mati86dl@gmail.com>                   */
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

#include "pragha-utils.h"
#include "pragha-database.h"
#include "pragha-database-provider.h"
#include "pragha-musicobject-mgmt.h"

#include "pragha-temp-provider.h"


struct _PraghaTempProvider {
	GObject        _parent;

	PraghaDatabase *database;
	PraghaDatabaseProvider *db_provider;

	gchar          *name;

	GHashTable     *db_table;
	GHashTable     *nw_table;

	GHashTable     *rm_table;
	GHashTable     *ins_table;
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
pragha_temp_provider_insert_track (PraghaTempProvider *provider,
                                   PraghaMusicobject  *mobj)
{
	const gchar *file = pragha_musicobject_get_file(mobj);
	g_hash_table_insert (provider->nw_table, g_strdup(file), g_object_ref(mobj));
}

void
pragha_temp_provider_merge_database (PraghaTempProvider *provider)
{
	GHashTableIter iter;
	gpointer key, value;

	// First look for the songs that don't exist anymore.
	g_hash_table_iter_init (&iter, provider->db_table);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		const gchar *filename = (gchar *) key;
		PraghaMusicobject *mobj = PRAGHA_MUSICOBJECT (value);
		if (!g_hash_table_contains (provider->nw_table, filename)) {
			g_hash_table_insert (provider->rm_table, g_strdup(filename), g_object_ref(mobj));
		}
	}

	// Look for upgraded or new songs.
	g_hash_table_iter_init (&iter, provider->nw_table);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		const gchar *filename = (gchar *) key;
		PraghaMusicobject *mobj = PRAGHA_MUSICOBJECT (value);

		// If exist on old db but something changed upgrade them.
		PraghaMusicobject *db_mobj = g_hash_table_lookup (provider->db_table, filename);
		if (db_mobj && pragha_musicobject_compare_tags(db_mobj, mobj)) {
			g_hash_table_insert (provider->rm_table, g_strdup(filename), g_object_ref(mobj));
			g_hash_table_insert (provider->ins_table, g_strdup(filename), g_object_ref(mobj));
		} else {
			g_hash_table_insert (provider->ins_table, g_strdup(filename), g_object_ref(mobj));
		}
	}
}

void
pragha_temp_provider_commit_database (PraghaTempProvider *provider)
{
	/* Remove old. */
	g_hash_table_foreach (provider->rm_table,
	                      pragha_temp_provider_forget_track_db,
	                      provider->database);

	/* Add song with changes. */
	g_hash_table_foreach (provider->ins_table,
	                      pragha_temp_provider_add_track_db,
	                      provider->database);

	/* Songs without changes remain there.. */
}

void
pragha_temp_provider_set_visible (PraghaTempProvider *provider,
                                  gboolean            visible)
{
	pragha_provider_set_visible (provider->db_provider, provider->name, visible);
	pragha_provider_update_done (provider->db_provider);
}


/*
 * Private api.
 */

static void
pragha_temp_provider_fill_database (PraghaTempProvider *provider)
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

	if (provider->db_provider) {
		g_object_unref (provider->db_provider);
		provider->db_provider = NULL;
	}

	if (provider->db_table) {
		g_hash_table_destroy (provider->db_table);
		provider->db_table = NULL;
	}

	if (provider->nw_table) {
		g_hash_table_destroy (provider->nw_table);
		provider->nw_table = NULL;
	}

	if (provider->rm_table) {
		g_hash_table_destroy (provider->rm_table);
		provider->rm_table = NULL;
	}

	if (provider->ins_table) {
		g_hash_table_destroy (provider->ins_table);
		provider->ins_table = NULL;
	}

	G_OBJECT_CLASS(pragha_temp_provider_parent_class)->dispose(object);
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
	provider->db_provider = pragha_database_provider_get ();

	provider->db_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	provider->nw_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	provider->rm_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	provider->ins_table = g_hash_table_new_full (g_str_hash,
	                                             g_str_equal,
	                                             g_free,
	                                             g_object_unref);

}

PraghaTempProvider *
pragha_temp_provider_new (const gchar         *name,
                          const gchar         *type,
                          const gchar         *friendly_name,
                          const gchar         *icon_name)
{
	PraghaTempProvider *provider = NULL;

	provider = g_object_new (PRAGHA_TYPE_TEMP_PROVIDER, NULL);

	provider->name = g_strdup (name);

	// Ensure provider exist and fill database
	if (!pragha_provider_exist(provider->db_provider, name)) {
		pragha_provider_add_new (provider->db_provider,
		                         name,
		                         type,
		                         friendly_name,
		                         icon_name);
	}
	pragha_temp_provider_fill_database (provider);

	return provider;
}
