/*************************************************************************/
/* Copyright (C) 2010-2012 matias <mati86dl@gmail.com>			 */
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

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
#if GST_CHECK_VERSION (1, 0, 0)
#include <gst/audio/streamvolume.h>
#else
#include <gst/interfaces/streamvolume.h>
#endif //GST_CHECK_VERSION
#define convert_volume(from, to, val) gst_stream_volume_convert_volume((from), (to), (val))
#define VOLUME_FORMAT_LINEAR GST_STREAM_VOLUME_FORMAT_LINEAR
#define VOLUME_FORMAT_CUBIC GST_STREAM_VOLUME_FORMAT_CUBIC
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

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_duration(cwin->cgst->pipeline, format, &song_length);
#else
	result = gst_element_query_duration(cwin->cgst->pipeline, &format, &song_length);
#endif

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

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_position(cwin->cgst->pipeline, format, &song_position);
#else
	result = gst_element_query_position(cwin->cgst->pipeline, &format, &song_position);
#endif

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

	g_object_get (G_OBJECT(cwin->cgst->pipeline), "volume", &volume, NULL);

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_LINEAR, VOLUME_FORMAT_CUBIC, volume);
#endif

	return volume;
}

gboolean
update_volume_notify_cb (struct con_win *cwin)
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
	g_idle_add ((GSourceFunc) update_volume_notify_cb, cwin);
}

void
backend_set_volume(gdouble volume, struct con_win *cwin)
{
	volume = CLAMP (volume, 0.0, 1.0);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);
#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_CUBIC, VOLUME_FORMAT_LINEAR, volume);
#endif
	g_object_set (G_OBJECT(cwin->cgst->pipeline), "volume", volume, NULL);

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
	gdouble volume;

	cwin->cgst->curr_vol = CLAMP (cwin->cgst->curr_vol, 0.0, 1.0);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->cgst->pipeline), volume_notify_cb, cwin);
#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_CUBIC, VOLUME_FORMAT_LINEAR, cwin->cgst->curr_vol);
#else
	volume = cwin->cgst->curr_vol;
#endif
	g_object_set (G_OBJECT(cwin->cgst->pipeline), "volume", volume, NULL);

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

	cwin->cstate->state = ST_STOPPED;

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_READY);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, error, cwin);
		gtk_tree_path_free(path);
	}

	update_panel_playback_state(cwin);
	update_menubar_playback_state(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_pause(struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Pause playback");

	cwin->cstate->state = ST_PAUSED;

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PAUSED);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
	update_panel_playback_state(cwin);
	update_menubar_playback_state(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
backend_resume(struct con_win *cwin)
{
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Resuming playback");

	cwin->cstate->state = ST_PLAYING;

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
	update_panel_playback_state(cwin);
	update_menubar_playback_state(cwin);

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

	if(cwin->cstate->playlist_change)
		return;

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

/* Signal handler for parse the error dialog response. */

static void backend_error_dialog_response(GtkDialog *dialog,
					  gint response,
					  GError *error)
{
	struct con_win *cwin;

	cwin = g_object_get_data (G_OBJECT(dialog), "cwin");

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
	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_error_free (error);
}

static void
backend_parse_error (GstMessage *message, struct con_win *cwin)
{
	GtkWidget *dialog;
	gboolean emit = TRUE;
	GError *error = NULL;
	gchar *dbg_info = NULL;

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

		cwin->cgst->emitted_error = TRUE;

		dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (cwin->mainwindow),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("<b>Error playing current track.</b>\n(%s)\n<b>Reason:</b> %s"),
						cwin->cstate->curr_mobj->file, error->message);

		gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_STOP, GTK_RESPONSE_ACCEPT);
		gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_MEDIA_NEXT, GTK_RESPONSE_APPLY);

		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY);

		g_object_set_data (G_OBJECT(dialog), "cwin", cwin);

		g_signal_connect(G_OBJECT(dialog), "response",
				 G_CALLBACK(backend_error_dialog_response), error);
		gtk_widget_show_all(dialog);
	}
	else {
		g_error_free (error);
	}
	g_free (dbg_info);
}

