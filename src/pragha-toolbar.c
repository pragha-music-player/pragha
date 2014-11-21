/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2014 matias <mati86dl@gmail.com>                   */
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

#include "pragha-toolbar.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <gdk/gdkkeysyms.h>

#include "pragha-simple-widgets.h"
#include "pragha-utils.h"
#include "pragha.h"

static void pragha_toolbar_finalize (GObject *object);

static void pragha_toolbar_set_remaning_mode (PraghaToolbar *toolbar, gboolean remaning_mode);
gboolean    pragha_toolbar_get_remaning_mode (PraghaToolbar *toolbar);


struct _PraghaToolbar {
#if GTK_CHECK_VERSION (3, 12, 0)
	GtkHeaderBar   __parent__;
#else
	GtkToolbar      __parent__;
#endif
	PraghaAlbumArt *albumart;
	PraghaTrackProgress *track_progress_bar;

	GtkToolItem    *prev_button;
	GtkToolItem    *play_button;
	GtkToolItem    *stop_button;
	GtkToolItem    *next_button;
	GtkToolItem    *unfull_button;
	GtkWidget      *vol_button;
	GtkWidget      *track_length_label;
	GtkWidget      *track_time_label;
	GtkWidget      *now_playing_label;
	GtkWidget      *extention_box;

	gboolean       remaning_mode;
};

enum {
	PROP_0,
	PROP_VOLUME,
	PROP_REMANING_MODE,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { 0 };

enum
{
	PREV_ACTIVATED,
	PLAY_ACTIVATED,
	STOP_ACTIVATED,
	NEXT_ACTIVATED,
	ALBUM_ART_ACTIVATED,
	TRACK_INFO_ACTIVATED,
	TRACK_PROGRESS_ACTIVATED,
	UNFULL_ACTIVATED,
	TRACK_TIME_ACTIVATED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

#if GTK_CHECK_VERSION (3, 12, 0)
G_DEFINE_TYPE(PraghaToolbar, pragha_toolbar, GTK_TYPE_HEADER_BAR)
#else
G_DEFINE_TYPE(PraghaToolbar, pragha_toolbar, GTK_TYPE_TOOLBAR)
#endif

void
pragha_toolbar_update_progress (PraghaToolbar *toolbar, gint length, gint progress)
{
	gdouble fraction = 0;
	gchar *tot_length = NULL, *cur_pos = NULL, *str_length = NULL, *str_cur_pos = NULL;

	cur_pos = convert_length_str(progress);
	str_cur_pos = g_markup_printf_escaped ("<small>%s</small>", cur_pos);

	if (length == 0 || !pragha_toolbar_get_remaning_mode (toolbar)) {
		tot_length = convert_length_str(length);
		str_length = g_markup_printf_escaped ("<small>%s</small>", tot_length);
	}
	else {
		tot_length = convert_length_str(length - progress);
		str_length = g_markup_printf_escaped ("<small>- %s</small>", tot_length);
	}

	gtk_label_set_markup (GTK_LABEL(toolbar->track_time_label), str_cur_pos);
	gtk_label_set_markup (GTK_LABEL(toolbar->track_length_label), str_length);

	gtk_tooltip_trigger_tooltip_query(gtk_widget_get_display (toolbar->track_length_label));

	if(length) {
		fraction = (gdouble) progress / (gdouble)length;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(toolbar->track_progress_bar), fraction);
	}

	g_free(cur_pos);
	g_free(str_cur_pos);

	g_free(tot_length);
	g_free(str_length);
}

void
pragha_toolbar_set_title (PraghaToolbar *toolbar, PraghaMusicobject *mobj)
{
	gchar *str = NULL, *str_title = NULL;

	const gchar *file = pragha_musicobject_get_file (mobj);
	const gchar *title = pragha_musicobject_get_title (mobj);
	const gchar *artist = pragha_musicobject_get_artist (mobj);
	const gchar *album = pragha_musicobject_get_album (mobj);

	if(string_is_not_empty(title))
		str_title = g_strdup(title);
	else
		str_title = get_display_filename(file, FALSE);

	if(string_is_not_empty(artist) && string_is_not_empty(album))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s <small><span weight=\"light\">in</span></small> %s"),
		                               str_title,
		                               artist,
		                               album);
	else if(string_is_not_empty(artist))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"),
		                                str_title,
		                                artist);
	else if(string_is_not_empty(album))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">in</span></small> %s"),
		                                str_title,
		                                album);
	else
		str = g_markup_printf_escaped("%s", str_title);

	gtk_label_set_markup(GTK_LABEL(toolbar->now_playing_label), str);

	g_free(str_title);
	g_free(str);
}

