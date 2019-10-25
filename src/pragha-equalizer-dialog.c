/*************************************************************************/
/* Copyright (C) 2012-2015 matias <mati86dl@gmail.com>                   */
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "pragha-equalizer-dialog.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <math.h>

#include "pragha-preferences.h"

#define NUM_BANDS 11

typedef struct _PraghaEqualizerDialog PraghaEqualizerDialog;

struct _PraghaEqualizerDialog {
	GtkWidget         *enable;
	GtkWidget         *vscales[NUM_BANDS];
	GtkWidget         *preset_combobox;

	PraghaPreferences *preferences;

	GstElement        *equalizer;
	GstElement        *preamp;
};

static const gchar *presets_names[] = {
	N_("Disabled"),
	N_("Classical"),
	N_("Club"),
	N_("Dance"),
	N_("Full Bass"),
	N_("Full Bass and Treble"),
	N_("Full Treble"),
	N_("Laptop Speakers and Headphones"),
	N_("Large Hall"),
	N_("Live"),
	N_("Party"),
	N_("Pop"),
	N_("Reggae"),
	N_("Rock"),
	N_("Ska"),
	N_("Smiley Face Curve"),
	N_("Soft"),
	N_("Soft Rock"),
	N_("Techno"),
	N_("Custom")
};

static const gchar *label_band_frec[] = {
	"30", "60", "120", "250", "500", "1k", "2k", "4k", "8k", "15k"
};

static gboolean
vscales_eq_set_by_user (GtkRange *range, GtkScrollType scroll, gdouble value, gpointer user_data)
{
	GtkWidget *eq_combobox = user_data;

	/* Set "custum" in combo */
	gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 19);

	return FALSE;
}

static void
eq_combobox_activated_cb (GtkComboBox *widget, gpointer user_data)
{
	PraghaEqualizerDialog *dialog = user_data;
	gint i, option = 0;

	gdouble value[][NUM_BANDS] =
	{
		{  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}, // "Disabled"
		{  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -7.2, -7.2, -7.2, -9.6}, // "Classical"
		{  0.0,  0.0,  0.0,  8.0,  5.6,  5.6,  5.6,  3.2,  0.0,  0.0,  0.0}, // "Club"
		{ -1.1,  9.6,  7.2,  2.4, -1.1, -1.1, -5.6, -7.2, -7.2, -1.1, -1.1}, // "Dance"
		{ -1.1, -8.0,  9.6,  9.6,  5.6,  1.6, -4.0, -8.0,-10.4,-11.2,-11.2}, //"Full Bass"
		{ -1.1,  7.2,  5.6, -1.1, -7.2, -4.8,  1.6,  8.0, 11.2, 12.0, 12.0}, // "Full Bass and Treble"
		{ -1.1, -9.6, -9.6, -9.6, -4.0,  2.4, 11.2, 11.5, 11.8, 11.8, 12.0}, // "Full Treble"
		{ -1.1,  4.8, 11.2,  5.6, -3.2, -2.4,  1.6,  4.8,  9.6, 11.9, 11.9}, // "Laptop Speakers and Headphones"
		{ -1.1, 10.4, 10.4,  5.6,  5.6, -1.1, -4.8, -4.8, -4.8, -1.1, -1.1}, // "Large Hall"
		{ -1.1, -4.8, -1.1,  4.0,  5.6,  5.6,  5.6,  4.0,  2.4,  2.4,  2.4}, // "Live"
		{ -1.1,  7.2,  7.2, -1.1, -1.1, -1.1, -1.1, -1.1, -1.1,  7.2,  7.2}, // "Party"
		{ -1.1, -1.6,  4.8,  7.2,  8.0,  5.6, -1.1, -2.4, -2.4, -1.6, -1.6}, // "Pop"
		{ -1.1, -1.1, -1.1, -1.1, -5.6, -1.1,  6.4,  6.4, -1.1, -1.1, -1.1}, // "Reggae"
		{ -1.1,  8.0,  4.8, -5.6, -8.0, -3.2,  4.0,  8.8, 11.2, 11.2, 11.2}, // "Rock"
		{ -1.1, -2.4, -4.8, -4.0, -1.1,  4.0,  5.6,  8.8,  9.6, 11.2,  9.6}, // "Ska"
		{ -7.0, 12.0,  8.0,  6.0,  3.0,  0.0,  0.0,  3.0,  6.0,  8.0, 12.0}, // "Smiley Face Curve"
		{ -1.1,  4.8,  1.6, -1.1, -2.4, -1.1,  4.0,  8.0,  9.6, 11.2, 12.0}, // "Soft"
		{ -1.1,  4.0,  4.0,  2.4, -1.1, -4.0, -5.6, -3.2, -1.1,  2.4,  8.8}, // "Soft Rock"
		{ -1.1,  8.0,  5.6, -1.1, -5.6, -4.8, -1.1,  8.0,  9.6,  9.6,  8.8}, // "Techno"
	};

	gtk_switch_set_state (GTK_SWITCH(dialog->enable), TRUE);

	option = gtk_combo_box_get_active (widget);

	if(option == 19)
		return;

	for (i = 0; i < NUM_BANDS; i++)
		gtk_range_set_value(GTK_RANGE(dialog->vscales[i]), value[option][i]);
}

