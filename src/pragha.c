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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <locale.h> /* require LC_ALL */
#include <libintl.h>

#include "pragha-window.h"
#include "pragha-playback.h"
#include "pragha-library-pane.h"
#include "pragha-menubar.h"
#include "pragha-lastfm.h"
#include "pragha-keybinder.h"
#include "pragha-dbus.h"
#include "pragha-window.h"
#include "pragha-notify.h"
#include "pragha-preferences-dialog.h"
#include "pragha-glyr.h"
#include "pragha-utils.h"
#include "pragha-dbus.h"
#include "pragha-debug.h"
#include "pragha.h"

gint debug_level;
#ifdef DEBUG
GThread *pragha_main_thread = NULL;
#endif

/*
 * Some calbacks..
 */
static void
pragha_library_pane_append_tracks (PraghaLibraryPane *library, struct con_win *cwin)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_append_mobj_list (cwin->cplaylist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks (PraghaLibraryPane *library, struct con_win *cwin)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (cwin->cplaylist);

		pragha_playlist_append_mobj_list (cwin->cplaylist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks_and_play (PraghaLibraryPane *library, struct con_win *cwin)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (cwin->cplaylist);

		pragha_playlist_append_mobj_list (cwin->cplaylist,
			                              list);

		if (pragha_backend_get_state (cwin->backend) != ST_STOPPED)
			pragha_playback_next_track(cwin);
		else
			pragha_playback_play_pause_resume(cwin);

		g_list_free(list);
	}
}

/*
 * Some public actions.
 */

PraghaBackend *
pragha_application_get_backend (struct con_win *cwin)
{
	return cwin->backend;
}

GdkPixbuf *
pragha_application_get_pixbuf_app (struct con_win *cwin)
{
	return cwin->pixbuf_app;
}

void
pragha_application_quit (struct con_win *cwin)
{
	gtk_main_quit();

	CDEBUG(DBG_INFO, "Halt.");
}

/* FIXME: Cleanup track refs */
static void
pragha_application_free (struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Cleaning up");

	pragha_playback_stop(cwin);

#ifdef HAVE_LIBGLYR
	pragha_glyr_free (cwin->glyr);
#endif
#ifdef HAVE_LIBCLASTFM
	pragha_lastfm_free (cwin->clastfm);
#endif
	pragha_playlist_free(cwin->cplaylist);
	pragha_sidebar_free(cwin->sidebar);
	mpris_free (cwin->cmpris2);
	g_object_unref (cwin->backend);
	pragha_art_cache_free (cwin->art_cache);
	pragha_window_free (cwin);
	pragha_scanner_free(cwin->scanner);
	g_object_unref(G_OBJECT(cwin->preferences));
	g_object_unref(cwin->cdbase);
	dbus_handlers_free (cwin);
	if (cwin->notify)
		pragha_notify_free (cwin->notify);

	if (cwin->cgnome_media_keys)
		gnome_media_keys_free (cwin->cgnome_media_keys);
#ifdef HAVE_LIBKEYBINDER
	else if (cwin->keybinder)
		keybinder_free ();
#endif
	pragha_cdda_free ();

	g_object_unref (cwin->systray_menu);

	g_slice_free(struct con_win, cwin);
}

