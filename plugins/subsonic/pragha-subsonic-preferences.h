/*************************************************************************/
/* Copyright (C) 2019 matias <mati86dl@gmail.com>                        */
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

#ifndef __PRAGHA_SUBSONIC_PREFERENCES_H__
#define __PRAGHA_SUBSONIC_PREFERENCES_H__

#include <glib.h>
#include <glib-object.h>

#include "src/pragha-preferences-dialog.h"

G_BEGIN_DECLS

#define PRAGHA_TYPE_SUBSONIC_PREFERENCES            (pragha_subsonic_preferences_get_type())
#define PRAGHA_SUBSONIC_PREFERENCES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SUBSONIC_PREFERENCES, PraghaSubsonicPreferences))
#define PRAGHA_SUBSONIC_PREFERENCES_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SUBSONIC_PREFERENCES, PraghaSubsonicPreferences const))
#define PRAGHA_SUBSONIC_PREFERENCES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_SUBSONIC_PREFERENCES, PraghaSubsonicPreferencesClass))
#define PRAGHA_IS_SUBSONIC_PREFERENCES(obj) (        G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SUBSONIC_PREFERENCES))
#define PRAGHA_IS_SUBSONIC_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_SUBSONIC_PREFERENCES))
#define PRAGHA_SUBSONIC_PREFERENCES_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_SUBSONIC_PREFERENCES, PraghaSubsonicPreferencesClass))

typedef struct _PraghaSubsonicPreferences      PraghaSubsonicPreferences;
typedef struct _PraghaSubsonicPreferencesClass PraghaSubsonicPreferencesClass;

struct _PraghaSubsonicPreferencesClass
{
	GObjectClass                 parent_class;

	void (*server_changed)      (PraghaSubsonicPreferences *subsonic);
	void (*credentials_changed) (PraghaSubsonicPreferences *subsonic);
};


/*
 * Public methods
 */

const gchar *
pragha_subsonic_preferences_get_server_text   (PraghaSubsonicPreferences *preferences);

const gchar *
pragha_subsonic_preferences_get_username_text (PraghaSubsonicPreferences *preferences);

const gchar *
pragha_subsonic_preferences_get_password_text (PraghaSubsonicPreferences *preferences);

void
pragha_subsonic_preferences_forget_settings   (PraghaSubsonicPreferences *preferences);

PraghaSubsonicPreferences *
pragha_subsonic_preferences_new               (PreferencesDialog *dialog);

G_END_DECLS

#endif /* __PRAGHA_SUBSONIC_PREFERENCES_H__ */
