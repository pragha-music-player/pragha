/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>			 */
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

#ifndef PRAGHA_MUSICOBJECT_H
#define PRAGHA_MUSICOBJECT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_MUSICOBJECT (pragha_musicobject_get_type())
#define PRAGHA_MUSICOBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_MUSICOBJECT, PraghaMusicobject))
#define PRAGHA_MUSICOBJECT_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_MUSICOBJECT, PraghaMusicobject const))
#define PRAGHA_MUSICOBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_MUSICOBJECT, PraghaMusicobjectClass))
#define PRAGHA_IS_MUSICOBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_MUSICOBJECT))
#define PRAGHA_IS_MUSICOBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_MUSICOBJECT))
#define PRAGHA_MUSICOBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_MUSICOBJECT, PraghaMusicobjectClass))

typedef struct _PraghaMusicobject PraghaMusicobject;
typedef struct _PraghaMusicobjectClass PraghaMusicobjectClass;
typedef struct _PraghaMusicobjectPrivate PraghaMusicobjectPrivate;

struct _PraghaMusicobject
{
   GObject parent;

   /*< private >*/
   PraghaMusicobjectPrivate *priv;
};

struct _PraghaMusicobjectClass
{
   GObjectClass parent_class;
};

#define PRAGHA_MUSICOBJECT_PARAM_STRING G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS

PraghaMusicobject *pragha_musicobject_new (void);
PraghaMusicobject *
pragha_musicobject_dup (PraghaMusicobject *musicobject);
void
pragha_musicobject_clean (PraghaMusicobject *musicobject);

GType pragha_musicobject_get_type (void) G_GNUC_CONST;

const gchar *
pragha_musicobject_get_file (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_file (PraghaMusicobject *musicobject,
                             const gchar *file);

gboolean
pragha_musicobject_is_local_file (PraghaMusicobject *musicobject);

gint
pragha_musicobject_get_file_type (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_file_type (PraghaMusicobject *musicobject,
                                  gint file_type);

const gchar *
pragha_musicobject_get_title (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_title (PraghaMusicobject *musicobject,
                              const gchar *title);

const gchar *
pragha_musicobject_get_artist (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_artist (PraghaMusicobject *musicobject,
                               const gchar *artist);

const gchar *
pragha_musicobject_get_album (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_album (PraghaMusicobject *musicobject,
                              const gchar *album);

const gchar *
pragha_musicobject_get_album_artist (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_album_artist (PraghaMusicobject *musicobject,
                                     const gchar *albu_artist);

const gchar *
pragha_musicobject_get_genre (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_genre (PraghaMusicobject *musicobject,
                              const gchar *genre);

const gchar *
pragha_musicobject_get_comment (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_comment (PraghaMusicobject *musicobject,
                                const gchar *comment);

gboolean
pragha_musicobject_is_compilation (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_compilation (PraghaMusicobject *musicobject,
                                    gboolean compilation);

guint
pragha_musicobject_get_year (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_year (PraghaMusicobject *musicobject,
                             guint year);

guint
pragha_musicobject_get_track_no (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_track_no (PraghaMusicobject *musicobject,
                                 guint track_no);

gint
pragha_musicobject_get_length (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_length (PraghaMusicobject *musicobject,
                               gint length);

gint
pragha_musicobject_get_bitrate (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_bitrate (PraghaMusicobject *musicobject,
                                gint bitrate);

gint
pragha_musicobject_get_channels (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_channels (PraghaMusicobject *musicobject,
                                 gint channels);

gint
pragha_musicobject_get_samplerate (PraghaMusicobject *musicobject);
void
pragha_musicobject_set_samplerate (PraghaMusicobject *musicobject,
                                   gint samplerate);
G_END_DECLS

#endif /* PRAGHA_MUSICOBJECT_H */
