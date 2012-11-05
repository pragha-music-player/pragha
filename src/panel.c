/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
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

/* Search the album art on cache and create a pixbuf of that file */
#ifdef HAVE_LIBGLYR
static gchar*
get_image_path_from_cache (struct con_win *cwin)
{
	gchar *path = NULL;

	path = g_strdup_printf("%s/album-%s-%s.jpeg",
				cwin->cpref->cache_folder,
				cwin->cstate->curr_mobj->tags->artist,
				cwin->cstate->curr_mobj->tags->album);

	if (g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == FALSE) {
		g_free(path);
		return NULL;
	}

	return path;
}
#endif

/* Get the first image file from the directory and create a pixbuf of that file */

static gchar*
get_image_path_from_dir (const gchar *path)
{
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL;
	gchar *result = NULL;

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR) &&
		    is_image_file(ab_file)) {
			result = ab_file;
			goto exit;
		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}

exit:
	g_dir_close(dir);
	return result;
}

/* Find out if any of the preferred album art files are present in the given dir.
   Runs through the patterns in sequence */

static gchar*
get_pref_image_path_dir (const gchar *path, struct con_win *cwin)
{
	GError *error = NULL;
	GDir *dir = NULL;
	const gchar *next_file = NULL;
	gchar *ab_file = NULL, **pattern;
	GSList *file_list = NULL;
	gint i = 0;

	/* Form a list of all files in the given dir */

	dir = g_dir_open(path, 0, &error);
	if (!dir) {
		g_critical("Unable to open dir: %s", path);
		g_error_free(error);
		return NULL;
	}

	next_file = g_dir_read_name(dir);
	while (next_file) {
		ab_file = g_strconcat(path, "/", next_file, NULL);
		if (g_file_test(ab_file, G_FILE_TEST_IS_REGULAR)) {
			file_list = g_slist_append(file_list, g_strdup(next_file));

		}
		g_free(ab_file);
		next_file = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	/* Now, run the preferred patterns through them */

	pattern = g_strsplit(cwin->cpref->album_art_pattern, ";",
			     ALBUM_ART_NO_PATTERNS);
	while (pattern[i]) {
		if (is_present_str_list(pattern[i], file_list)) {
			ab_file = g_strconcat(path, "/", pattern[i], NULL);
			if (is_image_file(ab_file))
				return ab_file;
			g_free(ab_file);
		}
		i++;
	}

	/* Cleanup */

	g_slist_free_full(file_list, g_free);
	g_strfreev(pattern);

	return NULL;
}

void __update_progress_song_info(struct con_win *cwin, gint length)
{
	gchar *tot_length = NULL, *cur_pos = NULL, *str_length = NULL, *str_cur_pos = NULL;

	if (!cwin->cstate->curr_mobj) {
		g_critical("Curr mobj is invalid");
		return;
	}

	cur_pos = convert_length_str(length);
	str_cur_pos = g_markup_printf_escaped ("<small>%s</small>", cur_pos);
	gtk_label_set_markup (GTK_LABEL(cwin->track_time_label), str_cur_pos);

	if(cwin->cstate->curr_mobj->tags->length == 0 || !cwin->cpref->timer_remaining_mode){
		tot_length = convert_length_str(cwin->cstate->curr_mobj->tags->length);
		str_length = g_markup_printf_escaped ("<small>%s</small>", tot_length);
	}
	else{
		tot_length = convert_length_str(cwin->cstate->curr_mobj->tags->length - length);
		str_length = g_markup_printf_escaped ("<small>- %s</small>", tot_length);
	}
	gtk_label_set_markup (GTK_LABEL(cwin->track_length_label), str_length);

	gtk_tooltip_trigger_tooltip_query(gtk_widget_get_display (cwin->track_length_label));

	g_free(cur_pos);
	g_free(str_cur_pos);

	g_free(tot_length);
	g_free(str_length);
}

void update_current_song_info(struct con_win *cwin)
{
	gint newsec = GST_TIME_AS_SECONDS(pragha_backend_get_current_position(cwin->backend));

	__update_progress_song_info(cwin, newsec);
}

void __update_current_song_info(struct con_win *cwin)
{
	gchar *str = NULL, *str_title = NULL;

	if (!cwin->cstate->curr_mobj) {
		g_critical("Curr mobj is invalid");
		return;
	}

	if(g_utf8_strlen(cwin->cstate->curr_mobj->tags->title, 4))
		str_title = g_strdup(cwin->cstate->curr_mobj->tags->title);
	else
		str_title = get_display_filename(cwin->cstate->curr_mobj->file, FALSE);

	if(g_utf8_strlen(cwin->cstate->curr_mobj->tags->artist, 4)
	 && g_utf8_strlen(cwin->cstate->curr_mobj->tags->album, 4))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s <small><span weight=\"light\">in</span></small> %s"), 
						str_title ,
						cwin->cstate->curr_mobj->tags->artist,
						cwin->cstate->curr_mobj->tags->album);
	else if(g_utf8_strlen(cwin->cstate->curr_mobj->tags->artist, 4))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"), 
						str_title ,
						cwin->cstate->curr_mobj->tags->artist);
	else if(g_utf8_strlen(cwin->cstate->curr_mobj->tags->album, 4))
		str = g_markup_printf_escaped (_("%s <small><span weight=\"light\">in</span></small> %s"), 
						str_title ,
						cwin->cstate->curr_mobj->tags->album);
	else	str = g_markup_printf_escaped ("%s", str_title);

	gtk_label_set_markup(GTK_LABEL(cwin->now_playing_label), str);

	g_free(str);
	g_free(str_title);
}

