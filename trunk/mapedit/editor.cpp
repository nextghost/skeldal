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
#include <windows.h>
#include <string.h>
#include <malloc.h>
#include "resource.h"
#include "../cztable.h"
#include <BGraph2Dx.h>
#include "stdio.h"
#include "editor.h"


int ResMessageBox(HWND hWnd, UINT idc, UINT flags)
  {
  char buff1[1024],buff2[256];
  HINSTANCE hInst=GetModuleHandle(NULL);
  LoadString(hInst,idc,buff1,sizeof(buff1));
  LoadString(hInst,IDS_WINTITLE,buff2,sizeof(buff2));
  return MessageBox(hWnd,buff1,buff2,flags);
  }

int ResMessageBox2(HWND hWnd, UINT idc, UINT flags,...)
  {
  char buff1[1024],buff2[256],buff3[2048];
  va_list args;
  va_start(args,flags);
  HINSTANCE hInst=GetModuleHandle(NULL);
  LoadString(hInst,idc,buff1,sizeof(buff1));
  LoadString(hInst,IDS_WINTITLE,buff2,sizeof(buff2));
  _vsnprintf(buff3,sizeof(buff3),buff1,args);
  return MessageBox(hWnd,buff3,buff2,flags);
  }

static char *LoadSkeldalFile(const char *filename)
  {
  HANDLE h=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (h==INVALID_HANDLE_VALUE) return NULL;
  DWORD fsize=GetFileSize(h,NULL);
  DWORD rd;
  char *buff=(char *)malloc(fsize+1);
  ReadFile(h,buff,fsize,&rd,NULL);
  CloseHandle(h);
  buff[fsize]=0;
  kamenik2windows(buff,fsize,buff);
  return buff;
  }

static bool SaveSkeldalFile(const char *filename, const char *data)
  {
  DWORD fsize=strlen(data);
  char *saved=(char *)malloc(fsize+1);  
  windows2kamenik(data,fsize+1,saved);
  HANDLE h=CreateFile(filename,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);  
  if (h==INVALID_HANDLE_VALUE) return false;
  DWORD rd;  
  WriteFile(h,saved,fsize,&rd,NULL);
  CloseHandle(h);
  free(saved);
  return rd==fsize;
  }


static void DrawLineNumbering(HWND hWnd, HDC hDC, HWND edit, HFONT font)
  {
  RECT rc;
  GetWindowRect(edit,&rc);
  ScreenToClient(hWnd,(LPPOINT)&rc);
  ScreenToClient(hWnd,((LPPOINT)&rc)+1);
  int start=SendDlgItemMessage(hWnd,IDC_EDIT,EM_GETFIRSTVISIBLELINE,0,0)+1;
  HFONT oldfont=(HFONT)SelectObject(hDC,font);
  SIZE sz={0,0};
  SetBkColor(hDC,GetSysColor(COLOR_BTNFACE));    
  for (int y=rc.top;y+sz.cy<rc.bottom;y+=sz.cy)
	{
	char str[100];
	sprintf(str,"%5d",start++);
    GetTextExtentPoint32(hDC,str,strlen(str),&sz);
	if (sz.cy==0) break;
	TextOut(hDC,rc.left-5-sz.cx,y,str,strlen(str));
	}
  SelectObject(hDC,oldfont);
  }

static WINDOWPLACEMENT defWP;

