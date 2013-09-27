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
#include "pragha-musicobject-mgmt.h"
#include "pragha-library-pane.h"
#include "pragha-menubar.h"
#include "pragha-statusicon.h"
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
pragha_library_pane_append_tracks (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (pragha->playlist);

		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);
		g_list_free(list);
	}
}

static void
pragha_library_pane_replace_tracks_and_play (PraghaLibraryPane *library, PraghaApplication *pragha)
{
	GList *list = NULL;
	list = pragha_library_pane_get_mobj_list (library);
	if (list) {
		pragha_playlist_remove_all (pragha->playlist);

		pragha_playlist_append_mobj_list (pragha->playlist,
			                              list);

		if (pragha_backend_get_state (pragha->backend) != ST_STOPPED)
			pragha_playback_next_track(pragha);
		else
			pragha_playback_play_pause_resume(pragha);

		g_list_free(list);
	}
}

static void
pragha_playlist_update_change_tags (PraghaPlaylist *playlist, gint changed, PraghaMusicobject *mobj, PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaMpris2 *mpris2;
	PraghaMusicobject *cmobj = NULL;

	backend = pragha_application_get_backend (pragha);

	if(pragha_backend_get_state (backend) != ST_STOPPED) {
		cmobj = pragha_backend_get_musicobject (backend);
		pragha_update_musicobject_change_tag (cmobj, changed, mobj);

		toolbar = pragha_application_get_toolbar (pragha);
		pragha_toolbar_set_title (toolbar, cmobj);

		mpris2 = pragha_application_get_mpris2 (pragha);
		pragha_mpris_update_metadata_changed (mpris2);
	}
}

static void
pragha_playlist_update_statusbar_playtime (PraghaPlaylist *playlist, PraghaApplication *pragha)
{
	PraghaStatusbar *statusbar;
	gint total_playtime = 0, no_tracks = 0;
	gchar *str, *tot_str;

	if(pragha_playlist_is_changing(playlist))
		return;

	total_playtime = pragha_playlist_get_total_playtime (playlist);
	no_tracks = pragha_playlist_get_no_tracks (playlist);

	tot_str = convert_length_str(total_playtime);
	str = g_strdup_printf("%i %s - %s",
	                      no_tracks,
	                      ngettext("Track", "Tracks", no_tracks),
	                      tot_str);

	CDEBUG(DBG_VERBOSE, "Updating status bar with new playtime: %s", tot_str);

	statusbar = pragha_application_get_statusbar (pragha);
	pragha_statusbar_set_main_text(statusbar, str);

	g_free(tot_str);
	g_free(str);
}

/*
 * Some public actions.
 */

PraghaPreferences *
pragha_application_get_preferences (PraghaApplication *pragha)
{
	return pragha->preferences;
}

PraghaDatabase *
pragha_application_get_database (PraghaApplication *pragha)
{
	return pragha->cdbase;
}

PraghaArtCache *
pragha_application_get_art_cache (PraghaApplication *pragha)
{
	return pragha->art_cache;
}

PraghaBackend *
pragha_application_get_backend (PraghaApplication *pragha)
{
	return pragha->backend;
}

PraghaScanner *
pragha_application_get_scanner (PraghaApplication *pragha)
{
	return pragha->scanner;
}

GtkWidget *
pragha_application_get_window (PraghaApplication *pragha)
{
	return pragha->mainwindow;
}

GdkPixbuf *
pragha_application_get_pixbuf_app (PraghaApplication *pragha)
{
	return pragha->pixbuf_app;
}

PraghaPlaylist *
pragha_application_get_playlist (PraghaApplication *pragha)
{
	return pragha->playlist;
}

PraghaLibraryPane *
pragha_application_get_library (PraghaApplication *pragha)
{
	return pragha->library;
}

PraghaToolbar *
pragha_application_get_toolbar (PraghaApplication *pragha)
{
	return pragha->toolbar;
}

PraghaSidebar *
pragha_application_get_sidebar (PraghaApplication *pragha)
{
	return pragha->sidebar;
}

