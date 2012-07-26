/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2012 matias <mati86dl@gmail.com>			 */
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
#include <locale.h> /* require LC_ALL */

gint debug_level;

/* FIXME: Cleanup track refs */
static void common_cleanup(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Cleaning up");

	backend_free (cwin);
	gui_free (cwin);
	state_free (cwin->cstate);
	preferences_free (cwin->cpref);
#ifdef HAVE_LIBGLYR
	uninit_glyr_related (cwin);
#endif
	db_free (cwin->cdbase);
#ifdef HAVE_LIBCLASTFM
	lastfm_free (cwin->clastfm);
#endif
	dbus_handlers_free (cwin);
	mpris_free (cwin->cmpris2);
	if (notify_is_initted())
		notify_uninit();

	if (cwin->cgnome_media_keys)
		gnome_media_keys_free (cwin->cgnome_media_keys);
#ifdef HAVE_LIBKEYBINDER
	else
		keybinder_free ();
#endif

	g_slice_free(struct con_win, cwin);
}

void exit_pragha(GtkWidget *widget, struct con_win *cwin)
{
	if (cwin->cpref->save_playlist)
		save_current_playlist_state(cwin);
	save_preferences(cwin);

	gtk_main_quit();

	CDEBUG(DBG_INFO, "Halt.");
}

gint main(gint argc, gchar *argv[])
{
	struct con_win *cwin;

	cwin = g_slice_new0(struct con_win);
	cwin->pixbuf = g_slice_new0(struct pixbuf);
	cwin->cpref = g_slice_new0(struct con_pref);
	cwin->cstate = g_slice_new0(struct con_state);
	cwin->cgst = g_slice_new0(struct con_gst);
#ifdef HAVE_LIBCLASTFM
	cwin->clastfm = g_slice_new0(struct con_lastfm);
	cwin->clastfm->ntags = g_slice_new0(struct tags);
#endif
	cwin->cmpris2 = g_slice_new0(struct con_mpris2);

	if(init_first_state(cwin) == -1)
		return -1;

	debug_level = 0;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	if (init_threads(cwin) == -1) {
		g_critical("Unable to init threads");
		return -1;
	}

	if (init_dbus(cwin) == -1) {
		g_critical("Unable to init dbus connection");
		return -1;
	}

	if (init_dbus_handlers(cwin) == -1) {
		g_critical("Unable to initialize DBUS filter handlers");
		return -1;
	}

	if (init_options(cwin, argc, argv) == -1)
		return -1;

	/* Allow only one instance */

	if (!cwin->cstate->unique_instance)
		return 0;

	if (init_config(cwin) == -1) {
		g_critical("Unable to init configuration");
		return -1;
	}

	if (init_musicdbase(cwin) == -1) {
		g_critical("Unable to init music dbase");
		return -1;
	}

	if (init_notify(cwin) == -1) {
		g_critical("Unable to initialize libnotify");
		return -1;
	}

	#ifdef HAVE_LIBCLASTFM
	if (init_lastfm_idle(cwin) == -1) {
		g_critical("Unable to initialize lastfm");
	}
	#endif

	#ifdef HAVE_LIBGLYR
	if (init_glyr_related(cwin) == -1) {
		g_critical("Unable to initialize libglyr");
	}
	#endif

	if (mpris_init(cwin) == -1) {
		g_critical("Unable to initialize MPRIS");
		return -1;
	}

	if(backend_init(cwin) == -1) {
		g_critical("Unable to initialize gstreamer");
		return -1;
	}

	/* Init the gui after bancked to sink volume. */
	gdk_threads_enter();
	init_gui(argc, argv, cwin);

	/* Init_gnome_media_keys requires constructed main window. */
	if (gnome_media_keys_will_be_useful()) {
		if (init_gnome_media_keys(cwin) == -1) {
			g_critical("Unable to initialize gnome media keys");
			return -1;
		}
	}
	#ifdef HAVE_LIBKEYBINDER
	else if (init_keybinder(cwin) == -1) {
		g_critical("Unable to initialize keybinder");
		return -1;
	}
	#endif

	CDEBUG(DBG_INFO, "Init done. Running ...");

	gtk_main();
	gdk_threads_leave();
	common_cleanup(cwin);

	return 0;
}
