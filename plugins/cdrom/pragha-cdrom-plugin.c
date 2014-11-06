/*************************************************************************/
/* Copyright (C) 2009-2014 matias <mati86dl@gmail.com>                   */
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <cdio/paranoia/cdda.h>
#include <cdio/cd_types.h>
#include <cddb/cddb.h>

#include <libpeas/peas.h>

#include "src/pragha.h"
#include "src/pragha-hig.h"
#include "src/pragha-utils.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-statusicon.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-window.h"

#if HAVE_GUDEV
#include "plugins/devices/pragha-devices-plugin.h"
#include "plugins/devices/pragha-device-client.h"
#endif

#include "plugins/pragha-plugin-macros.h"

#define PRAGHA_TYPE_CDROM_PLUGIN         (pragha_cdrom_plugin_get_type ())
#define PRAGHA_CDROM_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_CDROM_PLUGIN, PraghaCdromPlugin))
#define PRAGHA_CDROM_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_CDROM_PLUGIN, PraghaCdromPlugin))
#define PRAGHA_IS_CDROM_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_CDROM_PLUGIN))
#define PRAGHA_IS_CDROM_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_CDROM_PLUGIN))
#define PRAGHA_CDROM_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_CDROM_PLUGIN, PraghaCdromPluginClass))

struct _PraghaCdromPluginPrivate {
	PraghaApplication *pragha;

	GtkWidget          *device_setting_widget;
	GtkWidget          *audio_cd_device_w;
	GtkWidget          *cddb_setting_widget;
	GtkWidget          *use_cddb_w;

	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;
	guint              merge_id_syst_menu;
};
typedef struct _PraghaCdromPluginPrivate PraghaCdromPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_CDROM_PLUGIN,
                        PraghaCdromPlugin,
                        pragha_cdrom_plugin)

static PraghaMusicobject *
new_musicobject_from_cdda (PraghaApplication *pragha,
                           cdrom_drive_t *cdda_drive,
                           cddb_disc_t *cddb_disc,
                           gint track_no)
{
	PraghaPreferences *preferences;
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicobject *mobj = NULL;
	gint channels, start, end;
	gchar *ntitle = NULL, *nfile = NULL;

	CDEBUG(DBG_PLUGIN, "Creating new musicobject from cdda: %d", track_no);

	channels = cdio_get_track_channels(cdda_drive->p_cdio,
					   track_no);
	start = cdio_cddap_track_firstsector(cdda_drive, track_no);
	end = cdio_cddap_track_lastsector(cdda_drive, track_no);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT, NULL);

	preferences = pragha_application_get_preferences (pragha);
	if (pragha_preferences_get_use_cddb (preferences) && cddb_disc) {
		cddb_track_t *track;
		const gchar *title, *artist, *album, *genre;
		gint year;

		track = cddb_disc_get_track(cddb_disc, track_no - 1);
		if (track) {
			title = cddb_track_get_title(track);
			if (title)
				ntitle = g_strdup(title);

			artist = cddb_track_get_artist(track);
			if(artist)
				pragha_musicobject_set_artist(mobj, artist);

			album = cddb_disc_get_title(cddb_disc);
			if(album)
				pragha_musicobject_set_album(mobj, album);

			year = cddb_disc_get_year(cddb_disc);
			if(year)
				pragha_musicobject_set_year(mobj, year);

			genre = cddb_disc_get_genre(cddb_disc);
			if(genre)
				pragha_musicobject_set_genre(mobj, genre);
		}
	}

	enum_map = pragha_music_enum_get ();
	pragha_musicobject_set_source (mobj, pragha_music_enum_map_get(enum_map, "FILE_CDDA"));
	g_object_unref (enum_map);

	nfile = g_strdup_printf("cdda://%d", track_no);
	pragha_musicobject_set_file(mobj, nfile);
	pragha_musicobject_set_track_no(mobj, track_no);

	if (!ntitle)
		ntitle = g_strdup_printf("Track %d", track_no);
	pragha_musicobject_set_title(mobj, ntitle);

	pragha_musicobject_set_length(mobj, (end - start) / CDIO_CD_FRAMES_PER_SEC);
	pragha_musicobject_set_channels(mobj, (channels > 0) ? channels : 0);

	g_free(nfile);
	g_free(ntitle);

	return mobj;
}

