/*************************************************************************/
/* Copyright (C) 2019 matias <mati86dl@gmail.com>                        */
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

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "src/pragha-musicobject.h"

#include "pragha-mtp-thread-data.h"

/*
 * PraghaMtpThreadOpenedData *
 */
struct _PraghaMtpThreadOpenedData {
	gpointer  user_data;
	gchar    *device_id;
	gchar    *friendly_name;
};

PraghaMtpThreadOpenedData *
pragha_mtp_thread_opened_data_new (gpointer     user_data,
                                   const gchar *device_id,
                                   const gchar *friendly_name)
{
	PraghaMtpThreadOpenedData *data;

	data = g_slice_new (PraghaMtpThreadOpenedData);

	data->user_data = user_data;
	data->device_id = g_strdup(device_id);
	data->friendly_name = g_strdup(friendly_name);

	return data;
}

void
pragha_mtp_thread_opened_data_free (PraghaMtpThreadOpenedData *data)
{
	g_free (data->device_id);
	g_free (data->friendly_name);

	g_slice_free (PraghaMtpThreadOpenedData, data);
}

gpointer
pragha_mtp_thread_opened_data_get_user_data (PraghaMtpThreadOpenedData *data)
{
	return data->user_data;
}

const gchar *
pragha_mtp_thread_opened_data_get_device_id (PraghaMtpThreadOpenedData *data)
{
	return data->device_id;
}

const gchar *
pragha_mtp_thread_opened_data_get_friendly_name (PraghaMtpThreadOpenedData *data)
{
	return data->friendly_name;
}


/*
 * PraghaMtpThreadTracklistData *
 */
struct _PraghaMtpThreadTracklistData {
	gpointer user_data;
	GList   *list;
};

PraghaMtpThreadTracklistData *
pragha_mtp_thread_tracklist_data_new (gpointer  user_data,
                                      GList    *list)
{
	PraghaMtpThreadTracklistData *data;

	data = g_slice_new (PraghaMtpThreadTracklistData);

	data->user_data = user_data;
	data->list = list;

	return data;
}

void
pragha_mtp_thread_tracklist_data_free (PraghaMtpThreadTracklistData *data)
{
	g_slice_free (PraghaMtpThreadTracklistData, data);
}

gpointer
pragha_mtp_thread_tracklist_data_get_user_data (PraghaMtpThreadTracklistData *data)
{
	return data->user_data;
}

GList *
pragha_mtp_thread_tracklist_data_get_list (PraghaMtpThreadTracklistData *data)
{
	return data->list;
}


/*
 * PraghaMtpThreadProgressData *
 */
struct _PraghaMtpThreadProgressData {
	gpointer user_data;
	guint    progress;
	guint    total;
};

PraghaMtpThreadProgressData *
pragha_mtp_thread_progress_data_new (gpointer user_data,
                                     guint    progress,
                                     guint    total)
{
	PraghaMtpThreadProgressData *data;

	data = g_slice_new (PraghaMtpThreadProgressData);

	data->user_data = user_data;
	data->progress = progress;
	data->total = total;

	return data;
}

void
pragha_mtp_thread_progress_data_free (PraghaMtpThreadProgressData *data)
{
	g_slice_free (PraghaMtpThreadProgressData, data);
}

gpointer
pragha_mtp_thread_progress_data_get_user_data (PraghaMtpThreadProgressData *data)
{
	return data->user_data;
}

guint
pragha_mtp_thread_progress_data_get_progress (PraghaMtpThreadProgressData *data)
{
	return data->progress;
}

guint
pragha_mtp_thread_progress_data_get_total (PraghaMtpThreadProgressData *data)
{
	return data->total;
}


/*
 * PraghaMtpThreadDownloadData *
 */
struct _PraghaMtpThreadDownloadData {
	gpointer  user_data;
	gchar    *filename;
	gchar    *error;
};

PraghaMtpThreadDownloadData *
pragha_mtp_thread_download_data_new (gpointer     user_data,
                                     const gchar *filename,
                                     const gchar *error)
{
	PraghaMtpThreadDownloadData *data;

	data = g_slice_new (PraghaMtpThreadDownloadData);

	data->user_data = user_data;
	data->filename = g_strdup(filename);
	data->error = g_strdup (error);

	return data;
}

void
pragha_mtp_thread_download_data_free (PraghaMtpThreadDownloadData *data)
{
	g_free (data->filename);
	g_free (data->error);

	g_slice_free (PraghaMtpThreadDownloadData, data);
}

gpointer
pragha_mtp_thread_download_data_get_user_data (PraghaMtpThreadDownloadData *data)
{
	return data->user_data;
}

const gchar *
pragha_mtp_thread_download_data_get_filename (PraghaMtpThreadDownloadData *data)
{
	return data->filename;
}

const gchar *
pragha_mtp_thread_download_data_get_error (PraghaMtpThreadDownloadData *data)
{
	return data->error;
}


