/*************************************************************************/
/* Copyright (C) 2013-2019 matias <mati86dl@gmail.com>                   */
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

#include "pragha-statusbar.h"
#include "pragha-background-task-bar.h"

#include "pragha-preferences.h"

struct _PraghaStatusbarClass
{
	GtkStatusbarClass __parent__;
};

struct _PraghaStatusbar
{
	GtkBox    __parent__;

	GtkWidget *label;
};

G_DEFINE_TYPE (PraghaStatusbar, pragha_statusbar, GTK_TYPE_BOX)

static void
pragha_statusbar_class_init (PraghaStatusbarClass *klass)
{
}

static void
pragha_statusbar_init (PraghaStatusbar *statusbar)
{
	GtkStyleContext *context;

	statusbar->label = gtk_label_new (NULL);
	g_object_set (statusbar->label,
	              "margin-top", 2,
	              "margin-bottom", 2,
	              "margin-start", 12,
	              "margin-end", 12,
	              NULL);

	gtk_container_add(GTK_CONTAINER(statusbar), statusbar->label);

	context = gtk_widget_get_style_context (GTK_WIDGET (statusbar));
	gtk_style_context_add_class (context, "floating-bar");

	gtk_widget_show_all (GTK_WIDGET(statusbar));
}

/**
 * pragha_statusbar_set_main_text:
 * @statusbar : a #PraghaStatusbar instance.
 * @text      : the main text to be displayed in @statusbar.
 *
 * Sets up a new main text for @statusbar.
 **/
void
pragha_statusbar_set_main_text (PraghaStatusbar *statusbar,
                                const gchar     *text)
{
	g_return_if_fail (PRAGHA_IS_STATUSBAR (statusbar));
	g_return_if_fail (text != NULL);

	gtk_label_set_text (GTK_LABEL (statusbar->label), text);
}

/**
 * pragha_statusbar_get:
 *
 * Queries the global #GtkStatusbar instance, which is shared
 * by all modules. The function automatically takes a reference
 * for the caller, so you'll need to call g_object_unref() when
 * you're done with it.
 *
 * Return value: the global #GtkStatusbar instance.
 **/

PraghaStatusbar *
pragha_statusbar_get (void)
{
	static PraghaStatusbar *statusbar = NULL;

	if (G_UNLIKELY (statusbar == NULL)) {
		statusbar = g_object_new(PRAGHA_TYPE_STATUSBAR, NULL);
		g_object_add_weak_pointer(G_OBJECT (statusbar),
		                          (gpointer) &statusbar);
	}
	else {
		g_object_ref (G_OBJECT(statusbar));
	}
	return statusbar;
}
