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

#ifndef PRAGHA_TEMP_PROVIDER_H
#define PRAGHA_TEMP_PROVIDER_H

#include <glib.h>
#include <glib-object.h>

#include "pragha-musicobject.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_TEMP_PROVIDER (pragha_temp_provider_get_type())
#define PRAGHA_TEMP_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TEMP_PROVIDER, PraghaTempProvider))
#define PRAGHA_TEMP_PROVIDER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TEMP_PROVIDER, PraghaTempProvider const))
#define PRAGHA_TEMP_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TEMP_PROVIDER, PraghaTempProviderClass))
#define PRAGHA_IS_TEMP_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TEMP_PROVIDER))
#define PRAGHA_IS_TEMP_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TEMP_PROVIDER))
#define PRAGHA_TEMP_PROVIDER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TEMP_PROVIDER, PraghaTempProviderClass))

typedef struct _PraghaTempProvider PraghaTempProvider;
typedef struct _PraghaTempProviderClass PraghaTempProviderClass;

struct _PraghaTempProviderClass
{
	GObjectClass parent_class;
};


typedef gboolean (ProviderCheckFunc) (gpointer key, gpointer value, gpointer user_data);


void
pragha_temp_provider_insert_track (PraghaTempProvider *provider,
                                   PraghaMusicobject  *mobj);

void
pragha_temp_provider_delete_track (PraghaTempProvider *provider,
                                   PraghaMusicobject  *mobj);

void
pragha_temp_provider_replace_track (PraghaTempProvider *provider,
                                    PraghaMusicobject  *mobj);

void
pragha_temp_provider_foreach_purge (PraghaTempProvider *provider,
                                    ProviderCheckFunc  *check_func,
                                    gpointer            user_data);

PraghaTempProvider *pragha_temp_provider_new      (const gchar *name);

G_END_DECLS

#endif /* PRAGHA_TEMP_PROVIDER_H */
