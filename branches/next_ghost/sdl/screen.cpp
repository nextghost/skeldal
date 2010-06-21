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

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <SDL/SDL.h>
#include "sdl/screen.h"
#include "libs/system.h"

SDL_Surface *screen;
uint16_t *backBuffer, *frontBuffer, *curFront;

extern uint16_t scancodes[];

int SDLRenderer::_active = 0;

SDLRenderer::SDLRenderer(unsigned xs, unsigned ys) : SoftRenderer(xs, ys) {
	assert(!_active && "Video already initialized");
	_active = 1;
	_screen = SDL_SetVideoMode(xs, ys, 24, SDL_SWSURFACE);
	SDL_ShowCursor(SDL_DISABLE);
	assert(_screen);
	SDL_WM_SetCaption("Gates of Skeldal", "Gates of Skeldal");
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	_remap = SDL_CreateRGBSurface(SDL_SWSURFACE, xs, ys, 24, 0xff0000, 0xff00, 0xff, 0);
#else
	_remap = SDL_CreateRGBSurface(SDL_SWSURFACE, xs, ys, 24, 0xff, 0xff00, 0xff0000, 0);
#endif
	drawRect(0, 0, xs, ys);
}

SDLRenderer::~SDLRenderer(void) {
	SDL_FreeSurface(_remap);
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
	_active = 0;
}

void SDLRenderer::drawRect(unsigned x, unsigned y, unsigned xs, unsigned ys) {
	SDL_Rect rect = {x, y, xs, ys};
	SDL_LockSurface(_remap);
	memcpy(_remap->pixels, pixels(), width() * height() * 3 * sizeof(uint8_t));
	SDL_UnlockSurface(_remap);
	SDL_BlitSurface(_remap, &rect, _screen, &rect);
	SDL_UpdateRect(_screen, x, y, xs, ys);
}

char Screen_Init(char windowed, int zoom, int monitor, int refresh) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		return 0;
	}

	assert(!renderer && "Video already initialized");
	renderer = new SDLRenderer(640, 480);
	backbuffer = new SoftRenderer(640, 480);
	SDL_EnableUNICODE(1);

/*
	fprintf(stderr, "Masks: %02x %02x %02x\n", screen->format->Rmask, screen->format->Gmask, screen->format->Bmask);
	fprintf(stderr, "Shifts: %d %d %d\n", screen->format->Rshift, screen->format->Gshift, screen->format->Bshift);
*/

	// FIXME: mapped to mimic original DOS version behavior in DOSBox
	// on a notebook keyboard, should be double checked
	scancodes[SDLK_TAB] = 0x0f00;
	scancodes[SDLK_RETURN] = 0x1c00;
	scancodes[SDLK_ESCAPE] = 0x0100;
	scancodes[SDLK_SPACE] = 0x3900;
	// english keyboard numrow
	scancodes[SDLK_1] = 0x0200;
	scancodes[SDLK_2] = 0x0300;
	scancodes[SDLK_3] = 0x0400;
	scancodes[SDLK_4] = 0x0500;
	scancodes[SDLK_5] = 0x0600;
	scancodes[SDLK_6] = 0x0700;
	// czech keyboard numrow
	scancodes[SDLK_PLUS] = 0x0200;
	scancodes[SDLK_WORLD_76] = 0x0300;
	scancodes[SDLK_WORLD_25] = 0x0400;
	scancodes[SDLK_WORLD_72] = 0x0500;
	scancodes[SDLK_WORLD_88] = 0x0600;
	scancodes[SDLK_WORLD_30] = 0x0700;

	scancodes[SDLK_UP] = 0x4800;
	scancodes[SDLK_DOWN] = 0x5000;
	scancodes[SDLK_RIGHT] = 0x4d00;
	scancodes[SDLK_LEFT] = 0x4b00;
	scancodes[SDLK_f] = 0x2100;
	scancodes[SDLK_i] = 0x1700;
	scancodes[SDLK_m] = 0x3200;
	scancodes[SDLK_DELETE] = 0x5300;
	scancodes[SDLK_INSERT] = 0x5200;
	scancodes[SDLK_HOME] = 0x4700;
	scancodes[SDLK_END] = 0x7300;
	scancodes[SDLK_PAGEDOWN] = 0x7400;
	scancodes[SDLK_F2] = 0x3c00;
	scancodes[SDLK_F3] = 0x3d00;
	scancodes[SDLK_F4] = 0x3e00;

	return 1;
}

void Screen_Shutdown(void) {
	SDL_EnableUNICODE(0);
	delete renderer;
	delete backbuffer;
	renderer = NULL;
}
