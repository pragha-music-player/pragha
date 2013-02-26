/* GStreamer
 * Copyright (C) 2003 Thomas Vander Stichele <thomas@apestaart.org>
 *               2003 Benjamin Otte <in7y118@public.uni-hamburg.de>
 *               2005 Andy Wingo <wingo@pobox.com>
 *               2005 Jan Schmidt <thaytan@mad.scientist.com>
 *
 * gst-metadata.c: Use GStreamer to display metadata within files.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "pragha-gst-metadata.h"

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <gst/gst.h>
#include <glib.h>

static gboolean
message_loop (GstElement * element, GstTagList ** tags)
{
	GstBus *bus;
	gboolean done = FALSE;

	bus = gst_element_get_bus (element);
	g_return_val_if_fail (bus != NULL, FALSE);
 	g_return_val_if_fail (tags != NULL, FALSE);

	while (!done) {
		GstMessage *message;

		message = gst_bus_pop (bus);
		if (message == NULL)
			/* All messages read, we're done */
			break;

		switch (GST_MESSAGE_TYPE (message)) {
			case GST_MESSAGE_ERROR:
			case GST_MESSAGE_EOS:
				gst_message_unref (message);
				return TRUE;
			case GST_MESSAGE_TAG: {
				GstTagList *new_tags, *old_tags;

				gst_message_parse_tag (message, &new_tags);
				if (*tags) {
					old_tags = *tags;
					*tags = gst_tag_list_merge (old_tags, new_tags, GST_TAG_MERGE_KEEP);
					#if GST_CHECK_VERSION (1, 0, 0)
					gst_tag_list_unref (old_tags);
					#else
					gst_tag_list_free (old_tags);
					#endif
			 	}
			 	else
					 *tags = new_tags;
				break;
			}
			default:
				break;
		}
		gst_message_unref (message);
	}
	gst_object_unref (bus);

	return TRUE;
}

static void
print_tag (const GstTagList * list, const gchar * tag, gpointer unused)
{
	gint i, count;

	count = gst_tag_list_get_tag_size (list, tag);

	for (i = 0; i < count; i++) {
		gchar *str;

	if (gst_tag_get_type (tag) == G_TYPE_STRING) {
		if (!gst_tag_list_get_string_index (list, tag, i, &str))
			g_assert_not_reached ();
		}
		else {
			str = g_strdup_value_contents (gst_tag_list_get_value_index (list, tag, i));
		}

		if (i == 0) {
			g_print ("  %15s: %s\n", gst_tag_get_nick (tag), str);
		} 
		else {
			g_print ("                 : %s\n", str);
		}
		g_free (str);
	}
}

void
pragha_metadata_parser_print_tag (PraghaGstMetadataParser *parser, const gchar *filename)
{
	GstStateChangeReturn sret;
	GstState state;
	GstTagList *tags = NULL;

	g_object_set (parser->source, "location", filename, NULL);

	GST_DEBUG ("Starting reading for %s", filename);

	/* Decodebin will only commit to PAUSED if it actually finds a type;
	 * otherwise the state change fails */
	sret = gst_element_set_state (GST_ELEMENT (parser->pipeline), GST_STATE_PAUSED);

	if (GST_STATE_CHANGE_ASYNC == sret) {
		if (GST_STATE_CHANGE_SUCCESS !=
			gst_element_get_state (GST_ELEMENT (parser->pipeline), &state, NULL,
				5 * GST_SECOND)) {
			g_print ("State change failed for %s. Aborting\n", filename);
			return;
		}
	}
	else if (sret != GST_STATE_CHANGE_SUCCESS) {
		g_print ("%s - Could not read file\n", filename);
		return;
	}

	if (!message_loop (GST_ELEMENT (parser->pipeline), &tags)) {
		g_print ("Failed in message reading for %s\n", filename);
	}

	if (tags) {
		g_print ("Metadata for %s:\n", filename);
		gst_tag_list_foreach (tags, print_tag, NULL);
		#if GST_CHECK_VERSION (1, 0, 0)
		gst_tag_list_unref (tags);
		#else
		gst_tag_list_free (tags);
		#endif
		tags = NULL;
	}
	else
		g_print ("No metadata found for %s\n", filename);

	sret = gst_element_set_state (GST_ELEMENT (parser->pipeline), GST_STATE_NULL);
	if (GST_STATE_CHANGE_ASYNC == sret) {
		if (GST_STATE_CHANGE_FAILURE ==
			gst_element_get_state (GST_ELEMENT (parser->pipeline),
				                   &state, NULL,
				                   GST_CLOCK_TIME_NONE)) {
			g_print ("State change failed. Aborting");
			return;
		}
	}
	else if (sret != GST_STATE_CHANGE_SUCCESS) {
		g_print ("State change failed. Aborting\n");
		return;
	}
}

void
pragha_metadata_parser_free(PraghaGstMetadataParser *parser)
{
	gst_object_unref (parser->pipeline);
	g_slice_free(PraghaGstMetadataParser, parser);
}

PraghaGstMetadataParser *
pragha_metadata_parser_new(void)
{
	GstElement *decodebin;
	GstElement *pipeline = NULL;
	GstElement *source = NULL;

	PraghaGstMetadataParser *parser;

	parser = g_slice_new0(PraghaGstMetadataParser);

	pipeline = gst_pipeline_new (NULL);
	source = gst_element_factory_make ("filesrc", "source");
	g_assert (GST_IS_ELEMENT (source));
	decodebin = gst_element_factory_make ("decodebin", "decodebin");
	g_assert (GST_IS_ELEMENT (decodebin));

	gst_bin_add_many (GST_BIN (pipeline), source, decodebin, NULL);
	gst_element_link (source, decodebin);

	parser->pipeline = pipeline;
	parser->source = source;

	return parser;
}

