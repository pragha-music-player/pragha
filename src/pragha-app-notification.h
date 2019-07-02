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

#ifndef PRAGHA_APP_NOTIFICATION_H
#define PRAGHA_APP_NOTIFICATION_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define PRAGHA_APP_TYPE_NOTIFICATION (pragha_app_notification_get_type ())

G_DECLARE_FINAL_TYPE (PraghaAppNotification, pragha_app_notification, PRAGHA, APP_NOTIFICATION, GtkFrame)

PraghaAppNotification *
pragha_app_notification_new         (const char            *head,
                                     const char            *body);

void
pragha_app_notification_show        (PraghaAppNotification *self);

void
pragha_app_notification_set_timeout (PraghaAppNotification *self,
                                     guint                  timeout);

G_END_DECLS

#endif /* PRAGHA_APP_NOTIFICATION_H */
