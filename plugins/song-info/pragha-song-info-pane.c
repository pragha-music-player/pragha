/*************************************************************************/
/* Copyright (C) 2011-2018 matias <mati86dl@gmail.com>                   */
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

	/* Title */
	GtkWidget         *title;

	/* Text widget */
	GtkWidget         *text_view;

	/* List widget */
	GtkWidget         *list_view;
	GtkWidget         *append_button;

	/* Info that show thde pane */
	GLYR_GET_TYPE      info_type;

	/* Sidebar widgets */
	GtkWidget         *pane_title;
	GtkUIManager      *context_menu;
};

G_DEFINE_TYPE(PraghaSonginfoPane, pragha_songinfo_pane, GTK_TYPE_SCROLLED_WINDOW)

enum {
	SIGNAL_TYPE_CHANGED,
	SIGNAL_APPEND,
	SIGNAL_APPEND_ALL,
	SIGNAL_QUEUE,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL] = { 0 };

/*
 * Menus definitions
 *
 **/
static void pragha_songinfo_pane_show_artist_info_action (GtkAction *action, PraghaSonginfoPane *pane);
static void pragha_songinfo_pane_show_lyrics_action      (GtkAction *action, PraghaSonginfoPane *pane);
static void pragha_songinfo_pane_show_similar_action     (GtkAction *action, PraghaSonginfoPane *pane);

gchar *songinfo_pane_context_menu_xml = "<ui> \
	<popup>                                   \
	<menuitem action=\"Artist info\"/>        \
	<menuitem action=\"Lyrics\"/>             \
	<menuitem action=\"Similar songs\"/>      \
	</popup>                                  \
	</ui>";

GtkActionEntry songinfo_pane_context_aentries[] = {
	{"Artist info", NULL, N_("Artist info"),
	 "", "Artist info", G_CALLBACK(pragha_songinfo_pane_show_artist_info_action)},
	{"Lyrics", NULL, N_("Lyrics"),
	 "", "Lyrics", G_CALLBACK(pragha_songinfo_pane_show_lyrics_action)},
  	{"Similar songs", NULL, N_("Similar songs"),
	 "", "Similar songs", G_CALLBACK(pragha_songinfo_pane_show_similar_action)}
};

/*
 * Public Api
 */

GtkWidget *
pragha_songinfo_pane_row_new (PraghaMusicobject *mobj)
{
	GtkWidget *row, *box, *icon, *label;
	const gchar *provider = NULL, *title = NULL, *artist = NULL;
	gchar *song_name = NULL;

	title = pragha_musicobject_get_title (mobj);
	artist = pragha_musicobject_get_artist (mobj);
	provider = pragha_musicobject_get_provider (mobj);

	if (string_is_empty(provider))
		icon = gtk_image_new_from_icon_name ("edit-find-symbolic", GTK_ICON_SIZE_MENU);
	else
		icon = gtk_image_new_from_icon_name ("media-playback-start-symbolic", GTK_ICON_SIZE_MENU);

	row = gtk_list_box_row_new ();

	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 6);

	song_name = g_strdup_printf ("%s - %s", title, artist);
	label = gtk_label_new (song_name);
	gtk_label_set_ellipsize (GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(row), box);
	gtk_widget_show_all (row);

	g_object_set_data_full (G_OBJECT(row), "SONG", mobj, g_object_unref);

	g_free (song_name);

	return row;
}

void
pragha_songinfo_pane_set_title (PraghaSonginfoPane *pane,
                                const gchar        *title)
{
	gtk_label_set_text (GTK_LABEL(pane->title), title);
	gtk_widget_show (GTK_WIDGET(pane->title));
}

void
pragha_songinfo_pane_set_text (PraghaSonginfoPane *pane,
                               const gchar        *text,
                               const gchar        *provider)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pane->text_view));

	gtk_text_buffer_set_text (buffer, "", -1);

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(buffer), &iter);
	gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, text, -1);

	if (string_is_not_empty(provider)) {
		if (string_is_not_empty(text))
			gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, "\n\n", -1);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER(buffer), &iter, _("Thanks to "), -1);
		gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER(buffer), &iter, provider, -1, "style_bold", "style_italic", NULL);
	}
}

void
pragha_songinfo_pane_append_song_row (PraghaSonginfoPane *pane,
                                      GtkWidget          *row)
{
	gtk_list_box_insert (GTK_LIST_BOX(pane->list_view), row, 0);
	gtk_widget_show (GTK_WIDGET(pane->list_view));
	gtk_widget_show (GTK_WIDGET(pane->append_button));
}

void
pragha_songinfo_pane_clear_text (PraghaSonginfoPane *pane)
{
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pane->text_view));
	gtk_text_buffer_set_text (buffer, "", -1);

	gtk_widget_hide (GTK_WIDGET(pane->title));
}

