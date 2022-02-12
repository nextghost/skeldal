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
 *  Last commit made by: $Id: DUMP.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <skeldal_win.h>
#include <stdio.h>
#include <bgraph.h>
#include <dos.h>
#include <memman.h>
#include <debug.h>

static int latest_version(char *wild,int numpos)
  {
  WIN32_FIND_DATA ff;
  HANDLE rc;
  int i = 0,j = -1;
  char *p;

  rc = FindFirstFile(wild,&ff);
  if (rc != INVALID_HANDLE_VALUE)
    do
     {
     p = ff.cFileName+numpos;
     sscanf(p,"%d",&i);
     if (i>j) j = i;
     }
    while (FindNextFile(rc,&ff));
  FindClose(rc);
  return j;  
  }

void save_dump()
  {
  static dump_counter = -1;
  FILE *f;
  int i,r,g,b,x,y;
  word *a;
  char c[20];

  if (dump_counter == -1)
     {
     dump_counter = latest_version("DUMP*.BMP",4);
     SEND_LOG("(DUMP) Dump counter sets to %d",dump_counter,0);
     }
  sprintf(c,"DUMP%04d.BMP",++dump_counter);
  SEND_LOG("(DUMP) Saving screen shot named '%s'",c,0);
  f = fopen(c,"wb");
  fputc('B',f);fputc('M',f);
  i = 640*480*3+0x36;
  fwrite(&i,1,4,f);i = 0;
  fwrite(&i,1,4,f);
  i = 0x36;
  fwrite(&i,1,4,f);
  i = 0x28;
  fwrite(&i,1,4,f);
  i = 640;
  fwrite(&i,1,4,f);
  i = 480;
  fwrite(&i,1,4,f);
  i = 1;
  fwrite(&i,1,2,f);
  i = 24;
  fwrite(&i,1,2,f);
  i = 0;
  fwrite(&i,1,4,f);
  i = 640*480*3;
  fwrite(&i,1,4,f);
  for(i = 4,r = 0;i>0;i--) fwrite(&r,1,4,f);
  for(y = 480;y>0;y--)
     {
     word *scr = GetScreenAdr();
     a = scr+(y-1)*scr_linelen2;
     for(x = 0;x<640;x++)
        {
        i = a[x];
        b = (i & 0x1f)<<3;
        g = (i & 0x7ff)>>3;
        r = i>>8;
        i = ((r*256)+g)*256+b;
        fwrite(&i,1,3,f);
        }
     }
  fclose(f);
  }

