/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>                   */
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

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include "pragha-equalizer-dialog.h"
#include "pragha.h"

#define NUM_BANDS 10

struct _PraghaEqualizerDialog {
	GtkWidget         *widget;
	GtkWidget         *vscales[NUM_BANDS];
	GtkWidget         *preset_combobox;
	PraghaPreferences *preferences;
	GstElement        *equalizer;
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
	"30 Hz", "60 Hz", "120 Hz", "250 Hz", "500 Hz", "1 kHz", "2 kHz", "4 kHz", "8 kHz", "15 kHz"
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
	GtkWidget **vscales = user_data;
	gint i, option = 0;

	gdouble value[][NUM_BANDS] =
	{
		{  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}, // "Disabled"
		{  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -7.2, -7.2, -7.2, -9.6}, // "Classical"
		{  0.0,  0.0,  8.0,  5.6,  5.6,  5.6,  3.2,  0.0,  0.0,  0.0}, // "Club"
		{  9.6,  7.2,  2.4, -1.1, -1.1, -5.6, -7.2, -7.2, -1.1, -1.1}, // "Dance"
		{ -8.0,  9.6,  9.6,  5.6,  1.6, -4.0, -8.0,-10.4,-11.2,-11.2}, //"Full Bass"
		{  7.2,  5.6, -1.1, -7.2, -4.8,  1.6,  8.0, 11.2, 12.0, 12.0}, // "Full Bass and Treble"
		{ -9.6, -9.6, -9.6, -4.0,  2.4, 11.2, 11.5, 11.8, 11.8, 12.0}, // "Full Treble"
		{  4.8, 11.2,  5.6, -3.2, -2.4,  1.6,  4.8,  9.6, 11.9, 11.9}, // "Laptop Speakers and Headphones"
		{ 10.4, 10.4,  5.6,  5.6, -1.1, -4.8, -4.8, -4.8, -1.1, -1.1}, // "Large Hall"
		{ -4.8, -1.1,  4.0,  5.6,  5.6,  5.6,  4.0,  2.4,  2.4,  2.4}, // "Live"
		{  7.2,  7.2, -1.1, -1.1, -1.1, -1.1, -1.1, -1.1,  7.2,  7.2}, // "Party"
		{ -1.6,  4.8,  7.2,  8.0,  5.6, -1.1, -2.4, -2.4, -1.6, -1.6}, // "Pop"
		{ -1.1, -1.1, -1.1, -5.6, -1.1,  6.4,  6.4, -1.1, -1.1, -1.1}, // "Reggae"
		{  8.0,  4.8, -5.6, -8.0, -3.2,  4.0,  8.8, 11.2, 11.2, 11.2}, // "Rock"
		{ -2.4, -4.8, -4.0, -1.1,  4.0,  5.6,  8.8,  9.6, 11.2,  9.6}, // "Ska"
		{ 12.0,  8.0,  6.0,  3.0,  0.0,  0.0,  3.0,  6.0,  8.0, 12.0}, // "Smiley Face Curve"
		{  4.8,  1.6, -1.1, -2.4, -1.1,  4.0,  8.0,  9.6, 11.2, 12.0}, // "Soft"
		{  4.0,  4.0,  2.4, -1.1, -4.0, -5.6, -3.2, -1.1,  2.4,  8.8}, // "Soft Rock"
		{  8.0,  5.6, -1.1, -5.6, -4.8, -1.1,  8.0,  9.6,  9.6,  8.8}, // "Techno"
	};

	option = gtk_combo_box_get_active (widget);

	if(option == 19)
		return;

	for (i = 0; i < NUM_BANDS; i++)
		gtk_range_set_value(GTK_RANGE(vscales[i]), value[option][i]);
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

	text = g_strdup_printf("%.1lf", gtk_range_get_value(GTK_RANGE(vscale)));
	gtk_tooltip_set_text (tooltip, text);
	g_free(text);

	return TRUE;
}

static void
band_bind_to_backend (GtkRange *range, GstElement *equalizer, gint i)
{
	gchar *eq_property = g_strdup_printf ("band%i", i);
	GtkAdjustment *adj = gtk_range_get_adjustment (range);
	GBindingFlags flags = G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

	g_object_bind_property (equalizer, eq_property, adj, "value", flags);

	g_free (eq_property);
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
	gtk_widget_destroy(dialog->widget);

	g_slice_free(PraghaEqualizerDialog, dialog);
}