static void
pragha_toolbar_unset_song_info(PraghaToolbar *toolbar)
{
	gtk_label_set_markup(GTK_LABEL(toolbar->now_playing_label), _("<b>Not playing</b>"));
	gtk_label_set_markup(GTK_LABEL(toolbar->track_length_label),  "<small>--:--</small>");
	gtk_label_set_markup(GTK_LABEL(toolbar->track_time_label),    "<small>00:00</small>");

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(toolbar->track_progress_bar), 0);

	pragha_album_art_set_path(toolbar->albumart, NULL);
}

static void
pragha_toolbar_timer_label_event_change_mode (GtkWidget      *widget,
                                              GdkEventButton *event,
                                              PraghaToolbar  *toolbar)
{
	pragha_toolbar_set_remaning_mode (toolbar,
		!pragha_toolbar_get_remaning_mode (toolbar));
}

void
pragha_toolbar_set_image_album_art (PraghaToolbar *toolbar, const gchar *uri)
{
	pragha_album_art_set_path (toolbar->albumart, uri);
}

/* Grab focus on current playlist when press Up or Down and move between controls with Left or Right */

/*static gboolean
panel_button_key_press (GtkWidget *win, GdkEventKey *event, PraghaApplication *pragha)
{
	gboolean ret = FALSE;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down ||
	    event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_Page_Down) {
		ret = pragha_playlist_propagate_event(pragha->cplaylist, event);
	}

	return ret;
}*/

/*
 * Emit signals..
 */

static gboolean
play_button_handler(GtkButton *button, PraghaToolbar *toolbar)
{
	g_signal_emit (toolbar, signals[PLAY_ACTIVATED], 0);

	return TRUE;
}

static gboolean
stop_button_handler(GtkButton *button, PraghaToolbar *toolbar)
{
	g_signal_emit (toolbar, signals[STOP_ACTIVATED], 0);

	return TRUE;
}

static gboolean
prev_button_handler(GtkButton *button, PraghaToolbar *toolbar)
{
	g_signal_emit (toolbar, signals[PREV_ACTIVATED], 0);

	return TRUE;
}

static gboolean
next_button_handler(GtkButton *button, PraghaToolbar *toolbar)
{
	g_signal_emit (toolbar, signals[NEXT_ACTIVATED], 0);

	return TRUE;
}

static gboolean
unfull_button_handler (GtkButton *button, PraghaToolbar *toolbar)
{
	g_signal_emit (toolbar, signals[UNFULL_ACTIVATED], 0);

	return TRUE;
}

static gboolean
pragha_toolbar_album_art_activated (GtkWidget      *event_box,
                                    GdkEventButton *event,
                                    PraghaToolbar  *toolbar)
{
	if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS)
#if GTK_CHECK_VERSION (3, 12, 0)
		if (!gtk_header_bar_get_show_close_button(GTK_HEADER_BAR(toolbar)))
#endif
		g_signal_emit (toolbar, signals[ALBUM_ART_ACTIVATED], 0);

	return FALSE;
}

static gboolean
pragha_toolbar_song_label_event_edit (GtkWidget      *event_box,
                                      GdkEventButton *event,
                                      PraghaToolbar  *toolbar)
{
	if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS)
#if GTK_CHECK_VERSION (3, 12, 0)
		if (!gtk_header_bar_get_show_close_button(GTK_HEADER_BAR(toolbar)))
#endif
		g_signal_emit (toolbar, signals[TRACK_INFO_ACTIVATED], 0);

	return FALSE;
}

