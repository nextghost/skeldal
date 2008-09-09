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
 *  Last commit made by: $Id: MGIFMEM.H 7 2008-01-14 20:14:25Z bredysoft $
 */
//!!!! POZOR, NUTNE LINKOVAT SOUBOR LZWA.ASM

typedef void (*MGIF_PROC)(int,void *); //prvni cislo akce, druhy data akce

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


void mgif_install_proc(MGIF_PROC proc);
void *open_mgif(void *mgif); //vraci ukazatel na prvni frame
void *mgif_play(void *mgif); //dekoduje a zobrazi frame
void close_mgif();           //dealokuje buffery pro prehravani