static gint
cddb_add_tracks (cdrom_drive_t *cdda_drive, cddb_disc_t *cddb_disc)
{
	cddb_track_t *track;
	lba_t lba;
	gint num_tracks, first_track, i = 0;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return -1;

	first_track = cdio_get_first_track_num(cdda_drive->p_cdio);
	for (i = first_track; i <= num_tracks; i++) {
		track = cddb_track_new();
		if (!track)
			return -1;

		lba = cdio_get_track_lba(cdda_drive->p_cdio, i);
		if (lba == CDIO_INVALID_LBA)
			return -1;

		cddb_disc_add_track(cddb_disc, track);
		cddb_track_set_frame_offset(track, lba);
	}

	return 0;
}

static void
add_audio_cd_tracks (PraghaApplication *pragha, cdrom_drive_t *cdda_drive, cddb_disc_t *cddb_disc)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj;

	gint num_tracks = 0, i = 0;
	GList *list = NULL;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return;

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda(pragha, cdda_drive, cddb_disc, i);
		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);

		pragha_process_gtk_events ();
	}
	if (list) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list(playlist, list);
		g_list_free (list);
	}
}

static cdrom_drive_t*
find_audio_cd (PraghaApplication *pragha)
{
	cdrom_drive_t *drive = NULL;
	gchar **cdda_devices = NULL;
	PraghaPreferences *preferences;

	preferences = pragha_application_get_preferences (pragha);
	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device(preferences);

	if (!audio_cd_device) {
		cdda_devices = cdio_get_devices_with_cap(NULL, CDIO_FS_AUDIO, FALSE);
		if (!cdda_devices || (cdda_devices && !*cdda_devices)) {
			g_warning("No Audio CD found");
			return NULL;
		}

		CDEBUG(DBG_PLUGIN, "Trying Audio CD Device: %s", *cdda_devices);

		drive = cdio_cddap_identify(*cdda_devices, 0, NULL);
		if (!drive) {
			g_warning("Unable to identify Audio CD");
			goto exit;
		}
	} else {
		CDEBUG(DBG_PLUGIN, "Trying Audio CD Device: %s", audio_cd_device);

		drive = cdio_cddap_identify(audio_cd_device, 0, NULL);
		if (!drive) {
			g_warning("Unable to identify Audio CD");
			return NULL;
		}
	}
exit:
	if (cdda_devices)
		cdio_free_device_list(cdda_devices);

	return drive;
}

void
pragha_application_append_audio_cd (PraghaApplication *pragha)
{
	lba_t lba;
	gint matches;
	cddb_disc_t *cddb_disc = NULL;
	cddb_conn_t *cddb_conn = NULL;
	PraghaPreferences *preferences;

	cdrom_drive_t *cdda_drive = find_audio_cd(pragha);
	if (!cdda_drive)
		return;

	if (cdio_cddap_open(cdda_drive)) {
		g_warning("Unable to open Audio CD");
		return;
	}

	preferences = pragha_application_get_preferences (pragha);
	if (pragha_preferences_get_use_cddb (preferences)) {
		cddb_conn = cddb_new ();
		if (!cddb_conn)
			goto add;

		cddb_disc = cddb_disc_new();
		if (!cddb_disc)
			goto add;

		lba = cdio_get_track_lba(cdda_drive->p_cdio,
					 CDIO_CDROM_LEADOUT_TRACK);
		if (lba == CDIO_INVALID_LBA)
			goto add;

		cddb_disc_set_length(cddb_disc, FRAMES_TO_SECONDS(lba));
		if (cddb_add_tracks(cdda_drive, cddb_disc) < 0)
			goto add;

		if (!cddb_disc_calc_discid(cddb_disc))
			goto add;

		cddb_disc_set_category(cddb_disc, CDDB_CAT_MISC);

		matches = cddb_query(cddb_conn, cddb_disc);
		if (matches == -1)
			goto add;

		if (!cddb_read(cddb_conn,
			       cddb_disc)) {
			cddb_error_print(cddb_errno(cddb_conn));
			goto add;
		}

		CDEBUG(DBG_PLUGIN, "Successfully initialized CDDB");
		goto add;
	}

add:
	add_audio_cd_tracks(pragha, cdda_drive, cddb_disc);
	CDEBUG(DBG_PLUGIN, "Successfully opened Audio CD device");

	if (cdda_drive)
		cdio_cddap_close(cdda_drive);
	if (cddb_disc)
		cddb_disc_destroy(cddb_disc);
	if (cddb_conn)
		cddb_destroy(cddb_conn);
}

