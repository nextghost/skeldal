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

#include "TYPES.H"
#include "EVENT.H"
typedef struct tms_basic_info
  {
        int mouse_event;
        unsigned short mouse_code;
        unsigned short mouse_bx;
        unsigned short mouse_cx;
        unsigned short mouse_dx;
        signed short mouse_si;
        signed short mouse_di;
  }TMS_BASIC_INFO;

typedef struct ms_event
  {
   char event;
   word x,y;
   char tl1,tl2,tl3;
   word event_type;
  }MS_EVENT;

extern TMS_BASIC_INFO ms_basic_info;
extern char ms_fake_mode;

//int install_mouse_handler();
//int deinstall_mouse_handler();
//void hranice_mysky(int x1,int y1,int x2,int y2);
void get_ms_event(MS_EVENT *event);
int lock_region (void *address, unsigned length);
void keyboard(EVENT_MSG *msg,void *user_data);
char ms_get_keycount();
#endif
