/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

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

GtkWidget *
pragha_hig_workarea_table_add_section_title(GtkWidget *table, guint *row, const char *section_title)
{
	GtkWidget *section_label;

	section_label = gtk_label_new(section_title);

	gtk_misc_set_alignment(GTK_MISC(section_label), 0.0, 0.5);
	gtk_label_set_attribute_bold(GTK_LABEL(section_label));

	gtk_table_attach(GTK_TABLE(table), section_label,
			 0, 2, *row, *row + 1,
			 GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			 0, 0);
	++ * row;

	return section_label;
}

void
pragha_hig_workarea_table_add_wide_control(GtkWidget *table, guint *row, GtkWidget *widget)
{
	gtk_table_attach(GTK_TABLE(table), widget, 0, 2, *row, *row + 1, GTK_FILL, 0, 12, 0);

	++ * row;
}

void
pragha_hig_workarea_table_add_wide_tall_control(GtkWidget *table, guint *row, GtkWidget *widget)
{
	gtk_table_attach(GTK_TABLE(table), widget,
			 0, 2, *row, *row + 1,
			 GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL,
			 12, 0);

	++ * row;
}

void
pragha_hig_workarea_table_add_row(GtkWidget *table, guint *row, GtkWidget *label, GtkWidget *control)
{
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

	gtk_table_attach(GTK_TABLE(table), label, 0, 1, *row, *row + 1, GTK_FILL, GTK_FILL, 12, 0);
	gtk_table_attach(GTK_TABLE(table), control, 1, 2, *row, *row + 1, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);

	++ * row;
}

GtkWidget *
pragha_hig_workarea_table_new()
{
	GtkWidget *table;

	table = gtk_table_new(1, 2, FALSE);

	gtk_container_set_border_width(GTK_CONTAINER(table), 12);
	gtk_table_set_col_spacing(GTK_TABLE(table), 0, 12);

	gtk_table_set_row_spacings(GTK_TABLE(table), 6);

	return table;
}

void
pragha_hig_workarea_table_finish(GtkWidget *table, guint *row)
{
	gtk_table_resize(GTK_TABLE(table), *row, 2);
}