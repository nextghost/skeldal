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
#include "libs/bmouse.h"
#include "libs/event.h"

word scancodes[SDLK_LAST] = {0};

void Mouse_GetEvent(MS_EVENT *event) {
	*event = ms_last_event;
	event->event = 0;
/*
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
*/
}

int get_control_state(void) {
	SDL_PumpEvents();

	return (SDL_GetModState() & KMOD_CTRL) != 0;
}

int get_shift_state(void) {
	SDL_PumpEvents();

	return (SDL_GetModState() & KMOD_SHIFT) != 0;
}

void Sys_ProcessEvents(void) {
	SDL_Event event;
	word keycode;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			keycode = scancodes[event.key.keysym.sym];
			if (!(event.key.keysym.unicode & 0xff80)) {
				keycode |= event.key.keysym.unicode;
			}

			send_message(E_KEYBOARD, keycode);
			break;

		case SDL_MOUSEMOTION:
			ms_last_event.event = 1;
			ms_last_event.x = event.motion.x;
			ms_last_event.y = event.motion.y;
			ms_last_event.tl1 = !!(event.motion.state & SDL_BUTTON(1));
			ms_last_event.tl2 = !!(event.motion.state & SDL_BUTTON(3));
			ms_last_event.tl3 = !!(event.motion.state & SDL_BUTTON(2));
			ms_last_event.event_type = 0x1;

			send_message(E_MOUSE, &ms_last_event);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			ms_last_event.event = 1;
			ms_last_event.x = event.button.x;
			ms_last_event.y = event.button.y;

			if (event.button.button == SDL_BUTTON_LEFT) {
				ms_last_event.event_type = event.button.state == SDL_PRESSED ? 0x2 : 0x4;
				ms_last_event.tl1 = event.button.state == SDL_PRESSED;
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				ms_last_event.event_type = event.button.state == SDL_PRESSED ? 0x8 : 0x10;
				ms_last_event.tl2 = event.button.state == SDL_PRESSED;
			} else if (event.button.button == SDL_BUTTON_MIDDLE) {
				ms_last_event.event_type = event.button.state == SDL_PRESSED ? 0x20 : 0x40;
				ms_last_event.tl3 = event.button.state == SDL_PRESSED;
			}

			send_message(E_MOUSE, &ms_last_event);
			break;
		}
	}
}
