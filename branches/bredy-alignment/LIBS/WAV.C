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
#include <skeldal_win.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "WAV.H"

int find_chunk(FILE *riff,char *name)
  {
  char chunk_name[4];
  long next;

  fseek(riff,12,SEEK_SET);
  do
     {
     fread(chunk_name,1,4,riff);
     if (!strncmp(name,chunk_name,4)) return ftell(riff);
     if (fread(&next,1,4,riff)==0) return -1 ;
     if (fseek(riff,next,SEEK_CUR))return -1 ;
     }
  while (!feof(riff));
  return -1;
  }

int get_chunk_size(FILE *riff)
  {
  long size;

  fread(&size,1,4,riff);
  fseek(riff,-4,SEEK_CUR);
  return(size);
  }

int read_chunk(FILE *riff,void *mem)
  {
  long size,res;

  fread(&size,1,4,riff);
  res=fread(mem,1,size,riff);
  return res==size;
  }


