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
#if !defined(AFX_DLGNOVYDIALOG_H__00136ADF_E2C6_4081_83AB_D5110EE43612__INCLUDED_)
#define AFX_DLGNOVYDIALOG_H__00136ADF_E2C6_4081_83AB_D5110EE43612__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgNovyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgNovyDialog dialog

class DlgDialogy;

class DlgNovyDialog : public CDialog
{
// Construction
public:
	void DialogRules();
	DlgNovyDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgNovyDialog)
	enum { IDD = IDD_NOVYDIALOG };
	CComboBox	wCislo;
	CComboBox	wJmeno;
	int		vCislo;
	CString	vJmeno;
	CString	vPopis;
	//}}AFX_DATA

	CListCtrl *sourceLst;
	Pathname dlgSource;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgNovyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgNovyDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeJmeno();
	afx_msg void OnSelendokJmeno();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGNOVYDIALOG_H__00136ADF_E2C6_4081_83AB_D5110EE43612__INCLUDED_)
