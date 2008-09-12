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
 *  Last commit made by: $Id: DOSMEM.C 7 2008-01-14 20:14:25Z bredysoft $
 */
/*********************************************/
/*** DOS memory allocation - DOSMEM.C      ***/ 
/*** vykostena verze chlumakova memalloc.c ***/
/*********************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dosmem.h"


/*** Alokace pole v dolni pameti ***/

void *mem_alloc(int size) 
{
	static union REGS r;
	MEMREC memrec;

   	r.x.eax = 0x0100;	      
	r.x.ebx = (size>>4)+1;      
	size = r.x.ebx<<4;      
	int386(0x31,&r,&r);      
	if (r.x.cflag)        
		{
		printf ("No fucking DOS memory left!!!");          
		exit(1);          
		}                
	memrec.ptr = (void *)((r.x.eax&0xFFFF)<<4);      
	Selector = memrec.selector = (short int)r.x.edx;      

	return memrec.ptr;
} 


/*** Uvolneni dolni pameti ***/

void mem_free(void *ptr) 
{
	union REGS r;  

	if(ptr != NULL)        
		{
		r.x.eax = 0x0101;     	     
		r.x.edx = Selector;     	     
		int386(0x31,&r,&r);     	     
		if(r.x.cflag)     	       
			printf("Cannot free DOS memory!!!!");                  
		}

} 


/*** Vyvolani preruseni pomoci protected modu ***/

void WtNs386(int IntNum, DPMIREGS *dpmiregs) 
{
	union REGS r;
	struct SREGS sr;
  
	r.w.ax = 0x300;		
	r.h.bl = (char)IntNum;
	r.h.bh = 0;
	r.w.cx = 0;
	segread(&sr);
	sr.es = FP_SEG(dpmiregs);
	r.x.edi = FP_OFF(dpmiregs);

	int386x(0x31,&r,&r,&sr);
}

