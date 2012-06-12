/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#include "pragha-dbus.h"
#include "pragha-playback.h"
#include "pragha-file-utils.h"
#include "pragha-notify.h"
#include "pragha-window.h"
#include "pragha-debug.h"
#include "pragha.h"

static void dbus_play_handler(PraghaApplication *pragha)
{
	pragha_playback_play_pause_resume(pragha);
}

static void dbus_stop_handler(PraghaApplication *pragha)
{
	pragha_playback_stop(pragha);
}

static void dbus_pause_handler(PraghaApplication *pragha)
{
	pragha_playback_play_pause_resume(pragha);
}

static void dbus_next_handler(PraghaApplication *pragha)
{
	pragha_playback_next_track(pragha);
}

static void dbus_prev_handler(PraghaApplication *pragha)
{
	pragha_playback_prev_track(pragha);
}

static void dbus_shuffle_handler(PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	preferences = pragha_application_get_preferences (pragha);

	gboolean shuffle = pragha_preferences_get_shuffle (preferences);
	pragha_preferences_set_shuffle (preferences, !shuffle);
}

static void dbus_repeat_handler(PraghaApplication *pragha)
{
	PraghaPreferences *preferences;
	preferences = pragha_application_get_preferences (pragha);

	gboolean repeat = pragha_preferences_get_repeat (preferences);
	pragha_preferences_set_repeat (preferences, !repeat);
}

static void dbus_inc_vol_handler(PraghaApplication *pragha)
{
	PraghaBackend *backend;
	backend = pragha_application_get_backend (pragha);

	pragha_backend_set_delta_volume (backend, +0.05);
}

static void dbus_dec_vol_handler(PraghaApplication *pragha)
{
	PraghaBackend *backend;
	backend = pragha_application_get_backend (pragha);

	pragha_backend_set_delta_volume (backend, -0.05);
}

static void dbus_show_osd_handler(PraghaApplication *pragha)
{
	PraghaNotify *notify;
	notify = pragha_application_get_notify (pragha);

	if (notify)
		pragha_notify_show_osd (notify);
	else
		g_warning ("Notifications are disabled");
}

static void dbus_toggle_handler(PraghaApplication *pragha)
{
	pragha_window_toggle_state(pragha, TRUE);
}

static void dbus_add_file(DBusMessage *msg, PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	gchar *file;
	DBusError error;
	GList *mlist = NULL;

	dbus_error_init(&error);
	dbus_message_get_args(msg, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&error)) {
		g_critical("Unable to get file name out of add file signal");
		dbus_error_free(&error);
		return;
	}

	mlist = append_mobj_list_from_unknown_filename(mlist, file);
	if (mlist) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list (playlist, mlist);
		g_list_free (mlist);
	}
}

static void dbus_current_state(DBusMessage *msg, PraghaApplication *pragha)
{
	DBusMessage *reply_msg;
	const char *playing_str = "Playing";
	const char *paused_str = "Paused";
	const char *stopped_str = "Stopped";
	PraghaBackend *backend;

	reply_msg = dbus_message_new_method_return(msg);
	if (!reply_msg) {
		g_critical("Unable to allocate memory for DBUS message");
		return;
	}

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		PraghaMusicobject *mobj = pragha_backend_get_musicobject (backend);

		const char *file = pragha_musicobject_get_file (mobj);
		const char *title = pragha_musicobject_get_title (mobj);
		const char *artist = pragha_musicobject_get_artist (mobj);
		const char *album = pragha_musicobject_get_album (mobj);
		const char *genre = pragha_musicobject_get_genre (mobj);
		guint year = pragha_musicobject_get_year (mobj);
		guint track_no = pragha_musicobject_get_track_no (mobj);
		const char *comment = pragha_musicobject_get_comment (mobj);
		gint length = pragha_musicobject_get_length (mobj);
		gint bitrate = pragha_musicobject_get_bitrate (mobj);
		gint channels = pragha_musicobject_get_channels (mobj);
		gint samplerate = pragha_musicobject_get_samplerate (mobj);

		dbus_message_append_args(reply_msg,
			 DBUS_TYPE_STRING, (pragha_backend_get_state (backend) == ST_PLAYING) ? &playing_str : &paused_str,
			 DBUS_TYPE_STRING, &file,
			 DBUS_TYPE_STRING, &title,
			 DBUS_TYPE_STRING, &artist,
			 DBUS_TYPE_STRING, &album,
			 DBUS_TYPE_STRING, &genre,
			 DBUS_TYPE_INT32, &year,
			 DBUS_TYPE_INT32, &track_no,
			 DBUS_TYPE_STRING, &comment,
			 DBUS_TYPE_INT32, &length,
			 DBUS_TYPE_INT32, &bitrate,
			 DBUS_TYPE_INT32, &channels,
			 DBUS_TYPE_INT32, &samplerate,
			 DBUS_TYPE_INVALID);
	} else {
		dbus_message_append_args(reply_msg,
					 DBUS_TYPE_STRING, &stopped_str,
					 DBUS_TYPE_INVALID);
	}

	if (!dbus_connection_send(pragha->con_dbus, reply_msg, NULL)) {
		g_critical("Unable to send DBUS message");
		goto bad;
	}

	dbus_connection_flush(pragha->con_dbus);
