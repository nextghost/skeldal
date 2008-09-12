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
 *  Last commit made by: $Id: BGraph2Dx.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef __BGRAPH_DX_WRAPPER_
#define __BGRAPH_DX_WRAPPER_

#ifdef __cplusplus
extern "C" {
#endif

	extern long scr_linelen;
	extern long scr_linelen2;
	extern long dx_linelen;


	//inicializuje a otevira rezim 640x480x16b v DX - otevre okno, pripravi vse pro beh hry
	//Vraci 1 pri uspechu
	char DXInit64(char inwindow,int zoom,int monitor, int refresh); 

	//uzavre rezim grafiky
	void DXCloseMode();

	//void DXCopyRects32(unsigned short x,unsigned short y,unsigned short xs,unsigned short ys);
	void DXCopyRects64(unsigned short x,unsigned short y,unsigned short xs,unsigned short ys);
	void DXCopyRects64zoom2(unsigned short x,unsigned short y,unsigned short xs,unsigned short ys);

	void *DxPrepareWalk(int ypos);
	void DxZoomWalk(void *handle, int ypos, int *points,float phase, void *lodka);
	void DxDoneWalk(void *handle);

	void *DxPrepareTurn(int ypos);
	void DxTurn(void *handle, char right, int ypos,int border, float phase, void *lodka);
	void DxDoneTurn(void *handle);
	void DxTurnLeftRight(char right, float phase, int border, int ypos, int *last);


	void DxDialogs(char enable);

	void setvesa_displaystart(int x,int y);

	extern long scr_linelen;
	extern long scr_linelen2;

	void DXMouseTransform(unsigned short *x, unsigned short *y);

	HWND GetGameWindow();
	void DxLockBuffers(BOOL lock);

	void StripBlt(void *data, unsigned int startline, unsigned long width);


#ifdef __cplusplus
}
#endif


#endif
