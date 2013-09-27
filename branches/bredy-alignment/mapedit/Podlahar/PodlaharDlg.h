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
// PodlaháøDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ColorButton.h"
#include "ImageView.h"


// CPodlahDlg dialog
class CPodlahDlg : public CDialog
{
// Construction
public:
	CPodlahDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PODLAH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedBrowse();
  int type;
  CString vHlTextura;
  CString vVdTextura;
  CColorButton wColorButt;
  CString vTarget;
  CButton wBrowse1;
  CButton wBrowse2;
  CButton wBrowse3;
  CButton wSavePic;
  CImageView wPreview;
  afx_msg void OnBnClickedColor();
  afx_msg void OnBnClickedPreview();
  BOOL vGenMlhu;
  afx_msg void OnBnClickedSavepic();
  afx_msg void OnBnClickedBrowseSave();
  int vType;
};


extern "C"
{
    char *PodlahaStrop(int r, int g, int b, const char *filename1, const char *filename2, int genMlhu);
    char *GetPodlahaPalette();
    int save_pcx(char *filename,int x1,int y1,int x2,int y2);
    char CheckPCX(const char *pcx);

}