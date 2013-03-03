/*************************************************************************/
/* Copyright (C) 2010-2012 matias <mati86dl@gmail.com>			 */
/* Copyright (C) 2012-2013 Pavel Vasin					 */
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
#include "pragha-playback.h"

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

static void pragha_backend_evaluate_state(GstState old,
					  GstState new,
					  GstState pending,
					  PraghaBackend *backend);

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

struct PraghaBackendPrivate {
	struct con_win *cwin;
	GstElement *pipeline;
	GstElement *audio_sink;
	GstElement *equalizer;
	guint timer;
	gboolean is_live;
	gboolean can_seek;
	gboolean seeking; //this is hack, we should catch seek by seqnum, but it's currently broken in gstreamer
	gboolean emitted_error;
	GError *error;
	GstState target_state;
	enum player_state state;
};

#define PRAGHA_BACKEND_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PRAGHA_TYPE_BACKEND, PraghaBackendPrivate))

enum {
	PROP_0,
	PROP_VOLUME,
	PROP_TARGET_STATE,
	PROP_STATE,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { 0 };

enum {
	SIGNAL_TICK,
	SIGNAL_SEEKED,
	SIGNAL_BUFFERING,
	SIGNAL_ERROR,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (PraghaBackend, pragha_backend, G_TYPE_OBJECT);

static gboolean
emit_tick_cb (gpointer user_data)
{
	PraghaBackend *backend = user_data;

	g_signal_emit (backend, signals[SIGNAL_TICK], 0);

	return TRUE;
}

static void
pragha_backend_source_notify_cb (GObject *obj, GParamSpec *pspec, struct con_win *cwin)
{
	GObject *source;
	gint file_type = 0;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	file_type = pragha_musicobject_get_file_type(cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	if(file_type != FILE_CDDA)
		return;

	g_object_get (obj, "source", &source, NULL);

	if (source) {
		const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device(cwin->preferences);
		if (audio_cd_device) {
			g_object_set (source,  "device", audio_cd_device, NULL);
		}
		g_object_unref (source);
	}
}

gint64
pragha_backend_get_current_length (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gint64 song_length;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_duration(priv->pipeline, format, &song_length);
#else
	result = gst_element_query_duration(priv->pipeline, &format, &song_length);
#endif

	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;
 
	return song_length;
}

gint64
pragha_backend_get_current_position (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gint64 song_position;
	gboolean result;
	GstFormat format = GST_FORMAT_TIME;

#if GST_CHECK_VERSION (1, 0, 0)
	result = gst_element_query_position(priv->pipeline, format, &song_position);
#else
	result = gst_element_query_position(priv->pipeline, &format, &song_position);
#endif

	if (!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return song_position;
}

gboolean
pragha_backend_can_seek (PraghaBackend *backend)
{
	return backend->priv->can_seek;
}

void
pragha_backend_seek (PraghaBackend *backend, gint64 seek)
{
	PraghaBackendPrivate *priv = backend->priv;

	if (!priv->can_seek)
		return;

	CDEBUG(DBG_BACKEND, "Seeking playback");

	gboolean success = gst_element_seek (priv->pipeline,
	                                     1.0,
	                                     GST_FORMAT_TIME,
	                                     GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH,
	                                     GST_SEEK_TYPE_SET, seek * GST_SECOND,
	                                     GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

	if (success)
		priv->seeking = TRUE;
}

void
pragha_backend_set_soft_volume (PraghaBackend *backend, gboolean value)
{
	PraghaBackendPrivate *priv = backend->priv;
	GstPlayFlags flags;

	g_object_get (priv->pipeline, "flags", &flags, NULL);

	if (value)
		flags |= GST_PLAY_FLAG_SOFT_VOLUME;
	else
		flags &= ~GST_PLAY_FLAG_SOFT_VOLUME;

	g_object_set (priv->pipeline, "flags", flags, NULL);
}

gdouble
pragha_backend_get_volume (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gdouble volume;

	g_object_get (priv->pipeline, "volume", &volume, NULL);

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_LINEAR, VOLUME_FORMAT_CUBIC, volume);
#endif

	return volume;
}

static gboolean
emit_volume_notify_cb (gpointer user_data)
{
	PraghaBackend *backend = user_data;

	g_object_notify_by_pspec (G_OBJECT (backend), properties[PROP_VOLUME]);

	return FALSE;
}

static void
volume_notify_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	g_idle_add (emit_volume_notify_cb, user_data);
}

void
pragha_backend_set_volume (PraghaBackend *backend, gdouble volume)
{
	PraghaBackendPrivate *priv = backend->priv;

	volume = CLAMP (volume, 0.0, 1.0);

#if HAVE_GSTREAMER_AUDIO || HAVE_GSTREAMER_INTERFACES
	volume = convert_volume (VOLUME_FORMAT_CUBIC, VOLUME_FORMAT_LINEAR, volume);
#endif

	g_object_set (priv->pipeline, "volume", volume, NULL);
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
	PraghaBackendPrivate *priv = backend->priv;
	GstState state;
	gst_element_get_state (priv->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PLAYING)
		return TRUE;
	return FALSE;
}

gboolean
pragha_backend_is_paused (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	GstState state;
	gst_element_get_state(priv->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);

	if (state == GST_STATE_PAUSED)
		return TRUE;
	return FALSE;
}

gboolean
pragha_backend_emitted_error (PraghaBackend *backend)
{
	return backend->priv->emitted_error;
}

GError *
pragha_backend_get_error (PraghaBackend *backend)
{
	return backend->priv->error;
}

GstState
pragha_backend_get_target_state (PraghaBackend *backend)
{
	return backend->priv->target_state;
}

static void
pragha_backend_set_target_state (PraghaBackend *backend, GstState target_state)
{
	GstStateChangeReturn ret;
	PraghaBackendPrivate *priv = backend->priv;

	GstState old_state = priv->target_state;
	priv->target_state = target_state;

	ret = gst_element_set_state(priv->pipeline, target_state);

	switch (ret) {
		case GST_STATE_CHANGE_SUCCESS:
			if (target_state == GST_STATE_READY)
				pragha_backend_evaluate_state(old_state,
							      GST_STATE (priv->pipeline),
							      GST_STATE_PENDING (priv->pipeline),
							      backend);
			break;
		case GST_STATE_CHANGE_NO_PREROLL:
			if (target_state == GST_STATE_PLAYING)
				priv->is_live = TRUE;
			break;
		default:
			break;
	}

	g_object_notify_by_pspec (G_OBJECT (backend), properties[PROP_TARGET_STATE]);
}

const gchar *
pragha_playback_state_get_name(enum player_state state)
{
	switch (state) {
		case ST_PLAYING:
			return "ST_PLAYING";
		case ST_STOPPED:
			return "ST_STOPPED";
		case ST_PAUSED:
			return "ST_PAUSED";
		default:
			/* This is a memory leak */
			return g_strdup_printf ("UNKNOWN!(%d)", state);
	}
}

enum player_state
pragha_backend_get_state (PraghaBackend *backend)
{
	return backend->priv->state;
}

static void
pragha_backend_set_state (PraghaBackend *backend, enum player_state state)
{
	backend->priv->state = state;

	CDEBUG(DBG_BACKEND, "Setting new playback state: %s: ", pragha_playback_state_get_name(state));

	g_object_notify_by_pspec (G_OBJECT (backend), properties[PROP_STATE]);
}

void
pragha_backend_stop (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;

	CDEBUG(DBG_BACKEND, "Stopping playback");

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	if(cwin->cstate->curr_mobj)
		g_object_unref(cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	pragha_backend_set_target_state (backend, GST_STATE_READY);
}

void
pragha_backend_pause (PraghaBackend *backend)
{
	CDEBUG(DBG_BACKEND, "Pause playback");

	pragha_backend_set_target_state (backend, GST_STATE_PAUSED);
}

void
pragha_backend_resume (PraghaBackend *backend)
{
	CDEBUG(DBG_BACKEND, "Resuming playback");

	pragha_backend_set_target_state (backend, GST_STATE_PLAYING);
}

static void
pragha_backend_parse_error (PraghaBackend *backend, GstMessage *message)
{
	PraghaBackendPrivate *priv = backend->priv;
	gboolean emit = TRUE;
	GError *error = NULL;
	gchar *dbg_info = NULL;

	gst_message_parse_error (message, &error, &dbg_info);

	/* Gstreamer doc: When an error has occured
	 * playbin should be set back to READY or NULL state.
	 */
	gst_element_set_state(priv->pipeline, GST_STATE_NULL);

	/* Next code inspired on rhynthmbox.
	 * If we've already got an error, ignore 'internal data flow error'
	 * type messages, as they're too generic to be helpful.
	 */
	if (priv->emitted_error &&
		error->domain == GST_STREAM_ERROR &&
		error->code == GST_STREAM_ERROR_FAILED) {
		CDEBUG(DBG_BACKEND, "Ignoring generic error \"%s\"", error->message);
		emit = FALSE;
	}

	if (emit) {
		CDEBUG(DBG_BACKEND, "Gstreamer error \"%s\"", error->message);

		priv->emitted_error = TRUE;
		priv->error = error;

		g_signal_emit (backend, signals[SIGNAL_ERROR], 0, error);
	}
	else {
		g_error_free (error);
	}
	g_free (dbg_info);
}

static void
pragha_backend_parse_buffering (PraghaBackend *backend, GstMessage *message)
{
	PraghaBackendPrivate *priv = backend->priv;
	gint percent = 0;
	GstState cur_state;

	if (priv->is_live)
		return;

	if (priv->target_state == GST_STATE_READY) /* Prevent that buffering overlaps the stop command playing or pausing the playback */
		return;

	gst_message_parse_buffering (message, &percent);
	gst_element_get_state (priv->pipeline, &cur_state, NULL, 0);

	if (percent >= 100) {
		if (priv->target_state == GST_STATE_PLAYING && cur_state != GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering complete ... return to playback");
			gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);
		}
	}
	else {
		if (priv->target_state == GST_STATE_PLAYING && cur_state == GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering ... temporarily pausing playback");
			gst_element_set_state (priv->pipeline, GST_STATE_PAUSED);
		}
		else {
			CDEBUG(DBG_BACKEND, "Buffering (already paused) ... %d", percent);
		}
	}

	g_signal_emit (backend, signals[SIGNAL_BUFFERING], 0, percent);
}

static void
pragha_backend_parse_message_tag (PraghaBackend *backend, GstMessage *message)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	GstTagList *tag_list;
	PraghaMusicobject *nmobj;
	gchar *str = NULL;
	gint changed = 0, file_type = 0;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	file_type = pragha_musicobject_get_file_type(cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	if(file_type != FILE_HTTP)
		return;

	CDEBUG(DBG_BACKEND, "Parse message tag");

	nmobj = pragha_musicobject_new();

	gst_message_parse_tag(message, &tag_list);

	if (gst_tag_list_get_string(tag_list, GST_TAG_TITLE, &str))
	{
		changed |= TAG_TITLE_CHANGED;
		pragha_musicobject_set_title(nmobj, str);
		g_free(str);
	}
	if (gst_tag_list_get_string(tag_list, GST_TAG_ARTIST, &str))
	{
		changed |= TAG_ARTIST_CHANGED;
		pragha_musicobject_set_artist(nmobj, str);
		g_free(str);
	}

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	pragha_update_musicobject_change_tag(cwin->cstate->curr_mobj, changed, nmobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	__update_current_song_info(cwin);
	mpris_update_metadata_changed(cwin);

	pragha_playlist_update_current_track(cwin->cplaylist, changed);

	gst_tag_list_free(tag_list);
}

void
pragha_backend_start (PraghaBackend *backend,PraghaMusicobject *mobj)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	CDEBUG(DBG_BACKEND, "Starting playback");

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((priv->state == ST_PLAYING) ||
	    (priv->state == ST_PAUSED)) {
		pragha_backend_stop(backend);
	}

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	cwin->cstate->curr_mobj = g_object_ref(mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	pragha_backend_play(backend);
}

void
pragha_backend_play (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	gchar *file = NULL, *uri = NULL;
	gboolean local_file;

	pragha_mutex_lock (cwin->cstate->curr_mobj_mutex);
	g_object_get(cwin->cstate->curr_mobj,
	             "file", &file,
	             NULL);
	local_file = pragha_musicobject_is_local_file (cwin->cstate->curr_mobj);
	pragha_mutex_unlock (cwin->cstate->curr_mobj_mutex);

	if (string_is_empty(file))
		goto exit;

	CDEBUG(DBG_BACKEND, "Playing: %s", file);

	if (local_file) {
		uri = g_filename_to_uri (file, NULL, NULL);
		g_object_set (priv->pipeline, "uri", uri, NULL);
		g_free (uri);
	}
	else {
		g_object_set (priv->pipeline, "uri", file, NULL);
	}

	pragha_backend_set_target_state (backend, GST_STATE_PLAYING);

exit:
	g_free(file);
}

static void
pragha_backend_evaluate_if_can_seek(PraghaBackend *backend)
{
	GstQuery *query;

	PraghaBackendPrivate *priv = backend->priv;

	query = gst_query_new_seeking (GST_FORMAT_TIME);
	if (gst_element_query (priv->pipeline, query))
		gst_query_parse_seeking (query, NULL, &priv->can_seek, NULL, NULL);
	gst_query_unref (query);
}

static void
pragha_backend_evaluate_state (GstState old, GstState new, GstState pending, PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	if (pending != GST_STATE_VOID_PENDING)
		return;

	CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));

	switch (new) {
		case GST_STATE_PLAYING: {
			if (priv->target_state == GST_STATE_PLAYING) {
				pragha_backend_evaluate_if_can_seek(backend);

				if (priv->timer == 0)
					priv->timer = g_timeout_add_seconds (1, emit_tick_cb, backend);

				pragha_backend_set_state (backend, ST_PLAYING);
			}
			break;
		}
		case GST_STATE_PAUSED: {
			if (priv->target_state == GST_STATE_PAUSED) {
				if (priv->timer > 0) {
					g_source_remove(priv->timer);
					priv->timer = 0;
				}

				pragha_backend_set_state (backend, ST_PAUSED);
			}
			break;
		}
		case GST_STATE_READY:
			if (priv->target_state == GST_STATE_READY) {
				pragha_backend_set_state (backend, ST_STOPPED);

				priv->is_live = FALSE;
				priv->emitted_error = FALSE;
				g_clear_error(&priv->error);
				priv->seeking = FALSE;
			}
		case GST_STATE_NULL: {
			if (priv->timer > 0) {
				g_source_remove(priv->timer);
				priv->timer = 0;
			}
			break;
		}
		default:
			break;
	}
}

static void
pragha_backend_message_error (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	pragha_backend_parse_error (backend, msg);
}

static void
pragha_backend_message_eos (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	pragha_advance_playback (priv->cwin);
}

static void
pragha_backend_message_state_changed (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	GstState old, new, pending;

	PraghaBackendPrivate *priv = backend->priv;

	gst_message_parse_state_changed (msg, &old, &new, &pending);
	if (GST_MESSAGE_SRC (msg) == GST_OBJECT (priv->pipeline))
		pragha_backend_evaluate_state (old, new, pending, backend);
}

static void
pragha_backend_message_async_done (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	if (priv->seeking) {
		priv->seeking = FALSE;
		g_signal_emit (backend, signals[SIGNAL_SEEKED], 0);
	}
}

static void
pragha_backend_message_buffering (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	pragha_backend_parse_buffering (backend, msg);
}

static void
pragha_backend_message_clock_lost (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	gst_element_set_state (priv->pipeline, GST_STATE_PAUSED);
	gst_element_set_state (priv->pipeline, GST_STATE_PLAYING);
}

static void
pragha_backend_message_tag (GstBus *bus, GstMessage *msg, PraghaBackend *backend)
{
	pragha_backend_parse_message_tag (backend, msg);
}

static void
pragha_backend_finalize (GObject *object)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);
	PraghaBackendPrivate *priv = backend->priv;