void
pragha_songinfo_pane_clear_list (PraghaSonginfoPane *pane)
{
	GList *list, *l;
	GtkWidget *children;

	list = gtk_container_get_children (GTK_CONTAINER(pane->list_view));
	l = list;
	while (l != NULL) {
		children = l->data;
		gtk_container_remove(GTK_CONTAINER(pane->list_view), children);
		l = g_list_next(l);
	}
	g_list_free(list);

	gtk_widget_hide (GTK_WIDGET(pane->list_view));
	gtk_widget_hide (GTK_WIDGET(pane->append_button));
}

GList *
pragha_songinfo_get_mobj_list (PraghaSonginfoPane *pane)
{
	PraghaMusicobject *mobj;
	GList *mlist = NULL, *list, *l;
	GtkWidget *row;
	const gchar *provider = NULL;

	list = gtk_container_get_children (GTK_CONTAINER(pane->list_view));
	l = list;
	while (l != NULL) {
		row = l->data;
		mobj = g_object_get_data (G_OBJECT(row), "SONG");
		provider = pragha_musicobject_get_provider (mobj);
		if (string_is_not_empty(provider))
			mlist = g_list_append (mlist, mobj);
		l = g_list_next(l);
	}
	g_list_free (list);

	return mlist;
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

static void
pragha_song_info_row_activated (GtkListBox         *box,
                                GtkListBoxRow      *row,
                                PraghaSonginfoPane *pane)
{
	PraghaMusicobject *mobj = NULL;

	mobj = g_object_get_data (G_OBJECT(row), "SONG");
	if (mobj == NULL)
		return;

	g_signal_emit (pane, signals[SIGNAL_APPEND], 0, mobj);
}

static gboolean
pragha_song_info_row_key_press (GtkWidget          *widget,
                                GdkEventKey        *event,
                                PraghaSonginfoPane *pane)
{
	GtkListBoxRow *row;
	PraghaMusicobject *mobj = NULL;
	if (event->keyval != GDK_KEY_q && event->keyval != GDK_KEY_Q)
		return FALSE;

	row = gtk_list_box_get_selected_row (GTK_LIST_BOX (pane->list_view));
	mobj = g_object_get_data (G_OBJECT(row), "SONG");
	if (mobj == NULL)
		return FALSE;

	g_signal_emit (pane, signals[SIGNAL_QUEUE], 0, mobj);

	return TRUE;
}

static void
pragha_song_info_append_songs (GtkButton          *button,
                               PraghaSonginfoPane *pane)
{
	g_signal_emit (pane, signals[SIGNAL_APPEND_ALL], 0);
}

static void
song_list_header_func (GtkListBoxRow *row,
                       GtkListBoxRow *before,
                       gpointer       user_data)
{
	GtkWidget *header;
	header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_list_box_row_set_header (row, header);
}

static gint
song_list_sort_func (GtkListBoxRow *a,
                     GtkListBoxRow *b,
                     gpointer user_data)
{
	PraghaMusicobject *mobja = NULL, *mobjb = NULL;
	const gchar *providera = NULL, *providerb = NULL;

	mobja = g_object_get_data (G_OBJECT(a), "SONG");
	mobjb = g_object_get_data (G_OBJECT(b), "SONG");

	providera = pragha_musicobject_get_provider (mobja);
	providerb = pragha_musicobject_get_provider (mobjb);

	if (string_is_empty(providera) && string_is_not_empty(providerb))
		return 1;

	if (string_is_not_empty(providera) && string_is_empty(providerb))
		return -1;

	if (string_is_empty(providera) && string_is_empty(providerb))
		return -1;

	if (string_is_not_empty(providera) && string_is_not_empty(providerb))
		return 1;

	return 0;
}

/* Menus */

static void
pragha_songinfo_pane_show_artist_info_action (GtkAction *action, PraghaSonginfoPane *pane)
{
	gtk_label_set_text (GTK_LABEL(pane->pane_title), _("Artist info"));
	pane->info_type = GLYR_GET_ARTIST_BIO;

	g_signal_emit (pane, signals[SIGNAL_TYPE_CHANGED], 0);
}

static void
pragha_songinfo_pane_show_lyrics_action (GtkAction *action, PraghaSonginfoPane *pane)
{
	gtk_label_set_text (GTK_LABEL(pane->pane_title), _("Lyrics"));
	pane->info_type = GLYR_GET_LYRICS;

	g_signal_emit (pane, signals[SIGNAL_TYPE_CHANGED], 0);
}

static void
pragha_songinfo_pane_show_similar_action (GtkAction *action, PraghaSonginfoPane *pane)
{
	gtk_label_set_text (GTK_LABEL(pane->pane_title), _("Similar songs"));
	pane->info_type = GLYR_GET_SIMILAR_SONGS;

	g_signal_emit (pane, signals[SIGNAL_TYPE_CHANGED], 0);
}

void
pragha_koel_plugin_set_tiny_button (GtkWidget *button)
{
	GtkCssProvider *provider;
	GtkStyleContext *context;

	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (provider,
	                                 ".tiny-button {\n"
#if GTK_CHECK_VERSION (3, 14, 0)
	                                 " margin : 0px;\n"
	                                 " min-width: 14px; \n"
	                                 " min-height: 12px; \n"
#else
	                                 " -GtkButton-default-border : 0px;\n"
	                                 " -GtkButton-default-outside-border : 0px;\n"
	                                 " -GtkButton-inner-border: 0px;\n"
	                                 " -GtkWidget-focus-line-width: 0px;\n"
	                                 " -GtkWidget-focus-padding: 0px;\n"
#endif
	                                 " padding: 1px;}",
	                                 -1, NULL);

	context = gtk_widget_get_style_context (button);
	gtk_style_context_add_provider (context,
	                                GTK_STYLE_PROVIDER (provider),
	                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_style_context_add_class (context, "tiny-button");
	g_object_unref (provider);
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
	GtkWidget *box, *lbox, *label, *append_button, *icon, *view, *list;
	GtkTextBuffer *buffer;
	PangoAttrList *attrs;
	GtkStyleContext *context;

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	context = gtk_widget_get_style_context (box);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_VIEW);

	label = gtk_label_new (_("Lyrics"));
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_xalign (GTK_LABEL (label), 0.0);

	attrs = pango_attr_list_new ();
	pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
	pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_X_LARGE));
	gtk_label_set_attributes (GTK_LABEL (label), attrs);
	pango_attr_list_unref (attrs);

	lbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	context = gtk_widget_get_style_context (lbox);
	gtk_style_context_add_class (context, "linked");

	append_button = gtk_button_new ();
	pragha_koel_plugin_set_tiny_button (append_button);
	gtk_widget_set_tooltip_text (append_button, _("_Add to current playlist"));
	gtk_widget_set_valign (append_button, GTK_ALIGN_CENTER);
	icon = gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
	gtk_image_set_pixel_size (GTK_IMAGE(icon), 12);
	gtk_button_set_image(GTK_BUTTON(append_button), icon);

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

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pane),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(pane),
	                                     GTK_SHADOW_IN);

	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(pane), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(pane), NULL);

	list = gtk_list_box_new ();
	gtk_list_box_set_header_func (GTK_LIST_BOX (list),
	                              song_list_header_func, list, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (list),
	                            song_list_sort_func, list, NULL);

	gtk_box_pack_start (GTK_BOX(lbox), GTK_WIDGET(label), FALSE, FALSE, 4);
	gtk_box_pack_start (GTK_BOX(lbox), GTK_WIDGET(append_button), FALSE, FALSE, 4);

	gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(lbox), FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(list), FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(view), FALSE, FALSE, 2);
	gtk_container_add (GTK_CONTAINER (pane), box);

	gtk_widget_show_all (GTK_WIDGET(pane));

	pane->pane_title = gtk_label_new (_("Lyrics"));
	gtk_widget_set_halign (GTK_WIDGET(pane->pane_title), GTK_ALIGN_START);
	gtk_widget_set_valign (GTK_WIDGET(pane->pane_title), GTK_ALIGN_CENTER);

	g_signal_connect (list, "row-activated",
	                  G_CALLBACK(pragha_song_info_row_activated), pane);
	g_signal_connect (list, "key-press-event",
	                  G_CALLBACK(pragha_song_info_row_key_press), pane);
	g_signal_connect (append_button, "clicked",
	                  G_CALLBACK(pragha_song_info_append_songs), pane);

	pane->context_menu = pragha_songinfo_pane_context_menu_new(pane);
	pane->title = label;
	pane->text_view = view;
	pane->list_view = list;
	pane->append_button = append_button;
	pane->info_type = GLYR_GET_LYRICS;
}

static void
pragha_songinfo_pane_class_init (PraghaSonginfoPaneClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = pragha_songinfo_pane_finalize;

	signals[SIGNAL_TYPE_CHANGED] =
		g_signal_new ("type-changed",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSonginfoPaneClass, type_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_APPEND] =
		g_signal_new ("append",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSonginfoPaneClass, append),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[SIGNAL_APPEND_ALL] =
		g_signal_new ("append-all",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSonginfoPaneClass, append_all),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_QUEUE] =
		g_signal_new ("queue",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaSonginfoPaneClass, queue),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);
}

PraghaSonginfoPane *
pragha_songinfo_pane_new (void)
{
	return g_object_new (PRAGHA_TYPE_SONGINFO_PANE, NULL);
}
