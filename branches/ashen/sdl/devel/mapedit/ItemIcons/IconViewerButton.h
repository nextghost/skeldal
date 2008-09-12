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
 *  Last commit made by: $Id: IconViewerButton.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#pragma once


// CIconViewerButton

  struct BitMapInfo
  {
    BITMAPINFOHEADER hdr;
    RGBQUAD palette[256];
  };


class CIconViewerButton : public CStatic
{
	DECLARE_DYNAMIC(CIconViewerButton)

public:
	CIconViewerButton();
	virtual ~CIconViewerButton();

    void *_data;

protected:
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnPaint();
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


