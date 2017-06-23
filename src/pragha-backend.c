/*************************************************************************/
/* Copyright (C) 2010-2015 matias <mati86dl@gmail.com>                   */
/* Copyright (C) 2012-2013 Pavel Vasin                                   */
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

#include "pragha-backend.h"

#include <glib.h>
#include <stdlib.h>

#if HAVE_GSTREAMER_AUDIO
#include <gst/audio/streamvolume.h>
#endif

#include "pragha-art-cache.h"
#include "pragha-debug.h"
#include "pragha-musicobject.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-utils.h"

static void
pragha_backend_evaluate_state(GstState       old,
                              GstState       new,
                              GstState       pending,
                              PraghaBackend *backend);

#if HAVE_GSTREAMER_AUDIO
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

struct PraghaBackendPrivate {
	PraghaPreferences *preferences;
	PraghaArtCache *art_cache;
	GstElement *pipeline;
	GstElement *audio_sink;
	GstElement *preamp;
	GstElement *equalizer;
	guint timer;
	gboolean is_live;
	gboolean can_seek;
	gboolean seeking; //this is hack, we should catch seek by seqnum, but it's currently broken in gstreamer
	gboolean emitted_error;
	GError *error;
	GstState target_state;
	PraghaBackendState state;
	PraghaMusicobject *mobj;
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
	SIGNAL_SET_DEVICE,
	SIGNAL_PREPARE_SOURCE,
	SIGNAL_CLEAN_SOURCE,
	SIGNAL_TICK,
	SIGNAL_SEEKED,
	SIGNAL_BUFFERING,
	SIGNAL_FINISHED,
	SIGNAL_ERROR,
	SIGNAL_TAGS_CHANGED,
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
pragha_backend_source_notify_cb (GObject *obj, GParamSpec *pspec, PraghaBackend *backend)
{
	GstElement* element;

	g_object_get (obj, "source", &element, NULL);
	if (!element)
		return;

	if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "user-agent")) {
		g_object_set (element, "user-agent", PACKAGE_NAME"/"PACKAGE_VERSION, NULL);
		g_object_set(element, "ssl-use-system-ca-file", FALSE, NULL);
		g_object_set(element, "ssl-strict", FALSE, NULL);
	}

	g_signal_emit (backend, signals[SIGNAL_SET_DEVICE], 0, obj);
}

gint64
pragha_backend_get_current_length (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gint64 song_length;
	gboolean result;

	result = gst_element_query_duration (priv->pipeline, GST_FORMAT_TIME, &song_length);

	if (!result)
		return GST_CLOCK_TIME_NONE;
 
	return song_length;
}

