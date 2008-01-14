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
#if !defined(AFX_PALETTEEDITOR_H__45E6ACB6_CCFC_47F4_906C_BF0D2E76E2FE__INCLUDED_)
#define AFX_PALETTEEDITOR_H__45E6ACB6_CCFC_47F4_906C_BF0D2E76E2FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaletteEditor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPaletteEditor window

class CPaletteEditor : public CStatic
{
// Construction
  char *_palette;
  bool _selection[256];
  bool _drawsel;
public:
	CPaletteEditor();

	void SetPalette(char *pal) {_palette=pal;}
	bool IsSelected(int p) {return _selection[p];}
	int GetR(int p) {return _palette[p*3];}
	int GetG(int p) {return _palette[p*3+1];}
	int GetB(int p) {return _palette[p*3+2];}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteEditor)
	//}}AFX_VIRTUAL

// Implementation
public:
	int IndexFromPoint(CPoint pt);
	virtual ~CPaletteEditor();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPaletteEditor)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALETTEEDITOR_H__45E6ACB6_CCFC_47F4_906C_BF0D2E76E2FE__INCLUDED_)