void unset_current_song_info(struct con_win *cwin)
{
	gtk_label_set_markup(GTK_LABEL(cwin->now_playing_label),
				  _("<b>Not playing</b>"));
	gtk_label_set_markup(GTK_LABEL(cwin->track_length_label),"<small>--:--</small>");
	gtk_label_set_markup(GTK_LABEL(cwin->track_time_label),"<small>00:00</small>");

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), 0);

	#ifdef HAVE_LIBCLASTFM
	gtk_widget_hide(cwin->ntag_lastfm_button);
	#endif
}

void __update_track_progress_bar(struct con_win *cwin, gint length)
{
	gdouble fraction = 0;

	if(cwin->cstate->curr_mobj->tags->length == 0) {
		cwin->cstate->curr_mobj->tags->length = GST_TIME_AS_SECONDS(pragha_backend_get_current_length(cwin->backend));
	}
	else {
		fraction = (gdouble)length / (gdouble)cwin->cstate->curr_mobj->tags->length;

		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar),
					      fraction);
	}
}

void unset_track_progress_bar(struct con_win *cwin)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), 0);
}

void edit_tags_playing_event(GtkWidget *w, GdkEventButton* event, struct con_win *cwin)
{
	if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS)
		edit_tags_playing_action(NULL, cwin);
}

void timer_remaining_mode_change(GtkWidget *w, GdkEventButton* event, struct con_win *cwin)
{
	if(cwin->cpref->timer_remaining_mode)
		cwin->cpref->timer_remaining_mode = FALSE;
	else
		cwin->cpref->timer_remaining_mode = TRUE;
	if(pragha_backend_get_state (cwin->backend) != ST_STOPPED)
		update_current_song_info(cwin);
}

void track_progress_change_cb(GtkWidget *widget,
			      GdkEventButton *event,
			      struct con_win *cwin)
{
	gint seek = 0;
	gdouble fraction = 0;

	if (event->button != 1)
		return;

	if (pragha_backend_get_state (cwin->backend) != ST_PLAYING)
		return;

	if (!cwin->cstate->curr_mobj || cwin->cstate->curr_mobj->tags->length == 0)
		return;

	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	seek = (cwin->cstate->curr_mobj->tags->length * event->x) / allocation.width;
	if (seek >= cwin->cstate->curr_mobj->tags->length)
		seek = cwin->cstate->curr_mobj->tags->length;

