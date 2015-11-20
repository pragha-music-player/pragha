/*************************************************************************/
/* Copyright (C) 2015 matias <mati86dl@gmail.com>                        */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha-database.h"
#include "pragha-prepared-statement-private.h"

#include "pragha-database-provider.h"

G_DEFINE_TYPE(PraghaDatabaseProvider, pragha_database_provider, G_TYPE_OBJECT)

struct _PraghaDatabaseProviderPrivate
{
	PraghaDatabase *database;
};

enum {
	SIGNAL_WANT_UPDATE,
	SIGNAL_WANT_UPGRADE,
	SIGNAL_WANT_REMOVE,
	SIGNAL_UPDATE_DONE,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

/* Provider */

void
pragha_provider_add_new (PraghaDatabaseProvider *provider,
                         const gchar            *name,
                         const gchar            *type,
                         const gchar            *friendly_name,
                         const gchar            *icon_name)
{
	gint provider_type_id = 0;
	PraghaPreparedStatement *statement;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	const gchar *sql = "INSERT INTO PROVIDER (name, type, friendly_name, icon_name) VALUES (?, ?, ?, ?)";

	if ((provider_type_id = pragha_database_find_provider_type (priv->database, type)) == 0)
		provider_type_id = pragha_database_add_new_provider_type (priv->database, type);

	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_string (statement, 1, name);
	pragha_prepared_statement_bind_int    (statement, 2, provider_type_id);
	pragha_prepared_statement_bind_string (statement, 3, friendly_name);
	pragha_prepared_statement_bind_string (statement, 4, icon_name);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);
}

void
pragha_provider_remove (PraghaDatabaseProvider *provider,
                        const gchar            *name)
{
	PraghaPreparedStatement *statement;
	gint provider_id = 0;
	const gchar *sql;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	if ((provider_id = pragha_database_find_provider (priv->database, name)) == 0)
		return;

	/* Delete all tracks of provider */

	sql = "DELETE FROM PROVIDER WHERE id = ?";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_int (statement, 1, provider_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete all tracks of provider */

	sql = "DELETE FROM TRACK WHERE provider = ?";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_int (statement, 1, provider_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete the location entries */

	sql = "DELETE FROM LOCATION WHERE id NOT IN (SELECT location FROM TRACK)";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete all entries from PLAYLIST_TRACKS which match given dir */

	sql = "DELETE FROM PLAYLIST_TRACKS WHERE file NOT IN (SELECT name FROM LOCATION)";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Now flush unused artists, albums, genres, years */

	pragha_database_flush_stale_entries (priv->database);
}

void
pragha_provider_forget_songs (PraghaDatabaseProvider *provider,
                              const gchar            *name)
{
	PraghaPreparedStatement *statement;
	gint provider_id = 0;
	const gchar *sql;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	if ((provider_id = pragha_database_find_provider (priv->database, name)) == 0)
		return;

	/* Delete all tracks of provider */

	sql = "DELETE FROM TRACK WHERE provider = ?";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_int (statement, 1, provider_id);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete the location entries */

	sql = "DELETE FROM LOCATION WHERE id NOT IN (SELECT location FROM TRACK)";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Delete all entries from PLAYLIST_TRACKS which match given dir */

	sql = "DELETE FROM PLAYLIST_TRACKS WHERE file NOT IN (SELECT name FROM LOCATION)";
	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_step (statement);
	pragha_prepared_statement_free (statement);

	/* Now flush unused artists, albums, genres, years */

	pragha_database_flush_stale_entries (priv->database);
}

GSList *
pragha_provider_get_list (PraghaDatabaseProvider *provider)
{
	PraghaPreparedStatement *statement;
	GSList *list = NULL;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	const gchar *sql = "SELECT name FROM PROVIDER";

	statement = pragha_database_create_statement (priv->database, sql);
	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		list = g_slist_append (list, g_strdup(name));
	}

	pragha_prepared_statement_free (statement);

	return list;
}

GSList *
pragha_provider_get_handled_list (PraghaDatabaseProvider *provider)
{
	PraghaPreparedStatement *statement;
	GSList *list = NULL;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	const gchar *sql = "SELECT name FROM PROVIDER WHERE id IN (SELECT provider FROM TRACK)";

	statement = pragha_database_create_statement (priv->database, sql);
	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		list = g_slist_append (list, g_strdup(name));
	}

	pragha_prepared_statement_free (statement);

	return list;
}

GSList *
pragha_provider_get_list_by_type (PraghaDatabaseProvider *provider,
                                  const gchar            *provider_type)
{
	PraghaPreparedStatement *statement;
	GSList *list = NULL;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	const gchar *sql = "SELECT name FROM PROVIDER WHERE type = ?";

	statement = pragha_database_create_statement (priv->database, sql);

	pragha_prepared_statement_bind_int (statement, 1,
		pragha_database_find_provider_type (priv->database, provider_type));

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		list = g_slist_append (list, g_strdup(name));
	}
	pragha_prepared_statement_free (statement);

	return list;
}