static void
pragha_toolbar_progress_bar_event_seek (GtkWidget *widget,
                                        GdkEventButton *event,
                                        PraghaToolbar *toolbar)
{
	GtkAllocation allocation;
	gdouble fraction = 0;

	if (event->button != 1)
		return;

	gtk_widget_get_allocation(widget, &allocation);

	fraction = (gdouble) event->x / allocation.width;

	g_signal_emit (toolbar, signals[TRACK_PROGRESS_ACTIVATED], 0, fraction);
}

/*
 * Callbacks that response to gstreamer signals.
 */

void
pragha_toolbar_update_buffering_cb (PraghaBackend *backend, gint percent, gpointer user_data)
{
	PraghaToolbar *toolbar = user_data;

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(toolbar->track_progress_bar), (gdouble)percent/100);
}

void
pragha_toolbar_update_playback_progress(PraghaBackend *backend, gpointer user_data)
{
	gint length = 0, newsec = 0;
	PraghaMusicobject *mobj = NULL;

	PraghaToolbar *toolbar = user_data;

	newsec = GST_TIME_AS_SECONDS(pragha_backend_get_current_position(backend));

	if (newsec > 0) {
		mobj = pragha_backend_get_musicobject (backend);
		length = pragha_musicobject_get_length (mobj);

		if (length > 0) {
			pragha_toolbar_update_progress (toolbar, length, newsec);
		}
		else {
			gint nlength = GST_TIME_AS_SECONDS(pragha_backend_get_current_length(backend));
			pragha_musicobject_set_length (mobj, nlength);
		}
	}
}

void
pragha_toolbar_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data)
{
	PraghaToolbar *toolbar = user_data;
	PraghaBackendState state = pragha_backend_get_state (backend);

	gboolean playing = (state != ST_STOPPED);

	gtk_widget_set_sensitive (GTK_WIDGET(toolbar->prev_button), playing);

	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(toolbar->play_button),
	                               (state == ST_PLAYING) ? "media-playback-pause" : "media-playback-start");

	gtk_widget_set_sensitive (GTK_WIDGET(toolbar->stop_button), playing);
	gtk_widget_set_sensitive (GTK_WIDGET(toolbar->next_button), playing);

	if (playing == FALSE)
		pragha_toolbar_unset_song_info(toolbar);
}

void
pragha_toolbar_show_ramaning_time_cb (PraghaToolbar *toolbar, GParamSpec *pspec, gpointer user_data)
{
	PraghaBackend *backend = user_data;
	pragha_toolbar_update_playback_progress (backend, toolbar);
}

/*
 * Show the unfullscreen button according to the state of the window.
 */

