/*************************************************************************/
/* Copyright (C) 2010 matias <mati86dl@gmail.com>			 */
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

#define DURATION_IS_VALID(x) (x != 0 && x != (guint64) -1)

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

	g_idle_add(update_track_progress_bar, cwin);
	g_idle_add(update_current_song_info, cwin);

	return TRUE;
}

gboolean update_track_progress_bar(gpointer data)
{
	struct con_win *cwin = data;

	gint newsec = GST_TIME_AS_SECONDS(backend_get_current_position(cwin));

	if(newsec > 0);
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

void
waitforpipeline(int state, struct con_win *cwin)
{
	GstState istate, ipending;
	gst_element_get_state(cwin->cgst->pipeline, &istate, &ipending, GST_CLOCK_TIME_NONE);

	if ((istate == GST_STATE_VOID_PENDING) || (istate == state))
	        return;

	gst_element_set_state(cwin->cgst->pipeline, state);

	do {
		gst_element_get_state(cwin->cgst->pipeline, &istate, &ipending, GST_CLOCK_TIME_NONE);
		if (istate == GST_STATE_VOID_PENDING)
			return;

	} while (istate != state);
}

void
backend_stop(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Stopping playback");

	if(cwin->cgst->timer > 0) {
		g_source_remove(cwin->cgst->timer);
		cwin->cgst->timer = 0;
	}

	if (cwin->cstate->curr_mobj_clear) {
		delete_musicobject(cwin->cstate->curr_mobj);
		cwin->cstate->curr_mobj_clear = FALSE;
	}

	waitforpipeline(GST_STATE_NULL, cwin);

	unset_current_song_info(cwin);
	unset_track_progress_bar(cwin);
	unset_album_art(cwin);

	cwin->cstate->state = ST_STOPPED;
	play_button_toggle_state(cwin);
}

void
backend_pause(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Pause playback");

	if(cwin->cgst->timer > 0) {
		g_source_remove(cwin->cgst->timer);
		cwin->cgst->timer = 0;
	}

	waitforpipeline(GST_STATE_PAUSED, cwin);

	cwin->cstate->state = ST_PAUSED;
	play_button_toggle_state(cwin);
}

void
backend_resume(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Resuming playback");

	waitforpipeline(GST_STATE_PLAYING, cwin);

	if(cwin->cgst->timer == 0)
		cwin->cgst->timer = g_timeout_add_seconds (1, update_gui, cwin);

	cwin->cstate->state = ST_PLAYING;
	play_button_toggle_state(cwin);
}

void
backend_start(struct musicobject *mobj, struct con_win *cwin)
{
	if (!mobj) {
		g_critical("Dangling entry in current playlist");
		return;
	}

	if ((cwin->cstate->state == ST_PLAYING) ||
	    (cwin->cstate->state == ST_PAUSED)) {
		backend_stop(cwin);
	}

	cwin->cstate->curr_mobj = mobj;
	cwin->cstate->curr_mobj_clear = FALSE;

	backend_play(cwin);

	cwin->cstate->state = ST_PLAYING;
	play_button_toggle_state(cwin);

	CDEBUG(DBG_INFO, "Starting playback");
}

void
backend_play(struct con_win *cwin)
{
	gchar *uri = NULL;

	if (!cwin->cstate->curr_mobj->file)
		return;

	CDEBUG(DBG_INFO, "Playing : %s", cwin->cstate->curr_mobj->file);

	if(cwin->cstate->curr_mobj->file_type != FILE_CDDA) {
		uri = g_filename_to_uri (cwin->cstate->curr_mobj->file, NULL, NULL);
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "uri", uri, NULL);
		g_free (uri);
	}
	else {
		g_object_set(G_OBJECT(cwin->cgst->pipeline), "uri", cwin->cstate->curr_mobj->file, NULL);
	}

	waitforpipeline(GST_STATE_PLAYING, cwin);

	if(cwin->cgst->timer == 0)
		cwin->cgst->timer = g_timeout_add_seconds (1, update_gui, cwin);
}

static gboolean backend_gstreamer_bus_call(GstBus *bus, GstMessage *msg, struct con_win *cwin)
{
	switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			play_next_track(cwin);
			break;
		case GST_MESSAGE_ERROR: {
			GError *err = NULL;
			gchar *dbg_info = NULL;
			gst_message_parse_error (msg, &err, &dbg_info);
      			g_print("Gstreamer Error: %s\n", err->message);
			g_error_free (err);
			g_free (dbg_info);
			play_next_track(cwin);
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
		waitforpipeline(GST_STATE_NULL, cwin);
		gst_object_unref(GST_OBJECT(cwin->cgst->pipeline));
		cwin->cgst->pipeline = NULL;
	}
}

gint backend_init(struct con_win *cwin)
{
	GstBus *bus;

	gst_init(NULL, NULL);

	if ((cwin->cgst->pipeline = gst_element_factory_make("playbin2", "playbin")) == NULL)
		return -1;

	g_signal_connect (G_OBJECT (cwin->cgst->pipeline), "deep-notify::volume",
			  G_CALLBACK (volume_notify_cb), cwin);

	/* If no audio sink has been specified via the "audio-sink" property, playbin will use the autoaudiosink.
	   Need review then when return the audio preferences. */

	if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)){
		cwin->cgst->audio_sink = gst_element_factory_make ("alsasink", "audio-sink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)) {
		cwin->cgst->audio_sink = gst_element_factory_make ("oss4sink", "audio-sink");
	}
	else if (!g_ascii_strcasecmp(cwin->cpref->audio_sink, ALSA_SINK)) {
		cwin->cgst->audio_sink = gst_element_factory_make ("pulsesink", "audio-sink");
	}

	if(cwin->cgst->audio_sink == NULL) {
		cwin->cgst->audio_sink = gst_element_factory_make ("autoaudiosink", "audio-sink");
	}
	
	g_object_set(G_OBJECT(cwin->cgst->pipeline), "audio-sink", cwin->cgst->audio_sink, NULL);

	bus = gst_pipeline_get_bus(GST_PIPELINE(cwin->cgst->pipeline));

	gst_bus_add_watch(bus, (GstBusFunc) backend_gstreamer_bus_call, cwin);

	backend_set_soft_volume(cwin);
	emit_volume_changed_idle(cwin);

	gst_element_set_state(cwin->cgst->pipeline, GST_STATE_READY);

	gst_object_unref(bus);

	CDEBUG(DBG_INFO, "Pipeline construction complete");

	return 0;
}
