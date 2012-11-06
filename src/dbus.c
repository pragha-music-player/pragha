/************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>		        */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
/* 								        */
/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.				        */
/* 								        */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.			        */
/* 								        */
/* You should have received a copy of the GNU General Public License    */
/* along with this program.  If not, see <http:www.gnu.org/licenses/>.  */
/************************************************************************/

#include "pragha.h"

static void dbus_play_handler(struct con_win *cwin)
{
	pragha_playback_play_pause_resume(cwin);
}

static void dbus_stop_handler(struct con_win *cwin)
{
	pragha_playback_stop(cwin);
}

static void dbus_pause_handler(struct con_win *cwin)
{
	pragha_playback_play_pause_resume(cwin);
}

static void dbus_next_handler(struct con_win *cwin)
{
	pragha_playback_next_track(cwin);
}

static void dbus_prev_handler(struct con_win *cwin)
{
	pragha_playback_prev_track(cwin);
}

static void dbus_shuffle_handler(struct con_win *cwin)
{
	if (cwin->cpref->shuffle)
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(
				     cwin->shuffle_button), FALSE);
	else
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(
				     cwin->shuffle_button), TRUE);
}

static void dbus_repeat_handler(struct con_win *cwin)
{
	if (cwin->cpref->repeat)
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(
				     cwin->repeat_button), FALSE);
	else
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(
				     cwin->repeat_button), TRUE);
}

static void dbus_inc_vol_handler(struct con_win *cwin)
{
	pragha_backend_set_delta_volume (cwin->backend, +0.05);
}

static void dbus_dec_vol_handler(struct con_win *cwin)
{
	pragha_backend_set_delta_volume (cwin->backend, -0.05);
}

static void dbus_show_osd_handler(struct con_win *cwin)
{
	show_osd(cwin);
}

static void dbus_toggle_handler(struct con_win *cwin)
{
	toogle_main_window(cwin, TRUE);
}

static void dbus_add_file(DBusMessage *msg, struct con_win *cwin)
{
	gchar *file;
	DBusError error;
 	struct musicobject *mobj = NULL; 
	gint prev_tracks = 0;

	dbus_error_init(&error);
	dbus_message_get_args(msg, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&error)) {
		g_critical("Unable to get file name out of add file signal");
		dbus_error_free(&error);
		return;
	}

	prev_tracks = cwin->cplaylist->no_tracks;

	if (is_dir_and_accessible(file)) {
		if(cwin->cpref->add_recursively_files)
			__recur_add(file, cwin);
		else
			__non_recur_add(file, TRUE, cwin);
	}
	else if (is_playable_file(file)) {
		mobj = new_musicobject_from_file(file);
		if (mobj)
			append_current_playlist(NULL, mobj, cwin);
		CDEBUG(DBG_INFO, "Add file from command line: %s", file);
	}
	else {
		g_warning("Unable to add %s", file);
	}

	select_numered_path_of_current_playlist(prev_tracks, cwin);
	update_status_bar(cwin);
}

static void dbus_current_state(DBusMessage *msg, struct con_win *cwin)
{
	DBusMessage *reply_msg;
	const char *playing_str = "Playing";
	const char *paused_str = "Paused";
	const char *stopped_str = "Stopped";

	reply_msg = dbus_message_new_method_return(msg);
	if (!reply_msg) {
		g_critical("Unable to allocate memory for DBUS message");
		return;
	}

	if (pragha_backend_get_state (cwin->backend) != ST_STOPPED) {
		dbus_message_append_args(reply_msg,
			 DBUS_TYPE_STRING, (pragha_backend_get_state (cwin->backend) == ST_PLAYING) ? &playing_str : &paused_str,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->file,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->tags->title,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->tags->artist,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->tags->album,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->tags->genre,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->year,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->track_no,
			 DBUS_TYPE_STRING, &cwin->cstate->curr_mobj->tags->comment,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->length,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->bitrate,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->channels,
			 DBUS_TYPE_INT32, &cwin->cstate->curr_mobj->tags->samplerate,
			 DBUS_TYPE_INVALID);
	} else {
		dbus_message_append_args(reply_msg,
					 DBUS_TYPE_STRING, &stopped_str,
					 DBUS_TYPE_INVALID);
	}

	if (!dbus_connection_send(cwin->con_dbus, reply_msg, NULL)) {
		g_critical("Unable to send DBUS message");
		goto bad;
	}

	dbus_connection_flush(cwin->con_dbus);
bad:
	dbus_message_unref(reply_msg);
}

/* Parse messages from DBUS and choose the appropriate handler function */

DBusHandlerResult dbus_filter_handler(DBusConnection *conn,
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

void dbus_send_signal(const gchar *signal, struct con_win *cwin)
{
	DBusMessage *msg = NULL;

	msg = dbus_message_new_signal(DBUS_PATH, DBUS_INTERFACE, signal);

	if (!msg) {
		g_critical("Unable to allocate memory for DBUS message");
		return;
	}

	if (!dbus_connection_send(cwin->con_dbus, msg, NULL)) {
		g_critical("Unable to send DBUS message");
		goto exit;
	}

	if(!g_strcmp0(signal, DBUS_EVENT_UPDATE_STATE))
		mpris_update_any(cwin);

	dbus_connection_flush(cwin->con_dbus);
exit:
	dbus_message_unref(msg);
}

gint init_dbus(struct con_win *cwin)
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
		return -1;
	}

	ret = dbus_bus_request_name(conn, DBUS_NAME, 0, &error);
	if (ret == -1) {
		g_critical("Unable to request for DBUS service name");
		dbus_error_free(&error);
		return -1;
	}

	if (ret & DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		cwin->cstate->unique_instance = TRUE;
	else if (ret & DBUS_REQUEST_NAME_REPLY_EXISTS)
		cwin->cstate->unique_instance = FALSE;

	dbus_connection_setup_with_g_main(conn, NULL);
	cwin->con_dbus = conn;

	return 0;
}

gint init_dbus_handlers(struct con_win *cwin)
{
	DBusError error;

	dbus_error_init(&error);
	if (!cwin->con_dbus) {
		g_critical("No DBUS connection");
		return -1;
	}

	dbus_bus_add_match(cwin->con_dbus,
			   "type='signal',path='/org/pragha/DBus'",
			   &error);
	if (dbus_error_is_set(&error)) {
		g_critical("Unable to register match rule for DBUS");
		dbus_error_free(&error);
		return -1;
	}

	if (!dbus_connection_add_filter(cwin->con_dbus, dbus_filter_handler, cwin, NULL)) {
		g_critical("Unable to allocate memory for DBUS filter");
		return -1;
	}

	return 0;
}

void dbus_handlers_free (struct con_win *cwin)
{
	dbus_connection_remove_filter(cwin->con_dbus,
				      dbus_filter_handler,
				      cwin);
	dbus_bus_remove_match(cwin->con_dbus,
			      "type='signal',path='/org/pragha/DBus'",
			      NULL);
	dbus_connection_unref(cwin->con_dbus);
}
