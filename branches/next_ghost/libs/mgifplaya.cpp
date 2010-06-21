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
#include <cstdlib>
#include <inttypes.h>
#include "libs/mgfplay.h"
#include "libs/bgraph.h"

void show_full_lfb12e(uint16_t *target, ReadStream &stream, uint16_t *paleta) {
	int i;

	for (i = 0; i < 180 * 320; i++) {
		*target++ = paleta[stream.readUint8()];
	}
}

void show_delta_lfb12e(uint16_t *target, ReadStream &stream, uint16_t *paleta) {
	unsigned int i, j, k, size;
	uint16_t *tmp;
	uint8_t *map, *oldmap;

	size = stream.readUint32LE();
	oldmap = map = new uint8_t[size+1];

	for (i = 0; i < size; i++) {
		map[i] = stream.readUint8();
	}

	for (i = 0; i < 180; i += j) {
		tmp = target;

		for (j = *map++; (j & 0xc0) != 0xc0; j = *map++) {
			target += 2 * j;
			for (k = 0; k < *map; k++) {
				*target++ = paleta[stream.readUint8()];
				*target++ = paleta[stream.readUint8()];
			}
			map++;
		}

		j = (j & 0x3f) + 1;
		target = tmp + j * 320;
	}

	delete[] oldmap;
}

char test_next_frame(void *bufpos,int size)
  {
  return 0;
  }
//#pragma aux test_next_frame parm [edi][ecx] modify [ebx] value [al]

void *sound_decompress(void *source,void *bufpos,int size,void *ampl_tab)
  {
  return NULL;
  }
//#pragma aux sound_decompress parm [esi][edi][ecx][ebx] modify [eax edx] value [edi]
