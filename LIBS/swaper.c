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
#include <stdio.h>
#include "swaper.h"

#define SWAP_FREE_LIST 8192

typedef struct sw_free_block
  {
  long size,seek;
  }SW_FREE_BLOCK;

static SW_FREE_BLOCK swp_list[SWAP_FREE_LIST];
static int swp_ptr;
static int swp_fnot_used=-1;
static int swp_unuseds=0;
static int swpl_overruns=0;

void swap_init(void)
  {
  swp_ptr=0;
  swp_list[swp_ptr].seek=0;
  swp_list[swp_ptr].size=-1;
  swp_ptr=1;
  }

int swap_find_block(long size)
  {
  int i;
  SW_FREE_BLOCK *p;

  p=swp_list;
  for(i=0;i<swp_ptr;i++,p++)
      if (p->size>=size) return i;
      else if (p->seek==-1) swp_fnot_used=i;
  return 0;
  }

int swap_add_block(long size)
  {
  int i;
  long sp;
  SW_FREE_BLOCK *p;

  size+=255;
  size&=~255;
  i=swap_find_block(size);
  p=&swp_list[i];
  sp=p->seek;
  if (p->size==-1) p->seek+=size;
  else if (p->size==size)
     {
     p->seek=-1;
     p->size=-1;
     swp_unuseds++;
     swp_fnot_used=i;
     }
  else
     {
     p->seek+=size;
     p->size-=size;
     }
  return sp;
  }

void swap_find_seek(long seek1,long seek2,int *pos1,int *pos2)
  {
  int i;
  SW_FREE_BLOCK *p;

  p=swp_list;*pos1=-1;*pos2=-1;
  for(i=0;i<swp_ptr;i++,p++)
      {
      if (p->seek==seek2)
        {
        *pos2=i;
        if (*pos1>=0) return;
        }
      if (p->seek+p->size==seek1)
        {
        *pos1=i;
        if (*pos2>=0) return;
        }
      }
  }


void alloc_swp_block(long seek,long size)
  {

  if (swp_fnot_used<0 && swp_unuseds) swap_find_block(0x7fffffff);
  if (swp_fnot_used<0)
     {
     swp_list[swp_ptr].seek=seek;
     swp_list[swp_ptr].size=size;
     swp_unuseds=0;
     swp_ptr++;if (swp_ptr>=SWAP_FREE_LIST)
        {
        swpl_overruns++;
        return;
        }
     }
  else
     {
     swp_list[swp_fnot_used].seek=seek;
     swp_list[swp_fnot_used].size=size;
     swp_fnot_used=-1;
     swp_unuseds--;
     }
  }

void swap_free_block(long seek,long size)
  {
  int i1,i2;

  seek&=~255;
  size+=255;
  size&=~255;
  swap_find_seek(seek,seek+size,&i1,&i2);
    if (i2>=0 && i1>=0)
     {
     if (swp_list[i2].size!=-1) swp_list[i2].size+=swp_list[i1].size+size;
     swp_list[i2].seek=swp_list[i1].seek;
     swp_list[i1].seek=-1;
     swp_list[i1].size=-1;
     swp_unuseds++;swp_fnot_used=i1;
     }
  else if (i1>=0) swp_list[i1].size+=size;
  else if (i2>=0)
     {
     if (swp_list[i2].size!=-1) swp_list[i2].size+=size;
     swp_list[i2].seek=seek;
     }
  if (i1<0 && i2<0)
     {
     alloc_swp_block(seek,size);
     }
  }

/*main()
  {
  int i1,i2,i3,i4;
  swap_init();
  i1=swap_add_block(1024);
  i2=swap_add_block(250);
  i3=swap_add_block(1024);
  i4=swap_add_block(1024);

  swap_free_block(i1,1024);
  swap_free_block(i3,1024);
  swap_free_block(i2,250);
  swap_free_block(i4,1024);
  }
 */
