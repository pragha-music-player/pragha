/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

struct musicobject* new_musicobject_from_file(gchar *file)
{
	enum file_type type;
	struct musicobject *mobj;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from file: %s", file);

	type = get_file_type(file);
	if (type == -1)
		return NULL;

	mobj = g_slice_new0(struct musicobject);
	mobj->tags = g_slice_new0(struct tags);
	mobj->file = g_strdup(file);

	switch(type) {
	case FILE_WAV:
		if (get_wav_info(file, mobj->tags))
			mobj->file_type = FILE_WAV;
		else {
			g_critical("WAV Info failed");
			goto bad;
		}
		break;
	case FILE_MP3:
		if (get_mp3_info(file, mobj->tags))
			mobj->file_type = FILE_MP3;
		else {
			g_critical("MP3 Info failed");
			goto bad;
		}
		break;
	case FILE_FLAC:
		if (get_flac_info(file, mobj->tags))
			mobj->file_type = FILE_FLAC;
		else {
			g_critical("FLAC Info failed");
			goto bad;
		}
		break;
	case FILE_OGGVORBIS:
		if (get_ogg_info(file, mobj->tags))
			mobj->file_type = FILE_OGGVORBIS;
		else {
			g_critical("OGG Info failed");
			goto bad;
		}
		break;
	case FILE_MODPLUG:
		if (get_mod_info(file, mobj->tags))
			mobj->file_type = FILE_MODPLUG;
		else {
			g_critical("MOD Info failed");
			goto bad;
		}
		break;
	default:
		break;
	}

	return mobj;

bad:
	g_free(mobj->file);
	g_slice_free(struct tags, mobj->tags);
	g_slice_free(struct musicobject, mobj);
	mobj = NULL;
	return NULL;
}

struct musicobject* new_musicobject_from_db(gint location_id, struct con_win *cwin)
{
	gchar *query;
	struct db_result result;
	struct musicobject *mobj = NULL;
	gint i = 0;

	CDEBUG(DBG_MOBJ, "Creating new musicobject with location id: %d",
	       location_id);

	query = g_strdup_printf("SELECT \
TRACK.file_type, \
TRACK.samplerate, \
TRACK.channels, \
TRACK.length, \
TRACK.bitrate, \
YEAR.year, \
TRACK.track_no, \
GENRE.name, \
ALBUM.name, \
ARTIST.name, \
TRACK.title, \
LOCATION.name \
FROM TRACK, YEAR, GENRE, ALBUM, ARTIST, LOCATION \
WHERE TRACK.location = \"%d\" \
AND YEAR.id = TRACK.year \
AND GENRE.id = TRACK.genre \
AND ALBUM.id = TRACK.album \
AND ARTIST.id = TRACK.artist \
AND LOCATION.id = \"%d\";", location_id, location_id);
	if (!exec_sqlite_query(query, cwin, &result)) {
		g_critical("Track with location id : %d not found in DB",
			   location_id);
		return NULL;
	}
	else {
		i = result.no_columns;
		mobj = g_slice_new0(struct musicobject);
		mobj->tags = g_slice_new0(struct tags);

		mobj->file = g_strdup(result.resultp[i+11]);
		mobj->tags->title = g_strdup(result.resultp[i+10]);
		mobj->tags->artist = g_strdup(result.resultp[i+9]);
		mobj->tags->album = g_strdup(result.resultp[i+8]);
		mobj->tags->genre = g_strdup(result.resultp[i+7]);
		mobj->tags->track_no = atoi(result.resultp[i+6]);
		mobj->tags->year = atoi(result.resultp[i+5]);
		mobj->tags->bitrate = atoi(result.resultp[i+4]);
		mobj->tags->length = atoi(result.resultp[i+3]);
		mobj->tags->channels = atoi(result.resultp[i+2]);
		mobj->tags->samplerate = atoi(result.resultp[i+1]);
		mobj->file_type = atoi(result.resultp[i]);

		sqlite3_free_table(result.resultp);

		return mobj;
	}
}

struct musicobject* new_musicobject_from_cdda(struct con_win *cwin,
					      gint track_no)
{
	gint channels, start, end;
	struct musicobject *mobj;

	CDEBUG(DBG_MOBJ, "Creating new musicobject from cdda: %d",
	       track_no);

	channels = cdio_get_track_channels(cwin->cstate->cdda_drive->p_cdio,
					   track_no);
	start = cdio_cddap_track_firstsector(cwin->cstate->cdda_drive, track_no);
	end = cdio_cddap_track_lastsector(cwin->cstate->cdda_drive, track_no);

	mobj = g_slice_new0(struct musicobject);
	mobj->tags = g_slice_new0(struct tags);

	if (cwin->cpref->use_cddb && cwin->cstate->cddb_disc) {
		cddb_track_t *track;
		const gchar *title, *artist, *album, *genre;

		track = cddb_disc_get_track(cwin->cstate->cddb_disc, track_no - 1);
		if (track) {
			title = cddb_track_get_title(track);
			artist = cddb_track_get_artist(track);

			if (title)
				mobj->tags->title = g_strdup(title);
			if (artist)
				mobj->tags->artist = g_strdup(artist);
		}

		genre = cddb_disc_get_genre(cwin->cstate->cddb_disc);
		album = cddb_disc_get_title(cwin->cstate->cddb_disc);

		if (genre)
			mobj->tags->genre = g_strdup(genre);
		if (album)
			mobj->tags->album = g_strdup(album);

		mobj->tags->year = cddb_disc_get_year(cwin->cstate->cddb_disc);
	}

	if (!mobj->tags->title)
		mobj->tags->title = g_strdup_printf("Track %d", track_no);
	mobj->tags->track_no = track_no;
	mobj->tags->channels = (channels > 0) ? channels : 0;
	mobj->tags->length = (end - start) / CDIO_CD_FRAMES_PER_SEC;
	mobj->file = g_strdup_printf("cdda://%d", track_no);
	mobj->file_type = FILE_CDDA;

	return mobj;
}

void delete_musicobject(struct musicobject *mobj)
{
	CDEBUG(DBG_MOBJ, "Freeing musicobject: %s", mobj->file);

	g_free(mobj->tags->title);
	g_free(mobj->tags->artist);
	g_free(mobj->tags->album);
	g_free(mobj->tags->genre);
	g_free(mobj->file);
	g_slice_free(struct tags, mobj->tags);
	g_slice_free(struct musicobject, mobj);
}

void test_delete_musicobject(struct musicobject *mobj, struct con_win *cwin)
{
	if (!mobj)
		return;

	CDEBUG(DBG_MOBJ, "Test freeing musicobject: %s", mobj->file);

	if (mobj == cwin->cstate->curr_mobj)
		cwin->cstate->curr_mobj_clear = TRUE;
	else
		delete_musicobject(mobj);
}
