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

#include "pragha-background-task-widget.h"

typedef struct _PraghaBackgroundTaskWidgetClass PraghaBackgroundTaskWidgetClass;

struct _PraghaBackgroundTaskWidgetClass {
	GtkListBoxRowClass parent_class;
};
struct _PraghaBackgroundTaskWidget {
	GtkListBoxRow _parent;

	gchar         *icon_name;
	gchar         *description;
	gint           job_count;
	gint           job_progress;

	GCancellable  *cancellable;

	GtkWidget     *icon;
	GtkWidget     *progress;
	GtkWidget     *cancell_button;
};

enum {
	PROP_0,
	PROP_DESCRIPTION,
	PROP_ICON_NAME,
	PROP_JOB_COUNT,
	PROP_JOB_PROGRESS,
	PROP_CANCELLABLE,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { 0 };

G_DEFINE_TYPE(PraghaBackgroundTaskWidget, pragha_background_task_widget, GTK_TYPE_LIST_BOX_ROW)

void
pragha_background_task_widget_set_description (PraghaBackgroundTaskWidget *taskwidget,
                                               const gchar                *description)
{
	if (taskwidget->description)
		g_free(taskwidget->description);
	taskwidget->description = g_strdup (description);

	gtk_progress_bar_set_text (GTK_PROGRESS_BAR(taskwidget->progress), description);

	g_object_notify_by_pspec (G_OBJECT(taskwidget), properties[PROP_DESCRIPTION]);
}

const gchar *
pragha_background_task_widget_get_description (PraghaBackgroundTaskWidget *taskwidget)
{
	return taskwidget->description;
}

void
pragha_background_task_widget_set_icon_name (PraghaBackgroundTaskWidget *taskwidget,
                                             const gchar                *icon_name)
{
	if (taskwidget->icon_name)
		g_free(taskwidget->icon_name);
	taskwidget->icon_name = g_strdup (icon_name);

	gtk_image_set_from_icon_name (GTK_IMAGE(taskwidget->icon),
	                              icon_name,
	                              GTK_ICON_SIZE_SMALL_TOOLBAR);
}

const gchar *
pragha_background_task_widget_get_icon_name (PraghaBackgroundTaskWidget *taskwidget)
{
	return taskwidget->icon_name;
}

static void
pragha_background_task_widget_set_job_count (PraghaBackgroundTaskWidget *taskwidget,
                                             gint                        job_count)
{
	taskwidget->job_count = job_count;
}

static guint
pragha_background_task_widget_get_job_count (PraghaBackgroundTaskWidget *taskwidget)
{
	return taskwidget->job_count;
}

void
pragha_background_task_widget_set_job_progress (PraghaBackgroundTaskWidget *taskwidget,
                                                gint                        job_progress)
{
	if (job_progress > 0) {
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(taskwidget->progress), (gdouble)job_progress/100);
	}
	else {
		gtk_progress_bar_pulse (GTK_PROGRESS_BAR(taskwidget->progress));
	}
}

static guint
pragha_background_task_widget_get_job_progress (PraghaBackgroundTaskWidget *taskwidget)
{
	return 48;
}

static void
pragha_background_task_widget_set_cancellable (PraghaBackgroundTaskWidget *taskwidget,
                                               GCancellable               *cancellable)
{
	if (taskwidget->cancellable) {
		g_object_unref (taskwidget->cancellable);
	}
	taskwidget->cancellable = cancellable;
}

static GObject *
pragha_background_task_widget_get_cancellable (PraghaBackgroundTaskWidget *taskwidget)
{
	return G_OBJECT(taskwidget->cancellable);
}

