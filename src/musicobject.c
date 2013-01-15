/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
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

PraghaMusicobject *
new_musicobject_from_file(const gchar *file)
{
	PraghaMusicobject *mobj;
	enum file_type type;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from file: %s", file);

	type = get_file_type(file);
	if (type == -1)
		return NULL;

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", file,
	                     "file-type", type,
	                     NULL);

	if(pragha_musicobject_set_tags_from_file(mobj, file))
		return mobj;
	else
		g_object_unref(mobj);

	return NULL;
}

PraghaMusicobject *
new_musicobject_from_db(PraghaDatabase *cdbase, gint location_id)
{
	PraghaMusicobject *mobj;
	gchar *query;
	PraghaDbResponse result;
	gint i = 0;

	CDEBUG(DBG_MOBJ, "Creating new musicobject with location id: %d",
	       location_id);

	query = g_strdup_printf("SELECT \
TRACK.file_type, \
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
FROM TRACK, COMMENT, YEAR, GENRE, ALBUM, ARTIST, LOCATION \
WHERE TRACK.location = \"%d\" \
AND COMMENT.id = TRACK.comment \
AND YEAR.id = TRACK.year \
AND GENRE.id = TRACK.genre \
AND ALBUM.id = TRACK.album \
AND ARTIST.id = TRACK.artist \
AND LOCATION.id = \"%d\";", location_id, location_id);
	if (!pragha_database_exec_sqlite_query(cdbase, query, &result)) {
		g_critical("Track with location id : %d not found in DB",
			   location_id);
		return NULL;
	}
	else {
		i = result.no_columns;
		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", result.resultp[i+12],
		                     "file-type", atoi(result.resultp[i]),
		                     "title", result.resultp[i+11],
		                     "artist", result.resultp[i+10],
		                     "album", result.resultp[i+9],
		                     "genre", result.resultp[i+8],
		                     "comment", result.resultp[i+5],
		                     "year", atoi(result.resultp[i+6]),
		                     "track-no", atoi(result.resultp[i+7]),
		                     "length", atoi(result.resultp[i+3]),
		                     "bitrate", atoi(result.resultp[i+4]),
		                     "channels", atoi(result.resultp[i+2]),
		                     "samplerate", atoi(result.resultp[i+1]),
		                     NULL);

		sqlite3_free_table(result.resultp);

		return mobj;
	}
}

PraghaMusicobject *
new_musicobject_from_cdda(struct con_win *cwin,
                          gint track_no)
{
	PraghaMusicobject *mobj;
	gint channels, start, end;
	gchar *ntitle = NULL, *nfile = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from cdda: %d",
	       track_no);

	channels = cdio_get_track_channels(cwin->cstate->cdda_drive->p_cdio,
					   track_no);
	start = cdio_cddap_track_firstsector(cwin->cstate->cdda_drive, track_no);
	end = cdio_cddap_track_lastsector(cwin->cstate->cdda_drive, track_no);

	if (cwin->cpref->use_cddb && cwin->cstate->cddb_disc) {
		cddb_track_t *track;
		const gchar *title, *artist, *album, *genre;

		track = cddb_disc_get_track(cwin->cstate->cddb_disc, track_no - 1);
		if (track) {
			title = cddb_track_get_title(track);
			artist = cddb_track_get_artist(track);

			if (title)
				ntitle = g_strdup(title);
			else
				ntitle = g_strdup_printf("Track %d", track_no);
		}

		genre = cddb_disc_get_genre(cwin->cstate->cddb_disc);
		album = cddb_disc_get_title(cwin->cstate->cddb_disc);

		nfile = g_strdup_printf("cdda://%d", track_no);

		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
			             "file", nfile,
			             "file-type", FILE_CDDA,
			             "title", ntitle,
			             "artist", artist,
			             "album", album,
			             "genre", genre,
			             "year", cddb_disc_get_year(cwin->cstate->cddb_disc),
			             "track-no", track_no,
			             "length", (end - start) / CDIO_CD_FRAMES_PER_SEC,
			             "channels", (channels > 0) ? channels : 0,
			             NULL);
		g_free(nfile);
		g_free(ntitle);
	}

	return mobj;
}

PraghaMusicobject *
new_musicobject_from_location(const gchar *uri, const gchar *name)
{
	PraghaMusicobject *mobj;
	gchar *file = NULL;

	CDEBUG(DBG_MOBJ, "Creating new musicobject to location: %s", uri);

#ifdef HAVE_PLPARSER
	GSList *list = pragha_totem_pl_parser_parse_from_uri(uri);
	if(list) {
		file = g_strdup(list->data);
		g_slist_free_full(list, g_free);
	}
	else {
		file = g_strdup(uri);
	}
#else
	file = g_strdup(uri);
#endif

	mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", file,
	                     "file-type", FILE_HTTP,
	                     "title", name,
	                     NULL);

	g_free(file);

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
