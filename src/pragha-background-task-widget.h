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

#ifndef PRAGHA_BACKGROUND_TASK_WIDGET_H
#define PRAGHA_BACKGROUND_TASK_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_BACKGROUND_TASK_WIDGET (pragha_background_task_widget_get_type())
#define PRAGHA_BACKGROUND_TASK_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET, PraghaBackgroundTaskWidget))
#define PRAGHA_BACKGROUND_TASK_WIDGET_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET, PraghaBackgroundTaskWidget const))
#define PRAGHA_BACKGROUND_TASK_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET, PraghaBackgroundTaskWidgetClass))
#define PRAGHA_IS_BACKGROUND_TASK_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET))
#define PRAGHA_IS_BACKGROUND_TASK_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET))
#define PRAGHA_BACKGROUND_TASK_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_BACKGROUND_TASK_WIDGET, PraghaBackgroundTaskWidgetClass))

GType pragha_background_task_widget_get_type (void);

typedef struct _PraghaBackgroundTaskWidget PraghaBackgroundTaskWidget;

void
pragha_background_task_widget_set_description (PraghaBackgroundTaskWidget *taskwidget,
                                               const gchar                *description);

void
pragha_background_task_widget_set_job_progress (PraghaBackgroundTaskWidget *taskwidget,
                                                gint                        job_progress);

PraghaBackgroundTaskWidget *
pragha_background_task_widget_new (const gchar  *description,
                                   const gchar  *icon_name,
                                   gint          job_count,
                                   GCancellable *cancellable);

G_END_DECLS

#endif /* PRAGHA_BACKGROUND_TASK_WIDGET_H */