gboolean
pragha_toolbar_window_state_event (GtkWidget *widget, GdkEventWindowState *event, PraghaToolbar *toolbar)
{
	if (event->type == GDK_WINDOW_STATE && (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
		gtk_widget_set_visible(GTK_WIDGET(toolbar->unfull_button), (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0);
	}

	return FALSE;
}

/*
 * Public api.
 */

#if GTK_CHECK_VERSION (3, 12, 0)
void
pragha_toolbar_set_style (PraghaToolbar *toolbar, gboolean gnome_style)
{
	GtkStyleContext *context;
	context = gtk_widget_get_style_context (GTK_WIDGET(toolbar));

	if (gnome_style) {
		gtk_style_context_remove_class (context, GTK_STYLE_CLASS_TOOLBAR);
		gtk_style_context_remove_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
		gtk_style_context_add_class (context, "header-bar");
	}
	else {
		gtk_style_context_remove_class (context, "header-bar");
		gtk_style_context_add_class (context, GTK_STYLE_CLASS_TOOLBAR);
		gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	}
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(toolbar), gnome_style);
}
#endif

void
pragha_toolbar_add_extention_widget(PraghaToolbar *toolbar, GtkWidget *widget)
{
	GList *list;
	GtkWidget *children;

	list = gtk_container_get_children (GTK_CONTAINER(toolbar->extention_box));
	if(list) {
		children = list->data;
		gtk_container_remove(GTK_CONTAINER(toolbar->extention_box), children);
		gtk_widget_destroy(GTK_WIDGET(children));
		g_list_free(list);
	}
	gtk_container_add(GTK_CONTAINER(toolbar->extention_box), widget);
}

const gchar*
pragha_toolbar_get_progress_text(PraghaToolbar *toolbar)
{
	return gtk_label_get_text (GTK_LABEL(toolbar->track_time_label));
}

const gchar*
pragha_toolbar_get_length_text(PraghaToolbar *toolbar)
{
	return gtk_label_get_text (GTK_LABEL(toolbar->track_length_label));
}

PraghaAlbumArt *
pragha_toolbar_get_album_art(PraghaToolbar *toolbar)
{
	return toolbar->albumart;
}

/*
 * Pragha toolbar creation and destruction.
 */

GtkWidget *
pragha_toolbar_get_song_box (PraghaToolbar *toolbar)
{
	PraghaPreferences *preferences;
	PraghaTrackProgress *progress_bar;
	PraghaAlbumArt *albumart;
	PraghaContainer *box;
	GtkWidget *hbox, *vbox_aling, *vbox, *top_hbox, *botton_hbox;
	GtkWidget *album_art_frame,*title, *title_event_box, *extention_box;
	GtkWidget *progress_bar_event_box, *time_label, *time_align, *length_label, *length_align, *length_event_box;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	preferences = pragha_preferences_get();

	/*
	 * Main box that allow expand.
	 */
	box = pragha_container_new ();

 	/*
 	 * Main box: [Album][Song info]
 	 */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX(box), hbox, TRUE, TRUE, 0);

	album_art_frame = gtk_event_box_new ();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(album_art_frame), FALSE);
	g_signal_connect(G_OBJECT (album_art_frame), "button_press_event",
	                 G_CALLBACK (pragha_toolbar_album_art_activated), toolbar);
	gtk_box_pack_start (GTK_BOX(hbox), album_art_frame, FALSE, FALSE, 2);

	albumart = pragha_album_art_new ();
	g_object_bind_property (preferences, "album-art-size",
	                        albumart, "size", binding_flags);
	g_object_bind_property (preferences, "show-album-art",
	                        albumart, "visible", binding_flags);
	gtk_container_add(GTK_CONTAINER(album_art_frame), GTK_WIDGET(albumart));
	toolbar->albumart = albumart;

	/*
	 * Song info vbox
	 */
	vbox_aling = gtk_alignment_new(0.5, 0.5, 1, 0);

 	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(vbox_aling), GTK_WIDGET(vbox));

 	/*
 	 * Top box: [Title][extentions]
 	 */
	top_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), top_hbox, TRUE, TRUE, 0);

	/* The title widget. */

	title_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(title_event_box), FALSE);

	g_signal_connect (G_OBJECT(title_event_box), "button-press-event",
	                  G_CALLBACK(pragha_toolbar_song_label_event_edit), toolbar);

	title = gtk_label_new(NULL);
	gtk_label_set_ellipsize (GTK_LABEL(title), PANGO_ELLIPSIZE_END);
	gtk_label_set_markup(GTK_LABEL(title),_("<b>Not playing</b>"));
	gtk_misc_set_alignment(GTK_MISC(title), 0, 0.5);

	gtk_container_add (GTK_CONTAINER(title_event_box), title);

	/* The extentions box. */
	
	extention_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	/* Pack top widgets:  */

	gtk_box_pack_start (GTK_BOX(top_hbox),
	                    GTK_WIDGET(title_event_box),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(top_hbox),
	                    GTK_WIDGET(extention_box),
	                    FALSE, FALSE, 0);

	/*
	 * Botton box: [Time][ProgressBar][Length]
	 */
	botton_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), botton_hbox, FALSE, FALSE, 0);

	/* Time progress widget. */

	time_align = gtk_alignment_new(1, 0.5, 0, 0);

	time_label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(time_label),"<small>00:00</small>");

	gtk_container_add(GTK_CONTAINER(time_align), time_label);

	/* Progress bar widget. */

	progress_bar_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(progress_bar_event_box), FALSE);

	g_signal_connect (G_OBJECT(progress_bar_event_box), "button-press-event",
	                  G_CALLBACK(pragha_toolbar_progress_bar_event_seek), toolbar);

	progress_bar = pragha_track_progress_new ();

	gtk_container_add (GTK_CONTAINER(progress_bar_event_box),
	                   GTK_WIDGET(progress_bar));

	/* Length and remaining time widget. */

	length_event_box = gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(length_event_box), FALSE);
	g_signal_connect (G_OBJECT(length_event_box), "button-press-event",
	                  G_CALLBACK(pragha_toolbar_timer_label_event_change_mode), toolbar);

	length_align = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(length_event_box), length_align);

	length_label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(length_label),"<small>--:--</small>");

	gtk_container_add(GTK_CONTAINER(length_align), length_label);

	/* Pack widgets. */

	gtk_box_pack_start (GTK_BOX(botton_hbox),
	                    GTK_WIDGET(time_align),
	                    FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX(botton_hbox),
	                    GTK_WIDGET(progress_bar_event_box),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(botton_hbox),
	                    GTK_WIDGET(length_event_box),
	                    FALSE, FALSE, 3);

	/* Save references. */

	toolbar->track_progress_bar = progress_bar;
	toolbar->now_playing_label  = title;
	toolbar->track_time_label   = time_label;
	toolbar->track_length_label = length_label;
	toolbar->extention_box      = extention_box;

	gtk_box_pack_start(GTK_BOX(hbox), vbox_aling, TRUE, TRUE, 2);
	gtk_widget_show_all(GTK_WIDGET(box));

	g_object_unref(preferences);

	return GTK_WIDGET(box);
}

