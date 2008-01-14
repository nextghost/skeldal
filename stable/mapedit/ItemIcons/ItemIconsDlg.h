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
// ItemIconsDlg.h : header file
//

#pragma once
#include "iconviewerbutton.h"
#include "dlgopen.h"
#include "afxwin.h"



// CItemIconsDlg dialog
class CItemIconsDlg : public CDialog
{
// Construction
public:
	CItemIconsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ITEMICONS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  CIconViewerButton icons[18];
  char _iconlib[ICONLIBSIZE];
  afx_msg void OnOpen();
  DlgOpen _dlgOpen;
  CString _documentName;
  bool LoadDocument(const CString & name);
  CEdit wFilename;
  int _selicon;
  int IconFromWnd(CWnd * wnd);
  int ImportBMP(int icon,void *bmp, int dataoffset);
  afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
  afx_msg void OnPopupPaste();
  afx_msg void OnPopupCopy();
  void ExportBMP(int icon, BITMAPINFO * bitmap);
  afx_msg void OnPopupImportbmp();
  afx_msg void OnDropFiles(HDROP hDropInfo);
  void ImportBMP(const _TCHAR * name, int icon);
  void ExportBMP(int icon, const _TCHAR * filename);
  afx_msg void OnPopupExportbmp();
  int ManualSelectPic(void);
  afx_msg void OnEditCopy();
  afx_msg void OnEditPaste();
  afx_msg void OnClickIcon(UINT cmd);
  afx_msg void OnEditExport();
  afx_msg void OnEditImport();
  afx_msg void OnEditExportall();
  afx_msg void OnSave();
  bool SaveDocument(const _TCHAR * name);
  afx_msg void OnPopupVymazat();
  afx_msg void OnEditDelete();
  void SetTransparentIndex(int icon, int index);
  void AutodetectTransparent(int icon);
};

