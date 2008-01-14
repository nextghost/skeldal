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
// ImageView.cpp : implementation file
//

#include "stdafx.h"
#include "ImageView.h"
#include ".\imageview.h"


// CImageView

IMPLEMENT_DYNAMIC(CImageView, CStatic)
CImageView::CImageView()
{
  ZeroMemory(&_bmi,sizeof(_bmi._bmh));
  _bmi._bmh.biSize=sizeof(_bmi._bmh);
  _bmi._bmh.biWidth=640;
  _bmi._bmh.biHeight=-480;
  _bmi._bmh.biPlanes=1;
  _bmi._bmh.biBitCount=8;
  _bmi._bmh.biCompression=BI_RGB;
  _bmi._bmh.biSizeImage =0;  
  _buffer=NULL;
}

CImageView::~CImageView()
{
}


BEGIN_MESSAGE_MAP(CImageView, CStatic)
  ON_WM_PAINT()
END_MESSAGE_MAP()



// CImageView message handlers


void CImageView::OnPaint()
{
  CPaintDC dc(this); 
  if (_buffer)
  {
    SetDIBitsToDevice(dc,0,0,640,480,0,0,0,480,_buffer,(BITMAPINFO *)&_bmi,DIB_RGB_COLORS);
  }
}


void CImageView::UpdateBuffer(char *buffer)
{
  _buffer=buffer;
  Invalidate();
}

void CImageView::SetPalette(char *palette)
{
  for (int i=0;i<256;i++)
  {
    _bmi._palette[i].rgbRed=*palette++;
    _bmi._palette[i].rgbGreen=*palette++;
    _bmi._palette[i].rgbBlue=*palette++;
    _bmi._palette[i].rgbReserved=0;
  }
}