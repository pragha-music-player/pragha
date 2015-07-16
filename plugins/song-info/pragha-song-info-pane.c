/*************************************************************************/
/* Copyright (C) 2011-2015 matias <mati86dl@gmail.com>                   */
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

#include "src/pragha-album-art.h"
#include "src/pragha-utils.h"

struct _PraghaSonginfoPane {
	GtkScrolledWindow  parent;

	PraghaAlbumArt    *albumart_view;
	GtkWidget         *lyrics_view;

	/* Sidebar widgets */
	GtkWidget         *pane_title;
	GtkUIManager      *context_menu;
};

G_DEFINE_TYPE(PraghaSonginfoPane, pragha_songinfo_pane, GTK_TYPE_SCROLLED_WINDOW)

/*
 * Menus definitions
 *
 **/

gchar *songinfo_pane_context_menu_xml = "<ui> \
	<popup>                                   \
	<menuitem action=\"Show album art\"/>             \
	</popup>                                  \
	</ui>";

static GtkToggleActionEntry songinfo_pane_toggles_entries[] = {
	{"Show album art", NULL, N_("Show album art"),
	 "", "Show album art", NULL, TRUE}
};

/*
 * Public Api
 */
void
pragha_songinfo_pane_set_album_art (PraghaSonginfoPane *pane,
                                    const gchar        *uri)
{
	pragha_album_art_set_path (PRAGHA_ALBUM_ART(pane->albumart_view), uri);
}

void
pragha_songinfo_pane_set_text (PraghaSonginfoPane *pane,
                               const gchar        *title,
                               const gchar        *text,
                               const gchar        *provider)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pane->lyrics_view));

	gtk_text_buffer_set_text (buffer, "", -1);

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(buffer), &iter);
	gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER(buffer), &iter, title, -1,
	                                          "style_bold", "style_large", "margin_top", NULL);

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
	pragha_songinfo_pane_set_text (pane,
	                               _("Why not enjoy a song?"),
	                               _("Here you can view the information of your songs"),
	                               NULL);

	pragha_album_art_set_path (PRAGHA_ALBUM_ART(pane->albumart_view), NULL);
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

/*
 * Private
 */

/* Menus */


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

	gtk_action_group_add_toggle_actions (context_actions,
	                                     songinfo_pane_toggles_entries,
	                                     G_N_ELEMENTS(songinfo_pane_toggles_entries),
	                                     pane);
	gtk_ui_manager_insert_action_group (context_menu, context_actions, 0);

	GtkAction *action = gtk_ui_manager_get_action(context_menu, "/popup/Show album art");
	g_object_bind_property (pane->albumart_view, "visible", action, "active",
	                        G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

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

static GtkWidget *
pragha_songinfo_text_view_new (void)
{
	GtkWidget *view;
	GtkTextBuffer *buffer;

  	view = gtk_text_view_new ();

  	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);
	g_object_set (view, "left-margin", 4, "right-margin", 4, NULL);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_large", "scale", PANGO_SCALE_X_LARGE, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "style_italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "margin_top", "pixels-above-lines", 2, NULL);

	return view;
}

static void
pragha_songinfo_pane_init (PraghaSonginfoPane *pane)
{
	GtkWidget *grid;

	grid = gtk_grid_new ();
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(grid)),
	                             "view");

	pane->albumart_view = pragha_album_art_new ();
	pragha_album_art_set_size(pane->albumart_view, 128);
	pragha_album_art_set_path(pane->albumart_view, NULL);

	gtk_widget_set_hexpand (GTK_WIDGET(pane->albumart_view), TRUE);
	gtk_grid_attach (GTK_GRID(grid),
	                 GTK_WIDGET(pane->albumart_view),
	                 0, 0, 1, 1);

	pane->lyrics_view  = pragha_songinfo_text_view_new ();
	gtk_widget_set_hexpand (GTK_WIDGET(pane->lyrics_view), TRUE);
	gtk_grid_attach (GTK_GRID(grid),
	                 GTK_WIDGET(pane->lyrics_view),
	                 0, 1, 1, 1);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pane),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(pane),
	                                     GTK_SHADOW_IN);

	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(pane), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(pane), NULL);

	gtk_container_add (GTK_CONTAINER (pane), grid);

	gtk_widget_show_all (GTK_WIDGET(pane));

	pane->pane_title = gtk_label_new (_("Information about the song"));
	gtk_misc_set_alignment (GTK_MISC(pane->pane_title), 0.0, 0.5);

	pane->context_menu = pragha_songinfo_pane_context_menu_new(pane);
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
