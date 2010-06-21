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

#ifndef _INTERFAC_H_
#define _INTERFAC_H_

#include "libs/gui.h"

class SkeldalCheckBox : public GUIObject {
	int _phase;
	Texture *_background;

public:
	SkeldalCheckBox(int id, int x, int y, int width, int height, int align, int value = 0);
	~SkeldalCheckBox(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);

	void nextFrame(void);
	int getValue() const { return _phase & 1; }
	void setValue(int value);
};

class SetupOkButton : public GUIObject {
	char *_text;
	Texture *_background;
	int _toggle;

public:
	SetupOkButton(int id, int x, int y, int width, int height, int align, const char *text);
	~SetupOkButton(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);
};

class SkeldalSlider : public GUIObject {
	const int _range;
	int _value;
	Texture *_background;

public:
	SkeldalSlider(int id, int x, int y, int width, int height, int align, int range, int value = 0);
	~SkeldalSlider(void);

	void draw(int x, int y, int width, int height);
	void event(EVENT_MSG *msg);

	int getValue(void) const { return _value; }
};

#endif
