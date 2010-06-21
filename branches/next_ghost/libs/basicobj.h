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

#ifndef _BASICOBJ_H_
#define _BASICOBJ_H_

#include <inttypes.h>
#include "libs/gui.h"

#define MEMTEXT "Pamˆt: "

#define E_STATUS_LINE 60

void highlight(CTL3D *c, uint8_t r, uint8_t g, uint8_t b);
CTL3D *def_border(int btype, uint8_t r, uint8_t g, uint8_t b);
void xor_rectangle(int x,int y,int xs,int ys);

// status lines
//void *status_mem_info(EVENT_MSG *msg);
void *mouse_xy(EVENT_MSG *msg);
void *show_time(EVENT_MSG *msg);

// objects
class Button : public GUIObject {
protected:
	char *_text;
	int _data;

public:
	Button(int id, int x, int y, int width, int height, int align, const char *text);
	~Button(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);
};

class WindowLabel : public GUIObject {
private:
	char *_text;

public:
	WindowLabel(int id, int x, int y, int width, int height, int align, const char *text);
	~WindowLabel(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);
};

class InputLine : public GUIObject {
private:
	int _length, _shift;
	char *_str;

public:
	InputLine(int id, int x, int y, int width, int height, int align, int length, const char *str = "");
	~InputLine(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);

	const char *getString(void) const { return _str; }
};

class Label : public GUIObject {
private:
	char *_text;

public:
	Label(int id, int x, int y, int width, int height, int align, const char *text);
	~Label(void);

	void draw(int x, int y, int width, int height);
};

#endif
