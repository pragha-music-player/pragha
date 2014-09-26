/*************************************************************************/
/* Copyright (C) 2009-2014 matias <mati86dl@gmail.com>                   */
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

#include "pragha-simple-widgets.h"

/* Create a new haeder widget to use in preferences.
 * Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan. */

gpointer sokoke_xfce_header_new(const gchar* header, const gchar *icon)
{
	GtkWidget* xfce_heading;
	GtkWidget* hbox;
	GtkWidget* vbox;
	GtkWidget* image;
	GtkWidget* label;
	GtkWidget* separator;
	gchar* markup;

	xfce_heading = gtk_event_box_new();

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	if (icon)
		image = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_DIALOG);
	else
		image = gtk_image_new_from_icon_name ("dialog-information", GTK_ICON_SIZE_DIALOG);

	label = gtk_label_new(NULL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", header);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_misc_set_alignment (GTK_MISC(label), 0, 0.5);
	g_free(markup);

	gtk_style_context_add_class (gtk_widget_get_style_context (xfce_heading),
	                             GTK_STYLE_CLASS_ENTRY);

	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(xfce_heading), hbox);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (vbox), xfce_heading, FALSE, FALSE, 0);

	separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

	return vbox;
}

/*
 * PraghaTrackProgress: An extension of GtkProgressBar reducing their default size
 */
#define PRAGHA_TYPE_TRACK_PROGRESS (pragha_track_progress_get_type())
#define PRAGHA_TRACK_PROGRESS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TRACK_PROGRESS, PraghaTrackProgress))
#define PRAGHA_TRACK_PROGRESS_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TRACK_PROGRESS, PraghaTrackProgress const))
#define PRAGHA_TRACK_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TRACK_PROGRESS, PraghaTrackProgressClass))
#define PRAGHA_IS_TRACK_PROGRESS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TRACK_PROGRESS))
#define PRAGHA_IS_TRACK_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TRACK_PROGRESS))
#define PRAGHA_TRACK_PROGRESS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TRACK_PROGRESS, PraghaTrackProgressClass))

typedef struct _PraghaTrackProgressClass PraghaTrackProgressClass;

struct _PraghaTrackProgressClass {
	GtkProgressBarClass parent_class;
};
struct _PraghaTrackProgress {
	GtkProgressBar _parent;
};
G_DEFINE_TYPE(PraghaTrackProgress, pragha_track_progress, GTK_TYPE_PROGRESS_BAR)

static void
pragha_track_progress_get_preferred_height (GtkWidget *widget,
                                            gint      *minimum,
                                            gint      *natural)
{
	if (minimum)
		*minimum = 14;
	if (natural)
		*natural = 14;
}

static void
pragha_track_progress_class_init (PraghaTrackProgressClass *class)
{
	GtkWidgetClass *widget_class;

	widget_class = GTK_WIDGET_CLASS (class);
	widget_class->get_preferred_height = pragha_track_progress_get_preferred_height;
}

static void
pragha_track_progress_init (PraghaTrackProgress *progress)
{
}

PraghaTrackProgress *
pragha_track_progress_new (void)
{
	return g_object_new (PRAGHA_TYPE_TRACK_PROGRESS, NULL);
}

/*
 * PraghaContainer: An extension of GtkContainer to expand their default size.
 */
#define PRAGHA_TYPE_CONTAINER (pragha_container_get_type())
#define PRAGHA_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_CONTAINER, PraghaContainer))
#define PRAGHA_CONTAINER_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_CONTAINER, PraghaContainer const))
#define PRAGHA_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_CONTAINER, PraghaContainerClass))
#define PRAGHA_IS_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_CONTAINER))
#define PRAGHA_IS_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_CONTAINER))
#define PRAGHA_CONTAINER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_CONTAINER, PraghaContainerClass))

typedef struct _PraghaContainerClass PraghaContainerClass;

struct _PraghaContainerClass {
	GtkBoxClass parent_class;
};
struct _PraghaContainer {
	GtkBox _parent;
};
G_DEFINE_TYPE(PraghaContainer, pragha_container, GTK_TYPE_BOX)

static void
pragha_container_get_preferred_width (GtkWidget *widget,
                                      gint      *minimum,
                                      gint      *natural)
{
	if (minimum)
		*minimum = 140;
	if (natural)
		*natural = 860;
}

static void
pragha_container_class_init (PraghaContainerClass *class)
{
	GtkWidgetClass *widget_class;

	widget_class = GTK_WIDGET_CLASS (class);
	widget_class->get_preferred_width = pragha_container_get_preferred_width;
}

static void
pragha_container_init (PraghaContainer *widget)
{
	gtk_orientable_set_orientation (GTK_ORIENTABLE (widget), GTK_ORIENTATION_HORIZONTAL);
}

PraghaContainer *
pragha_container_new (void)
{
	return g_object_new (PRAGHA_TYPE_CONTAINER, NULL);
}