/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_DBUS_H
#define PRAGHA_DBUS_H

#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>

/* pragha.h */
struct con_win;

#define DBUS_PATH      "/org/pragha/DBus"
#define DBUS_NAME      "org.pragha.DBus"
#define DBUS_INTERFACE "org.pragha.DBus"

#define DBUS_SIG_PLAY             "play"
#define DBUS_SIG_STOP             "stop"
#define DBUS_SIG_PAUSE            "pause"
#define DBUS_SIG_NEXT             "next"
#define DBUS_SIG_PREV             "prev"
#define DBUS_SIG_SHUFFLE          "shuffle"
#define DBUS_SIG_REPEAT           "repeat"
#define DBUS_SIG_INC_VOL          "inc_vol"
#define DBUS_SIG_DEC_VOL          "dec_vol"
#define DBUS_SIG_TOGGLE_VIEW      "toggle_view"
#define DBUS_SIG_SHOW_OSD         "show_osd"
#define DBUS_SIG_ADD_FILE         "add_files"

#define DBUS_METHOD_CURRENT_STATE "curent_state"
#define DBUS_EVENT_UPDATE_STATE   "update_state"

void dbus_send_signal(const gchar *signal, struct con_win *cwin);
void dbus_handlers_free(struct con_win *cwin);

DBusConnection *pragha_init_dbus          (struct con_win *cwin);
gint            pragha_init_dbus_handlers (struct con_win *cwin);

#endif /* PRAGHA_DBUS_H */