static gboolean
pragha_musicobject_is_cdda_type (PraghaMusicobject *mobj)
{
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicSource file_source = FILE_NONE;

	enum_map = pragha_music_enum_get ();
	file_source = pragha_music_enum_map_get(enum_map, "FILE_CDDA");
	g_object_unref (enum_map);

	return (file_source == pragha_musicobject_get_source (mobj));
}

static void
pragha_cdrom_plugin_set_device (PraghaBackend *backend, GObject *obj, gpointer user_data)
{
	PraghaMusicobject *mobj = NULL;
	GObject *source;

	PraghaCdromPlugin *plugin = user_data;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_cdda_type (mobj))
		return;

	g_object_get (obj, "source", &source, NULL);
	if (source) {
		PraghaPreferences *preferences = pragha_application_get_preferences (priv->pragha);
		const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device (preferences);
		if (audio_cd_device) {
			g_object_set (source, "device", audio_cd_device, NULL);
		}
		g_object_unref (source);
	}
}

static void
pragha_cdrom_plugin_prepare_source (PraghaBackend *backend, gpointer user_data)
{
	PraghaMusicobject *mobj;
	const gchar *uri = NULL;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_cdda_type (mobj))
		return;

	uri = pragha_musicobject_get_file (mobj);
	pragha_backend_set_playback_uri (backend, uri);
}

/*
 * GUDEV signals.
 */

#ifdef HAVE_GUDEV
static void
pragha_cdrom_plugin_device_added_response (GtkWidget *dialog,
                                           gint       response,
                                           gpointer   user_data)
{
	PraghaCdromPlugin *plugin = user_data;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	switch (response) {
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_application_append_audio_cd (priv->pragha);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}

	gtk_widget_destroy (dialog);
}

static void
pragha_cdrom_plugin_device_added (PraghaDeviceClient *device_client,
                                  PraghaDeviceType    device_type,
                                  GUdevDevice        *u_device,
                                  gpointer            user_data)
{
	GtkWidget *dialog;

	PraghaCdromPlugin *plugin = user_data;

	if (device_type != PRAGHA_DEVICE_AUDIO_CD)
		return;

	dialog = pragha_gudev_dialog_new (NULL, _("Audio/Data CD"), "media-optical",
	                                 _("Was inserted an Audio Cd."), NULL,
	                                 _("Add Audio _CD"), PRAGHA_DEVICE_RESPONSE_PLAY);

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_cdrom_plugin_device_added_response), plugin);

	gtk_widget_show_all (dialog);
}

void
pragha_cdrom_plugin_device_removed (PraghaDeviceClient *device_client,
                                    PraghaDeviceType    device_type,
                                    GUdevDevice        *u_device,
                                    gpointer            user_data)
{
	if (device_type != PRAGHA_DEVICE_AUDIO_CD)
		return;

	g_print ("CDROM REMOVEDDDDD.. Cri cri.. never detect it.. .\n");
}
#endif


