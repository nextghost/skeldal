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
 *  Last commit made by: $Id: BMOUSE.H 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef __BMOUSE_H
#define __BMOUSE_H

#include "event.h"
#include "devices.h"


#define get_mouse(info) ((MS_EVENT *)(*(long *) info->data))

extern MS_EVENT ms_last_event;

short init_mysky();
short done_mysky();
void ukaz_mysku();
void schovej_mysku();
void zobraz_mysku();
void set_ms_finger(int x,int y);
void update_mysky(void);
char je_myska_zobrazena();
#endif
