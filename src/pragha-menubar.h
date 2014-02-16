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

#ifndef PRAGHA_MENU_H
#define PRAGHA_MENU_H

#include <gtk/gtk.h>
#include "pragha-backend.h"

/* pragha.h */
typedef struct _PraghaApplication PraghaApplication;

/*
 * Helper to GMenumodel definitions.
 *
 * NOTE: Remember use "&lt;" and "&gt;" as "<" and ">" in accelerators.
 */

#define NEW_MENU(_MENU) \
	"<interface>" \
	"  <menu id='" _MENU "'>"

#define NEW_SUBMENU(_LABEL) \
	"  <submenu>" \
	"    <attribute name='label' translatable='yes'>" _LABEL "</attribute>" \
	"      <section>"

#define NEW_ITEM(_LABEL,_PREFIX,_ACTION) \
	"        <item>" \
	"          <attribute name='label' translatable='yes'>" _LABEL "</attribute>" \
	"          <attribute name='action'>" _PREFIX "." _ACTION  "</attribute>"  \
	"        </item>"

#define NEW_ACCEL_ITEM(_LABEL,_ACCEL,_PREFIX,_ACTION) \
	"        <item>" \
	"          <attribute name='label' translatable='yes'>" _LABEL "</attribute>" \
	"          <attribute name='action'>" _PREFIX "." _ACTION  "</attribute>"  \
	"          <attribute name='accel'>" _ACCEL  "</attribute>"  \
	"        </item>"

#define NEW_ICON_ITEM(_LABEL,_ICON,_PREFIX,_ACTION) \
	"        <item>" \
	"          <attribute name='label' translatable='yes'>" _LABEL "</attribute>" \
	"          <attribute name='action'>" _PREFIX "." _ACTION  "</attribute>"  \
	"          <attribute name='icon'>" _ICON "</attribute>" \
	"        </item>"

#define NEW_ICON_ACCEL_ITEM(_LABEL,_ICON,_ACCEL,_PREFIX,_ACTION) \
	"        <item>" \
	"          <attribute name='label' translatable='yes'>" _LABEL "</attribute>" \
	"          <attribute name='action'>" _PREFIX "." _ACTION  "</attribute>"  \
	"          <attribute name='icon'>" _ICON "</attribute>" \
	"          <attribute name='accel'>" _ACCEL  "</attribute>"  \
	"        </item>"

#define SEPARATOR \
	"      </section>" \
	"      <section>"
#define NEW_PLACEHOLDER(_TAG) \
	"      <section id='" _TAG "'/>"

#define CLOSE_SUBMENU \
	"      </section>" \
	"    </submenu>"

#define CLOSE_MENU \
	"  </menu>" \
	"</interface>"

GMenu *
pragha_menubar_get_menu_section (PraghaApplication *pragha,
                                 const char        *id);
void
pragha_menubar_emthy_menu_section (PraghaApplication *pragha,
                                   const char        *id);

/*
 * Public api..
 */

GtkBuilder *pragha_application_set_menubar (PraghaApplication *pragha);

#endif /* PRAGHA_MENU_H */