void pragha_equalizer_dialog_show(struct con_win *cwin)
{
	PraghaEqualizerDialog *dialog;
	GtkWidget *mhbox, *hbox, *dbvbox, *label;
	gint i;

	dialog = g_slice_new0 (PraghaEqualizerDialog);

	dialog->equalizer = pragha_backend_get_equalizer (cwin->backend);
	dialog->preferences = pragha_preferences_get ();

	/* Create vertical scales band to equalizer */

	for (i = 0; i < NUM_BANDS; i++) {
		dialog->vscales[i] = gtk_vscale_new_with_range(-24.0, 12.0, 0.1);
		gtk_range_set_inverted(GTK_RANGE(dialog->vscales[i]), TRUE);
		gtk_scale_set_draw_value (GTK_SCALE(dialog->vscales[i]), FALSE);

		g_object_set (G_OBJECT(dialog->vscales[i]), "has-tooltip", TRUE, NULL);
		g_signal_connect(G_OBJECT(dialog->vscales[i]), "query-tooltip",
		                 G_CALLBACK(pragha_equalizer_band_get_tooltip),
		                 NULL);
	}

	/* Create the db scale */

	mhbox = gtk_hbox_new(FALSE, 0);

	dbvbox = gtk_vbutton_box_new  ();
	gtk_box_set_spacing (GTK_BOX(dbvbox), 0);
	label = gtk_label_new("+12 db");
	gtk_misc_set_alignment (GTK_MISC(label), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(dbvbox), label, FALSE, FALSE, 0);
	label = gtk_label_new("0 db");
	gtk_misc_set_alignment (GTK_MISC(label), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(dbvbox), label, FALSE, FALSE, 0);
	label = gtk_label_new("-12 db");
	gtk_misc_set_alignment (GTK_MISC(label), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(dbvbox), label, FALSE, FALSE, 0);
	label = gtk_label_new("-24 db");
	gtk_misc_set_alignment (GTK_MISC(label), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(dbvbox), label, FALSE, FALSE, 0);

	/* Create the scales with frequency bands */

	hbox = gtk_hbox_new(FALSE, 0);
	for (i = 0; i < G_N_ELEMENTS(label_band_frec); i++) {
		label = gtk_label_new(label_band_frec[i]);
		gtk_label_set_angle(GTK_LABEL(label), 90);
		gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), dialog->vscales[i], FALSE, FALSE, 0);
	}

	/* Add the widgets */

	gtk_box_pack_start(GTK_BOX(mhbox), dbvbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mhbox), hbox, TRUE, TRUE, 0);

	/* Create the dialog */

	dialog->widget = gtk_dialog_new_with_buttons (_("Equalizer"),
	                                              GTK_WINDOW(cwin->mainwindow),
	                                              GTK_DIALOG_DESTROY_WITH_PARENT,
	                                              GTK_STOCK_OK,
	                                              GTK_RESPONSE_OK,
	                                              NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog->widget), -1, 200);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->widget), GTK_RESPONSE_OK);

	/* Append list of default presets */

	dialog->preset_combobox = gtk_combo_box_text_new ();

	for (i = 0; i < G_N_ELEMENTS(presets_names); i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog->preset_combobox), _(presets_names[i]));

	/* Conect the signals */

	for (i = 0; i < NUM_BANDS; i++) {
		g_signal_connect(dialog->vscales[i], "change-value",
				 G_CALLBACK(vscales_eq_set_by_user), dialog->preset_combobox);
	}
	g_signal_connect(G_OBJECT(dialog->preset_combobox), "changed",
			 G_CALLBACK(eq_combobox_activated_cb), dialog->vscales);

	/* Append and show the dialog */

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog->widget))), dialog->preset_combobox, FALSE, FALSE, 0);
	gtk_button_box_set_child_secondary(GTK_BUTTON_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog->widget))), dialog->preset_combobox, TRUE);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog->widget))), mhbox, TRUE, TRUE, 0);

	if (dialog->equalizer != NULL) {
		for (i = 0; i < NUM_BANDS; i++)
			band_bind_to_backend(GTK_RANGE(dialog->vscales[i]), dialog->equalizer, i);
		pragha_equalizer_dialog_init_bands (dialog);
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(hbox), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(dialog->preset_combobox), FALSE);
	}

	gtk_widget_show_all(dialog->widget);

	g_signal_connect (G_OBJECT (dialog->widget), "response",
	                  G_CALLBACK (pragha_equalizer_dialog_response), dialog);
}
