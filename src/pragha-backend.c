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

G_DEFINE_TYPE (PraghaBackend, pragha_backend, G_TYPE_OBJECT);

static gboolean update_gui(gpointer data)
{
	struct con_win *cwin = data;

	gint newsec = GST_TIME_AS_SECONDS(pragha_backend_get_current_position(cwin->backend));

	if(newsec > 0) {
		__update_track_progress_bar(cwin, newsec);
		__update_progress_song_info(cwin, newsec);
	}

	return TRUE;
}

static void
pragha_backend_source_notify_cb (GObject *obj, GParamSpec *pspec, struct con_win *cwin)
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
pragha_backend_get_current_length (PraghaBackend *backend)
{
	gint64 song_length;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_duration(backend->pipeline, format, &song_length);
#else
	result = gst_element_query_duration(backend->pipeline, &format, &song_length);
#endif

	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;
 
	return song_length;
}

gint64
pragha_backend_get_current_position (PraghaBackend *backend)
{
	gint64 song_position;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_position(backend->pipeline, format, &song_position);
#else
	result = gst_element_query_position(backend->pipeline, &format, &song_position);
#endif

	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return song_position;
}

void
pragha_backend_seek (PraghaBackend *backend, guint64 seek)
{
	CDEBUG(DBG_BACKEND, "Seeking playback");

	if (!backend->seek_enabled)
		return;

	gst_element_seek (backend->pipeline,
	       1.0,
	       GST_FORMAT_TIME,
	       GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH,
	       GST_SEEK_TYPE_SET, seek * GST_SECOND,
	       GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void
pragha_backend_set_soft_volume (PraghaBackend *backend, gboolean value)
{
	GstPlayFlags flags;

	g_object_get (backend->pipeline, "flags", &flags, NULL);

	if (value)
		flags |= GST_PLAY_FLAG_SOFT_VOLUME;
	else
		flags &= ~GST_PLAY_FLAG_SOFT_VOLUME;

	g_object_set (backend->pipeline, "flags", flags, NULL);
}

gdouble
pragha_backend_get_volume (PraghaBackend *backend)
{
	gdouble volume;

	g_object_get (backend->pipeline, "volume", &volume, NULL);

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_LINEAR, VOLUME_FORMAT_CUBIC, volume);
#endif

	return volume;
}

gboolean
update_volume_notify_cb (struct con_win *cwin)
{
	gdouble volume = pragha_backend_get_volume (cwin->backend);

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button), volume);
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);

	return FALSE;
}

static void
volume_notify_cb (GObject *element, GstObject *prop_object, GParamSpec *pspec, struct con_win *cwin)
{
	g_idle_add ((GSourceFunc) update_volume_notify_cb, cwin);
}

void
pragha_backend_set_volume (PraghaBackend *backend, gdouble volume)
{
	struct con_win *cwin = backend->cwin;

	volume = CLAMP (volume, 0.0, 1.0);

	g_signal_handlers_block_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button), volume);
	g_signal_handlers_unblock_by_func (G_OBJECT(cwin->vol_button), vol_button_handler, cwin);

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_CUBIC, VOLUME_FORMAT_LINEAR, volume);
#endif

	/* ignore the deep-notify we get directly from the sink, as it causes deadlock.
	 * we still get another one anyway. */

	g_signal_handlers_block_by_func (backend->pipeline, volume_notify_cb, cwin);
	g_object_set (backend->pipeline, "volume", volume, NULL);
	g_signal_handlers_unblock_by_func (backend->pipeline, volume_notify_cb, cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);
}

void
pragha_backend_set_delta_volume (PraghaBackend *backend, gdouble delta)
{
	gdouble volume = pragha_backend_get_volume (backend);
	volume += delta;
	pragha_backend_set_volume (backend, volume);
}

gboolean
pragha_backend_is_playing (PraghaBackend *backend)
{
	GstState state;
	gst_element_get_state (backend->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PLAYING)
		return TRUE;
	return FALSE;
}

gboolean
pragha_backend_is_paused (PraghaBackend *backend)
{
	GstState state;
	gst_element_get_state(backend->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PAUSED)
		return TRUE;
	return FALSE;
}

