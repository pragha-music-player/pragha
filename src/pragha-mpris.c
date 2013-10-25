/*************************************************************************/
/* Copyright (C) 2011-2013 matias <mati86dl@gmail.com>                   */
/* Copyright (C) 2011      hakan  <smultimeter@gmail.com>                */
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

#include "pragha-mpris.h"
#include "pragha-playback.h"
#include "pragha-menubar.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha.h"

struct _PraghaMpris2 {
	PraghaApplication *pragha;
	guint owner_id;
	GDBusNodeInfo *introspection_data;
	GDBusConnection *dbus_connection;
	GQuark interface_quarks[4];
	gboolean saved_playbackstatus;
	gboolean saved_shuffle;
	gchar *saved_title;
	gdouble volume;
	PraghaBackendState state;
};

static const gchar mpris2xml[] =
"<node>"
"        <interface name='org.mpris.MediaPlayer2'>"
"                <method name='Raise'/>"
"                <method name='Quit'/>"
"                <property name='CanQuit' type='b' access='read'/>"
"                <property name='CanRaise' type='b' access='read'/>"
"                <property name='HasTrackList' type='b' access='read'/>"
"                <property name='Identity' type='s' access='read'/>"
"                <property name='DesktopEntry' type='s' access='read'/>"
"                <property name='SupportedUriSchemes' type='as' access='read'/>"
"                <property name='SupportedMimeTypes' type='as' access='read'/>"
"        </interface>"
"        <interface name='org.mpris.MediaPlayer2.Player'>"
"                <method name='Next'/>"
"                <method name='Previous'/>"
"                <method name='Pause'/>"
"                <method name='PlayPause'/>"
"                <method name='Stop'/>"
"                <method name='Play'/>"
"                <method name='Seek'>"
"				 		<arg direction='in' name='Offset' type='x'/>"
"				 </method>"
"                <method name='SetPosition'>"
"						<arg direction='in' name='TrackId' type='o'/>"
"						<arg direction='in' name='Position' type='x'/>"
"                </method>"
"                <method name='OpenUri'>"
"				 		<arg direction='in' name='Uri' type='s'/>"
"				 </method>"
"                <signal name='Seeked'><arg name='Position' type='x'/></signal>"
"                <property name='PlaybackStatus' type='s' access='read'/>"
"                <property name='LoopStatus' type='s' access='readwrite'/>"
"                <property name='Rate' type='d' access='readwrite'/>"
"                <property name='Shuffle' type='b' access='readwrite'/>"
"                <property name='Metadata' type='a{sv}' access='read'/>"
"                <property name='Volume' type='d' access='readwrite'/>"
"                <property name='Position' type='x' access='read'/>"
"                <property name='MinimumRate' type='d' access='read'/>"
"                <property name='MaximumRate' type='d' access='read'/>"
"                <property name='CanGoNext' type='b' access='read'/>"
"                <property name='CanGoPrevious' type='b' access='read'/>"
"                <property name='CanPlay' type='b' access='read'/>"
"                <property name='CanPause' type='b' access='read'/>"
"                <property name='CanSeek' type='b' access='read'/>"
"                <property name='CanControl' type='b' access='read'/>"
"        </interface>"
"        <interface name='org.mpris.MediaPlayer2.Playlists'>"
"                <method name='ActivatePlaylist'>"
"						<arg direction='in' name='PlaylistId' type='o'/>"
"				</method>"
"                <method name='GetPlaylists'>"
"						<arg direction='in' name='Index' type='u'/>"
"						<arg direction='in' name='MaxCount' type='u'/>"
"						<arg direction='in' name='Order' type='s'/>"
"						<arg direction='in' name='ReverseOrder' type='b'/>"
"						<arg direction='out' name='Playlists' type='a(oss)'/>"
"                </method>"
"                <property name='PlaylistCount' type='u' access='read'/>"
"                <property name='Orderings' type='as' access='read'/>"
"                <property name='ActivePlaylist' type='(b(oss))' access='read'/>"
"        </interface>"
"        <interface name='org.mpris.MediaPlayer2.TrackList'>"
"                <method name='GetTracksMetadata'>"
"                        <arg direction='in' name='TrackIds' type='ao'/>"
"                        <arg direction='out' name='Metadata' type='aa{sv}'/>"
"                </method>"
"                <method name='AddTrack'>"
"                        <arg direction='in' name='Uri' type='s'/>"
"                        <arg direction='in' name='AfterTrack' type='o'/>"
"                        <arg direction='in' name='SetAsCurrent' type='b'/>"
"                </method>"
"                <method name='RemoveTrack'>"
"                        <arg direction='in' name='TrackId' type='o'/>"
"                </method>"
"                <method name='GoTo'>"
"                        <arg direction='in' name='TrackId' type='o'/>"
"                </method>"
"                <signal name='TrackListReplaced'>"
"                        <arg name='Tracks' type='ao'/>"
"                        <arg name='CurrentTrack' type='o'/>"
"                </signal>"
"                <signal name='TrackAdded'>"
"                        <arg name='Metadata' type='a{sv}'/>"
"                        <arg name='AfterTrack' type='o'/>"
"                </signal>"
"                <signal name='TrackRemoved'>"
"                        <arg name='TrackId' type='o'/>"
"                </signal>"
"                <signal name='TrackMetadataChanged'>"
"                        <arg name='TrackId' type='o'/>"
"                        <arg name='Metadata' type='a{sv}'/>"
"                </signal>"
"                <property name='Tracks' type='ao' access='read'/>"
"                <property name='CanEditTracks' type='b' access='read'/>"
"        </interface>"
"</node>";

