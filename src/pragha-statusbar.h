/*************************************************************************/
/* Copyright (C) 2013-2016 matias <mati86dl@gmail.com>                   */
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


#ifndef __PRAGHA_STATUSBAR_H__
#define __PRAGHA_STATUSBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _PraghaStatusbarClass PraghaStatusbarClass;
typedef struct _PraghaStatusbar      PraghaStatusbar;

#define PRAGHA_TYPE_STATUSBAR             (pragha_statusbar_get_type ())
#define PRAGHA_STATUSBAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_STATUSBAR, PraghaStatusbar))
#define PRAGHA_STATUSBAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_STATUSBAR, PraghaStatusbarClass))
#define PRAGHA_IS_STATUSBAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_STATUSBAR))
#define PRAGHA_IS_STATUSBAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_STATUSBAR))
#define PRAGHA_STATUSBAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_STATUSBAR, PraghaStatusbarClass))

void
pragha_statusbar_set_main_text (PraghaStatusbar *statusbar,
                                const gchar     *text);

void
pragha_statusbar_set_misc_text (PraghaStatusbar *statusbar,
                                const gchar     *text);

void
pragha_statusbar_add_widget(PraghaStatusbar *statusbar,
                            GtkWidget       *widget);

void
pragha_statusbar_add_task_widget    (PraghaStatusbar *statusbar,
                                     GtkWidget       *widget);

void
pragha_statusbar_remove_task_widget (PraghaStatusbar *statusbar,
                                     GtkWidget       *widget);


GType      pragha_statusbar_get_type    (void) G_GNUC_CONST;

PraghaStatusbar *
pragha_statusbar_get (void);

G_END_DECLS;

#endif /* !__PRAGHA_STATUSBAR_H__ */

