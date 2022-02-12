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

#include "libs/bgraph.h"

class SDLRenderer : public SoftRenderer {
private:
	static int _active;
	SDL_Surface *_screen, *_remap, *_mouse;
	int _x, _mousex, _mousey, _drawMouse;

	void flushRect(unsigned x, unsigned y, unsigned xs, unsigned ys);

public:
	SDLRenderer(unsigned xs, unsigned ys);
	~SDLRenderer(void);

	void drawRect(unsigned x, unsigned y, unsigned xs, unsigned ys);
	void xshift(int shift);
	void setMouseCursor(const Texture &tex);
	void showMouse(void);
	void hideMouse(void);
	void moveMouse(int x, int y);

	size_t memsize(void) const;
};
