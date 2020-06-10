/*************************************************************************/
/* Copyright (C) 2019-2020 matias <mati86dl@gmail.com>                   */
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

#ifndef __PRAGHA_SUBSONIC_API_H__
#define __PRAGHA_SUBSONIC_API_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
	S_USER_CANCELLED = -2,
	S_GENERIC_OK = -1,
	S_GENERIC_ERROR = 0,
	S_PARAMETER_MISSING = 10,
	S_CLIENT_MUST_UPGRADE = 20,
	S_SERVER_MUST_UPGRADE = 30,
	S_WROMG_CREDENTIAL = 40,
	S_NOT_SUPPORT_LDAP_USER = 41,
	S_OPERATION_NOT_AUTHORIZED = 50,
	S_TRIAL_OVER = 60,
	S_DATA_NOT_FOUND = 70
} SubsonicStatusCode;

#define PRAGHA_TYPE_SUBSONIC_API            (pragha_subsonic_api_get_type())
#define PRAGHA_SUBSONIC_API(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SUBSONIC_API, PraghaSubsonicApi))
#define PRAGHA_SUBSONIC_API_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_SUBSONIC_API, PraghaSubsonicApi const))
#define PRAGHA_SUBSONIC_API_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_SUBSONIC_API, PraghaSubsonicApiClass))
#define PRAGHA_IS_SUBSONIC_API(obj) (        G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_SUBSONIC_API))
#define PRAGHA_IS_SUBSONIC_API_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_SUBSONIC_API))
#define PRAGHA_SUBSONIC_API_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_SUBSONIC_API, PraghaSubsonicApiClass))

typedef struct _PraghaSubsonicApi      PraghaSubsonicApi;
typedef struct _PraghaSubsonicApiClass PraghaSubsonicApiClass;

struct _PraghaSubsonicApiClass
{
	GObjectClass           parent_class;

	void (*authenticated) (PraghaSubsonicApi *subsonic, SubsonicStatusCode code);
	void (*pong)          (PraghaSubsonicApi *subsonic, SubsonicStatusCode code);
	void (*scan_progress) (PraghaSubsonicApi *subsonic, gint progress);
	void (*scan_total)    (PraghaSubsonicApi *subsonic, gint total);
	void (*scan_finished) (PraghaSubsonicApi *subsonic, SubsonicStatusCode code);
};


/*
 * Helpers
 */

gchar *
pragha_subsonic_api_get_playback_url      (PraghaSubsonicApi *subsonic,
                                           const gchar       *friendly_url);


/*
 * Public methods
 */

PraghaSubsonicApi *
pragha_subsonic_api_new (void);

void
pragha_subsonic_api_authentication        (PraghaSubsonicApi *subsonic,
                                           const gchar       *server,
                                           const gchar       *username,
                                           const gchar       *password);

void
pragha_subsonic_api_deauthentication      (PraghaSubsonicApi *subsonic);

void
pragha_subsonic_api_ping_server           (PraghaSubsonicApi *subsonic);

void
pragha_subsonic_api_scan_server           (PraghaSubsonicApi *subsonic);

void
pragha_subsonic_api_cancel                (PraghaSubsonicApi *subsonic);


gboolean
pragha_subsonic_api_is_authtenticated     (PraghaSubsonicApi *subsonic);

gboolean
pragha_subsonic_api_is_connected          (PraghaSubsonicApi *subsonic);

gboolean
pragha_subsonic_api_is_scanning           (PraghaSubsonicApi *subsonic);


GCancellable *
pragha_subsonic_get_cancellable           (PraghaSubsonicApi *subsonic);

GSList *
pragha_subsonic_api_get_songs_list        (PraghaSubsonicApi *subsonic);

G_END_DECLS

#endif /* __PRAGHA_SUBSONIC_API_H__ */
