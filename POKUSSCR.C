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
 *  Last commit made by: $Id: POKUSSCR.C 7 2008-01-14 20:14:25Z bredysoft $
 */
/*
 * Nahodi SVGA
 */
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_AREA 0xa000
#define SCREEN_LIN_ADDR ((SCREEN_AREA) << 4)
#define SCREEN_SIZE 65537

unsigned char	pomoc[256];
void main()
{
	unsigned short       *pscreen;
	unsigned int 	sizegran;
	int         	i,err;
	int         	col;
	int         	g;
	union REGPACK  	regs;
	//-------------------------------------------------------------
	//pomoc = (unsigned char *)malloc(512);
	memset( &regs, 0, sizeof(union REGPACK) );
	regs.w.ax = 0x4f02;
	regs.w.bx = 0x0111;
	intr( 0x10, &regs);//nastaveni videomodu
	//regs.w.ax = 0x4f01;
	//regs.w.cx = 0x0101;
	//regs.w.es = FP_SEG (pomoc);
	//regs.w.di = FP_OFF (pomoc);
	//intr( 0x10, &regs);//zjisteni granularity v KB
	//err = regs.w.ax;
	//sizegran = pomoc[4];
	//sizegran = sizegran*1024;
	//if (sizegran<= 65536)
	//----------------------------
	for( col = 0; col < 100; col++ ) {
trace:if (!(inp( 0x03da )&8 )) goto trace;// cekani na raytrace
      for( g = 0; g < 10; g++ ) {
	      regs.w.ax = 0x4f05;
	      regs.w.bx = 0;
	      regs.w.dx = g;
	      intr( 0x10, &regs);//prepnuti videostranky
	      pscreen = (unsigned short *)SCREEN_LIN_ADDR;
	      for( i = 0; i < SCREEN_SIZE; i += sizeof(*pscreen)  ) {
		      *pscreen = i+col*g;
		      pscreen += 1;
	      }
      }
	}
	regs.w.ax = 0x0003;
	intr (0x10,&regs);
	//for( g = 0; g < 15; g++ ) printf("%X...%X\n",g,pomoc[g]*1024);
	//free (pomoc);
	//printf("err %X\n",err);
}
