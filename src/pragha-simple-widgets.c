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

#include "pragha-simple-widgets.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

/* Create a new haeder widget to use in preferences.
 * Based in Midori Web Browser. Copyright (C) 2007 Christian Dywan. */

gpointer sokoke_xfce_header_new(const gchar* header, const gchar *icon_name)
{
	GtkWidget* xfce_heading;
	GtkWidget* hbox;
	GtkWidget* vbox;
	GdkPixbuf* icon;
	GtkWidget* image;
	GtkWidget* label;
	GtkWidget* separator;
	gchar* markup;
	gint width = 1, height = 1;

	xfce_heading = gtk_event_box_new();

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &width, &height);
	icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
	                                 icon_name ? icon_name : "dialog-information",
	                                 width,
	                                 GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
	image = gtk_image_new_from_pixbuf (GDK_PIXBUF(icon));
	g_object_unref (icon);

	label = gtk_label_new(NULL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);

	markup = g_strdup_printf("<span size='large' weight='bold'>%s</span>", header);
	gtk_label_set_markup(GTK_LABEL(label), markup);
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
		*natural = 1600;
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

/*
 * PraghaToolbarButton
 */
#define PRAGHA_TYPE_TOOLBAR_BUTTON (pragha_toolbar_button_get_type())
#define PRAGHA_TOOLBAR_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TOOLBAR_BUTTON, PraghaToolbarButton))
#define PRAGHA_TOOLBAR_BUTTON_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TOOLBAR_BUTTON, PraghaToolbarButton const))
#define PRAGHA_TOOLBAR_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TOOLBAR_BUTTON, PraghaToolbarButtonClass))
#define PRAGHA_IS_TOOLBAR_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TOOLBAR_BUTTON))
#define PRAGHA_IS_TOOLBAR_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TOOLBAR_BUTTON))
#define PRAGHA_TOOLBAR_BUTTON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TOOLBAR_BUTTON, PraghaToolbarButtonClass))

typedef struct _PraghaToolbarButtonClass PraghaToolbarButtonClass;

struct _PraghaToolbarButtonClass {
	GtkButtonClass parent_class;
};

struct _PraghaToolbarButton {
	GtkButton   __parent;

	gchar       *icon_name;
	GtkIconSize  icon_size;
};

enum
{
	PROP_0,
	PROP_ICON_NAME,
	PROP_ICON_SIZE
};

G_DEFINE_TYPE(PraghaToolbarButton, pragha_toolbar_button, GTK_TYPE_BUTTON)

static void
pragha_toolbar_button_update_icon (PraghaToolbarButton *button)
{
	gtk_button_set_image (GTK_BUTTON(button),
		gtk_image_new_from_icon_name (button->icon_name,
		                              button->icon_size));
}

void
pragha_toolbar_button_set_icon_name (PraghaToolbarButton *button, const gchar *icon_name)
{
	if (g_strcmp0(button->icon_name, icon_name)) {
		if (button->icon_name)
			g_free (button->icon_name);
		button->icon_name = g_strdup (icon_name);

		pragha_toolbar_button_update_icon (button);
	}
}

void
pragha_toolbar_button_set_icon_size (PraghaToolbarButton *button, GtkIconSize icon_size)
{
	if (button->icon_size != icon_size) {
		button->icon_size = icon_size;
		pragha_toolbar_button_update_icon (button);
	}
}

