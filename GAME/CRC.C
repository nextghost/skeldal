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
 *  Last commit made by: $Id: CRC.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>
#include <mem.h>

unsigned long l;

#define ZAKLAD_CRC 0xC005

char data[100000];
long delka;
FILE *f;


main()
  {
  int i;
  f = fopen("CRC.C","rb");
  memset(data,0,sizeof(data));
  delka = fread(data,1,sizeof(data),f);
  fclose(f);
  memcpy(&l,data,2);i = 0;
  l%= ZAKLAD_CRC;
  while(i<delka)
     {
     i += 2;
     l<<= 16;
     memcpy(&l,data+i,2);
     l%= ZAKLAD_CRC;
     }
  printf("CRC: %x\n",l);
  }
z
