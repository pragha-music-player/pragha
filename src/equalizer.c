/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"

void
backend_update_eq(gpointer data)
{
	gdouble gain;
	GtkWidget *vscale;
	struct con_win *cwin;

	cwin = g_object_get_data(data, "cwin");

	vscale = g_object_get_data(data, "band0");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band0", gain, NULL);

	vscale = g_object_get_data(data, "band1");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band1", gain, NULL);

	vscale = g_object_get_data(data, "band2");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band2", gain, NULL);

	vscale = g_object_get_data(data, "band3");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band3", gain, NULL);

	vscale = g_object_get_data(data, "band4");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band4", gain, NULL);

	vscale = g_object_get_data(data, "band5");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band5", gain, NULL);

	vscale = g_object_get_data(data, "band6");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band6", gain, NULL);

	vscale = g_object_get_data(data, "band7");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band7", gain, NULL);

	vscale = g_object_get_data(data, "band8");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band8", gain, NULL);

	vscale = g_object_get_data(data, "band9");
	gain = gtk_range_get_value(GTK_RANGE(vscale));
	g_object_set(G_OBJECT(cwin->cgst->equalizer), "band9", gain, NULL);
}

static gboolean
vscales_eq_set_by_user(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data)
{
	GtkWidget *eq_combobox;

	backend_update_eq(data);

	/* Set "custum" in combo */
	eq_combobox = g_object_get_data(data, "eq_combobox");
	gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 19);

	return FALSE;
}

void
eq_combobox_activated_cb (GtkComboBox *widget, gpointer data)
{
	gint option = 0;
	GtkWidget *vscale;

	gdouble value[][10] =
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

	vscale = g_object_get_data(data, "band0");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][0]);

	vscale = g_object_get_data(data, "band1");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][1]);

	vscale = g_object_get_data(data, "band2");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][2]);

	vscale = g_object_get_data(data, "band3");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][3]);

	vscale = g_object_get_data(data, "band4");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][4]);

	vscale = g_object_get_data(data, "band5");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][5]);

	vscale = g_object_get_data(data, "band6");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][6]);

	vscale = g_object_get_data(data, "band7");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][7]);

	vscale = g_object_get_data(data, "band8");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][8]);

	vscale = g_object_get_data(data, "band9");
	gtk_range_set_value(GTK_RANGE(vscale), value[option][9]);
	
	backend_update_eq(data);
}

