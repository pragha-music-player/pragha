/*************************************************************************/
/* Copyright (C) 2010-2011 matias <mati86dl@gmail.com>			 */
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

#if HAVE_GSTREAMER_PLUGINS_BASE
#include <gst/interfaces/streamvolume.h>
#endif

typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9)
} GstPlayFlags;

static gboolean update_gui(gpointer data)
{
	struct con_win *cwin = data;

	if(cwin->cstate->curr_mobj->file_type == FILE_HTTP)
		return TRUE;

	gint newsec = GST_TIME_AS_SECONDS(backend_get_current_position(cwin));

	if(newsec > 0) {
		__update_track_progress_bar(cwin, newsec);
		__update_progress_song_info(cwin, newsec);
	}

	return TRUE;
}

gboolean update_track_progress_bar(gpointer data)
{
	struct con_win *cwin = data;

	gint newsec = GST_TIME_AS_SECONDS(backend_get_current_position(cwin));

	if(newsec > 0)
		__update_track_progress_bar(cwin, newsec);

	return FALSE;
}

gboolean update_current_song_info(gpointer data)
{
	struct con_win *cwin = data;

	gint newsec = GST_TIME_AS_SECONDS(backend_get_current_position(cwin));

	__update_progress_song_info(cwin, newsec);

	return FALSE;
}

static void
backend_source_notify_cb (GObject *obj, GParamSpec *pspec, struct con_win *cwin)
{
	GObject *source;

	if(cwin->cstate->curr_mobj->file_type != FILE_CDDA)
		return;

	g_object_get (obj, "source", &source, NULL);

	if (source) {
		if (G_LIKELY(cwin->cpref->audio_cd_device)) {
			g_object_set (source,  "device", cwin->cpref->audio_cd_device, NULL);
		}
		g_object_unref (source);
	}
}

gint64
backend_get_current_length(struct con_win *cwin)
{
	gint64 song_length;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

	result = gst_element_query_duration(cwin->cgst->pipeline, &format, &song_length);

	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;
 
	return song_length;
}

gint64
backend_get_current_position(struct con_win *cwin)
{
	gint64 song_position;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

	result = gst_element_query_position(cwin->cgst->pipeline, &format, &song_position);
	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return song_position;
}

void
backend_seek (guint64 seek, struct con_win *cwin)
{
	CDEBUG(DBG_BACKEND, "Seeking playback");

	gst_element_seek (cwin->cgst->pipeline,
	       1.0,
	       GST_FORMAT_TIME,
	       GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH,
	       GST_SEEK_TYPE_SET, seek*(1000*1000*1000),
	       GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void
backend_set_soft_volume(struct con_win *cwin)
{
	GstPlayFlags flags;

	g_object_get (G_OBJECT(cwin->cgst->pipeline), "flags", &flags, NULL);

	if (cwin->cpref->software_mixer)
		flags |= GST_PLAY_FLAG_SOFT_VOLUME;
	else
		flags &= ~GST_PLAY_FLAG_SOFT_VOLUME;

	g_object_set (G_OBJECT(cwin->cgst->pipeline), "flags", flags, NULL);
}

gdouble
backend_get_volume(struct con_win *cwin)
{
	gdouble volume;

#if HAVE_GSTREAMER_PLUGINS_BASE
	if (gst_element_implements_interface (cwin->cgst->pipeline, GST_TYPE_STREAM_VOLUME)) {
		volume = gst_stream_volume_get_volume (GST_STREAM_VOLUME (cwin->cgst->pipeline),
						    GST_STREAM_VOLUME_FORMAT_CUBIC);
	} else {
		g_object_get (G_OBJECT(cwin->cgst->pipeline), "volume", &volume, NULL);
	}
#else
	g_object_get (G_OBJECT(cwin->cgst->pipeline), "volume", &volume, NULL);
#endif

	return volume;
}

gboolean
emit_volume_changed_idle (struct con_win *cwin)
{
	cwin->cgst->curr_vol = backend_get_volume(cwin);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button), 100 * cwin->cgst->curr_vol);
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);

	return FALSE;
}

void
volume_notify_cb (GObject *element, GstObject *prop_object, GParamSpec *pspec, struct con_win *cwin)
{
	g_idle_add ((GSourceFunc) emit_volume_changed_idle, cwin);
}

void
backend_set_volume(gdouble volume, struct con_win *cwin)
{
	volume = CLAMP (volume, 0.0, 1.0);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);
#if HAVE_GSTREAMER_PLUGINS_BASE
	if (gst_element_implements_interface (cwin->cgst->pipeline, GST_TYPE_STREAM_VOLUME))
		gst_stream_volume_set_volume (GST_STREAM_VOLUME (cwin->cgst->pipeline),
					      GST_STREAM_VOLUME_FORMAT_CUBIC, volume);
	else
		g_object_set (cwin->cgst->pipeline, "volume", volume, NULL);
