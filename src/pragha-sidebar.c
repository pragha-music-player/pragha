/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>                        */
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

#include "pragha-sidebar.h"
#include "pragha-preferences.h"

struct _PraghaSidebar {
	GtkWidget *widget;
	GtkWidget *container;
	GtkWidget *header;
	GtkWidget *menu_button;
	GtkWidget *close_button;
	GtkWidget *title_box;
	GtkMenu *popup_menu;
};

/*
 * Public Api.
 */

void
pragha_sidebar_attach_plugin (PraghaSidebar *sidebar,
                              GtkWidget     *widget,
                              GtkWidget     *title,
                              GtkMenu       *popup_menu)
{
	if (!widget || !title)
		return;

	gtk_notebook_insert_page (GTK_NOTEBOOK(sidebar->container),
	                          widget,
	                          NULL,
	                          0);

	gtk_container_add (GTK_CONTAINER(sidebar->title_box), title);

	if (popup_menu) {
		gtk_menu_attach_to_widget(GTK_MENU(popup_menu), title, NULL);
		sidebar->popup_menu = popup_menu;
	}
	gtk_widget_show_all (title);
}

GtkWidget *
pragha_sidebar_get_widget(PraghaSidebar *sidebar)
{
	return sidebar->widget;
}

/*
 * Internal Calbacks.
 */

static void
pragha_sidebar_menu_position (GtkMenu  *menu,
                              gint     *x,
                              gint     *y,
                              gboolean *push_in,
                              gpointer  user_data)
{
	GtkWidget *widget;
	GtkAllocation allocation;
	GtkRequisition requisition;
	gint menu_xpos, menu_ypos;

	widget = GTK_WIDGET (user_data);

	gtk_widget_get_preferred_size (GTK_WIDGET(menu), &requisition, NULL);

	gdk_window_get_origin (gtk_widget_get_window(widget), &menu_xpos, &menu_ypos);

	gtk_widget_get_allocation(widget, &allocation);

	menu_xpos += allocation.x;
	menu_ypos += allocation.y;

	if (menu_ypos > gdk_screen_get_height (gtk_widget_get_screen (widget)) / 2)
		menu_ypos -= requisition.height;
	else
		menu_ypos += allocation.height;

	*x = menu_xpos;
	*y = menu_ypos - 5;

	*push_in = TRUE;
}

static void
pragha_sidebar_close_button_cb (GtkWidget *widget, PraghaSidebar *sidebar)
{
	gtk_widget_hide(GTK_WIDGET(sidebar->widget));
}

static gboolean
pragha_sidebar_right_click_cb(GtkWidget *widget,
                              GdkEventButton *event,
                              PraghaSidebar *sidebar)
{
	gboolean ret = FALSE;

	if(!sidebar->popup_menu)
		return FALSE;

	if(!gtk_widget_get_sensitive(gtk_notebook_get_nth_page (GTK_NOTEBOOK(sidebar->container), 0)))
		return FALSE;

	switch(event->button) {
		case 3:
			gtk_menu_popup(GTK_MENU(sidebar->popup_menu),
			               NULL, NULL, NULL, NULL,
			               event->button, event->time);
			ret = TRUE;
			break;
		case 1:
			if (widget == sidebar->menu_button) {
				gtk_menu_popup(GTK_MENU(sidebar->popup_menu),
				                NULL, NULL,
				                (GtkMenuPositionFunc) pragha_sidebar_menu_position,
				                widget,
				                0,
				                gtk_get_current_event_time());
				ret = TRUE;
			}
			break;
		default:
			break;
	}

	return ret;
}

/**
 * Construction:
 **/


GtkWidget *
praga_sidebar_menu_button_new (PraghaSidebar *sidebar)
{
	GtkWidget *button, *hbox, *arrow;

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

	arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);

	gtk_box_pack_start (GTK_BOX(hbox),
	                    sidebar->title_box,
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(hbox),
	                    arrow,
	                    FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER(button), hbox);

	g_signal_connect(G_OBJECT(button),
	                 "button-press-event",
	                 G_CALLBACK(pragha_sidebar_right_click_cb),
	                 sidebar);

	return button;
}

GtkWidget *
pragha_sidebar_close_button_new(PraghaSidebar *sidebar)
{
	GtkWidget *button, *image;
    
	button = gtk_button_new ();
	image = gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_container_add (GTK_CONTAINER (button), image);

	g_signal_connect(G_OBJECT (button),
	                 "clicked",
	                 G_CALLBACK(pragha_sidebar_close_button_cb),
	                 sidebar);

	return button;
}

GtkWidget *
pragha_sidebar_header_new(PraghaSidebar *sidebar)
{
	GtkWidget *hbox;

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

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

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

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

	gtk_widget_show_all(vbox);

	return vbox;
}

void
pragha_sidebar_free(PraghaSidebar *sidebar)
{
	g_slice_free(PraghaSidebar, sidebar);
}

PraghaSidebar *
pragha_sidebar_new(void)
{
	PraghaSidebar *sidebar;
	PraghaPreferences *preferences;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	sidebar = g_slice_new0(PraghaSidebar);

	sidebar->title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	sidebar->menu_button = praga_sidebar_menu_button_new(sidebar);
	sidebar->close_button = pragha_sidebar_close_button_new(sidebar);

	sidebar->header = pragha_sidebar_header_new(sidebar);
	sidebar->container = pragha_sidebar_container_new(sidebar);

	sidebar->widget = pragha_sidebar_widget_new(sidebar);

	sidebar->popup_menu = NULL;

	preferences = pragha_preferences_get();
	g_object_bind_property (preferences, "lateral-panel",
	                        sidebar->widget, "visible",
	                        binding_flags);
	g_object_unref (G_OBJECT(preferences));

	return sidebar;
}