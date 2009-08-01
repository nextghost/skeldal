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
#ifndef _WAV_H

#define _WAV_H

#pragma pack(1)

#define WAV_RIFF "RIFF"
#define WAV_WAVE "WAVE"
#define WAV_FMT  "fmt "
#define WAV_DATA "data"

typedef struct t_wave
  {
  unsigned short wav_mode,chans;
  long freq,bps;
  }T_WAVE;

int find_chunk(FILE *riff,char *name); //-1 neuspech, jinak pozice
int get_chunk_size(FILE *riff);       //velikost
int read_chunk(FILE *riff,void *mem); // 1 neuspech

#pragma option align=reset

#endif
