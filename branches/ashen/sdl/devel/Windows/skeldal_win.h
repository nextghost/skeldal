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
 *  Last commit made by: $Id: skeldal_win.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <windows.h>

#define BGSWITCHBIT 0x0020

#define SKELDALINI "WSKELDAL.INI"

#ifdef __cplusplus
extern "C"
  {
#endif


#define _KEYBRD_READY 0
#define _KEYBRD_READ 1

#define TIMERSPEED 20;

unsigned long _bios_keybrd(int mode);


LRESULT GameMainWindowWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void CheckMessageQueue();

void DSReportWindowCreation(HWND hWindow);
char *AutodetectWinAmp();

#ifdef __cplusplus
  }
#endif

#define RGB888(r,g,b) ((unsigned short)((((r)<<8)&0xF800) | (((g)<<3) & 0x7C0) | ((b)>>3)))
#define RGB555(r,g,b) ((unsigned short)(((r)<<11) | ((g)<<6) | (b)))

#pragma warning (disable : 4244 4761 4133)


void *LoadDefaultFont();
void *LoadResourceFont(const char *name);

void ShareCPU();
void SetWheelMapping(char up, char down);


//------------- BGRAPH DX wrapper -------------------
#include "BGraph2Dx.h"

#define WM_RELOADMAP (WM_APP+215)
#define E_RELOADMAP 40
