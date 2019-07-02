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

#ifndef PRAGHA_APP_NOTIFICATION_CONTAINER_H
#define PRAGHA_APP_NOTIFICATION_CONTAINER_H

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define PRAGHA_TYPE_APP_NOTIFICATION_CONTAINER (pragha_app_notification_container_get_type ())

G_DECLARE_FINAL_TYPE (PraghaAppNotificationContainer, pragha_app_notification_container, PRAGHA, APP_NOTIFICATION_CONTAINER, GtkRevealer)

PraghaAppNotificationContainer *
pragha_app_notification_container_get_default      (void);

void
pragha_app_notification_container_add_notification (PraghaAppNotificationContainer *self,
                                                    GtkWidget                      *notification);

guint
pragha_app_notification_container_get_num_children (PraghaAppNotificationContainer *self);

G_END_DECLS

#endif /* PRAGHA_APP_NOTIFICAION_CONTAINER_H */
