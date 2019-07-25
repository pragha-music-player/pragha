/*************************************************************************/
/* Copyright (C) 2019 matias <mati86dl@gmail.com>                        */
/*                                                                       */
/* Original code: Rhythmbox mtpdevice plugin                             */
/* Copyright (C) 2009 Jonathan Matthew  <jonathan@d14n.org>              */
/*  - https://gitlab.gnome.org/GNOME/rhythmbox                           */
/*  - file:///rhythmbox/plugins/mtpdevice/rb-mtp-thread.c                */
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

#ifndef __PRAGHA_MTP_THREAD_H
#define __PRAGHA_MTP_THREAD_H

#define PRAGHA_TYPE_MTP_THREAD         (pragha_mtp_thread_get_type ())
#define PRAGHA_MTP_THREAD(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_MTP_THREAD, PraghaMtpThread))
#define PRAGHA_MTP_THREAD_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k),     PRAGHA_TYPE_MTP_THREAD, PraghaMtpThreadClass))
#define PRAGHA_IS_MTP_THREAD(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_MTP_THREAD))
#define PRAGHA_IS_MTP_THREAD_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    PRAGHA_TYPE_MTP_THREAD))
#define PRAGHA_MTP_THREAD_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  PRAGHA_TYPE_MTP_THREAD, PraghaMtpThreadClass))

typedef struct _PraghaMtpThread      PraghaMtpThread;
typedef struct _PraghaMtpThreadClass PraghaMtpThreadClass;

/*
 * Public functions.
 */

PraghaMtpThread *
pragha_mtp_thread_new (void);

void
pragha_mtp_thread_open_device    (PraghaMtpThread           *thread,
                                  LIBMTP_raw_device_t       *raw_device,
                                  GSourceFunc                finish_func,
                                  gpointer                   user_data);

void
pragha_mtp_thread_get_stats      (PraghaMtpThread           *thread,
                                  GSourceFunc                finish_func,
                                  gpointer                   user_data);

void
pragha_mtp_thread_get_track_list (PraghaMtpThread           *thread,
                                  GSourceFunc                finish_func,
                                  GSourceFunc                progress_func,
                                  gpointer                   user_data);

void
pragha_mtp_thread_download_track (PraghaMtpThread           *thread,
                                  guint                      track_id,
                                  gchar                     *filename,
                                  GSourceFunc                finish_func,
                                  GSourceFunc                progress_func,
                                  gpointer                   data);

void
pragha_mtp_thread_upload_track   (PraghaMtpThread           *thread,
                                  PraghaMusicobject         *mobj,
                                  GSourceFunc                finish_func,
                                  gpointer                   data);

void
pragha_mtp_thread_close_device   (PraghaMtpThread          *thread,
                                  GSourceFunc               finish_func,
                                  gpointer                  data);

void
pragha_mtp_thread_report_errors  (PraghaMtpThread           *thread);

#endif // __PRAGHA_MTP_THREAD_H