static void
pragha_toolbar_button_set_property (GObject       *object,
                                    guint          prop_id,
                                    const GValue  *value,
                                    GParamSpec    *pspec)
{
	PraghaToolbarButton *button = PRAGHA_TOOLBAR_BUTTON (object);

	switch (prop_id)
	{
		case PROP_ICON_NAME:
			pragha_toolbar_button_set_icon_name (button, g_value_get_string (value));
			break;
		case PROP_ICON_SIZE:
			pragha_toolbar_button_set_icon_size (button, g_value_get_enum (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_toolbar_button_get_property (GObject     *object,
                                    guint        prop_id,
                                    GValue      *value,
                                    GParamSpec  *pspec)
{
	PraghaToolbarButton *button = PRAGHA_TOOLBAR_BUTTON (object);
	switch (prop_id)
	{
		case PROP_ICON_NAME:
			g_value_set_string (value, button->icon_name);
			break;
		case PROP_ICON_SIZE:
			g_value_set_enum (value, button->icon_size);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_toolbar_button_finalize (GObject *object)
{
	PraghaToolbarButton *button = PRAGHA_TOOLBAR_BUTTON (object);
	if (button->icon_name) {
		g_free (button->icon_name);
		button->icon_name = NULL;
	}
	(*G_OBJECT_CLASS (pragha_toolbar_button_parent_class)->finalize) (object);
}

static void
pragha_toolbar_button_class_init (PraghaToolbarButtonClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  	gobject_class->set_property = pragha_toolbar_button_set_property;
	gobject_class->get_property = pragha_toolbar_button_get_property;
	gobject_class->finalize = pragha_toolbar_button_finalize;

	g_object_class_install_property (gobject_class, PROP_ICON_NAME,
	                                 g_param_spec_string ("icon-name",
	                                                      "Icon Name",
	                                                      "The name of the icon from the icon theme",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class, PROP_ICON_SIZE,
	                                 g_param_spec_enum ("icon-size",
	                                                    "Icon size",
	                                                     "The icon size",
	                                                     GTK_TYPE_ICON_SIZE,
	                                                     GTK_ICON_SIZE_SMALL_TOOLBAR,
	                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
pragha_toolbar_button_init (PraghaToolbarButton *widget)
{
	gtk_button_set_relief (GTK_BUTTON(widget), GTK_RELIEF_NONE);
	gtk_widget_show_all (GTK_WIDGET(widget));
}

PraghaToolbarButton *
pragha_toolbar_button_new (const gchar *icon_name)
{
	PraghaToolbarButton *button;
	GtkWidget *image;

	image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
	button =  g_object_new (PRAGHA_TYPE_TOOLBAR_BUTTON,
	                        "image", image,
	                        "icon-name", icon_name,
	                        "icon-size", GTK_ICON_SIZE_LARGE_TOOLBAR,
	                        "valign", GTK_ALIGN_CENTER,
	                        NULL);

	return button;
}

/*
 * PraghaTogleButton
 */
#define PRAGHA_TYPE_TOGGLE_BUTTON (pragha_toggle_button_get_type())
#define PRAGHA_TOGGLE_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TOGGLE_BUTTON, PraghaToggleButton))
#define PRAGHA_TOGGLE_BUTTON_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_TOGGLE_BUTTON, PraghaToggleButton const))
#define PRAGHA_TOGGLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_TOGGLE_BUTTON, PraghaToggleButtonClass))
#define PRAGHA_IS_TOGGLE_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_TOGGLE_BUTTON))
#define PRAGHA_IS_TOGGLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_TOGGLE_BUTTON))
#define PRAGHA_TOGGLE_BUTTON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_TOGGLE_BUTTON, PraghaToggleButtonClass))

typedef struct _PraghaToggleButtonClass PraghaToggleButtonClass;

struct _PraghaToggleButtonClass {
	GtkToggleButtonClass parent_class;
};

struct _PraghaToggleButton {
	GtkToggleButton   __parent;

	gchar       *icon_name;
	GtkIconSize  icon_size;
};

G_DEFINE_TYPE(PraghaToggleButton, pragha_toggle_button, GTK_TYPE_TOGGLE_BUTTON)

static void
pragha_toggle_button_update_icon (PraghaToggleButton *button)
{
	gtk_button_set_image (GTK_BUTTON(button),
		gtk_image_new_from_icon_name (button->icon_name,
		                              button->icon_size));
}

void
pragha_toggle_button_set_icon_name (PraghaToggleButton *button, const gchar *icon_name)
{
	if (g_strcmp0(button->icon_name, icon_name)) {
		if (button->icon_name)
			g_free (button->icon_name);
		button->icon_name = g_strdup (icon_name);

		pragha_toggle_button_update_icon (button);
	}
}

void
pragha_toggle_button_set_icon_size (PraghaToggleButton *button, GtkIconSize icon_size)
{
	if (button->icon_size != icon_size) {
		button->icon_size = icon_size;
		pragha_toggle_button_update_icon (button);
	}
}

static void
pragha_toggle_button_set_property (GObject       *object,
                                   guint          prop_id,
                                   const GValue  *value,
                                   GParamSpec    *pspec)
{
	PraghaToggleButton *button = PRAGHA_TOGGLE_BUTTON (object);

	switch (prop_id)
	{
		case PROP_ICON_NAME:
			pragha_toggle_button_set_icon_name (button, g_value_get_string (value));
			break;
		case PROP_ICON_SIZE:
			pragha_toggle_button_set_icon_size (button, g_value_get_enum (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_toggle_button_get_property (GObject     *object,
                                   guint        prop_id,
                                   GValue      *value,
                                   GParamSpec  *pspec)
{
	PraghaToggleButton *button = PRAGHA_TOGGLE_BUTTON (object);
	switch (prop_id)
	{
		case PROP_ICON_NAME:
			g_value_set_string (value, button->icon_name);
			break;
		case PROP_ICON_SIZE:
			g_value_set_enum (value, button->icon_size);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
pragha_toggle_button_finalize (GObject *object)
{
	PraghaToggleButton *button = PRAGHA_TOGGLE_BUTTON (object);
	if (button->icon_name) {
		g_free (button->icon_name);
		button->icon_name = NULL;
	}
	(*G_OBJECT_CLASS (pragha_toggle_button_parent_class)->finalize) (object);
}

static void
pragha_toggle_button_class_init (PraghaToggleButtonClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  	gobject_class->set_property = pragha_toggle_button_set_property;
	gobject_class->get_property = pragha_toggle_button_get_property;
	gobject_class->finalize = pragha_toggle_button_finalize;

	g_object_class_install_property (gobject_class, PROP_ICON_NAME,
	                                 g_param_spec_string ("icon-name",
	                                                      "Icon Name",
	                                                      "The name of the icon from the icon theme",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (gobject_class, PROP_ICON_SIZE,
	                                 g_param_spec_enum ("icon-size",
	                                                    "Icon size",
	                                                     "The icon size",
	                                                     GTK_TYPE_ICON_SIZE,
	                                                     GTK_ICON_SIZE_SMALL_TOOLBAR,
	                                                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
pragha_toggle_button_init (PraghaToggleButton *widget)
{
	gtk_button_set_relief (GTK_BUTTON(widget), GTK_RELIEF_NONE);
	gtk_widget_show_all (GTK_WIDGET(widget));
}

PraghaToggleButton *
pragha_toggle_button_new (const gchar *icon_name)
{
	PraghaToggleButton *button;
	GtkWidget *image;

	image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
	button =  g_object_new (PRAGHA_TYPE_TOGGLE_BUTTON,
	                        "image", image,
	                        "icon-name", icon_name,
	                        "icon-size", GTK_ICON_SIZE_LARGE_TOOLBAR,
	                        "valign", GTK_ALIGN_CENTER,
	                        NULL);

	return button;
}