static void
pragha_equalizer_dialog_init_bands (PraghaEqualizerDialog *dialog)
{
	gchar *eq_preset = NULL;
	gdouble *saved_bands;
	gint i;

	eq_preset = pragha_preferences_get_string (dialog->preferences,
	                                           GROUP_AUDIO,
	                                           KEY_EQ_PRESET);

	if(eq_preset != NULL) {
		if (g_ascii_strcasecmp(eq_preset, "Custom") == 0) {
			saved_bands = pragha_preferences_get_double_list (dialog->preferences,
			                                                  GROUP_AUDIO,
			                                                  KEY_EQ_10_BANDS);
			if (saved_bands != NULL) {
				for (i = 0; i < NUM_BANDS; i++)
					gtk_range_set_value(GTK_RANGE(dialog->vscales[i]), saved_bands[i]);

				g_free(saved_bands);
			}
			gtk_combo_box_set_active (GTK_COMBO_BOX(dialog->preset_combobox), G_N_ELEMENTS(presets_names) - 1);
		}
		else {
			for (i = 0; i < G_N_ELEMENTS(presets_names); i++) {
				if (g_ascii_strcasecmp(eq_preset, presets_names[i]) == 0) {
					gtk_combo_box_set_active (GTK_COMBO_BOX(dialog->preset_combobox), i);
					break;
				}
			}
		}
		g_free(eq_preset);
	}
	else {
		gtk_combo_box_set_active (GTK_COMBO_BOX(dialog->preset_combobox), 0);
	}
}

static void
pragha_equalizer_dialog_save_preset (PraghaEqualizerDialog *dialog)
{
	gdouble bands[NUM_BANDS];
	gint i, preset;

	preset = gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->preset_combobox));

	for (i = 0; i < NUM_BANDS; i++)
		bands[i] = gtk_range_get_value(GTK_RANGE(dialog->vscales[i]));

	pragha_preferences_set_string(dialog->preferences,
	                              GROUP_AUDIO,
	                              KEY_EQ_PRESET,
	                              presets_names[preset]);

	pragha_preferences_set_double_list(dialog->preferences,
	                                   GROUP_AUDIO,
	                                   KEY_EQ_10_BANDS,
	                                   bands,
	                                   NUM_BANDS);
}

static gboolean
pragha_equalizer_band_get_tooltip (GtkWidget  *vscale,
                                   gint        x,
                                   gint        y,
                                   gboolean    keyboard_mode,
                                   GtkTooltip *tooltip,
                                   gpointer    data)
{
	gchar *text = NULL;

	text = g_strdup_printf("%.1lf dB", gtk_range_get_value(GTK_RANGE(vscale)));
	gtk_tooltip_set_text (tooltip, text);
	g_free(text);

	return TRUE;
}

static void
pragha_equalizer_dialog_bind_bands_to_backend (PraghaEqualizerDialog *dialog)
{
	gchar *eq_property;
	GtkAdjustment *adjustment;
	gint i;

	GBindingFlags flags = G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	for (i = 1; i < NUM_BANDS; i++) {
		eq_property = g_strdup_printf ("band%i", i-1);
		adjustment = gtk_range_get_adjustment (GTK_RANGE(dialog->vscales[i]));

		g_object_bind_property (dialog->equalizer, eq_property,
		                        adjustment, "value",
		                        flags);

		g_free (eq_property);
	}
}