void init_eq_preset(struct con_win *cwin, gpointer data)
{
	GtkWidget *eq_combobox, *vscale;
	gchar *eq_preset = NULL;
	gdouble *saved_bands;
	GError *error = NULL;

	eq_combobox = g_object_get_data(data, "eq_combobox");
	
	eq_preset = g_key_file_get_string(cwin->cpref->configrc_keyfile,
					  GROUP_AUDIO,
					  KEY_EQ_PRESET,
					  &error);
	if(eq_preset != NULL) {
		if (!g_ascii_strcasecmp(eq_preset, _("Disabled")))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 0);
		else if (!g_ascii_strcasecmp(eq_preset, "Classical"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 1);
		else if (!g_ascii_strcasecmp(eq_preset, "Club"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 2);
		else if (!g_ascii_strcasecmp(eq_preset, "Dance"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 3);
		else if (!g_ascii_strcasecmp(eq_preset, "Full Bass"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 4);
		else if (!g_ascii_strcasecmp(eq_preset, "Full Bass and Treble"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 5);
		else if (!g_ascii_strcasecmp(eq_preset, "Full Treble"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 6);
		else if (!g_ascii_strcasecmp(eq_preset, "Laptop Speakers and Headphones"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 7);
		else if (!g_ascii_strcasecmp(eq_preset, "Large Hall"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 8);
		else if (!g_ascii_strcasecmp(eq_preset, "Live"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 9);
		else if (!g_ascii_strcasecmp(eq_preset, "Party"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 10);
		else if (!g_ascii_strcasecmp(eq_preset, "Pop"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 11);
		else if (!g_ascii_strcasecmp(eq_preset, "Reggae"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 12);
		else if (!g_ascii_strcasecmp(eq_preset, "Rock"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 13);
		else if (!g_ascii_strcasecmp(eq_preset, "Ska"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 14);
		else if (!g_ascii_strcasecmp(eq_preset, "Smiley Face Curve"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 15);
		else if (!g_ascii_strcasecmp(eq_preset, "Soft"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 16);
		else if (!g_ascii_strcasecmp(eq_preset, "Soft Rock"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 17);
		else if (!g_ascii_strcasecmp(eq_preset, "Techno"))
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 18);
		else {
			gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 19);

			saved_bands = g_key_file_get_double_list(cwin->cpref->configrc_keyfile,
								 GROUP_AUDIO,
								 KEY_EQ_10_BANDS,
								 NULL,
								 &error);
			if (saved_bands != NULL) {
				vscale = g_object_get_data(data, "band0");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[0]);

				vscale = g_object_get_data(data, "band1");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[1]);

				vscale = g_object_get_data(data, "band2");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[2]);

				vscale = g_object_get_data(data, "band3");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[3]);

				vscale = g_object_get_data(data, "band4");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[4]);

				vscale = g_object_get_data(data, "band5");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[5]);

				vscale = g_object_get_data(data, "band6");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[6]);

				vscale = g_object_get_data(data, "band7");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[7]);

				vscale = g_object_get_data(data, "band8");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[8]);

				vscale = g_object_get_data(data, "band9");
				gtk_range_set_value(GTK_RANGE(vscale), saved_bands[9]);

				g_free(saved_bands);
			}
			else {
				g_error_free(error);
				error = NULL;
			}
		}
		g_free(eq_preset);
	}
	else {
		gtk_combo_box_set_active (GTK_COMBO_BOX(eq_combobox), 0);

		g_error_free(error);
		error = NULL;
	}
}

void save_eq_preset(struct con_win *cwin, gpointer data)
{
	gdouble *tmp_array;
	GtkWidget *eq_combobox, *vscale;
	gchar *presset = NULL;

	tmp_array = g_new (gdouble, 10);

	eq_combobox = g_object_get_data(data, "eq_combobox");

	presset = gtk_combo_box_get_active_text (GTK_COMBO_BOX(eq_combobox));

	g_key_file_set_string(cwin->cpref->configrc_keyfile,
			      GROUP_AUDIO,
			      KEY_EQ_PRESET,
			      presset);

	vscale = g_object_get_data(data, "band0");
	tmp_array[0] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band1");
	tmp_array[1] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band2");
	tmp_array[2] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band3");
	tmp_array[3] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band4");
	tmp_array[4] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band5");
	tmp_array[5] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band6");
	tmp_array[6] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band7");
	tmp_array[7] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band8");
	tmp_array[8] = gtk_range_get_value(GTK_RANGE(vscale));
	vscale = g_object_get_data(data, "band9");
	tmp_array[9] = gtk_range_get_value(GTK_RANGE(vscale));

	g_key_file_set_double_list (cwin->cpref->configrc_keyfile,
				    GROUP_AUDIO,
				    KEY_EQ_10_BANDS,
				    tmp_array,
				    10);
	g_free(tmp_array);
	g_free(presset);
}

gboolean eq_band_get_tooltip (GtkWidget        *vscale,
				 gint              x,
				 gint              y,
				 gboolean          keyboard_mode,
				 GtkTooltip       *tooltip,
				 gpointer data)
{
	gchar *text = NULL;

	text = g_strdup_printf("%.1lf", gtk_range_get_value(GTK_RANGE(vscale)));
	gtk_tooltip_set_text (tooltip, text);
	g_free(text);

	return TRUE;
}

void show_equalizer_action(GtkAction *action, struct con_win *cwin)
{
	GtkWidget *dialog;
	GtkWidget *mhbox, *hbox, *dbvbox, *label, *eq_combobox;
	GtkWidget *vscales[10];
	gpointer storage;
	gint i, result;

	/* Create vertical scales to equalizer */
	for(i=0 ; i < 10 ; i++) {
		vscales[i] = gtk_vscale_new_with_range(-24.0, 12.0, 0.1);

		gtk_range_set_inverted(GTK_RANGE(vscales[i]), TRUE);
		gtk_scale_set_draw_value (GTK_SCALE(vscales[i]), FALSE);
		g_object_set (G_OBJECT(vscales[i]), "has-tooltip", TRUE, NULL);
		g_signal_connect(G_OBJECT(vscales[i]), "query-tooltip",
				 G_CALLBACK(eq_band_get_tooltip),
				 NULL);
	}

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

	gtk_box_pack_start(GTK_BOX(mhbox), dbvbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("30 Hz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[0], FALSE, FALSE, 0);

	label = gtk_label_new("60 Hz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[1], FALSE, FALSE, 0);

	label = gtk_label_new("120 Hz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[2], FALSE, FALSE, 0);

	label = gtk_label_new("250 Hz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[3], FALSE, FALSE, 0);

	label = gtk_label_new("500 Hz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[4], FALSE, FALSE, 0);

	label = gtk_label_new("1 kHz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[5], FALSE, FALSE, 0);

	label = gtk_label_new("2 kHz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[6], FALSE, FALSE, 0);

	label = gtk_label_new("4 kHz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[7], FALSE, FALSE, 0);

	label = gtk_label_new("8 kHz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[8], FALSE, FALSE, 0);

	label = gtk_label_new("15 kHz");
	gtk_label_set_angle(GTK_LABEL(label), 90);
	gtk_misc_set_alignment (GTK_MISC(label), 1, 1);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vscales[9], FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(mhbox), hbox, TRUE, TRUE, 0);

	/* Create the dialog */

	dialog = gtk_dialog_new_with_buttons(_("Equalizer"),
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), -1, 200);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	/* Append list of default presets */

	eq_combobox = gtk_combo_box_new_text ();

	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), _("Disabled"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Classical");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Club");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Dance");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Full Bass");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Full Bass and Treble");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Full Treble");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Laptop Speakers and Headphones");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Large Hall");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Live");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Party");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Pop");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Reggae");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Rock");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Ska");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Smiley Face Curve");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Soft");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Soft Rock");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), "Techno");
	gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), _("Custom"));

	/* Set useful data */

	storage = g_object_new(G_TYPE_OBJECT, NULL);

	g_object_set_data(storage, "cwin", cwin);
	g_object_set_data(storage, "band0", vscales[0]);
	g_object_set_data(storage, "band1", vscales[1]);
	g_object_set_data(storage, "band2", vscales[2]);
	g_object_set_data(storage, "band3", vscales[3]);
	g_object_set_data(storage, "band4", vscales[4]);
	g_object_set_data(storage, "band5", vscales[5]);
	g_object_set_data(storage, "band6", vscales[6]);
	g_object_set_data(storage, "band7", vscales[7]);
	g_object_set_data(storage, "band8", vscales[8]);
	g_object_set_data(storage, "band9", vscales[9]);
	g_object_set_data(storage, "eq_combobox", eq_combobox);

	/* Conect the signals */

	for(i=0 ; i < 10 ; i++) {
		g_signal_connect(vscales[i], "change-value",
				 G_CALLBACK(vscales_eq_set_by_user), storage);
	}
	g_signal_connect(G_OBJECT(eq_combobox), "changed",
			 G_CALLBACK(eq_combobox_activated_cb), storage);

	/* Append and show the dialog */

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), eq_combobox, FALSE, FALSE, 0);
	gtk_button_box_set_child_secondary(GTK_BUTTON_BOX(GTK_DIALOG(dialog)->action_area), eq_combobox, TRUE);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), mhbox);

	if(cwin->cgst->equalizer == NULL) {
		gtk_widget_set_sensitive(GTK_WIDGET(hbox), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(eq_combobox), FALSE);
	}

	init_eq_preset(cwin, storage);

	gtk_widget_show_all(dialog);

	while ((result = gtk_dialog_run (GTK_DIALOG (dialog))) &&
		(result != GTK_RESPONSE_OK) &&
		(result != GTK_RESPONSE_DELETE_EVENT)) {
	}
	save_eq_preset(cwin, storage);

	gtk_widget_destroy(dialog);
}