	fraction = (gdouble) event->x / allocation.width;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), fraction);

	pragha_backend_seek(cwin->backend, seek);
}

void update_album_art(struct musicobject *mobj, struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Update album art");

	gchar *album_path = NULL, *path = NULL;

	if (pragha_album_art_get_visible(cwin->albumart)) {
		if (G_LIKELY(mobj &&
		    mobj->file_type != FILE_CDDA &&
		    mobj->file_type != FILE_HTTP)) {
			#ifdef HAVE_LIBGLYR
			album_path = get_image_path_from_cache(cwin);
			#endif
			if (album_path == NULL) {
				path = g_path_get_dirname(mobj->file);
				if (cwin->cpref->album_art_pattern) {
					album_path = get_pref_image_path_dir(path, cwin);
					if (!album_path)
						album_path = get_image_path_from_dir(path);
				}
				else album_path = get_image_path_from_dir(path);
				g_free(path);
			}
			pragha_album_art_set_path(cwin->albumart, album_path);
			g_free(album_path);
		}
	}
}

gboolean
album_art_frame_press_callback (GtkWidget      *event_box,
				GdkEventButton *event,
				struct con_win *cwin)
{
	if (pragha_backend_get_state (cwin->backend) != ST_STOPPED &&
	   (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS))
	{
		gchar *uri = g_filename_to_uri (pragha_album_art_get_path (cwin->albumart), NULL, NULL);
		open_url (cwin, uri);
		g_free (uri);
	}

	return TRUE;
}

/* Grab focus on current playlist when press Up or Down and move between controls with Left or Right */

gboolean panel_button_key_press (GtkWidget *win, GdkEventKey *event, struct con_win *cwin)
{
	gboolean ret = FALSE;

	if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down){
		GdkEvent *new_event;

		new_event = gdk_event_copy ((GdkEvent *) event);
		gtk_widget_grab_focus(cwin->current_playlist);
		ret = gtk_widget_event (GTK_WIDGET (cwin->current_playlist), new_event);
		gdk_event_free (new_event);
	}
	return ret;
}

/* Handler for the 'Leave fullscren' button item in Panel */

void
unfull_button_handler (GtkToggleToolButton *button, struct con_win *cwin)
{
	GtkAction *action_fullscreen;

	action_fullscreen = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/ViewMenu/Fullscreen");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_fullscreen), FALSE);
}

/* Handler for the 'Shuffle' button item in Panel */

