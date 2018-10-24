/*************************************************************************/
/* Copyright (C) 2016-2018 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-background-task-widget.h"
#include "pragha-background-task-bar.h"

typedef struct _PraghaBackgroundTaskBarClass PraghaBackgroundTaskBarClass;

struct _PraghaBackgroundTaskBarClass {
	GtkButtonClass parent_class;
};

struct _PraghaBackgroundTaskBar {
	GtkButton _parent;

	GBinding  *label_binding;

	gint       task_count;

	GtkWidget  *popover;
	GtkWidget  *listbox;
	GtkWidget  *spinner;
};

G_DEFINE_TYPE(PraghaBackgroundTaskBar, pragha_background_task_bar, GTK_TYPE_BUTTON)

static gboolean
pragha_background_task_bar_clicked_event (GtkWidget	  *widget,
                                          GtkWidget      *wtaskbar)
{
	PraghaBackgroundTaskBar *taskbar = PRAGHA_BACKGROUND_TASK_BAR(wtaskbar);

	if (taskbar->task_count <= 0)
		return FALSE;

	gtk_widget_show (GTK_WIDGET(taskbar->popover));

	return FALSE;
}

static void
pragha_background_task_bar_class_init (PraghaBackgroundTaskBarClass *class)
{
	/*GtkWidgetClass *widget_class;
	widget_class = GTK_WIDGET_CLASS (class);
	widget_class->button_press_event = pragha_background_task_bar_button_press_event;*/
}

static void
pragha_background_task_bar_init (PraghaBackgroundTaskBar *taskbar)
{
	/* Main widget */

	gtk_button_set_relief (GTK_BUTTON(taskbar), GTK_RELIEF_NONE);

	taskbar->spinner = gtk_spinner_new ();
	gtk_container_add (GTK_CONTAINER(taskbar), GTK_WIDGET(taskbar->spinner));

	gtk_widget_set_tooltip_text (GTK_WIDGET(taskbar), _("There are background tasks working"));

	g_signal_connect (G_OBJECT (taskbar), "clicked",
	                  G_CALLBACK (pragha_background_task_bar_clicked_event), taskbar);

	/* Popover */

	taskbar->popover = gtk_popover_new (GTK_WIDGET(taskbar));
	gtk_popover_set_relative_to (GTK_POPOVER(taskbar->popover), GTK_WIDGET(taskbar));

	/* List box */

	taskbar->listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode (GTK_LIST_BOX(taskbar->listbox), GTK_SELECTION_NONE);
	gtk_widget_show (taskbar->listbox);

	gtk_container_add (GTK_CONTAINER(taskbar->popover), taskbar->listbox);

	/* Show properly.. */

	gtk_widget_show_all(GTK_WIDGET(taskbar));
	gtk_widget_hide(GTK_WIDGET(taskbar));
}

static void
pragha_background_task_bar_show_first_description (PraghaBackgroundTaskBar *taskbar)
{
	GtkListBoxRow *row;
	row = gtk_list_box_get_row_at_index (GTK_LIST_BOX(taskbar->listbox), 0);
	taskbar->label_binding =
		g_object_bind_property (PRAGHA_BACKGROUND_TASK_WIDGET (row), "description",
		                        taskbar, "tooltip-text",
		                        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
}

static void
pragha_background_task_bar_show_generic_description (PraghaBackgroundTaskBar *taskbar)
{
	if (taskbar->label_binding)  {
		g_binding_unbind (taskbar->label_binding);
		taskbar->label_binding = NULL;
	}

	gtk_widget_set_tooltip_text (GTK_WIDGET(taskbar), _("There are background tasks working"));
}

void
pragha_background_task_bar_prepend_widget (PraghaBackgroundTaskBar *taskbar,
                                           GtkWidget               *widget)
{
	/* Append the widget and ref it */

	gtk_list_box_prepend (GTK_LIST_BOX(taskbar->listbox), widget);

	taskbar->task_count++;

	/* Update description */

	if (taskbar->task_count == 1)
		pragha_background_task_bar_show_first_description (taskbar);
	else
		pragha_background_task_bar_show_generic_description (taskbar);

	/* Show widgets */

	gtk_widget_show_all (GTK_WIDGET(taskbar));
	gtk_widget_show (GTK_WIDGET(taskbar->popover));
	gtk_widget_show (widget);

	gtk_spinner_start (GTK_SPINNER(taskbar->spinner));
}

void
pragha_background_task_bar_remove_widget (PraghaBackgroundTaskBar *taskbar,
                                          GtkWidget               *widget)
{
	/* Remove the widget and unref it */

	gtk_container_remove (GTK_CONTAINER(taskbar->listbox), widget);

	taskbar->task_count--;

	/* Update description */

	if (taskbar->task_count == 1)
		pragha_background_task_bar_show_first_description (taskbar);
	else
		pragha_background_task_bar_show_generic_description (taskbar);

	/* Hide widgets when unnecessary */

	if (taskbar->task_count == 0) {
		gtk_widget_hide (GTK_WIDGET(taskbar->popover));
		gtk_spinner_stop (GTK_SPINNER(taskbar->spinner));
		gtk_widget_hide (GTK_WIDGET(taskbar));
	}
}

PraghaBackgroundTaskBar *
pragha_background_task_bar_new (void)
{
	return g_object_new (PRAGHA_TYPE_BACKGROUND_TASK_BAR, NULL);
}
