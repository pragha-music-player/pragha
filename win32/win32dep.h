/*
 * easytag
 *
 * File: win32dep.h
 *
 * Copyright (C) 2002-2003, Herman Bloggs <hermanator12002@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef _WIN32DEP_H_
#define _WIN32DEP_H_

#include <glib.h> /* Needed for G_OS_WIN32. */

#ifdef G_OS_WIN32

G_BEGIN_DECLS

/*
 * Windows helper functions
 */

/* Determine Pragha paths */

const gchar * weasytag_install_dir (void);
const gchar * weasytag_locale_dir (void);
const gchar * weasytag_pixbuf_dir (void);

#undef DATADIR
#undef PACKAGE_LOCALE_DIR
#undef PIXMAPDIR
#define DATADIR weasytag_install_dir()
#define PACKAGE_LOCALE_DIR weasytag_locale_dir()
#define PIXMAPDIR weasytag_pixbuf_dir()

G_END_DECLS

#endif /* G_OS_WIN32 */

#endif /* _WIN32DEP_H_ */