static void
backend_parse_buffering (GstMessage *message, struct con_win *cwin)
{
	gint percent = 0;
	GstState cur_state;

	if(cwin->cstate->state == ST_STOPPED) /* Prevent that buffering overlaps the stop command playing or pausing the playback */
		return;

	gst_message_parse_buffering (message, &percent);
	gst_element_get_state (cwin->cgst->pipeline, &cur_state, NULL, 0);

	if (percent >= 100) {
		if(cwin->cstate->state == ST_PLAYING && cur_state != GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering complete ... return to playback");
			gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);
		}
	}
	else {
		if (cwin->cstate->state == ST_PLAYING && cur_state == GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering ... temporarily pausing playback");
			gst_element_set_state (cwin->cgst->pipeline, GST_STATE_PAUSED);
		}
		else {
			CDEBUG(DBG_BACKEND, "Buffering (already paused) ... %d", percent);
		}
	}

	gdk_threads_enter();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar), (gdouble)percent/100);
	gdk_threads_leave();
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
	mpris_update_metadata_changed(cwin);

	path = current_playlist_get_actual(cwin);
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->cplaylist->wplaylist));

	if(gtk_tree_model_get_iter(model, &iter, path))
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

	gst_tag_list_free(tag_list);
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

	cwin->cstate->state = ST_PLAYING;

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_PLAYING);

	update_panel_playback_state (cwin);
	update_menubar_playback_state(cwin);

	update_related_state (cwin);

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
	backend_stop(NULL, cwin);

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(cwin->cgst->pipeline));
	cwin->cgst->pipeline = NULL;

	CDEBUG(DBG_BACKEND, "Pipeline destruction complete");
}


void
backend_init_equalizer_preset(struct con_win *cwin)
{
	gdouble *saved_bands;
	GError *error = NULL;

	if(cwin->cgst->equalizer == NULL)
		return;

	saved_bands = g_key_file_get_double_list(cwin->cpref->configrc_keyfile,
						 GROUP_AUDIO,
						 KEY_EQ_10_BANDS,
						 NULL,
						 &error);
	if (saved_bands != NULL) {
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band0", saved_bands[0], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band1", saved_bands[1], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band2", saved_bands[2], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band3", saved_bands[3], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band4", saved_bands[4], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band5", saved_bands[5], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band6", saved_bands[6], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band7", saved_bands[7], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band8", saved_bands[8], NULL);
		g_object_set(G_OBJECT(cwin->cgst->equalizer), "band9", saved_bands[9], NULL);

		g_free(saved_bands);
	}
	else {
		g_error_free(error);
		error = NULL;
	}
}

gint backend_init(struct con_win *cwin)
{
	GstBus *bus;
	gchar *audiosink = NULL;
	gboolean can_set_device = TRUE;

	gst_init(NULL, NULL);

#if GST_CHECK_VERSION (1, 0, 0)
	cwin->cgst->pipeline = gst_element_factory_make("playbin", "playbin");
#else
	cwin->cgst->pipeline = gst_element_factory_make("playbin2", "playbin");
#endif

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
		audiosink = g_strdup("alsasink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS4_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss4 like audio sink");
		audiosink = g_strdup("oss4sink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss like audio sink");
		audiosink = g_strdup("osssink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, PULSE_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Pulseaudio like audio sink");
		audiosink = g_strdup("pulsesink");
	}
	else {
		CDEBUG(DBG_BACKEND, "Setting autoaudiosink like audio sink");
		can_set_device = FALSE;
		audiosink = g_strdup("autoaudiosink");
	}

	cwin->cgst->audio_sink = gst_element_factory_make (audiosink, "audio-sink");

	if (cwin->cgst->audio_sink != NULL) {
		/* Set the audio device to use. */
		if (can_set_device && cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0')
			g_object_set(G_OBJECT(cwin->cgst->audio_sink), "device", cwin->cpref->audio_device, NULL);

		/* Test 10bands equalizer and test it. */
		cwin->cgst->equalizer = gst_element_factory_make ("equalizer-10bands", "equalizer");
		if (cwin->cgst->equalizer != NULL) {
			GstElement *bin = gst_bin_new("audiobin");
			GstPad* audiopad = gst_element_get_static_pad (cwin->cgst->equalizer, "sink");

			gst_bin_add_many (GST_BIN(bin), cwin->cgst->equalizer, cwin->cgst->audio_sink, NULL);
			gst_element_link_many (cwin->cgst->equalizer, cwin->cgst->audio_sink, NULL);

			gst_element_add_pad (GST_ELEMENT(bin), gst_ghost_pad_new("sink", audiopad));
		
			gst_object_unref (audiopad);

			g_object_set(G_OBJECT(cwin->cgst->pipeline), "audio-sink", bin, NULL);
		}
		else {
			g_warning("Failed to create the 10bands equalizer element. Not use it.");

			g_object_set(G_OBJECT(cwin->cgst->pipeline), "audio-sink", cwin->cgst->audio_sink, NULL);
		}
	}
	else {
		g_warning("Failed to create audio-sink element. Use default sink, without equalizer. ");

		cwin->cgst->equalizer = NULL;
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "audio-sink", cwin->cgst->audio_sink, NULL);
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(cwin->cgst->pipeline));

	gst_bus_add_watch(bus, (GstBusFunc) backend_gstreamer_bus_call, cwin);

	backend_set_soft_volume(cwin);
	backend_init_equalizer_preset(cwin);

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_READY);

	gst_object_unref(bus);
	g_free(audiosink);

	CDEBUG(DBG_BACKEND, "Pipeline construction complete");

	return 0;
}
