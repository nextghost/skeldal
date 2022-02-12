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
#include <cstring>

struct czxlat
  {
  unsigned char kamenik;
  unsigned char windows;
  };


static czxlat czxlattab[]=
  {
	{0xA0,'�'},
	{0x87,'�'},
	{0x83,'�'},
	{0x82,'�'},
	{0x88,'�'},
	{0xA1,'�'},
	{0x8D,'�'},
	{0x8C,'�'},
	{0xA4,'�'},
	{0xA2,'�'},
	{0xAA,'�'},
	{0xA9,'�'},
	{0xA8,'�'},
	{0x9f,'�'},
	{0xA3,'�'},
	{0x96,'�'},
	{0x98,'�'},
	{0x91,'�'},

	{0x8F,'�'},
	{0x80,'�'},
	{0x85,'�'},
	{0x90,'�'},
	{0x89,'�'},
	{0x8B,'�'},
	{0x8A,'�'},
	{0x9C,'�'},
	{0xA5,'�'},
	{0x95,'�'},
	{0xAB,'�'},
	{0x9E,'�'},
	{0x9B,'�'},
	{0x86,'�'},
	{0x97,'�'},
	{0xA6,'�'},
	{0x9D,'�'},
	{0x92,'�'},
  };

static char xlatkm2win[256];
static char xlatwin2km[256];
static char prepare=1;

static void PrepareTabs()
  {
  size_t i;
  for (i=0;i<256;i++) {xlatkm2win[i]=i;xlatwin2km[i]=i;}
  for (i=0;i<sizeof(czxlattab)/sizeof(czxlattab[0]);i++)
	{
	xlatkm2win[czxlattab[i].kamenik]=czxlattab[i].windows;
	xlatwin2km[czxlattab[i].windows]=czxlattab[i].kamenik;
	}
  prepare=0;
  }

extern "C"
  {

void windows2kamenik(const char *src, int size, char *trg)
  {
  if (prepare) PrepareTabs();
  if (size<0) size=strlen(src)+1;
  for (int i=0;i<size;i++) *trg++=xlatwin2km[(unsigned char)*src++];
  }

void kamenik2windows(const char *src, int size, char *trg)
  {
  if (prepare) PrepareTabs();
  if (size<0) size=strlen(src)+1;
  for (int i=0;i<size;i++) *trg++=xlatkm2win[(unsigned char)*src++];
  }

  }
