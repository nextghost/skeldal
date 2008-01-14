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


// CImageView

class CImageView : public CStatic
{
	DECLARE_DYNAMIC(CImageView)
    char *_buffer;
    struct BMI
    {
      BITMAPINFOHEADER _bmh;
      RGBQUAD _palette[256];
    } _bmi;
public:
	CImageView();
	virtual ~CImageView();

protected:
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnPaint();
  void UpdateBuffer(char *buffer);
  void SetPalette(char *palette);
};