static LRESULT EditorProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
  {
  HFONT editfont=(HFONT)GetWindowLong(hDlg,DWL_USER);
  const char *editorfile=(const char *)GetWindowLong(hDlg,GWL_USERDATA);
  switch (msg)
	{
	case WM_INITDIALOG:
	  {
	  editorfile=strdup((const char *)lParam);
	  SetWindowLong(hDlg,GWL_USERDATA,(LONG)editorfile);
	  SendMessage(GetDlgItem(hDlg,IDC_EDIT),EM_SETLIMITTEXT,0,0);
	  char *text=LoadSkeldalFile(editorfile);
	  if (text) 
		{
		SetDlgItemText(hDlg,IDC_EDIT,text);
		free(text);
		}
	  editfont=CreateFont(13,0,0,0,800,FALSE,FALSE,FALSE,EASTEUROPE_CHARSET,0,0,0,0,"Courier");
	  SetWindowLong(hDlg,DWL_USER,(LONG)editfont);
	  const char *z=strrchr(editorfile,'\\');
	  char *tend;
	  if (z==NULL) z=editorfile;else z++;
	  int titlesz=GetWindowTextLength(hDlg)+strlen(z)+10;
	  text=(char *)alloca(titlesz);
	  sprintf(text,"%s - ",z);
	  tend=strchr(text,0);
	  GetWindowText(hDlg,tend,titlesz);
	  SetWindowText(hDlg,text);
	  SendDlgItemMessage(hDlg,IDC_EDIT,WM_SETFONT,(WPARAM)editfont,MAKELPARAM(1,0));
	  SetClassLong(hDlg,GCL_HICON,(LONG)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_NOTEPADICON)));
	  SetWindowPlacement(hDlg,&defWP);
	  break;
	  }
	case WM_DESTROY:
	  DeleteObject(editfont);
	  free((void *)editorfile);
	  break;	
	case WM_PAINT:
	  {
	  PAINTSTRUCT ps;
	  HDC dc=BeginPaint(hDlg,&ps);
	  DrawLineNumbering(hDlg,dc,GetDlgItem(hDlg,IDC_EDIT),editfont);
	  EndPaint(hDlg,&ps);
	  break;
	  }
  case WM_APP+9998:
    {
      MSG *msg=(MSG *)lParam;
      if (msg->message==WM_KEYDOWN && msg->wParam=='S' && GetKeyState(VK_CONTROL)<0)
      {
        SendMessage(hDlg,WM_COMMAND,IDOK,0);
        SetWindowLong(hDlg,DWL_MSGRESULT,1);
      }
      else
        SetWindowLong(hDlg,DWL_MSGRESULT,IsDialogMessage(hDlg,msg));
      return 1;
    }
	case WM_COMMAND:	  
	  switch (LOWORD(wParam))
		{
		case IDCANCEL: DestroyWindow(hDlg);break;		
		case IDOK:
		  {
		  int txtsz=GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT))+1;
		  char *buff=(char *)malloc(txtsz);
		  GetDlgItemText(hDlg,IDC_EDIT,buff,txtsz);
		  if (!SaveSkeldalFile(editorfile,buff))
			{
			ResMessageBox2(hDlg,IDC_EDITORSAVEERROR,MB_OK,editorfile);
			}
		  free(buff);
		  }
		  break;
		case ID_PRAVY_UNDO: SendDlgItemMessage(hDlg,IDC_EDIT,WM_UNDO,0,0);break;
		case ID_UPRAVY_COPY: SendDlgItemMessage(hDlg,IDC_EDIT,WM_COPY,0,0);break;
		case ID_UPRAVY_VYJMOUT: SendDlgItemMessage(hDlg,IDC_EDIT,WM_CUT,0,0);break;
		case ID_UPRAVY_VLOZIT: SendDlgItemMessage(hDlg,IDC_EDIT,WM_PASTE,0,0);break;
		case ID_UPRAVY_VYMAZAT: SendDlgItemMessage(hDlg,IDC_EDIT,EM_REPLACESEL,1,(LPARAM)"");break;
		default: return 0;
		}
	  case WM_CTLCOLOREDIT:
		{
		  HDC dc=GetDC(hDlg);
		  DrawLineNumbering(hDlg, dc, GetDlgItem(hDlg,IDC_EDIT), editfont);
		  return 0;
		}
	  case WM_SIZE:
		{
		int fwSizeType = wParam;      // resizing flag 
		int cx= LOWORD(lParam);  // width of client area 
		int cy= HIWORD(lParam); // height of client area 
		if (fwSizeType!=SIZE_MINIMIZED)
		{
		  RECT rc1;
		  SIZE sz;
		  GetWindowRect(GetDlgItem(hDlg,IDOK),&rc1);
		  sz.cx=rc1.right-rc1.left;
		  sz.cy=rc1.bottom-rc1.top;
		  SetWindowPos(GetDlgItem(hDlg,IDOK),NULL,cx-sz.cx-2,cy-sz.cy-2,0,0,SWP_NOZORDER|SWP_NOSIZE);
		  GetWindowRect(GetDlgItem(hDlg,IDC_EDIT),&rc1);
		  ScreenToClient(hDlg,(LPPOINT)&rc1);
		  ScreenToClient(hDlg,((LPPOINT)&rc1)+1);
		  rc1.right=cx-2;
		  rc1.bottom=cy-sz.cy-4;
		  sz.cx=rc1.right-rc1.left;
		  sz.cy=rc1.bottom-rc1.top;
		  SetWindowPos(GetDlgItem(hDlg,IDC_EDIT),NULL,0,0,sz.cx,sz.cy,SWP_NOZORDER|SWP_NOMOVE);
		  defWP.length=sizeof(defWP);
		  GetWindowPlacement(hDlg,&defWP);
		}
		break;
		}
	  case MSG_FORCESAVE:
		SendMessage(hDlg,WM_COMMAND,IDOK,0);
		break;
	  case MSG_FINDANDPOPUP:
		{
		  const char *name=(const char *)lParam;
		  if (stricmp(name,editorfile)==0)
		  {
			SetActiveWindow(hDlg);
			SetForegroundWindow(hDlg);
			BringWindowToTop(hDlg);
			SetWindowLong(hDlg,DWL_MSGRESULT,1);
			return 1;
		  }
		  else
			return 0;
		}
	  case MSG_CLOSEEDITOR:
		DestroyWindow(hDlg);break;
	default: return 0;
	}
  return 1;
  }


extern "C"
  {
void EditSkeldalFile(const char *filename)
  {
  CreateDialogParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_TEXTEDITOR),NULL,(DLGPROC)EditorProc,(LPARAM)filename);
  }
  }
