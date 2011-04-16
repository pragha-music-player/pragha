/************************************************************************
 * Copyright (C) 2009-2010 matias <mati86dl@gmail.com>                  *
 * Copyright (C) 2011      hakan  <smultimeter@gmail.com>		*
 * 								        *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.				        *
 * 								        *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.			        *
 * 								        *
 * You should have received a copy of the GNU General Public License    * 
 * along with this program.  If not, see <http:www.gnu.org/licenses/>.  * 
 ************************************************************************/

#include "pragha.h"

static guint owner_id;
static GDBusNodeInfo *introspection_data = NULL;
static GDBusConnection *dbus_connection = NULL;
static GQuark interface_quarks[4];
static gboolean saved_playbackstatus, saved_shuffle;
static gchar *saved_title;
static enum player_state state;
static const gchar MPRIS_NAME[] = "org.mpris.MediaPlayer2.pragha";
static const gchar MPRIS_PATH[] = "/org/mpris/MediaPlayer2";
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
"                        <arg direction='out' name='Metadata' type='aa{sv}'>"
"                        </arg>"
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
"                        <arg name='Metadata' type='a{sv}'>"
"                        </arg>"
"                        <arg name='AfterTrack' type='o'/>"
"                </signal>"
"                <signal name='TrackRemoved'>"
"                        <arg name='TrackId' type='o'/>"
"                </signal>"
"                <signal name='TrackMetadataChanged'>"
"                        <arg name='TrackId' type='o'/>"
"                        <arg name='Metadata' type='a{sv}'>"
"                        </arg>"
"                </signal>"
"                <property name='Tracks' type='ao' access='read'/>"
"                <property name='CanEditTracks' type='b' access='read'/>"
"        </interface>"
"</node>";
GError **property_error; /* for returning errors in propget/propput */
GDBusMethodInvocation *method_invocation; /* for returning errors during methods */

