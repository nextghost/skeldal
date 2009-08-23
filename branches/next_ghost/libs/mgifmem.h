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
//!!!! POZOR, NUTNE LINKOVAT SOUBOR LZWA.ASM
#ifndef _MGIFMEM_H
#define _MGIFMEM_H

#include <inttypes.h>

#pragma pack(1)
typedef void (*MGIF_PROC)(int,void *,int csize); //prvni cislo akce, druhy data akce

#define MGIF "MGIF"
#define MGIF_Y "97"
#define VER 0x100
#define MGIF_EMPTY  0
#define MGIF_LZW    1
#define MGIF_DELTA  2
#define MGIF_PAL    3
#define MGIF_SOUND  4
#define MGIF_TEXT   5
#define MGIF_COPY   6
#define MGIF_SINIT  7

#define SMD_256 1
#define SMD_HICOLOR 2

typedef struct mgif_header
    {
    char sign[4];
    char year[2];
    int8_t eof;
    uint16_t ver;
    int32_t frames;
    uint16_t snd_chans;
    int32_t snd_freq;
    int16_t ampl_table[256];
    int16_t reserved[32];
    }MGIF_HEADER_T;


void mgif_install_proc(MGIF_PROC proc);
void *open_mgif(void *mgif); //vraci ukazatel na prvni frame
void *mgif_play(void *mgif); //dekoduje a zobrazi frame
void close_mgif();           //dealokuje buffery pro prehravani

void show_full_lfb12e(uint16_t *target,uint8_t *buff,uint16_t *paleta);
void show_delta_lfb12e(uint16_t *target,uint8_t *buff,uint16_t *paleta);

#pragma option align=reset
#endif
