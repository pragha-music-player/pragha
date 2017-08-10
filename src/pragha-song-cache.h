/*************************************************************************/
/* Copyright (C) 2017 matias <mati86dl@gmail.com>                        */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
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

#ifndef PRAGHA_SONG_CACHE_H
#define PRAGHA_SONG_CACHE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_SONG_CACHE (pragha_song_cache_get_type())
#define PRAGHA_SONG_CACHE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SONG_CACHE, PraghaSongCache))
#define PRAGHA_SONG_CACHE_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SONG_CACHE, PraghaSongCache const))
#define PRAGHA_SONG_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_SONG_CACHE, PraghaSongCacheClass))
#define PRAGHA_IS_SONG_CACHE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SONG_CACHE))
#define PRAGHA_IS_SONG_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_SONG_CACHE))
#define PRAGHA_SONG_CACHE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_SONG_CACHE, PraghaSongCacheClass))

typedef struct _PraghaSongCache PraghaSongCache;
typedef struct _PraghaSongCacheClass PraghaSongCacheClass;

struct _PraghaSongCacheClass
{
	GObjectClass parent_class;
};

PraghaSongCache *pragha_song_cache_get                (void);

void             pragha_song_cache_put_location      (PraghaSongCache *cache, const gchar *location, const gchar *filename);
gchar           *pragha_song_cache_get_from_location (PraghaSongCache *cache, const gchar *location);


G_END_DECLS

#endif /* PRAGHA_SONG_CACHE_H */