gint64
pragha_backend_get_current_position (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gint64 song_position;
	gboolean result;

	result = gst_element_query_position (priv->pipeline, GST_FORMAT_TIME, &song_position);

	if (!result)
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

static void
pragha_backend_optimize_audio_flags (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	GstPlayFlags flags;

	g_object_get (priv->pipeline, "flags", &flags, NULL);

	/* Disable all video features */
	flags &= ~GST_PLAY_FLAG_VIDEO;
	flags &= ~GST_PLAY_FLAG_TEXT;
	flags &= ~GST_PLAY_FLAG_VIS;
	flags &= ~GST_PLAY_FLAG_NATIVE_VIDEO;

	g_object_set (priv->pipeline, "flags", flags, NULL);
}

gdouble
pragha_backend_get_volume (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gdouble volume;

	g_object_get (priv->pipeline, "volume", &volume, NULL);

#if HAVE_GSTREAMER_AUDIO
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

#if HAVE_GSTREAMER_AUDIO
	volume = convert_volume (VOLUME_FORMAT_CUBIC, VOLUME_FORMAT_LINEAR, volume);
#endif

	g_object_set (priv->pipeline, "volume", volume, NULL);

	if (pragha_preferences_get_software_mixer (priv->preferences))
		pragha_preferences_set_software_volume (priv->preferences, volume);
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
pragha_playback_state_get_name(PraghaBackendState state)
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

PraghaBackendState
pragha_backend_get_state (PraghaBackend *backend)
{
	return backend->priv->state;
}

static void
pragha_backend_set_state (PraghaBackend *backend, PraghaBackendState state)
{
	if (backend->priv->state == state)
		return;

	backend->priv->state = state;

	CDEBUG(DBG_BACKEND, "Setting new playback state: %s: ", pragha_playback_state_get_name(state));

	g_object_notify_by_pspec (G_OBJECT (backend), properties[PROP_STATE]);
}

void
pragha_backend_stop (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	CDEBUG(DBG_BACKEND, "Stopping playback");

	pragha_backend_set_target_state (backend, GST_STATE_READY);

	if(priv->mobj) {
		g_signal_emit (backend, signals[SIGNAL_CLEAN_SOURCE], 0);
		g_object_unref(priv->mobj);
		priv->mobj = NULL;
	}
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
save_embedded_art (PraghaBackend *backend, const GstTagList *taglist)
{
	PraghaBackendPrivate *priv = backend->priv;
	GstSample *sample = NULL;

	gst_tag_list_get_sample_index (taglist, GST_TAG_IMAGE, 0, &sample);
	if (!sample) //try harder
		gst_tag_list_get_sample_index (taglist, GST_TAG_PREVIEW_IMAGE, 0, &sample);

	if (!sample)
		goto out;

	//got art, now check if we need it

	const gchar *artist = pragha_musicobject_get_artist (priv->mobj);
	const gchar *album = pragha_musicobject_get_album (priv->mobj);

	if (pragha_art_cache_contains (priv->art_cache, artist, album))
		goto out;

	//ok, we need it

	GstBuffer *buf = gst_sample_get_buffer (sample);
	if (!buf)
		goto out;

	GstMapInfo info;
	if (!gst_buffer_map (buf, &info, GST_MAP_READ))
		goto out;

	pragha_art_cache_put (priv->art_cache, artist, album, info.data, info.size);

	gst_buffer_unmap (buf, &info);

out:
	if (sample)
		gst_sample_unref (sample);
}

static void
pragha_backend_parse_message_tag (PraghaBackend *backend, GstMessage *message)
{
	PraghaBackendPrivate *priv = backend->priv;
	GstTagList *tag_list;
	gchar *str = NULL;
	gint changed = 0;

	CDEBUG(DBG_BACKEND, "Parse message tag");

	gst_message_parse_tag(message, &tag_list);

	save_embedded_art (backend, tag_list);

	if (pragha_musicobject_get_source (priv->mobj) != FILE_HTTP)
		goto out;

	if (gst_tag_list_get_string(tag_list, GST_TAG_TITLE, &str))
	{
		changed |= TAG_TITLE_CHANGED;
		pragha_musicobject_set_title(priv->mobj, str);
		g_free(str);
	}
	if (gst_tag_list_get_string(tag_list, GST_TAG_ARTIST, &str))
	{
		changed |= TAG_ARTIST_CHANGED;
		pragha_musicobject_set_artist(priv->mobj, str);
		g_free(str);
	}

	g_signal_emit (backend, signals[SIGNAL_TAGS_CHANGED], 0, changed);

out:
	gst_tag_list_free(tag_list);
}

void
pragha_backend_set_playback_uri (PraghaBackend *backend, const gchar *uri)
{
	PraghaBackendPrivate *priv = backend->priv;
	g_object_set (priv->pipeline, "uri", uri, NULL);
}

void
pragha_backend_set_musicobject (PraghaBackend *backend, PraghaMusicobject *mobj)
{
	PraghaBackendPrivate *priv = backend->priv;

	CDEBUG(DBG_BACKEND, "Starting playback");

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((priv->state == ST_PLAYING) ||
	    (priv->state == ST_PAUSED)) {
		pragha_backend_stop(backend);
	}

	priv->mobj = pragha_musicobject_dup(mobj);
}

PraghaMusicobject *
pragha_backend_get_musicobject(PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;

	return priv->mobj;
}

void
pragha_backend_play (PraghaBackend *backend)
{
	PraghaMusicSource file_source = FILE_NONE;
	gchar *file = NULL, *uri = NULL;

	PraghaBackendPrivate *priv = backend->priv;

	g_object_get(priv->mobj,
	             "file", &file,
	             "source", &file_source,
	             NULL);

	if (string_is_empty(file))
		goto exit;

	CDEBUG(DBG_BACKEND, "Playing: %s", file);

	switch (file_source) {
		case FILE_USER_L:
		case FILE_USER_3:
		case FILE_USER_2:
		case FILE_USER_1:
		case FILE_USER_0:
			g_signal_emit (backend, signals[SIGNAL_PREPARE_SOURCE], 0);
			break;
		case FILE_LOCAL:
			uri = g_filename_to_uri (file, NULL, NULL);
			g_object_set (priv->pipeline, "uri", uri, NULL);
			g_free (uri);
			break;
		case FILE_HTTP:
			g_object_set (priv->pipeline, "uri", file, NULL);
			break;
		case FILE_NONE:
		default:
			break;
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
	g_signal_emit (backend, signals[SIGNAL_FINISHED], 0);
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
		g_signal_emit (backend, signals[SIGNAL_TICK], 0);
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
pragha_backend_dispose (GObject *object)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);
	PraghaBackendPrivate *priv = backend->priv;

	if (priv->pipeline) {
		gst_element_set_state (priv->pipeline, GST_STATE_NULL);
		gst_object_unref (priv->pipeline);
		priv->pipeline = NULL;
	}
	if (priv->preferences) {
		g_object_unref (priv->preferences);
		priv->preferences = NULL;
	}
	if (priv->art_cache) {
		g_object_unref (priv->art_cache);
		priv->art_cache = NULL;
	}

	G_OBJECT_CLASS (pragha_backend_parent_class)->dispose (object);
}

static void
pragha_backend_finalize (GObject *object)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);
	PraghaBackendPrivate *priv = backend->priv;

	if (priv->error)
		g_error_free (priv->error);

	CDEBUG(DBG_BACKEND, "Pipeline destruction complete");

	G_OBJECT_CLASS (pragha_backend_parent_class)->finalize (object);
}

GstElement *
pragha_backend_get_equalizer (PraghaBackend *backend)
{
	return backend->priv->equalizer;
}

GstElement *
pragha_backend_get_preamp (PraghaBackend *backend)
{
	return backend->priv->preamp;
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
pragha_backend_init_equalizer_preset (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	gdouble *saved_bands;

	if (priv->equalizer == NULL)
		return;

	saved_bands = pragha_preferences_get_double_list (priv->preferences,
							  GROUP_AUDIO,
							  KEY_EQ_10_BANDS);

	if (saved_bands != NULL) {
		pragha_backend_update_equalizer (backend, saved_bands);
		g_free (saved_bands);
	}
}

#ifndef G_OS_WIN32
static GstElement *
make_audio_sink (PraghaPreferences *preferences)
{
	const gchar *audiosink;
	const gchar *sink_pref = pragha_preferences_get_audio_sink (preferences);

	if (!g_ascii_strcasecmp (sink_pref, ALSA_SINK)) {
		CDEBUG (DBG_BACKEND, "Setting Alsa like audio sink");
		audiosink = "alsasink";
	}
	else if (!g_ascii_strcasecmp (sink_pref, OSS4_SINK)) {
		CDEBUG (DBG_BACKEND, "Setting Oss4 like audio sink");
		audiosink = "oss4sink";
	}
	else if (!g_ascii_strcasecmp (sink_pref, OSS_SINK)) {
		CDEBUG (DBG_BACKEND, "Setting Oss like audio sink");
		audiosink = "osssink";
	}
	else if (!g_ascii_strcasecmp (sink_pref, PULSE_SINK)) {
		CDEBUG (DBG_BACKEND, "Setting Pulseaudio like audio sink");
		audiosink = "pulsesink";
	}
	else {
		CDEBUG (DBG_BACKEND, "Setting autoaudiosink like audio sink");
		audiosink = "autoaudiosink";
	}

	return gst_element_factory_make (audiosink, "audio-sink");
}
#endif

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
	gobject_class->dispose = pragha_backend_dispose;
	gobject_class->finalize = pragha_backend_finalize;

	properties[PROP_VOLUME] =
		g_param_spec_double ("volume", "Volume", "Playback volume.",
		                     0.0, 1.0, 0.5,
		                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_TARGET_STATE] =
		g_param_spec_int ("targetstate", "TargetState", "Playback target state.",
		                  G_MININT, G_MAXINT, 0,
		                  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	properties[PROP_STATE] =
		g_param_spec_int ("state", "State", "Playback state.",
		                  G_MININT, G_MAXINT, 0,
		                  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (gobject_class, PROP_LAST, properties);

	signals[SIGNAL_SET_DEVICE] =
		g_signal_new ("set-device",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, set_device),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[SIGNAL_PREPARE_SOURCE] =
		g_signal_new ("prepare-source",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, prepare_source),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_CLEAN_SOURCE] =
		g_signal_new ("clean-source",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, clean_source),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_TICK] =
		g_signal_new ("tick",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, tick),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_SEEKED] =
		g_signal_new ("seeked",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, seeked),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_BUFFERING] =
		g_signal_new ("buffering",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, buffering),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SIGNAL_FINISHED] =
		g_signal_new ("finished",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, finished),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[SIGNAL_ERROR] =
		g_signal_new ("error",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, error),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[SIGNAL_TAGS_CHANGED] =
		g_signal_new ("tags-changed",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaBackendClass, tags_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__INT,
		              G_TYPE_NONE, 1, G_TYPE_INT);

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
	priv->preferences = pragha_preferences_get ();
	priv->art_cache = pragha_art_cache_get ();

	priv->pipeline = gst_element_factory_make("playbin", "playbin");

	if (priv->pipeline == NULL) {
		g_critical ("Failed to create playbin element. Please check your GStreamer installation.");
		exit (1);
	}

	/* If no audio sink has been specified via the "audio-sink" property, playbin will use the autoaudiosink.
	   Need review then when return the audio preferences. */
	#ifndef G_OS_WIN32
	priv->audio_sink = make_audio_sink (priv->preferences);
	#else
	priv->audio_sink = gst_element_factory_make ("directsoundsink", "audio-sink");
	#endif

	if (priv->audio_sink != NULL) {
		const gchar *audio_device_pref = pragha_preferences_get_audio_device (priv->preferences);
		gboolean can_set_device = g_object_class_find_property (G_OBJECT_GET_CLASS (priv->audio_sink), "device") != NULL;

		/* Set the audio device to use. */
		if (can_set_device && string_is_not_empty (audio_device_pref))
			g_object_set (priv->audio_sink, "device", audio_device_pref, NULL);

		/* Test 10bands equalizer and test it. */
		priv->equalizer = gst_element_factory_make ("equalizer-10bands", "equalizer");
		priv->preamp = gst_element_factory_make ("volume", "preamp");

		if (priv->equalizer != NULL && priv->preamp != NULL) {
			GstElement *bin;
			GstPad *pad, *ghost_pad;

			bin = gst_bin_new ("audiobin");
			gst_bin_add_many (GST_BIN(bin), priv->preamp, priv->equalizer, priv->audio_sink, NULL);
			gst_element_link_many (priv->preamp, priv->equalizer, priv->audio_sink, NULL);

			pad = gst_element_get_static_pad (priv->preamp, "sink");
			ghost_pad = gst_ghost_pad_new ("sink", pad);
			gst_pad_set_active (ghost_pad, TRUE);
			gst_element_add_pad (bin, ghost_pad);
			gst_object_unref (pad);

			g_object_set (priv->pipeline, "audio-sink", bin, NULL);
		} else {
			g_warning ("Failed to create the 10bands equalizer element. Not use it.");

			g_object_set (priv->pipeline, "audio-sink", priv->audio_sink, NULL);
		}
	} else {
		if (priv->equalizer) {
			g_object_unref(priv->equalizer);
			priv->equalizer = NULL;
		}
		if (priv->preamp) {
			g_object_unref(priv->preamp);
			priv->preamp = NULL;
		}
		g_warning ("Failed to create audio-sink element. Use default sink, without equalizer.");

		g_object_set (priv->pipeline, "audio-sink", priv->audio_sink, NULL);
	}

	/* Disable all video features */
	pragha_backend_optimize_audio_flags(backend);

	GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (priv->pipeline));
	gst_bus_add_signal_watch (bus);
	g_signal_connect (bus, "message::error", G_CALLBACK (pragha_backend_message_error), backend);
	g_signal_connect (bus, "message::eos", G_CALLBACK (pragha_backend_message_eos), backend);
	g_signal_connect (bus, "message::state-changed", G_CALLBACK (pragha_backend_message_state_changed), backend);
	g_signal_connect (bus, "message::async-done", G_CALLBACK (pragha_backend_message_async_done), backend);
	g_signal_connect (bus, "message::buffering", G_CALLBACK (pragha_backend_message_buffering), backend);
	g_signal_connect (bus, "message::clock-lost", G_CALLBACK (pragha_backend_message_clock_lost), backend);
	g_signal_connect (bus, "message::tag", G_CALLBACK (pragha_backend_message_tag), backend);
	gst_object_unref (bus);

	if(pragha_preferences_get_software_mixer (priv->preferences)) {
		pragha_backend_set_soft_volume (backend, TRUE);
		pragha_backend_set_volume(backend, pragha_preferences_get_software_volume (priv->preferences));
	}
	pragha_backend_init_equalizer_preset (backend);

	//notify::volume is emitted from gstreamer worker thread
	g_signal_connect (priv->pipeline, "notify::volume",
			  G_CALLBACK (volume_notify_cb), backend);
	g_signal_connect (priv->pipeline, "notify::source",
			  G_CALLBACK (pragha_backend_source_notify_cb), backend);

	gst_element_set_state (priv->pipeline, GST_STATE_READY);

	CDEBUG (DBG_BACKEND, "Pipeline construction completed");
}

PraghaBackend *
pragha_backend_new (void)
{
	gst_init (NULL, NULL);

	PraghaBackend *backend = g_object_new (PRAGHA_TYPE_BACKEND, NULL);

	return backend;
}
