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
 *  Last commit made by: $Id: VESA.H 7 2008-01-14 20:14:25Z bredysoft $
 */
/********************************/
/* Prace se SVGA VESA adapterem */
/********************************/

#ifndef __VESA_H
#define __VESA_H


typedef struct
{ char VESASignature[4];
  short VESAversion;
  unsigned OEMStringPtr;
  char capabilities[4];
  short int *videomodeptr;
  short int TotalMemory;
  char dummy[236];
  } VESA_INFO_BLOCK;

typedef struct
{ short unsigned ModeAttributes;
  char WinAAttributes;
  char WinBAttributes;
  short unsigned WinGranularity;
  short unsigned WinSize;
  short unsigned WinASegment;
  short unsigned WinBSegment;
  short unsigned WinFuncPtroff;
  short unsigned WinFuncPtrseg;
  short unsigned BytesPerScanLine;

  short unsigned XResolution;
  short unsigned YResolution;
  char Xcharsize;
  char Ycharsize;
  char NumberOfPlanes;
  char BitsPerPixel;
  char NumberOfBanks;
  char MemoryModel;
  char BankSize;
  char NumberOfImagePages;
  char Reserved;

  char RedMaskSize;
  char RedFieldPosition;
  char GreenMaskSize;
  char GreenFieldPosition;
  char BlueMaskSize;
  char BlueFieldPosition;
  char RsvdMaskSize;
  char DirectColorModeInfo;
  char Dummy[216];
  } VESA_MODE_INFO_BLOCK;


VESA_MODE_INFO_BLOCK VesaModeInfoBlock;
VESA_INFO_BLOCK VesaInfoBlock;

extern unsigned char _VGAPage=0;
extern unsigned char _VGAGran=0;

void Set_VESA_mode (int mode);
void Show_screen (char *display);
void Get_VESA_info (void);
void Get_mode_info (int mode);

void Put_image (int x, int y, int xlen, int ylen, char *image);
void Get_image (int x, int y, int xlen, int ylen, char *image);

#endif

