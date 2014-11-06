/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#ifdef HAVE_LIBXFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#endif

#include <stdlib.h>

#include "pragha-playback.h"
#include "pragha-window.h"
#include "pragha.h"

static struct PraghaCmdLineOptions {
	gchar *audio_backend;
	gchar *audio_device;
	gchar *audio_mixer;
	gchar *logfile;
	gboolean play;
	gboolean stop;
	gboolean pause;
	gboolean prev;
	gboolean next;
	gboolean shuffle;
	gboolean repeat;
	gboolean inc_volume;
	gboolean dec_volume;
	gboolean toggle_view;
	gboolean current_state;
	gchar **files;
} cmdline_options;

static void
clear_cmdline_options ()
{
	g_free (cmdline_options.audio_backend);
	g_free (cmdline_options.audio_device);
	g_free (cmdline_options.audio_mixer);
	g_free (cmdline_options.logfile);
	g_strfreev (cmdline_options.files);
	memset (&cmdline_options, 0, sizeof(cmdline_options));
}

static gboolean
cmd_version (const gchar *opt_name, const gchar *val, gpointer arg_data, GError **error)
{
	g_print ("%s %s\n", PACKAGE, VERSION);

	exit (0);
}

static void
cmd_current_state (PraghaApplication *pragha, GApplicationCommandLine *command_line)
{
	const char *playing_str = "Playing";
	const char *paused_str = "Paused";
	const char *stopped_str = "Stopped";

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		const char *state = (pragha_backend_get_state (backend) == ST_PLAYING) ? playing_str : paused_str;

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

		g_application_command_line_print (command_line, "state: %s\nfile: %s\ntitle: %s\nartist: %s\n"
			"album: %s\ngenre: %s\nyear: %d\ntrack_no: %d\ncomment: %s\n"
			"length: %d\nbitrate: %d\nchannels: %d\nsamplerate: %d\n",
			state, file, title, artist, album, genre, year,
			track_no, comment, length, bitrate, channels, samplerate);
	} else {
		g_application_command_line_print (command_line, "state: %s\n", stopped_str);
	}
}

static void
cmd_add_files (PraghaApplication *pragha, GApplicationCommandLine *command_line)
{
	gchar **file_names = cmdline_options.files;
	GPtrArray *files = g_ptr_array_new_with_free_func (g_object_unref);

	while (*file_names) {
#if GLIB_CHECK_VERSION (2, 36, 0)
		GFile *file = g_application_command_line_create_file_for_arg (command_line, *file_names);
#else
		const gchar *cwd = g_application_command_line_get_cwd (command_line);
		gchar *filename = g_build_filename (cwd, *file_names, NULL);
		GFile *file = g_file_new_for_path (filename);
		g_free (filename);
#endif
		g_ptr_array_add (files, file);
		file_names++;
	}

	if (files->len > 0)
		g_application_open (G_APPLICATION (pragha), (GFile **) files->pdata, files->len, "");

	g_ptr_array_unref (files);
}

static void
process_options (PraghaApplication *pragha, GApplicationCommandLine *command_line)
{
	if (!command_line)
		return;

	if (cmdline_options.logfile) {
		g_log_set_default_handler (pragha_log_to_file, cmdline_options.logfile);
	}
	if (cmdline_options.play) {
		pragha_playback_play_pause_resume (pragha);
	}
	if (cmdline_options.stop) {
		pragha_playback_stop (pragha);
	}
	if (cmdline_options.pause) {
		pragha_playback_play_pause_resume (pragha);
	}
	if (cmdline_options.prev) {
		pragha_playback_prev_track (pragha);
	}
	if (cmdline_options.next) {
		pragha_playback_next_track (pragha);
	}
	if (cmdline_options.shuffle) {
		PraghaPreferences *preferences = pragha_application_get_preferences (pragha);
		gboolean shuffle = pragha_preferences_get_shuffle (preferences);
		pragha_preferences_set_shuffle (preferences, !shuffle);
	}
	if (cmdline_options.repeat) {
		PraghaPreferences *preferences = pragha_application_get_preferences (pragha);
		gboolean repeat = pragha_preferences_get_repeat (preferences);
		pragha_preferences_set_repeat (preferences, !repeat);
	}
	if (cmdline_options.inc_volume) {
		PraghaBackend *backend = pragha_application_get_backend (pragha);
		pragha_backend_set_delta_volume (backend, +0.05);
	}
	if (cmdline_options.dec_volume) {
		PraghaBackend *backend = pragha_application_get_backend (pragha);
		pragha_backend_set_delta_volume (backend, -0.05);
	}
	if (cmdline_options.toggle_view) {
		pragha_window_toggle_state (pragha, TRUE);
	}
	if (cmdline_options.current_state) {
		cmd_current_state (pragha, command_line);
	}
	if (cmdline_options.files) {
		cmd_add_files (pragha, command_line);
	}
}

