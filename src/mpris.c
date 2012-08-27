/************************************************************************
 * Copyright (C) 2011-2012 matias <mati86dl@gmail.com>                  *
 * Copyright (C) 2011      hakan  <smultimeter@gmail.com>               *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http:www.gnu.org/licenses/>.  *
 ************************************************************************/

#include "pragha.h"

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
	if(g_quark_try_string(interface_name)==cwin->cmpris2->interface_quarks[x]) {
#define MAP_METHOD(x,y) \
	if(!g_strcmp0(#y, method_name)) { \
		mpris_##x##_##y(invocation, parameters, cwin); return; }
#define PROPGET(x,y) \
	if(!g_strcmp0(#y, property_name)) \
		return mpris_##x##_get_##y(error, cwin);
#define PROPPUT(x,y) \
	if(g_quark_try_string(property_name)==g_quark_from_static_string(#y)) \
		mpris_##x##_put_##y(value, error, cwin);
#define END_INTERFACE }

/* org.mpris.MediaPlayer2 */
static void mpris_Root_Raise (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gtk_window_present(GTK_WINDOW(cwin->mainwindow));
	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Root_Quit (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	exit_pragha(NULL, cwin);
	g_dbus_method_invocation_return_value (invocation, NULL);
}

static GVariant* mpris_Root_get_CanQuit (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_CanRaise (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_HasTrackList (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Root_get_Identity (GError **error, struct con_win *cwin)
{
	return g_variant_new_string("Pragha Music Player");
}

static GVariant* mpris_Root_get_DesktopEntry (GError **error, struct con_win *cwin)
{
	GVariant* ret_val = g_variant_new_string("pragha");
	return ret_val;
}

static GVariant* mpris_Root_get_SupportedUriSchemes (GError **error, struct con_win *cwin)
{
	return g_variant_parse(G_VARIANT_TYPE("as"),
		"['file', 'cdda']", NULL, NULL, NULL);
}

static GVariant* mpris_Root_get_SupportedMimeTypes (GError **error, struct con_win *cwin)
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
static void mpris_Player_Play (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		play_track(cwin);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Next (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		play_next_track(cwin);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Previous (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		play_prev_track(cwin);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Pause (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		pragha_backend_pause(cwin->backend);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_PlayPause (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		play_pause_resume(cwin);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Stop (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if(cwin->backend->emitted_error == FALSE)
		pragha_backend_stop(cwin->backend, NULL);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_Seek (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	if (!cwin->cstate->curr_mobj) {
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Nothing to seek");
		return;
	}

	gdouble fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar));
	gint seek = cwin->cstate->curr_mobj->tags->length * fraction;
	gint64 param;
	g_variant_get(parameters, "(x)", &param);
	seek += (param / 1000000);

	if (seek >= cwin->cstate->curr_mobj->tags->length)
		seek = cwin->cstate->curr_mobj->tags->length;

	pragha_backend_seek(cwin->backend, seek);
	mpris_update_seeked(cwin, seek);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_Player_SetPosition (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gint64 param;
	gchar *path = NULL;
	struct musicobject *mobj = NULL;

	g_variant_get(parameters, "(ox)", &path, &param);
	if (handle_path_request(cwin, path, &mobj, NULL) && cwin->cstate->curr_mobj == mobj) {
		gint seek = (param / 1000000);

		if (seek >= cwin->cstate->curr_mobj->tags->length)
			seek = cwin->cstate->curr_mobj->tags->length;

		pragha_backend_seek(cwin->backend, seek);
		mpris_update_seeked(cwin, seek);
	}
	g_free(path);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

void mpris_update_seeked(struct con_win *cwin, gint position) {
	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS emit seeked signal..");

	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.Player", "Seeked",
		 g_variant_new ("(x)", (gint64)(position * 1000000)), NULL);
}

static void mpris_Player_OpenUri (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gchar *uri = NULL;
	g_variant_get(parameters, "(s)", &uri);
	gboolean failed = FALSE;

	CDEBUG(DBG_MPRIS, "MPRIS Player OpenUri");

	if(uri) {
		// TODO: Translate "cdda://sr0/Track 01.wav" URIs for new_musicobject_from_cdda()
		//       If there is such a convention on other players
		gchar *path = g_filename_from_uri(uri, NULL, NULL);
		if(path && is_playable_file(path)) {
			struct musicobject *mobj = new_musicobject_from_file(path);
			if(mobj) {
				GtkTreePath *tree_path;
				append_current_playlist_ex(NULL, mobj, cwin, &tree_path);
				update_status_bar(cwin);

				// Dangerous: reusing double-click-handler here.
				current_playlist_row_activated_cb(
					GTK_TREE_VIEW(cwin->current_playlist), tree_path, NULL, cwin);

				gtk_tree_path_free(tree_path);
			} else {
				failed = TRUE;
			}
			g_free(uri);
		} else {
			failed = TRUE;
		}
		g_free(path);
	} else {
		failed = TRUE;
	}
	if(failed)
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_INVALID_FILE_CONTENT, "This file does not play here.");
	else
		g_dbus_method_invocation_return_value (invocation, NULL);
}

static GVariant* mpris_Player_get_PlaybackStatus (GError **error, struct con_win *cwin)
{
	switch (cwin->cstate->state) {
	case ST_PLAYING:	return g_variant_new_string("Playing");
	case ST_PAUSED:		return g_variant_new_string("Paused");
	default:		return g_variant_new_string("Stopped");
	}
}

static GVariant* mpris_Player_get_LoopStatus (GError **error, struct con_win *cwin)
{
	return g_variant_new_string(cwin->cpref->repeat ? "Playlist" : "None");
}

static void mpris_Player_put_LoopStatus (GVariant *value, GError **error, struct con_win *cwin)
{
	const gchar *new_loop = g_variant_get_string(value, NULL);
	gboolean repeat = g_strcmp0("Playlist", new_loop) ? FALSE : TRUE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->repeat_button), repeat);
}

static GVariant* mpris_Player_get_Rate (GError **error, struct con_win *cwin)
{
	return g_variant_new_double(1.0);
}

static void mpris_Player_put_Rate (GVariant *value, GError **error, struct con_win *cwin)
{
	g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "This is not alsaplayer.");
}

static GVariant* mpris_Player_get_Shuffle (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(cwin->cpref->shuffle);
}

static void mpris_Player_put_Shuffle (GVariant *value, GError **error, struct con_win *cwin)
{
	gboolean shuffle = g_variant_get_boolean(value);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cwin->shuffle_button), shuffle);
}

static GVariant * handle_get_trackid(struct musicobject *mobj) {
	gchar *o = alloca(260);
	if(NULL == mobj)
		return g_variant_new_object_path("/");
	g_snprintf(o, 260, "%s/TrackList/%p", MPRIS_PATH, mobj);
	return g_variant_new_object_path(o);
}

void handle_strings_request(GVariantBuilder *b, gchar *tag, gchar *val)
{
	GVariant *vval = g_variant_new_string(val);
	GVariant *vvals = g_variant_new_array(G_VARIANT_TYPE_STRING, &vval, 1);

	g_variant_builder_add (b, "{sv}", tag, vvals);
}

static void handle_get_metadata(struct musicobject *mobj, GVariantBuilder *b)
{
	gchar *date = g_strdup_printf("%d", mobj->tags->year);

	gchar *url = (mobj->file_type == FILE_HTTP || mobj->file_type == FILE_CDDA) ?
			g_strdup(mobj->file) : g_filename_to_uri(mobj->file, NULL, NULL);

	CDEBUG(DBG_MPRIS, "MPRIS handle get metadata");

	g_variant_builder_add (b, "{sv}", "mpris:trackid",
		handle_get_trackid(mobj));
	g_variant_builder_add (b, "{sv}", "xesam:url",
		g_variant_new_string(url));
	g_variant_builder_add (b, "{sv}", "xesam:title",
		g_variant_new_string(mobj->tags->title));
	handle_strings_request(b, "xesam:artist", mobj->tags->artist);
	g_variant_builder_add (b, "{sv}", "xesam:album",
		g_variant_new_string(mobj->tags->album));
	handle_strings_request(b, "xesam:genre", mobj->tags->genre);
	g_variant_builder_add (b, "{sv}", "xesam:contentCreated",
		g_variant_new_string (date));
	g_variant_builder_add (b, "{sv}", "xesam:trackNumber",
		g_variant_new_int32(mobj->tags->track_no));
	handle_strings_request(b, "xesam:comment", mobj->tags->comment);
	g_variant_builder_add (b, "{sv}", "mpris:length",
		g_variant_new_int64((gint64)(mobj->tags->length * 1000000)));
	g_variant_builder_add (b, "{sv}", "audio-bitrate",
		g_variant_new_int32(mobj->tags->bitrate));
	g_variant_builder_add (b, "{sv}", "audio-channels",
		g_variant_new_int32(mobj->tags->channels));
	g_variant_builder_add (b, "{sv}", "audio-samplerate",
		g_variant_new_int32(mobj->tags->samplerate));

	g_free(date);
	g_free(url);
}

static GVariant* mpris_Player_get_Metadata (GError **error, struct con_win *cwin)
{
	gchar *artUrl_uri = NULL;
	GVariantBuilder b;

	CDEBUG(DBG_MPRIS, "MPRIS Player get Metadata");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("a{sv}"));

	if (cwin->cstate->state != ST_STOPPED) {
		handle_get_metadata(cwin->cstate->curr_mobj, &b);
		/* Append the album art url metadata. */
		if(cwin->cstate->arturl != NULL) {
			artUrl_uri = g_filename_to_uri(cwin->cstate->arturl, NULL, NULL);
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

static GVariant* mpris_Player_get_Volume (GError **error, struct con_win *cwin)
{
	return g_variant_new_double(pragha_backend_get_volume (cwin->backend));
}

static void mpris_Player_put_Volume (GVariant *value, GError **error, struct con_win *cwin)
{
	gdouble volume = g_variant_get_double(value);
	pragha_backend_set_volume(cwin->backend, volume);
}

static GVariant* mpris_Player_get_Position (GError **error, struct con_win *cwin)
{
	if (cwin->cstate->state == ST_STOPPED)
		return g_variant_new_int64(0);
	else
		return g_variant_new_int64(pragha_backend_get_current_position(cwin->backend) / 1000);
}

static GVariant* mpris_Player_get_MinimumRate (GError **error, struct con_win *cwin)
{
	return g_variant_new_double(1.0);
}

static GVariant* mpris_Player_get_MaximumRate (GError **error, struct con_win *cwin)
{
	return g_variant_new_double(1.0);
}

static GVariant* mpris_Player_get_CanGoNext (GError **error, struct con_win *cwin)
{
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Player_get_CanGoPrevious (GError **error, struct con_win *cwin)
{
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}

static GVariant* mpris_Player_get_CanPlay (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(NULL != cwin->cstate->curr_mobj);
}

static GVariant* mpris_Player_get_CanPause (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(NULL != cwin->cstate->curr_mobj);
}

static GVariant* mpris_Player_get_CanSeek (GError **error, struct con_win *cwin)
{
	return g_variant_new_boolean(cwin->backend->seek_enabled);
}

static GVariant* mpris_Player_get_CanControl (GError **error, struct con_win *cwin)
{
	// always?
	return g_variant_new_boolean(TRUE);
}

/* org.mpris.MediaPlayer2.Playlists */
static void mpris_Playlists_ActivatePlaylist (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gchar *get_playlist = NULL, *test_playlist = NULL, *found_playlist = NULL;
	gchar **db_playlists = NULL;
	gint i = 0;

	CDEBUG(DBG_MPRIS, "MPRIS Playlists ActivatePlaylist");

	g_variant_get(parameters, "(o)", &get_playlist);

	if(get_playlist && g_str_has_prefix(get_playlist, MPRIS_PATH)) {
		db_playlists = get_playlist_names_db(cwin->cdbase);
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
		gdk_threads_enter();
		clear_current_playlist(NULL, cwin);
		add_playlist_current_playlist(NULL, found_playlist, cwin);
		gdk_threads_leave();

		pragha_backend_stop(cwin->backend, NULL);
		play_first_current_playlist (cwin);

		g_free(found_playlist);

		g_dbus_method_invocation_return_value (invocation, NULL);
	}
	else {
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Unknown or malformed playlist object path.");
	}

	g_free (get_playlist);
}

static void mpris_Playlists_GetPlaylists (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	GVariantBuilder builder;
	guint i = 0, start, max;
	gchar *order;
	gchar ** lists = NULL;
	gchar *listpath = NULL;
	gboolean reverse;
	gint imax = 0;

	CDEBUG(DBG_MPRIS, "MPRIS Playlists GetPlaylists");

	g_variant_builder_init(&builder, G_VARIANT_TYPE("(a(oss))"));
	g_variant_builder_open(&builder, G_VARIANT_TYPE("a(oss)"));

	lists = get_playlist_names_db(cwin->cdbase);

	if (lists) {
		g_variant_get(parameters, "(uusb)", &start, &max, &order, &reverse);
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

static GVariant* mpris_Playlists_get_ActivePlaylist (GError **error, struct con_win *cwin)
{
	return g_variant_new("(b(oss))",
		FALSE, "/", _("Playlists"), _("Playlists"));

	/* Formally this is correct, but in the practice only is used to
	   display a confuse message "invalid" in the ubuntu-soundmenu.
	return g_variant_new("(b(oss))",
		FALSE, "/", "invalid", "invalid");*/
}

static GVariant* mpris_Playlists_get_Orderings (GError **error, struct con_win *cwin)
{
	return g_variant_parse(G_VARIANT_TYPE("as"),
		"['UserDefined']", NULL, NULL, NULL);
}

static GVariant* mpris_Playlists_get_PlaylistCount (GError **error, struct con_win *cwin)
{
	return g_variant_new_uint32(get_playlist_count_db(cwin->cdbase));
}

gboolean handle_path_request(struct con_win *cwin, const gchar *dbus_path,
		struct musicobject **mobj, GtkTreePath **tree_path) {
	gchar *base = g_strdup_printf("%s/TrackList/", MPRIS_PATH);
	gboolean found = FALSE;
	*mobj = NULL;
	if(g_str_has_prefix(dbus_path, base)) {

		void *request = NULL;
		sscanf(dbus_path + strlen(base), "%p", &request);

		if(request) {
			GtkTreePath *path = current_playlist_path_at_mobj(request, cwin);
			if(path) {
				found = TRUE;
				*mobj = request;
				if(tree_path)
					*tree_path = path;
				else
					gtk_tree_path_free(path);
			}
		}
	}
	g_free(base);
	return found;
}

/* org.mpris.MediaPlayer2.TrackList */
static void mpris_TrackList_GetTracksMetadata (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	/* In: (ao) out: aa{sv} */

	GVariant *param1 = g_variant_get_child_value(parameters, 0);
	gsize i, length;
	GVariantBuilder b;
	const gchar *path;

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist GetTracksMetada");

	g_variant_builder_init(&b, G_VARIANT_TYPE("(aa{sv})"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("aa{sv}"));

	length = g_variant_n_children(param1);
	
	for(i = 0; i < length; i++) {
		g_variant_builder_open(&b, G_VARIANT_TYPE("a{sv}"));
		struct musicobject *mobj= NULL;
		path = g_variant_get_string(g_variant_get_child_value(param1, i), NULL);
		if (handle_path_request(cwin, path, &mobj, NULL)) {
			handle_get_metadata(mobj, &b);
		} else {
			g_variant_builder_add (&b, "{sv}", "mpris:trackid",
			g_variant_new_object_path(path));
		}
		g_variant_builder_close(&b);
	}
	g_variant_builder_close(&b);

	g_dbus_method_invocation_return_value (invocation, g_variant_builder_end (&b));
}

static void mpris_TrackList_AddTrack (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gchar *uri;
	gchar *after_track; //TODO use this
	gboolean set_as_current; //TODO use this

	g_variant_get(parameters, "(sob)", &uri, &after_track, &set_as_current);

	gchar *file = g_filename_from_uri(uri, NULL, NULL);

	if (!file) {
		g_warning("Invalid uri: %s", uri);
		goto exit;
	}

	gdk_threads_enter();

	if (is_dir_and_accessible(file, cwin)) {
		if(cwin->cpref->add_recursively_files)
			__recur_add(file, cwin);
		else
			__non_recur_add(file, TRUE, cwin);
	}
	else if (is_playable_file(file)) {
		struct musicobject *mobj = new_musicobject_from_file(file);
		if (mobj)
			append_current_playlist(NULL, mobj, cwin);
		CDEBUG(DBG_INFO, "Add file from mpris: %s", file);
	}
	else {
		g_warning("Unable to add file %s", file);
	}
	select_last_path_of_current_playlist(cwin);
	update_status_bar(cwin);

	gdk_threads_leave();

	g_free(file);
exit:
	g_free(uri);
	g_free(after_track);

	g_dbus_method_invocation_return_value (invocation, NULL);
}

static void mpris_TrackList_RemoveTrack (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	g_dbus_method_invocation_return_error_literal (invocation,
		G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "TrackList is read-only.");
}

static void mpris_TrackList_GoTo (GDBusMethodInvocation *invocation, GVariant* parameters, struct con_win *cwin)
{
	gchar *path = NULL;
	GtkTreePath *tree_path = NULL;
	g_variant_get(parameters, "(o)", &path);
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist GoTo");

	if(handle_path_request(cwin, path, &mobj, &tree_path)) {
		// Dangerous: reusing double-click handler here.
		current_playlist_row_activated_cb(
			GTK_TREE_VIEW(cwin->current_playlist), tree_path, NULL, cwin);
		g_dbus_method_invocation_return_value (invocation, NULL);
	} else
		g_dbus_method_invocation_return_error_literal (invocation,
				G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Unknown or malformed playlist object path.");

	gtk_tree_path_free (tree_path);
	g_free (path);
}

static GVariant* mpris_TrackList_get_Tracks (GError **error, struct con_win *cwin)
{
	GVariantBuilder builder;
	GtkTreeModel *model;
	GtkTreeIter iter;
	struct musicobject *mobj = NULL;

	CDEBUG(DBG_MPRIS, "MPRIS Tracklist get Tracks");

	g_variant_builder_init(&builder, G_VARIANT_TYPE("ao"));

	// TODO: remove tree access
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	
	if (!gtk_tree_model_get_iter_first(model, &iter))
		goto bad;

	do {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj) {
			g_variant_builder_add_value(&builder, handle_get_trackid(mobj));
		}
	} while(gtk_tree_model_iter_next(model, &iter));

bad:
	return g_variant_builder_end(&builder);
}

static GVariant* mpris_TrackList_get_CanEditTracks (GError **error, struct con_win *cwin)
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
                    gpointer               user_data) {
	struct con_win *cwin = user_data;
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
                     gpointer          user_data) {
	struct con_win *cwin = user_data;
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
	struct con_win *cwin = user_data;
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
	struct con_win *cwin = user_data;

	for(i = 0; i < 4; i++)
	{
		cwin->cmpris2->interface_quarks[i] = g_quark_from_string(cwin->cmpris2->introspection_data->interfaces[i]->name);
		registration_id = g_dbus_connection_register_object (connection,
									MPRIS_PATH,
									cwin->cmpris2->introspection_data->interfaces[i],
									&interface_vtable,
									cwin,  /* user_data */
									NULL,  /* user_data_free_func */
									NULL); /* GError** */
		g_assert (registration_id > 0);
	}
	
	cwin->cmpris2->dbus_connection = connection;
	g_object_ref(G_OBJECT(cwin->cmpris2->dbus_connection));

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
	struct con_win *cwin = user_data;
	if(NULL != cwin->cmpris2->dbus_connection) {
		g_object_unref(G_OBJECT(cwin->cmpris2->dbus_connection));
		cwin->cmpris2->dbus_connection = NULL;
	}

	CDEBUG(DBG_INFO, "Lost DBus name %s", name);
}

/* pragha callbacks */

void mpris_update_any(struct con_win *cwin)
{
	gboolean change_detected = FALSE;
	GVariantBuilder b;
	gchar *newtitle = NULL;
	gdouble curr_vol = pragha_backend_get_volume (cwin->backend);

	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update any");

	if (cwin->cstate->state != ST_STOPPED)
		newtitle = cwin->cstate->curr_mobj->file;

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));
	if(cwin->cmpris2->saved_shuffle != cwin->cpref->shuffle)
	{
		change_detected = TRUE;
		cwin->cmpris2->saved_shuffle = cwin->cpref->shuffle;
		g_variant_builder_add (&b, "{sv}", "Shuffle", mpris_Player_get_Shuffle (NULL, cwin));
	}
	if(cwin->cmpris2->state != cwin->cstate->state)
	{
		change_detected = TRUE;
		cwin->cmpris2->state = cwin->cstate->state;
		g_variant_builder_add (&b, "{sv}", "PlaybackStatus", mpris_Player_get_PlaybackStatus (NULL, cwin));
	}
	if(cwin->cmpris2->saved_playbackstatus != cwin->cpref->repeat)
	{
		change_detected = TRUE;
		cwin->cmpris2->saved_playbackstatus = cwin->cpref->repeat;
		g_variant_builder_add (&b, "{sv}", "LoopStatus", mpris_Player_get_LoopStatus (NULL, cwin));
	}
	if(cwin->cmpris2->volume != curr_vol)
	{
		change_detected = TRUE;
		cwin->cmpris2->volume = curr_vol;
		g_variant_builder_add (&b, "{sv}", "Volume", mpris_Player_get_Volume (NULL, cwin));
	}
	if(g_strcmp0(cwin->cmpris2->saved_title, newtitle))
	{
		change_detected = TRUE;
		if(cwin->cmpris2->saved_title)
			g_free(cwin->cmpris2->saved_title);
		if(newtitle)
			cwin->cmpris2->saved_title = g_strdup(newtitle);
		else 
			cwin->cmpris2->saved_title = NULL;
		g_variant_builder_add (&b, "{sv}", "Metadata", mpris_Player_get_Metadata (NULL, cwin));
	}
	if(change_detected)
	{
		GVariant * tuples[] = {
			g_variant_new_string("org.mpris.MediaPlayer2.Player"),
			g_variant_builder_end(&b),
			g_variant_new_strv(NULL, 0)
		};

		g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
			"org.freedesktop.DBus.Properties", "PropertiesChanged",
			g_variant_new_tuple(tuples, 3) , NULL);
	}
	else
	{
		g_variant_builder_clear(&b);
	}
}

void
mpris_update_metadata_changed(struct con_win *cwin)
{
	GVariantBuilder b;

	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update metadata of current track.");

	g_variant_builder_init(&b, G_VARIANT_TYPE("a{sv}"));

	g_variant_builder_add (&b, "{sv}", "Metadata", mpris_Player_get_Metadata (NULL, cwin));

	GVariant * tuples[] = {
		g_variant_new_string("org.mpris.MediaPlayer2.Player"),
		g_variant_builder_end(&b),
		g_variant_new_strv(NULL, 0)
	};

	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.freedesktop.DBus.Properties", "PropertiesChanged",
		g_variant_new_tuple(tuples, 3) , NULL);
}

void mpris_update_mobj_remove(struct con_win *cwin, struct musicobject *mobj)
{

	GVariant * tuples[1];
	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update mobj remove");

	tuples[0] = handle_get_trackid(mobj);

	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackRemoved",
		g_variant_new_tuple(tuples, 1), NULL);
}

void mpris_update_mobj_added(struct con_win *cwin, struct musicobject *mobj, GtkTreeIter *iter)
{
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	struct musicobject *prev = NULL;
	GVariantBuilder b;

	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	if(NULL == model)
		return;

	CDEBUG(DBG_MPRIS, "MPRIS update mobj added");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(a{sv}o)"));
	path = gtk_tree_model_get_path(model, iter);

	if (gtk_tree_path_prev(path)) {
		prev = current_playlist_mobj_at_path(path, cwin);
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

	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackAdded",
		g_variant_builder_end(&b), NULL);
}

void mpris_update_mobj_changed(struct con_win *cwin, struct musicobject *mobj, gint bitmask) {
	GVariantBuilder b;

	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update mobj changed");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(a{sv})"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("a{sv}"));

	// should we only submit the changed metadata here? The spec is not clear.
	// If yes, use the portions in the bitmask parameter only.
	handle_get_metadata(mobj, &b);

	g_variant_builder_close(&b);

	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackChanged",
		g_variant_builder_end(&b), NULL);
}

void mpris_update_tracklist_replaced(struct con_win *cwin) {
	GVariantBuilder b;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean iter_valid;
	struct musicobject *mobj = NULL;

	if(NULL == cwin->cmpris2->dbus_connection)
		return; /* better safe than sorry */

	CDEBUG(DBG_MPRIS, "MPRIS update tracklist changed");

	g_variant_builder_init(&b, G_VARIANT_TYPE ("(aoo)"));
	g_variant_builder_open(&b, G_VARIANT_TYPE("ao"));

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(cwin->current_playlist));
	
	iter_valid = gtk_tree_model_get_iter_first(model, &iter);

	while (iter_valid) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
		g_variant_builder_add_value(&b, handle_get_trackid(mobj));
		iter_valid = gtk_tree_model_iter_next(model, &iter);
	}
	
	g_variant_builder_close(&b);
	g_variant_builder_add_value(&b, handle_get_trackid(cwin->cstate->curr_mobj));
	g_dbus_connection_emit_signal(cwin->cmpris2->dbus_connection, NULL, MPRIS_PATH,
		"org.mpris.MediaPlayer2.TrackList", "TrackListReplaced",
		g_variant_builder_end(&b), NULL);
}

static void
volume_notify_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	struct con_win *cwin = user_data;
	mpris_update_any (cwin);
}

gint mpris_init(struct con_win *cwin)
{
	if (!cwin->cpref->use_mpris2)
		return 0;

	CDEBUG(DBG_INFO, "Initializing MPRIS");
	g_type_init();

	cwin->cmpris2->saved_shuffle = false;
	cwin->cmpris2->saved_playbackstatus = false;
	cwin->cmpris2->saved_title = NULL;
	cwin->cmpris2->volume = 0;

	cwin->cmpris2->introspection_data = g_dbus_node_info_new_for_xml (mpris2xml, NULL);
	g_assert (cwin->cmpris2->introspection_data != NULL);

	cwin->cmpris2->owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
				MPRIS_NAME,
				G_BUS_NAME_OWNER_FLAGS_NONE,
				on_bus_acquired,
				on_name_acquired,
				on_name_lost,
				cwin,
				NULL);

	//FIXME disconnect in mpris_close
	if (!cwin->cmpris2->notify_volume_id)
		cwin->cmpris2->notify_volume_id = g_signal_connect (cwin->backend, "notify::volume",
                                                                    G_CALLBACK (volume_notify_cb), cwin);

	return (cwin->cmpris2->owner_id) ? 0 : -1;
}

void mpris_close (struct con_mpris2 *cmpris2)
{
	if(NULL != cmpris2->dbus_connection)
		g_bus_unown_name (cmpris2->owner_id);

	if(NULL != cmpris2->introspection_data) {
		g_dbus_node_info_unref (cmpris2->introspection_data);
		cmpris2->introspection_data = NULL;
	}
	if(NULL != cmpris2->dbus_connection) {
		g_object_unref (G_OBJECT (cmpris2->dbus_connection));
		cmpris2->dbus_connection = NULL;
	}
}

void mpris_free (struct con_mpris2 *cmpris2)
{
	mpris_close (cmpris2);
	g_slice_free (struct con_mpris2, cmpris2);
}

// still todo:
// * emit Playlists.PlaylistChanged signal when playlist rename is implemented
// * provide an Icon for a playlist when e.g. 'smart playlists' are implemented
// * emit couple of TrackList signals when drag'n drop reordering
// * find a better object path than mobj address & remove all gtk tree model access
// * [optional] implement tracklist edit
