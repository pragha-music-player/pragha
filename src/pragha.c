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
#include <tag_c.h>

#include "pragha-window.h"
#include "pragha-playback.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-menubar.h"
#include "pragha-keybinder.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha.h"

gint debug_level;
#ifdef DEBUG
GThread *pragha_main_thread = NULL;
#endif

struct _PraghaApplication {
	GApplication base_instance;

	/* Main window and icon */

	GtkWidget         *mainwindow;
	GdkPixbuf         *pixbuf_app;

	/* Main stuff */

	PraghaBackend     *backend;
	PraghaPreferences *preferences;
	PraghaDatabase    *cdbase;
	PraghaArtCache    *art_cache;

	PraghaScanner     *scanner;

	/* Main widgets */

	GtkUIManager      *menu_ui_manager;
	PraghaToolbar     *toolbar;
	GtkWidget         *infobox;
	GtkWidget         *pane;
	PraghaSidebar     *sidebar;
	PraghaLibraryPane *library;
	PraghaPlaylist    *playlist;
	PraghaStatusbar   *statusbar;

	PraghaStatusIcon  *status_icon;

	/* Plugins?. */

	PraghaNotify      *notify;
#ifdef HAVE_LIBGLYR
	PraghaGlyr        *glyr;
#endif
#ifdef HAVE_LIBCLASTFM
	PraghaLastfm      *clastfm;
#endif
	PraghaMpris2      *mpris2;
	con_gnome_media_keys *cgnome_media_keys;

#ifdef HAVE_LIBKEYBINDER
	gboolean           keybinder;
#endif
};

G_DEFINE_TYPE (PraghaApplication, pragha_application, G_TYPE_APPLICATION);

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

static void
pragha_application_construct_window (PraghaApplication *pragha)
{
	/* Main window */

	pragha->mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL); //TODO GtkApplicationWindow

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

static void
pragha_application_dispose (GObject *object)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (object);

	CDEBUG(DBG_INFO, "Cleaning up");

#ifdef HAVE_LIBGLYR
	if (pragha->glyr) {
		pragha_glyr_free (pragha->glyr);
		pragha->glyr = NULL;
	}
#endif
#ifdef HAVE_LIBCLASTFM
	if (pragha->clastfm) {
		pragha_lastfm_free (pragha->clastfm);
		pragha->clastfm = NULL;
	}
#endif
	if (pragha->sidebar) {
		pragha_sidebar_free (pragha->sidebar);
		pragha->sidebar = NULL;
	}
	if (pragha->mpris2) {
		pragha_mpris_free (pragha->mpris2);
		pragha->mpris2 = NULL;
	}
	if (pragha->backend) {
		pragha_playback_stop (pragha);
		g_object_unref (pragha->backend);
		pragha->backend = NULL;
	}
	if (pragha->art_cache) {
		pragha_art_cache_free (pragha->art_cache);
		pragha->art_cache = NULL;
	}
	if (pragha->cgnome_media_keys) {
		gnome_media_keys_free (pragha->cgnome_media_keys);
		pragha->cgnome_media_keys = NULL;
	}
#ifdef HAVE_LIBKEYBINDER
	if (pragha->keybinder) {
		keybinder_free ();
		pragha->keybinder = FALSE;
	}
#endif
	if (pragha->mainwindow) {
		pragha_window_free (pragha);
		/* Explicit destroy mainwindow to finalize lifecycle of childrens */
		gtk_widget_destroy (pragha->mainwindow);
		pragha->mainwindow = NULL;
	}
	if (pragha->scanner) {
		pragha_scanner_free (pragha->scanner);
		pragha->scanner = NULL;
	}
	if (pragha->notify) {
		pragha_notify_free (pragha->notify);
		pragha->notify = NULL;
	}

	pragha_cdda_free ();

	if (pragha->pixbuf_app) {
		g_object_unref (pragha->pixbuf_app);
		pragha->pixbuf_app = NULL;
	}

	if (pragha->menu_ui_manager) {
		g_object_unref (pragha->menu_ui_manager);
		pragha->menu_ui_manager = NULL;
	}

	/* Save Preferences and database. */

	if (pragha->preferences) {
		g_object_unref (pragha->preferences);
		pragha->preferences = NULL;
	}
	if (pragha->cdbase) {
		g_object_unref (pragha->cdbase);
		pragha->cdbase = NULL;
	}

	G_OBJECT_CLASS (pragha_application_parent_class)->dispose (object);
}

