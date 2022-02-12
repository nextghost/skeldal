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
 *  Last commit made by: $Id: DDLReaderDlg.h 7 2008-01-14 20:14:25Z bredysoft $
 */
// DDLReaderDlg.h : header file
//

#include "t:\h\atlmfc\include\afxwin.h"
#include "ddlfile.h"
#include "WPathname.h"
#if !defined(AFX_DDLREADERDLG_H__E765355F_0112_4B3E_90CE_111E28FE8EC6__INCLUDED_)
#define AFX_DDLREADERDLG_H__E765355F_0112_4B3E_90CE_111E28FE8EC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDDLReaderDlg dialog

class CDDLReaderDlg : public CDialog, public IDDLFileEnumerator
{
// Construction
    CSize _dlgSize; 
    WPathname _lastTemp;
public:
	CDDLReaderDlg(CWnd* pParent = NULL);	// standard constructor
    ~CDDLReaderDlg();

// Dialog Data
	//{{AFX_DATA(CDDLReaderDlg)
	enum { IDD = IDD_DDLREADER_DIALOG };
	CEdit	wFolder;
	CListCtrl	wFileList;
	CButton	wBrowse;
	CButton	wExport;
	CString	vFolder;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDDLReaderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDDLReaderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnSize(UINT nType, int cx, int cy);
  CStatic wPopisek;
  CEdit wDDLFile;
  CButton wDDLBrowse;
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  DDLFile _ddlfile;
  afx_msg void OnEnKillfocusDdlfile();
  void UpdateList(void);
  virtual bool File(WString name, int group, unsigned long offset);  
  afx_msg void OnBnClickedDdlbrowse();
  afx_msg void OnLvnColumnclickFilelist(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedBrowse();
  afx_msg void OnBnClickedExport();
  afx_msg void OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult);

  WPathname CreateTemp(int index);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DDLREADERDLG_H__E765355F_0112_4B3E_90CE_111E28FE8EC6__INCLUDED_)
