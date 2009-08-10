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
#include "libs/bgraph.h"
//#include "libs/bgraph2dx.h"
#include <stdio.h>
#include <string.h>
#include "libs/types.h"
#include "libs/memman.h"
//#include "libs/mem.h"
#include "libs/mgifmem.h"
#include "libs/sound.h"
#include "libs/system.h"

//static HANDLE mapped_mgif;
//static HANDLE mgif_file;
static MGIF_HEADER_T *mgif_header;

static short mgif_accnums[2];
static long mgif_writepos;

void *OpenMGFFile(const char *filename);
void CloseMGFFile(void *file);

/* Moved to windows backend directory
static void *OpenMGFFile(const char *filename)
  {
  mgif_file=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (mgif_file==INVALID_HANDLE_VALUE) return NULL;
  mapped_mgif=CreateFileMapping(mgif_file,NULL,PAGE_READONLY,0,0,NULL);
  if (mapped_mgif==INVALID_HANDLE_VALUE) return NULL;
  return MapViewOfFile(mapped_mgif,FILE_MAP_READ,0,0,0);
  }
*/

static word paleta[256];

static word *picture;
static word *anim_render_buffer;
static void *sound;

static void StretchImageHQ(word *src, word *trg, unsigned long linelen, char full) {
	word xs = src[0], ys = src[1];
	word *s, *t;
	int x, y;
	src += 3;  
	for (y = 0, s = src, t = trg; y < ys; y++, t += linelen * 2, s += xs) {
		for (x = 0; x < xs; x++) {
			word val;
//			t[x*2] = s[x] + (s[x] & 0x7fe0);
			t[x*2] = s[x];
			if (x) {
//				val = ((s[x-1] & 0x7bde) + (s[x] & 0x7bde)) >> 1;
//				t[x*2-1] = val + (val & 0x7fe0);
				t[x*2-1] = Screen_ColorAvg(s[x-1], s[x]);
			}
			if (full) {
				if (y) {
//					val = ((s[x-xs] & 0x7bde) + (s[x] & 0x7bde)) >> 1;
//					t[x*2-linelen] = val + (val & 0x7fe0);
					t[x*2-linelen] = Screen_ColorAvg(s[x-xs], s[x]);
				}
				if (x && y) {
//					val=((s[x-xs-1] & 0x7bde) + (s[x] & 0x7bde))>>1;
//					t[x*2-linelen-1] = val + (val & 0x7fe0);
					t[x*2-linelen-1] = Screen_ColorAvg(s[x-xs-1], s[x]);
				}
			}
		}
	}
}

static void PlayMGFFile(void *file, MGIF_PROC proc,int ypos,char full)
  {
  mgif_install_proc(proc);
  sound=PrepareVideoSound(22050,256*1024);
  mgif_accnums[0]=mgif_accnums[1]=0;
  mgif_writepos=65536;
  picture=getmem(2*3+320*180*2);
  picture[0]=320;
  picture[1]=180;
  picture[2]=15;
  memset(picture+3,0,320*180*2);
  anim_render_buffer=picture+3;
  mgif_header=(MGIF_HEADER_T *)file;
  file=open_mgif(file);
  if (file==NULL) return;
  while (file) 
	{
//    __try
	  {
	  file=mgif_play(file);
	  }
//  __except(1)
/*
  	  {
	  SEND_LOG("(PLAYANIM) Exception raised",0,0);
	  file=NULL;
	  }
*/
	StretchImageHQ(picture, Screen_GetAddr()+ypos*Screen_GetXSize(), Screen_GetXSize(),full);
	showview(0,ypos,0,360);
//	if (_bios_keybrd(_KEYBRD_READY)==0) Sound_MixBack(0);
	if (!Input_Kbhit()) Sound_MixBack(0);
	else 
	  {
//	  _bios_keybrd(_KEYBRD_READ);
	  Input_ReadKey();
	  break;
	  }
	}
  close_mgif();  
  DoneVideoSound(sound);
  free(picture);
  }

void show_full_lfb12e(word *target,byte *buff,word *paleta);
void show_delta_lfb12e(word *target,byte *buff,word *paleta);
void show_delta_lfb12e_dx(void *target,void *buff,void *paleta);
void show_full_lfb12e_dx(void *target,void *buff,void *paleta);


void BigPlayProc(int act, void *data, int csize)
  {
  switch (act)
     {
     case MGIF_LZW:
     case MGIF_COPY:
     	show_full_lfb12e(anim_render_buffer, data, paleta);
	break;

     case MGIF_DELTA:
     	show_delta_lfb12e(anim_render_buffer, data, paleta);
	break;

     case MGIF_PAL:
//     	paleta = data;
	memcpy(paleta, data, csize);
	Screen_FixMGIFPalette(paleta, csize / 2);
	break;

     case MGIF_SOUND: 
		while (LoadNextVideoFrame(sound,data,csize,mgif_header->ampl_table,mgif_accnums,&mgif_writepos)==0) {
			Timer_Sleep(5);
		}
     }
  }

void play_animation(char *filename, char mode, int posy, char sound) {
	void *mgf = OpenMGFFile(filename);
	Sound_ChangeMusic(NULL);
	if (!mgf) {
		return;
	}
	PlayMGFFile(mgf, BigPlayProc, posy, mode & 0x80);
	CloseMGFFile(mgf);
}

void set_title_list(char **titles)
  {

  }
void set_play_attribs(void *screen,char rdraw,char bm,char colr64)
  {

  }
