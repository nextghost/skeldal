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
// ColorButton.cpp : implementation file
//

#include "stdafx.h"
#include "ColorButton.h"
#include ".\colorbutton.h"


// CColorButton

IMPLEMENT_DYNAMIC(CColorButton, CButton)
CColorButton::CColorButton()
{
}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
END_MESSAGE_MAP()



// CColorButton message handlers


void CColorButton::DrawItem(LPDRAWITEMSTRUCT dis)
  {
  CRect rcitem(dis->rcItem);
  CDC dc;
  dc.Attach(dis->hDC);
  dc.DrawFrameControl(&dis->rcItem,DFC_BUTTON,DFCS_BUTTONPUSH|(dis->itemState & 1?DFCS_PUSHED:0));
  rcitem-=CRect(2,2,2,2);
  if (dis->itemState & ODS_FOCUS) dc.DrawFocusRect(&rcitem);
  rcitem-=CRect(2,2,2,2);
  dc.FillSolidRect(&rcitem,_color);  
  dc.Detach();
  }

void CColorButton ::SetColor(COLORREF color, BOOL redraw)
  {
  _color=color;
  Invalidate(FALSE);
  }
