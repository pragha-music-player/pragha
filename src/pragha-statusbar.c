/*************************************************************************/
/* Copyright (C) 2013 matias <mati86dl@gmail.com>			 */
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

#include "pragha-statusbar.h"

struct _PraghaStatusbarClass
{
	GtkStatusbarClass __parent__;
};

struct _PraghaStatusbar
{
	GtkStatusbar __parent__;

	guint        main_context_id;
	guint        misc_context_id;
};

G_DEFINE_TYPE (PraghaStatusbar, pragha_statusbar, GTK_TYPE_STATUSBAR)

static void
pragha_statusbar_class_init (PraghaStatusbarClass *klass)
{
}

static void
pragha_statusbar_init (PraghaStatusbar *statusbar)
{
	statusbar->main_context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Main text");
	statusbar->misc_context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Misc info text");
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

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), statusbar->main_context_id);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), statusbar->main_context_id, text);
}

/**
 * pragha_statusbar_clean_misc_text:
 * @statusbar : a #PraghaStatusbar instance.
 *
 * Clean the last misc text for @statusbar.
 **/
static void
pragha_statusbar_clean_misc_text(PraghaStatusbar *statusbar)
{
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), statusbar->misc_context_id);
}

gboolean
pragha_statusbar_clean_misc_text_idle(gpointer data)
{
	PraghaStatusbar *statusbar = data;

	pragha_statusbar_clean_misc_text(statusbar);

	return FALSE;
}

/**
 * pragha_statusbar_set_misc_text:
 * @statusbar : a #PraghaStatusbar instance.
 * @text      : the misc text to be displayed in @statusbar.
 *
 * Sets up a new misc text for @statusbar.
 **/
void
pragha_statusbar_set_misc_text (PraghaStatusbar *statusbar,
                                const gchar     *text)
{
	g_return_if_fail (PRAGHA_IS_STATUSBAR (statusbar));
	g_return_if_fail (text != NULL);

	pragha_statusbar_clean_misc_text(statusbar);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), statusbar->misc_context_id, text);

	g_timeout_add_seconds(5, pragha_statusbar_clean_misc_text_idle, statusbar);
}

/**
 * pragha_statusbar_add_widget:
 * @statusbar : a #PraghaStatusbar instance.
 * @wdget     : the widget to append in @statusbar.
 *
 * Sets up a new misc text for @statusbar.
 **/
void
pragha_statusbar_add_widget(PraghaStatusbar *statusbar,
                            GtkWidget       *widget)
{
	GtkWidget *hbox;

	g_return_if_fail (PRAGHA_IS_STATUSBAR (statusbar));
	g_return_if_fail (widget != NULL);

	hbox = gtk_statusbar_get_message_area(GTK_STATUSBAR (statusbar));

	gtk_box_pack_end (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
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