	gst_element_set_state(priv->pipeline, GST_STATE_NULL);
	gst_object_unref(priv->pipeline);
	if(priv->error)
		g_error_free(priv->error);

	CDEBUG(DBG_BACKEND, "Pipeline destruction complete");

	G_OBJECT_CLASS (pragha_backend_parent_class)->finalize (object);
}

GstElement *
pragha_backend_get_equalizer (PraghaBackend *backend)
{
	return backend->priv->equalizer;
}

void
pragha_backend_update_equalizer (PraghaBackend *backend, const gdouble *bands)
{
	PraghaBackendPrivate *priv = backend->priv;

	g_object_set (priv->equalizer,
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
	PraghaBackendPrivate *priv = cwin->backend->priv;
	gdouble *saved_bands;

	if (priv->equalizer == NULL)
		return;

	saved_bands = pragha_preferences_get_double_list(cwin->preferences,
							 GROUP_AUDIO,
							 KEY_EQ_10_BANDS);

	if (saved_bands != NULL) {
		pragha_backend_update_equalizer(cwin->backend, saved_bands);
		g_free(saved_bands);
	}
}

static void
pragha_backend_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);

	switch (property_id)
	{
		case PROP_VOLUME:
			pragha_backend_set_volume (backend, g_value_get_double (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_backend_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);

	switch (property_id)
	{
		case PROP_VOLUME:
			g_value_set_double (value, pragha_backend_get_volume (backend));
			break;

		case PROP_TARGET_STATE:
			g_value_set_int (value, pragha_backend_get_target_state (backend));
			break;

		case PROP_STATE:
			g_value_set_int (value, pragha_backend_get_state (backend));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
pragha_backend_class_init (PraghaBackendClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->set_property = pragha_backend_set_property;
	gobject_class->get_property = pragha_backend_get_property;
	gobject_class->finalize = pragha_backend_finalize;

	properties[PROP_VOLUME] = g_param_spec_double ("volume", "Volume", "Playback volume.",
                                                       0.0, 1.0, 0.5,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS);

	properties[PROP_TARGET_STATE] = g_param_spec_int ("targetstate", "TargetState", "Playback target state.",
                                                   G_MININT, G_MAXINT, 0,
                                                   G_PARAM_READABLE |
                                                   G_PARAM_STATIC_STRINGS);

	properties[PROP_STATE] = g_param_spec_int ("state", "State", "Playback state.",
                                                   G_MININT, G_MAXINT, 0,
                                                   G_PARAM_READABLE |
                                                   G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class, PROP_LAST, properties);

	signals[SIGNAL_TICK] = g_signal_new ("tick",
                                             G_TYPE_FROM_CLASS (gobject_class),
                                             G_SIGNAL_RUN_LAST,
                                             G_STRUCT_OFFSET (PraghaBackendClass, tick),
                                             NULL, NULL,
                                             g_cclosure_marshal_VOID__VOID,
                                             G_TYPE_NONE, 0);

	signals[SIGNAL_SEEKED] = g_signal_new ("seeked",
                                               G_TYPE_FROM_CLASS (gobject_class),
                                               G_SIGNAL_RUN_LAST,
                                               G_STRUCT_OFFSET (PraghaBackendClass, seeked),
                                               NULL, NULL,
                                               g_cclosure_marshal_VOID__VOID,
                                               G_TYPE_NONE, 0);

	signals[SIGNAL_BUFFERING] = g_signal_new ("buffering",
                                                  G_TYPE_FROM_CLASS (gobject_class),
                                                  G_SIGNAL_RUN_LAST,
                                                  G_STRUCT_OFFSET (PraghaBackendClass, buffering),
                                                  NULL, NULL,
                                                  g_cclosure_marshal_VOID__INT,
                                                  G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SIGNAL_ERROR] = g_signal_new ("error",
                                              G_TYPE_FROM_CLASS (gobject_class),
                                              G_SIGNAL_RUN_LAST,
                                              G_STRUCT_OFFSET (PraghaBackendClass, error),
                                              NULL, NULL,
                                              g_cclosure_marshal_VOID__POINTER,
                                              G_TYPE_NONE, 1, G_TYPE_POINTER);

	g_type_class_add_private (klass, sizeof (PraghaBackendPrivate));
}

static void
pragha_backend_init (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = PRAGHA_BACKEND_GET_PRIVATE (backend);
	backend->priv = priv;
	priv->target_state = GST_STATE_READY;
	priv->state = ST_STOPPED;
	priv->is_live = FALSE;
	priv->can_seek = FALSE;
	priv->seeking = FALSE;
	priv->emitted_error = FALSE;
	priv->error = NULL;
	/* FIXME */
}

gint backend_init (struct con_win *cwin)
{
	GstBus *bus;
	const gchar *audiosink = NULL;
	gboolean can_set_device = TRUE;
	PraghaBackend *backend = g_object_new (PRAGHA_TYPE_BACKEND, NULL);
	PraghaBackendPrivate *priv = backend->priv;
	priv->cwin = cwin;
	cwin->backend = backend;

	gst_init(NULL, NULL);

#if GST_CHECK_VERSION (1, 0, 0)
	priv->pipeline = gst_element_factory_make("playbin", "playbin");
#else
	priv->pipeline = gst_element_factory_make("playbin2", "playbin");
#endif

	if (priv->pipeline == NULL)
		return -1;

	//notify::volume is emitted from gstreamer worker thread
	g_signal_connect (priv->pipeline, "notify::volume",
			  G_CALLBACK (volume_notify_cb), backend);
	g_signal_connect (priv->pipeline, "notify::source",
			  G_CALLBACK (pragha_backend_source_notify_cb), cwin);

	/* If no audio sink has been specified via the "audio-sink" property, playbin will use the autoaudiosink.
	   Need review then when return the audio preferences. */

	const gchar *sink_pref = pragha_preferences_get_audio_sink(cwin->preferences);

	if (!g_ascii_strcasecmp(sink_pref, ALSA_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Alsa like audio sink");
		audiosink = "alsasink";
	}
	else if (!g_ascii_strcasecmp(sink_pref, OSS4_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss4 like audio sink");
		audiosink = "oss4sink";
	}
	else if (!g_ascii_strcasecmp(sink_pref, OSS_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss like audio sink");
		audiosink = "osssink";
	}
	else if (!g_ascii_strcasecmp(sink_pref, PULSE_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Pulseaudio like audio sink");
		audiosink = "pulsesink";
	}
	else {
		CDEBUG(DBG_BACKEND, "Setting autoaudiosink like audio sink");
		can_set_device = FALSE;
		audiosink = "autoaudiosink";
	}

	priv->audio_sink = gst_element_factory_make (audiosink, "audio-sink");

	const gchar *audio_device_pref = pragha_preferences_get_audio_device(cwin->preferences);

	if (priv->audio_sink != NULL) {
		/* Set the audio device to use. */
		if (can_set_device && string_is_not_empty(audio_device_pref))
			g_object_set(priv->audio_sink, "device", audio_device_pref, NULL);

		/* Test 10bands equalizer and test it. */
		priv->equalizer = gst_element_factory_make ("equalizer-10bands", "equalizer");
		if (priv->equalizer != NULL) {
			GstElement *bin;
			GstPad *pad, *ghost_pad;

			bin = gst_bin_new("audiobin");
			gst_bin_add_many (GST_BIN(bin), priv->equalizer, priv->audio_sink, NULL);
			gst_element_link_many (priv->equalizer, priv->audio_sink, NULL);

			pad = gst_element_get_static_pad (priv->equalizer, "sink");
			ghost_pad = gst_ghost_pad_new ("sink", pad);
			gst_pad_set_active (ghost_pad, TRUE);
			gst_element_add_pad (GST_ELEMENT(bin), ghost_pad);
			gst_object_unref (pad);

			g_object_set(priv->pipeline, "audio-sink", bin, NULL);
		}
		else {
			g_warning("Failed to create the 10bands equalizer element. Not use it.");

			g_object_set(priv->pipeline, "audio-sink", priv->audio_sink, NULL);
		}
	}
	else {
		g_warning("Failed to create audio-sink element. Use default sink, without equalizer. ");

		priv->equalizer = NULL;
		g_object_set(priv->pipeline, "audio-sink", priv->audio_sink, NULL);
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(priv->pipeline));

	gst_bus_add_signal_watch(bus);
	g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)pragha_backend_message_error, backend);
	g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback)pragha_backend_message_eos, backend);
	g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)pragha_backend_message_state_changed, backend);
	g_signal_connect(G_OBJECT(bus), "message::async-done", (GCallback)pragha_backend_message_async_done, backend);
	g_signal_connect(G_OBJECT(bus), "message::buffering", (GCallback)pragha_backend_message_buffering, backend);
	g_signal_connect(G_OBJECT(bus), "message::clock-lost", (GCallback)pragha_backend_message_clock_lost, backend);
	g_signal_connect(G_OBJECT(bus), "message::tag", (GCallback)pragha_backend_message_tag, backend);
	gst_object_unref (bus);

	pragha_backend_set_soft_volume(backend, pragha_preferences_get_software_mixer(cwin->preferences));
	pragha_backend_init_equalizer_preset(cwin);

	gst_element_set_state(priv->pipeline, GST_STATE_READY);

	CDEBUG(DBG_BACKEND, "Pipeline construction complete");

	return 0;
}
