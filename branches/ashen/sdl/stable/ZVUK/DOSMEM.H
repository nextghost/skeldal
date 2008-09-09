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
 *  Last commit made by: $Id: DOSMEM.H 7 2008-01-14 20:14:25Z bredysoft $
 */
/************************************/
/*** Hlavickovt soubor k DOSMEM.H ***/
/************************************/


typedef enum
{ DOS_MEMORY,
  NEW,
} MEMORY_ITEMS;

typedef struct
{ int EDI;
  int ESI;
  int EBP;
  int reserved;
  int EBX;
  int EDX;
  int ECX;
  int EAX;
  short int Flags;
  short int ES;
  short int DS;
  short int GS;
  short int IP;
  short int CS;
  short int SP;
  short int SS;
  } DPMIREGS;

typedef struct
{ void *ptr;
  int  size;
  int  selector;  //smysl jen u DOS_MEMORY
  } MEMREC;

#define D32RealSeg(P) ((((unsigned int)(P))>>4)&0xFFFF)
#define D32RealOff(P) (((unsigned int)(P))&0xF)

int Selector;
DPMIREGS dpmiregs;

void *mem_alloc(int size);
void mem_free(void *ptr); 
void WtNs386(int IntNum, DPMIREGS *dpmiregs); 


