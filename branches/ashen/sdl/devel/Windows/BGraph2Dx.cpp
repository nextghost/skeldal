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
 *  Last commit made by: $Id: BGraph2Dx.cpp 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <windows.h>
#include <stdio.h>
#define DWORD_PTR DWORD *
//#include <d3d9.h>
#include <debug.h>
#include "Skeldal_win.h"
#include "resource.h"
#include <SDL/SDL.h>
#include <string>
#define SKELDALCLASSNAME "BranySkeldalDXWindow"
#define INWINDOW runinwindow
#define SEND_LOG(format,parm2) fprintf(stderr,"%s "format"\n",parm2),fflush(stderr)

static int dxWindowZoom = 1;

//static HWND hMainWnd;
static SDL_Surface *hMainWnd;
// static IDirect3D9 *DxHandle;

//static IDirect3DDevice9 *DxDevice;
//static IDirect3DSurface9 *DxBackBuffer;
static SDL_Surface *DxBackBuffer;
//static D3DPRESENT_PARAMETERS pparm;

static unsigned short *mainBuffer = NULL;
static unsigned short *secondBuffer = NULL;
static unsigned short *curBuffer = NULL;
static unsigned long main_linelen;

static bool dialogs = false;
static bool runinwindow = false;
static int shiftscrx = 0, shiftscry = 0;

void DXMouseTransform(unsigned short *x, unsigned short *y)
{
	*x = *x*2/dxWindowZoom;
	*y = *y*2/dxWindowZoom;
}


