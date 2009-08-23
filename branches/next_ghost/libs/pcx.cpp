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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "libs/pcx.h"
#include "libs/memman.h"
#include "libs/system.h"

#define SHADE_STEPS 5
#define SHADE_PAL (SHADE_STEPS*512*2)

//void *get_palette_ptr=NULL;


void decomprimate_line_256(uint8_t *src,int8_t *trg,int linelen,int *srcstep)
  {
  uint8_t *srcsave;

  srcsave=src;
  while (linelen--)
     {
     if (*src>=0xc0)
        {
        int i;
        i=*src++ & 0x3f;memset(trg,*src++,i);
        trg+=i;linelen-=i-1;
        }
     else
        *trg++=*src++;
     }
  *srcstep=src-srcsave;
  }
void decomprimate_line_hi(uint8_t *src,uint16_t *trg,uint16_t *paleta,int linelen,int *srcstep)
  {
  uint8_t *srcsave;

  srcsave=src;
  while (linelen--)
     {
     if (*src>=0xc0)
        {
        int i,j;
        i=(*src++) & 0x3f;
        for (j=0;j<i;j++) *trg++=paleta[*src];
        src++;
        linelen-=i-1;
        }
     else
        *trg++=paleta[*src++];
     }
  *srcstep=src-srcsave;
  }

void palette_shadow(uint8_t *pal1,uint16_t pal2[][256],int tr,int tg,int tb)
  {
  int i,j;
  uint8_t *bt;
  int r,g,b;
  uint16_t hi;

  for (j=0;j<SHADE_STEPS;j++)
     {
     bt=pal1;
     i=0;
     do
       {
       r=(tr+(*(bt++)-tr)*(3*SHADE_STEPS-3*j-1)/(3*SHADE_STEPS-1))>>3;
       g=(tg+(*(bt++)-tg)*(3*SHADE_STEPS-3*j-1)/(3*SHADE_STEPS-1))>>3;
       b=(tb+(*(bt++)-tb)*(3*SHADE_STEPS-3*j-1)/(3*SHADE_STEPS-1))>>3;
//       hi=(r<<11)+(g<<6)+b;
	hi = Screen_RGB(r, g, b);
       pal2[j][i]=hi;
       }
    while (++i & 0xff);
    }
  for (j=0;j<SHADE_STEPS;j++)
     {
     bt=pal1;
     i=0;
     do
       {
       r=((*(bt++))*(SHADE_STEPS-j)/SHADE_STEPS)>>3;
       g=((*(bt++))*(SHADE_STEPS-j)/SHADE_STEPS)>>3;
       b=((*(bt++))*(SHADE_STEPS-j)/SHADE_STEPS)>>3;
//       hi=(r<<11)+(g<<6)+b;
	hi = Screen_RGB(r, g, b);
       pal2[j+SHADE_STEPS][i]=hi;
       }
    while (++i & 0xff);
    }
  }


int load_pcx(char *pcx,long fsize,int conv_type,uint8_t **buffer, ... )
  //dale nasleduji int hodnoty poctu prechodu a R,G,B barvy
  {
  uint16_t paleta2[256];
  uint8_t *paleta1;
  uint8_t *ptr1;uint16_t *ptr2;
  int8_t *ptr3;
  int i;
  PCXHEADER pcxdata;
  int xsize,ysize;


  if (pcx==0) return -1;
  paleta1 = (uint8_t*)pcx+fsize-768;
  ptr1=paleta1;ptr2=paleta2;
/*
  if (get_palette_ptr!=NULL)
     memcpy(get_palette_ptr,ptr1,768);
*/

	for (i = 0; i < 256; i++) {
		*ptr2++ = Screen_RGB(ptr1[0] >> 3, ptr1[1] >> 3, ptr1[2] >> 3);
		ptr1 += 3;
/*
     *ptr2=*(ptr1++)>>3;
     *ptr2=(*ptr2<<5)+(*(ptr1++)>>3);
     *ptr2=(*ptr2<<6)+(*(ptr1++)>>3);
     ptr2++;
*/
	}

  memcpy(&pcxdata,pcx,sizeof(pcxdata));
  xsize=pcxdata.xmax-pcxdata.xmin+1;
  ysize=pcxdata.ymax-pcxdata.ymin+1;
  switch (conv_type)
     {
     case A_8BIT: *buffer=(uint8_t *)getmem(xsize*ysize+256*sizeof(uint16_t)+16);break;
     case A_16BIT: *buffer=(uint8_t *)getmem(xsize*ysize*2+16);break;
     case A_FADE_PAL: *buffer=(uint8_t *)getmem(xsize*ysize+SHADE_PAL+16);break;
     case A_8BIT_NOPAL: *buffer=(uint8_t *)getmem(xsize*ysize+16);break;
     case A_NORMAL_PAL: *buffer=(uint8_t *)getmem(xsize*ysize+16+768);break;
     default: return -2; //invalid type specificied
     }
  ptr1=*buffer;
  *(uint16_t *)ptr1++=xsize;ptr1++;
  *(uint16_t *)ptr1++=ysize;ptr1++;
  *(uint16_t *)ptr1++=conv_type;ptr1++;
  pcx+=sizeof(pcxdata);
  ptr3 = (int8_t*)pcx;
  if (conv_type==A_NORMAL_PAL)
     {
     memcpy(ptr1,paleta1,768);
     ptr1+=768;
     }
  if (conv_type==A_8BIT)
     {
     memcpy(ptr1,paleta2,256*sizeof(uint16_t));
     ptr1+=256*sizeof(uint16_t);
     }
  if (conv_type==A_FADE_PAL)
     {
     int *i,tr,tg,tb;

     i=(int *)&buffer;i++;
     tr=*i++;tg=*i++;tb=*i++;
     palette_shadow(paleta1,(uint16_t (*)[256])ptr1,tr,tg,tb);
     ptr1+=SHADE_PAL;
     }
  ysize++;
  while (--ysize)
     {
     int step;
     if (conv_type==A_16BIT)
        {
        decomprimate_line_hi((uint8_t*)ptr3,(uint16_t *)ptr1,paleta2,pcxdata.bytesperline,&step);
        ptr1+=2*xsize;
        }
     else
        {
        decomprimate_line_256((uint8_t*)ptr3,(int8_t*)ptr1,pcxdata.bytesperline,&step);
        ptr1+=xsize;
        }
     ptr3+=step;
     }
  return 0;

}

int open_pcx(char *filename,int type,uint8_t **buffer,...)
  {
  FILE *pcx;
  char *src;
  long fsize;

  pcx=fopen(filename,"rb");
  if (pcx==NULL) return -1;
  fseek(pcx,0,SEEK_END);
  fsize=ftell(pcx);
  fseek(pcx,0,SEEK_SET);
  src=(char*)getmem(fsize);
  fread(src,1,fsize,pcx);
  fsize=load_pcx(src,fsize,type,buffer,*((int *)&buffer+1),*((int *)&buffer+2),*((int *)&buffer+3));
  fclose(pcx);
  free(src);
  return fsize;
  }

/*void initmode32b();

main()
  {
  int8_t *buf;

  initmode32b();
  open_pcx("DESK.pcx",A_8BIT,&buf,0,0,0);
  put_picture(0,480-102,buf);
  showview(0,0,0,0);
  getchar();
  return 0;
  }


*/

