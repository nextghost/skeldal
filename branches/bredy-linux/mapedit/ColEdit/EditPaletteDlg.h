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
#if !gui_defined(AFX_EDITPALETTEDLG_H__8ED55BD3_E477_4C1A_80E7_D79CDA43A55E__INCLUDED_)
#define AFX_EDITPALETTEDLG_H__8ED55BD3_E477_4C1A_80E7_D79CDA43A55E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditPaletteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditPaletteDlg dialog

#include "PaletteEditor.h"
#include "..\Podlahar\ImageView.h"

class CEditPaletteDlg : public CDialog
{
// Construction
  char _paletteUndo[768];
  char _paletteScrl[768];
  float _curh,_curv,_curs;
public:
	CEditPaletteDlg(CWnd* pParent = NULL);   // standard constructor
	char *_palette;
	CImageView *_imgView;
	

// Dialog Data
	//{{AFX_DATA(CEditPaletteDlg)
	enum { IDD = IDD_EDITPALETTE };
	CSliderCtrl	wSitost;
	CSliderCtrl	wJas;
	CSliderCtrl	wBarva;
	CPaletteEditor	wPaleta;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPaletteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CalculateAvgHSV();

	// Generated message map functions
	//{{AFX_MSG(CEditPaletteDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaleta();
	virtual void OnCancel();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUndo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !gui_defined(AFX_EDITPALETTEDLG_H__8ED55BD3_E477_4C1A_80E7_D79CDA43A55E__INCLUDED_)
