/*
 * Copyright (C) 2009-2012 matias <mati86dl@gmail.com>
 * Copyright (C) 2012 Pavel Vasin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PRAGHA_BACKEND_H
#define PRAGHA_BACKEND_H

G_BEGIN_DECLS

//forward declarations
struct con_win;
struct musicobject;

enum player_state {
	ST_PLAYING = 1,
	ST_STOPPED,
	ST_PAUSED
};

#define PRAGHA_TYPE_BACKEND                  (pragha_backend_get_type ())
#define PRAGHA_BACKEND(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_BACKEND, PraghaBackend))
#define PRAGHA_IS_BACKEND(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_BACKEND))
#define PRAGHA_BACKEND_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_BACKEND, PraghaBackendClass))
#define PRAGHA_IS_BACKEND_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_BACKEND))
#define PRAGHA_BACKEND_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_BACKEND, PraghaBackendClass))

struct PraghaBackendPrivate;
typedef struct PraghaBackendPrivate PraghaBackendPrivate;

typedef struct {
	GObject parent;
	PraghaBackendPrivate *priv;
} PraghaBackend;

typedef struct {
	GObjectClass parent_class;
	void (*tick) (PraghaBackend *backend);
	void (*seeked) (PraghaBackend *backend);
	void (*buffering) (PraghaBackend *backend, gint percent);
} PraghaBackendClass;

gboolean pragha_backend_can_seek (PraghaBackend *backend);
void pragha_backend_seek (PraghaBackend *backend, gint64 seek);
gint64 pragha_backend_get_current_length(PraghaBackend *backend);
gint64 pragha_backend_get_current_position(PraghaBackend *backend);
void pragha_backend_set_soft_volume(PraghaBackend *backend, gboolean value);
gdouble pragha_backend_get_volume (PraghaBackend *backend);
void pragha_backend_set_volume (PraghaBackend *backend, gdouble volume);
void pragha_backend_set_delta_volume (PraghaBackend *backend, gdouble delta);
gboolean pragha_backend_is_playing(PraghaBackend *backend);
gboolean pragha_backend_is_paused(PraghaBackend *backend);
gboolean pragha_backend_emitted_error (PraghaBackend *backend);
enum player_state pragha_backend_get_state (PraghaBackend *backend);
void pragha_backend_pause (PraghaBackend *backend);
void pragha_backend_resume (PraghaBackend *backend);
void pragha_backend_play (PraghaBackend *backend);
void pragha_backend_stop (PraghaBackend *backend, GError *error);
void pragha_backend_start (PraghaBackend *backend, struct musicobject *mobj);
GstElement * pragha_backend_get_equalizer (PraghaBackend *backend);
void pragha_backend_update_equalizer (PraghaBackend *backend, const gdouble *bands);
gint backend_init(struct con_win *cwin);

G_END_DECLS

#endif /* PRAGHA_BACKEND_H */
