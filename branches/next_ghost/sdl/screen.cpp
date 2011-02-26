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

SDLRenderer::SDLRenderer(unsigned xs, unsigned ys) : SoftRenderer(xs, ys),
	_screen(NULL), _remap(NULL), _mouse(NULL), _x(0), _mousex(0),
	_mousey(0), _drawMouse(0) {

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
	SDL_FreeSurface(_mouse);
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
	_active = 0;
}

void SDLRenderer::drawRect(unsigned x, unsigned y, unsigned xs, unsigned ys) {
	SDL_LockSurface(_remap);
	memcpy(_remap->pixels, pixels(), width() * height() * 3 * sizeof(uint8_t));
	SDL_UnlockSurface(_remap);
	flushRect(x, y, xs, ys);
}

void SDLRenderer::flushRect(unsigned x, unsigned y, unsigned xs, unsigned ys) {
	SDL_Rect dstrect = {x + _x, y, xs, ys}, srcrect = {x, y, xs, ys};
	SDL_Rect rect3 = {0, 0, 0, height()};
	Uint32 black = SDL_MapRGB(_screen->format, 0, 0, 0);

	if (_x > 0) {
		rect3.x = 0;
		rect3.w = _x;
		SDL_FillRect(_screen, &rect3, black);
	} else if (_x < 0) {
		rect3.x = width() + _x;
		rect3.w = -_x;
		SDL_FillRect(_screen, &rect3, black);
	}

	SDL_BlitSurface(_remap, &srcrect, _screen, &dstrect);

	if (_drawMouse) {
		dstrect.x = _mousex;
		dstrect.y = _mousey;
		dstrect.w = _mouse->w;
		dstrect.h = _mouse->h;
		SDL_BlitSurface(_mouse, NULL, _screen, &dstrect);
	}

	x = x < 0 ? 0 : x;
	x = x >= width() ? width() : x;
	y = y < 0 ? 0 : y;
	y = y >= height() ? height() : y;
	xs = x + xs > width() ? width() - x : xs;
	ys = y + ys > height() ? height() - y : ys;
	SDL_UpdateRect(_screen, x, y, xs, ys);
}

void SDLRenderer::xshift(int shift) {
	_x = shift;
}

void SDLRenderer::setMouseCursor(const Texture &tex) {
	SDL_Color *pal;
	int i, w, h;

	assert(tex.depth() == 1 && "Mouse cursor must use palette");

	w = _mouse && _mouse->w > tex.width() ? _mouse->w : tex.width();
	h = _mouse && _mouse->h > tex.height() ? _mouse->h : tex.height();
	SDL_FreeSurface(_mouse);
	_mouse = SDL_CreateRGBSurface(SDL_SWSURFACE, tex.width(), tex.height(), 8, 0, 0, 0, 0);
	SDL_LockSurface(_mouse);
	memset(_mouse->pixels, 0, _mouse->pitch * _mouse->h * sizeof(uint8_t));

	for (i = 0; i < tex.height(); i++) {
		memcpy((uint8_t*)_mouse->pixels + i * _mouse->pitch, tex.pixels() + i * tex.width(), tex.width() * sizeof(uint8_t));
	}

	SDL_UnlockSurface(_mouse);
	pal = new SDL_Color[256];

	for (i = 0; i < 256; i++) {
		pal[i].r = tex.palette()[3 * i];
		pal[i].g = tex.palette()[3 * i + 1];
		pal[i].b = tex.palette()[3 * i + 2];
		pal[i].unused = 0;
	}

	SDL_SetPalette(_mouse, SDL_LOGPAL, pal, 0, 256);
	delete[] pal;
	SDL_SetColorKey(_mouse, SDL_SRCCOLORKEY, 0);

	if (_drawMouse) {
		flushRect(_mousex, _mousey, w, h);
	}
}

void SDLRenderer::showMouse(void) {
	assert(_mouse && "No mouse texture set before showMouse() call");
	_drawMouse = 1;
	flushRect(_mousex, _mousey, _mouse->w, _mouse->h);
}

void SDLRenderer::hideMouse(void) {
	_drawMouse = 0;

	if (_mouse) {
		flushRect(_mousex, _mousey, _mouse->w, _mouse->h);
	}
}

void SDLRenderer::moveMouse(int x, int y) {
	int top, left, bottom, right;

	top = y < _mousey ? y : _mousey;
	left = x < _mousex ? x : _mousex;
	bottom = (y < _mousey ? _mousey : y) - top;
	right = (x < _mousex ? _mousex : x) - left;
	_mousex = x;
	_mousey = y;

	if (_drawMouse) {
		flushRect(left, top, right + _mouse->w, bottom + _mouse->h);
	}
}

unsigned SDLRenderer::memsize(void) const {
	unsigned ret;

	ret = sizeof(SDL_Surface) + _screen->h * _screen->pitch;
	ret += sizeof(SDL_Surface) + _remap->h * _remap->pitch;
	ret += _screen->format ? sizeof(SDL_PixelFormat) : 0;
	ret += _remap->format ? sizeof(SDL_PixelFormat) : 0;

	if (_mouse) {
		ret += sizeof(SDL_Surface) + _mouse->h * _mouse->pitch;
		ret += _mouse->format ? sizeof(SDL_PixelFormat) : 0;
	}

	return ret + sizeof(*this);
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
