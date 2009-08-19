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
#ifndef _PCX_H_
#define _PCX_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

#define A_8BIT 8
#define A_16BIT 16
#define A_FADE_PAL (256+8)
#define A_8BIT_NOPAL (512+8)
#define A_NORMAL_PAL (768+8)

  typedef struct pcxrecord
     {
     uint16_t id;
     int8_t encoding;
     int8_t bitperpixel;
     uint16_t xmin,ymin,xmax,ymax;
     uint16_t hdpi,vdpi;
     int8_t colormap[48];
     int8_t reserved;
     int8_t mplanes;
     uint16_t bytesperline;
     uint16_t paleteinfo;
     uint16_t hscreen,vscreen;
     int8_t filler[54];
     }PCXHEADER;


int load_pcx(char *pcx,long fsize,int conv_type,int8_t **buffer, ... );
int open_pcx(char *filename,int type,int8_t **buffer,...);
void palette_shadow(uint8_t *pal1,uint16_t pal2[][256],int tr,int tg,int tb);
//extern void *get_palette_ptr;

#pragma option align=reset

#ifdef __cplusplus
}
#endif
#endif
