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
#pragma once
#include "afxwin.h"


// DlgOpen dialog

class DlgOpen : public CDialog
{
	DECLARE_DYNAMIC(DlgOpen)
    CString _umisteni;
    CString _selected;
    char *_iconlib;
    CIconViewerButton icons[18];

public:
	DlgOpen(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgOpen();

// Dialog Data
	enum { IDD = IDD_OPENDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
  void ReadFolder(void);
  CListBox wList;
  afx_msg void OnBnClickedBrowse();
    CString GetSelectedFile();    
protected:
  virtual void OnOK();
public:
  afx_msg void OnLbnDblclkList();
  afx_msg void OnDestroy();
  afx_msg void OnLbnSelchangeList();
};
