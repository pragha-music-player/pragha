/*
 * easytag
 *
 * File: win32dep.c
 * Date: June, 2002
 * Description: Windows dependant code for Easytag
 * this code if largely taken from win32 Gaim and Purple
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

/* Needed for G_OS_WIN32. */
#include <glib.h>

#ifdef G_OS_WIN32

#include <gdk/gdkwin32.h>

#include "win32dep.h"

/*
 *  DEFINES & MACROS
 */
#define _(x) gettext(x)

/*
 * LOCALS
 */
static char *install_dir = NULL, *locale_dir = NULL;

/*
 *  PUBLIC CODE
 */

/* Determine Easytag Paths during Runtime */
const gchar *
weasytag_install_dir (void)
{
    static gboolean initialized = FALSE;

    if (!initialized)
    {
        gchar *tmp;

        tmp = g_win32_get_package_installation_directory_of_module (NULL);

        if (tmp == NULL)
        {
            tmp = g_win32_error_message (GetLastError ());
            g_debug ("GetModuleFileName error: %s", tmp);
            g_free (tmp);
            return NULL;
        }
        else
        {
            install_dir = tmp;
            initialized = TRUE;
        }
    }

    return install_dir;
}

const gchar *
weasytag_locale_dir (void)
{
    static gboolean initialized = FALSE;

    if (!initialized)
    {
        const gchar *inst_dir = weasytag_install_dir ();

        if (inst_dir != NULL)
        {
            locale_dir = g_build_filename (inst_dir, "share", "locale", NULL);
            initialized = TRUE;
        }
        else
        {
            return NULL;
        }
    }
    return locale_dir;
}

const gchar *
weasytag_pixbuf_dir (void)
{
    static gboolean initialized = FALSE;

    if (!initialized)
    {
        const gchar *inst_dir = weasytag_install_dir ();

        if (inst_dir != NULL)
        {
            locale_dir = g_build_filename (inst_dir, "share", "pixmaps", "pragha", NULL);
            initialized = TRUE;
        }
        else
        {
            return NULL;
        }
    }

    return locale_dir;
}

#endif /* G_OS_WIN32 */
