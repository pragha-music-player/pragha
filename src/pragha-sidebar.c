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

struct _PraghaSidebar {
	GtkBox     __parent__;

	GtkWidget *container;
	GtkWidget *header;
	GtkWidget *menu_button;
	GtkWidget *close_button;
	GtkWidget *title_box;

	GtkMenu   *popup_menu;
};

G_DEFINE_TYPE(PraghaSidebar, pragha_sidebar, GTK_TYPE_BOX)

enum {
	SIGNAL_CHILDREN_CHANGED,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL] = { 0 };

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

	g_signal_emit (sidebar, signals[SIGNAL_CHILDREN_CHANGED], 0);
}

void
pragha_sidebar_remove_plugin (PraghaSidebar *sidebar,
                              GtkWidget     *widget)
{
	GList *list;
	GtkWidget *children;
	gint page;

	page = gtk_notebook_page_num (GTK_NOTEBOOK(sidebar->container), widget);

	if (page >= 0) {
		gtk_notebook_remove_page (GTK_NOTEBOOK(sidebar->container), page);
		gtk_menu_detach (sidebar->popup_menu);

		list = gtk_container_get_children (GTK_CONTAINER(sidebar->title_box));
		if (list) {
			children = list->data;
			gtk_container_remove(GTK_CONTAINER(sidebar->title_box), children);
			g_list_free(list);
		}
	}

	g_signal_emit (sidebar, signals[SIGNAL_CHILDREN_CHANGED], 0);
}

gint
pragha_sidebar_get_n_panes (PraghaSidebar *sidebar)
{
	return 	gtk_notebook_get_n_pages (GTK_NOTEBOOK(sidebar->container));
}

void
pragha_sidebar_style_position (PraghaSidebar *sidebar, GtkPositionType position)
{
	GtkWidget *parent;
	parent  = gtk_widget_get_parent (GTK_WIDGET(sidebar->close_button));
	gtk_box_reorder_child (GTK_BOX(parent),
	                       GTK_WIDGET(sidebar->close_button),
	                       (position == GTK_POS_RIGHT) ? 0 : 1);
}

/*
 * Internal Calbacks.
 */

#if !GTK_CHECK_VERSION (3, 22, 0)
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
#endif

static void
pragha_sidebar_close_button_cb (GtkWidget *widget, PraghaSidebar *sidebar)
{
	gtk_widget_hide (GTK_WIDGET(sidebar));
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
#if GTK_CHECK_VERSION (3, 22, 0)
			gtk_menu_popup_at_pointer (GTK_MENU(sidebar->popup_menu),
			                           (const GdkEvent *)event);
#else
			gtk_menu_popup(GTK_MENU(sidebar->popup_menu),
			               NULL, NULL, NULL, NULL,
			               event->button, event->time);
#endif
			ret = TRUE;
			break;
		case 1:
			if (widget == sidebar->menu_button) {
#if GTK_CHECK_VERSION (3, 22, 0)
				gtk_menu_popup_at_widget (GTK_MENU(sidebar->popup_menu), widget,
				                          GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST,
				                          (const GdkEvent *)event);
#else
				gtk_menu_popup(GTK_MENU(sidebar->popup_menu),
				                NULL, NULL,
				                (GtkMenuPositionFunc) pragha_sidebar_menu_position,
				                widget,
				                0,
				                gtk_get_current_event_time());
#endif
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

static void
pragha_sidebar_set_tiny_button (GtkWidget *button)
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
	                                 " padding: 1px;}",
	                                 -1, NULL);
	gtk_style_context_add_provider (gtk_widget_get_style_context (button),
	                                GTK_STYLE_PROVIDER (provider),
	                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_widget_set_name (button, "s-tiny-button");
	g_object_unref (provider);
}

static GtkWidget *
praga_sidebar_menu_button_new (PraghaSidebar *sidebar)
{
	GtkWidget *button, *hbox, *arrow;

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

	arrow = gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_MENU);

	gtk_box_pack_start (GTK_BOX(hbox),
	                    sidebar->title_box,
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(hbox),
	                    arrow,
	                    FALSE, FALSE, 0);

	gtk_widget_set_valign (GTK_WIDGET(sidebar->title_box), GTK_ALIGN_CENTER);

	gtk_container_add (GTK_CONTAINER(button), hbox);

	g_signal_connect(G_OBJECT(button),
	                 "button-press-event",
	                 G_CALLBACK(pragha_sidebar_right_click_cb),
	                 sidebar);

	return button;
}

static GtkWidget *
pragha_sidebar_close_button_new(PraghaSidebar *sidebar)
{
	GtkWidget *button;
	GIcon *icon = NULL;

 	const gchar *fallback_icons[] = {
		"view-left-close",
		"tab-close",
		"window-close",
		NULL,
	};

	button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION (3, 20, 0)
	gtk_widget_set_focus_on_click (button, FALSE);
#else
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
#endif
	pragha_sidebar_set_tiny_button (button);
	gtk_widget_set_valign (button, GTK_ALIGN_CENTER);

	icon = g_themed_icon_new_from_names ((gchar **)fallback_icons, -1);
	gtk_button_set_image (GTK_BUTTON (button),
		gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_MENU));
	g_object_unref (icon);

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

static void
pragha_sidebar_finalize (GObject *object)
{
	//PraghaSidebar *sidebar = PRAGHA_SIDEBAR (object);
	(*G_OBJECT_CLASS (pragha_sidebar_parent_class)->finalize) (object);
}

static void
pragha_sidebar_init (PraghaSidebar *sidebar)
{
	gtk_orientable_set_orientation (GTK_ORIENTABLE (sidebar),
	                                GTK_ORIENTATION_VERTICAL);

	gtk_box_set_spacing (GTK_BOX(sidebar), 2);

	sidebar->title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	sidebar->menu_button = praga_sidebar_menu_button_new (sidebar);
	sidebar->close_button = pragha_sidebar_close_button_new (sidebar);

	sidebar->header = pragha_sidebar_header_new (sidebar);
	sidebar->container = pragha_sidebar_container_new (sidebar);

	sidebar->popup_menu = NULL;

	gtk_box_pack_start (GTK_BOX(sidebar), sidebar->header,
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(sidebar), sidebar->container,
	                    TRUE, TRUE, 0);

	gtk_widget_show_all (GTK_WIDGET(sidebar->header));
	gtk_widget_show_all (GTK_WIDGET(sidebar->container));
}

static void
pragha_sidebar_class_init (PraghaSidebarClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = pragha_sidebar_finalize;

	signals[SIGNAL_CHILDREN_CHANGED] =
		g_signal_new ("children-changed",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSidebarClass, children_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
}

PraghaSidebar *
pragha_sidebar_new (void)
{
	return g_object_new (PRAGHA_TYPE_SIDEBAR, NULL);
}