PraghaStatusbar *
pragha_application_get_statusbar (PraghaApplication *pragha)
{
	return pragha->statusbar;
}

PraghaStatusIcon *
pragha_application_get_status_icon (PraghaApplication *pragha)
{
	return pragha->status_icon;
}

GtkUIManager *
pragha_application_get_menu_ui_manager (PraghaApplication *pragha)
{
	return pragha->menu_ui_manager;
}

GtkAction *
pragha_application_get_menu_action (PraghaApplication *pragha, const gchar *path)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_action (ui_manager, path);
}

GtkWidget *
pragha_application_get_menu_action_widget (PraghaApplication *pragha, const gchar *path)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_widget (ui_manager, path);
}

GtkWidget *
pragha_application_get_menubar (PraghaApplication *pragha)
{
	GtkUIManager *ui_manager = pragha_application_get_menu_ui_manager (pragha);

	return gtk_ui_manager_get_widget (ui_manager, "/Menubar");
}

GtkWidget *
pragha_application_get_infobox_container (PraghaApplication *pragha)
{
	return pragha->infobox;
}

GtkWidget *
pragha_application_get_pane (PraghaApplication *pragha)
{
	return pragha->pane;
}

PraghaNotify *
pragha_application_get_notify (PraghaApplication *pragha)
{
	return pragha->notify;
}

void
pragha_application_set_notify (PraghaApplication *pragha, PraghaNotify *notify)
{
	pragha->notify = notify;
}

PraghaMpris2 *
pragha_application_get_mpris2 (PraghaApplication *pragha)
{
	return pragha->mpris2;
}

#ifdef HAVE_LIBCLASTFM
PraghaLastfm *
pragha_application_get_lastfm (PraghaApplication *pragha)
{
	return pragha->clastfm;
}
#endif

#ifdef HAVE_LIBGLYR
PraghaGlyr *
pragha_application_get_glyr (PraghaApplication *pragha)
{
	return pragha->glyr;
}
#endif

gboolean
pragha_application_is_first_run (PraghaApplication *pragha)
{
	return string_is_empty (pragha_preferences_get_installed_version (pragha->preferences));
}

void
pragha_application_quit (void)
{
	gtk_main_quit();

	CDEBUG(DBG_INFO, "Halt.");
}

/*
 *
 */
static void
pragha_application_construct_window (PraghaApplication *pragha)
{
	/* Main window */

	pragha->mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	pragha->pixbuf_app = gdk_pixbuf_new_from_file (PIXMAPDIR"/pragha.png", NULL);
	if (!pragha->pixbuf_app)
		g_warning("Unable to load pragha png");
	else
		gtk_window_set_icon (GTK_WINDOW(pragha->mainwindow),
		                     pragha->pixbuf_app);
	
	gtk_window_set_title(GTK_WINDOW(pragha->mainwindow), _("Pragha Music Player"));

	/* Get all widgets instances */

	pragha->menu_ui_manager = pragha_menubar_new ();
	pragha->toolbar = pragha_toolbar_new ();
	pragha->infobox = gtk_vbox_new (FALSE, 0);
	pragha->pane = gtk_hpaned_new ();
	pragha->sidebar = pragha_sidebar_new ();
	pragha->library = pragha_library_pane_new ();
	pragha->playlist = pragha_playlist_new ();
	pragha->statusbar = pragha_statusbar_get ();
	pragha->scanner = pragha_scanner_new();

	pragha->status_icon = pragha_status_icon_new (pragha);

	pragha_menubar_connect_signals (pragha->menu_ui_manager, pragha);

	/* Contruct the window. */

	pragha_window_new (pragha);
}