GSList *
pragha_provider_get_handled_list_by_type (PraghaDatabaseProvider *provider,
                                          const gchar            *provider_type)
{
	PraghaPreparedStatement *statement;
	GSList *list = NULL;

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	const gchar *sql = "SELECT name FROM PROVIDER WHERE id IN (SELECT provider FROM TRACK) AND type = ?";

	statement = pragha_database_create_statement (priv->database, sql);

	pragha_prepared_statement_bind_int (statement, 1,
		pragha_database_find_provider_type (priv->database, provider_type));

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		list = g_slist_append (list, g_strdup(name));
	}

	pragha_prepared_statement_free (statement);

	return list;
}

gchar *
pragha_provider_get_friendly_name (PraghaDatabaseProvider *provider, const gchar *name)
{
	PraghaPreparedStatement *statement;
	PraghaDatabaseProviderPrivate *priv = provider->priv;
	gchar *friendly_name = NULL;

	const gchar *sql = "SELECT friendly_name FROM PROVIDER WHERE name = ?";

	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_string (statement, 1, name);
	pragha_prepared_statement_step (statement);
	friendly_name = g_strdup(pragha_prepared_statement_get_string (statement, 0));
	pragha_prepared_statement_free (statement);

	return friendly_name;
}

gchar *
pragha_provider_get_icon_name (PraghaDatabaseProvider *provider, const gchar *name)
{
	PraghaPreparedStatement *statement;
	PraghaDatabaseProviderPrivate *priv = provider->priv;
	gchar *icon_name = NULL;

	const gchar *sql = "SELECT icon_name FROM PROVIDER WHERE name = ?";

	statement = pragha_database_create_statement (priv->database, sql);
	pragha_prepared_statement_bind_string (statement, 1, name);
	pragha_prepared_statement_step (statement);
	icon_name = g_strdup(pragha_prepared_statement_get_string (statement, 0));
	pragha_prepared_statement_free (statement);

	return icon_name;
}

/*
 * Signals.
 */

void
pragha_provider_want_upgrade (PraghaDatabaseProvider *provider, gint provider_id)
{
	g_return_if_fail(PRAGHA_IS_DATABASE_PROVIDER(provider));

	g_signal_emit (provider, signals[SIGNAL_WANT_UPDATE], 0, provider_id);
}

void
pragha_provider_want_update (PraghaDatabaseProvider *provider, gint provider_id)
{
	g_return_if_fail(PRAGHA_IS_DATABASE_PROVIDER(provider));

	g_signal_emit (provider, signals[SIGNAL_WANT_UPDATE], 0, provider_id);
}

void
pragha_provider_want_remove (PraghaDatabaseProvider *provider, gint provider_id)
{
	g_return_if_fail(PRAGHA_IS_DATABASE_PROVIDER(provider));

	g_signal_emit (provider, signals[SIGNAL_WANT_UPDATE], 0, provider_id);
}


void
pragha_provider_update_done (PraghaDatabaseProvider *provider)
{
	g_return_if_fail(PRAGHA_IS_DATABASE_PROVIDER(provider));

	g_signal_emit (provider, signals[SIGNAL_UPDATE_DONE], 0);
}


/*
 * PraghaDatabaseProvider implementation.
 */

static void
pragha_database_provider_dispose (GObject *object)
{
	PraghaDatabaseProvider *provider = PRAGHA_DATABASE_PROVIDER(object);
	PraghaDatabaseProviderPrivate *priv = provider->priv;

	if (priv->database) {
		g_object_unref (priv->database);
		priv->database = NULL;
	}

	G_OBJECT_CLASS(pragha_database_provider_parent_class)->dispose(object);
}

static void
pragha_database_provider_class_init (PraghaDatabaseProviderClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = pragha_database_provider_dispose;

	signals[SIGNAL_WANT_UPGRADE] =
		g_signal_new ("want-upgrade",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDatabaseProviderClass, want_upgrade),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SIGNAL_WANT_UPDATE] =
		g_signal_new ("want-update",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDatabaseProviderClass, want_update),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SIGNAL_WANT_REMOVE] =
		g_signal_new ("want-remove",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDatabaseProviderClass, want_remove),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SIGNAL_UPDATE_DONE] =
		g_signal_new ("update-done",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaDatabaseProviderClass, update_done),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	g_type_class_add_private(object_class, sizeof(PraghaDatabaseProviderPrivate));
}

static void
pragha_database_provider_init (PraghaDatabaseProvider *provider)
{
	provider->priv = G_TYPE_INSTANCE_GET_PRIVATE(provider,
	                                             PRAGHA_TYPE_DATABASE_PROVIDER,
	                                             PraghaDatabaseProviderPrivate);

	PraghaDatabaseProviderPrivate *priv = provider->priv;

	/* Database instance */

	priv->database = pragha_database_get ();
}

/**
 * pragha_database_provider_get:
 *
 * Queries the global #PraghaDatabaseProvider instance, which is
 * shared by all modules. The function automatically takes a
 * reference for the caller, so you'll need to call g_object_unref()
 * when you're done with it.
 *
 * Return value: the global #PraghaDatabaseProvider instance.
 **/
PraghaDatabaseProvider *
pragha_database_provider_get (void)
{
   static PraghaDatabaseProvider *provider = NULL;

   if (G_UNLIKELY (provider == NULL)) {
      provider = g_object_new(PRAGHA_TYPE_DATABASE_PROVIDER, NULL);
      g_object_add_weak_pointer(G_OBJECT (provider),
                                (gpointer) &provider);
   }
   else {
      g_object_ref (G_OBJECT (provider));
   }

   return provider;
}
