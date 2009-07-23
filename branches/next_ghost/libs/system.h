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

#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef WIN32
#include "windows/system.h"
#else
#include "unix/system.h"
#endif

#define E_RELOADMAP 40
#define TIMERSPEED 20
#define SKELDALINI "wskeldal.ini"

void Sys_ErrorBox(const char *msg);
void Sys_WarnBox(const char *msg);
void Sys_InfoBox(const char *msg);

void Sys_Shutdown(void);

int Screen_GetXSize(void);
int Screen_GetYSize(void);
unsigned short *Screen_GetAddr(void);
unsigned short *Screen_GetBackAddr(void);
void Screen_SetAddr(unsigned short *addr);
void Screen_SetBackAddr();
void Screen_Restore(void);
void Screen_DrawRect(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys);
void Screen_DrawRectZoom2(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys);

char Sound_SetEffect(int filter, int data);
int Sound_GetEffect(int filter);
char Sound_IsActive(void);

int Timer_GetValue(void);
int Timer_GetTick(void);

int get_control_state(void);
int get_shift_state(void);

void *LoadDefaultFont();

#endif
