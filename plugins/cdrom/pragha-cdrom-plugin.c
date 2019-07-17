/*************************************************************************/
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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
#include "src/pragha-menubar.h"
#include "src/pragha-musicobject.h"
#include "src/pragha-musicobject-mgmt.h"
#include "src/pragha-plugins-engine.h"
#include "src/pragha-statusicon.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-database-provider.h"
#include "src/pragha-window.h"

#if HAVE_GUDEV
#include "src/pragha-device-client.h"
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

#if HAVE_GUDEV
	PraghaDeviceClient *device_client;
#endif

	guint64             bus_hooked;
	guint64             device_hooked;
	gchar              *disc_id;

	GtkWidget          *device_setting_widget;
	GtkWidget          *audio_cd_device_w;
	GtkWidget          *cddb_setting_widget;
	GtkWidget          *use_cddb_w;

	gchar              *audio_cd_device;
	gboolean            use_cddb;

	GtkActionGroup    *action_group_main_menu;
	guint              merge_id_main_menu;
};
typedef struct _PraghaCdromPluginPrivate PraghaCdromPluginPrivate;

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_CDROM_PLUGIN,
                        PraghaCdromPlugin,
                        pragha_cdrom_plugin)

/*
 * CDROM plugin.
 */
#define KEY_USE_CDDB        "use_cddb"
#define KEY_AUDIO_CD_DEVICE "audio_cd_device"

static gboolean
pragha_preferences_get_use_cddb (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL;
	gboolean use_cddb = FALSE;

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");
	use_cddb = pragha_preferences_get_boolean (preferences,
	                                           plugin_group,
	                                           KEY_USE_CDDB);
	g_free (plugin_group);

	return use_cddb;
}

static void
pragha_preferences_set_use_cddb (PraghaPreferences *preferences,
                                 gboolean           use_cddb)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");
	pragha_preferences_set_boolean (preferences,
	                                plugin_group,
	                                KEY_USE_CDDB,
	                                use_cddb);
	g_free (plugin_group);
}

static gchar *
pragha_preferences_get_audio_cd_device (PraghaPreferences *preferences)
{
	gchar *plugin_group = NULL, *audio_cd_device = NULL;

	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");
	audio_cd_device = pragha_preferences_get_string (preferences,
	                                                 plugin_group,
	                                                 KEY_AUDIO_CD_DEVICE);
	g_free (plugin_group);

	return audio_cd_device;
}

static void
pragha_preferences_set_audio_cd_device (PraghaPreferences *preferences,
                                        const gchar       *device)
{
	gchar *plugin_group = NULL;
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");

	if (string_is_not_empty(device))
		pragha_preferences_set_string (preferences,
		                               plugin_group,
		                               KEY_AUDIO_CD_DEVICE,
		                               device);
	else
		pragha_preferences_remove_key (preferences,
		                               plugin_group,
		                               KEY_AUDIO_CD_DEVICE);
	g_free (plugin_group);
}

static PraghaMusicobject *
new_musicobject_from_cdda (PraghaCdromPlugin *plugin,
                           cdrom_drive_t     *cdda_drive,
                           cddb_disc_t       *cddb_disc,
                           gint               track_no)
{
	PraghaPreferences *preferences;
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicobject *mobj = NULL;
	gint channels, start, end;
	gchar *ntitle = NULL, *nfile = NULL;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Creating new musicobject from cdda: %d", track_no);

	channels = cdio_get_track_channels(cdda_drive->p_cdio, track_no);
	start = cdio_cddap_track_firstsector(cdda_drive, track_no);
	end = cdio_cddap_track_lastsector(cdda_drive, track_no);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     NULL);

	preferences = pragha_application_get_preferences (priv->pragha);
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
	pragha_musicobject_set_source (mobj, pragha_music_enum_map_get(enum_map, "CDROM"));
	g_object_unref (enum_map);

	if (priv->disc_id)
		pragha_musicobject_set_provider (mobj, priv->disc_id);

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
pragha_cdrom_plugin_add_cddb_tracks (cdrom_drive_t *cdda_drive,
                                     cddb_disc_t   *cddb_disc)
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

static GList *
pragha_cdrom_plugin_get_mobj_list (PraghaCdromPlugin *plugin,
                                   cdrom_drive_t     *cdda_drive,
                                   cddb_disc_t       *cddb_disc)
{
	PraghaMusicobject *mobj;
	gint num_tracks = 0, i = 0;
	GList *list = NULL;

	num_tracks = cdio_cddap_tracks(cdda_drive);
	if (!num_tracks)
		return NULL;

	for (i = 1; i <= num_tracks; i++) {
		mobj = new_musicobject_from_cdda (plugin, cdda_drive, cddb_disc, i);
		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);

		pragha_process_gtk_events ();
	}
	return list;
}

