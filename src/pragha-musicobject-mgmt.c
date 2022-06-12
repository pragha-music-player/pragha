/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2018 matias <mati86dl@gmail.com>                   */
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

#include "pragha-musicobject-mgmt.h"

#include <glib.h>
#include <glib/gstdio.h>

#include "pragha-file-utils.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-tags-mgmt.h"
#include "pragha-music-enum.h"

PraghaMusicobject *
new_musicobject_from_file(const gchar *file, const gchar *provider)
{
	PraghaMusicobject *mobj = NULL;
	gchar *mime_type = NULL;

	if (pragha_file_get_media_type(file) != MEDIA_TYPE_AUDIO)
		return NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from file: %s", file);

	mime_type = pragha_file_get_music_type(file);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", file,
	                     "source", FILE_LOCAL,
	                     "provider", provider,
	                     "mime-type", mime_type,
	                     NULL);

	g_free (mime_type);

	pragha_musicobject_set_tags_from_file (mobj, file);

	return mobj;
}

PraghaMusicobject *
new_musicobject_from_db(PraghaDatabase *cdbase, gint location_id)
{
	PraghaPreparedStatement *statement = NULL;
	PraghaMusicEnum *enum_map = NULL;
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject with location id: %d", location_id);

	const gchar *sql =
		"SELECT LOCATION.name, PROVIDER_TYPE.name, PROVIDER.name, MIME_TYPE.name, TRACK.title, ARTIST.name, ALBUM.name, GENRE.name, COMMENT.name, YEAR.year, TRACK.track_no, TRACK.length, TRACK.bitrate, TRACK.channels, TRACK.samplerate \
		 FROM LOCATION, PROVIDER_TYPE, PROVIDER, MIME_TYPE, TRACK, ARTIST, ALBUM, GENRE, COMMENT, YEAR \
		 WHERE TRACK.location = ? AND PROVIDER.id = TRACK.provider AND PROVIDER_TYPE.id = PROVIDER.type AND MIME_TYPE.id = TRACK.file_type AND ARTIST.id = TRACK.artist AND ALBUM.id = TRACK.album AND GENRE.id = TRACK.genre AND COMMENT.id = TRACK.comment AND YEAR.id = TRACK.year \
		 AND LOCATION.id = ?";

	statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	pragha_prepared_statement_bind_int (statement, 2, location_id);

	if (pragha_prepared_statement_step (statement))
	{
		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", pragha_prepared_statement_get_string (statement, 0),
		                     "provider", pragha_prepared_statement_get_string (statement, 2),
		                     "mime-type", pragha_prepared_statement_get_string (statement, 3),
		                     "title", pragha_prepared_statement_get_string (statement, 4),
		                     "artist", pragha_prepared_statement_get_string (statement, 5),
		                     "album", pragha_prepared_statement_get_string (statement, 6),
		                     "genre", pragha_prepared_statement_get_string (statement, 7),
		                     "comment", pragha_prepared_statement_get_string (statement, 8),
		                     "year", pragha_prepared_statement_get_int (statement, 9),
		                     "track-no", pragha_prepared_statement_get_int (statement, 10),
		                     "length", pragha_prepared_statement_get_int (statement, 11),
		                     "bitrate", pragha_prepared_statement_get_int (statement, 12),
		                     "channels", pragha_prepared_statement_get_int (statement, 13),
		                     "samplerate", pragha_prepared_statement_get_int (statement, 14),
		                     NULL);

		enum_map = pragha_music_enum_get ();
		pragha_musicobject_set_source (mobj,
			pragha_music_enum_map_get(enum_map,
				pragha_prepared_statement_get_string (statement, 1)));
		g_object_unref (enum_map);
	}
	else
	{
		g_critical("Track with location id : %d not found in DB", location_id);
	}

	pragha_prepared_statement_free (statement);

	return mobj;
}

PraghaMusicobject *
new_musicobject_from_location(const gchar *uri, const gchar *name)
{
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject to location: %s", uri);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file",      uri,
	                     "source", FILE_HTTP,
	                     NULL);
	if (name)
		pragha_musicobject_set_title(mobj, name);

	return mobj;
}

void
pragha_update_musicobject_change_tag(PraghaMusicobject *mobj, gint changed, PraghaMusicobject *nmobj)
{
	if (!changed)
		return;

	CDEBUG(DBG_VERBOSE, "Tags Updates: 0x%x", changed);

	if (changed & TAG_TNO_CHANGED) {
		pragha_musicobject_set_track_no(mobj, pragha_musicobject_get_track_no(nmobj));
	}
	if (changed & TAG_TITLE_CHANGED) {
		pragha_musicobject_set_title(mobj, pragha_musicobject_get_title(nmobj));
	}
	if (changed & TAG_ARTIST_CHANGED) {
		pragha_musicobject_set_artist (mobj, pragha_musicobject_get_artist(nmobj));
	}
	if (changed & TAG_ALBUM_CHANGED) {
		pragha_musicobject_set_album(mobj, pragha_musicobject_get_album(nmobj));
	}
	if (changed & TAG_GENRE_CHANGED) {
		pragha_musicobject_set_genre(mobj, pragha_musicobject_get_genre(nmobj));
	}
	if (changed & TAG_YEAR_CHANGED) {
		pragha_musicobject_set_year(mobj, pragha_musicobject_get_year(nmobj));
	}
	if (changed & TAG_COMMENT_CHANGED) {
		pragha_musicobject_set_comment(mobj, pragha_musicobject_get_comment(nmobj));
	}
}

PraghaMusicobject *
pragha_database_get_artist_and_title_song (PraghaDatabase *cdbase,
                                           const gchar    *artist,
                                           const gchar    *title)
{
	PraghaMusicobject *mobj = NULL;
	gint location_id = 0;

	const gchar *sql =
		"SELECT LOCATION.id "
		"FROM TRACK, ARTIST, PROVIDER, LOCATION "
		"WHERE ARTIST.id = TRACK.artist "
		"AND LOCATION.id = TRACK.location "
		"AND TRACK.provider = PROVIDER.id AND PROVIDER.visible <> 0 "
		"AND TRACK.title = ? COLLATE NOCASE "
		"AND ARTIST.name = ? COLLATE NOCASE "
		"ORDER BY RANDOM() LIMIT 1;";

	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, title);
	pragha_prepared_statement_bind_string (statement, 2, artist);

	if (pragha_prepared_statement_step (statement)) {
		location_id = pragha_prepared_statement_get_int (statement, 0);
		mobj = new_musicobject_from_db (cdbase, location_id);
	}

	pragha_prepared_statement_free (statement);

	return mobj;
}
