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
//#include <skeldal_win.h>
#include <stdio.h>
#include "libs/types.h"
#include "libs/memman.h"
//#include "libs/mem.h"
#include "libs/mgifmem.h"

#define MGIF "MGIF"
#define LZW_MAX_CODES  16384
#define LZW_BUFFER 64000

MGIF_PROC show_proc;
static int mgif_frames;
static int cur_frame;


typedef struct double_s
  {
  short group,chr,first,next;
  }DOUBLE_S;

typedef DOUBLE_S CODE_TABLE[LZW_MAX_CODES];

DOUBLE_S *compress_dic=NULL;
static void *lzw_buffer=NULL;
static int clear_code;
static int end_code;
static int free_code;
static int nextgroup;
static int bitsize,init_bitsize;
char old_value=0;

void do_clear_code()
  {
  int i;

  old_value=0;
  nextgroup=free_code;
  bitsize=init_bitsize;
  for(i=0;i<clear_code;i++)
     {
     DOUBLE_S *p;
     p=&compress_dic[i];
     p->group=i;p->chr=-1;p->next=-1;p->first=-1;
     }
  }

void reinit_lzw()
  {
  do_clear_code();
  }

void init_lzw_compressor(int dic_size)
  {

	if (!compress_dic) {
		compress_dic = (DOUBLE_S *)getmem(sizeof(CODE_TABLE));
	}
  clear_code=1<<dic_size;
  end_code=clear_code+1;
  free_code=end_code+1;
  nextgroup=free_code;
  init_bitsize=bitsize=dic_size+1;
  do_clear_code();
  }


void done_lzw_compressor()
  {
  free(compress_dic);
  compress_dic=NULL;
  }

void mgif_install_proc(MGIF_PROC proc)
  {
  show_proc=proc;
  }

void *open_mgif(void *mgif) //vraci ukazatel na prvni frame
  {
  char *c;
  struct mgif_header *mgh;

  c=mgif;
  if (strncmp(c,MGIF,4)) return NULL;
  mgh=mgif;
  mgif_frames=mgh->frames;
  cur_frame=0;
  c+=sizeof(*mgh);
  init_lzw_compressor(8);
  if (lzw_buffer==NULL) lzw_buffer=getmem(LZW_BUFFER);
  return c;
  }

void close_mgif()           //dealokuje buffery pro prehravani
  {
  done_lzw_compressor();
  free(lzw_buffer);
  lzw_buffer=NULL;
  }


unsigned input_code(byte *source,long *bitepos,int bitsize,int mask) {
	unsigned ret = *(unsigned *)(source + (*bitepos >> 3));
	ret >>= *bitepos & 0x7;
	*bitepos += bitsize;
	return ret & mask;
}


int de_add_code(int group,int chr,int mask)
  {
  DOUBLE_S *q;

  q=&compress_dic[nextgroup];q->group=group;q->chr=chr;q->first=compress_dic[group].first+1;
  nextgroup++;
  if (nextgroup==mask)
     {
     mask=(mask<<1)+1;
     bitsize++;
     }
  return mask;
  }


char fast_expand_code(int code, byte **target) {
	if (code < 256) {
		old_value = **target = (code + old_value) & 0xff;
		++*target;
		return code;
	}

	*target += compress_dic[code].first;
	char ret, *ptr = *target;
	int idx = code;

	do {
		*ptr-- = compress_dic[idx].chr & 0xff;
	} while ((idx = compress_dic[idx].group) >= 256);

	ret = idx;
	idx = (idx + old_value) & 0xff;
	*ptr++ = idx;
	code = compress_dic[code].first;

	do {
		idx = (*ptr + idx) & 0xff;
		*ptr++ = idx;
	} while (--code);

	old_value = idx;
	++*target;
	return ret;
}

void lzw_decode(void *source,byte *target)
  {
  long bitpos=0;
  register int code;
  int old,i;
  //int group,chr;
  int old_first;
  register int mask=0xff;


  for(i=0;i<LZW_MAX_CODES;i++) compress_dic[i].first=0;
  clear:
  old_value=0;
  nextgroup=free_code;
  bitsize=init_bitsize;
  mask=(1<<bitsize)-1;
  code=input_code(source,&bitpos,bitsize,mask);
  old_first=fast_expand_code(code,&target);
//  old_first=expand_code(code,&target);
  old=code;
  while ((code=input_code(source,&bitpos,bitsize,mask))!=end_code)
     {
     if (code==clear_code)
        {
        goto clear;
        }
     else if (code<nextgroup)
        {
        old_first=fast_expand_code(code,&target);
//        old_first=expand_code(code,&target);
        //group=old;
        //chr=old_first;
        mask=de_add_code(old,old_first,mask);
        old=code;
        }
     else
        {
        //p.group=old;
        //p.chr=old_first;
        mask=de_add_code(old,old_first,mask);
        old_first=fast_expand_code(code,&target);
//        old_first=expand_code(code,&target);
        old=code;
        }
     }
    }

//dekoduje a zobrazi frame
void *mgif_play(void *mgif) {
	byte *pf, *pc, *ff;
	int acts, size, act, csize;
	void *scr_sav;
	int scr_act = -1;
	
	pf = mgif;
	acts = *pf++;
	size = (*(unsigned*)pf) & 0xffffff;
	pf += 3;
	pc = pf;
	pf += size;
	for (; acts; acts--) {
		act = *pc++;
		csize = (*(unsigned*)pc) & 0xffffff;
		pc += 3;
		if ((act == MGIF_LZW) || (act == MGIF_DELTA)) {
			ff = lzw_buffer;
			lzw_decode(pc, ff);
			scr_sav = ff;
			scr_act = act;
		} else if (act == MGIF_COPY) {
			scr_sav = ff;
			scr_act = act;
		} else {
			ff = pc;
			show_proc(act, ff, csize);
		}

		pc += csize;
	}

	if (scr_act != -1) {
		show_proc(scr_act, scr_sav, csize);
	}

	cur_frame += 1;

	if (cur_frame == mgif_frames) {
		return NULL;
	}

	return pf;
}