extern "C" 
{
	/* 
	 * Set window's coordinates when in restored or maximized position
	 * Used for shake effect in chveni() 
	 */
/*
 * Disabled also in GAME/BUILDER.C !
 * void setvesa_displaystart(int x,int y)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(hMainWnd,&wp);
		wp.ptMaxPosition.x += x-shiftscrx;
		wp.ptMaxPosition.y += y-shiftscry;
		wp.rcNormalPosition.left += x-shiftscrx;
		wp.rcNormalPosition.top += y-shiftscry;
		wp.rcNormalPosition.right += x-shiftscrx;
		wp.rcNormalPosition.bottom += y-shiftscry;
		shiftscrx = x;
		shiftscry = y;
		SetWindowPlacement(hMainWnd,&wp);
	}
*/

#ifndef WINDOWCLASSFLAGS 
#define WINDOWCLASSFLAGS 0
#endif
/* Registers a window class for subsequent use in call to CreateSkeldalWindow() */
/*	static void RegisterWindowClass()
	{
		WNDCLASSEX cls;
		cls.cbSize = sizeof(cls);
		cls.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC|WINDOWCLASSFLAGS ;
		cls.lpfnWndProc = (WNDPROC)GameMainWindowWindowProc;
		cls.cbClsExtra = 0;
		cls.cbWndExtra = 0;
		cls.hInstance = GetModuleHandle(NULL);
		cls.hIcon = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MAINICON));;
		cls.hCursor = NULL;
		cls.hbrBackground = NULL;
		cls.lpszMenuName = NULL;
		cls.lpszClassName = SKELDALCLASSNAME;
		cls.hIconSm = NULL;
		if (RegisterClassEx(&cls) == NULL) 
		{
			MessageBox(NULL,"RegisterClassFailed",NULL,MB_OK);
			exit(0);
		}
	}
*/
	static void CreateSkeldalWindow()
	{
		//char buff[256];

		//LoadString(GetModuleHandle(NULL),IDS_WINTITLE,buff,sizeof(buff));
//		strcpy(buff, "Skeldal, SDL version");

//		RECT rc = {0,0,640*dxWindowZoom/2,480*dxWindowZoom/2};
//		DWORD flags = (INWINDOW?(WS_OVERLAPPED|WS_SYSMENU|WS_MINIMIZEBOX|WS_CAPTION|WS_BORDER):(WS_POPUP|WS_SYSMENU))|WS_VISIBLE;
//		AdjustWindowRect(&rc,flags,FALSE);
		//hMainWnd = CreateWindow(SKELDALCLASSNAME,buff,flags,100,100,rc.right-rc.left,rc.bottom-rc.top,NULL,NULL,GetModuleHandle(NULL),NULL);
		hMainWnd = SDL_SetVideoMode(640, 480, 24, SDL_SWSURFACE);
		if (hMainWnd == NULL)
		{
		//	MessageBox(NULL,"WindowCreationFailed",NULL,MB_OK);
			exit(0);
		}  
		SDL_WM_SetCaption("The Gates of Skeldal - SDL version", NULL);
		//UpdateWindow(hMainWnd);  
		//SDL_Flip(hMainWnd);
		SDL_UpdateRect(hMainWnd, 0, 0, 0, 0);
	}

	/* 
	 * retrieves information about one of the graphics modes for a display device 
	 * and changes the the setting to it 
	 */


	/* Shows logo.bmp */
	static bool ShowLogo()
	{
	//	HBITMAP logo = (HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_INITLOGO),IMAGE_BITMAP,0,0,0);
		std::string filename = "logo.bmp";
		SEND_LOG("Entering ShowLogo() function",0);
		SDL_Surface *logo = NULL; 
		logo = SDL_LoadBMP(filename.c_str());
		SEND_LOG("Loading logo.bmp",0);
		if (logo == NULL) {
			SEND_LOG("logo.bmp NOT FOUND! ",0);
			SDL_Delay(5000);
		}
		SEND_LOG("ShowLogo(): Logo successfully loaded. ",0);
		SDL_Surface *optimized = SDL_DisplayFormat(logo);
		SDL_FreeSurface(logo);
		SDL_BlitSurface(optimized, NULL, hMainWnd, NULL);
		SDL_FreeSurface(optimized);

		//SDL_CreateRGBSurfaceF

		/* HDC hDC;
		CheckResult(DxBackBuffer->GetDC(&hDC));
		HDC hBitmap = CreateCompatibleDC(hDC);
		HBITMAP old = (HBITMAP)SelectObject(hBitmap,logo);
		BitBlt(hDC,0,0,640,480,hBitmap,0,0,SRCCOPY);
		SelectObject(hBitmap,old);
		DeleteDC(hBitmap);
		DxBackBuffer->ReleaseDC(hDC);
		DeleteObject(logo); */
		return true;
	}

	 /*
	 * Initializates and opens 640x480x16b mode in DX 
	 * Returns 1 on success
	 * inwindow - 1 = run in window/ 0 = fullscreen
	 */ 
	char DXInit64(char inwindow, int zoom, int monitor, int refresh)
	{
		runinwindow = inwindow != 0;
		dxWindowZoom = inwindow?zoom+1:2;
		//SDL_Init();
//		RegisterWindowClass();
		CreateSkeldalWindow();  

		// DxHandle = Direct3DCreate9(D3D_SDK_VERSION);
		/* if (DxHandle == NULL)
		{
			MessageBox(hMainWnd,"Nepodarilo se inicializovat DirectX. "
					"Preinstalujte prosim DirectX ze stranek www.microsoft.com/directx. "
					"Ke spusteni je potreba mit aspon verzi DirectX 9.0c",NULL,MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}*/
/*
		HMONITOR mon = DxHandle->GetAdapterMonitor(monitor);
		if (mon != NULL)
		{
			MONITORINFO moninfo;
			moninfo.cbSize = sizeof(moninfo);
			GetMonitorInfo(mon,&moninfo);
			SetWindowPos(hMainWnd,NULL,moninfo.rcWork.left,moninfo.rcWork.top,0,0,SWP_NOZORDER|SWP_NOSIZE);    
		}
*/
		/*
		pparm.BackBufferWidth = 640;
		pparm. BackBufferHeight = 480;
		pparm.BackBufferFormat = D3DFMT_R5G6B5;
		pparm.BackBufferCount = 1;
		pparm.MultiSampleType = D3DMULTISAMPLE_NONE;
		pparm.SwapEffect = D3DSWAPEFFECT_COPY;
		pparm.hDeviceWindow = hMainWnd;
		pparm.Windowed = INWINDOW;
		pparm.EnableAutoDepthStencil = FALSE;
		pparm.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		pparm.FullScreen_RefreshRateInHz = refresh;
		pparm.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE ;
		pparm.MultiSampleQuality = 0;

		HRESULT res;

		res = DxHandle->CreateDevice(monitor,DXDEVICE_TYPE,hMainWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pparm,&DxDevice);  
		if (res != 0 && inwindow)
		{
			DisplayMode(1);    
			res = DxHandle->CreateDevice(monitor,DXDEVICE_TYPE,hMainWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pparm,&DxDevice);  
		}
		if (res != 0)
		{
			res = DxHandle->CreateDevice(monitor,D3DDEVTYPE_REF,hMainWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pparm,&DxDevice);  
		}
		if (res != 0)
		{    
			CheckResult(res);
			return 0;
		}
*/
		//res = DxDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&DxBackBuffer);

//		CheckResult(res);
		
		bool logo;
		logo = ShowLogo();
		if(!logo)SEND_LOG("logo: %i", logo);

Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif

		DxBackBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 640 , 480 , 32, rmask, gmask, bmask, amask);
		if (DxBackBuffer == NULL) {
			SEND_LOG("SDL_CreateRGBSurface failed",0);
			return -1;
		}

		SEND_LOG("SDL_CreateRGBSurface succeeded.",0);
//		D3DLOCKED_RECT lrc;
	
//lrc.Pitch = 640/
		main_linelen = dx_linelen = scr_linelen = 640;
		scr_linelen2= scr_linelen/2;

		curBuffer = mainBuffer = (unsigned short *)DxBackBuffer->pixels;
		SEND_LOG("secondBuffer: DxBackBuffer->pitch:" , 0);
		// SEND_LOG("secondBuffer: DxBackBuffer->pitch: %d", DxBackBuffer->pitch);
		secondBuffer = (unsigned short *)malloc(DxBackBuffer->pitch*480);

		if (logo)
		{
			//InvalidateRect(hMainWnd,NULL,TRUE);
			SDL_UpdateRect(hMainWnd, 0, 0, 0, 0);
			/*
			DWORD tm = GetTickCount()+5000;
			while (tm>GetTickCount()) {Sleep(100);CheckMessageQueue();}
			*/
			SDL_Delay(5000);
		}
		SEND_LOG("Logo showed", 0);
		return 1;
	}

	/*
	 * deactivates and removes focus,
	 * closes main window
	 */
	void DXCloseMode()
	{
		if (DxBackBuffer != NULL) SDL_FreeSurface(DxBackBuffer);
		SDL_FreeSurface(hMainWnd);
		free(secondBuffer);
	}


