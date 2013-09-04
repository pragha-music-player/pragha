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

#ifndef PRAGHA_DEVICES_H
#define PRAGHA_DEVICES_H

#include <gudev/gudev.h>

/* pragha.h */
struct con_win;

typedef struct _PraghaDevices PraghaDevices;

enum
{
	PRAGHA_DEVICE_RESPONSE_NONE,
	PRAGHA_DEVICE_RESPONSE_PLAY,
	PRAGHA_DEVICE_RESPONSE_BROWSE,
};

enum
{
	PRAGHA_DEVICE_MOUNTABLE,
	PRAGHA_DEVICE_AUDIO_CD,
	PRAGHA_DEVICE_MTP,
	PRAGHA_DEVICE_UNKNOWN,
};

void           pragha_devices_free (PraghaDevices *devices);
PraghaDevices *pragha_devices_new  (struct con_win *cwin);

#endif /* PRAGHA_DEVICES_H */
