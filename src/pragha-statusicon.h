/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_STATUSICON_H
#define PRAGHA_STATUSICON_H

#include <gtk/gtk.h>

/* pragha.h */
typedef struct _PraghaApplication PraghaApplication;

#define PRAGHA_TYPE_STATUS_ICON                  (pragha_status_icon_get_type ())
#define PRAGHA_STATUS_ICON(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_STATUS_ICON, PraghaStatusIcon))
#define PRAGHA_IS_STATUS_ICON(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_STATUS_ICON))
#define PRAGHA_STATUS_ICON_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_STATUS_ICON, PraghaStatusIconClass))
#define PRAGHA_IS_STATUS_ICON_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_STATUS_ICON))
#define PRAGHA_STATUS_ICON_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_STATUS_ICON, PraghaStatusIconClass))

typedef struct {
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	GtkStatusIconClass __parent__;
G_GNUC_END_IGNORE_DEPRECATIONS
} PraghaStatusIconClass;

typedef struct _PraghaStatusIcon PraghaStatusIcon;

void
pragha_systray_append_action (PraghaStatusIcon *status_icon,
                              const gchar      *placeholder,
                              GSimpleAction    *action,
                              GMenuItem        *item);

void
pragha_systray_remove_action (PraghaStatusIcon *status_icon,
                              const gchar      *placeholder,
                              const gchar      *action_name);

PraghaStatusIcon *pragha_status_icon_new (PraghaApplication *pragha);

#endif /* PRAGHA_STATUSICON_H */