/*	static void HandleDeviceLost()
	{
		//HRESULT res = D3DERR_DEVICELOST;
		SDL_FreeSurface(DxBackBuffer);
	//	DxBackBuffer->Release();
		DxBackBuffer = NULL;
		
		//res = DxDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&DxBackBuffer);    
		//CheckResult(res);
//		res = DxDevice->Present(NULL,NULL,hMainWnd,NULL);
		SDL_UpdateRect(hMainWnd, 0, 0, 0, 0);
	}
*/
	/*
	 * Presents the contents of the next buffer in the sequence of back buffers owned by the device.
	 * Draw prc rectangle into hMainWnd
	 */
	static void GlobalPresent(SDL_Rect *prc)
	{
		SDL_Rect rc = *prc;
		//SDL_Rect winrc = rc;

		//int z = dxWindowZoom/2;
		/*if (dxWindowZoom != 2)
		{
			rc.left -= z;
			rc.top -= z;
			rc.right += z;
			rc.bottom += z;
			if (rc.left<0) rc.left = 0;
			if (rc.top<0) rc.top = 0;
			if (rc.right>= winrc.right) rc.right = winrc.right;
			if (rc.bottom>= winrc.bottom) rc.bottom = winrc.bottom;
			rc.left &=~1;
			rc.top &=~1;
			rc.right &=~1;
			rc.bottom &=~1;
			winrc = rc;  
			winrc.left = winrc.left*dxWindowZoom/2;
			winrc.top = winrc.top*dxWindowZoom/2;
			winrc.right = winrc.right*dxWindowZoom/2;
			winrc.bottom = winrc.bottom*dxWindowZoom/2;  
		}*/
		if (!dialogs)
		{
		// !!!!	
		SDL_BlitSurface(hMainWnd, &rc, DxBackBuffer, &rc);
		//	LRESULT res = DxDevice->Present(&rc,&winrc,hMainWnd,NULL);
		//	if (res == D3DERR_DEVICELOST) HandleDeviceLost();
		}
	}


	static bool UnLockBuffers()
	{
		if (DxBackBuffer == NULL) return false;
		//HRESULT res;
		SDL_UnlockSurface(DxBackBuffer);
//		res = DxBackBuffer->UnlockRect();
//		CheckResult(res);
		return true;
	}

	static void LockBuffers()
	{
		//D3DLOCKED_RECT lrc;
		//HRESULT res;
		//res = DxBackBuffer->LockRect(&lrc,NULL,0);
		SDL_LockSurface(DxBackBuffer);
//		CheckResult(res);

		curBuffer = mainBuffer = (unsigned short *)DxBackBuffer->pixels;
		if (dx_linelen != DxBackBuffer->pitch)
		{
			free(secondBuffer);
			main_linelen = dx_linelen = scr_linelen = (long )DxBackBuffer->pixels;
			scr_linelen2= scr_linelen/2;
			curBuffer = mainBuffer = (unsigned short *)DxBackBuffer->pixels;
			secondBuffer = (unsigned short *) malloc(DxBackBuffer->pitch*480);
		}
	}

	void DXCopyRects64(unsigned short x,unsigned short y,unsigned short xs,unsigned short ys)
	{
		if (UnLockBuffers() == false) return;

		SDL_Rect rc;
		rc.x = x;
		rc.y = y;
		rc.w= x+xs;
		rc.h = y+ys;
		GlobalPresent(&rc);
		LockBuffers();

	}

	unsigned short *GetScreenAdr()
	{
		return curBuffer;
	}

	unsigned short *GetBuffer2nd()
	{
		return secondBuffer;
	}

	void RedirectScreen(unsigned short *newaddr)
	{
		curBuffer = newaddr;
	}

	void RestoreScreen()
	{
		curBuffer = mainBuffer;
		scr_linelen = main_linelen;
		scr_linelen2= main_linelen>>1;

	}


	void RedirectScreenBufferSecond()
	{
		curBuffer = secondBuffer;
	}