static const GOptionEntry cmd_entries[] = {
	{"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
	 cmd_version, "Version", NULL},
	{"debug", 'e', 0, G_OPTION_ARG_INT,
	 &debug_level, "Enable Debug ( Levels: 1,2,3,4 )", NULL},
	{ "log-file", 'l', 0, G_OPTION_ARG_FILENAME,
	 &cmdline_options.logfile, "Redirects console warnings to the specified FILENAME", N_("FILENAME")},
	{"play", 'p', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.play, "Play", NULL},
	{"stop", 's', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.stop, "Stop", NULL},
	{"pause", 't', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.pause, "Play/Pause/Resume", NULL},
	{"prev", 'r', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.prev, "Prev", NULL},
	{"next", 'n', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.next, "Next", NULL},
	{"shuffle", 'f', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.shuffle, "Shuffle", NULL},
	{"repeat", 'u', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.repeat, "Repeat", NULL},
	{"inc_vol", 'i', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.inc_volume, "Increase volume by 1", NULL},
	{"dec_vol", 'd', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.dec_volume, "Decrease volume by 1", NULL},
	{"toggle_view", 'x', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.toggle_view, "Toggle player visibility", NULL},
	{"current_state", 'c', 0, G_OPTION_ARG_NONE,
	 &cmdline_options.current_state, "Get current player state", NULL},
	{"audio_backend", 'a', 0, G_OPTION_ARG_STRING,
	 &cmdline_options.audio_backend, "Audio backend (valid options: alsa/oss)", NULL},
	{"audio_device", 'g', 0, G_OPTION_ARG_STRING,
	 &cmdline_options.audio_device, "Audio Device (For ALSA: hw:0,0 etc.., For OSS: /dev/dsp etc..)", NULL},
	{"audio_mixer", 'm', 0, G_OPTION_ARG_STRING,
	 &cmdline_options.audio_mixer, "Mixer Element (For ALSA: Master, PCM, etc.., For OSS: /dev/mixer, etc...)", NULL},
	{G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
	 &cmdline_options.files, "", "[FILE1 [FILE2...]]"},
	{NULL}
};

gint
handle_command_line (PraghaApplication *pragha, GApplicationCommandLine *command_line, gint argc, gchar **args)
{
	gint i;
	int ret = 0;
	GError *error = NULL;

	/* We have to make an extra copy of the array, since g_option_context_parse()
	 * assumes that it can remove strings from the array without freeing them.
	 */
	gchar **argv = g_new (gchar*, argc + 1);
	for (i = 0; i <= argc; i++)
		argv[i] = args[i];

	GOptionContext *context = g_option_context_new ("- A lightweight music player");
	GOptionGroup *group = g_option_group_new ("General", "General", "General Options", NULL, NULL);
	g_option_group_add_entries (group, cmd_entries);
	g_option_context_set_main_group (context, group);
	#ifdef HAVE_LIBXFCE4UI
	g_option_context_add_group (context, xfce_sm_client_get_option_group (argc, argv));
	#endif

	g_option_context_parse (context, &argc, &argv, &error);

	if (error) {
		g_print ("%s\n%s\n", error->message, _("Use --help to see a full list of available command line options."));
		g_error_free (error);
		ret = -1;
	}

	process_options (pragha, command_line);
	clear_cmdline_options ();

	g_option_context_free (context);
	g_free (argv);

	return ret;
}