static void
pragha_background_task_widget_set_property (GObject      *object,
                                            guint         property_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	PraghaBackgroundTaskWidget *taskwidget = PRAGHA_BACKGROUND_TASK_WIDGET(object);

	switch (property_id)
	{
		case PROP_DESCRIPTION:
			pragha_background_task_widget_set_description (taskwidget, g_value_get_string (value));
			break;
		case PROP_ICON_NAME:
			pragha_background_task_widget_set_icon_name (taskwidget, g_value_get_string (value));
			break;
		case PROP_JOB_COUNT:
			pragha_background_task_widget_set_job_count (taskwidget, g_value_get_uint (value));
			break;
		case PROP_JOB_PROGRESS:
			pragha_background_task_widget_set_job_progress (taskwidget, g_value_get_uint (value));
			break;
		case PROP_CANCELLABLE:
			pragha_background_task_widget_set_cancellable (taskwidget, g_value_dup_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_background_task_widget_get_property (GObject    *object,
                                            guint       property_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
	PraghaBackgroundTaskWidget *taskwidget = PRAGHA_BACKGROUND_TASK_WIDGET(object);

	switch (property_id)
	{
		case PROP_DESCRIPTION:
			g_value_set_string (value, pragha_background_task_widget_get_description (taskwidget));
			break;
		case PROP_ICON_NAME:
			g_value_set_string (value, pragha_background_task_widget_get_icon_name (taskwidget));
			break;
		case PROP_JOB_COUNT:
			g_value_set_uint (value, pragha_background_task_widget_get_job_count (taskwidget));
			break;
		case PROP_JOB_PROGRESS:
			g_value_set_uint (value, pragha_background_task_widget_get_job_progress (taskwidget));
			break;
		case PROP_CANCELLABLE:
			g_value_set_object (value, pragha_background_task_widget_get_cancellable (taskwidget));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_background_task_widget_dispose (GObject *object)
{
	PraghaBackgroundTaskWidget *taskwidget = PRAGHA_BACKGROUND_TASK_WIDGET(object);

	if (taskwidget->cancellable != NULL) {
		g_object_unref (taskwidget->cancellable);
		taskwidget->cancellable = NULL;
	}
	G_OBJECT_CLASS (pragha_background_task_widget_parent_class)->dispose (object);
}

static void
pragha_background_task_widget_class_init (PraghaBackgroundTaskWidgetClass *class)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (class);
	gobject_class->set_property = pragha_background_task_widget_set_property;
	gobject_class->get_property = pragha_background_task_widget_get_property;
	gobject_class->dispose = pragha_background_task_widget_dispose;

	/**
	 * PraghaBackgroundTaskBar:description:
	 *
	 */
	properties[PROP_DESCRIPTION] =
		g_param_spec_string("description",
		                    "Description",
		                    "The description of task",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_STATIC_STRINGS);

	/**
	 * PraghaBackgroundTaskBar:icon-name:
	 *
	 */
	properties[PROP_ICON_NAME] =
		g_param_spec_string("icon-name",
		                    "IconName",
		                    "The icon-name used on task",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_STATIC_STRINGS);

	/**
	 * PraghaBackgroundTask:job-count:
	 *
	 */
	properties[PROP_JOB_COUNT] =
		g_param_spec_uint("job-count",
		                  "Job-Count",
		                  "The job-count to show progress",
		                  0, 128,
		                  0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_STATIC_STRINGS);

	/**
	 * PraghaBackgroundTask:job-progress:
	 *
	 */
	properties[PROP_JOB_PROGRESS] =
		g_param_spec_uint("job-progress",
		                  "Job-Progress",
		                  "The job progress",
		                  0, 128,
		                  0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_STATIC_STRINGS);

	/**
	 * PraghaBackgroundTask:cancellable:
	 *
	 */
	properties[PROP_CANCELLABLE] =
		g_param_spec_object("cancellable",
		                    "Cancellable",
		                    "A GCancellable to cancel the task.",
		                    G_TYPE_CANCELLABLE,
		                    G_PARAM_READWRITE |
		                    G_PARAM_CONSTRUCT_ONLY |
		                    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class, PROP_LAST, properties);

}

static void
pragha_background_task_widget_cancell_handler (GtkButton                  *button,
                                               PraghaBackgroundTaskWidget *taskwidget)
{
	g_cancellable_cancel (taskwidget->cancellable);
}

static void
pragha_background_task_widget_init (PraghaBackgroundTaskWidget *taskwidget)
{
	GtkWidget *box, *image;

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	taskwidget->icon = gtk_image_new_from_icon_name ("folder-music", GTK_ICON_SIZE_MENU);

	taskwidget->progress = gtk_progress_bar_new();
	gtk_widget_set_size_request (taskwidget->progress, 300, -1);
	gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR(taskwidget->progress), TRUE);

	taskwidget->cancell_button = gtk_button_new ();
	image = gtk_image_new_from_icon_name ("process-stop", GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (taskwidget->cancell_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (taskwidget->cancell_button), FALSE);
	gtk_container_add (GTK_CONTAINER (taskwidget->cancell_button), image);

	g_signal_connect (G_OBJECT (taskwidget->cancell_button), "clicked",
	                  G_CALLBACK(pragha_background_task_widget_cancell_handler), taskwidget);

	gtk_box_pack_start (GTK_BOX (box), taskwidget->icon, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), taskwidget->progress, TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), taskwidget->cancell_button, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (taskwidget), box);

	gtk_widget_show_all (GTK_WIDGET(taskwidget));
}

PraghaBackgroundTaskWidget *
pragha_background_task_widget_new (const gchar  *description,
                                   const gchar  *icon_name,
                                   gint          job_count,
                                   GCancellable *cancellable)
{
	return g_object_new (PRAGHA_TYPE_BACKGROUND_TASK_WIDGET,
	                     "description", description,
	                     "icon-name", icon_name,
	                     "job-count", job_count,
	                     "cancellable", cancellable,
	                     NULL);
}