static cdrom_drive_t *
pragha_cdrom_plugin_get_drive (PraghaPreferences *preferences)
{
	cdrom_drive_t *drive = NULL;
	gchar **cdda_devices = NULL;
	const gchar *audio_cd_device = NULL;

	audio_cd_device = pragha_preferences_get_audio_cd_device(preferences);

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
	}
	else {
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

static void
pragha_application_append_audio_cd (PraghaCdromPlugin *plugin)
{
	PraghaDatabaseProvider *provider;
	PraghaDatabase *database;
	PraghaPlaylist *playlist;
	PraghaPreferences *preferences;
	PraghaMusicobject *mobj;
	lba_t lba;
	gint matches;
	cdrom_drive_t *cdda_drive = NULL;
	cddb_disc_t *cddb_disc = NULL;
	cddb_conn_t *cddb_conn = NULL;
	const gchar *title_disc = NULL;
	guint discid = 0;
	GList *list = NULL, *l;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	preferences = pragha_application_get_preferences (priv->pragha);

	cdda_drive = pragha_cdrom_plugin_get_drive (preferences);
	if (!cdda_drive)
		return;

	if (cdio_cddap_open(cdda_drive)) {
		g_warning("Unable to open Audio CD");
		return;
	}

	if (pragha_preferences_get_use_cddb (preferences)) {
		cddb_conn = cddb_new ();
		if (!cddb_conn)
			goto add;

		cddb_disc = cddb_disc_new();
		if (!cddb_disc)
			goto add;

		lba = cdio_get_track_lba(cdda_drive->p_cdio, CDIO_CDROM_LEADOUT_TRACK);
		if (lba == CDIO_INVALID_LBA)
			goto add;

		cddb_disc_set_length(cddb_disc, FRAMES_TO_SECONDS(lba));
		if (pragha_cdrom_plugin_add_cddb_tracks(cdda_drive, cddb_disc) < 0)
			goto add;

		if (!cddb_disc_calc_discid(cddb_disc))
			goto add;

		discid = cddb_disc_get_discid (cddb_disc);
		if (discid)
			priv->disc_id = g_strdup_printf ("Discid://%x", discid);

		cddb_disc_set_category(cddb_disc, CDDB_CAT_MISC);

		matches = cddb_query(cddb_conn, cddb_disc);
		if (matches == -1)
			goto add;

		if (!cddb_read(cddb_conn, cddb_disc)) {
			cddb_error_print(cddb_errno(cddb_conn));
			goto add;
		}

		CDEBUG(DBG_PLUGIN, "Successfully initialized CDDB");

		goto add;
	}

add:
	list = pragha_cdrom_plugin_get_mobj_list (plugin, cdda_drive, cddb_disc);
	if (list) {
		playlist = pragha_application_get_playlist (priv->pragha);
		pragha_playlist_append_mobj_list (playlist, list);

		if (priv->disc_id) {
			title_disc = cddb_disc_get_title (cddb_disc);

			provider = pragha_database_provider_get ();
			pragha_provider_add_new (provider,
			                         priv->disc_id,
			                         "CDROM",
			                         title_disc ? title_disc : _("Audio CD"),
			                         "media-optical");
			pragha_provider_set_visible (provider, priv->disc_id, TRUE);

			database = pragha_application_get_database (priv->pragha);
			for (l = list; l != NULL; l = l->next) {
				mobj = l->data;
				pragha_database_add_new_musicobject (database, mobj);
			}
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}
		g_list_free (list);
	}

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
	file_source = pragha_music_enum_map_get(enum_map, "CDROM");
	g_object_unref (enum_map);

	return (file_source == pragha_musicobject_get_source (mobj));
}

static void
pragha_cdrom_plugin_set_device (PraghaBackend *backend, GObject *obj, gpointer user_data)
{
	PraghaPreferences *preferences;
	PraghaMusicobject *mobj = NULL;
	const gchar *audio_cd_device;
	GObject *source;

	PraghaCdromPlugin *plugin = user_data;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_cdda_type (mobj))
		return;

	g_object_get (obj, "source", &source, NULL);
	if (source) {
		preferences = pragha_application_get_preferences (priv->pragha);
		audio_cd_device = pragha_preferences_get_audio_cd_device (preferences);
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

	switch (response) {
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_application_append_audio_cd (plugin);
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
	PraghaCdromPluginPrivate *priv = plugin->priv;

	if (device_type != PRAGHA_DEVICE_AUDIO_CD)
		return;

	if (priv->bus_hooked || priv->device_hooked)
		return;

	priv->bus_hooked = g_udev_device_get_property_as_uint64 (u_device, "BUSNUM");
	priv->device_hooked = g_udev_device_get_property_as_uint64 (u_device, "DEVNUM");

	dialog = pragha_gudev_dialog_new (pragha_application_get_window (priv->pragha),
	                                 _("Audio/Data CD"), "media-optical",
	                                 _("An audio CD was inserted"), NULL,
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
	PraghaDatabaseProvider *provider;
	PraghaMusicEnum *enum_map = NULL;
	guint64 busnum = 0;
	guint64 devnum = 0;

	PraghaCdromPlugin *plugin = user_data;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	if (device_type != PRAGHA_DEVICE_AUDIO_CD)
		return;

	busnum = g_udev_device_get_property_as_uint64(u_device, "BUSNUM");
	devnum = g_udev_device_get_property_as_uint64(u_device, "DEVNUM");

	if (busnum == priv->bus_hooked && devnum == priv->device_hooked) {
		if (priv->disc_id) {
			provider = pragha_database_provider_get ();
			pragha_provider_remove (provider, priv->disc_id);
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}

		priv->bus_hooked = 0;
		priv->device_hooked = 0;

		if (priv->disc_id) {
			g_free (priv->disc_id);
			priv->disc_id = NULL;
		}

		enum_map = pragha_music_enum_get ();
		pragha_music_enum_map_remove (enum_map, "CDROM");
		g_object_unref (enum_map);
	}
}
#endif

/*
 * Menubar
 */
static void
pragha_cdrom_plugin_append_action (GtkAction *action, PraghaCdromPlugin *plugin)
{
	pragha_application_append_audio_cd (plugin);
}

static void
pragha_gmenu_add_cdrom_action (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
	pragha_cdrom_plugin_append_action (NULL, PRAGHA_CDROM_PLUGIN(user_data));
}

static const GtkActionEntry main_menu_actions [] = {
	{"Add Audio CD", NULL, N_("Add Audio _CD"),
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

	preferences = pragha_preferences_get();
	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		pragha_gtk_entry_set_text(GTK_ENTRY(priv->audio_cd_device_w),
			priv->audio_cd_device);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_cddb_w),
			priv->use_cddb);
		break;
	case GTK_RESPONSE_OK:
		audio_cd_device = gtk_entry_get_text (GTK_ENTRY(priv->audio_cd_device_w));
		if (audio_cd_device) {
			pragha_preferences_set_audio_cd_device (preferences, audio_cd_device);

			g_free (priv->audio_cd_device);
			priv->audio_cd_device = g_strdup(audio_cd_device);
		}
		priv->use_cddb =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->use_cddb_w));
		pragha_preferences_set_use_cddb (preferences, priv->use_cddb);
		break;
	default:
		break;
	}
	g_object_unref (preferences);
}

