/*
 *  This file is part of Skeldal project
 * 
 *  Skeldal is free software: you can redistribute 
 *  it and/or modify it under the terms of the GNU General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 3 of the License, or (at your option) any later version.
 *
 *  OpenSkeldal is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Skeldal.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  --------------------
 *
 *  Project home: https://sourceforge.net/projects/skeldal/
 *  
 *  Last commit made by: $Id$
 */
#ifndef __DEVICES_H
#define __DEVICES_H

#include <inttypes.h>
#include "libs/event.h"

#pragma pack(1)
typedef struct ms_event
  {
   int8_t event;
   uint16_t x,y;
   int8_t tl1,tl2,tl3;
   uint16_t event_type;
  }MS_EVENT;

extern char ms_fake_mode;

int lock_region (void *address, unsigned length);
void keyboard(EVENT_MSG *msg,void *user_data);
char ms_get_keycount();
#pragma option align=reset
#endif
