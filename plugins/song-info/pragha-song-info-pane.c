/*************************************************************************/
/* Copyright (C) 2011-2014 matias <mati86dl@gmail.com>                   */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glyr/glyr.h>

#include "pragha-song-info-pane.h"

#include "src/pragha-utils.h"

struct _PraghaSonginfoPane {
	GtkScrolledWindow  parent;

	/* Text widget */
	GtkWidget         *text_view;

	/* Info that show thde pane */
	GLYR_GET_TYPE      info_type;

	/* Sidebar widgets */
	GtkWidget         *pane_title;
	GtkUIManager      *context_menu;
};

G_DEFINE_TYPE(PraghaSonginfoPane, pragha_songinfo_pane, GTK_TYPE_SCROLLED_WINDOW)

/*
 * Menus definitions
 *
 **/
static void pragha_songinfo_pane_show_artist_info_action (GtkAction *action, PraghaSonginfoPane *pane);
static void pragha_songinfo_pane_show_lyrics_action      (GtkAction *action, PraghaSonginfoPane *pane);

gchar *songinfo_pane_context_menu_xml = "<ui> \
	<popup>                                   \
	<menuitem action=\"Artist info\"/>        \
	<menuitem action=\"Lyrics\"/>             \
	</popup>                                  \
	</ui>";

GtkActionEntry songinfo_pane_context_aentries[] = {
	{"Artist info", NULL, N_("Artist info"),
	 "", "Artist info", G_CALLBACK(pragha_songinfo_pane_show_artist_info_action)},
	{"Lyrics", NULL, N_("Lyrics"),
	 "", "Lyrics", G_CALLBACK(pragha_songinfo_pane_show_lyrics_action)}
};

/*
 * Public Api
 */

void
pragha_songinfo_pane_set_text (PraghaSonginfoPane *pane,
                               const gchar        *title,
                               const gchar        *text,
                               const gchar        *provider)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pane->text_view));

	gtk_text_buffer_set_text (buffer, "", -1);

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER(buffer), &iter, title, -1,
	                                          "style_bold", "style_large", NULL);

	gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, "\n\n", -1);
	gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, text, -1);

	if (string_is_not_empty(provider)) {
		gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, "\n\n", -1);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, _("Thanks to "), -1);
		gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER(buffer), &iter, provider, -1, "style_bold", "style_italic", NULL);
	}
}

void
pragha_songinfo_pane_clear_text (PraghaSonginfoPane *pane)
{
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pane->text_view));
	gtk_text_buffer_set_text (buffer, "", -1);
}

GtkWidget *
pragha_songinfo_pane_get_pane_title (PraghaSonginfoPane *pane)
{
	return pane->pane_title;
}

GtkMenu *
pragha_songinfo_pane_get_popup_menu (PraghaSonginfoPane *pane)
{
	return GTK_MENU(gtk_ui_manager_get_widget(pane->context_menu, "/popup"));
}

GtkUIManager *
pragha_songinfo_pane_get_pane_context_menu (PraghaSonginfoPane *pane)
{
	return pane->context_menu;
}

GLYR_GET_TYPE
pragha_songinfo_pane_get_default_view (PraghaSonginfoPane *pane)
{
	return pane->info_type;
}

/*
 * Private
 */

/* Menus */

static void
pragha_songinfo_pane_show_artist_info_action (GtkAction *action, PraghaSonginfoPane *pane)
{
	gtk_label_set_text (GTK_LABEL(pane->pane_title), _("Artist info"));
	pane->info_type = GLYR_GET_ARTIST_BIO;
}

static void
pragha_songinfo_pane_show_lyrics_action (GtkAction *action, PraghaSonginfoPane *pane)
{
	gtk_label_set_text (GTK_LABEL(pane->pane_title), _("Lyrics"));
	pane->info_type = GLYR_GET_LYRICS;
}

/* Construction */

static GtkUIManager *
pragha_songinfo_pane_context_menu_new (PraghaSonginfoPane *pane)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new ("Header Songinfo Pane Context Actions");
	context_menu = gtk_ui_manager_new ();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string (context_menu,
	                                        songinfo_pane_context_menu_xml,
	                                        -1, &error)) {
		g_critical ("(%s): Unable to create header songinfo tree context menu, err : %s",
		            __func__, error->message);
	}

	gtk_action_group_add_actions (context_actions,
	                              songinfo_pane_context_aentries,
	                              G_N_ELEMENTS(songinfo_pane_context_aentries),
	                              (gpointer) pane);
	gtk_ui_manager_insert_action_group (context_menu, context_actions, 0);

	g_object_unref (context_actions);

	return context_menu;
}

static void
pragha_songinfo_pane_finalize (GObject *object)
{
	PraghaSonginfoPane *pane = PRAGHA_SONGINFO_PANE (object);

	g_object_unref (pane->context_menu);

	(*G_OBJECT_CLASS (pragha_songinfo_pane_parent_class)->finalize) (object);
}

static void
pragha_songinfo_pane_init (PraghaSonginfoPane *pane)
{
	GtkWidget *view;
	GtkTextBuffer *buffer;

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_large", "scale", PANGO_SCALE_X_LARGE, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_italic", "style", PANGO_STYLE_ITALIC, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pane),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(pane),
	                                     GTK_SHADOW_IN);

	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(pane), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(pane), NULL);

	gtk_container_set_border_width (GTK_CONTAINER (pane), 2);

	gtk_container_add (GTK_CONTAINER (pane), view);

	gtk_widget_show_all (GTK_WIDGET(pane));

	pane->pane_title = gtk_label_new (_("Lyrics"));
	gtk_misc_set_alignment (GTK_MISC(pane->pane_title), 0.0, 0.5);

	pane->context_menu = pragha_songinfo_pane_context_menu_new(pane);
	pane->text_view = view;
	pane->info_type = GLYR_GET_LYRICS;
}

static void
pragha_songinfo_pane_class_init (PraghaSonginfoPaneClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = pragha_songinfo_pane_finalize;
}

PraghaSonginfoPane *
pragha_songinfo_pane_new (void)
{
	return g_object_new (PRAGHA_TYPE_SONGINFO_PANE, NULL);
}