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

#include <stdio.h>
#include <SDL/SDL.h>
#include "libs/devices.h"

void Mouse_GetEvent(MS_EVENT *event) {
	static Uint8 oldstate = 0;
	static int oldx = 0, oldy = 0;
	Uint8 newstate, diff;
	int x, y;

	SDL_PumpEvents();
	newstate = SDL_GetMouseState(&x, &y);

//	fprintf(stderr, "oldstate %02x, newstate %02x, x %d, y %d, oldx %d, oldy %d\n", oldstate, newstate, x, y, oldx, oldy);
	diff = oldstate ^ newstate;

	event->event_type = 0;
	event->x = x;
	event->y = y;

	if (x != oldx || y != oldy) {
		event->event_type |= 0x1;
		oldx = x;
		oldy = y;
	}

	if (diff & SDL_BUTTON(1)) {
		event->event_type |= newstate & SDL_BUTTON(1) ? 0x2 : 0x4;
	}

	if (diff & SDL_BUTTON(3)) {
		event->event_type |= newstate & SDL_BUTTON(3) ? 0x8 : 0x10;
	}

	if (diff & SDL_BUTTON(2)) {
		event->event_type |= newstate & SDL_BUTTON(2) ? 0x20 : 0x40;
	}

	oldstate = newstate;

//	fprintf(stderr, "event_type %02x\n", event->event_type);

	event->event = event->event_type != 0;
	event->tl1 = !!(newstate & SDL_BUTTON(1));	// left
	event->tl2 = !!(newstate & SDL_BUTTON(3));	// right
	event->tl3 = !!(newstate & SDL_BUTTON(2));	// middle
}
