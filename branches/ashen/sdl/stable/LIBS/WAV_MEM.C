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
 *  Last commit made by: $Id: WAV_MEM.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "WAV_MEM.H"

char *find_chunk(char *wav,char *name)
  {
  long next;

  wav += 12;
  do
     {
     if (!strncmp(name,wav,4)) return wav+4;
     wav += 4;
     memcpy(&next,wav,4);
     wav += next+4;
     }
  while (1);
  }

int get_chunk_size(char *wav)
  {
  long size;

  memcpy(&size,wav,4);
  return(size);
  }

int read_chunk(char *wav,void *mem)
  {

  wav += 4;
  memcpy(mem,wav,get_chunk_size(wav-4));
  return 0;
  }