#else
	g_object_set (G_OBJECT(cwin->cgst->pipeline), "volume", volume, NULL);
#endif
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);

	g_signal_handlers_block_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button), 100 * volume);
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);

	cwin->cgst->curr_vol = volume;

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_update_volume(struct con_win *cwin)
{
	cwin->cgst->curr_vol = CLAMP (cwin->cgst->curr_vol, 0.0, 1.0);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);
#if HAVE_GSTREAMER_PLUGINS_BASE
	if (gst_element_implements_interface (cwin->cgst->pipeline, GST_TYPE_STREAM_VOLUME))
		gst_stream_volume_set_volume (GST_STREAM_VOLUME (cwin->cgst->pipeline),
					      GST_STREAM_VOLUME_FORMAT_CUBIC, cwin->cgst->curr_vol);
	else
		g_object_set (cwin->cgst->pipeline, "volume", cwin->cgst->curr_vol, NULL);
#else
	g_object_set (G_OBJECT(cwin->cgst->pipeline), "volume", cwin->cgst->curr_vol, NULL);
#endif
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);

	g_signal_handlers_block_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button), 100 * cwin->cgst->curr_vol);
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

gboolean
backend_is_playing(struct con_win *cwin)
{
	GstState state;
	gst_element_get_state (GST_ELEMENT(cwin->cgst->pipeline), &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PLAYING)
		return TRUE;
	return FALSE;
}

gboolean
backend_is_paused(struct con_win *cwin)
{
	GstState state;
	gst_element_get_state(GST_ELEMENT(cwin->cgst->pipeline), &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PAUSED)
		return TRUE;
	return FALSE;
}

gboolean
backend_need_stopped(struct con_win *cwin)
{
	GstState state;
	gst_element_get_state(GST_ELEMENT(cwin->cgst->pipeline), &state, NULL, GST_CLOCK_TIME_NONE);

	if ((state == GST_STATE_PAUSED) || (state == GST_STATE_PLAYING))
		return TRUE;

	return FALSE;
}

void
backend_stop(GError *error, struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Stopping playback");

	if (cwin->cstate->curr_mobj_clear) {
		delete_musicobject(cwin->cstate->curr_mobj);
		cwin->cstate->curr_mobj_clear = FALSE;
	}

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_READY);

	cwin->cstate->state = ST_STOPPED;

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, error, cwin);
		gtk_tree_path_free(path);
	}

	play_button_toggle_state(cwin);

	unset_current_song_info(cwin);
	unset_track_progress_bar(cwin);
	unset_album_art(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_pause(struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Pause playback");

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PAUSED);

	cwin->cstate->state = ST_PAUSED;

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
	play_button_toggle_state(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_resume(struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Resuming playback");

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);

	cwin->cstate->state = ST_PLAYING;

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
	play_button_toggle_state(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_advance_playback (GError *error, struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Advancing to next track");

	/* Stop to set ready and clear all info */
	backend_stop(error, cwin);

	/* Get the next track to be played */
	path = current_playlist_get_next (cwin);

	/* No more tracks */
	if (!path)
		return;

	/* Start playing new track */
	mobj = current_playlist_mobj_at_path (path, cwin);
	backend_start (mobj, cwin);

	update_current_state (path, PLAYLIST_NEXT, cwin);
	gtk_tree_path_free (path);
}

static void
backend_parse_error (GstMessage *message, struct con_win *cwin)
{
	GtkWidget *dialog;
	gboolean emit = TRUE;
	GError *error = NULL;
	gchar *dbg_info = NULL;
	gint response;

	gst_message_parse_error (message, &error, &dbg_info);

	/* Gstreamer doc: When an error has occured
	 * playbin should be set back to READY or NULL state.
	 */
	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_NULL);

	/* Next code inspired on rhynthmbox.
	 * If we've already got an error, ignore 'internal data flow error'
	 * type messages, as they're too generic to be helpful.
	 */
	if (cwin->cgst->emitted_error &&
		error->domain == GST_STREAM_ERROR &&
		error->code == GST_STREAM_ERROR_FAILED) {
		CDEBUG(DBG_BACKEND, "Ignoring generic error \"%s\"", error->message);
		emit = FALSE;
	}

	if(emit) {
		CDEBUG(DBG_BACKEND, "Gstreamer error \"%s\"", error->message);

		gdk_threads_enter();

		#ifdef HAVE_LIBKEYBINDER
		keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
		keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
		keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
		keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
		keybinder_unbind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler);
		#endif

		dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (cwin->mainwindow),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("<b>Error playing current track.</b>\n(%s)\n<b>Reason:</b> %s"),
						cwin->cstate->curr_mobj->file, error->message);

		gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_STOP, GTK_RESPONSE_ACCEPT);
		gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_NEXT, GTK_RESPONSE_APPLY);

		response = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		#ifdef HAVE_LIBKEYBINDER
		keybinder_bind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler, cwin);
		keybinder_bind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler, cwin);
		keybinder_bind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler, cwin);
		keybinder_bind("XF86AudioNext", (KeybinderHandler) keybind_next_handler, cwin);
		keybinder_bind("XF86AudioMedia", (KeybinderHandler) keybind_media_handler, cwin);
		#endif

		gdk_threads_leave();

		switch (response) {
			case GTK_RESPONSE_APPLY: {
				backend_advance_playback (error, cwin);
				break;
			}
			case GTK_RESPONSE_ACCEPT:
			case GTK_RESPONSE_DELETE_EVENT:
			default: {
				backend_stop (error, cwin);
				break;
			}
		}
		cwin->cgst->emitted_error = TRUE;
	}
	g_error_free (error);
	g_free (dbg_info);
}

