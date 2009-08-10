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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "libs/system.h"

SDL_Surface *screen;
void *backBuffer, *frontBuffer, *curFront;

extern word scancodes[];

char Screen_Init(char windowed, int zoom, int monitor, int refresh) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		return 0;
	}

	screen = SDL_SetVideoMode(640, 480, 15, SDL_SWSURFACE);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1);

	if (!screen) {
		return 0;
	}

	SDL_WM_SetCaption("Gates of Skeldal", "Gates of Skeldal");

/*
	fprintf(stderr, "Masks: %02x %02x %02x\n", screen->format->Rmask, screen->format->Gmask, screen->format->Bmask);
	fprintf(stderr, "Shifts: %d %d %d\n", screen->format->Rshift, screen->format->Gshift, screen->format->Bshift);

	if (SDL_MUSTLOCK(screen)) {
		fprintf(stderr, "Screen needs locking :-(\n");
		return 0;
	}
*/

	curFront = frontBuffer = malloc(screen->h * screen->pitch);
	backBuffer = malloc(screen->h * screen->pitch);

	scancodes[SDLK_TAB] = 0x0f00;
	scancodes[SDLK_RETURN] = 0x1c00;
	scancodes[SDLK_ESCAPE] = 0x0100;
	scancodes[SDLK_SPACE] = 0x3900;
	scancodes[SDLK_UP] = 0x4800;
	scancodes[SDLK_DOWN] = 0x5000;
	scancodes[SDLK_RIGHT] = 0x4d00;
	scancodes[SDLK_LEFT] = 0x4b00;
	scancodes[SDLK_f] = 0x2100;
	scancodes[SDLK_i] = 0x1700;
	scancodes[SDLK_m] = 0x3200;
	scancodes[SDLK_INSERT] = 0x5200;
	scancodes[SDLK_END] = 0x7300;
	scancodes[SDLK_PAGEDOWN] = 0x7400;
	scancodes[SDLK_F2] = 0x3c00;
	scancodes[SDLK_F3] = 0x3d00;
	scancodes[SDLK_F4] = 0x3e00;

	return 1;
}

void Screen_Shutdown(void) {
	SDL_EnableUNICODE(0);
	SDL_ShowCursor(SDL_ENABLE);
	free(frontBuffer);
	free(backBuffer);
	SDL_Quit();
}

int Screen_GetXSize(void) {
	return screen->w;
}

int Screen_GetYSize(void) {
	return screen->h;
}

unsigned short *Screen_GetAddr(void) {
	return curFront;
}

long Screen_GetSize(void) {
	return screen->h * screen->pitch;
}

int Screen_GetScan(void) {
	return screen->pitch;
}

unsigned short *Screen_GetBackAddr(void) {
	return backBuffer;
}

void Screen_SetAddr(unsigned short *addr) {
	curFront = addr;
}

void Screen_SetBackAddr() {
	curFront = backBuffer;
}

void Screen_Restore(void) {
	curFront = frontBuffer;
}

void Screen_DrawRect(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {
	SDL_LockSurface(screen);
	memcpy(screen->pixels, frontBuffer, Screen_GetSize());
	SDL_UpdateRect(screen, x, y, xs, ys);
	SDL_UnlockSurface(screen);
}

void Screen_FixPalette(word *pal, int size) {
	int i, r, g, b;

	for (i = 0; i < size; i++) {
		r = (pal[i] >> 11) & 0x1f;
		g = (pal[i] >> 6) & 0x1f;
		b = pal[i] & 0x1f;
	
		pal[i] = Screen_RGB(r, g, b);
	}
}


void Screen_FixMGIFPalette(word *pal, int size) {
	unsigned i, r, g, b;

	for (i = 0; i < size; i++) {
		r = (pal[i] >> 10) & 0x1f;
		g = (pal[i] >> 5) & 0x1f;
		b = pal[i] & 0x1f;
	
		pal[i] = Screen_RGB(r, g, b);
	}
}

word Screen_RGB(unsigned r, unsigned g, unsigned b) {
	return (r << screen->format->Rshift) | (g << screen->format->Gshift) | 
		(b << screen->format->Bshift);
}

word Screen_ColorMin(word c1, word c2) {
	unsigned r, g, b;

	r = min(c1 & screen->format->Rmask, c2 & screen->format->Rmask);
	g = min(c1 & screen->format->Gmask, c2 & screen->format->Gmask);
	b = min(c1 & screen->format->Bmask, c2 & screen->format->Bmask);

	return r | g | b;
}

word Screen_ColorSub(word color, int sub) {
	int r, g, b;

	r = ((color & screen->format->Rmask) >> screen->format->Rshift) - sub;
	g = ((color & screen->format->Gmask) >> screen->format->Gshift) - sub;
	b = ((color & screen->format->Bmask) >> screen->format->Bshift) - sub;

	return Screen_RGB(r < 0 ? 0 : r, g < 0 ? 0 : g, b < 0 ? 0 : b);
}

word Screen_ColorAvg(word c1, word c2) {
	unsigned r, g, b;

	r = (c1 & screen->format->Rmask) + (c2 & screen->format->Rmask);
	g = (c1 & screen->format->Gmask) + (c2 & screen->format->Gmask);
	b = (c1 & screen->format->Bmask) + (c2 & screen->format->Bmask);

	r = (r / 2) & screen->format->Rmask;
	g = (g / 2) & screen->format->Gmask;
	b = (b / 2) & screen->format->Bmask;

	return r | g | b;
}

word Screen_ColorBlend(word c1, word c2, float factor) {
	unsigned r1, g1, b1, r2, g2, b2;
	
	r1 = ((c1 & screen->format->Rmask) >> screen->format->Rshift);
	g1 = ((c1 & screen->format->Gmask) >> screen->format->Gshift);
	b1 = ((c1 & screen->format->Bmask) >> screen->format->Bshift);

	r2 = ((c2 & screen->format->Rmask) >> screen->format->Rshift);
	g2 = ((c2 & screen->format->Gmask) >> screen->format->Gshift);
	b2 = ((c2 & screen->format->Bmask) >> screen->format->Bshift);

	r1 = r1 + factor * (r2 - r1);
	g1 = g1 + factor * (g2 - g1);
	b1 = b1 + factor * (b2 - b1);

	return Screen_RGB(r1, g1, b1);
}

unsigned Screen_ColorR(word c) {
	return (c & screen->format->Rmask) >> screen->format->Rshift;
}

unsigned Screen_ColorG(word c) {
	return (c & screen->format->Gmask) >> screen->format->Gshift;
}

unsigned Screen_ColorB(word c) {
	return (c & screen->format->Bmask) >> screen->format->Bshift;
}
