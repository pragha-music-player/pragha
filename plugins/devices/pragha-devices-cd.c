/*************************************************************************/
/* Copyright (C) 2012-2014 matias <mati86dl@gmail.com>                   */
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

#include <gudev/gudev.h>
#include <stdlib.h>

#include "src/pragha-cdda.h"
#include "src/pragha-file-utils.h"
#include "src/pragha-utils.h"
#include "src/pragha-debug.h"
#include "src/pragha.h"

#include "pragha-devices-plugin.h"

void
pragha_devices_audio_cd_added (PraghaDevicesPlugin *plugin)
{
	gint response;
	response = pragha_gudev_show_dialog (NULL, _("Audio/Data CD"), "media-optical",
	                                     _("Was inserted an Audio Cd."), NULL,
	                                     _("Add Audio _CD"), PRAGHA_DEVICE_RESPONSE_PLAY);
	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_application_append_audio_cd (pragha_device_get_application(plugin));
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			break;
	}
}