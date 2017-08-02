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

#ifndef PRAGHA_BACKGROUND_TASK_BAR_H
#define PRAGHA_BACKGROUND_TASK_BAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _PraghaBackgroundTaskBar PraghaBackgroundTaskBar;

void
pragha_background_task_bar_prepend_widget (PraghaBackgroundTaskBar *taskbar,
                                           GtkWidget               *widget);

void
pragha_background_task_bar_remove_widget  (PraghaBackgroundTaskBar *taskbar,
                                           GtkWidget               *widget);

PraghaBackgroundTaskBar *
pragha_background_task_bar_new            (void);

G_END_DECLS

#endif /* PRAGHA_BACKGROUND_TASK_BAR_H */
