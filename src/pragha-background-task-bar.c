/*************************************************************************/
/* Copyright (C) 2016 matias <mati86dl@gmail.com>                        */
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

#define PRAGHA_TYPE_BACKGROUND_TASK_BAR (pragha_background_task_bar_get_type())
#define PRAGHA_BACKGROUND_TASK_BAR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_BACKGROUND_TASK_BAR, PraghaBackgroundTaskBar))
#define PRAGHA_BACKGROUND_TASK_BAR_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_BACKGROUND_TASK_BAR, PraghaBackgroundTaskBar const))
#define PRAGHA_BACKGROUND_TASK_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_BACKGROUND_TASK_BAR, PraghaBackgroundTaskBarClass))
#define PRAGHA_IS_BACKGROUND_TASK_BAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_BACKGROUND_TASK_BAR))
#define PRAGHA_IS_BACKGROUND_TASK_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_BACKGROUND_TASK_BAR))
#define PRAGHA_BACKGROUND_TASK_BAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_BACKGROUND_TASK_BAR, PraghaBackgroundTaskBarClass))

typedef struct _PraghaBackgroundTaskBarClass PraghaBackgroundTaskBarClass;

struct _PraghaBackgroundTaskBarClass {
	GtkBoxClass parent_class;
};
struct _PraghaBackgroundTaskBar {
	GtkBox    _parent;

	gint      task_count;

	GtkWidget *popover;
	GtkWidget *listbox;
	GtkWidget *spinner;
	GtkWidget *label;
};
G_DEFINE_TYPE(PraghaBackgroundTaskBar, pragha_background_task_bar, GTK_TYPE_BOX)

static gboolean
pragha_background_task_bar_button_press_event (GtkWidget	  *widget,
                                               GdkEventButton *event,
                                               GtkWidget      *wtaskbar)
{
	PraghaBackgroundTaskBar *taskbar = PRAGHA_BACKGROUND_TASK_BAR(wtaskbar);

	if (taskbar->task_count < 0)
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
	GtkWidget *event_box, *hbox;

	/* Main widget */

	event_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box), FALSE);
	g_signal_connect (G_OBJECT (event_box), "button_press_event",
	                  G_CALLBACK (pragha_background_task_bar_button_press_event), taskbar);

	gtk_container_add (GTK_CONTAINER(taskbar), GTK_WIDGET(event_box));

	taskbar->label = gtk_label_new (_("There are background tasks working."));
	taskbar->spinner = gtk_spinner_new ();

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX(hbox), taskbar->label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox), taskbar->spinner, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(event_box), GTK_WIDGET(hbox));

	/* Popover */

	taskbar->popover = gtk_popover_new (GTK_WIDGET(taskbar));
	gtk_popover_set_relative_to (GTK_POPOVER(taskbar->popover), GTK_WIDGET(taskbar));

	/* List box */

	taskbar->listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode (GTK_LIST_BOX(taskbar->listbox), GTK_SELECTION_NONE);
	gtk_widget_show (taskbar->listbox);

	gtk_container_add (GTK_CONTAINER(taskbar->popover), taskbar->listbox);

	gtk_widget_show_all(GTK_WIDGET(taskbar));
	gtk_widget_hide(GTK_WIDGET(taskbar));
}

void
pragha_background_task_bar_prepend_widget (PraghaBackgroundTaskBar *taskbar,
                                           GtkWidget               *widget)
{
	gtk_list_box_prepend (GTK_LIST_BOX(taskbar->listbox), widget);

	taskbar->task_count++;

	gtk_widget_show_all (widget);
	gtk_widget_show_all (GTK_WIDGET(taskbar));
	gtk_widget_show_all (GTK_WIDGET(taskbar->popover));

	gtk_spinner_start (GTK_SPINNER(taskbar->spinner));
}

void
pragha_background_task_bar_remove_widget (PraghaBackgroundTaskBar *taskbar,
                                          GtkWidget               *widget)
{
	gtk_container_remove (GTK_CONTAINER(taskbar->listbox), widget);

	taskbar->task_count--;
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