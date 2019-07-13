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

typedef struct _PraghaAppNotificationClass PraghaAppNotificationClass;
typedef struct _PraghaAppNotification      PraghaAppNotification;

#define PRAGHA_TYPE_APP_NOTIFICATION             (pragha_app_notification_get_type ())
#define PRAGHA_APP_NOTIFICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_APP_NOTIFICATION, PraghaAppNotification))
#define PRAGHA_APP_NOTIFICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_APP_NOTIFICATION, PraghaAppNotificationClass))
#define PRAGHA_IS_APP_NOTIFICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_APP_NOTIFICATION))
#define PRAGHA_IS_APP_NOTIFICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_APP_NOTIFICATION))
#define PRAGHA_APP_NOTIFICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_APP_NOTIFICATION, PraghaAppNotificationClass))

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
