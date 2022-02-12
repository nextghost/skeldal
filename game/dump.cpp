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
#include <cstdio>
#include <cassert>
#include <inttypes.h>
#include "libs/bgraph.h"
#include "libs/memman.h"
#include "libs/system.h"

void save_dump() {
	static int dump_counter = -1;
	int i, r, g, b, x, y;
	char c[20];
	WriteFile f;

	if (dump_counter == -1) {
		dump_counter = Sys_LatestFile("DUMP*.BMP", 4);
		SEND_LOG("(DUMP) Dump counter sets to %d", dump_counter, 0);
	}

	sprintf(c, "DUMP%04d.BMP", ++dump_counter);
	SEND_LOG("(DUMP) Saving screen shot named '%s'", c, 0);
	f.open(c);
	f.writeUint8('B');
	f.writeUint8('M');
	f.writeUint32LE(renderer->width() * renderer->height() * 3 + 0x36);
	f.writeUint32LE(0);
	f.writeUint32LE(0x36);
	f.writeUint32LE(0x28);
	f.writeUint32LE(renderer->width());
	f.writeUint32LE(renderer->height());
	f.writeUint16LE(1);
	f.writeUint16LE(24);
	f.writeUint32LE(0);
	f.writeUint32LE(renderer->width() * renderer->height() * 3);

	for (i = 4, r = 0; i > 0; i--) {
		f.writeUint32LE(0);
	}

	for (y = renderer->height(); y > 0; y--) {
		const uint8_t *scr = renderer->pixels() + (y - 1) * renderer->width() * 3;

		for (x = 0; x < renderer->width(); x++) {
			f.writeUint8(scr[3 * x + 2]);
			f.writeUint8(scr[3 * x + 1]);
			f.writeUint8(scr[3 * x]);
		}
	}
}