/*
 * Menubar
 */
static void
pragha_cdrom_plugin_append_action (GtkAction *action, PraghaCdromPlugin *plugin)
{
	PraghaCdromPluginPrivate *priv = plugin->priv;
	pragha_application_append_audio_cd (priv->pragha);
}

static const GtkActionEntry main_menu_actions [] = {
	{"Add Audio CD", "media-optical", N_("Add Audio _CD"),
	 "", "Append a Audio CD", G_CALLBACK(pragha_cdrom_plugin_append_action)}
};

static const gchar *main_menu_xml = "<ui>							\
	<menubar name=\"Menubar\">										\
		<menu action=\"PlaylistMenu\">								\
			<placeholder name=\"pragha-append-music-placeholder\">	\
				<menuitem action=\"Add Audio CD\"/>					\
			</placeholder>											\
		</menu>														\
	</menubar>														\
</ui>";

static const gchar *syst_menu_xml = "<ui>							\
	<popup>															\
	<placeholder name=\"pragha-append-music-placeholder\">			\
		<menuitem action=\"Add Audio CD\"/>							\
	</placeholder>													\
	</popup>														\
	</ui>";

/*
 * Cdrom Settings
 */
static void
pragha_cdrom_preferences_dialog_response (GtkDialog         *dialog_w,
                                          gint               response_id,
                                          PraghaCdromPlugin *plugin)
{
	PraghaPreferences *preferences;
	const gchar *audio_cd_device;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		break;
	case GTK_RESPONSE_OK:
		preferences = pragha_preferences_get();
		audio_cd_device = gtk_entry_get_text (GTK_ENTRY(priv->audio_cd_device_w));
		if (audio_cd_device) {
			pragha_preferences_set_audio_cd_device (preferences, audio_cd_device);
		}
		pragha_preferences_set_use_cddb (preferences,
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->use_cddb_w)));
		g_object_unref (preferences);
		break;
	default:
		break;
	}
}

static void
pragha_cdrom_init_settings (PraghaCdromPlugin *plugin)
{
	PraghaPreferences *preferences;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get();

	const gchar *audio_cd_device = pragha_preferences_get_audio_cd_device (preferences);

	if (string_is_not_empty(audio_cd_device))
		gtk_entry_set_text(GTK_ENTRY(priv->audio_cd_device_w), audio_cd_device);

	if (pragha_preferences_get_use_cddb(preferences))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_cddb_w), TRUE);

	g_object_unref (preferences);
}

static void
pragha_cdrom_plugin_append_setting (PraghaCdromPlugin *plugin)
{
	GtkWidget *table;
	GtkWidget *audio_cd_device_label,*audio_cd_device_entry, *use_cddb;
	guint row = 0;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	/* Cd Device */

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Audio CD"));

	audio_cd_device_label = gtk_label_new(_("Audio CD Device"));
	gtk_misc_set_alignment (GTK_MISC (audio_cd_device_label), 0, 0);

	audio_cd_device_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(audio_cd_device_entry), AUDIO_CD_DEVICE_ENTRY_LEN);
	gtk_entry_set_activates_default (GTK_ENTRY(audio_cd_device_entry), TRUE);

	pragha_hig_workarea_table_add_row (table, &row, audio_cd_device_label, audio_cd_device_entry);

	/* Store references */

	priv->device_setting_widget = table;
	priv->audio_cd_device_w = audio_cd_device_entry;

	/* CDDB Option */
	row = 0;
	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title (table, &row, "CDDB");

	use_cddb = gtk_check_button_new_with_label (_("Connect to CDDB server"));
	pragha_hig_workarea_table_add_wide_control (table, &row, use_cddb);

	priv->cddb_setting_widget = table;
	priv->use_cddb_w = use_cddb;

	/* Append panes */

	pragha_preferences_append_audio_setting (priv->pragha,
	                                         priv->device_setting_widget, FALSE);
	pragha_preferences_append_services_setting (priv->pragha,
	                                            priv->cddb_setting_widget, FALSE);

	/* Configure handler and settings */
	pragha_preferences_dialog_connect_handler (priv->pragha,
	                                           G_CALLBACK(pragha_cdrom_preferences_dialog_response),
	                                           plugin);

	pragha_cdrom_init_settings (plugin);
}

