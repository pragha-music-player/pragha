/*************************************************************************/
/* Copyright (C) 2016 matias <mati86dl@gmail.com>                        */
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

#ifndef PRAGHA_PROVIDER_H
#define PRAGHA_PROVIDER_H

#include <glib-object.h>

G_BEGIN_DECLS

GType pragha_provider_get_type (void);

#define PRAGHA_TYPE_PROVIDER (pragha_provider_get_type())
#define PRAGHA_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PROVIDER, PraghaProvider))
#define PRAGHA_PROVIDER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PROVIDER, PraghaProvider const))
#define PRAGHA_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PROVIDER, PraghaProviderClass))
#define PRAGHA_IS_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PROVIDER))
#define PRAGHA_IS_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PROVIDER))
#define PRAGHA_PROVIDER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PROVIDER, PraghaProviderClass))

typedef struct _PraghaProvider PraghaProvider;
typedef struct _PraghaProviderClass PraghaProviderClass;

struct _PraghaProviderClass {
	GObjectClass parent_class;
};

/*
 * Public api.
 */

PraghaProvider *
pragha_provider_new (const gchar *name,
                     const gchar *kind,
                     const gchar *friendly_name,
                     const gchar *icon_name,
                     gboolean     visible,
                     gboolean     ignored);

const gchar *
pragha_provider_get_name          (PraghaProvider *provider);
const gchar *
pragha_provider_get_kind          (PraghaProvider *provider);
const gchar *
pragha_provider_get_friendly_name (PraghaProvider *provider);
const gchar *
pragha_provider_get_icon_name     (PraghaProvider *provider);
gboolean
pragha_provider_get_visible       (PraghaProvider *provider);
gboolean
pragha_provider_get_ignored       (PraghaProvider *provider);

G_END_DECLS

#endif /* PRAGHA_PROVIDER_H */