static void
pragha_cdrom_init_settings (PraghaCdromPlugin *plugin)
{
	PraghaPreferences *preferences;
	gchar *plugin_group = NULL;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	preferences = pragha_preferences_get();
	plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");
	if (pragha_preferences_has_group (preferences, plugin_group)) {
		priv->audio_cd_device =
			pragha_preferences_get_audio_cd_device (preferences);
		priv->use_cddb =
			pragha_preferences_get_use_cddb(preferences);
	}
	else {
		priv->audio_cd_device = NULL;
		priv->use_cddb = TRUE;
	}

	pragha_gtk_entry_set_text(GTK_ENTRY(priv->audio_cd_device_w), priv->audio_cd_device);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_cddb_w), priv->use_cddb);

	g_object_unref (preferences);
	g_free (plugin_group);
}

static void
pragha_cdrom_plugin_append_setting (PraghaCdromPlugin *plugin)
{
	PreferencesDialog *dialog;
	GtkWidget *table;
	GtkWidget *audio_cd_device_label,*audio_cd_device_entry, *use_cddb;
	guint row = 0;

	PraghaCdromPluginPrivate *priv = plugin->priv;

	/* Cd Device */

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("Audio CD"));

	audio_cd_device_label = gtk_label_new(_("Audio CD Device"));
	gtk_widget_set_halign (GTK_WIDGET(audio_cd_device_label), GTK_ALIGN_START);
	gtk_widget_set_valign (GTK_WIDGET(audio_cd_device_label), GTK_ALIGN_START);

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

	dialog = pragha_application_get_preferences_dialog (priv->pragha);
	pragha_preferences_append_audio_setting (dialog,
	                                         priv->device_setting_widget, FALSE);
	pragha_preferences_append_services_setting (dialog,
	                                            priv->cddb_setting_widget, FALSE);

	/* Configure handler and settings */
	pragha_preferences_dialog_connect_handler (dialog,
	                                           G_CALLBACK(pragha_cdrom_preferences_dialog_response),
	                                           plugin);

	pragha_cdrom_init_settings (plugin);
}

