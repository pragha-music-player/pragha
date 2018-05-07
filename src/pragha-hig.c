/*************************************************************************/
/* Copyright (C) 2012-2018 matias <mati86dl@gmail.com>                   */
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

#include "pragha-hig.h"

#if !GTK_CHECK_VERSION (3, 12, 0)
#define gtk_widget_set_margin_start gtk_widget_set_margin_left
#endif

void
gtk_label_set_attribute_bold(GtkLabel *label)
{
	PangoAttrList *Bold = pango_attr_list_new();
	PangoAttribute *Attribute = NULL;
	Attribute = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
	pango_attr_list_insert(Bold, Attribute);

	gtk_label_set_attributes(label, Bold);

	pango_attr_list_unref(Bold);
}

void
pragha_hig_set_tiny_button (GtkWidget *button)
{
	GtkCssProvider *provider;
	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider,
	                                 "#s-tiny-button {\n"
#if GTK_CHECK_VERSION (3, 14, 0)
	                                 " margin : 0px;\n"
	                                 " min-width: 10px; \n"
	                                 " min-height: 10px; \n"
#else
	                                 " -GtkButton-default-border : 0px;\n"
	                                 " -GtkButton-default-outside-border : 0px;\n"
	                                 " -GtkButton-inner-border: 0px;\n"
	                                 " -GtkWidget-focus-line-width: 0px;\n"
	                                 " -GtkWidget-focus-padding: 0px;\n"
#endif
	                                 " padding: 0px;}",
	                                 -1, NULL);
	gtk_style_context_add_provider (gtk_widget_get_style_context (button),
	                                GTK_STYLE_PROVIDER (provider),
	                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_widget_set_name (button, "s-tiny-button");
	g_object_unref (provider);
}

GtkWidget *
pragha_hig_workarea_table_add_section_title(GtkWidget *table, guint *row, const char *section_title)
{
	GtkWidget *section_label;

	section_label = gtk_label_new(section_title);

	gtk_misc_set_alignment(GTK_MISC(section_label), 0.0, 0.5);
	gtk_label_set_attribute_bold(GTK_LABEL(section_label));

	gtk_grid_attach (GTK_GRID(table), section_label, 0, *row, 2, 1);
	++ * row;

	return section_label;
}

void
pragha_hig_workarea_table_add_wide_control(GtkWidget *table, guint *row, GtkWidget *widget)
{
	gtk_grid_attach (GTK_GRID(table), widget, 0, *row, 2, 1);
	gtk_widget_set_margin_start (GTK_WIDGET(widget), 12);

	gtk_widget_set_hexpand (GTK_WIDGET(widget), TRUE);

	++ * row;
}

void
pragha_hig_workarea_table_add_wide_tall_control(GtkWidget *table, guint *row, GtkWidget *widget)
{
	gtk_grid_attach (GTK_GRID(table), widget, 0, *row, 2, 1);
	gtk_widget_set_margin_start (GTK_WIDGET(widget), 12);

	gtk_widget_set_hexpand (widget, TRUE);
	gtk_widget_set_vexpand (widget, TRUE);

	++ * row;
}

void
pragha_hig_workarea_table_add_row(GtkWidget *table, guint *row, GtkWidget *label, GtkWidget *control)
{
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_widget_set_margin_start (GTK_WIDGET(label), 12);

	gtk_grid_attach (GTK_GRID(table), label, 0, *row, 1, 1);
	gtk_grid_attach (GTK_GRID(table), control, 1, *row, 1, 1);

	gtk_widget_set_hexpand (control, TRUE);

	++ * row;
}

GtkWidget *
pragha_hig_workarea_table_new()
{
	GtkWidget *table;

	table = gtk_grid_new ();

	gtk_container_set_border_width(GTK_CONTAINER(table), 12);

	gtk_grid_set_row_spacing (GTK_GRID(table), 12);
	gtk_grid_set_column_spacing (GTK_GRID(table), 6);

	return table;
}
