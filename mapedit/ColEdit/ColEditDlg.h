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
// ColEditDlg.h : header file
//

#if !gui_defined(AFX_COLEDITDLG_H__2FF53390_ECD2_468E_B1C9_60F1718D1470__INCLUDED_)
#define AFX_COLEDITDLG_H__2FF53390_ECD2_468E_B1C9_60F1718D1470__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CColEditDlg dialog


#include "..\Podlahar\ImageView.h"
#define MAX_PALET 100

class CColEditDlg : public CDialog
{
// Construction
  char _palety[MAX_PALET][768];
  int _palused;
  int _indexes[MAX_PALET];
  CString fname;

  char _stbuff[640*480];

public:
	bool SaveDocument(LPCTSTR name);
	int DuplicateCurrent();
	void UpdateCB();
	bool LoadDocument(LPCTSTR name);
	void UpdateLB();
	CColEditDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CColEditDlg)
	enum { IDD = IDD_COLEDIT_DIALOG };
	CButton	wUp;
	CButton	wDown;
	CComboBox	wImageSel;
	CImageView	wImage;
	CListBox	wColList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CColEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoad();
	afx_msg void OnSelchangeColorlist();
	afx_msg void OnSelchangeImagesel();
	afx_msg void OnInsert();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnDuplicate();
	afx_msg void OnDelete();
	virtual void OnOK();
	afx_msg void OnOpravpaletu();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !gui_defined(AFX_COLEDITDLG_H__2FF53390_ECD2_468E_B1C9_60F1718D1470__INCLUDED_)