static void
backend_parse_buffering (GstMessage *message, struct con_win *cwin)
{
	gint percent = 0;
	GtkTreePath *path = NULL;
	gboolean changed = FALSE;

	CDEBUG(DBG_BACKEND, "Buffering...");

	gst_message_parse_buffering (message, &percent);

	if (percent >= 100 && cwin->cstate->state != ST_PLAYING) {
		gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);
		cwin->cstate->state = ST_PLAYING;
		changed = TRUE;
	}
	else if (cwin->cstate->state != ST_PAUSED) {
		gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PAUSED);
		cwin->cstate->state = ST_PAUSED;
		changed = TRUE;
	}

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), (gdouble)percent/100);

	if (changed) {
		path = current_playlist_get_actual(cwin);
		if (path) {
			update_pixbuf_state_on_path(path, NULL, cwin);
			gtk_tree_path_free(path);
		}
		play_button_toggle_state(cwin);
	}
}

static void
backend_parse_message_tag(GstMessage *message, struct con_win *cwin)
{
	GstTagList *tag_list;
	struct musicobject *mobj = NULL;
	struct tags ntag;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gchar *str = NULL;

	gint changed = 0;

	if(cwin->cstate->curr_mobj->file_type != FILE_HTTP)
		return;

	CDEBUG(DBG_BACKEND, "Parse message tag");

	memset(&ntag, 0, sizeof(struct tags));

	gst_message_parse_tag(message, &tag_list);

	if (gst_tag_list_get_string(tag_list, GST_TAG_TITLE, &str))
	{
		changed |= TAG_TITLE_CHANGED;
		ntag.title = g_strdup(str);
		g_free(str);
	}
	if (gst_tag_list_get_string(tag_list, GST_TAG_ARTIST, &str))
	{
		changed |= TAG_ARTIST_CHANGED;
		ntag.artist = g_strdup(str);
		g_free(str);
	}

	update_musicobject(cwin->cstate->curr_mobj, changed, &ntag, cwin);
	__update_current_song_info(cwin);

	path = current_playlist_get_actual(cwin);
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	if (G_UNLIKELY(mobj == NULL))
		g_warning("Invalid mobj pointer");
	else
		update_track_current_playlist(&iter, changed, mobj, cwin);

	if(ntag.title)
		g_free(ntag.title);
	if(ntag.artist)
		g_free(ntag.artist);

	gtk_tree_path_free(path);
}


void
backend_start(struct musicobject *mobj, struct con_win *cwin)
{
	CDEBUG(DBG_BACKEND, "Starting playback");

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((cwin->cstate->state == ST_PLAYING) ||
	    (cwin->cstate->state == ST_PAUSED)) {
		backend_stop(NULL, cwin);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->curr_mobj_clear = FALSE;

	backend_play(cwin);
}

void
backend_play(struct con_win *cwin)
{
	gchar *uri = NULL;

	if (!cwin->cstate->curr_mobj->file)
		return;

	CDEBUG(DBG_BACKEND, "Playing: %s", cwin->cstate->curr_mobj->file);

	if(cwin->cstate->curr_mobj->file_type == FILE_CDDA ||
	   cwin->cstate->curr_mobj->file_type == FILE_HTTP) {
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "uri", cwin->cstate->curr_mobj->file, NULL);
	}
	else {
		uri = g_filename_to_uri (cwin->cstate->curr_mobj->file, NULL, NULL);
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "uri", uri, NULL);
		g_free (uri);
	}

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);

	cwin->cstate->state = ST_PLAYING;

	play_button_toggle_state (cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);

	cwin->cgst->emitted_error = FALSE;
}

