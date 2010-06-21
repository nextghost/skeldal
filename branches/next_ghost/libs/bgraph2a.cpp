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
#include <cstring>
#include <cassert>
#include <inttypes.h>
#include "libs/bgraph.h"


void bar32(int x1, int y1, int x2, int y2) {
	uint16_t *begline;
	int i, j;

	if (x1 > x2) {
		swap_int(x1, x2);
	}

	if (y1 > y2) {
		swap_int(y1, y2);
	}

	if (x1 < 0) {
		x1 = 0;
	}

	if (y1 < 0) {
		y1 = 0;
	}

	renderer->bar(x1, y1, x2 - x1 + 1, y2 - y1 + 1, curcolor[0], curcolor[1], curcolor[2]);
}

void hor_line_xor(int x1, int y1, int x2) {
	if (x1 > x2) {
		swap_int(x1, x2);
	}

	if (x1 < 0) {
		x1 = 0;
	}

	renderer->bar(x1, y1, x2 - x1 + 1, 1, curcolor[0], curcolor[1], curcolor[2], 3);
}

void ver_line_xor(int x1, int y1, int y2) {
	if (y1 > y2) {
		swap_int(y1, y2);
	}

	if (y1 < 0) {
		y1 = 0;
	}

	renderer->bar(x1, y1, 1, y2 - y1 + 1, curcolor[0], curcolor[1], curcolor[3], 3);
}

void trans_bar(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b) {
	int x1 = x;
	int y1 = y;
	int x2 = x + xs - 1;
	int y2 = y + ys - 1;

	if (x1 > x2) {
		swap_int(x1, x2);
	}

	if (y1 > y2) {
		swap_int(y1,y2);
	}

	if (x1 < 0) {
		x1 = 0;
	}

	if (y1 < 0) {
		y1 = 0;
	}

	renderer->bar(x1, y1, x2 - x1 + 1, y2 - y1 + 1, r, g, b, 1);
}

void trans_line_x(int x, int y, int xs, uint8_t r, uint8_t g, uint8_t b) {
	trans_bar(x, y, xs, 1, r, g, b);
}

void trans_line_y(int x, int y, int ys, uint8_t r, uint8_t g, uint8_t b) {
	trans_bar(x, y, 1, ys, r, g, b);
}

void trans_bar25(int x, int y, int xs, int ys) {
	int x1 = x;
	int y1 = y;
	int x2 = x + xs - 1;
	int y2 = y + ys - 1;

	if (x1 > x2) {
		swap_int(x1, x2);
	}

	if (y1 > y2) {
		swap_int(y1, y2);
	}

	if (x1 < 0) {
		x1 = 0;
	}

	if (y1 < 0) {
		y1 = 0;
	}

	renderer->bar(x1, y1, x2 - x1 + 1, y2 - y1 + 1, 4, 4, 4, 2);
}

void wait_retrace()
  {

  }
