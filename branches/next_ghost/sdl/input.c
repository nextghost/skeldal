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
#include "libs/system.h"
#include "libs/bmouse.h"
#include "libs/event.h"

word scancodes[SDLK_LAST] = {0};
static int kbhit = 0;

// FIXME: change game input system
void Mouse_GetEvent(MS_EVENT *event) {
	*event = ms_last_event;
	event->event = 0;
}

// FIXME: change game input system
int get_control_state(void) {
	SDL_PumpEvents();

	return (SDL_GetModState() & KMOD_CTRL) != 0;
}

// FIXME: change game input system
int get_shift_state(void) {
	SDL_PumpEvents();

	return (SDL_GetModState() & KMOD_SHIFT) != 0;
}

int Input_Kbhit(void) {
	Sys_ProcessEvents();
	return kbhit != 0;
}

int Input_ReadKey(void) {
	kbhit = 0;
	return 0;
}

void Sys_ProcessEvents(void) {
	SDL_Event event;
	word keycode;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			kbhit = 1;
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
