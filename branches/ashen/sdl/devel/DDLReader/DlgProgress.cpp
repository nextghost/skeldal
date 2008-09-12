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
 *  Last commit made by: $Id: DlgProgress.cpp 7 2008-01-14 20:14:25Z bredysoft $
 */
// DlgProgress.cpp : implementation file
//

#include "stdafx.h"
#include "DDLReader.h"
#include "DlgProgress.h"
#include ".\dlgprogress.h"


// DlgProgress dialog

IMPLEMENT_DYNAMIC(DlgProgress, CDialog)
DlgProgress::DlgProgress(CWnd* pParent /*= NULL*/)
	: CDialog(DlgProgress::IDD, pParent)
{
  stop = false;
}

DlgProgress::~DlgProgress()
{
}

void DlgProgress::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PROGRESS, wProgress);
  DDX_Control(pDX, IDC_NAME, wDesc);
}


BEGIN_MESSAGE_MAP(DlgProgress, CDialog)
  ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// DlgProgress message handlers

void DlgProgress::OnBnClickedButton1()
{
  stop = true;
}
