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

#ifndef PRAGHA_DATABASE_PROVIDER_H
#define PRAGHA_DATABASE_PROVIDER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_DATABASE_PROVIDER            (pragha_database_provider_get_type())
#define PRAGHA_DATABASE_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_DATABASE_PROVIDER, PraghaDatabaseProvider))
#define PRAGHA_DATABASE_PROVIDER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_DATABASE_PROVIDER, PraghaDatabaseProvider const))
#define PRAGHA_DATABASE_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_DATABASE_PROVIDER, PraghaDatabaseProviderClass))
#define PRAGHA_IS_DATABASE_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_DATABASE_PROVIDER))
#define PRAGHA_IS_DATABASE_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_DATABASE_PROVIDER))
#define PRAGHA_DATABASE_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_DATABASE_PROVIDER, PraghaDatabaseProviderClass))

typedef struct _PraghaDatabaseProvider        PraghaDatabaseProvider;
typedef struct _PraghaDatabaseProviderClass   PraghaDatabaseProviderClass;
typedef struct _PraghaDatabaseProviderPrivate PraghaDatabaseProviderPrivate;

struct _PraghaDatabaseProvider
{
	GObject parent;

	/*< private >*/
	PraghaDatabaseProviderPrivate *priv;
};

struct _PraghaDatabaseProviderClass
{
	GObjectClass parent_class;

	void (*visibility_changed) (PraghaDatabaseProvider *provider);
	void (*want_update)        (PraghaDatabaseProvider *provider);
	void (*update_done)        (PraghaDatabaseProvider *provider);
};

/*
 * Public api.
 */

void
pragha_provider_add_new (PraghaDatabaseProvider *provider,
                         const gchar            *name,
                         const gchar            *type,
                         const gchar            *friendly_name,
                         const gchar            *icon_name);

void
pragha_provider_remove (PraghaDatabaseProvider *provider,
                        const gchar            *name);

GSList *
pragha_provider_get_list (PraghaDatabaseProvider *provider);

GSList *
pragha_provider_get_handled_list (PraghaDatabaseProvider *provider);

GSList *
pragha_provider_get_list_by_type (PraghaDatabaseProvider *provider,
                                  const gchar            *provider_type);

GSList *
pragha_provider_get_handled_list_by_type (PraghaDatabaseProvider *provider,
                                          const gchar            *provider_type);

gchar *
pragha_provider_get_friendly_name (PraghaDatabaseProvider *provider, const gchar *name);

gchar *
pragha_provider_get_icon_name (PraghaDatabaseProvider *provider, const gchar *name);

void
pragha_provider_visibility_changed (PraghaDatabaseProvider *provider);

void
pragha_provider_want_update (PraghaDatabaseProvider *provider);

void
pragha_provider_update_done (PraghaDatabaseProvider *provider);

PraghaDatabaseProvider *
pragha_database_provider_get (void);

GType pragha_database_provider_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* PRAGHA_DATABASE_PROVIDER_H */
