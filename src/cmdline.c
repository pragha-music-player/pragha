/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

/* Handler for 'version' option on the cmdline */

gboolean cmd_version(const gchar *opt_name, const gchar *val,
		     struct con_win *cwin, GError **error)
{
	g_print("pragha %s\n", PACKAGE_VERSION);
	exit(0);
}

gboolean cmd_play(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PLAY, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_stop(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_STOP, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_pause(const gchar *opt_name, const gchar *val,
		   struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PAUSE, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_prev(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PREV, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_next(const gchar *opt_name, const gchar *val,
		  struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_NEXT, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_shuffle(const gchar *opt_name, const gchar *val,
		     struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_SHUFFLE, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_repeat(const gchar *opt_name, const gchar *val,
		    struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_REPEAT, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_inc_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_INC_VOL, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_dec_volume(const gchar *opt_name, const gchar *val,
			struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_DEC_VOL, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_show_osd(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_SHOW_OSD, cwin);
		exit(0);
	}

	return TRUE;
}

gboolean cmd_current_state(const gchar *opt_name, const gchar *val,
			   struct con_win *cwin, GError **error)
{
	gboolean ret = TRUE;
	DBusMessage *msg = NULL;
	DBusMessage *reply_msg = NULL;
	DBusError d_error;
	const char *state, *file, *title, *artist, *album, *genre;
	gint year, track_no, length, bitrate, channels, samplerate;

	year = track_no = length = bitrate = channels = samplerate = 0;

	if (cwin->cstate->unique_instance)
		return TRUE;

	dbus_error_init(&d_error);

	msg = dbus_message_new_method_call(DBUS_NAME,
					   DBUS_PATH,
					   DBUS_INTERFACE,
					   DBUS_METHOD_CURRENT_STATE);
	if (!msg) {
		g_critical("Unable to allocate memory for DBUS message");
		ret = FALSE;
		goto exit;
	}

	reply_msg = dbus_connection_send_with_reply_and_block(cwin->con_dbus, msg,
							      -1, &d_error);
	if (!reply_msg) {
		g_critical("Unable to send DBUS message");
		dbus_error_free(&d_error);
		ret = FALSE;
		goto bad;
	}

	if (!dbus_message_get_args(reply_msg, &d_error,
				   DBUS_TYPE_STRING, &state,
				   DBUS_TYPE_INVALID)) {
		g_critical("Unable to get player state");
		dbus_error_free(&d_error);
		ret = FALSE;
		goto bad;
	}

	if (g_ascii_strcasecmp(state, "Stopped")) {
		dbus_message_get_args(reply_msg, &d_error,
				      DBUS_TYPE_STRING, &state,
				      DBUS_TYPE_STRING, &file,
				      DBUS_TYPE_STRING, &title,
				      DBUS_TYPE_STRING, &artist,
				      DBUS_TYPE_STRING, &album,
				      DBUS_TYPE_STRING, &genre,
				      DBUS_TYPE_INT32, &year,
				      DBUS_TYPE_INT32, &track_no,
				      DBUS_TYPE_INT32, &length,
				      DBUS_TYPE_INT32, &bitrate,
				      DBUS_TYPE_INT32, &channels,
				      DBUS_TYPE_INT32, &samplerate,
				      DBUS_TYPE_INVALID);
		if (!dbus_message_get_args(reply_msg, &d_error,
					   DBUS_TYPE_STRING, &state,
					   DBUS_TYPE_INVALID)) {
			g_critical("Unable to get player state details");
			dbus_error_free(&d_error);
			ret = FALSE;
			goto bad;
		}

		g_print("state: %s\nfile: %s\ntitle: %s\nartist: %s\n"
			"album: %s\ngenre: %s\nyear: %d\ntrack_no: %d\nlength: %d\n"
			"bitrate: %d\nchannels: %d\nsamplerate: %d\n",
			state, file, title, artist, album, genre, year,
			track_no, length, bitrate, channels, samplerate);
	} else {
		g_print("state: %s\n", state);
	}
bad:
	dbus_message_unref(msg);
exit:
	if (!ret)
		exit(0);
	else
		return ret;
}

gboolean cmd_add_file(const gchar *opt_name, const gchar *val,
		      struct con_win *cwin, GError **error)
{
	gboolean ret = TRUE;
	DBusMessage *msg = NULL;

	msg = dbus_message_new_signal(DBUS_PATH, DBUS_INTERFACE, DBUS_SIG_ADD_FILE);
	if (!msg) {
		g_critical("Unable to allocate memory for DBUS message");
		ret = FALSE;
		goto exit;
	}
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &val, DBUS_TYPE_INVALID);

	if (!dbus_connection_send(cwin->con_dbus, msg, NULL)) {
		g_critical("Unable to send DBUS message");
		ret = FALSE;
		goto bad;
	}

	dbus_connection_flush(cwin->con_dbus);
bad:
	dbus_message_unref(msg);
exit:
	return ret;
}
