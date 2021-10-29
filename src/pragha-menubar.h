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

#include "pragha.h"

void pragha_menubar_update_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, gpointer user_data);

/*
 * Public api..
 */

void pragha_menubar_connect_signals (GtkUIManager *menu_ui_manager, PraghaApplication *pragha);

void
pragha_menubar_set_enable_action (GtkWindow  *window,
                                  const char *action_name,
                                  gboolean    enabled);

void
pragha_menubar_append_action (PraghaApplication *pragha,
                              const gchar       *placeholder,
                              GSimpleAction     *action,
                              GMenuItem         *item);
void
pragha_menubar_remove_action (PraghaApplication *pragha,
                              const gchar       *placeholder,
                              const gchar       *action_name);

void
pragha_menubar_append_submenu (PraghaApplication  *pragha,
                               const gchar        *placeholder,
                               const gchar        *xml_ui,
                               const gchar        *menu_id,
                               const gchar        *label,
                               gpointer            user_data);
void
pragha_menubar_remove_by_id (PraghaApplication *pragha,
                             const gchar       *placeholder,
                             const gchar       *item_id);

gint
pragha_menubar_append_plugin_action (PraghaApplication *pragha,
                                     GtkActionGroup    *action_group,
                                     const gchar       *menu_xml);

void
pragha_menubar_remove_plugin_action (PraghaApplication *pragha,
                                     GtkActionGroup    *action_group,
                                     gint               merge_id);

GtkActionGroup *
pragha_menubar_plugin_action_new (const gchar                *name,
                                  const GtkActionEntry       *action_entries,
                                  guint                       n_action_entries,
                                  const GtkToggleActionEntry *toggle_entries,
                                  guint                       n_toggle_entries,
                                  gpointer                    user_data);


GtkUIManager *pragha_menubar_new       (void);
GtkBuilder   *pragha_gmenu_toolbar_new (PraghaApplication *pragha);

#endif /* PRAGHA_MENU_H */
