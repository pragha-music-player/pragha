/*************************************************************************/
/* Copyright (C) 2014 matias <mati86dl@gmail.com>                        */
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

#ifndef __PRAGHA_THREADED_SOCKET_SERVICE_H__
#define __PRAGHA_THREADED_SOCKET_SERVICE_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define PRAGHA_TYPE_THREADED_SOCKET_SERVICE                 (pragha_threaded_socket_service_get_type ())
#define PRAGHA_THREADED_SOCKET_SERVICE(inst)                (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             PRAGHA_TYPE_THREADED_SOCKET_SERVICE,                         \
                                                             PraghaThreadedSocketService))
#define PRAGHA_THREADED_SOCKET_SERVICE_CLASS(class)         (G_TYPE_CHECK_CLASS_CAST ((class),                       \
                                                             PRAGHA_TYPE_THREADED_SOCKET_SERVICE,                         \
                                                             PraghaThreadedSocketServiceClass))
#define PRAGHA_IS_THREADED_SOCKET_SERVICE(inst)             (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             PRAGHA_TYPE_THREADED_SOCKET_SERVICE))
#define PRAGHA_IS_THREADED_SOCKET_SERVICE_CLASS(class)      (G_TYPE_CHECK_CLASS_TYPE ((class),                       \
                                                             PRAGHA_TYPE_THREADED_SOCKET_SERVICE))
#define PRAGHa_THREADED_SOCKET_SERVICE_GET_CLASS(inst)      (G_TYPE_INSTANCE_GET_CLASS ((inst),                      \
                                                             PRAGHA_TYPE_THREADED_SOCKET_SERVICE,                         \
                                                             PraghaThreadedSocketServiceClass))

typedef struct _PraghaThreadedSocketServicePrivate           PraghaThreadedSocketServicePrivate;
typedef struct _PraghaThreadedSocketServiceClass             PraghaThreadedSocketServiceClass;

struct _PraghaThreadedSocketServiceClass
{
	GSocketServiceClass parent_class;

	gboolean (* run) (GThreadedSocketService *service,
	                  GSocketConnection      *connection,
	                  GCancellable           *cancel);
};

struct _PraghaThreadedSocketService
{
	GSocketService                      parent_instance;
	PraghaThreadedSocketServicePrivate *priv;
};

GType           pragha_threaded_socket_service_get_type         (void);

void            pragha_threaded_socket_service_cancel           (GSocketService *service);
GSocketService *pragha_threaded_socket_service_new              (void);

G_END_DECLS

#endif /* __PRAGHA_THREADED_SOCKET_SERVICE_H__ */