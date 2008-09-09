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
 *  Last commit made by: $Id: VESATST.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include "types.h"
#include <vesa.h>
#include <dpmi.h>
#include <stdio.h>
#include <malloc.h>
#include "bgraph.h"
#include <i86.h>

MODEinfo vesadata;
word lastbank = 0;

word *mapvesaadr(word *a);
#pragma aux mapvesaadr parm [edi] value [edi]

void showview32b(word x,word y,word xs,word ys)
  {
  register longint a;

  if (x>640 || y>480) return;
  if (xs == 0) xs = 640;
  if (ys == 0) ys = 480;
  if (x+xs>640) xs = 640-x;
  if (y+ys>480) ys = 480-y;
  if (xs>550 && ys>400)
     {
     redraw32b(screen,lbuffer,NULL);
     return;
     }
  a = (x<<1)+linelen*y;
  redrawbox32b(xs,ys,(void *)((char *)screen+a),(void *)a);
  }

int initmode32b()
  {
  getmodeinfo(&vesadata,0x110);
  if (!(vesadata.modeattr & MA_SUPP)) return -1;
  setvesamode(0x110,-1);
  lbuffer = (word *)0xa0000;
  screen = lbuffer;
  linelen = 640*2;
  showview = showview32b;
  screen = (void *)malloc(screen_buffer_size);
  return 0;
  }


word *mapvesaadr1(word *a)
  {
  word bank;

  bank = (long)a>>16;
  if (bank != lastbank)
     {
     lastbank = bank;
     bank = bank;
          {
           union REGS regs;
           regs.w.ax = 0x4f05;
           regs.w.bx = 0;
           regs.w.dx = bank;
           int386 (0x10,&regs,&regs); // window A
          }
     }
 return (word *)(((long)a & 0xffff)+0xa0000);
}

void switchvesabank(word bank)
#pragma aux switchvesabank parm [eax]
  {
           union REGS regs;
           regs.w.ax = 0x4f05;
           regs.w.bx = 0;
           regs.w.dx = bank;
           int386 (0x10,&regs,&regs); // window A
  }



void vesatst()
  {
  int i,j;
  word *a;

  a = screen;
  for (i = 0;i<480;i++)
     for(j = i;j<i+640;j++)
       *(a++) = i+j;
  showview(10,10,400,300);
  }

void main()
  {
  initmode32b();
  getchar();
  vesatst();
  getchar();
  }

