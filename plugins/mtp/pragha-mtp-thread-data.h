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

#include <libmtp.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

#include "src/pragha-musicobject.h"

#ifndef __PRAGHA_MTP_THREAD_DATA_H
#define __PRAGHA_MTP_THREAD_DATA_H

/*
 * PraghaMtpThreadOpenedData *
 */
typedef struct _PraghaMtpThreadOpenedData PraghaMtpThreadOpenedData;

PraghaMtpThreadOpenedData *
pragha_mtp_thread_opened_data_new (gpointer     user_data,
                                   const gchar *device_id,
                                   const gchar *friendly_name);

void
pragha_mtp_thread_opened_data_free (PraghaMtpThreadOpenedData *data);

gpointer
pragha_mtp_thread_opened_data_get_user_data (PraghaMtpThreadOpenedData *data);

const gchar *
pragha_mtp_thread_opened_data_get_device_id (PraghaMtpThreadOpenedData *data);

const gchar *
pragha_mtp_thread_opened_data_get_friendly_name (PraghaMtpThreadOpenedData *data);


/*
 * PraghaMtpThreadTracklistData *
 */
typedef struct _PraghaMtpThreadTracklistData PraghaMtpThreadTracklistData;

PraghaMtpThreadTracklistData *
pragha_mtp_thread_tracklist_data_new (gpointer  user_data,
                                      GList    *list);

void
pragha_mtp_thread_tracklist_data_free (PraghaMtpThreadTracklistData *data);

gpointer
pragha_mtp_thread_tracklist_data_get_user_data (PraghaMtpThreadTracklistData *data);

GList *
pragha_mtp_thread_tracklist_data_get_list (PraghaMtpThreadTracklistData *data);


/*
 * PraghaMtpThreadProgressData *
 */
typedef struct _PraghaMtpThreadProgressData PraghaMtpThreadProgressData;


PraghaMtpThreadProgressData *
pragha_mtp_thread_progress_data_new (gpointer user_data,
                                     guint    progress,
                                     guint    total);

void
pragha_mtp_thread_progress_data_free (PraghaMtpThreadProgressData *data);

gpointer
pragha_mtp_thread_progress_data_get_user_data (PraghaMtpThreadProgressData *data);

guint
pragha_mtp_thread_progress_data_get_progress (PraghaMtpThreadProgressData *data);

guint
pragha_mtp_thread_progress_data_get_total (PraghaMtpThreadProgressData *data);


/*
 * PraghaMtpThreadDownloadData *
 */
typedef struct _PraghaMtpThreadDownloadData  PraghaMtpThreadDownloadData;

PraghaMtpThreadDownloadData *
pragha_mtp_thread_download_data_new (gpointer     user_data,
                                     const gchar *filename,
                                     const gchar *error);

void
pragha_mtp_thread_download_data_free (PraghaMtpThreadDownloadData *data);

gpointer
pragha_mtp_thread_download_data_get_user_data (PraghaMtpThreadDownloadData *data);

const gchar *
pragha_mtp_thread_download_data_get_filename (PraghaMtpThreadDownloadData *data);

const gchar *
pragha_mtp_thread_download_data_get_error (PraghaMtpThreadDownloadData *data);


/*
 * PraghaMtpThreadUploadData *
 */
typedef struct _PraghaMtpThreadUploadData  PraghaMtpThreadUploadData;

PraghaMtpThreadUploadData *
pragha_mtp_thread_upload_data_new (gpointer           user_data,
                                   PraghaMusicobject *mobj,
                                   const gchar       *error);

void
pragha_mtp_thread_upload_data_free (PraghaMtpThreadUploadData *data);

gpointer
pragha_mtp_thread_upload_data_get_user_data (PraghaMtpThreadUploadData *data);

PraghaMusicobject *
pragha_mtp_thread_upload_data_get_musicobject (PraghaMtpThreadUploadData *data);

const gchar *
pragha_mtp_thread_upload_data_get_error (PraghaMtpThreadUploadData *data);


/*
 * PraghaMtpThreadStatsData *
 */
typedef struct _PraghaMtpThreadStatsData  PraghaMtpThreadStatsData;

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
                                  const gchar *error);

void
pragha_mtp_thread_stats_data_free (PraghaMtpThreadStatsData *data);

gpointer
pragha_mtp_thread_stats_data_get_user_data (PraghaMtpThreadStatsData *data);

const gchar *
pragha_mtp_thread_stats_data_get_first_storage_description (PraghaMtpThreadStatsData *data);

guint64
pragha_mtp_thread_stats_data_get_first_storage_capacity (PraghaMtpThreadStatsData *data);

guint64
pragha_mtp_thread_stats_data_get_first_storage_free_space (PraghaMtpThreadStatsData *data);

const gchar *
pragha_mtp_thread_stats_data_get_second_storage_description (PraghaMtpThreadStatsData *data);

guint64
pragha_mtp_thread_stats_data_get_second_storage_capacity (PraghaMtpThreadStatsData *data);

guint64
pragha_mtp_thread_stats_data_get_second_storage_free_space (PraghaMtpThreadStatsData *data);

guint8
pragha_mtp_thread_stats_data_get_maximun_battery_level (PraghaMtpThreadStatsData *data);

guint8
pragha_mtp_thread_stats_data_get_current_battery_level (PraghaMtpThreadStatsData *data);

const gchar *
pragha_mtp_thread_stats_data_get_error (PraghaMtpThreadStatsData *data);

#endif // __PRAGHA_MTP_THREAD_DATA_H