static struct con_win *
pragha_application_new (gint argc, gchar *argv[])
{
	PraghaToolbar *toolbar;
	struct con_win *cwin;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	/* Alocate the main structure. */
	cwin = g_slice_new0(struct con_win);

	/* Allocate memory for simple structures */

	cwin->cmpris2 = pragha_mpris_new();

	cwin->con_dbus = pragha_init_dbus(cwin);
	if (!cwin->con_dbus) {
		g_critical("Unable to init dbus connection");
		return NULL;
	}
	if (pragha_init_dbus_handlers(cwin) == -1) {
		g_critical("Unable to initialize DBUS filter handlers");
		return NULL;
	}

	/* Parse command line arguments */
	if (init_options(cwin, argc, argv) == -1)
		return NULL;

	/* Allow only one instance */
	if (!cwin->unique_instance)
		return cwin;

	cwin->preferences = pragha_preferences_get();

	if (string_is_empty(pragha_preferences_get_installed_version(cwin->preferences)))
		cwin->first_run = TRUE;

	cwin->cdbase = pragha_database_get();
	if (pragha_database_start_successfully(cwin->cdbase) == FALSE) {
		g_critical("Unable to init music dbase");
		return NULL;
	}

	if (pragha_preferences_get_show_osd (cwin->preferences))
		cwin->notify = pragha_notify_new (cwin);
	else
		cwin->notify = NULL;

	cwin->art_cache = pragha_art_cache_new ();

	cwin->backend = pragha_backend_new (cwin);

	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK(pragha_backend_notificate_new_state), cwin);
	g_signal_connect (cwin->backend, "finished",
	                  G_CALLBACK(pragha_backend_finished_song), cwin);
	g_signal_connect (cwin->backend, "tags-changed",
	                  G_CALLBACK(pragha_backend_tags_changed), cwin);

	g_signal_connect (cwin->backend, "error",
	                 G_CALLBACK(gui_backend_error_show_dialog_cb), cwin);
	g_signal_connect (cwin->backend, "error",
	                  G_CALLBACK(gui_backend_error_update_current_playlist_cb), cwin);
	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), cwin);

	if (mpris_init(cwin) == -1) {
		g_critical("Unable to initialize MPRIS");
		return NULL;
	}

	/* Init the gui after bancked to sink volume. */

	pragha_window_new (cwin);

	/* Toolbar Signals and Bindings. */

	toolbar = pragha_window_get_toolbar (cwin);
	g_signal_connect_swapped (toolbar, "prev",
	                          G_CALLBACK(pragha_playback_prev_track), cwin);
	g_signal_connect_swapped (toolbar, "play",
	                          G_CALLBACK(pragha_playback_play_pause_resume), cwin);
	g_signal_connect_swapped (toolbar, "stop",
	                          G_CALLBACK(pragha_playback_stop), cwin);
	g_signal_connect_swapped (toolbar, "next",
	                          G_CALLBACK(pragha_playback_next_track), cwin);
	g_signal_connect (toolbar, "unfull-activated",
	                  G_CALLBACK(pragha_window_unfullscreen), cwin);
	g_signal_connect (toolbar, "album-art-activated",
	                  G_CALLBACK(pragha_playback_show_current_album_art), cwin);
	g_signal_connect (toolbar, "track-info-activated",
	                  G_CALLBACK(pragha_playback_edit_current_track), cwin);
	g_signal_connect (toolbar, "track-progress-activated",
	                  G_CALLBACK(pragha_playback_seek_fraction), cwin);

	g_signal_connect (cwin->clibrary, "library-append-playlist",
	                  G_CALLBACK(pragha_library_pane_append_tracks), cwin);
	g_signal_connect (cwin->clibrary, "library-replace-playlist",
	                  G_CALLBACK(pragha_library_pane_replace_tracks), cwin);
	g_signal_connect (cwin->clibrary, "library-replace-playlist-and-play",
	                  G_CALLBACK(pragha_library_pane_replace_tracks_and_play), cwin);

	g_signal_connect (G_OBJECT(cwin->mainwindow), "window-state-event",
	                  G_CALLBACK(pragha_toolbar_window_state_event), toolbar);
	g_signal_connect (G_OBJECT(toolbar), "notify::timer-remaining-mode",
	                  G_CALLBACK(pragha_toolbar_show_ramaning_time_cb), cwin->backend);

	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK(pragha_toolbar_playback_state_cb), toolbar);
	g_signal_connect (cwin->backend, "tick",
	                 G_CALLBACK(pragha_toolbar_update_playback_progress), toolbar);
	g_signal_connect (cwin->backend, "buffering",
	                  G_CALLBACK(pragha_toolbar_update_buffering_cb), toolbar);

	g_object_bind_property (cwin->backend, "volume",
	                        toolbar, "volume",
	                        binding_flags);

	g_object_bind_property (cwin->preferences, "timer-remaining-mode",
	                        toolbar, "timer-remaining-mode",
	                        binding_flags);

	#ifdef HAVE_LIBGLYR
	cwin->glyr = pragha_glyr_new (cwin);
	#endif
	#ifdef HAVE_LIBCLASTFM
	cwin->clastfm = pragha_lastfm_new(cwin);
	#endif

	/* Init_gnome_media_keys requires constructed main window. */
	if (gnome_media_keys_will_be_useful()) {
		if (init_gnome_media_keys(cwin) == -1) {
			g_critical("Unable to initialize gnome media keys");
			return NULL;
		}
	}
	#ifdef HAVE_LIBKEYBINDER
	else if (keybinder_will_be_useful()) {
		if (init_keybinder(cwin) == -1) {
			g_critical("Unable to initialize keybinder");
			return NULL;
		}
	}
	#endif

	return cwin;
}

gint main(gint argc, gchar *argv[])
{
	struct con_win *cwin;
#ifdef DEBUG
	g_print ("debug enabled\n");
	pragha_main_thread = g_thread_self ();
#endif
	debug_level = 0;

	/* setup translation domain */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Force unicode to taglib. */
	taglib_set_strings_unicode(TRUE);

	/* Setup application name and pulseaudio role */
	g_set_application_name(_("Pragha Music Player"));
	g_setenv("PULSE_PROP_media.role", "audio", TRUE);

  /* Initialize the GThread system */
#if !GLIB_CHECK_VERSION(2,31,0)
	if (!g_thread_supported())
		g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2,35,1)
	g_type_init ();
#endif

	/* Initialize GTK+ */
	gtk_init(&argc, &argv);

	/* Get a instanse of pragha */
	cwin = pragha_application_new (argc, argv);
	if (cwin) {
		if (cwin->unique_instance) {
			/* Runs the main loop */
			CDEBUG(DBG_INFO, "Init done. Running ...");
			gtk_main();

			/* Close.. So, free memory and quit. */
			pragha_application_free (cwin);
		}
	}
	else
		return -1;

	return 0;
}
