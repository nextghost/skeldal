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

typedef void (*MGIF_PROC)(int,void *,int csize); //prvni cislo akce, druhy data akce

#define _MGIFMEM_H
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
    char eof;
    word ver;
    long frames;
    word snd_chans;
    int snd_freq;
    short ampl_table[256];
    short reserved[32];
    }MGIF_HEADER_T;


void mgif_install_proc(MGIF_PROC proc);
void *open_mgif(void *mgif); //vraci ukazatel na prvni frame
void *mgif_play(void *mgif); //dekoduje a zobrazi frame
void close_mgif();           //dealokuje buffery pro prehravani
#endif
