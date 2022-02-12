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
 *  Last commit made by: $Id: FASTLOAD.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>
#include <event.h>

#define LOAD_BUFFER 4096

int _fast_load(char *ptr,long size,FILE *f)
  {
  if (size>LOAD_BUFFER) size = 4096;
  return fread(ptr,1,size,f);
  }

size_t fread(void *ptr,size_t i,size_t j,FILE *f)
  {
  long s,z,celk = 0;
  char *c;

  c = ptr;
  s = i*j;
  do
     {
     z = _fast_load(c,s,f);
     s -= z;
     c += z;
     celk += z;
     do_events();
     }
  while(s || !z);
  return z;
  }