void
pragha_backend_stop (PraghaBackend *backend, GError *error)
{
	struct con_win *cwin = backend->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Stopping playback");

	if (cwin->cstate->curr_mobj_clear) {
		delete_musicobject(cwin->cstate->curr_mobj);
		cwin->cstate->curr_mobj_clear = FALSE;
	}

	cwin->cstate->state = ST_STOPPED;

	gst_element_set_state(backend->pipeline, GST_STATE_READY);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, error, cwin);
		gtk_tree_path_free(path);
	}

	update_panel_playback_state(cwin);
	update_menubar_playback_state(cwin);

	update_related_state (cwin);

	dbus_send_signal(DBUS_EVENT_UPDATE_STATE, cwin);

	backend->is_live = FALSE;
	backend->emitted_error = FALSE;
}

void
pragha_backend_pause (PraghaBackend *backend)
{
	struct con_win *cwin = backend->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Pause playback");

	cwin->cstate->state = ST_PAUSED;

	gst_element_set_state(backend->pipeline, GST_STATE_PAUSED);

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
pragha_backend_resume (PraghaBackend *backend)
{
	struct con_win *cwin = backend->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Resuming playback");

	cwin->cstate->state = ST_PLAYING;

	gst_element_set_state(backend->pipeline, GST_STATE_PLAYING);

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

static void
pragha_backend_advance_playback (GError *error, struct con_win *cwin)
{
	GtkTreePath *path = NULL;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_BACKEND, "Advancing to next track");

	/* Stop to set ready and clear all info */
	pragha_backend_stop(cwin->backend, error);

	if(cwin->cstate->playlist_change)
		return;

	/* Get the next track to be played */
	path = current_playlist_get_next (cwin);

	/* No more tracks */
	if (!path)
		return;

	/* Start playing new track */
	mobj = current_playlist_mobj_at_path (path, cwin);
	pragha_backend_start (cwin->backend, mobj);

	update_current_state (path, PLAYLIST_NEXT, cwin);
	gtk_tree_path_free (path);
}

/* Signal handler for parse the error dialog response. */

static void pragha_backend_error_dialog_response(GtkDialog *dialog,
					  gint response,
					  GError *error)
{
	struct con_win *cwin;

	cwin = g_object_get_data (G_OBJECT(dialog), "cwin");

	switch (response) {
		case GTK_RESPONSE_APPLY: {
			pragha_backend_advance_playback (error, cwin);
			break;
		}
		case GTK_RESPONSE_ACCEPT:
		case GTK_RESPONSE_DELETE_EVENT:
		default: {
			pragha_backend_stop (cwin->backend, error);
			break;
		}
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_error_free (error);
}

static void
pragha_backend_parse_error (PraghaBackend *backend, GstMessage *message)
{
	struct con_win *cwin = backend->cwin;
	GtkWidget *dialog;
	gboolean emit = TRUE;
	GError *error = NULL;
	gchar *dbg_info = NULL;

	gst_message_parse_error (message, &error, &dbg_info);

	/* Gstreamer doc: When an error has occured
	 * playbin should be set back to READY or NULL state.
	 */
	gst_element_set_state(backend->pipeline, GST_STATE_NULL);

	/* Next code inspired on rhynthmbox.
	 * If we've already got an error, ignore 'internal data flow error'
	 * type messages, as they're too generic to be helpful.
	 */
	if (backend->emitted_error &&
		error->domain == GST_STREAM_ERROR &&
		error->code == GST_STREAM_ERROR_FAILED) {
		CDEBUG(DBG_BACKEND, "Ignoring generic error \"%s\"", error->message);
		emit = FALSE;
	}

	if(emit) {
		CDEBUG(DBG_BACKEND, "Gstreamer error \"%s\"", error->message);

		backend->emitted_error = TRUE;

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
				 G_CALLBACK(pragha_backend_error_dialog_response), error);
		gtk_widget_show_all(dialog);
	}
	else {
		g_error_free (error);
	}
	g_free (dbg_info);
}

static void
pragha_backend_parse_buffering (PraghaBackend *backend, GstMessage *message)
{
	struct con_win *cwin = backend->cwin;
	gint percent = 0;
	GstState cur_state;

	if (backend->is_live)
		return;

	if(cwin->cstate->state == ST_STOPPED) /* Prevent that buffering overlaps the stop command playing or pausing the playback */
		return;

	gst_message_parse_buffering (message, &percent);
	gst_element_get_state (backend->pipeline, &cur_state, NULL, 0);

	if (percent >= 100) {
		if(cwin->cstate->state == ST_PLAYING && cur_state != GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering complete ... return to playback");
			gst_element_set_state(backend->pipeline, GST_STATE_PLAYING);
		}
	}
	else {
		if (cwin->cstate->state == ST_PLAYING && cur_state == GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering ... temporarily pausing playback");
			gst_element_set_state (backend->pipeline, GST_STATE_PAUSED);
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
pragha_backend_parse_message_tag (PraghaBackend *backend, GstMessage *message)
{
	struct con_win *cwin = backend->cwin;
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
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));

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
pragha_backend_start (PraghaBackend *backend, struct musicobject *mobj)
{
	struct con_win *cwin = backend->cwin;
	CDEBUG(DBG_BACKEND, "Starting playback");

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((cwin->cstate->state == ST_PLAYING) ||
	    (cwin->cstate->state == ST_PAUSED)) {
		pragha_backend_stop(backend, NULL);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->curr_mobj_clear = FALSE;

	pragha_backend_play(backend);
}

void
pragha_backend_play (PraghaBackend *backend)
{
	struct con_win *cwin = backend->cwin;
	gchar *uri = NULL;
	GstStateChangeReturn ret;

	if (!cwin->cstate->curr_mobj->file)
		return;

	CDEBUG(DBG_BACKEND, "Playing: %s", cwin->cstate->curr_mobj->file);

	if(cwin->cstate->curr_mobj->file_type == FILE_CDDA ||
	   cwin->cstate->curr_mobj->file_type == FILE_HTTP) {
		g_object_set(backend->pipeline, "uri", cwin->cstate->curr_mobj->file, NULL);
	}
	else {
		uri = g_filename_to_uri (cwin->cstate->curr_mobj->file, NULL, NULL);
		g_object_set(backend->pipeline, "uri", uri, NULL);
		g_free (uri);
	}

	cwin->cstate->state = ST_PLAYING;

	ret = gst_element_set_state(backend->pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_NO_PREROLL)
		backend->is_live = TRUE;

	update_panel_playback_state (cwin);
	update_menubar_playback_state(cwin);

	update_related_state (cwin);
}

static void
pragha_backend_evaluate_state (GstState old, GstState new, GstState pending, struct con_win *cwin)
{
	if (pending != GST_STATE_VOID_PENDING)
		return;

	switch (new) {
		case GST_STATE_PLAYING: {
			if (cwin->cstate->state == ST_PLAYING) {
				GstQuery *query;
				query = gst_query_new_seeking (GST_FORMAT_TIME);
				if (gst_element_query (cwin->backend->pipeline, query))
					gst_query_parse_seeking (query, NULL, &cwin->backend->seek_enabled, NULL, NULL);
				gst_query_unref (query);

				if(cwin->backend->timer == 0)
					cwin->backend->timer = gdk_threads_add_timeout_seconds (1, update_gui, cwin);

				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_PAUSED: {
			if (cwin->cstate->state == ST_PAUSED) {
				if(cwin->backend->timer > 0) {
					g_source_remove(cwin->backend->timer);
					cwin->backend->timer = 0;
				}
				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_READY:
		case GST_STATE_NULL: {
			if(cwin->backend->timer > 0) {
				g_source_remove(cwin->backend->timer);
				cwin->backend->timer = 0;
			}
			CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			break;
		}
		default:
			break;
	}
}

static gboolean pragha_backend_gstreamer_bus_call(GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			pragha_backend_advance_playback (NULL, cwin);
			break;
		case GST_MESSAGE_STATE_CHANGED: {
			GstState old, new, pending;
			gst_message_parse_state_changed (msg, &old, &new, &pending);
			if (GST_MESSAGE_SRC (msg) == GST_OBJECT (cwin->backend->pipeline))
				pragha_backend_evaluate_state (old, new, pending, cwin);
			break;
		}
		case GST_MESSAGE_BUFFERING: {
			pragha_backend_parse_buffering (cwin->backend, msg);
			break;
		}
		case GST_MESSAGE_TAG: {
			pragha_backend_parse_message_tag (cwin->backend, msg);
			break;
		}
		case GST_MESSAGE_ERROR: {
			pragha_backend_parse_error (cwin->backend, msg);
			break;
		}
		case GST_MESSAGE_CLOCK_LOST: {
			/* Get a new clock */
			gst_element_set_state (cwin->backend->pipeline, GST_STATE_PAUSED);
			gst_element_set_state (cwin->backend->pipeline, GST_STATE_PLAYING);
			break;
		}
    		default:
			break;
	}
	return TRUE;
}

static void
pragha_backend_finalize (GObject *object)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);

	pragha_backend_stop(backend, NULL);

	gst_element_set_state(backend->pipeline, GST_STATE_NULL);
	gst_object_unref(backend->pipeline);

	CDEBUG(DBG_BACKEND, "Pipeline destruction complete");

	G_OBJECT_CLASS (pragha_backend_parent_class)->finalize (object);
}

void
pragha_backend_update_equalizer (PraghaBackend *backend, const gdouble *bands)
{
	g_object_set (backend->equalizer,
			"band0", bands[0],
			"band1", bands[1],
			"band2", bands[2],
			"band3", bands[3],
			"band4", bands[4],
			"band5", bands[5],
			"band6", bands[6],
			"band7", bands[7],
			"band8", bands[8],
			"band9", bands[9],
			NULL);
}

static void
pragha_backend_init_equalizer_preset (struct con_win *cwin)
{
	gdouble *saved_bands;
	GError *error = NULL;

	if(cwin->backend->equalizer == NULL)
		return;

	saved_bands = g_key_file_get_double_list(cwin->cpref->configrc_keyfile,
						 GROUP_AUDIO,
						 KEY_EQ_10_BANDS,
						 NULL,
						 &error);
	if (saved_bands != NULL) {
		pragha_backend_update_equalizer(cwin->backend, saved_bands);
		g_free(saved_bands);
	}
	else {
		g_error_free(error);
		error = NULL;
	}
}

static void
pragha_backend_class_init (PraghaBackendClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = pragha_backend_finalize;
}

static void
pragha_backend_init (PraghaBackend *backend)
{
	/* FIXME */
}

gint backend_init (struct con_win *cwin)
{
	GstBus *bus;
	gchar *audiosink = NULL;
	gboolean can_set_device = TRUE;
	PraghaBackend *backend = g_object_new (PRAGHA_TYPE_BACKEND, NULL);
	backend->cwin = cwin;
	cwin->backend = backend;

	gst_init(NULL, NULL);

#if GST_CHECK_VERSION (1, 0, 0)
	backend->pipeline = gst_element_factory_make("playbin", "playbin");
#else
	backend->pipeline = gst_element_factory_make("playbin2", "playbin");
#endif

	if (backend->pipeline == NULL)
		return -1;

	g_signal_connect (backend->pipeline, "deep-notify::volume",
			  G_CALLBACK (volume_notify_cb), cwin);
	g_signal_connect (backend->pipeline, "notify::source",
			  G_CALLBACK (pragha_backend_source_notify_cb), cwin);

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

	backend->audio_sink = gst_element_factory_make (audiosink, "audio-sink");

	if (backend->audio_sink != NULL) {
		/* Set the audio device to use. */
		if (can_set_device && cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0')
			g_object_set(backend->audio_sink, "device", cwin->cpref->audio_device, NULL);

		/* Test 10bands equalizer and test it. */
		backend->equalizer = gst_element_factory_make ("equalizer-10bands", "equalizer");
		if (backend->equalizer != NULL) {
			GstElement *bin;
			GstPad *pad, *ghost_pad;

			bin = gst_bin_new("audiobin");
			gst_bin_add_many (GST_BIN(bin), backend->equalizer, backend->audio_sink, NULL);
			gst_element_link_many (backend->equalizer, backend->audio_sink, NULL);

			pad = gst_element_get_static_pad (backend->equalizer, "sink");
			ghost_pad = gst_ghost_pad_new ("sink", pad);
			gst_pad_set_active (ghost_pad, TRUE);
			gst_element_add_pad (GST_ELEMENT(bin), ghost_pad);
			gst_object_unref (pad);

			g_object_set(backend->pipeline, "audio-sink", bin, NULL);
		}
		else {
			g_warning("Failed to create the 10bands equalizer element. Not use it.");

			g_object_set(backend->pipeline, "audio-sink", backend->audio_sink, NULL);
		}
	}
	else {
		g_warning("Failed to create audio-sink element. Use default sink, without equalizer. ");

		backend->equalizer = NULL;
		g_object_set(backend->pipeline, "audio-sink", backend->audio_sink, NULL);
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(backend->pipeline));

	gst_bus_add_watch(bus, (GstBusFunc) pragha_backend_gstreamer_bus_call, cwin);

	pragha_backend_set_soft_volume(backend, cwin->cpref->software_mixer);
	pragha_backend_init_equalizer_preset(cwin);

	gst_element_set_state(backend->pipeline, GST_STATE_READY);

	backend->is_live = FALSE;
	backend->seek_enabled = FALSE;
	backend->emitted_error = FALSE;

	gst_object_unref(bus);
	g_free(audiosink);

	CDEBUG(DBG_BACKEND, "Pipeline construction complete");

	return 0;
}