/* FIXME: Cleanup track refs */
static void
pragha_application_free (PraghaApplication *pragha)
{
	CDEBUG(DBG_INFO, "Cleaning up");

	pragha_playback_stop(pragha);

#ifdef HAVE_LIBGLYR
	pragha_glyr_free (pragha->glyr);
#endif
#ifdef HAVE_LIBCLASTFM
	pragha_lastfm_free (pragha->clastfm);
#endif
	pragha_sidebar_free(pragha->sidebar);
	pragha_mpris_free (pragha->mpris2);
	g_object_unref (pragha->backend);
	pragha_art_cache_free (pragha->art_cache);
	pragha_window_free (pragha);
	pragha_scanner_free(pragha->scanner);
	dbus_handlers_free (pragha);
	if (pragha->notify)
		pragha_notify_free (pragha->notify);

	if (pragha->cgnome_media_keys)
		gnome_media_keys_free (pragha->cgnome_media_keys);
#ifdef HAVE_LIBKEYBINDER
	else if (pragha->keybinder)
		keybinder_free ();
#endif
	pragha_cdda_free ();

	if (pragha->pixbuf_app)
		g_object_unref(pragha->pixbuf_app);

	g_object_unref (pragha->menu_ui_manager);

	/* Explicit destroy mainwindow to finalize lifecycle of childrens */

	gtk_widget_destroy (pragha->mainwindow);

	/* Save Preferences and database. */

	g_object_unref(G_OBJECT(pragha->preferences));
	g_object_unref(pragha->cdbase);

	g_slice_free(PraghaApplication, pragha);
}