static void
pragha_cdrom_plugin_remove_setting (PraghaCdromPlugin *plugin)
{
	PraghaCdromPluginPrivate *priv = plugin->priv;

	pragha_preferences_remove_audio_setting (priv->pragha,
	                                         priv->device_setting_widget);
	pragha_preferences_remove_services_setting (priv->pragha,
	                                            priv->cddb_setting_widget);

	pragha_preferences_dialog_disconnect_handler (priv->pragha,
	                                              G_CALLBACK(pragha_cdrom_preferences_dialog_response),
	                                              plugin);
}

/*
 * Cdrom plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	PraghaStatusIcon *status_icon = NULL;
	PraghaMusicEnum *enum_map = NULL;

	PraghaCdromPlugin *plugin = PRAGHA_CDROM_PLUGIN (activatable);
	PraghaCdromPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN,"CDROM plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	/* Attach main menu */

	priv->action_group_main_menu = gtk_action_group_new ("PraghaCdromPlugin");
	gtk_action_group_set_translation_domain (priv->action_group_main_menu, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group_main_menu,
	                              main_menu_actions,
	                              G_N_ELEMENTS (main_menu_actions),
	                              plugin);

	priv->merge_id_main_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                                priv->action_group_main_menu,
	                                                                main_menu_xml);
	status_icon = pragha_application_get_status_icon(priv->pragha);
	priv->merge_id_syst_menu = pragha_systray_append_plugin_action (status_icon,
	                                                                priv->action_group_main_menu,
	                                                                syst_menu_xml);
	g_object_ref (priv->action_group_main_menu);

	/* Connect signals */
	backend = pragha_application_get_backend (priv->pragha);
	g_signal_connect (backend, "set-device",
	                  G_CALLBACK(pragha_cdrom_plugin_set_device), plugin);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_cdrom_plugin_prepare_source), plugin);

	#ifdef HAVE_GUDEV
	pragha_devices_plugin_connect_signals (G_CALLBACK(pragha_cdrom_plugin_device_added),
	                                       G_CALLBACK(pragha_cdrom_plugin_device_removed),
	                                       plugin);
	#endif

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_get (enum_map, "FILE_CDDA");
	g_object_unref (enum_map);

	/* Settings */
	pragha_cdrom_plugin_append_setting (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaBackend *backend;
	PraghaStatusIcon *status_icon = NULL;
	PraghaMusicEnum *enum_map = NULL;

	PraghaCdromPlugin *plugin = PRAGHA_CDROM_PLUGIN (activatable);
	PraghaCdromPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN,"CDROM plugin %s", G_STRFUNC);

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	status_icon = pragha_application_get_status_icon(priv->pragha);
	pragha_systray_remove_plugin_action (status_icon,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_syst_menu);
	priv->merge_id_syst_menu = 0;

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_handlers_disconnect_by_func (backend, pragha_cdrom_plugin_set_device, plugin);
	g_signal_handlers_disconnect_by_func (backend, pragha_cdrom_plugin_prepare_source, plugin);

	#ifdef HAVE_GUDEV
	pragha_devices_plugin_disconnect_signals (G_CALLBACK(pragha_cdrom_plugin_device_added),
	                                          G_CALLBACK(pragha_cdrom_plugin_device_removed),
	                                          plugin);
	#endif

	pragha_cdrom_plugin_remove_setting (plugin);

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_remove (enum_map, "FILE_CDDA");
	g_object_unref (enum_map);

	libcddb_shutdown ();
}