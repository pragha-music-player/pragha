/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __PRAGHA_PROGRESS_BAR_H__
#define __PRAGHA_PROGRESS_BAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_PROGRESS_BAR            (pragha_progress_bar_get_type ())
#define PRAGHA_PROGRESS_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PROGRESS_BAR, PraghaProgressBar))
#define PRAGHA_PROGRESS_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PROGRESS_BAR, PraghaProgressBarClass))
#define PRAGHA_IS_PROGRESS_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PROGRESS_BAR))
#define PRAGHA_IS_PROGRESS_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PROGRESS_BAR))
#define PRAGHA_PROGRESS_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PROGRESS_BAR, PraghaProgressBarClass))

typedef struct _PraghaProgressBar           PraghaProgressBar;
typedef struct _PraghaProgressBarPrivate    PraghaProgressBarPrivate;
typedef struct _PraghaProgressBarClass      PraghaProgressBarClass;

struct _PraghaProgressBar
{
	GtkWidget parent;

	/*< private >*/
	PraghaProgressBarPrivate *priv;
};

struct _PraghaProgressBarClass
{
	GtkWidgetClass parent_class;
};


GType      pragha_progress_bar_get_type             (void) G_GNUC_CONST;
GtkWidget* pragha_progress_bar_new                  (void);

void       pragha_progress_bar_set_fraction         (PraghaProgressBar *pbar,
                                                     gdouble            fraction);

gdouble    pragha_progress_bar_get_fraction         (PraghaProgressBar *pbar);

G_END_DECLS

#endif /* __PRAGHA_PROGRESS_BAR_H__ */