static gboolean
db_to_volume_transform_func (GBinding     *binding,
                             const GValue *from_value,
                             GValue       *to_value,
                             gpointer      user_data)
{
	gdouble db = 0, volume = 0;

	db = g_value_get_double (from_value);
	volume = pow(10.0, db / 20.0);
	g_value_set_double (to_value, volume);

	return TRUE;
}

static gboolean
volume_to_db_transform_func (GBinding     *binding,
                             const GValue *from_value,
                             GValue       *to_value,
                             gpointer      user_data)
{
	gdouble db = 0, volume = 0;

	volume = g_value_get_double (from_value);
	db = 20 * log (volume/100);
	g_value_set_double (to_value, db);

	return TRUE;
}

static void
pragha_equalizer_dialog_enabled_cb (GtkSwitch *enable,
									gpointer   data1,
                                    gpointer   data)
{
	PraghaEqualizerDialog *dialog = data;

	if (!gtk_switch_get_active (enable)) {
		gtk_combo_box_set_active (GTK_COMBO_BOX(dialog->preset_combobox), 0);
	}
}

static void
pragha_equalizer_dialog_response (GtkWidget *w_dialog,
                                  gint       response_id,
                                  gpointer   data)
{
	PraghaEqualizerDialog *dialog = data;

	if(dialog->equalizer != NULL)
		pragha_equalizer_dialog_save_preset (dialog);

	g_object_unref (dialog->preferences);
	gtk_widget_destroy(w_dialog);

	g_slice_free(PraghaEqualizerDialog, dialog);
}