/* some MFCisms */
#define BEGIN_INTERFACE(x) \
	if(g_quark_try_string(interface_name)==mpris2->interface_quarks[x]) {
#define MAP_METHOD(x,y) \
	if(!g_strcmp0(#y, method_name)) { \
		mpris_##x##_##y(invocation, parameters, mpris2); return; }
#define PROPGET(x,y) \
	if(!g_strcmp0(#y, property_name)) \
		return mpris_##x##_get_##y(error, mpris2);
#define PROPPUT(x,y) \
	if(g_quark_try_string(property_name)==g_quark_from_static_string(#y)) \
		mpris_##x##_put_##y(value, error, mpris2);
#define END_INTERFACE }

static PraghaMusicobject *
get_mobj_at_mpris2_track_id(PraghaApplication *pragha, const gchar *track_id)
{
	gchar *base = NULL;
	void *mobj_request = NULL;

	base = g_strdup_printf("%s/TrackList/", MPRIS_PATH);

	if(g_str_has_prefix(track_id, base))
		sscanf(track_id + strlen(base), "%p", &mobj_request);

	g_free(base);

	return mobj_request;
}

/* org.mpris.MediaPlayer2 */
static void mpris_Root_Raise (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	gtk_window_present(GTK_WINDOW(pragha_application_get_window(pragha)));

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Root_Quit (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	pragha_application_quit (mpris2->pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static GVariant* mpris_Root_get_CanQuit (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_CanRaise (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_HasTrackList (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_Identity (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_string("Pragha Music Player");
}

static GVariant* mpris_Root_get_DesktopEntry (GError **error, PraghaMpris2 *mpris2)
{
	GVariant* ret_val = g_variant_new_string("pragha");
	return ret_val;
}

static GVariant* mpris_Root_get_SupportedUriSchemes (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_parse(G_VARIANT_TYPE("as"),
		"['file', 'cdda']", NULL, NULL, NULL);
}

static GVariant* mpris_Root_get_SupportedMimeTypes (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_parse(G_VARIANT_TYPE("as"),
		"['audio/x-mp3', 'audio/mpeg', 'audio/x-mpeg', 'audio/mpeg3', "
		"'audio/mp3', 'application/ogg', 'application/x-ogg', 'audio/vorbis', "
		"'audio/x-vorbis', 'audio/ogg', 'audio/x-ogg', 'audio/x-flac', "
		#if defined(TAGLIB_WITH_ASF) && (TAGLIB_WITH_ASF==1)
		"'video/x-ms-asf', 'audio/x-ms-wma', "
		#endif
		#if defined(TAGLIB_WITH_MP4) && (TAGLIB_WITH_MP4==1)
		"'audio/x-m4a', "
		#endif
		#ifdef HAVE_TAGLIB_1_7
		"'application/x-ape', 'audio/ape', 'audio/x-ape', "
		#endif
		"'application/x-flac', 'audio/flac', 'audio/x-wav']", NULL, NULL, NULL);
}

/* org.mpris.MediaPlayer2.Player */
static void mpris_Player_Play (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_play_pause_resume(pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Next (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_next_track(pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Previous (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_prev_track(pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Pause (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_backend_pause (backend);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_PlayPause (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_play_pause_resume(pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Stop (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);
	if (pragha_backend_emitted_error (backend) == FALSE)
		pragha_playback_stop(pragha);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Seek (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;

	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);

	if(pragha_backend_get_state (backend) == ST_STOPPED) {
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Nothing to seek");
		return;
	}

	gint64 param;
	g_variant_get(parameters, "(x)", &param);

	gint64 curr_pos = pragha_backend_get_current_position (backend) / GST_USECOND;
	gint64 seek = (curr_pos + param) / GST_MSECOND;

	seek = CLAMP (seek, 0, pragha_musicobject_get_length (pragha_backend_get_musicobject (backend)));

	pragha_backend_seek (backend, seek);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_SetPosition (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;
	PraghaMusicobject *current_mobj = NULL;
	gint64 param;
	PraghaMusicobject *mobj = NULL;
	gchar *track_id = NULL;

	PraghaApplication *pragha = mpris2->pragha;

	g_variant_get(parameters, "(ox)", &track_id, &param);
	mobj = get_mobj_at_mpris2_track_id(pragha, track_id);
	g_free(track_id);

	/* FIXME: Ugly hack... */
	backend = pragha_application_get_backend (pragha);
	current_mobj = pragha_backend_get_musicobject (backend);

	if(mobj != NULL && mobj == current_mobj) {
		gint seek = (param / 1000000);
		if (seek >= pragha_musicobject_get_length(current_mobj))
			seek = pragha_musicobject_get_length(current_mobj);

		pragha_backend_seek(backend, seek);

	}

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void
seeked_cb (PraghaBackend *backend, gpointer user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	PraghaApplication *pragha = mpris2->pragha;

	if(NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS emit seeked signal..");

	backend = pragha_application_get_backend (pragha);

	gint64 position = pragha_backend_get_current_position (backend);

	g_dbus_connection_emit_signal(mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.Player", "Seeked",
		 g_variant_new ("(x)", GST_TIME_AS_USECONDS (position)), NULL);
}

static void mpris_Player_OpenUri (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaPlaylist *playlist;
	gchar *uri = NULL, *path = NULL;
	PraghaMusicobject *mobj = NULL;
	gboolean happened = FALSE;

	PraghaApplication *pragha = mpris2->pragha;

	g_variant_get(parameters, "(s)", &uri);

	CDEBUG(DBG_MPRIS, "MPRIS Player OpenUri");

	if(uri) {
		// TODO: Translate "cdda://sr0/Track 01.wav" URIs for new_musicobject_from_cdda()
		//       If there is such a convention on other players
		path = g_filename_from_uri(uri, NULL, NULL);
		if(path && is_playable_file(path)) {
			mobj = new_musicobject_from_file(path);
			if(mobj) {
				playlist = pragha_application_get_playlist (pragha);

				pragha_playlist_append_mobj_and_play(playlist, mobj);
				happened = TRUE;
			}
		}
		g_free(uri);
		g_free(path);
	}

	if(happened)
		g_dbus_method_invocation_return_value (invocation, NULL);
	else
		g_dbus_method_invocation_return_error_literal (invocation,
							       G_DBUS_ERROR,
							       G_DBUS_ERROR_INVALID_FILE_CONTENT,
							       "This file does not play here.");
}

static GVariant* mpris_Player_get_PlaybackStatus (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;
	PraghaBackend *backend = pragha_application_get_backend (pragha);

	switch (pragha_backend_get_state (backend)) {
		case ST_PLAYING:
			return g_variant_new_string("Playing");
		case ST_PAUSED:
			return g_variant_new_string("Paused");
		default:
			return g_variant_new_string("Stopped");
	}
}

static GVariant* mpris_Player_get_LoopStatus (GError **error, PraghaMpris2 *mpris2)
{
	PraghaPreferences *preferences;
	gboolean repeat;

	PraghaApplication *pragha = mpris2->pragha;

	preferences = pragha_application_get_preferences (pragha);
	repeat = pragha_preferences_get_repeat (preferences);

	return g_variant_new_string(repeat ? "Playlist" : "None");
}

static void mpris_Player_put_LoopStatus (GVariant *value, GError **error, PraghaMpris2 *mpris2)
{
	PraghaPreferences *preferences;

	PraghaApplication *pragha = mpris2->pragha;

	const gchar *new_loop = g_variant_get_string(value, NULL);

	gboolean repeat = g_strcmp0("Playlist", new_loop) ? FALSE : TRUE;

	preferences = pragha_application_get_preferences (pragha);
	pragha_preferences_set_repeat (preferences, repeat);
}

static GVariant* mpris_Player_get_Rate (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_double(1.0);
}

static void mpris_Player_put_Rate (GVariant *value, GError **error, PraghaMpris2 *mpris2)
{
	g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "This is not alsaplayer.");
}

static GVariant* mpris_Player_get_Shuffle (GError **error, PraghaMpris2 *mpris2)
{
	PraghaPreferences *preferences;
	gboolean shuffle;

	PraghaApplication *pragha = mpris2->pragha;

	preferences = pragha_application_get_preferences (pragha);
	shuffle = pragha_preferences_get_shuffle (preferences);

	return g_variant_new_boolean(shuffle);
}

static void mpris_Player_put_Shuffle (GVariant *value, GError **error, PraghaMpris2 *mpris2)
{
	PraghaPreferences *preferences;
	gboolean shuffle = g_variant_get_boolean(value);

	PraghaApplication *pragha = mpris2->pragha;

	preferences = pragha_application_get_preferences (pragha);
	pragha_preferences_set_shuffle (preferences, shuffle);
}

static GVariant * handle_get_trackid(PraghaMusicobject *mobj) {
	gchar *o = alloca(260);
	if(NULL == mobj)
		return g_variant_new_object_path("/");
	g_snprintf(o, 260, "%s/TrackList/%p", MPRIS_PATH, mobj);
	return g_variant_new_object_path(o);
}

void handle_strings_request(GVariantBuilder *b, const gchar *tag, const gchar *val)
{
	GVariant *vval = g_variant_new_string(val);
	GVariant *vvals = g_variant_new_array(G_VARIANT_TYPE_STRING, &vval, 1);

	g_variant_builder_add (b, "{sv}", tag, vvals);
}

static void handle_get_metadata(PraghaMusicobject *mobj, GVariantBuilder *b)
{
	const gchar *title, *artist, *album, *genre, *comment, *file;
	gint track_no, year, length, bitrate, channels, samplerate;
	gchar *date = NULL, *url = NULL;

	CDEBUG(DBG_MPRIS, "MPRIS handle get metadata");

	file = pragha_musicobject_get_file(mobj);
	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);
	album = pragha_musicobject_get_album(mobj);
	genre = pragha_musicobject_get_genre(mobj);
	track_no = pragha_musicobject_get_track_no(mobj);
	year = pragha_musicobject_get_year(mobj);
	comment = pragha_musicobject_get_comment(mobj);
	length = pragha_musicobject_get_length(mobj);
	bitrate = pragha_musicobject_get_bitrate(mobj);
	channels = pragha_musicobject_get_channels(mobj);
	samplerate = pragha_musicobject_get_samplerate(mobj);

	date = g_strdup_printf("%d", year);

	url = (pragha_musicobject_is_local_file(mobj)) ?
	       g_filename_to_uri(file, NULL, NULL) : g_strdup(file);

	g_variant_builder_add (b, "{sv}", "mpris:trackid",
		handle_get_trackid(mobj));
	g_variant_builder_add (b, "{sv}", "xesam:url",
		g_variant_new_string(url));
	g_variant_builder_add (b, "{sv}", "xesam:title",
		g_variant_new_string(title));
	handle_strings_request(b, "xesam:artist", artist);
	g_variant_builder_add (b, "{sv}", "xesam:album",
		g_variant_new_string(album));
	handle_strings_request(b, "xesam:genre", genre);
	g_variant_builder_add (b, "{sv}", "xesam:contentCreated",
		g_variant_new_string(date));
	g_variant_builder_add (b, "{sv}", "xesam:trackNumber",
		g_variant_new_int32(track_no));
	handle_strings_request(b, "xesam:comment", comment);
	g_variant_builder_add (b, "{sv}", "mpris:length",
		g_variant_new_int64((gint64)length * 1000000));
	g_variant_builder_add (b, "{sv}", "audio-bitrate",
		g_variant_new_int32(bitrate));
	g_variant_builder_add (b, "{sv}", "audio-channels",
		g_variant_new_int32(channels));
	g_variant_builder_add (b, "{sv}", "audio-samplerate",
		g_variant_new_int32(samplerate));

	g_free(date);
	g_free(url);
}

static GVariant* mpris_Player_get_Metadata (GError **error, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;
	PraghaToolbar *toolbar;
	PraghaAlbumArt *albumart;
	gchar *artUrl_uri = NULL;
	GVariantBuilder b;
	const gchar *arturl;

	PraghaApplication *pragha = mpris2->pragha;

	CDEBUG(DBG_MPRIS, "MPRIS Player get Metadata");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("a{sv}"));

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		handle_get_metadata(pragha_backend_get_musicobject(backend), &b);

		toolbar = pragha_application_get_toolbar (pragha);
		albumart = pragha_toolbar_get_album_art (toolbar);

		arturl = pragha_album_art_get_path(albumart);
		if (string_is_not_empty(arturl)) {
			artUrl_uri = g_filename_to_uri(arturl, NULL, NULL);
			g_variant_builder_add (&b, "{sv}", "mpris:artUrl",
				g_variant_new_string(artUrl_uri));
			g_free(artUrl_uri);
		}
	}
	else {
		g_variant_builder_add (&b, "{sv}", "mpris:trackid",
			handle_get_trackid(NULL));
	}
	return g_variant_builder_end(&b);
}

static GVariant* mpris_Player_get_Volume (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	return g_variant_new_double(pragha_backend_get_volume (backend));
}

static void mpris_Player_put_Volume (GVariant *value, GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	gdouble volume = g_variant_get_double(value);
	pragha_backend_set_volume (backend, volume);
}

static GVariant* mpris_Player_get_Position (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) == ST_STOPPED)
		return g_variant_new_int64(0);
	else
		return g_variant_new_int64(pragha_backend_get_current_position(backend) / 1000);
}

static GVariant* mpris_Player_get_MinimumRate (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_double(1.0);
}

static GVariant* mpris_Player_get_MaximumRate (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_double(1.0);
}

static GVariant* mpris_Player_get_CanGoNext (GError **error, PraghaMpris2 *mpris2)
{
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Player_get_CanGoPrevious (GError **error, PraghaMpris2 *mpris2)
{
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Player_get_CanPlay (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	return g_variant_new_boolean(pragha_backend_get_state (backend) == ST_PAUSED);
}

static GVariant* mpris_Player_get_CanPause (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	return g_variant_new_boolean(pragha_backend_get_state (backend) == ST_PLAYING);
}

static GVariant* mpris_Player_get_CanSeek (GError **error, PraghaMpris2 *mpris2)
{
	PraghaApplication *pragha = mpris2->pragha;

	PraghaBackend *backend = pragha_application_get_backend (pragha);

	return g_variant_new_boolean (pragha_backend_can_seek (backend));
}

static GVariant* mpris_Player_get_CanControl (GError **error, PraghaMpris2 *mpris2)
{
	// always?
	return g_variant_new_boolean(TRUE);
}

/* org.mpris.MediaPlayer2.Playlists */
static void mpris_Playlists_ActivatePlaylist (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;
	PraghaDatabase *cdbase;
	PraghaPlaylist *playlist;
	gchar *get_playlist = NULL, *test_playlist = NULL, *found_playlist = NULL;
	gchar **db_playlists = NULL;
	gint i = 0;

	PraghaApplication *pragha = mpris2->pragha;

	CDEBUG(DBG_MPRIS, "MPRIS Playlists ActivatePlaylist");

	g_variant_get(parameters, "(o)", &get_playlist);

	if(get_playlist && g_str_has_prefix(get_playlist, MPRIS_PATH)) {
		cdbase = pragha_application_get_database (pragha);
		db_playlists = pragha_database_get_playlist_names (cdbase);
		if(db_playlists) {
			while(db_playlists[i]) {
				test_playlist = g_strdup_printf("%s/Playlists/%d", MPRIS_PATH, i);
				if(0 == g_strcmp0(test_playlist, get_playlist))
					found_playlist = g_strdup(db_playlists[i]);
				g_free(test_playlist);
				i++;
			}
			g_strfreev(db_playlists);
		}
	}

	if(found_playlist) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_remove_all (playlist);

		add_playlist_current_playlist(found_playlist, pragha);

		backend = pragha_application_get_backend (pragha);
		if(pragha_backend_get_state (backend) != ST_STOPPED)
			pragha_playback_next_track(pragha);
		else
			pragha_playback_play_pause_resume(pragha);

		g_free(found_playlist);

		g_dbus_method_invocation_return_value (invocation, NULL);
	}
	else {
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Unknown or malformed playlist object path.");
	}

	g_free (get_playlist);
}

static void mpris_Playlists_GetPlaylists (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaDatabase *cdbase;
	GVariantBuilder builder;
	guint i = 0, start, max;
	const gchar *order;
	gchar ** lists = NULL;
	gchar *listpath = NULL;
	gboolean reverse;
	gint imax = 0;

	PraghaApplication *pragha = mpris2->pragha;

	CDEBUG(DBG_MPRIS, "MPRIS Playlists GetPlaylists");

	g_variant_builder_init(&builder, G_VARIANT_TYPE("(a(oss))"));
	g_variant_builder_open(&builder, G_VARIANT_TYPE("a(oss)"));

	cdbase = pragha_application_get_database (pragha);
	lists = pragha_database_get_playlist_names (cdbase);

	if (lists) {
		g_variant_get (parameters, "(uu&sb)", &start, &max, &order, &reverse);
		imax = max;
		while(lists[i]) {
			if(i >= start && imax > 0) {
				listpath = g_strdup_printf("%s/Playlists/%d", MPRIS_PATH, i);
				g_variant_builder_add (&builder, "(oss)", listpath, lists[i], "");

				g_free(listpath);
				imax--;
			}
			i++;
		}
		g_strfreev(lists);
	}
	g_variant_builder_close(&builder);

	g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (&builder));
}

static GVariant* mpris_Playlists_get_ActivePlaylist (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new("(b(oss))",
		FALSE, "/", _("Playlists"), _("Playlists"));

	/* Formally this is correct, but in the practice only is used to
	   display a confuse message "invalid" in the ubuntu-soundmenu.
	return g_variant_new("(b(oss))",
		FALSE, "/", "invalid", "invalid");*/
}

static GVariant* mpris_Playlists_get_Orderings (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_parse(G_VARIANT_TYPE("as"),
		"['UserDefined']", NULL, NULL, NULL);
}

static GVariant* mpris_Playlists_get_PlaylistCount (GError **error, PraghaMpris2 *mpris2)
{
	PraghaDatabase *cdbase;

	PraghaApplication *pragha = mpris2->pragha;

	cdbase = pragha_application_get_database (pragha);

	return g_variant_new_uint32 (pragha_database_get_playlist_count (cdbase));
}

/* org.mpris.MediaPlayer2.TrackList */
static void mpris_TrackList_GetTracksMetadata (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	/* In: (ao) out: aa{sv} */

	GVariant *param1 = g_variant_get_child_value(parameters, 0);
	gsize i, length;
	GVariantBuilder b;
	const gchar *track_id;

	PraghaApplication *pragha = mpris2->pragha;

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist GetTracksMetada");

	g_variant_builder_init(&b, G_VARIANT_TYPE("(aa{sv})"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("aa{sv}"));

	length = g_variant_n_children(param1);
	
	for(i = 0; i < length; i++) {
		g_variant_builder_open(&b, G_VARIANT_TYPE("a{sv}"));
		PraghaMusicobject *mobj= NULL;
		track_id = g_variant_get_string(g_variant_get_child_value(param1, i), NULL);
		mobj = get_mobj_at_mpris2_track_id(pragha, track_id);
		if (mobj) {
			handle_get_metadata(mobj, &b);
		} else {
			g_variant_builder_add (&b, "{sv}", "mpris:trackid",
			g_variant_new_object_path(track_id));
		}
		g_variant_builder_close(&b);
	}
	g_variant_builder_close(&b);

	g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (&b));
}

static void mpris_TrackList_AddTrack (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaPlaylist *playlist;
	gchar *uri;
	gchar *after_track; //TODO use this
	gboolean set_as_current; //TODO use this
	GList *mlist = NULL;

	PraghaApplication *pragha = mpris2->pragha;

	g_variant_get(parameters, "(sob)", &uri, &after_track, &set_as_current);

	gchar *file = g_filename_from_uri(uri, NULL, NULL);

	if (!file) {
		g_warning("Invalid uri: %s", uri);
		goto exit;
	}

	mlist = append_mobj_list_from_unknown_filename(mlist, file);
	if (mlist) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list(playlist, mlist);
		g_list_free (mlist);
	}

	g_free(file);
exit:
	g_free(uri);
	g_free(after_track);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_TrackList_RemoveTrack (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	g_dbus_method_invocation_return_error_literal (invocation,
		G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "TrackList is read-only.");
}

static void mpris_TrackList_GoTo (GDBusMethodInvocation *invocation, GVariant* parameters, PraghaMpris2 *mpris2)
{
	PraghaPlaylist *playlist;
	PraghaMusicobject *mobj = NULL;
	gchar *track_id = NULL;

	PraghaApplication *pragha = mpris2->pragha;

	g_variant_get(parameters, "(o)", &track_id);

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist GoTo");

	mobj = get_mobj_at_mpris2_track_id(pragha, track_id);

	if (mobj) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_activate_unique_mobj (playlist, mobj);
		g_dbus_method_invocation_return_value (invocation, NULL);
	}
	else
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Unknown or malformed playlist object path.");

	g_free (track_id);
}

static GVariant* mpris_TrackList_get_Tracks (GError **error, PraghaMpris2 *mpris2)
{
	PraghaPlaylist *playlist;
	GVariantBuilder builder;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL, *i;

	PraghaApplication *pragha = mpris2->pragha;

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist get Tracks");

	g_variant_builder_init(&builder, G_VARIANT_TYPE("ao"));

	playlist = pragha_application_get_playlist (pragha);
	list = pragha_playlist_get_mobj_list (playlist);

	if(list != NULL) {
		list = g_list_reverse(list);
		for (i=list; i != NULL; i = i->next) {
			mobj = i->data;
			g_variant_builder_add_value(&builder, handle_get_trackid(mobj));
		}
		g_list_free(list);
	}

	return g_variant_builder_end(&builder);
}

static GVariant* mpris_TrackList_get_CanEditTracks (GError **error, PraghaMpris2 *mpris2)
{
	return g_variant_new_boolean(FALSE);
}

/* dbus callbacks */
static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	/* org.mpris.MediaPlayer2 */
	BEGIN_INTERFACE(0)
		MAP_METHOD(Root, Raise)
		MAP_METHOD(Root, Quit)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Player */
	BEGIN_INTERFACE(1)
		MAP_METHOD(Player, Next)
		MAP_METHOD(Player, Previous)
		MAP_METHOD(Player, Pause)
		MAP_METHOD(Player, PlayPause)
		MAP_METHOD(Player, Stop)
		MAP_METHOD(Player, Play)
		MAP_METHOD(Player, Seek)
		MAP_METHOD(Player, SetPosition)
		MAP_METHOD(Player, OpenUri)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Playlists */
	BEGIN_INTERFACE(2)
		MAP_METHOD(Playlists, ActivatePlaylist)
		MAP_METHOD(Playlists, GetPlaylists)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.TrackList */
	BEGIN_INTERFACE(3)
		MAP_METHOD(TrackList, GetTracksMetadata)
		MAP_METHOD(TrackList, AddTrack)
		MAP_METHOD(TrackList, RemoveTrack)
		MAP_METHOD(TrackList, GoTo)
	END_INTERFACE
}

static GVariant *
handle_get_property (GDBusConnection  *connection,
                     const gchar      *sender,
                     const gchar      *object_path,
                     const gchar      *interface_name,
                     const gchar      *property_name,
                     GError          **error,
                     gpointer          user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	/* org.mpris.MediaPlayer2 */
	BEGIN_INTERFACE(0)
		PROPGET(Root, CanQuit)
		PROPGET(Root, CanRaise)
		PROPGET(Root, HasTrackList)
		PROPGET(Root, Identity)
		PROPGET(Root, DesktopEntry)
		PROPGET(Root, SupportedUriSchemes)
		PROPGET(Root, SupportedMimeTypes)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Player */
	BEGIN_INTERFACE(1)
		PROPGET(Player, PlaybackStatus)
		PROPGET(Player, LoopStatus)
		PROPGET(Player, Rate)
		PROPGET(Player, Shuffle)
		PROPGET(Player, Metadata)
		PROPGET(Player, Volume)
		PROPGET(Player, Position)
		PROPGET(Player, MinimumRate)
		PROPGET(Player, MaximumRate)
		PROPGET(Player, CanGoNext)
		PROPGET(Player, CanGoPrevious)
		PROPGET(Player, CanPlay)
		PROPGET(Player, CanPause)
		PROPGET(Player, CanSeek)
		PROPGET(Player, CanControl)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Playlists */
	BEGIN_INTERFACE(2)
		PROPGET(Playlists, PlaylistCount)
		PROPGET(Playlists, Orderings)
		PROPGET(Playlists, ActivePlaylist)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.TrackList */
	BEGIN_INTERFACE(3)
		PROPGET(TrackList, Tracks)
		PROPGET(TrackList, CanEditTracks)
	END_INTERFACE
	return NULL;
}

static gboolean
handle_set_property (GDBusConnection  *connection,
                     const gchar      *sender,
                     const gchar      *object_path,
                     const gchar      *interface_name,
                     const gchar      *property_name,
                     GVariant         *value,
                     GError          **error,
                     gpointer          user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	/* org.mpris.MediaPlayer2 */
	BEGIN_INTERFACE(0)
		/* all properties readonly */
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Player */
	BEGIN_INTERFACE(1)
		PROPPUT(Player, LoopStatus)
		PROPPUT(Player, Rate)
		PROPPUT(Player, Shuffle)
		PROPPUT(Player, Volume)
	END_INTERFACE
	/* org.mpris.MediaPlayer2.Playlists */
	BEGIN_INTERFACE(2)
		/* all properties readonly */
	END_INTERFACE
	/* org.mpris.MediaPlayer2.TrackList */
	BEGIN_INTERFACE(3)
		/* all properties readonly */
	END_INTERFACE
	return (NULL == *error);
}

static const GDBusInterfaceVTable interface_vtable =
{
  handle_method_call,
  handle_get_property,
  handle_set_property
};

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	guint registration_id;
	gint i;
	PraghaMpris2 *mpris2 = user_data;

	for(i = 0; i < 4; i++)
	{
		mpris2->interface_quarks[i] = g_quark_from_string(mpris2->introspection_data->interfaces[i]->name);
		registration_id = g_dbus_connection_register_object (connection,
									MPRIS_PATH,
									mpris2->introspection_data->interfaces[i],
									&interface_vtable,
									mpris2,  /* user_data */
									NULL,  /* user_data_free_func */
									NULL); /* GError** */
		g_assert (registration_id > 0);
	}
	
	mpris2->dbus_connection = connection;
	g_object_ref(G_OBJECT(mpris2->dbus_connection));
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
	CDEBUG(DBG_INFO, "Acquired DBus name %s", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	if(NULL != mpris2->dbus_connection) {
		g_object_unref(G_OBJECT(mpris2->dbus_connection));
		mpris2->dbus_connection = NULL;
	}

	CDEBUG(DBG_INFO, "Lost DBus name %s", name);
}

/* pragha callbacks */

void pragha_mpris_update_any (PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;
	PraghaPreferences *preferences;
	gboolean change_detected = FALSE, shuffle, repeat;
	GVariantBuilder b;
	const gchar *newtitle = NULL;
	gdouble curr_vol;

	PraghaApplication *pragha = mpris2->pragha;

	if(NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update any");

	backend = pragha_application_get_backend (pragha);

	if (pragha_backend_get_state (backend) != ST_STOPPED) {
		newtitle = pragha_musicobject_get_file (pragha_backend_get_musicobject (backend));
	}

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));

	preferences = pragha_application_get_preferences (pragha);

	shuffle = pragha_preferences_get_shuffle (preferences);
	if(mpris2->saved_shuffle != shuffle)
	{
		change_detected = TRUE;
		mpris2->saved_shuffle = shuffle;
		g_variant_builder_add (&b, "{sv}", "Shuffle", mpris_Player_get_Shuffle (NULL, mpris2));
	}
	if(mpris2->state != pragha_backend_get_state (backend))
	{
		change_detected = TRUE;
		mpris2->state = pragha_backend_get_state (backend);
		g_variant_builder_add (&b, "{sv}", "PlaybackStatus", mpris_Player_get_PlaybackStatus (NULL, mpris2));
	}
	repeat = pragha_preferences_get_repeat (preferences);
	if(mpris2->saved_playbackstatus != repeat)
	{
		change_detected = TRUE;
		mpris2->saved_playbackstatus = repeat;
		g_variant_builder_add (&b, "{sv}", "LoopStatus", mpris_Player_get_LoopStatus (NULL, mpris2));
	}
	curr_vol = pragha_backend_get_volume (backend);
	if(mpris2->volume != curr_vol)
	{
		change_detected = TRUE;
		mpris2->volume = curr_vol;
		g_variant_builder_add (&b, "{sv}", "Volume", mpris_Player_get_Volume (NULL, mpris2));
	}
	if(g_strcmp0(mpris2->saved_title, newtitle))
	{
		change_detected = TRUE;
		if(mpris2->saved_title)
			g_free(mpris2->saved_title);
		if(string_is_not_empty(newtitle))
			mpris2->saved_title = g_strdup(newtitle);
		else
			mpris2->saved_title = NULL;
		g_variant_builder_add (&b, "{sv}", "Metadata", mpris_Player_get_Metadata (NULL, mpris2));
	}
	if(change_detected)
	{
		GVariant * tuples[] = {
			g_variant_new_string("org.mpris.MediaPlayer2.Player"),
			g_variant_builder_end(&b),
			g_variant_new_strv(NULL, 0)
		};

		g_dbus_connection_emit_signal(mpris2->dbus_connection, NULL, MPRIS_PATH,
			"org.freedesktop.DBus.Properties", "PropertiesChanged",
			g_variant_new_tuple(tuples, 3) , NULL);
	}
	else
	{
		g_variant_builder_clear(&b);
	}
}

void
pragha_mpris_update_metadata_changed (PraghaMpris2 *mpris2)
{
	GVariantBuilder b;

	if (NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update metadata of current track.");

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));

	g_variant_builder_add (&b, "{sv}", "Metadata", mpris_Player_get_Metadata (NULL, mpris2));

	GVariant * tuples[] = {
		g_variant_new_string("org.mpris.MediaPlayer2.Player"),
		g_variant_builder_end(&b),
		g_variant_new_strv(NULL, 0)
	};

	g_dbus_connection_emit_signal(mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.freedesktop.DBus.Properties", "PropertiesChanged",
		g_variant_new_tuple(tuples, 3) , NULL);
}

void pragha_mpris_update_mobj_remove(PraghaMpris2 *mpris2, PraghaMusicobject *mobj)
{
	GVariant * tuples[1];

	if(NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update mobj remove");

	tuples[0] = handle_get_trackid(mobj);

	g_dbus_connection_emit_signal (mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackRemoved",
		g_variant_new_tuple(tuples, 1), NULL);
}

void pragha_mpris_update_mobj_added (PraghaMpris2 *mpris2, PraghaMusicobject *mobj, GtkTreeIter *iter)
{
	PraghaPlaylist *playlist;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	PraghaMusicobject *prev = NULL;
	GVariantBuilder b;

	PraghaApplication *pragha = mpris2->pragha;

	if(NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	playlist = pragha_application_get_playlist (pragha);
	model = pragha_playlist_get_model (playlist);

	CDEBUG(DBG_MPRIS, "MPRIS update mobj added");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(a{sv}o)"));
	path = gtk_tree_model_get_path(model, iter);

	if (gtk_tree_path_prev(path)) {
		prev = current_playlist_mobj_at_path(path, playlist);
	}
	gtk_tree_path_free(path);

	g_variant_builder_open(&b, G_VARIANT_TYPE("a{sv}"));
	handle_get_metadata(mobj, &b);
	g_variant_builder_close(&b);

	g_variant_builder_add_value(&b, (prev) ?
		handle_get_trackid(prev) :
		g_variant_new_object_path("/"));
		// or use g_variant_new_string(""); ?
		// "/" is the only legal empty object path, but
		// the spec wants an empty string. What do the others do?

	g_dbus_connection_emit_signal (mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackAdded",
		g_variant_builder_end(&b), NULL);
}

void pragha_mpris_update_mobj_changed(PraghaMpris2 *mpris2, PraghaMusicobject *mobj, gint bitmask) {
	GVariantBuilder b;

	if(NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update mobj changed");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(a{sv})"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("a{sv}"));

	// should we only submit the changed metadata here? The spec is not clear.
	// If yes, use the portions in the bitmask parameter only.
	handle_get_metadata(mobj, &b);

	g_variant_builder_close(&b);

	g_dbus_connection_emit_signal(mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackChanged",
		g_variant_builder_end(&b), NULL);
}

void pragha_mpris_update_tracklist_replaced (PraghaMpris2 *mpris2)
{
	PraghaPlaylist *playlist;
	PraghaBackend *backend;
	GVariantBuilder b;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL, *i;

	PraghaApplication *pragha = mpris2->pragha;

	if (NULL == mpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update tracklist changed");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(aoo)"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("ao"));

	playlist = pragha_application_get_playlist (pragha);
	list = pragha_playlist_get_mobj_list (playlist);

	if(list != NULL) {
		list = g_list_reverse(list);
		for (i=list; i != NULL; i = i->next) {
			mobj = i->data;
			g_variant_builder_add_value(&b, handle_get_trackid(mobj));
		}
		g_list_free(list);
	}

	backend = pragha_application_get_backend (pragha);

	g_variant_builder_close(&b);
	g_variant_builder_add_value(&b, handle_get_trackid(pragha_backend_get_musicobject(backend)));
	g_dbus_connection_emit_signal (mpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackListReplaced",
		g_variant_builder_end(&b), NULL);
}

static void
any_notify_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	PraghaMpris2 *mpris2 = user_data;

	pragha_mpris_update_any (mpris2);
}

gint pragha_mpris_init (PraghaMpris2 *mpris2, PraghaApplication *pragha)
{
	PraghaBackend *backend;
	PraghaPreferences *preferences;

	preferences = pragha_application_get_preferences (pragha);
	if (!pragha_preferences_get_use_mpris2 (preferences))
		return 0;

	if(NULL != mpris2->dbus_connection)
		return 0;

	CDEBUG(DBG_INFO, "Initializing MPRIS");

	mpris2->pragha = pragha;

	mpris2->saved_shuffle = FALSE;
	mpris2->saved_playbackstatus = FALSE;
	mpris2->saved_title = NULL;
	mpris2->volume = 0;

	mpris2->introspection_data = g_dbus_node_info_new_for_xml (mpris2xml, NULL);
	g_assert (mpris2->introspection_data != NULL);

	mpris2->owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
	                                   MPRIS_NAME,
	                                   G_BUS_NAME_OWNER_FLAGS_NONE,
	                                   on_bus_acquired,
	                                   on_name_acquired,
	                                   on_name_lost,
	                                   mpris2,
	                                   NULL);

	g_signal_connect (preferences, "notify::shuffle", G_CALLBACK (any_notify_cb), mpris2);
	g_signal_connect (preferences, "notify::repeat", G_CALLBACK (any_notify_cb), mpris2);

	backend = pragha_application_get_backend (pragha);
	g_signal_connect (backend, "notify::volume", G_CALLBACK (any_notify_cb), mpris2);
	g_signal_connect (backend, "notify::state", G_CALLBACK (any_notify_cb), mpris2);
	g_signal_connect (backend, "seeked", G_CALLBACK (seeked_cb), mpris2);

	return (mpris2->owner_id) ? 0 : -1;
}

void pragha_mpris_close (PraghaMpris2 *mpris2)
{
	PraghaBackend *backend;
	PraghaApplication *pragha = mpris2->pragha;

	backend = pragha_application_get_backend (pragha);

	if(NULL == mpris2->dbus_connection)
		return;

	g_signal_handlers_disconnect_by_func (backend, seeked_cb, mpris2);
	g_signal_handlers_disconnect_by_func (backend, any_notify_cb, mpris2);

	if(NULL != mpris2->dbus_connection)
		g_bus_unown_name (mpris2->owner_id);

	if(NULL != mpris2->introspection_data) {
		g_dbus_node_info_unref (mpris2->introspection_data);
		mpris2->introspection_data = NULL;
	}
	if(NULL != mpris2->dbus_connection) {
		g_object_unref (G_OBJECT (mpris2->dbus_connection));
		mpris2->dbus_connection = NULL;
	}

	g_free (mpris2->saved_title);
}

void pragha_mpris_free (PraghaMpris2 *mpris2)
{
	pragha_mpris_close (mpris2);
	g_slice_free (PraghaMpris2, mpris2);
}

PraghaMpris2 *pragha_mpris_new()
{
	return g_slice_new0(PraghaMpris2);
}

// still todo:
// * emit Playlists.PlaylistChanged signal when playlist rename is implemented
// * provide an Icon for a playlist when e.g. 'smart playlists' are implemented
// * emit couple of TrackList signals when drag'n drop reordering
// * find a better object path than mobj address & remove all gtk tree model access
// * [optional] implement tracklist edit