bad:
	dbus_message_unref(reply_msg);
}

/* Parse messages from DBUS and choose the appropriate handler function */

static DBusHandlerResult
dbus_filter_handler(DBusConnection *conn,
				      DBusMessage *msg,
				      gpointer data)
{
	if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_PLAY)) {
		dbus_play_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_STOP)) {
		dbus_stop_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_PAUSE)) {
		dbus_pause_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_NEXT)) {
		dbus_next_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_PREV)) {
		dbus_prev_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_SHUFFLE)) {
		dbus_shuffle_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_REPEAT)) {
		dbus_repeat_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_INC_VOL)) {
		dbus_inc_vol_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_DEC_VOL)) {
		dbus_dec_vol_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_SHOW_OSD)) {
		dbus_show_osd_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_TOGGLE_VIEW)) {
		dbus_toggle_handler(data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE, DBUS_SIG_ADD_FILE)) {
		dbus_add_file(msg, data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(msg, DBUS_INTERFACE, DBUS_METHOD_CURRENT_STATE)) {
		dbus_current_state(msg, data);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(msg, DBUS_INTERFACE, DBUS_EVENT_UPDATE_STATE)) {
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* Send a signal to a running instance */

void dbus_send_signal(const gchar *signal, PraghaApplication *pragha)
{
	PraghaMpris2 *mpris2;
	DBusMessage *msg = NULL;

	msg = dbus_message_new_signal(DBUS_PATH, DBUS_INTERFACE, signal);

	if (!msg) {
		g_critical("Unable to allocate memory for DBUS message");
		return;
	}

	if (!dbus_connection_send(pragha->con_dbus, msg, NULL)) {
		g_critical("Unable to send DBUS message");
		goto exit;
	}

	if(!g_strcmp0(signal, DBUS_EVENT_UPDATE_STATE)) {
		mpris2 = pragha_application_get_mpris2 (pragha);
		pragha_mpris_update_any (mpris2);
	}

	dbus_connection_flush(pragha->con_dbus);
exit:
	dbus_message_unref(msg);
}

DBusConnection *
pragha_init_dbus(PraghaApplication *pragha)
{
	DBusConnection *conn = NULL;
	DBusError error;
	gint ret = 0;

	CDEBUG(DBG_INFO, "Initializing DBUS");

	dbus_error_init(&error);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (!conn) {
		g_critical("Unable to get a DBUS connection");
		dbus_error_free(&error);
		return NULL;
	}

	ret = dbus_bus_request_name(conn, DBUS_NAME, 0, &error);
	if (ret == -1) {
		g_critical("Unable to request for DBUS service name");
		dbus_error_free(&error);
		return NULL;
	}

	dbus_connection_setup_with_g_main(conn, NULL);

	return conn;
}

gint
pragha_init_dbus_handlers(PraghaApplication *pragha)
{
	DBusError error;

	dbus_error_init(&error);
	if (!pragha->con_dbus) {
		g_critical("No DBUS connection");
		return -1;
	}

	dbus_bus_add_match(pragha->con_dbus,
			   "type='signal',path='/org/pragha/DBus'",
			   &error);
	if (dbus_error_is_set(&error)) {
		g_critical("Unable to register match rule for DBUS");
		dbus_error_free(&error);
		return -1;
	}

	if (!dbus_connection_add_filter(pragha->con_dbus, dbus_filter_handler, pragha, NULL)) {
		g_critical("Unable to allocate memory for DBUS filter");
		return -1;
	}

	return 0;
}

void dbus_handlers_free (PraghaApplication *pragha)
{
	dbus_connection_remove_filter(pragha->con_dbus,
				      dbus_filter_handler,
				      pragha);
	dbus_bus_remove_match(pragha->con_dbus,
			      "type='signal',path='/org/pragha/DBus'",
			      NULL);
	dbus_connection_unref(pragha->con_dbus);
}
