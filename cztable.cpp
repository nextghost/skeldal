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


static czxlat czxlattab[] = {
	{0xA0, 0xE1},
	{0x87, 0xE8},
	{0x83, 0xEF},
	{0x82, 0xE9},
	{0x88, 0xEC},
	{0xA1, 0xED},
	{0x8D, 0xE5},
	{0x8C, 0xBE},
	{0xA4, 0xF2},
	{0xA2, 0xF3},
	{0xAA, 0xE0},
	{0xA9, 0xF8},
	{0xA8, 0x9A},
	{0x9f, 0x9D},
	{0xA3, 0xFA},
	{0x96, 0xF9},
	{0x98, 0xFD},
	{0x91, 0x9E},

	{0x8F, 0xC1},
	{0x80, 0xC8},
	{0x85, 0xCF},
	{0x90, 0xC9},
	{0x89, 0xCC},
	{0x8B, 0xCD},
	{0x8A, 0xC5},
	{0x9C, 0xBC},
	{0xA5, 0xD2},
	{0x95, 0xD3},
	{0xAB, 0xC0},
	{0x9E, 0xD8},
	{0x9B, 0x8A},
	{0x86, 0x8D},
	{0x97, 0xDA},
	{0xA6, 0xD9},
	{0x9D, 0xDD},
	{0x92, 0x8E}
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