/*
 * PraghaMtpThreadUploadData *
 */
struct _PraghaMtpThreadUploadData {
	gpointer           user_data;
	PraghaMusicobject *mobj;
	gchar             *error;
};

PraghaMtpThreadUploadData *
pragha_mtp_thread_upload_data_new (gpointer           user_data,
                                   PraghaMusicobject *mobj,
                                   const gchar       *error)
{
	PraghaMtpThreadUploadData *data;

	data = g_slice_new (PraghaMtpThreadUploadData);

	data->user_data = user_data;
	if (mobj)
		data->mobj = g_object_ref(mobj);
	data->error = g_strdup (error);

	return data;
}

void
pragha_mtp_thread_upload_data_free (PraghaMtpThreadUploadData *data)
{
	if (data->mobj)
		g_object_unref (data->mobj);
	g_free (data->error);

	g_slice_free (PraghaMtpThreadUploadData, data);
}

gpointer
pragha_mtp_thread_upload_data_get_user_data (PraghaMtpThreadUploadData *data)
{
	return data->user_data;
}

PraghaMusicobject *
pragha_mtp_thread_upload_data_get_musicobject (PraghaMtpThreadUploadData *data)
{
	return data->mobj;
}

const gchar *
pragha_mtp_thread_upload_data_get_error (PraghaMtpThreadUploadData *data)
{
	return data->error;
}

/*
 * PraghaMtpThreadStatsData *
 */
struct _PraghaMtpThreadStatsData {
	gpointer  user_data;
	gchar    *first_storage_description;
	guint64   first_storage_capacity;
	guint64   first_storage_free_space;
	gchar    *second_storage_description;
	guint64   second_storage_capacity;
	guint64   second_storage_free_space;
	guint8    maximum_battery_level;
	guint8    current_battery_level;
	gchar    *error;
};

PraghaMtpThreadStatsData *
pragha_mtp_thread_stats_data_new (gpointer     user_data,
                                  const gchar *first_storage_description,
                                  guint64      first_storage_capacity,
                                  guint64      first_storage_free_space,
                                  const gchar *second_storage_description,
                                  guint64      second_storage_capacity,
                                  guint64      second_storage_free_space,
                                  guint8       maximum_battery_level,
                                  guint8       current_battery_level,
                                  const gchar *error)
{
	PraghaMtpThreadStatsData *data;

	data = g_slice_new (PraghaMtpThreadStatsData);

	data->user_data = user_data;
	data->first_storage_description = g_strdup (first_storage_description);
	data->first_storage_capacity = first_storage_capacity;
	data->first_storage_free_space = first_storage_free_space;
	data->second_storage_description = g_strdup (second_storage_description);
	data->second_storage_capacity = second_storage_capacity;
	data->second_storage_free_space = second_storage_free_space;
	data->maximum_battery_level = maximum_battery_level;
	data->current_battery_level = current_battery_level;
	data->error = g_strdup (error);

	return data;
}

void
pragha_mtp_thread_stats_data_free (PraghaMtpThreadStatsData *data)
{
	g_free (data->first_storage_description);
	g_free (data->second_storage_description);
	g_free (data->error);

	g_slice_free (PraghaMtpThreadStatsData, data);
}

gpointer
pragha_mtp_thread_stats_data_get_user_data (PraghaMtpThreadStatsData *data)
{
	return data->user_data;
}

const gchar *
pragha_mtp_thread_stats_data_get_first_storage_description (PraghaMtpThreadStatsData *data)
{
	return data->first_storage_description;
}

guint64
pragha_mtp_thread_stats_data_get_first_storage_capacity (PraghaMtpThreadStatsData *data)
{
	return data->first_storage_capacity;
}

guint64
pragha_mtp_thread_stats_data_get_first_storage_free_space (PraghaMtpThreadStatsData *data)
{
	return data->first_storage_free_space;
}

const gchar *
pragha_mtp_thread_stats_data_get_second_storage_description (PraghaMtpThreadStatsData *data)
{
	return data->second_storage_description;
}

guint64
pragha_mtp_thread_stats_data_get_second_storage_capacity (PraghaMtpThreadStatsData *data)
{
	return data->second_storage_capacity;
}

guint64
pragha_mtp_thread_stats_data_get_second_storage_free_space (PraghaMtpThreadStatsData *data)
{
	return data->second_storage_free_space;
}

guint8
pragha_mtp_thread_stats_data_get_maximun_battery_level (PraghaMtpThreadStatsData *data)
{
	return data->maximum_battery_level;
}

guint8
pragha_mtp_thread_stats_data_get_current_battery_level (PraghaMtpThreadStatsData *data)
{
	return data->current_battery_level;
}

const gchar *
pragha_mtp_thread_stats_data_get_error (PraghaMtpThreadStatsData *data)
{
	return data->error;
}