#if !GTK_CHECK_VERSION (3, 12, 0)
static void
gtk_tool_insert_generic_item(GtkToolbar *toolbar, GtkWidget *item)
{
	GtkWidget *align_box;
	GtkToolItem *boxitem;

	boxitem = gtk_tool_item_new ();

	align_box = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(align_box), item);

	gtk_container_add (GTK_CONTAINER(boxitem), align_box);
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);
}
#endif

static void
vol_button_value_changed (GtkVolumeButton *button, gdouble value, PraghaToolbar *toolbar)
{
	g_object_notify_by_pspec (G_OBJECT (toolbar), properties[PROP_VOLUME]);
}

static void
pragha_toolbar_set_volume (PraghaToolbar *toolbar, gdouble volume)
{
	gtk_scale_button_set_value (GTK_SCALE_BUTTON(toolbar->vol_button), volume);
}

gdouble
pragha_toolbar_get_volume (PraghaToolbar *toolbar)
{
	return gtk_scale_button_get_value (GTK_SCALE_BUTTON(toolbar->vol_button));
}

static void
pragha_toolbar_set_remaning_mode (PraghaToolbar *toolbar, gboolean remaning_mode)
{
	toolbar->remaning_mode = remaning_mode;

	g_object_notify_by_pspec(G_OBJECT(toolbar), properties[PROP_REMANING_MODE]);
}

gboolean
pragha_toolbar_get_remaning_mode (PraghaToolbar *toolbar)
{
	return toolbar->remaning_mode;
}