void
shuffle_button_handler (GtkToggleToolButton *button, struct con_win *cwin)
{
	GtkAction *action_shuffle;

	cwin->cpref->shuffle = gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON(button));

	action_shuffle = gtk_ui_manager_get_action(cwin->bar_context_menu, "/Menubar/PlaybackMenu/Shuffle");

	g_signal_handlers_block_by_func (action_shuffle, shuffle_action, cwin);

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_shuffle), cwin->cpref->shuffle);
		shuffle_button(cwin);

	g_signal_handlers_unblock_by_func (action_shuffle, shuffle_action, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
repeat_button_handler (GtkToggleToolButton *button, struct con_win *cwin)
{
	GtkAction *action_repeat;
	action_repeat = gtk_ui_manager_get_action(cwin->bar_context_menu,"/Menubar/PlaybackMenu/Repeat");

	cwin->cpref->repeat = gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON(button));

	g_signal_handlers_block_by_func (action_repeat, repeat_action, cwin);

		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action_repeat), cwin->cpref->repeat);

	g_signal_handlers_unblock_by_func (action_repeat, repeat_action, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void shuffle_button (struct con_win *cwin)
{
	GtkTreeRowReference *ref;

	if(cwin->cstate->tracks_curr_playlist){
		current_playlist_clear_dirty_all(cwin);

		if (!cwin->cpref->shuffle) {
			CDEBUG(DBG_INFO, "Turning shuffle off");
			cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist;
			if (cwin->cstate->curr_rand_ref)
				cwin->cstate->curr_seq_ref =
					gtk_tree_row_reference_copy(cwin->cstate->curr_rand_ref);
			else
				cwin->cstate->curr_seq_ref = NULL;
		}
		else if (cwin->cpref->shuffle) {
			CDEBUG(DBG_INFO, "Turning shuffle on");
			if (cwin->cstate->curr_seq_ref) {
				ref = gtk_tree_row_reference_copy(cwin->cstate->curr_seq_ref);
				cwin->cstate->unplayed_tracks = cwin->cstate->tracks_curr_playlist - 1;
				reset_rand_track_refs(ref, cwin);
			}
		}
	}
}

void play_button_handler(GtkButton *button, struct con_win *cwin)
{
	pragha_playback_play_pause_resume(cwin);
}

void stop_button_handler(GtkButton *button, struct con_win *cwin)
{
	pragha_playback_stop(cwin);
}

void prev_button_handler(GtkButton *button, struct con_win *cwin)
{
	pragha_playback_prev_track(cwin);
}

void next_button_handler(GtkButton *button, struct con_win *cwin)
{
	pragha_playback_next_track(cwin);
}

static void
update_panel_playback_state_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;
	enum player_state state = pragha_backend_get_state (cwin->backend);

	gboolean playing = (state != ST_STOPPED);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->prev_button), playing);

	gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(cwin->play_button),
				     (state == ST_PLAYING) ?
				     GTK_STOCK_MEDIA_PAUSE :
				     GTK_STOCK_MEDIA_PLAY);

	gtk_widget_set_sensitive(GTK_WIDGET(cwin->stop_button), playing);
	gtk_widget_set_sensitive(GTK_WIDGET(cwin->next_button), playing);

	if (playing == FALSE) {
		unset_current_song_info(cwin);
		unset_track_progress_bar(cwin);
		pragha_album_art_set_path(cwin->albumart, NULL);
	}
}

static void
update_gui(PraghaBackend *backend, gpointer user_data)
{
	struct con_win *cwin = user_data;

	gint newsec = GST_TIME_AS_SECONDS(pragha_backend_get_current_position(cwin->backend));

	if (newsec > 0) {
		__update_track_progress_bar(cwin, newsec);
		__update_progress_song_info(cwin, newsec);
	}
}

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


void init_toolbar_preferences_saved(struct con_win *cwin)
{
	GError *error = NULL;
	gboolean show_album_art;
	gint album_art_size;

	show_album_art =
		g_key_file_get_boolean(cwin->cpref->configrc_keyfile,
				       GROUP_WINDOW,
				       KEY_SHOW_ALBUM_ART,
				       &error);
	if (error) {
		g_error_free(error);
		error = NULL;
		show_album_art = TRUE;
	}

	album_art_size = g_key_file_get_integer(cwin->cpref->configrc_keyfile,
						GROUP_WINDOW,
						KEY_ALBUM_ART_SIZE,
						&error);
	if (error) {
		g_error_free(error);
		error = NULL;
		album_art_size = ALBUM_ART_SIZE;
	}

	pragha_album_art_set_size(cwin->albumart, album_art_size);
	pragha_album_art_set_visible(cwin->albumart, show_album_art);
	pragha_album_art_set_path(cwin->albumart, NULL);
}

