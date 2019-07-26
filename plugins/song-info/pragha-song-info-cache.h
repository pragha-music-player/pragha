/*************************************************************************/
/* Copyright (C) 2018-2019 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_INFO_CACHE_H
#define PRAGHA_INFO_CACHE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_INFO_CACHE (pragha_info_cache_get_type())
#define PRAGHA_INFO_CACHE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_INFO_CACHE, PraghaInfoCache))
#define PRAGHA_INFO_CACHE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_INFO_CACHE, PraghaInfoCache const))
#define PRAGHA_INFO_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_INFO_CACHE, PraghaInfoCacheClass))
#define PRAGHA_IS_INFO_CACHE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_INFO_CACHE))
#define PRAGHA_IS_INFO_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_INFO_CACHE))
#define PRAGHA_INFO_CACHE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_INFO_CACHE, PraghaInfoCacheClass))

typedef struct _PraghaInfoCache PraghaInfoCache;
typedef struct _PraghaInfoCacheClass PraghaInfoCacheClass;

struct _PraghaInfoCacheClass
{
	GObjectClass parent_class;
	void (*cache_changed)    (PraghaInfoCache *cache);
};

PraghaInfoCache *
pragha_info_cache_get                (void);

gboolean
pragha_info_cache_contains_similar_songs (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist);


GList *
pragha_info_cache_get_similar_songs      (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist,
                                          gchar          **provider);


void
pragha_info_cache_save_similar_songs     (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist,
                                          const gchar     *provider,
                                          GList           *mlist);

gboolean
pragha_info_cache_contains_song_lyrics   (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist);

void
pragha_info_cache_save_song_lyrics       (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist,
                                          const gchar     *provider,
                                          const gchar     *lyrics);

gchar *
pragha_info_cache_get_song_lyrics        (PraghaInfoCache *cache,
                                          const gchar     *title,
                                          const gchar     *artist,
                                          gchar          **provider);
gboolean
pragha_info_cache_contains_artist_bio    (PraghaInfoCache *cache,
                                          const gchar     *artist);

gboolean
pragha_info_cache_contains_ini_artist_bio(PraghaInfoCache *cache,
                                          const gchar     *artist);

gchar *
pragha_info_cache_get_artist_bio         (PraghaInfoCache *cache,
                                          const gchar     *artist,
                                          gchar          **provider);

void
pragha_info_cache_save_artist_bio        (PraghaInfoCache *cache,
                                          const gchar     *artist,
                                          const gchar     *provider,
                                          const gchar     *bio);

G_END_DECLS

#endif /* PRAGHA_INFO_CACHE_H */
