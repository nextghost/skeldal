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
#define A_8BIT 8
#define A_15BIT 15
#define A_FADE_PAL 256*8

typedef struct pcxrecord
     {
     unsigned short id;
     char encoding;
     char bitperpixel;
     unsigned short xmin,ymin,xmax,ymax;
     unsigned short hdpi,vdpi;
     char colormap[48];
     char reserved;
     char mplanes;
     unsigned short bytesperline;
     unsigned short paleteinfo;
     unsigned short hscreen,vscreen;
     char filler[54];
     }PCXHEADER;



int load_pcx(char *pcx,long fsize,int conv_type,char **buffer, ... );
int open_pcx(char *filename,int type,char **buffer,...);