static void
backend_evaluate_state (GstState old, GstState new, GstState pending, struct con_win *cwin)
{
	if (pending != GST_STATE_VOID_PENDING)
		return;

	switch (new) {
		case GST_STATE_PLAYING: {
			if (cwin->cstate->state == ST_PLAYING) {
				if(cwin->cgst->timer == 0)
					cwin->cgst->timer = gdk_threads_add_timeout_seconds (1, update_gui, cwin);

				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_PAUSED: {
			if (cwin->cstate->state == ST_PAUSED) {
				if(cwin->cgst->timer > 0) {
					g_source_remove(cwin->cgst->timer);
					cwin->cgst->timer = 0;
				}
				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_READY:
		case GST_STATE_NULL: {
			if(cwin->cgst->timer > 0) {
				g_source_remove(cwin->cgst->timer);
				cwin->cgst->timer = 0;
			}
			CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			break;
		}
		default:
			break;
	}
}

static gboolean backend_gstreamer_bus_call(GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			backend_advance_playback (NULL, cwin);
			break;
		case GST_MESSAGE_STATE_CHANGED: {
			GstState old, new, pending;
			gst_message_parse_state_changed (msg, &old, &new, &pending);
			if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cwin->cgst->pipeline))
				backend_evaluate_state (old, new, pending, cwin);
			break;
		}
		case GST_MESSAGE_BUFFERING: {
			backend_parse_buffering (msg, cwin);
			break;
		}
		case GST_MESSAGE_TAG: {
			backend_parse_message_tag (msg, cwin);
			break;
		}
		case GST_MESSAGE_ERROR: {
			backend_parse_error (msg, cwin);
			break;
		}
    		default:
			break;
	}
	return TRUE;
}

void
backend_quit(struct con_win *cwin)
{
	if (cwin->cgst->pipeline != NULL) {
		gst_element_set_state(cwin->cgst->pipeline, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(cwin->cgst->pipeline));
		cwin->cgst->pipeline = NULL;
	}
	CDEBUG(DBG_BACKEND, "Pipeline destruction complete");
}

gint backend_init(struct con_win *cwin)
{
	GstBus *bus;
	gchar *audiosink = NULL;

	gst_init(NULL, NULL);

	cwin->cgst->pipeline = gst_element_factory_make("playbin2", "playbin");

	if (cwin->cgst->pipeline == NULL)
		return -1;

	g_signal_connect (G_OBJECT (cwin->cgst->pipeline), "deep-notify::volume",
			  G_CALLBACK (volume_notify_cb), cwin);
	g_signal_connect (G_OBJECT (cwin->cgst->pipeline), "notify::source",
			  G_CALLBACK (backend_source_notify_cb), cwin);

	/* If no audio sink has been specified via the "audio-sink" property, playbin will use the autoaudiosink.
	   Need review then when return the audio preferences. */

	if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Alsa like audio sink");

		if (cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0') {
			audiosink = g_strdup_printf ("alsasink device=\"%s\"", cwin->cpref->audio_device);
		}
		else
			audiosink = g_strdup("alsasink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS4_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss4 like audio sink");

		if (cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0') {
			audiosink = g_strdup_printf ("oss4sink device=\"%s\"", cwin->cpref->audio_device);
		}
		else
			audiosink = g_strdup("oss4sink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss like audio sink");

		if (cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0') {
			audiosink = g_strdup_printf ("osssink device=\"%s\"", cwin->cpref->audio_device);
		}
		else
			audiosink = g_strdup("osssink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, PULSE_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Pulseaudio like audio sink");

		audiosink = g_strdup("pulsesink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, AUTO_SINK)){
		CDEBUG(DBG_BACKEND, "Setting autoaudiosink like audio sink");

		audiosink = g_strdup("autoaudiosink");
	}

	if(audiosink != NULL)
		cwin->cgst->audio_sink = gst_element_factory_make (audiosink, "audio-sink");

	if(cwin->cgst->audio_sink == NULL) {
		CDEBUG(DBG_BACKEND, "Try to use the default audiosink");
	}
	else {
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "audio-sink", cwin->cgst->audio_sink, NULL);
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(cwin->cgst->pipeline));

	gst_bus_add_watch(bus, (GstBusFunc) backend_gstreamer_bus_call, cwin);

	backend_set_soft_volume(cwin);
	emit_volume_changed_idle(cwin);

	cwin->cgst->emitted_error = FALSE;

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_READY);

	gst_object_unref(bus);

	CDEBUG(DBG_BACKEND, "Pipeline construction complete");

	return 0;
}