void
pragha_equalizer_dialog_show (PraghaBackend *backend, GtkWidget *parent)
{
	PraghaEqualizerDialog *dialog;
	GtkWidget *w_dialog, *grid, *label;
	GtkWidget *preamp_scale;
	gint i;

	dialog = g_slice_new0 (PraghaEqualizerDialog);

	dialog->equalizer = pragha_backend_get_equalizer (backend);
	dialog->preamp = pragha_backend_get_preamp (backend);
	dialog->preferences = pragha_preferences_get ();

	/* The main grid of dialog */

	grid = gtk_grid_new ();

	/* Enable switch button */

	dialog->enable = gtk_switch_new ();
	gtk_switch_set_state (GTK_SWITCH(dialog->enable), TRUE);
	gtk_widget_set_valign (GTK_WIDGET(dialog->enable), GTK_ALIGN_CENTER);
	gtk_widget_set_halign (GTK_WIDGET(dialog->enable), GTK_ALIGN_CENTER);
	g_object_set (G_OBJECT(dialog->enable), "margin", 4, NULL);
	gtk_grid_attach (GTK_GRID(grid), GTK_WIDGET(dialog->enable),
	                 0, 0, 1, 1);
	g_signal_connect (dialog->enable, "notify::active",
	                  G_CALLBACK (pragha_equalizer_dialog_enabled_cb), dialog);

	/* Preamp scale */

	preamp_scale = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL,
	                                         -12.0, 12.0, 0.1);
	gtk_scale_add_mark (GTK_SCALE(preamp_scale), 0.0, GTK_POS_LEFT, NULL);
	gtk_range_set_inverted(GTK_RANGE(preamp_scale), TRUE);
	gtk_scale_set_draw_value (GTK_SCALE(preamp_scale), FALSE);
	gtk_grid_attach (GTK_GRID(grid), GTK_WIDGET(preamp_scale),
	                 0, 1, 1, 3);

	g_object_bind_property_full (dialog->preamp, "volume",
	                             gtk_range_get_adjustment(GTK_RANGE(preamp_scale)), "value",
	                             G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL,
	                             volume_to_db_transform_func,
	                             db_to_volume_transform_func,
		                         NULL, NULL);

	g_object_set (G_OBJECT(preamp_scale), "has-tooltip", TRUE, NULL);
	g_signal_connect (G_OBJECT(preamp_scale), "query-tooltip",
	                  G_CALLBACK(pragha_equalizer_band_get_tooltip),
	                  NULL);

	dialog->vscales[0] = preamp_scale;

	/* Preamp label */

	label = gtk_label_new("Preamp");

	gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_widget_set_halign (GTK_WIDGET(label), GTK_ALIGN_CENTER);
	gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_CENTER);

	gtk_grid_attach (GTK_GRID(grid), label,
	                 0, 4, 1, 1);

	/* Equalizer scales marks */

	label = gtk_label_new("+12 dB");
	gtk_widget_set_vexpand (label, TRUE);
	gtk_widget_set_halign (GTK_WIDGET(label), GTK_ALIGN_END);
	gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID(grid), label,
	                 1, 1, 1, 1);

	label = gtk_label_new("0 dB");
	gtk_widget_set_vexpand (label, TRUE);
	gtk_widget_set_halign (GTK_WIDGET(label), GTK_ALIGN_END);
	gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID(grid), label,
	                 1, 2, 1, 1);
	label = gtk_label_new("-12 dB");
	gtk_widget_set_vexpand (label, TRUE);
	gtk_widget_set_halign (GTK_WIDGET(label), GTK_ALIGN_END);
	gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID(grid), label,
	                 1, 3, 1, 1);

	/* Create vertical scales band to equalizer */

	for (i = 1; i < NUM_BANDS; i++) {
		dialog->vscales[i] = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL,
		                                               -12.0, 12.0, 0.1);
		gtk_range_set_inverted(GTK_RANGE(dialog->vscales[i]), TRUE);
		gtk_scale_set_draw_value (GTK_SCALE(dialog->vscales[i]), FALSE);

		gtk_scale_add_mark (GTK_SCALE(dialog->vscales[i]),  12.0, GTK_POS_LEFT, NULL);
		gtk_scale_add_mark (GTK_SCALE(dialog->vscales[i]),   0.0, GTK_POS_LEFT, NULL);
		gtk_scale_add_mark (GTK_SCALE(dialog->vscales[i]), -12.0, GTK_POS_LEFT, NULL);

		g_object_set (G_OBJECT(dialog->vscales[i]), "has-tooltip", TRUE, NULL);
		g_signal_connect(G_OBJECT(dialog->vscales[i]), "query-tooltip",
		                 G_CALLBACK(pragha_equalizer_band_get_tooltip),
		                 NULL);

		gtk_widget_set_vexpand (dialog->vscales[i], TRUE);
		gtk_widget_set_hexpand (dialog->vscales[i], TRUE);

		gtk_grid_attach (GTK_GRID(grid), dialog->vscales[i],
		                 i+1, 1, 1, 3);
	}

	for (i = 0; i < G_N_ELEMENTS(label_band_frec); i++) {
		label = gtk_label_new(label_band_frec[i]);

		gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_widget_set_halign (GTK_WIDGET(label), GTK_ALIGN_CENTER);
		gtk_widget_set_valign (GTK_WIDGET(label), GTK_ALIGN_CENTER);

		gtk_grid_attach (GTK_GRID(grid), GTK_WIDGET(label),
		                 i+2, 4, 1, 1);
	}

	/* Default pressets */

	dialog->preset_combobox = gtk_combo_box_text_new ();
	gtk_widget_set_halign (GTK_WIDGET(dialog->preset_combobox), GTK_ALIGN_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER(dialog->preset_combobox), 4);

	gtk_grid_attach (GTK_GRID(grid), dialog->preset_combobox,
	                 2, 0, 10, 1);

  	for (i = 0; i < G_N_ELEMENTS(presets_names); i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog->preset_combobox), _(presets_names[i]));

	/* Create the dialog */

	w_dialog = gtk_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW(w_dialog), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent (GTK_WINDOW(w_dialog), TRUE);
	gtk_window_set_title (GTK_WINDOW(w_dialog), _("Equalizer"));
  	gtk_window_set_default_size(GTK_WINDOW (w_dialog), 400, 200);

	/* Conect the signals */

	for (i = 0; i < NUM_BANDS; i++) {
		g_signal_connect(dialog->vscales[i], "change-value",
				 G_CALLBACK(vscales_eq_set_by_user), dialog->preset_combobox);
	}

	g_signal_connect(G_OBJECT(dialog->preset_combobox), "changed",
	                 G_CALLBACK(eq_combobox_activated_cb), dialog);

	/* Append and show the dialog */

	gtk_box_pack_start (GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(w_dialog))),
	                    grid, TRUE, TRUE, 0);

	if (dialog->equalizer != NULL && dialog->preamp != NULL) {
		pragha_equalizer_dialog_bind_bands_to_backend (dialog);
		pragha_equalizer_dialog_init_bands (dialog);
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(grid), FALSE);
	}

	gtk_widget_show_all(w_dialog);

	g_signal_connect (G_OBJECT (w_dialog), "response",
	                  G_CALLBACK (pragha_equalizer_dialog_response), dialog);
}
