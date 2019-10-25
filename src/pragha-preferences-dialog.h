/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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

#ifndef PRAGHA_PREFERENCES_DIALOG_H
#define PRAGHA_PREFERENCES_DIALOG_H

#include <gtk/gtk.h>

#define LASTFM_UNAME_LEN           256
#define LASTFM_PASS_LEN            512
#define ALBUM_ART_PATTERN_LEN      1024
#define AUDIO_CD_DEVICE_ENTRY_LEN  32

#define PRAGHA_TYPE_PREFERENCES_DIALOG (pragha_preferences_dialog_get_type())
#define PRAGHA_PREFERENCES_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES_DIALOG, PraghaPreferencesDialog))
#define PRAGHA_PREFERENCES_DIALOG_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_PREFERENCES_DIALOG, PraghaPreferencesDialog const))
#define PRAGHA_PREFERENCES_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), PRAGHA_TYPE_PREFERENCES_DIALOG, PraghaPreferencesDialogClass))
#define PRAGHA_IS_PREFERENCES_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_PREFERENCES_DIALOG))
#define PRAGHA_IS_PREFERENCES_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRAGHA_TYPE_PREFERENCES_DIALOG))
#define PRAGHA_PREFERENCES_DIALOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), PRAGHA_TYPE_PREFERENCES_DIALOG, PraghaPreferencesDialogClass))

GType pragha_preferences_dialog_get_type (void);

typedef struct _PraghaPreferencesDialog PraghaPreferencesDialog;

void               pragha_gtk_entry_set_text                  (GtkEntry *entry, const gchar *text);

void               pragha_preferences_append_audio_setting    (PraghaPreferencesDialog *dialog, GtkWidget *widget, gboolean expand);
void               pragha_preferences_remove_audio_setting    (PraghaPreferencesDialog *dialog, GtkWidget *widget);

void               pragha_preferences_append_desktop_setting  (PraghaPreferencesDialog *dialog, GtkWidget *widget, gboolean expand);
void               pragha_preferences_remove_desktop_setting  (PraghaPreferencesDialog *dialog, GtkWidget *widget);

void               pragha_preferences_append_services_setting (PraghaPreferencesDialog *dialog, GtkWidget *widget, gboolean expand);
void               pragha_preferences_remove_services_setting (PraghaPreferencesDialog *dialog, GtkWidget *widget);

void               pragha_preferences_dialog_connect_handler    (PraghaPreferencesDialog *dialog,
                                                                 GCallback          callback,
                                                                 gpointer           user_data);
void               pragha_preferences_dialog_disconnect_handler (PraghaPreferencesDialog *pragha,
                                                                 GCallback          callback,
                                                                 gpointer           user_data);

void               pragha_preferences_dialog_show            (PraghaPreferencesDialog *dialog);

void               pragha_preferences_dialog_set_parent         (PraghaPreferencesDialog *dialog, GtkWidget *parent);

PraghaPreferencesDialog *
pragha_preferences_dialog_get (void);

#endif /* PRAGHA_PREFERENCES_DIALOG_H */
