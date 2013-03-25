/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2011 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef HAVE_LIBXFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#endif

#include <stdlib.h>
#include "pragha-dbus.h"
#include "pragha-debug.h"
#include "pragha.h"

static gchar *audio_backend = NULL;
static gchar *audio_device = NULL;
static gchar *audio_mixer = NULL;
static gchar *logfile = NULL;

static gboolean
cmd_version (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	g_print("pragha %s\n", PACKAGE_VERSION);

	exit(0);
}

static gboolean
cmd_play (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PLAY, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_stop (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_STOP, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_pause (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PAUSE, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_prev (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_PREV, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_next (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_NEXT, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_shuffle (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_SHUFFLE, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_repeat (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_REPEAT, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_inc_volume (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_INC_VOL, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_dec_volume (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_DEC_VOL, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_show_osd (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_SHOW_OSD, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_toggle_view (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	if (!cwin->cstate->unique_instance) {
		dbus_send_signal(DBUS_SIG_TOGGLE_VIEW, cwin);
		exit(0);
	}
	return FALSE;
}

static gboolean
cmd_current_state (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	gboolean ret = TRUE;
	DBusMessage *msg = NULL;
	DBusMessage *reply_msg = NULL;
	DBusError d_error;
	const char *state, *file, *title, *artist, *album, *genre, *comment;
	gint year, track_no, length, bitrate, channels, samplerate;

	year = track_no = length = bitrate = channels = samplerate = 0;

	if (cwin->cstate->unique_instance) {
		ret = FALSE;
		goto exit;
	}

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
				      DBUS_TYPE_STRING, &comment,
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
			"album: %s\ngenre: %s\nyear: %d\ntrack_no: %d\ncomment: %s\n"
			"length: %d\nbitrate: %d\nchannels: %d\nsamplerate: %d\n",
			state, file, title, artist, album, genre, year,
			track_no, comment, length, bitrate, channels, samplerate);
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

static gboolean
cmd_add_file (const gchar *opt_name, const gchar *val, struct con_win *cwin, GError **error)
{
	gboolean ret = TRUE;
	DBusMessage *msg = NULL;
	gchar *uri = NULL;

	if(g_path_is_absolute(val)) {
		uri = g_strdup (val);
	}
	else {
		gchar *dir = g_get_current_dir ();
		uri = g_build_filename (dir, val, NULL);
		g_free (dir);
	}

	msg = dbus_message_new_signal(DBUS_PATH, DBUS_INTERFACE, DBUS_SIG_ADD_FILE);
	if (!msg) {
		g_critical("Unable to allocate memory for DBUS message");
		ret = FALSE;
		goto exit;
	}
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

	if (!dbus_connection_send(cwin->con_dbus, msg, NULL)) {
		g_critical("Unable to send DBUS message");
		ret = FALSE;
		goto bad;
	}

	dbus_connection_flush(cwin->con_dbus);
bad:
	dbus_message_unref(msg);
exit:
	g_free(uri);

	return ret;
}

static const GOptionEntry cmd_entries[] = {
	{"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_version, "Version", NULL},
	{"debug", 'e', 0, G_OPTION_ARG_INT,
	 &debug_level, "Enable Debug ( Levels: 1,2,3,4 )", NULL},
	{ "log-file", 'l', 0, G_OPTION_ARG_FILENAME,
	&logfile, "Redirects console warnings to the specified FILENAME", N_("FILENAME")},
	{"play", 'p', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_play, "Play", NULL},
	{"stop", 's', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_stop, "Stop", NULL},
	{"pause", 't', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_pause, "Play/Pause/Resume", NULL},
	{"prev", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_prev, "Prev", NULL},
	{"next", 'n', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_next, "Next", NULL},
	{"shuffle", 'f', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_shuffle, "Shuffle", NULL},
	{"repeat", 'u', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_repeat, "Repeat", NULL},
	{"inc_vol", 'i', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_inc_volume, "Increase volume by 1", NULL},
	{"dec_vol", 'd', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_dec_volume, "Decrease volume by 1", NULL},
	{"show_osd", 'o', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_show_osd, "Show OSD notification", NULL},
	{"toggle_view", 'x', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_toggle_view, "Toggle player visibility", NULL},
	{"current_state", 'c', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_current_state, "Get current player state", NULL},
	{"audio_backend", 'a', 0, G_OPTION_ARG_STRING,
	 &audio_backend, "Audio backend (valid options: alsa/oss)", NULL},
	{"audio_device", 'g', 0, G_OPTION_ARG_STRING,
	 &audio_device, "Audio Device (For ALSA: hw:0,0 etc.., For OSS: /dev/dsp etc..)", NULL},
	{"audio_mixer", 'm', 0, G_OPTION_ARG_STRING,
	 &audio_mixer, "Mixer Element (For ALSA: Master, PCM, etc.., For OSS: /dev/mixer, etc...)", NULL},
	{G_OPTION_REMAINING, 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_CALLBACK,
	 cmd_add_file, "", "[FILE1 [FILE2...]]"},
	{NULL}
};

gint
init_options (struct con_win *cwin, int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;
	GOptionGroup *group;

	CDEBUG(DBG_INFO, "Initializing Command line options");

	context = g_option_context_new("- A lightweight music player");
	group = g_option_group_new("General", "General", "General Options", cwin, NULL);
	g_option_group_add_entries(group, cmd_entries);
	g_option_context_set_main_group(context, group);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	#ifdef HAVE_LIBXFCE4UI
	g_option_context_add_group (context, xfce_sm_client_get_option_group (argc, argv));
	#endif

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		gchar *txt;

		g_message("Unable to parse options. Some options need another instance of pragha running.");
		txt = g_option_context_get_help(context, TRUE, NULL);
		g_print("%s", txt);
		g_free(txt);
		g_option_context_free(context);
		g_error_free(error);
		return -1;
	}

	if (logfile)
		g_log_set_default_handler (pragha_log_to_file, (gpointer)logfile);

	g_option_context_free(context);
	return 0;
}
