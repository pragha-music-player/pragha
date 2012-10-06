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
	enum player_state state;
};

#define PRAGHA_BACKEND_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PRAGHA_TYPE_BACKEND, PraghaBackendPrivate))

enum {
	PROP_0,
	PROP_VOLUME,
	PROP_STATE,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { 0 };

enum {
	SIGNAL_TICK,
	SIGNAL_SEEKED,
	SIGNAL_BUFFERING,
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

enum player_state
pragha_backend_get_state (PraghaBackend *backend)
{
	return backend->priv->state;
}

static void
pragha_backend_set_state (PraghaBackend *backend, enum player_state state)
{
	backend->priv->state = state;
	g_object_notify_by_pspec (G_OBJECT (backend), properties[PROP_STATE]);
}

void
pragha_backend_stop (PraghaBackend *backend, GError *error)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Stopping playback");

	if (cwin->cstate->curr_mobj_clear) {
		delete_musicobject(cwin->cstate->curr_mobj);
		cwin->cstate->curr_mobj_clear = FALSE;
	}

	pragha_backend_set_state (backend, ST_STOPPED);

	gst_element_set_state(priv->pipeline, GST_STATE_READY);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, error, cwin);
		gtk_tree_path_free(path);
	}

	priv->is_live = FALSE;
	priv->emitted_error = FALSE;
	priv->seeking = FALSE;
}

void
pragha_backend_pause (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Pause playback");

	pragha_backend_set_state (backend, ST_PAUSED);

	gst_element_set_state(priv->pipeline, GST_STATE_PAUSED);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
}