static void
pragha_cdrom_plugin_remove_setting (PraghaCdromPlugin *plugin)
{
	PreferencesDialog *dialog;
	PraghaCdromPluginPrivate *priv = plugin->priv;

	dialog = pragha_application_get_preferences_dialog (priv->pragha);

	pragha_preferences_dialog_disconnect_handler (dialog,
	                                              G_CALLBACK(pragha_cdrom_preferences_dialog_response),
	                                              plugin);

	pragha_preferences_remove_audio_setting (dialog,
	                                         priv->device_setting_widget);
	pragha_preferences_remove_services_setting (dialog,
	                                            priv->cddb_setting_widget);
}

/*
 * Cdrom plugin
 */
static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	GMenuItem *item;
	GSimpleAction *action;
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

	/* Systray and gear Menu*/

	action = g_simple_action_new ("add-cdrom", NULL);
	g_signal_connect (G_OBJECT (action), "activate",
	                  G_CALLBACK (pragha_gmenu_add_cdrom_action), plugin);

	item = g_menu_item_new (_("Add Audio _CD"), "syst.add-cdrom");
	status_icon = pragha_application_get_status_icon(priv->pragha);
	pragha_systray_append_action (status_icon, "pragha-systray-append-music", action, item);
	g_object_unref (item);

	item = g_menu_item_new (_("Add Audio _CD"), "win.add-cdrom");
	pragha_menubar_append_action (priv->pragha, "pragha-plugins-append-music", action, item);
	g_object_unref (item);

	/* Connect signals */

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_connect (backend, "set-device",
	                  G_CALLBACK(pragha_cdrom_plugin_set_device), plugin);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_cdrom_plugin_prepare_source), plugin);

#ifdef HAVE_GUDEV
	priv->device_client = pragha_device_client_get();

	g_signal_connect (G_OBJECT(priv->device_client), "device-added",
	                  G_CALLBACK(pragha_cdrom_plugin_device_added), plugin);
	g_signal_connect (G_OBJECT(priv->device_client), "device-removed",
	                  G_CALLBACK(pragha_cdrom_plugin_device_removed), plugin);
#endif

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_get (enum_map, "CDROM");
	g_object_unref (enum_map);

	/* Settings */
	pragha_cdrom_plugin_append_setting (plugin);
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDatabaseProvider *provider;
	PraghaBackend *backend;
	PraghaPreferences *preferences;
	PraghaStatusIcon *status_icon = NULL;
	PraghaMusicEnum *enum_map = NULL;
	gchar *plugin_group = NULL;

	PraghaCdromPlugin *plugin = PRAGHA_CDROM_PLUGIN (activatable);
	PraghaCdromPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN,"CDROM plugin %s", G_STRFUNC);

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_main_menu,
	                                     priv->merge_id_main_menu);
	priv->merge_id_main_menu = 0;

	status_icon = pragha_application_get_status_icon(priv->pragha);
	pragha_systray_remove_action (status_icon, "pragha-systray-append-music", "add-cdrom");

	pragha_menubar_remove_action (priv->pragha, "pragha-plugins-append-music", "add-cdrom");

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_handlers_disconnect_by_func (backend, pragha_cdrom_plugin_set_device, plugin);
	g_signal_handlers_disconnect_by_func (backend, pragha_cdrom_plugin_prepare_source, plugin);

#ifdef HAVE_GUDEV
	g_signal_handlers_disconnect_by_func (priv->device_client,
	                                      pragha_cdrom_plugin_device_added,
	                                      plugin);
	g_signal_handlers_disconnect_by_func (priv->device_client,
	                                      pragha_cdrom_plugin_device_removed,
	                                      plugin);
	g_object_unref (priv->device_client);
#endif

	/* Remove from database */

	if (priv->disc_id) {
		provider = pragha_database_provider_get ();
		pragha_provider_remove (provider, priv->disc_id);
		g_object_unref (provider);
	}

	/* Crop library to not save from playlist */

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_remove (enum_map, "CDROM");
	g_object_unref (enum_map);

	/* If plugin is disables by user remove the rest of preferences */

	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha)))
	{
		/* Remove setting widgets */

		pragha_cdrom_plugin_remove_setting (plugin);

		/* Remove settings */

		preferences = pragha_application_get_preferences (priv->pragha);
		plugin_group = pragha_preferences_get_plugin_group_name (preferences, "cdrom");
		pragha_preferences_remove_group (preferences, plugin_group);
		g_free (plugin_group);

		/* Force update library view */

		if (priv->disc_id) {
			provider = pragha_database_provider_get ();
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}
	}

	/* Free and shutdown */

	if (priv->disc_id)
		g_free (priv->disc_id);

	libcddb_shutdown ();
}
