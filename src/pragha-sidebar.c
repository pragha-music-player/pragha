/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>			 */
/*									 */
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

/*
 * Public Api.
 */

void
pragha_sidebar_header_set_text(PraghaSidebar *sidebar, const gchar *text)
{
	gtk_label_set_text (GTK_LABEL(sidebar->label), text);
}

void
pragha_sidebar_atach_menu(PraghaSidebar *sidebar, GtkMenu *menu)
{
	gtk_menu_attach_to_widget(GTK_MENU(menu), sidebar->menu_button, NULL);
}

void
pragha_sidebar_add_widget(PraghaSidebar *sidebar, GtkWidget *widget)
{
	gtk_notebook_append_page(GTK_NOTEBOOK(sidebar->container),
	                         widget,
	                         NULL);
}

/*
 * Internal Calbacks.
 */

static void
pragha_sidebar_close (GtkWidget *widget, struct con_win *cwin)
{
	GtkAction *action;
	action = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Lateral panel");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION(action), FALSE);
}

/**
 * Construction:
 **/

GtkWidget *
praga_sidebar_label_new()
{
	GtkWidget *label;

	label = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	return label;
}

GtkWidget *
praga_sidebar_menu_button_new(PraghaSidebar *sidebar, struct con_win *cwin)
{
	GtkWidget *button, *hbox, *arrow;

	hbox = gtk_hbox_new(FALSE, 0);

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

	arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);

	gtk_box_pack_start(GTK_BOX(hbox),
			   sidebar->label,
			   TRUE,
			   TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(hbox),
			   arrow,
			   FALSE,
			   FALSE,
			   0);

	gtk_container_add (GTK_CONTAINER(button), hbox);

	/*g_signal_connect(G_OBJECT(button),
			 "button-press-event",
			 G_CALLBACK(library_page_right_click_cb),
			 cwin);*/

	return button;
}

GtkWidget *
pragha_sidebar_close_button_new(PraghaSidebar *sidebar, struct con_win *cwin)
{
	GtkWidget *button, *image;
    
	button = gtk_button_new ();
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_container_add (GTK_CONTAINER (button), image);

	g_signal_connect(G_OBJECT (button),
	                 "clicked",
	                 G_CALLBACK(pragha_sidebar_close),
	                 cwin);

	return button;
}

GtkWidget *
pragha_sidebar_header_new(PraghaSidebar *sidebar)
{
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox),
	                   sidebar->menu_button,
	                   TRUE,
	                   TRUE,
	                   0);
	gtk_box_pack_start(GTK_BOX(hbox),
	                   sidebar->close_button,
	                   FALSE,
	                   FALSE,
	                   0);
	return hbox;
}

GtkWidget *
pragha_sidebar_container_new(PraghaSidebar *sidebar)
{
	GtkWidget *notebook;

	notebook = gtk_notebook_new();

	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_popup_disable(GTK_NOTEBOOK(notebook));

	return notebook;
}

GtkWidget *
pragha_sidebar_widget_new(PraghaSidebar *sidebar)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 2);

	gtk_box_pack_start(GTK_BOX(vbox),
	                   sidebar->header,
	                   FALSE,
	                   FALSE,
	                   0);
	gtk_box_pack_start(GTK_BOX(vbox),
	                   sidebar->container,
	                   TRUE,
	                   TRUE,
	                   0);

	return vbox;
}

void
pragha_sidebar_free(PraghaSidebar *sidebar)
{
	g_slice_free(PraghaSidebar, sidebar);
}

PraghaSidebar *
pragha_sidebar_new(struct con_win *cwin)
{
	PraghaSidebar *sidebar;

	sidebar = g_slice_new0(PraghaSidebar);

	sidebar->label = praga_sidebar_label_new();
	sidebar->menu_button = praga_sidebar_menu_button_new(sidebar, cwin);
	sidebar->close_button = pragha_sidebar_close_button_new(sidebar, cwin);

	sidebar->header = pragha_sidebar_header_new(sidebar);
	sidebar->container = pragha_sidebar_container_new(sidebar);

	sidebar->widget = pragha_sidebar_widget_new(sidebar);

	return sidebar;
}