/*	void DXCopyRects64zoom2(unsigned short x,unsigned short y,unsigned short xs,unsigned short ys)
	{

		HRESULT res;
		if (UnLockBuffers() == false) return;


		RECT rc;
		rc.left = x;
		rc.top = y;
		rc.right = x+xs;
		rc.bottom = y+ys;
		RECT zoom;
		zoom.left = x;
		zoom.top = y;
		zoom.right = x+2*xs;
		zoom.bottom = y+2*ys;

		res = DxDevice->Present(&rc,&zoom,hMainWnd,NULL);
		if (res == D3DERR_DEVICELOST) HandleDeviceLost();


		LockBuffers();  
	}
*/

	void *DxPrepareWalk(int ypos)
	{

//		IDirect3DSurface9 *tempbuff;
Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif
		SDL_Surface *tempbuff = SDL_CreateRGBSurface(SDL_SWSURFACE, 640 , 360, 32, rmask, gmask, bmask, amask);

//		res = DxDevice->CreateRenderTarget(640,360,D3DFMT_R5G6B5,D3DMULTISAMPLE_NONE,0,TRUE,&tempbuff,NULL);
		//SDL_Rect = 
		//CheckResult(res);

		UnLockBuffers();
		SDL_Rect src;src.x = 0;src.y = ypos;src.w = 640;src.h = ypos+360;
		SDL_Rect trg;trg.x = 0;trg.y = 0;trg.w = 640;trg.h = 360;
		
		//res = DxDevice->StretchRect(DxBackBuffer,&src,tempbuff,&trg,D3DTEXF_NONE);
		SDL_BlitSurface(DxBackBuffer, &src, tempbuff, &trg);
		LockBuffers();

		return (void *)tempbuff;
	}

	void DxZoomWalk(void *handle, int ypos, int *points,float phase, void *lodka)
	{
		if (phase>1.0) phase = 1.0f;
		if (phase<0.0) phase = 0.0f;
		UnLockBuffers();
		SDL_Surface *surf = (SDL_Surface *)handle;
		SDL_Rect rc1;rc1.x = 0;rc1.y = 0;rc1.w = 640;rc1.h = 360;
		SDL_Rect rc2;rc2.x = points[0];rc2.y = points[1];rc2.w = points[2];rc2.h = points[3];
		SDL_Rect rcx;
		rcx.x = (int)(rc1.x +(rc2.x-rc1.x)*phase);
		rcx.y = (int)(rc1.y +(rc2.y -rc1.y )*phase);
		rcx.w = (int)(rc1.w+(rc2.w-rc1.w)*phase);
		rcx.h = (int)(rc1.h +(rc2.h-rc1.h)*phase);
		rc1.y += ypos;
		rc1.h += ypos;

		/*  HDC srcdc;
		    HDC trgdc;
		    HRESULT res;
		    res = surf->GetDC(&srcdc);CheckResult(res);
		    res = DxBackBuffer->GetDC(&trgdc);CheckResult(res);
		    StretchBlt(
		    trgdc,rc1.left,rc1.top,rc1.right-rc1.left,rc1.bottom-rc1.top,
		    srcdc,rcx.left,rcx.top,rcx.right-rcx.left,rcx.bottom-rcx.top,SRCCOPY);
		    surf->ReleaseDC(srcdc);
		    DxBackBuffer->ReleaseDC(trgdc);
		    */


//		res = DxDevice->StretchRect(surf,&rcx,DxBackBuffer,&rc1,D3DTEXF_NONE);
		SDL_BlitSurface(surf, &rcx, DxBackBuffer, &rc1);

		GlobalPresent(&rc1);
		LockBuffers();
	}

	void DxDoneWalk(void *handle)
	{
		SDL_Surface *surf = (SDL_Surface *)handle;
		if (surf) SDL_FreeSurface(surf);
	}



	void DxTurnLeftRight(char right, float phase, int border, int ypos, int *last)
	{
		if (phase>1.0) phase = 1.0f;
		if (phase<0.0) phase = 0.0f;
		if (phase<0.05f) return;
		int space = 640-2*border;

		unsigned short *scr = GetScreenAdr()+scr_linelen2*ypos;
		unsigned short *buf = GetBuffer2nd()+scr_linelen2*ypos;  

		int turnout = (float)(space*phase)+border;
		int difto = turnout-*last;
		if (!right)
		{
			buf += border;
			for (int y = 0;y<360;y++)
			{
				memcpy(scr,scr+difto,(640-turnout)*2);
				memcpy(scr+(640-turnout),buf,(turnout)*2);
				scr += scr_linelen2;
				buf += scr_linelen2;
			}      
			*last = turnout;
		}
		else
		{
			for (int y = 0;y<360;y++)
			{
				memmove(scr+*last+difto-border,scr+*last-border,(640-turnout+border)*2);
				memcpy(scr,buf+(640-turnout)-border,turnout*2);
				scr += scr_linelen2;
				buf += scr_linelen2;
			}
			*last = turnout;
		}
		DXCopyRects64(0,ypos,640,360+ypos);
	}

	void *DxPrepareTurn(int ypos)
	{
		//IDirect3DSurface9 **handle = new IDirect3DSurface9 *[2];
		SDL_Surface *handle[2];

Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif
		handle[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, 640 , 360, 32, rmask, gmask, bmask, amask);
	//	res = DxDevice->CreateRenderTarget(640,360,D3DFMT_R5G6B5,D3DMULTISAMPLE_NONE,0,TRUE,handle,NULL);
		UnLockBuffers();
		
		SDL_Rect src;src.x = 0;src.y = ypos;src.w = 640;src.h = ypos+360;
		SDL_Rect trg;trg.x = 0;trg.y = 0;trg.w = 640;trg.h = 360;
	
		//res = DxDevice->StretchRect(DxBackBuffer,&src,handle[0],&trg,D3DTEXF_NONE);

		SDL_BlitSurface(DxBackBuffer, &src, handle[0], &trg);


		handle[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 640 , 360, 32, rmask, gmask, bmask, amask);
//		res = DxDevice->CreateRenderTarget(640,360,D3DFMT_R5G6B5,D3DMULTISAMPLE_NONE,0,TRUE,handle+1,NULL);
//		res = handle[1]->LockRect(&lrect,NULL,0);
		SDL_LockSurface(handle[1]);


		unsigned short *trgptr = (unsigned short *)handle[1]->pixels;
		unsigned short *srcptr = secondBuffer+scr_linelen2*ypos;
		for (int i = 0;i<360;i++,trgptr = (unsigned short *)((char *)trgptr+handle[1]->pitch),srcptr += scr_linelen2)
			memcpy(trgptr,srcptr,640*2);
//		handle[1]->UnlockRect();
		SDL_UnlockSurface(handle[1]);

		LockBuffers();

		return (void *)handle;
	}


	static inline void CopySurfaceAtPos(SDL_Surface *src, int width, int height, int xpos,int ypos, int border)
	{
		SDL_Rect trc;
		trc.x = xpos;
		trc.y = ypos;
		trc.w = xpos+width;
		trc.h = ypos+height;
		SDL_Rect s_rc;
		s_rc.x = 0;
		s_rc.y = 0;
		s_rc.w = width;
		s_rc.h = height;
		if (border<0) 
		{
			trc.x -= border;
			s_rc.x -= border;
		}
		else
		{
			trc.w -= border;
			s_rc.w -= border;
		}
		if (trc.x <0) {s_rc.x -= trc.x*0.7;trc.x = 0;}
		if (trc.w >= 640) {s_rc.w -= (trc.w -640)*0.7;trc.w = 640;}
		//CheckResult(DxDevice->StretchRect(src,&s_rc,DxBackBuffer,&trc,D3DTEXF_NONE));
		SDL_BlitSurface(src, &s_rc, DxBackBuffer, &trc);

	}

	void DxTurn(void *handle, char right, int ypos,int border, float phase, void *lodka)
	{
		right =!right;
		if (phase>1.0) phase = 1.0f;
		if (phase<0.0) phase = 0.0f;
		UnLockBuffers();
		SDL_Surface **ihandle = (SDL_Surface **)handle;
		SDL_Surface *sleft = ihandle[right?0:1];
		SDL_Surface *sright = ihandle[right?1:0];
		int width = 640-border*2;
		int xpos = (int)border+width*phase;
		if (right) xpos = 640-xpos;
		CopySurfaceAtPos(sleft,640,360,xpos-640+border,ypos,border);
		CopySurfaceAtPos(sright,640,360,xpos-border,ypos,-border);


		SDL_Rect rc = {0,ypos,640,ypos+360};

		GlobalPresent(&rc);
		LockBuffers();
	}


	void DxDoneTurn(void *handle)
	{
		SDL_Surface **ihandle = (SDL_Surface **)handle;
		SDL_FreeSurface(ihandle[0]);
		SDL_FreeSurface(ihandle[1]);
		delete [] ihandle;  
	}

	void DxDialogs(char enable)
	{
		dialogs = enable != 0;
	}

}

void DxLockBuffers(BOOL lock)
{
	if (lock) LockBuffers();
	else UnLockBuffers();
}

void StripBlt(void *data, unsigned int startline, unsigned long width)
{
	unsigned short *start = startline*scr_linelen2+GetScreenAdr();
	while (width--)
	{
		memcpy(start,data,640*2);
		data = (void *)((char *)data+scr_linelen);
		start = start+scr_linelen2;
	}
}