static void
pragha_toolbar_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	PraghaToolbar *toolbar = PRAGHA_TOOLBAR (object);

	switch (property_id)
	{
		case PROP_VOLUME:
			pragha_toolbar_set_volume (toolbar, g_value_get_double (value));
			break;
		case PROP_REMANING_MODE:
			pragha_toolbar_set_remaning_mode (toolbar, g_value_get_boolean (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_toolbar_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	PraghaToolbar *toolbar = PRAGHA_TOOLBAR (object);

	switch (property_id)
	{
		case PROP_VOLUME:
			g_value_set_double (value, pragha_toolbar_get_volume (toolbar));
			break;
		case PROP_REMANING_MODE:
			g_value_set_boolean (value, pragha_toolbar_get_remaning_mode (toolbar));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_toolbar_class_init (PraghaToolbarClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->set_property = pragha_toolbar_set_property;
	gobject_class->get_property = pragha_toolbar_get_property;
	gobject_class->finalize = pragha_toolbar_finalize;

	/*
	 * Properties:
	 */
	properties[PROP_VOLUME] = g_param_spec_double ("volume", "Volume", "Volume showed on toolbar",
	                                               0.0, 1.0, 0.5,
	                                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_REMANING_MODE] = g_param_spec_boolean ("timer-remaining-mode", "TimerRemainingMode", "Show Remaining Time",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class, PROP_LAST, properties);

	/*
	 * Signals:
	 */
	signals[PREV_ACTIVATED] = g_signal_new ("prev",
	                                        G_TYPE_FROM_CLASS (gobject_class),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (PraghaToolbarClass, prev),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__VOID,
	                                        G_TYPE_NONE, 0);
	signals[PLAY_ACTIVATED] = g_signal_new ("play",
	                                        G_TYPE_FROM_CLASS (gobject_class),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (PraghaToolbarClass, play),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__VOID,
	                                        G_TYPE_NONE, 0);
	signals[STOP_ACTIVATED] = g_signal_new ("stop",
	                                        G_TYPE_FROM_CLASS (gobject_class),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (PraghaToolbarClass, stop),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__VOID,
	                                        G_TYPE_NONE, 0);
	signals[NEXT_ACTIVATED] = g_signal_new ("next",
	                                        G_TYPE_FROM_CLASS (gobject_class),
	                                        G_SIGNAL_RUN_LAST,
	                                        G_STRUCT_OFFSET (PraghaToolbarClass, next),
	                                        NULL, NULL,
	                                        g_cclosure_marshal_VOID__VOID,
	                                        G_TYPE_NONE, 0);
	signals[ALBUM_ART_ACTIVATED] = g_signal_new ("album-art-activated",
	                                             G_TYPE_FROM_CLASS (gobject_class),
	                                             G_SIGNAL_RUN_LAST,
	                                             G_STRUCT_OFFSET (PraghaToolbarClass, album_art_activated),
	                                             NULL, NULL,
	                                             g_cclosure_marshal_VOID__VOID,
	                                             G_TYPE_NONE, 0);
	signals[TRACK_INFO_ACTIVATED] = g_signal_new ("track-info-activated",
	                                              G_TYPE_FROM_CLASS (gobject_class),
	                                              G_SIGNAL_RUN_LAST,
	                                              G_STRUCT_OFFSET (PraghaToolbarClass, track_info_activated),
	                                              NULL, NULL,
	                                              g_cclosure_marshal_VOID__VOID,
	                                              G_TYPE_NONE, 0);
	signals[TRACK_PROGRESS_ACTIVATED] = g_signal_new ("track-progress-activated",
	                                                  G_TYPE_FROM_CLASS (gobject_class),
	                                                  G_SIGNAL_RUN_LAST,
	                                                  G_STRUCT_OFFSET (PraghaToolbarClass, track_progress_activated),
	                                                  NULL, NULL,
	                                                  g_cclosure_marshal_VOID__DOUBLE,
	                                                  G_TYPE_NONE, 1, G_TYPE_DOUBLE);
	signals[UNFULL_ACTIVATED] = g_signal_new ("unfull-activated",
	                                          G_TYPE_FROM_CLASS (gobject_class),
	                                          G_SIGNAL_RUN_LAST,
	                                          G_STRUCT_OFFSET (PraghaToolbarClass, unfull),
	                                          NULL, NULL,
	                                          g_cclosure_marshal_VOID__VOID,
	                                          G_TYPE_NONE, 0);
}

static void
pragha_toolbar_init (PraghaToolbar *toolbar)
{
	PraghaPreferences *preferences;
	GtkToolItem *prev_button, *play_button, *stop_button, *next_button;
	GtkToolItem *unfull_button, *shuffle_button, *repeat_button;
	GtkWidget *vol_button;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	preferences = pragha_preferences_get();

	/* Setup Left control buttons */

	prev_button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(prev_button), "media-skip-backward");
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), _("Previous Track"));
	toolbar->prev_button = prev_button;

	play_button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(play_button), "media-playback-start");
	gtk_widget_set_tooltip_text(GTK_WIDGET(play_button), _("Play / Pause Track"));
	toolbar->play_button = play_button;

	stop_button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(stop_button), "media-playback-stop");
	gtk_widget_set_tooltip_text(GTK_WIDGET(stop_button), _("Stop playback"));
	toolbar->stop_button = stop_button;

	next_button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(next_button), "media-skip-forward");
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), _("Next Track"));
	toolbar->next_button = next_button;

#if GTK_CHECK_VERSION (3, 12, 0)
	gtk_header_bar_pack_start(GTK_HEADER_BAR(toolbar), GTK_WIDGET(prev_button));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(toolbar), GTK_WIDGET(play_button));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(toolbar), GTK_WIDGET(stop_button));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(toolbar), GTK_WIDGET(next_button));