/* some MFCisms */
#define BEGIN_INTERFACE(x) \
	if(g_quark_try_string(interface_name)==interface_quarks[x]) {
#define MAP_METHOD(x,y) \
	if(!g_strcmp0(#y, method_name)) { \
		g_dbus_method_invocation_return_value (invocation, mpris_##x##_##y(cwin, parameters)); return; }
#define PROPGET(x,y) \
	if(!g_strcmp0(#y, property_name)) \
		return mpris_##x##_get_##y(cwin);
#define PROPPUT(x,y) \
	if(g_quark_try_string(property_name)==g_quark_from_static_string(#y)) \
		mpris_##x##_put_##y(cwin, value);
#define END_INTERFACE }	

/* org.mpris.MediaPlayer2 */
static GVariant* mpris_Root_Raise(struct con_win *cwin, GVariant* parameters) { 
	gtk_window_present(GTK_WINDOW(cwin->mainwindow));
	return NULL; 
}
static GVariant* mpris_Root_Quit(struct con_win *cwin, GVariant* parameters) { 
	exit_pragha(NULL, cwin);
	return NULL; 
}
static GVariant* mpris_Root_get_CanQuit(struct con_win *cwin) { 
	return g_variant_new_boolean(TRUE); 
}
static GVariant* mpris_Root_get_CanRaise(struct con_win *cwin) { 
	return g_variant_new_boolean(TRUE); 
}
static GVariant* mpris_Root_get_HasTrackList(struct con_win *cwin) { 
	return g_variant_new_boolean(TRUE);
}
static GVariant* mpris_Root_get_Identity(struct con_win *cwin) { 	

	return g_variant_new_string("Pragha Music Player"); 
}
static GVariant* mpris_Root_get_DesktopEntry(struct con_win *cwin) { 
	GVariant* ret_val = g_variant_new_string(DESKTOPENTRY); 
	return ret_val;
}
static GVariant* mpris_Root_get_SupportedUriSchemes(struct con_win *cwin) { 
	return g_variant_parse(G_VARIANT_TYPE("as"), 
		"['file', 'cdda']", NULL, NULL, NULL);
}
static GVariant* mpris_Root_get_SupportedMimeTypes(struct con_win *cwin) { 
	return g_variant_parse(G_VARIANT_TYPE("as"), 
		"['audio/x-mp3', 'audio/mpeg', 'audio/x-mpeg', 'audio/mpeg3', "
		"'audio/mp3', 'application/ogg', 'application/x-ogg', 'audio/vorbis', "
		"'audio/x-vorbis', 'audio/ogg', 'audio/x-ogg', 'audio/x-flac', "
		"'application/x-flac', 'audio/flac', 'audio/x-wav']", NULL, NULL, NULL);
}

/* org.mpris.MediaPlayer2.Player */
static GVariant* mpris_Player_Play(struct con_win *cwin, GVariant* parameters) {
	play_track(cwin);
	return NULL; 
}
static GVariant* mpris_Player_Next(struct con_win *cwin, GVariant* parameters) {
	play_next_track(cwin); 
	return NULL; 
}
static GVariant* mpris_Player_Previous(struct con_win *cwin, GVariant* parameters) {
	play_prev_track(cwin); 
	return NULL; 
}
static GVariant* mpris_Player_Pause(struct con_win *cwin, GVariant* parameters) {
	backend_pause(cwin); 
	return NULL; 
}
static GVariant* mpris_Player_PlayPause(struct con_win *cwin, GVariant* parameters) {
	play_pause_resume(cwin);
	return NULL; 
}
static GVariant* mpris_Player_Stop(struct con_win *cwin, GVariant* parameters) {
	backend_stop(cwin); 
	return NULL; 
}
static GVariant* mpris_Player_Seek(struct con_win *cwin, GVariant* parameters) {
	/*gdouble fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar));
	gint seek = cwin->cstate->curr_mobj->tags->length * fraction;
	gint64 param;
	g_variant_get(parameters, "(x)", &param);
	seek += (param / 1000000);
	fraction += (gdouble)seek / (gdouble)cwin->cstate->curr_mobj->tags->length;
	seek_playback(cwin, seek, fraction);*/
	return NULL;
}
static GVariant* mpris_Player_SetPosition(struct con_win *cwin, GVariant* parameters) {
	/*gint64 param;
	gchar *path = NULL;
	g_variant_get(parameters, "(sx)", &path, &param);
	if(!g_strcmp0(cwin->cstate->curr_mobj->file, path)) {
		gdouble fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(cwin->track_progress_bar));
		gint seek = cwin->cstate->curr_mobj->tags->length * fraction;
		seek += (param / 1000000);
		fraction += (gdouble)seek / (gdouble)cwin->cstate->curr_mobj->tags->length;
		seek_playback(cwin, seek, fraction);
	}
	g_free(path);*/
	return NULL;
}
static GVariant* mpris_Player_OpenUri(struct con_win *cwin, GVariant* parameters) {
	gchar *uri = NULL;
	g_variant_get(parameters, "(s)", &uri);
	if(uri) {
		gchar *path = g_filename_from_uri(uri, NULL, NULL);
		if(path && g_file_test(path, G_FILE_TEST_EXISTS)) {
			// TODO: find in current playlist & jump or append and jump.
			play_next_track(cwin);
		}
		g_free(path);
	}
	g_free(uri);
	return NULL;
}
static GVariant* mpris_Player_get_PlaybackStatus(struct con_win *cwin) { 
	switch (cwin->cstate->state) {
	case ST_PLAYING:	return g_variant_new_string("Playing");
	case ST_PAUSED:		return g_variant_new_string("Paused");
	default:		return g_variant_new_string("Stopped");
	}
}
static GVariant* mpris_Player_get_LoopStatus(struct con_win *cwin) { 
	return g_variant_new_string(cwin->cpref->repeat ? "Playlist" : "None");
}
static void mpris_Player_put_LoopStatus(struct con_win *cwin, GVariant *value) {
	const gchar *new_loop = g_variant_get_string(value, NULL); 
	cwin->cpref->repeat = g_strcmp0("Playlist", new_loop) ? FALSE : TRUE;
}
static GVariant* mpris_Player_get_Rate(struct con_win *cwin) { 
	return g_variant_new_double(1.0);
}
static void mpris_Player_put_Rate(struct con_win *cwin, GVariant *value) { 
	g_dbus_method_invocation_return_dbus_error(method_invocation, 
		DBUS_ERROR_NOT_SUPPORTED, "This is not alsaplayer.");
}
static GVariant* mpris_Player_get_Shuffle(struct con_win *cwin) {
	return g_variant_new_boolean(cwin->cpref->shuffle);
}
static void mpris_Player_put_Shuffle(struct con_win *cwin, GVariant *value) { 
	cwin->cpref->shuffle = g_variant_get_boolean(value);
}
static GVariant* mpris_Player_get_Metadata(struct con_win *cwin) { 
	GVariantBuilder *b = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
	if (cwin->cstate->state != ST_STOPPED) {
		gchar *genres = g_strdup_printf("['%s']", cwin->cstate->curr_mobj->tags->genre);
		gchar *date = g_strdup_printf("%d", cwin->cstate->curr_mobj->tags->year);
		gchar *comments = g_strdup_printf("['%s']", cwin->cstate->curr_mobj->tags->comment);
		gchar *url = g_str_has_prefix(cwin->cstate->curr_mobj->file, "cdda") ? 
			g_strdup(cwin->cstate->curr_mobj->file) : 
			g_filename_to_uri(cwin->cstate->curr_mobj->file, NULL, NULL);
		g_variant_builder_add (b, "{sv}", "mpris:trackid", 
			g_variant_new_string(cwin->cstate->curr_mobj->file));
		g_variant_builder_add (b, "{sv}", "xesam:url", 
			g_variant_new_string(url));
		g_variant_builder_add (b, "{sv}", "xesam:title", 
			g_variant_new_string(cwin->cstate->curr_mobj->tags->title));
		g_variant_builder_add (b, "{sv}", "xesam:artist", 
			g_variant_new_string(cwin->cstate->curr_mobj->tags->artist));
		g_variant_builder_add (b, "{sv}", "xesam:album", 
			g_variant_new_string(cwin->cstate->curr_mobj->tags->album));
		g_variant_builder_add (b, "{sv}", "xesam:genre", 
			g_variant_parse(G_VARIANT_TYPE("as"), genres, NULL, NULL, NULL));
		g_variant_builder_add (b, "{sv}", "xesam:contentCreated", 
			g_variant_new_string (date));
		g_variant_builder_add (b, "{sv}", "xesam:trackNumber", 
			g_variant_new_int32(cwin->cstate->curr_mobj->tags->track_no));
		g_variant_builder_add (b, "{sv}", "xesam:comment", 
			g_variant_new_string(comments));
		g_variant_builder_add (b, "{sv}", "mpris:length", 
			g_variant_new_int64(cwin->cstate->curr_mobj->tags->length * 1000000l));
		g_variant_builder_add (b, "{sv}", "audio-bitrate", 
			g_variant_new_int32(cwin->cstate->curr_mobj->tags->bitrate));
		g_variant_builder_add (b, "{sv}", "audio-channels", 
			g_variant_new_int32(cwin->cstate->curr_mobj->tags->channels));
		g_variant_builder_add (b, "{sv}", "audio-samplerate", 
			g_variant_new_int32(cwin->cstate->curr_mobj->tags->samplerate));
		g_free(genres);
		g_free(date);
		g_free(comments);
		g_free(url);
	} else {

		g_variant_builder_add (b, "{sv}", "mpris:trackid", 
			g_variant_new_string("(Unknown)"));
	}
	return g_variant_builder_end(b); 
}
static GVariant* mpris_Player_get_Volume(struct con_win *cwin) { 
	return g_variant_new_double(cwin->cgst->curr_vol);
}
static void mpris_Player_put_Volume(struct con_win *cwin, GVariant *value) { 
	gdouble volume = g_variant_get_double(value);
	backend_set_volume(volume, cwin);
}
static GVariant* mpris_Player_get_Position(struct con_win *cwin) { 
	if (cwin->cstate->state == ST_STOPPED)
		return g_variant_new_int64(0);
	else
		return g_variant_new_int64(backend_get_current_position(cwin));
}
static GVariant* mpris_Player_get_MinimumRate(struct con_win *cwin) { 
	return g_variant_new_double(1.0);
}
static GVariant* mpris_Player_get_MaximumRate(struct con_win *cwin) { 
	return g_variant_new_double(1.0);
}
static GVariant* mpris_Player_get_CanGoNext(struct con_win *cwin) { 
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}
static GVariant* mpris_Player_get_CanGoPrevious(struct con_win *cwin) { 
	// do we need to go into such detail?
	return g_variant_new_boolean(TRUE);
}
static GVariant* mpris_Player_get_CanPlay(struct con_win *cwin) { 
	return g_variant_new_boolean(NULL != cwin->cstate->curr_mobj);
}
static GVariant* mpris_Player_get_CanPause(struct con_win *cwin) { 
	return g_variant_new_boolean(NULL != cwin->cstate->curr_mobj);
}
static GVariant* mpris_Player_get_CanSeek(struct con_win *cwin) { 
	return g_variant_new_boolean(TRUE);
}
static GVariant* mpris_Player_get_CanControl(struct con_win *cwin) { 
	// always?
	return g_variant_new_boolean(TRUE);
}

/* org.mpris.MediaPlayer2.Playlists */
static GVariant* mpris_Playlists_ActivatePlaylist(struct con_win *cwin, GVariant* parameters) { 
	gchar* playlist = NULL;
	g_variant_get(parameters, "(o)", &playlist);
	if(playlist && g_str_has_prefix(playlist, MPRIS_PATH)) {
		gint i = 0;
		gchar **playlists = get_playlist_names_db(cwin);
		while(playlists[i]) {
			gchar *list = g_strdup_printf("%s/Playlists/%d", MPRIS_PATH, i);
			if(!g_strcmp0(list, playlist)) {
				backend_stop(cwin);
				clear_current_playlist(NULL, cwin);
				add_playlist_current_playlist(playlists[i], cwin);
				play_track(cwin);
				break;
			}
			g_free(list);
			i++;
		}
		g_free(playlists);
	}
	g_free(playlist);
	return NULL; 
}
static GVariant* mpris_Playlists_GetPlaylists(struct con_win *cwin, GVariant* parameters) { 
	GVariantBuilder *builder;
	GVariant *value;
	guint start, max;
	gchar *order;
	gboolean reverse;
	g_variant_get(parameters, "(uusb)", &start, &max, &order, &reverse);
	gchar ** lists = get_playlist_names_db(cwin);
	builder = g_variant_builder_new (G_VARIANT_TYPE ("a(oss)"));
	gint i = 0;
	gint imax = max;
	while(lists[i]) {
		if(i >= start && imax > 0) {
			gchar *listpath = g_strdup_printf("%s/Playlists/%d", MPRIS_PATH, i);
			g_variant_builder_add (builder, "(oss)", listpath, lists[i], "");
			g_free(listpath);
			imax--;
		}
		i++;
	}
	g_free(lists);
	value = g_variant_new ("a(oss)", builder);
	g_variant_builder_unref (builder);
	return g_variant_new_tuple(&value, 1); 
}
static GVariant* mpris_Playlists_get_ActivePlaylist(struct con_win *cwin) { 
	return g_variant_new("(b(oss))", 
		FALSE, MPRIS_PATH, "invalid", "invalid");
}
static GVariant* mpris_Playlists_get_Orderings(struct con_win *cwin) {
	return g_variant_parse(G_VARIANT_TYPE("as"), 
		"['UserDefined']", NULL, NULL, NULL);
}
static GVariant* mpris_Playlists_get_PlaylistCount(struct con_win *cwin) { 
	return g_variant_new_uint32(get_playlist_count_db(cwin)); 
}

/* org.mpris.MediaPlayer2.TrackList */
static GVariant* mpris_TrackList_GetTracksMetadata(struct con_win *cwin, GVariant* parameters) { 
	return NULL; 
}
static GVariant* mpris_TrackList_AddTrack(struct con_win *cwin, GVariant* parameters) {
	g_dbus_method_invocation_return_dbus_error(method_invocation, 
		DBUS_ERROR_NOT_SUPPORTED, "TrackList is read-only.");
	return NULL; 
}
static GVariant* mpris_TrackList_RemoveTrack(struct con_win *cwin, GVariant* parameters) { 
	g_dbus_method_invocation_return_dbus_error(method_invocation, 
		DBUS_ERROR_NOT_SUPPORTED, "TrackList is read-only.");
	return NULL; 
}
static GVariant* mpris_TrackList_GoTo(struct con_win *cwin, GVariant* parameters) { 
	return NULL; 
}
static GVariant* mpris_TrackList_get_Tracks(struct con_win *cwin) { 
	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("ao"));
	gint numTracks = get_playlist_count_db(cwin);
	gchar o[260];
	gint i;
	for(i = 0; i < numTracks; i++) {
		g_snprintf(o, 260, "%s/TrackList/%d", MPRIS_PATH, i);
		g_variant_builder_add(builder, "o", o);
	}
	return g_variant_builder_end(builder); 
}
static GVariant* mpris_TrackList_get_CanEditTracks(struct con_win *cwin) { 
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
	method_invocation = invocation;
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
	method_invocation = NULL;
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
	property_error = error;
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
	property_error = NULL;
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
	property_error = error;
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
	property_error = NULL;
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
	for(i = 0; i < 4; i++)
	{
		interface_quarks[i] = g_quark_from_string(introspection_data->interfaces[i]->name);
		registration_id = g_dbus_connection_register_object (connection,
									MPRIS_PATH,
									introspection_data->interfaces[i],
									&interface_vtable,
									user_data,  /* user_data */
									NULL,  /* user_data_free_func */
									NULL); /* GError** */
		g_assert (registration_id > 0);
	}
	
	dbus_connection = connection;
	g_object_ref(G_OBJECT(dbus_connection));
	
	saved_shuffle = false;
	saved_playbackstatus = false;
	saved_title = NULL;
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
	if(NULL != dbus_connection)
		g_object_unref(G_OBJECT(dbus_connection));
	/*TODO: Handle emergency*/
	g_warning("Lost DBus name %s", name);

}

/* pragha calls */
void mpris_update_any(struct con_win *cwin) {
	g_warning("Lost DBus name");
	gboolean change_detected = FALSE;
	GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	gchar *newtitle = cwin->cstate->curr_mobj->file;
	g_warning("Lost DBus name");
	if(saved_shuffle != cwin->cpref->shuffle)
	{
		change_detected = TRUE;
		saved_shuffle = cwin->cpref->shuffle;
		g_variant_builder_add (b, "{sv}", "Shuffle", mpris_Player_get_Shuffle(cwin));
	}
	if(state != cwin->cstate->state)
	{
		change_detected = TRUE;
		state = cwin->cstate->state;
		g_variant_builder_add (b, "{sv}", "PlaybackStatus", mpris_Player_get_PlaybackStatus(cwin));
	}
	if(saved_playbackstatus != cwin->cpref->repeat)
	{
		change_detected = TRUE;
		saved_playbackstatus = cwin->cpref->repeat;
		g_variant_builder_add (b, "{sv}", "LoopStatus", mpris_Player_get_LoopStatus(cwin));
	}
	if(g_strcmp0(saved_title, newtitle))
	{
		change_detected = TRUE;
		if(saved_title)
			g_free(saved_title);
		if(newtitle)
			saved_title = g_strdup(newtitle);
		g_variant_builder_add (b, "{sv}", "Metadata", mpris_Player_get_Metadata(cwin));
	}
	if(change_detected)
	{
		GVariant * tuples[] = {
			g_variant_builder_end(b),
			g_variant_parse(G_VARIANT_TYPE("as"), "[]", NULL, NULL, NULL)
		};
		if(g_variant_is_floating(tuples[0]))
			g_variant_ref_sink(tuples[0]);
		if(g_variant_is_floating(tuples[1]))
			g_variant_ref_sink(tuples[1]);
		g_dbus_connection_emit_signal(dbus_connection, NULL, MPRIS_PATH, 
			"org.freedesktop.DBus.Properties", "PropertiesChanged",
			g_variant_new_tuple(tuples, 2) , NULL);
	}
	g_warning("Lost DBus name");
}

gint mpris_init(struct con_win *cwin)
{
    CDEBUG(DBG_INFO, "Initializing MPRIS");

    g_type_init();
	
    introspection_data = g_dbus_node_info_new_for_xml (mpris2xml, NULL);
    g_assert (introspection_data != NULL);
	
    owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                             MPRIS_NAME,
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             on_bus_acquired,
                             on_name_acquired,
                             on_name_lost,
                             cwin,
                             NULL);
    return (owner_id) ? 0 : -1;
}

void mpris_cleanup()
{
  g_bus_unown_name(owner_id);

  g_dbus_node_info_unref(introspection_data);
  if(NULL != dbus_connection)
	g_object_unref(G_OBJECT(dbus_connection));
}
