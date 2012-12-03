/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#ifndef PRAGHA_PREFERENCES_H
#define PRAGHA_PREFERENCES_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_PREFERENCES (pragha_preferences_get_type())
#define PRAGHA_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferences))
#define PRAGHA_PREFERENCES_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferences const))
#define PRAGHA_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PREFERENCES, PraghaPreferencesClass))
#define PRAGHA_IS_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PREFERENCES))
#define PRAGHA_IS_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PREFERENCES))
#define PRAGHA_PREFERENCES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PREFERENCES, PraghaPreferencesClass))

typedef struct _PraghaPreferences PraghaPreferences;
typedef struct _PraghaPreferencesClass PraghaPreferencesClass;
typedef struct _PraghaPreferencesPrivate PraghaPreferencesPrivate;

struct _PraghaPreferences
{
   GObject parent;

   /*< private >*/
   PraghaPreferencesPrivate *priv;
};

struct _PraghaPreferencesClass
{
   GObjectClass parent_class;
};

PraghaPreferences* pragha_preferences_get (void);
GType pragha_preferences_get_type (void) G_GNUC_CONST;

gint *
pragha_preferences_get_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gsize *length);
void
pragha_preferences_set_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gint list[],
                                     gsize length);

gdouble *
pragha_preferences_get_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key);
void
pragha_preferences_set_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gdouble list[],
                                    gsize length);

gchar *
pragha_preferences_get_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key);

void
pragha_preferences_set_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key,
                               const gchar *string);

gchar **
pragha_preferences_get_string_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gsize *length);
void
pragha_preferences_set_string_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    const gchar * const list[],
                                    gsize length);

void
pragha_preferences_remove_key (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key);

void
pragha_preferences_set_approximate_search (PraghaPreferences *prefernces,
                                           gboolean approximate_search);
gboolean
pragha_preferences_get_approximate_search (PraghaPreferences *preferences);

void
pragha_preferences_set_instant_search (PraghaPreferences *preferences,
                                       gboolean instant_search);
gboolean
pragha_preferences_get_instant_search (PraghaPreferences *preferences);

void
pragha_preferences_set_shuffle (PraghaPreferences *preferences,
                                gboolean shuffle);
gboolean
pragha_preferences_get_shuffle (PraghaPreferences *preferences);

void
pragha_preferences_set_repeat (PraghaPreferences *preferences,
                               gboolean repeat);
gboolean
pragha_preferences_get_repeat (PraghaPreferences *preferences);

void
pragha_preferences_set_use_hint (PraghaPreferences *preferences,
                                 gboolean use_hint);
gboolean
pragha_preferences_get_use_hint (PraghaPreferences *preferences);

void
pragha_preferences_set_restore_playlist (PraghaPreferences *preferences,
                                         gboolean restore_playlist);
gboolean
pragha_preferences_get_restore_playlist (PraghaPreferences *preferences);

G_END_DECLS

#endif /* PRAGHA_PREFERENCES_H */
