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

#include "pragha-musicobject-mgmt.h"

#include <glib.h>
#include <glib/gstdio.h>

#include "pragha-file-utils.h"
#include "pragha-playlists-mgmt.h"
#include "pragha-tags-mgmt.h"

PraghaMusicobject *
new_musicobject_from_file(const gchar *file, const gchar *provider)
{
	PraghaMusicobject *mobj = NULL;
	gchar *mime_type = NULL;
	gboolean ret = FALSE;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from file: %s", file);

	mime_type = pragha_file_get_music_type(file);

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", file,
	                     "source", FILE_LOCAL,
	                     "provider", provider,
	                     "mime-type", mime_type,
	                     NULL);

	g_free (mime_type);

	ret = pragha_musicobject_set_tags_from_file (mobj, file);

	if (G_LIKELY(ret))
		return mobj;
	else {
		g_critical("Fail to create musicobject from file");
		g_object_unref(mobj);
	}

	return NULL;
}

PraghaMusicobject *
new_musicobject_from_db(PraghaDatabase *cdbase, gint location_id)
{
	PraghaMusicobject *mobj = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject with location id: %d",
	       location_id);

	const gchar *sql = "SELECT \
MIME_TYPE.name, \
TRACK.samplerate, \
TRACK.channels, \
TRACK.length, \
TRACK.bitrate, \
COMMENT.name, \
YEAR.year, \
TRACK.track_no, \
GENRE.name, \
ALBUM.name, \
ARTIST.name, \
TRACK.title, \
LOCATION.name \
FROM MIME_TYPE, TRACK, COMMENT, YEAR, GENRE, ALBUM, ARTIST, LOCATION \
WHERE TRACK.location = ? \
AND MIME_TYPE.id = TRACK.file_type \
AND COMMENT.id = TRACK.comment \
AND YEAR.id = TRACK.year \
AND GENRE.id = TRACK.genre \
AND ALBUM.id = TRACK.album \
AND ARTIST.id = TRACK.artist \
AND LOCATION.id = ?";

	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_int (statement, 1, location_id);
	pragha_prepared_statement_bind_int (statement, 2, location_id);

	if (pragha_prepared_statement_step (statement)) {
		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", pragha_prepared_statement_get_string (statement, 12),
		                     "source", FILE_LOCAL,
		                     "mime-type", pragha_prepared_statement_get_string (statement, 0),
		                     "title", pragha_prepared_statement_get_string (statement, 11),
		                     "artist", pragha_prepared_statement_get_string (statement, 10),
		                     "album", pragha_prepared_statement_get_string (statement, 9),
		                     "genre", pragha_prepared_statement_get_string (statement, 8),
		                     "comment", pragha_prepared_statement_get_string (statement, 5),
		                     "year", pragha_prepared_statement_get_int (statement, 6),
		                     "track-no", pragha_prepared_statement_get_int (statement, 7),
		                     "length", pragha_prepared_statement_get_int (statement, 3),
		                     "bitrate", pragha_prepared_statement_get_int (statement, 4),
		                     "channels", pragha_prepared_statement_get_int (statement, 2),
		                     "samplerate", pragha_prepared_statement_get_int (statement, 1),
		                     NULL);
	}
	else {
		g_critical("Track with location id : %d not found in DB",
			   location_id);
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