GtkWidget*
create_toolbar(struct con_win *cwin)
{
	GtkWidget *toolbar, *box;
	GtkToolItem *boxitem, *prev_button, *play_button, *stop_button, *next_button;
	GtkWidget *album_art_frame = NULL, *playing;
	GtkToolItem *unfull_button, *shuffle_button, *repeat_button;
	GtkWidget *vol_button;
	PraghaAlbumArt *albumart;

	toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
#if GTK_CHECK_VERSION (3, 0, 0)
	GtkStyleContext * context = gtk_widget_get_style_context (toolbar);
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
#endif

	/* Setup Left control buttons */

	prev_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), cwin);
	g_signal_connect (G_OBJECT (prev_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_button), _("Previous Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(prev_button));
	cwin->prev_button = prev_button;

	play_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), cwin);
	g_signal_connect (G_OBJECT (play_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(play_button), _("Play / Pause Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(play_button));
	cwin->play_button = play_button;

	stop_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), cwin);
	g_signal_connect (G_OBJECT (stop_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(stop_button), _("Stop playback"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(stop_button));
	cwin->stop_button = stop_button;

	next_button = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), cwin);
	g_signal_connect (G_OBJECT (next_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_button), _("Next Track"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(next_button));
	cwin->next_button = next_button;

	/* Setup album art widget */

	boxitem = gtk_tool_item_new ();
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);
	box = gtk_hbox_new(FALSE, 0);

	album_art_frame = gtk_event_box_new ();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(album_art_frame), FALSE);
	g_signal_connect (G_OBJECT (album_art_frame),
			"button_press_event",
			G_CALLBACK (album_art_frame_press_callback),
			cwin);
	gtk_container_add (GTK_CONTAINER(boxitem), box);
	gtk_box_pack_start (GTK_BOX(box), album_art_frame, TRUE, TRUE, 2);

	albumart = pragha_album_art_new ();

	gtk_container_add(GTK_CONTAINER(album_art_frame), GTK_WIDGET(albumart));

	cwin->albumart = albumart;

	/* Setup playing box */

	boxitem = gtk_tool_item_new ();
	gtk_tool_item_set_expand (boxitem, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(boxitem), -1);

	playing = create_playing_box(cwin);

	box = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box), playing, TRUE, TRUE, 5);
	gtk_container_add (GTK_CONTAINER(boxitem), box);

	/* Setup Right control buttons */

	unfull_button = gtk_tool_button_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN);
	g_signal_connect(G_OBJECT(unfull_button), "clicked",
			 G_CALLBACK(unfull_button_handler), cwin);
	g_signal_connect(G_OBJECT (unfull_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(unfull_button), _("Leave Fullscreen"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(unfull_button));
	cwin->unfull_button = unfull_button;

	shuffle_button = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(shuffle_button), "media-playlist-shuffle");
	g_signal_connect(G_OBJECT(shuffle_button), "toggled",
			 G_CALLBACK(shuffle_button_handler), cwin);
	g_signal_connect(G_OBJECT (shuffle_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(shuffle_button), _("Play songs in a random order"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(shuffle_button));
	cwin->shuffle_button = shuffle_button;

	repeat_button = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(repeat_button), "media-playlist-repeat");
	g_signal_connect(G_OBJECT(repeat_button), "toggled",
			 G_CALLBACK(repeat_button_handler), cwin);
	g_signal_connect(G_OBJECT (repeat_button), "key-press-event",
			 G_CALLBACK(panel_button_key_press), cwin);
	gtk_widget_set_tooltip_text(GTK_WIDGET(repeat_button), _("Repeat playback list at the end"));
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), GTK_WIDGET(repeat_button));
	cwin->repeat_button = repeat_button;

	vol_button = gtk_volume_button_new();
	gtk_button_set_relief(GTK_BUTTON(vol_button), GTK_RELIEF_NONE);
	g_object_set(G_OBJECT(vol_button), "size", GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
	g_object_bind_property (cwin->backend, "volume", vol_button, "value", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
	g_signal_connect (G_OBJECT (vol_button), "key-press-event",
			  G_CALLBACK(panel_button_key_press), cwin);
	gtk_tool_insert_generic_item(GTK_TOOLBAR(toolbar), vol_button);
	cwin->vol_button = vol_button;

	/* Insensitive Prev/Stop/Next buttons and set unknown album art. */

	init_toolbar_preferences_saved(cwin);

	g_signal_connect (cwin->backend, "tick", G_CALLBACK (update_gui), cwin);
	g_signal_connect (cwin->backend, "notify::state", G_CALLBACK (update_panel_playback_state_cb), cwin);

	cwin->toolbar = toolbar;

	return toolbar;
}
