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
#ifndef __LIBS_ZVUK_H___
#define __LIBS_ZVUK_H___

#define SND_MAXFUNCT 11
#define SND_PING    0  //Ping function
#define SND_GVOLUME 1  //SetGlobalVolume
#define SND_BASS    2  //SetBass
#define SND_TREBL   3  //SetTrebles
#define SND_SWAP    4  //SetSwapChannels
#define SND_LSWAP   5  //SetLinearSwapping
#define SND_SURROUND 6 //Surrourd
#define SND_OUTFILTER 7//Out Filter
#define SND_GFX     8  //setgfxvolume
#define SND_MUSIC   9  //setmusicvolume
#define SND_XBASS  10  //setxbassy

extern void (*konec_skladby)(char **jmeno);

#endif
