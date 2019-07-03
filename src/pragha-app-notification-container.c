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

#include "config.h"
#include "pragha-app-notification-container.h"


struct _PraghaAppNotificationContainer {
	GtkRevealer  parent_instance;

	GtkWidget   *grid;
};

struct _PraghaAppNotificationContainerClass {
	GtkRevealerClass parent_class;
};

G_DEFINE_TYPE (PraghaAppNotificationContainer, pragha_app_notification_container, GTK_TYPE_REVEALER);

static PraghaAppNotificationContainer *notification_container = NULL;

static void
pragha_app_notification_container_init (PraghaAppNotificationContainer *self)
{
	/* Globally accessible singleton */
	g_assert (notification_container == NULL);
	notification_container = self;
	g_object_add_weak_pointer (G_OBJECT (notification_container),
	                           (gpointer *)&notification_container);

	gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_CENTER);
	gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_START);

	self->grid = gtk_grid_new ();
	gtk_orientable_set_orientation (GTK_ORIENTABLE (self->grid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (GTK_GRID (self->grid), 6);
	gtk_container_add (GTK_CONTAINER (self), self->grid);
}

static void
pragha_app_notification_container_class_init (PraghaAppNotificationContainerClass *klass)
{
}

PraghaAppNotificationContainer *
pragha_app_notification_container_get_default (void)
{
	if (notification_container != NULL)
		return notification_container;

	return g_object_new (PRAGHA_TYPE_APP_NOTIFICATION_CONTAINER,
	                     NULL);
}

void
pragha_app_notification_container_add_notification (PraghaAppNotificationContainer *self,
                                                    GtkWidget                      *notification)
{
	g_assert (PRAGHA_IS_APP_NOTIFICATION_CONTAINER (self));
	g_assert (GTK_IS_WIDGET (notification));

	gtk_container_add (GTK_CONTAINER (self->grid), notification);

	gtk_widget_show (GTK_WIDGET (self));
	gtk_widget_show (GTK_WIDGET (self->grid));
	gtk_widget_show (GTK_WIDGET (notification));

	gtk_revealer_set_reveal_child (GTK_REVEALER (self), TRUE);
}

guint
pragha_app_notification_container_get_num_children (PraghaAppNotificationContainer *self)
{
	GList *children;
	guint retval;

	g_assert (PRAGHA_IS_APP_NOTIFICATION_CONTAINER (self));

	children = gtk_container_get_children (GTK_CONTAINER (self->grid));
	retval = g_list_length (children);
	g_list_free (children);

	return retval;
}