static PraghaApplication *
pragha_application_new (gint argc, gchar *argv[])
{
	PraghaToolbar *toolbar;
	PraghaPlaylist *playlist;
	PraghaApplication *pragha;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	/* Alocate the main structure. */
	pragha = g_slice_new0(PraghaApplication);

	/* Allocate memory for simple structures */

	pragha->mpris2 = pragha_mpris_new();

	pragha->con_dbus = pragha_init_dbus(pragha);
	if (!pragha->con_dbus) {
		g_critical("Unable to init dbus connection");
		return NULL;
	}
	if (pragha_init_dbus_handlers(pragha) == -1) {
		g_critical("Unable to initialize DBUS filter handlers");
		return NULL;
	}

	/* Parse command line arguments */
	if (init_options(pragha, argc, argv) == -1)
		return NULL;

	/* Allow only one instance */
	if (!pragha->unique_instance)
		return pragha;

	pragha->preferences = pragha_preferences_get();

	pragha->cdbase = pragha_database_get();
	if (pragha_database_start_successfully(pragha->cdbase) == FALSE) {
		g_critical("Unable to init music dbase");
		return NULL;
	}

	if (pragha_preferences_get_show_osd (pragha->preferences))
		pragha->notify = pragha_notify_new (pragha);
	else
		pragha->notify = NULL;

	pragha->art_cache = pragha_art_cache_new ();

	pragha->backend = pragha_backend_new (pragha);

	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK(pragha_backend_notificate_new_state), pragha);
	g_signal_connect (pragha->backend, "finished",
	                  G_CALLBACK(pragha_backend_finished_song), pragha);
	g_signal_connect (pragha->backend, "tags-changed",
	                  G_CALLBACK(pragha_backend_tags_changed), pragha);

	g_signal_connect (pragha->backend, "error",
	                 G_CALLBACK(gui_backend_error_show_dialog_cb), pragha);
	g_signal_connect (pragha->backend, "error",
	                  G_CALLBACK(gui_backend_error_update_current_playlist_cb), pragha);
	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), pragha);

	if (pragha_mpris_init(pragha->mpris2, pragha) == -1) {
		g_critical("Unable to initialize MPRIS");
		return NULL;
	}

	/*
	 * Collect widgets and construct the window.
	 */

	pragha_application_construct_window (pragha);

	/* Connect Signals and Bindings. */

	toolbar = pragha->toolbar;
	g_signal_connect_swapped (toolbar, "prev",
	                          G_CALLBACK(pragha_playback_prev_track), pragha);
	g_signal_connect_swapped (toolbar, "play",
	                          G_CALLBACK(pragha_playback_play_pause_resume), pragha);
	g_signal_connect_swapped (toolbar, "stop",
	                          G_CALLBACK(pragha_playback_stop), pragha);
	g_signal_connect_swapped (toolbar, "next",
	                          G_CALLBACK(pragha_playback_next_track), pragha);
	g_signal_connect (toolbar, "unfull-activated",
	                  G_CALLBACK(pragha_window_unfullscreen), pragha);
	g_signal_connect (toolbar, "album-art-activated",
	                  G_CALLBACK(pragha_playback_show_current_album_art), pragha);
	g_signal_connect (toolbar, "track-info-activated",
	                  G_CALLBACK(pragha_playback_edit_current_track), pragha);
	g_signal_connect (toolbar, "track-progress-activated",
	                  G_CALLBACK(pragha_playback_seek_fraction), pragha);

	playlist = pragha->playlist;
	g_signal_connect (playlist, "playlist-set-track",
	                  G_CALLBACK(pragha_playback_set_playlist_track), pragha);
	g_signal_connect (playlist, "playlist-change-tags",
	                  G_CALLBACK(pragha_playlist_update_change_tags), pragha);
	g_signal_connect (playlist, "playlist-changed",
	                  G_CALLBACK(pragha_playlist_update_statusbar_playtime), pragha);
	pragha_playlist_update_statusbar_playtime (playlist, pragha);
		
	g_signal_connect (pragha->library, "library-append-playlist",
	                  G_CALLBACK(pragha_library_pane_append_tracks), pragha);
	g_signal_connect (pragha->library, "library-replace-playlist",
	                  G_CALLBACK(pragha_library_pane_replace_tracks), pragha);
	g_signal_connect (pragha->library, "library-replace-playlist-and-play",
	                  G_CALLBACK(pragha_library_pane_replace_tracks_and_play), pragha);

	g_signal_connect (G_OBJECT(pragha->mainwindow), "window-state-event",
	                  G_CALLBACK(pragha_toolbar_window_state_event), toolbar);
	g_signal_connect (G_OBJECT(toolbar), "notify::timer-remaining-mode",
	                  G_CALLBACK(pragha_toolbar_show_ramaning_time_cb), pragha->backend);

	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK(pragha_toolbar_playback_state_cb), toolbar);
	g_signal_connect (pragha->backend, "tick",
	                 G_CALLBACK(pragha_toolbar_update_playback_progress), toolbar);
	g_signal_connect (pragha->backend, "buffering",
	                  G_CALLBACK(pragha_toolbar_update_buffering_cb), toolbar);

	g_signal_connect (pragha->backend, "notify::state",
	                  G_CALLBACK (update_current_playlist_view_playback_state_cb), pragha->playlist);

	g_object_bind_property (pragha->backend, "volume",
	                        toolbar, "volume",
	                        binding_flags);

	g_object_bind_property (pragha->preferences, "timer-remaining-mode",
	                        toolbar, "timer-remaining-mode",
	                        binding_flags);

	#ifdef HAVE_LIBGLYR
	pragha->glyr = pragha_glyr_new (pragha);
	#endif
	#ifdef HAVE_LIBCLASTFM
	pragha->clastfm = pragha_lastfm_new(pragha);
	#endif

	/* Init_gnome_media_keys requires constructed main window. */
	if (gnome_media_keys_will_be_useful()) {
	    pragha->cgnome_media_keys = init_gnome_media_keys (pragha);
	}
	#ifdef HAVE_LIBKEYBINDER
	else if (keybinder_will_be_useful()) {
		pragha->keybinder = init_keybinder (pragha);
	}
	#endif

	return pragha;
}

gint main(gint argc, gchar *argv[])
{
	PraghaApplication *pragha;
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
	pragha = pragha_application_new (argc, argv);
	if (pragha) {
		if (pragha->unique_instance) {
			/* Runs the main loop */
			CDEBUG(DBG_INFO, "Init done. Running ...");
			gtk_main();

			/* Close.. So, free memory and quit. */
			pragha_application_free (pragha);
		}
	}
	else
		return -1;

	return 0;
}