void
pragha_backend_resume (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	GtkTreePath *path = NULL;

	CDEBUG(DBG_BACKEND, "Resuming playback");

	pragha_backend_set_state (backend, ST_PLAYING);

	gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);

	path = current_playlist_get_actual(cwin);
	if (path) {
		update_pixbuf_state_on_path(path, NULL, cwin);
		gtk_tree_path_free(path);
	}
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
			pragha_advance_playback (error, cwin);
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
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	GtkWidget *dialog;
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
	PraghaBackendPrivate *priv = backend->priv;
	gint percent = 0;
	GstState cur_state;

	if (priv->is_live)
		return;

	if (priv->state == ST_STOPPED) /* Prevent that buffering overlaps the stop command playing or pausing the playback */
		return;

	gst_message_parse_buffering (message, &percent);
	gst_element_get_state (priv->pipeline, &cur_state, NULL, 0);

	if (percent >= 100) {
		if (priv->state == ST_PLAYING && cur_state != GST_STATE_PLAYING) {
			CDEBUG(DBG_BACKEND, "Buffering complete ... return to playback");
			gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);
		}
	}
	else {
		if (priv->state == ST_PLAYING && cur_state == GST_STATE_PLAYING) {
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
		ntag.title = str;
	}
	if (gst_tag_list_get_string(tag_list, GST_TAG_ARTIST, &str))
	{
		changed |= TAG_ARTIST_CHANGED;
		ntag.artist = str;
	}

	update_musicobject(cwin->cstate->curr_mobj, changed, &ntag);
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
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	CDEBUG(DBG_BACKEND, "Starting playback");

	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((priv->state == ST_PLAYING) ||
	    (priv->state == ST_PAUSED)) {
		pragha_backend_stop(backend, NULL);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->curr_mobj_clear = FALSE;

	pragha_backend_play(backend);
}

void
pragha_backend_play (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = backend->priv;
	struct con_win *cwin = priv->cwin;
	gchar *uri = NULL;
	GstStateChangeReturn ret;

	if (!cwin->cstate->curr_mobj->file)
		return;

	CDEBUG(DBG_BACKEND, "Playing: %s", cwin->cstate->curr_mobj->file);

	if(cwin->cstate->curr_mobj->file_type == FILE_CDDA ||
	   cwin->cstate->curr_mobj->file_type == FILE_HTTP) {
		g_object_set(priv->pipeline, "uri", cwin->cstate->curr_mobj->file, NULL);
	}
	else {
		uri = g_filename_to_uri (cwin->cstate->curr_mobj->file, NULL, NULL);
		g_object_set(priv->pipeline, "uri", uri, NULL);
		g_free (uri);
	}

	pragha_backend_set_state (backend, ST_PLAYING);

	ret = gst_element_set_state(priv->pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_NO_PREROLL)
		priv->is_live = TRUE;
}

static void
pragha_backend_evaluate_state (GstState old, GstState new, GstState pending, struct con_win *cwin)
{
	PraghaBackendPrivate *priv = cwin->backend->priv;

	if (pending != GST_STATE_VOID_PENDING)
		return;

	switch (new) {
		case GST_STATE_PLAYING: {
			if (priv->state == ST_PLAYING) {
				GstQuery *query;
				query = gst_query_new_seeking (GST_FORMAT_TIME);
				if (gst_element_query (priv->pipeline, query))
					gst_query_parse_seeking (query, NULL, &priv->can_seek, NULL, NULL);
				gst_query_unref (query);

				if (priv->timer == 0)
					priv->timer = g_timeout_add_seconds (1, emit_tick_cb, cwin->backend);

				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_PAUSED: {
			if (priv->state == ST_PAUSED) {
				if (priv->timer > 0) {
					g_source_remove(priv->timer);
					priv->timer = 0;
				}
				CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			}
			break;
		}
		case GST_STATE_READY:
		case GST_STATE_NULL: {
			if (priv->timer > 0) {
				g_source_remove(priv->timer);
				priv->timer = 0;
			}
			CDEBUG(DBG_BACKEND, "Gstreamer inform the state change: %s", gst_element_state_get_name (new));
			break;
		}
		default:
			break;
	}
}

static void
pragha_backend_message_error (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	pragha_backend_parse_error (cwin->backend, msg);
}

static void
pragha_backend_message_eos (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	pragha_advance_playback (NULL, cwin);
}

static void
pragha_backend_message_state_changed (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	GstState old, new, pending;

	PraghaBackendPrivate *priv = cwin->backend->priv;

	gst_message_parse_state_changed (msg, &old, &new, &pending);
	if (GST_MESSAGE_SRC (msg) == GST_OBJECT (priv->pipeline))
		pragha_backend_evaluate_state (old, new, pending, cwin);
}

static void
pragha_backend_message_async_done (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	PraghaBackendPrivate *priv = cwin->backend->priv;

	if (priv->seeking) {
		priv->seeking = FALSE;
		g_signal_emit (cwin->backend, signals[SIGNAL_SEEKED], 0);
	}
}

static void
pragha_backend_message_buffering (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	pragha_backend_parse_buffering (cwin->backend, msg);
}

static void
pragha_backend_message_clock_lost (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	PraghaBackendPrivate *priv = cwin->backend->priv;

	gst_element_set_state (priv->pipeline, GST_STATE_PAUSED);
	gst_element_set_state (priv->pipeline, GST_STATE_PLAYING);
}

static void
pragha_backend_message_tag (GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	pragha_backend_parse_message_tag (cwin->backend, msg);
}

static void
pragha_backend_finalize (GObject *object)
{
	PraghaBackend *backend = PRAGHA_BACKEND (object);
	PraghaBackendPrivate *priv = backend->priv;

	gst_element_set_state(priv->pipeline, GST_STATE_NULL);
	gst_object_unref(priv->pipeline);

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
	GError *error = NULL;

	if (priv->equalizer == NULL)
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

	g_type_class_add_private (klass, sizeof (PraghaBackendPrivate));
}

static void
pragha_backend_init (PraghaBackend *backend)
{
	PraghaBackendPrivate *priv = PRAGHA_BACKEND_GET_PRIVATE (backend);
	backend->priv = priv;
	priv->state = ST_STOPPED;
	priv->is_live = FALSE;
	priv->can_seek = FALSE;
	priv->seeking = FALSE;
	priv->emitted_error = FALSE;
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

	if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Alsa like audio sink");
		audiosink = "alsasink";
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS4_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss4 like audio sink");
		audiosink = "oss4sink";
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, OSS_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Oss like audio sink");
		audiosink = "osssink";
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, PULSE_SINK)) {
		CDEBUG(DBG_BACKEND, "Setting Pulseaudio like audio sink");
		audiosink = "pulsesink";
	}
	else {
		CDEBUG(DBG_BACKEND, "Setting autoaudiosink like audio sink");
		can_set_device = FALSE;
		audiosink = "autoaudiosink";
	}

	priv->audio_sink = gst_element_factory_make (audiosink, "audio-sink");

	if (priv->audio_sink != NULL) {
		/* Set the audio device to use. */
		if (can_set_device && cwin->cpref->audio_device != NULL && *cwin->cpref->audio_device != '\0')
			g_object_set(priv->audio_sink, "device", cwin->cpref->audio_device, NULL);

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
	g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)pragha_backend_message_error, cwin);
	g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback)pragha_backend_message_eos, cwin);
	g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)pragha_backend_message_state_changed, cwin);
	g_signal_connect(G_OBJECT(bus), "message::async-done", (GCallback)pragha_backend_message_async_done, cwin);
	g_signal_connect(G_OBJECT(bus), "message::buffering", (GCallback)pragha_backend_message_buffering, cwin);
	g_signal_connect(G_OBJECT(bus), "message::clock-lost", (GCallback)pragha_backend_message_clock_lost, cwin);
	g_signal_connect(G_OBJECT(bus), "message::tag", (GCallback)pragha_backend_message_tag, cwin);
	gst_object_unref (bus);

	pragha_backend_set_soft_volume(backend, cwin->cpref->software_mixer);
	pragha_backend_init_equalizer_preset(cwin);

	gst_element_set_state(priv->pipeline, GST_STATE_READY);

	CDEBUG(DBG_BACKEND, "Pipeline construction complete");

	return 0;
}