static void
pragha_application_startup (GApplication *application)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	PraghaToolbar *toolbar;
	PraghaPlaylist *playlist;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	G_APPLICATION_CLASS (pragha_application_parent_class)->startup (application);

	/* Allocate memory for simple structures */

	pragha->mpris2 = pragha_mpris_new();

	pragha->preferences = pragha_preferences_get();

	pragha->cdbase = pragha_database_get();
	if (pragha_database_start_successfully(pragha->cdbase) == FALSE) {
		g_error("Unable to init music dbase");
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

	g_application_hold (application); //TODO don't hold if gtkapp
}

static void
pragha_application_activate (GApplication *application)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	gtk_window_present (GTK_WINDOW (pragha->mainwindow));
}

static void
pragha_application_open (GApplication *application, GFile **files, gint n_files, const gchar *hint)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);
	gint i;
	GList *mlist = NULL;

	for (i = 0; i < n_files; i++) {
		gchar *path = g_file_get_path (files[i]);
		mlist = append_mobj_list_from_unknown_filename (mlist, path);
		g_free (path);
	}

	if (mlist) {
		pragha_playlist_append_mobj_list (pragha->playlist, mlist);
		g_list_free (mlist);
	}

	gtk_window_present (GTK_WINDOW (pragha->mainwindow));
}

static int
pragha_application_command_line (GApplication *application, GApplicationCommandLine *command_line)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);
	int ret = 0;
	gint argc;

	gchar **argv = g_application_command_line_get_arguments (command_line, &argc);

	if (argc <= 1) {
		pragha_application_activate (application);
		goto exit;
	}

	ret = handle_command_line (pragha, command_line, argc, argv);

exit:
	g_strfreev (argv);

	return ret;
}

//it's used for --help and --version
static gboolean
pragha_application_local_command_line (GApplication *application, gchar ***arguments, int *exit_status)
{
	PraghaApplication *pragha = PRAGHA_APPLICATION (application);

	gchar **argv = *arguments;
	gint argc = g_strv_length (argv);

	*exit_status = handle_command_line (pragha, NULL, argc, argv);

	return FALSE;
}

//TODO consider use of GApplication::shutdown to save preferences and playlist

void
pragha_application_quit (PraghaApplication *pragha)
{
#if GLIB_CHECK_VERSION (2, 32, 0)
	g_application_quit (G_APPLICATION (pragha));
#else
	g_application_release (G_APPLICATION (pragha));
#endif
}

static void
pragha_application_class_init (PraghaApplicationClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GApplicationClass *application_class = G_APPLICATION_CLASS (class);

	object_class->dispose = pragha_application_dispose;

	application_class->startup = pragha_application_startup;
	application_class->activate = pragha_application_activate;
	application_class->open = pragha_application_open;
	application_class->command_line = pragha_application_command_line;
	application_class->local_command_line = pragha_application_local_command_line;
}

static void
pragha_application_init (PraghaApplication *pragha)
{
}

PraghaApplication *
pragha_application_new ()
{
	return g_object_new (PRAGHA_TYPE_APPLICATION,
	                     "application-id", "org.pragha",
	                     "flags", G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}

gint main(gint argc, gchar *argv[])
{
	PraghaApplication *pragha;
	int status;
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
	taglib_set_string_management_enabled(FALSE);

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
	gtk_init(&argc, &argv); //TODO delete if gtk app

	pragha = pragha_application_new ();
	status = g_application_run (G_APPLICATION (pragha), argc, argv);
	g_object_unref (pragha);

	return status;
}
