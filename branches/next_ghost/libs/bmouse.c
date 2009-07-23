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
//#include <skeldal_win.h>
#include "libs/types.h"
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include <stdio.h>

char visible=0;
MS_EVENT ms_last_event;
integer h_x,h_y=0;


void ukaz_mysku()
  {
  if (!visible) return;
  visible--;
  if (!visible)
     {
     show_ms_cursor(ms_last_event.x-h_x,ms_last_event.y-h_y);
     }
  }

void schovej_mysku()
  {
  if (!visible)
     hide_ms_cursor();
  visible++;
  }

void zobraz_mysku()
  {
  visible=1;
  ukaz_mysku();
  }

void ms_idle_event(EVENT_MSG *info,void *user_data)
  {
  void *i;MS_EVENT x;
  user_data;info;
  if (info->msg==E_WATCH)
     {
     *otevri_zavoru=1;
     Mouse_GetEvent(&x);
     if (x.event)
       {
       ms_last_event=x;
       i=&ms_last_event;
       *otevri_zavoru=1;
       send_message(E_MOUSE,i);
       }
     }
  }

void ms_draw_event(EVENT_MSG *info,void *user_data)
  {
  MS_EVENT *ms_ev;

  user_data;
  if (info->msg==E_MOUSE)
     {
     ms_ev=get_mouse(info);
     if (ms_ev->event_type & 1)
       if (!visible) move_ms_cursor(ms_ev->x-h_x,ms_ev->y-h_y,0);
     }
  }


void update_mysky(void)
  {
  MS_EVENT x;

  Mouse_GetEvent(&x);
  if (x.event)
     {
     ms_last_event=x;
     }
  if(!visible) move_ms_cursor(x.x-h_x,x.y-h_y,0);
  }

char je_myska_zobrazena()
  {
  return !visible;
  }


void set_ms_finger(int x,int y)
  {
  h_x=x;
  h_y=y;
  }

void *mouse()
 {
 send_message(E_ADD,E_WATCH,ms_idle_event);
 send_message(E_ADD,E_MOUSE,ms_draw_event);
 return NULL;
 }

short init_mysky()
  {

//  i=install_mouse_handler();
//  hranice_mysky(0,0,639,479);
  visible=1;
  send_message(E_INIT,mouse);
  return 0;
  }

short done_mysky()
  {

//  i=deinstall_mouse_handler();
  send_message(E_DONE,E_WATCH,ms_idle_event);
  send_message(E_DONE,E_MOUSE,ms_draw_event);
  return 0;
  }