#else
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(prev_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(play_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(stop_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(next_button));
#endif

	/* Song info Box */

#if !GTK_CHECK_VERSION (3, 12, 0)
	GtkToolItem *boxitem = gtk_tool_item_new ();
	gtk_tool_item_set_expand (GTK_TOOL_ITEM(boxitem), TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);

	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add (GTK_CONTAINER(boxitem), box);

	GtkWidget *playing = pragha_toolbar_get_song_box (toolbar);
	gtk_box_pack_start (GTK_BOX(box), playing, TRUE, TRUE, 5);
#endif

	/* Setup Right control buttons */

	unfull_button = gtk_tool_button_new (NULL, NULL);
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON(unfull_button), "view-restore");
	gtk_widget_set_tooltip_text(GTK_WIDGET(unfull_button), _("Leave Fullscreen"));
	toolbar->unfull_button = unfull_button;

	shuffle_button = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(shuffle_button), "media-playlist-shuffle");
	gtk_widget_set_tooltip_text(GTK_WIDGET(shuffle_button), _("Play songs in a random order"));

	repeat_button = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(repeat_button), "media-playlist-repeat");
	gtk_widget_set_tooltip_text(GTK_WIDGET(repeat_button), _("Repeat playback list at the end"));

	vol_button = gtk_volume_button_new();
	g_object_set(vol_button, "use-symbolic", FALSE, NULL);
	gtk_button_set_relief(GTK_BUTTON(vol_button), GTK_RELIEF_NONE);
	g_object_set(G_OBJECT(vol_button), "size", GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	toolbar->vol_button = vol_button;

#if GTK_CHECK_VERSION (3, 12, 0)
	gtk_header_bar_pack_end(GTK_HEADER_BAR(toolbar), GTK_WIDGET(vol_button));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(toolbar), GTK_WIDGET(repeat_button));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(toolbar), GTK_WIDGET(shuffle_button));
	gtk_header_bar_pack_end(GTK_HEADER_BAR(toolbar), GTK_WIDGET(unfull_button));
#else
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(unfull_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(shuffle_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(repeat_button));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(vol_button));
#endif

	/* Connect signals */

	g_signal_connect(G_OBJECT(prev_button), "clicked",
	                 G_CALLBACK(prev_button_handler), toolbar);
	g_signal_connect(G_OBJECT(play_button), "clicked",
	                 G_CALLBACK(play_button_handler), toolbar);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
	                 G_CALLBACK(stop_button_handler), toolbar);
	g_signal_connect(G_OBJECT(next_button), "clicked",
	                 G_CALLBACK(next_button_handler), toolbar);
	g_signal_connect(G_OBJECT(unfull_button), "clicked",
	                 G_CALLBACK(unfull_button_handler), toolbar);

	/*g_signal_connect(G_OBJECT (prev_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (play_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (stop_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (next_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (next_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (unfull_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (shuffle_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (repeat_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);
	g_signal_connect(G_OBJECT (vol_button), "key-press-event",
	                 G_CALLBACK(panel_button_key_press), toolbar);*/

	g_signal_connect (G_OBJECT (vol_button), "value-changed",
	                  G_CALLBACK (vol_button_value_changed), toolbar);

	g_object_bind_property(preferences, "shuffle", shuffle_button, "active", binding_flags);
	g_object_bind_property(preferences, "repeat", repeat_button, "active", binding_flags);

	/* Fix styling */
#if GTK_CHECK_VERSION (3, 12, 0)
	pragha_toolbar_set_style(toolbar,
		pragha_preferences_get_gnome_style (preferences));
#endif

	gtk_widget_show_all(GTK_WIDGET(toolbar));
	gtk_widget_hide(GTK_WIDGET(toolbar->unfull_button));

	g_object_unref(preferences);
}

static void
pragha_toolbar_finalize (GObject *object)
{
	(*G_OBJECT_CLASS (pragha_toolbar_parent_class)->finalize) (object);
}

PraghaToolbar *
pragha_toolbar_new (void)
{
	return g_object_new (PRAGHA_TYPE_TOOLBAR, NULL);
}

