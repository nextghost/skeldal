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
#include <skeldal_win.h>
#include <resource.h>
#include <richedit.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>

static bool IsModal=false;

static DWORD CALLBACK RtfViewerCallback(
  DWORD dwCookie, // application-gui_defined value
  LPBYTE pbBuff,  // pointer to a buffer
  LONG cb,        // number of bytes to read or write
  LONG *pcb       // pointer to number of bytes transferred
)
  {
  char **rdptr=(char **)dwCookie;
  char *p=*rdptr;
  *pcb=0;
  while (*p && cb)
	{
	*pbBuff=*(LPBYTE)p;
	pbBuff++;
	p++;
	cb--;
	pcb[0]++;
	}
  *rdptr=p;
  return 0;
  }


static LRESULT RtfViewer(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
  switch (msg)
	{
	case WM_INITDIALOG:
	  {
	  HWND rtf=GetDlgItem(hWnd,IDC_RICHEDIT);
	  EDITSTREAM edstr;
	  char *rdptr=(char *)lParam;
	  edstr.dwCookie=(DWORD)(&rdptr);
	  edstr.pfnCallback=RtfViewerCallback;
	  SendMessage(rtf,EM_STREAMIN,SF_RTF|SFF_PLAINRTF,(LPARAM)(&edstr));
	  break;
	  }
	case WM_COMMAND:
	  if (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)
		{
		if (IsModal) EndDialog(hWnd,LOWORD(wParam));
		else DestroyWindow(hWnd);
		break;
		}
	  return 0;
	default: return 0;
	}
  return 1;
  }

DWORD ShowRichView(HWND hWnd, const char *resourceName, const char *resourceType, bool modal)
  {

  HMODULE mod;
  mod=GetModuleHandle("RICHED20.DLL");
  if (mod==NULL) mod=LoadLibrary("RICHED20.DLL");
  if (mod==NULL) 
	{
	char buff[256];
	LoadString(GetModuleHandle(NULL),IDS_RICHEDMISSING,buff,sizeof(buff));
	MessageBox(hWnd,buff,NULL,MB_OK|MB_ICONEXCLAMATION);
	return 0;
	}
  HRSRC rsrc=FindResource(GetModuleHandle(NULL),resourceName,resourceType);
  if (rsrc==NULL) return 0;
  HGLOBAL src=LoadResource(GetModuleHandle(NULL),rsrc);
  const char *ptr=(const char *)LockResource(src);
  DWORD res=0;
  if (modal)
	{
	IsModal=true;
	res=DialogBoxParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_RTFVIEWER),hWnd,(DLGPROC)RtfViewer,(LPARAM)ptr);
	IsModal=false;	
	}
  else
	{
	HWND hDlg=CreateDialogParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_RTFVIEWER),hWnd,(DLGPROC)RtfViewer,(LPARAM)ptr);
	ShowWindow(hDlg,SW_SHOW);
	res=(DWORD)hDlg;
	}
  return res;
  }
