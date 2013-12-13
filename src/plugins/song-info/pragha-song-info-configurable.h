/*
 * pragha-hello-world-configurable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2009-2010 Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PRAGHA_SONG_INFO_CONFIGURABLE_H__
#define __PRAGHA_SONG_INFO_CONFIGURABLE_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_SONG_INFO_CONFIGURABLE         (pragha_song_info_configurable_get_type ())
#define PRAGHA_SONG_INFO_CONFIGURABLE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_SONG_INFO_CONFIGURABLE, PraghaSongInfoConfigurable))
#define PRAGHA_SONG_INFO_CONFIGURABLE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_SONG_INFO_CONFIGURABLE, PraghaSongInfoConfigurable))
#define PRAGHA_IS_SONG_INFO_CONFIGURABLE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_SONG_INFO_CONFIGURABLE))
#define PRAGHA_IS_SONG_INFO_CONFIGURABLE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_SONG_INFO_CONFIGURABLE))
#define PRAGHA_SONG_INFO_CONFIGURABLE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_SONG_INFO_CONFIGURABLE, PraghaSongInfoConfigurableClass))

typedef struct _PraghaSongInfoConfigurable      PraghaSongInfoConfigurable;
typedef struct _PraghaSongInfoConfigurableClass PraghaSongInfoConfigurableClass;

struct _PraghaSongInfoConfigurable {
	PeasExtensionBase parent;
};

struct _PraghaSongInfoConfigurableClass {
	PeasExtensionBaseClass parent_class;
};

GType   pragha_song_info_configurable_get_type  (void) G_GNUC_CONST;
void    pragha_song_info_configurable_register  (GTypeModule *module);

G_END_DECLS

#endif /* __PRAGHA_SONG_INFO_CONFIGURABLE_H__ */
