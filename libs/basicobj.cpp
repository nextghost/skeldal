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
// toto je include soubor, jenz je pouzit v knihovne GUI.C

#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <cstring>
#include "libs/memman.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/gui.h"
#include "libs/basicobj.h"

#define MEMTEXT "Pamˆt: "

#define E_STATUS_LINE 60

//FC_TABLE f_bila={0xffff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};

void highlight(CTL3D *c, uint8_t r, uint8_t g, uint8_t b) {
	c->light[0] = r >= 128 ? 255 : 2 * r;
	c->light[1] = g >= 128 ? 255 : 2 * g;
	c->light[2] = b >= 128 ? 255 : 2 * b;
	c->shadow[0] = r / 2;
	c->shadow[1] = g / 2;
	c->shadow[2] = b / 2;
}

CTL3D *def_border(int btype, uint8_t r, uint8_t g, uint8_t b) {
	static CTL3D ctl;

	highlight(&ctl, r, g, b);

	switch (btype) {
	case 0:
		ctl.bsize = 0;
		// FIXME: really fall through?

	case 1:
		ctl.light[0] = ctl.shadow[0] = r;
		ctl.light[1] = ctl.shadow[1] = g;
		ctl.light[2] = ctl.shadow[2] = b;
		ctl.bsize = 1;
		break;

	case 2:
		ctl.bsize = 2;
		ctl.ctldef = 0;
		break;

	case 3:
		ctl.bsize = 2;
		ctl.ctldef = 3;
		break;

	case 4:
		ctl.bsize = 1;
		ctl.ctldef = 0;
		break;

	case 5:
		ctl.bsize = 1;
		ctl.ctldef = 1;
		break;

	case 6:
		ctl.bsize = 2;
		ctl.ctldef = 1;
		break;

	case 7:
		ctl.bsize = 2;
		ctl.ctldef = 2;
		break;
	}

	return &ctl;
}

//------------------------------------------

Button::Button(int id, int x, int y, int width, int height, int align,
	const char *text) : GUIObject(id, x, y, width, height, align),
	_text(NULL), _data(0) {

	_text = new char[strlen(text) + 1];
	strcpy(_text, text);
}

Button::~Button(void) {
	delete[] _text;
}

void Button::draw(int x, int y, int width, int height) {
	CTL3D ctl;

	bar(x, y, x + width, y + height);
	highlight(&ctl, _color[0], _color[1], _color[2]);
	ctl.bsize = 2 - _data;
	ctl.ctldef = 3 * _data;
	draw_border(x + 2, y + 2, width - 4, height - 4, &ctl);
	renderer->drawAlignedText(x + width / 2 + _data * 2, y + height / 2 + _data * 2, HALIGN_CENTER, VALIGN_CENTER, _text);
}

void Button::event(EVENT_MSG *msg) {
	if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->event_type & 0x06) {
			if (ms->tl1) {
				_data = 1;
				redraw_object(this);
			} else if (_data) {
				_data = 0;
				redraw_object(this);
				set_change();
			}
		}
	}

	if (msg->msg == E_GET_FOCUS || msg->msg == E_LOST_FOCUS) {
		_data = 0;
		redraw_object(this);
	}
}

//------------------------------------------

/*
void *status_mem_info(EVENT_MSG *msg)
  {
  char *c;
  unsigned long long l;
  static char memtext[]=MEMTEXT;
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof(mem);

  if (msg->msg==E_INIT) return &status_mem_info;
  c=(char *)msg->data;
  strcpy(c,memtext);
  c+=strlen(memtext);
  GlobalMemoryStatusEx(&mem);
  l=mem.ullAvailPhys;
  sprintf(c,"%u KB ",l/1024);
  c=strchr(c,'\0');
  msg->data=(void *)c;
  return NULL;
  }
*/

void *status_idle(EVENT_MSG *msg, void **data)
  {
  if (msg->msg==E_INIT) return (void*)&status_idle;
  send_message(E_STATUS_LINE,msg->msg);
  return NULL;
  }

/* The following code is seriously broken
void status_line(EVENT_MSG *msg, T_EVENT_ROOT **user_data) {
	T_EVENT_ROOT **p;
	static char st_line[256], oldline[256] = {"\0"};
	EVENT_MSG tg;
	static char recurse = 1;
	
	if(msg->msg == E_INIT) {
		if (recurse) {
			T_EVENT_ROOT *p;
			recurse = 0;
			send_message(E_ADD, E_IDLE, status_idle);
			send_message(E_ADD, E_REDRAW, status_idle);
			p = NULL;
			*user_data = p;
			draw_status_line(NULL);
			recurse = 1;
			return;
		} else {
			return;
		}
	}

	va_copy(tg.data, msg->data);
	tg.msg = va_arg(tg.data, int);

	if (tg.msg == E_REDRAW) {
		draw_status_line(oldline);
		va_end(tg.data);
		return;
	}

	p = user_data;

	if (tg.msg == E_IDLE) {
		EVENT_MSG msg;

		msg.msg = E_IDLE;
		msg.data = &st_line;
		enter_event(p, &msg);

		if (strcmp(st_line, oldline)) {
			draw_status_line(st_line);
			strcpy(oldline, st_line);
		}
	} else {
		tree_basics(p, &tg);
	}

	va_end(tg.data);
	return;
}

void *mouse_xy(EVENT_MSG *msg, void **userdata) {
	char *c;

	if (msg->msg == E_INIT) {
		return &mouse_xy;
	}

	c = (char *)msg->data;
	sprintf(c, " X: %d Y: %d", ms_last_event.x, ms_last_event.y);
	c = strchr(c, '\0');
	msg->data = (void *)c;
	return NULL;
}

void *show_time(EVENT_MSG *msg) {
	char *c;
	time_t t;
	struct tm cas;

	if (msg->msg == E_INIT) {
		return &show_time;
	}

	c = (char *)msg->data;
	t = time(NULL);
	cas = *localtime(&t);

	sprintf(c, "%02d:%02d:%02d ", cas.tm_hour, cas.tm_min, cas.tm_sec);
	c = strchr(c, '\0');
	msg->data = (void *)c;
	return NULL;
}
*/
//------------------------------------------

void xor_rectangle(int x, int y, int xs, int ys) {
	curcolor[0] = 255;
	curcolor[1] = 255;
	curcolor[2] = 255;

	if (x < 0) {
		x = 0;
	}

	if (y < 0) {
		y = 0;
	}

	if (x + xs >= renderer->width()) {
		xs = renderer->width() - x - 1;
	}

	if (y + ys >= renderer->height()) {
		ys = renderer->height() - y - 1;
	}

	hor_line_xor(x, y, x + xs);
	ver_line_xor(x, y, y + ys);

	if (xs && ys) {
		hor_line_xor(x, y + ys, x + xs);
		ver_line_xor(x + xs, y, y + ys);
	}

	showview(x, y, x + xs, 4);
	showview(x, y, 4, ys + 4);

	if (xs && ys) {
		showview(x,y+ys,x+xs,4);
		showview(x+xs,y,4,ys+4);
	}
}

void win_label_move(EVENT_MSG *msg) {
	MS_EVENT *ms;
	static char run = 0;
	static uint16_t xref, yref;
	static WINDOW w;
	static int moved = 0;
	static int drawed = 0;

	if (msg->msg == E_INIT) {
		return;
	}

	if (msg->msg == E_TIMER) {
		send_message(E_TIMER);
		if (!drawed && !moved) {
			drawed=1;
			redraw_desktop();
			moved=0;
		} else {
			drawed=0;
			moved=0;
		}
	}

	if (msg->msg == E_MOUSE) {
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (run) {
			if (ms->event_type & 4) {
				run = 0;
				redraw_desktop();
				send_message(E_DONE, E_MOUSE, win_label_move);
				msg->msg = -1;
				return;
			}

			w.x = ms->x - xref;
			w.y = ms->y - yref;
			check_window(&w);
			waktual->x = w.x;
			waktual->y = w.y;
			waktual->xs = w.xs;
			waktual->ys = w.ys;
			moved = 1;
			drawed = 0;
			redraw_window();
			redraw_desktop();
		} else if (ms->event_type & 2) {
			run = 1;
			memcpy(&w, waktual, sizeof(WINDOW));
			xref = ms->x - waktual->x;
			yref = ms->y - waktual->y;
			send_message(E_ADD, E_MOUSE, win_label_move);
			freeze_on_exit = 1;
		}
	}

	if (msg->msg == E_LOST_FOCUS && run) {
		run = 0;
		redraw_desktop();
		send_message(E_DONE, E_MOUSE, win_label_move);
		msg->msg = -1;
		return;
	}

	msg->msg = -1;
	return;
}

WindowLabel::WindowLabel(int id, int x, int y, int width, int height, int align,
	const char *text) : GUIObject(id, x, y, width, height, align),
	_text(NULL) {

	_text = new char[strlen(text) + 1];
	strcpy(_text, text);
}

WindowLabel::~WindowLabel(void) {
	delete[] _text;
}

void WindowLabel::draw(int x, int y, int width, int height) {
	bar(x, y, x + width, y + height);
	renderer->drawAlignedText(x + 5, y + height / 2, HALIGN_LEFT, VALIGN_CENTER, _text);
}

void WindowLabel::event(EVENT_MSG *msg) {
	win_label_move(msg);
}

//------------------------------------------

//------------------------------------------

InputLine::InputLine(int id, int x, int y, int width, int height, int align,
	int length, const char *str) : GUIObject(id, x, y, width, height,
	align), _length(length), _shift(0), _str(NULL) {

	_str = new char[_length + 1];
	memset(_str, 0, (_length + 1) * sizeof(char));
	strcpy(_str, str);
}

InputLine::~InputLine(void) {
	delete[] _str;
}

void InputLine::draw(int x, int y, int width, int height) {
	char d[2] = " ", *str = _str;
	int tw, writepos;
	int len;
	int shift;

	bar(x, y, x + width, y + height);

	if (!*str) {
		return;
	}

	len = strlen(str);
	shift = _shift;

	if (shift >= len) {
		shift = 0;
	}

	str += shift;
	d[0] = *str++;
	tw = x + renderer->textWidth(d);
	writepos = x;

	while (tw < x + width) {
		renderer->drawText(writepos, y, d);
		writepos = tw;

		if (!*str) {
			break;
		}

		d[0] = *str++;
		tw = writepos + renderer->textWidth(d);
	}
}

// FIXME: get rid of static vars
void InputLine::event(EVENT_MSG *msg) {
	static int cursor = 0;
	int slen;
	static char *save;
	static char clear_context;
	va_list args;

	slen = strlen(_str);

	switch (msg->msg) {
	case E_GET_FOCUS:
		cursor = 0;
		save = new char[_length + 1];
		strcpy(save, _str);
		clear_context = 1;
		break;

	case E_LOST_FOCUS:
		cursor = 0;
		_shift = 0;
		delete[] save;
		redraw_object(this);
		break;

	case E_CURSOR_TICK:
		{
			int xpos, i, j = -1, d;

			do {
				xpos = 0;
				j++;

				for (i = _shift; i < cursor; i++) {
					if ((d = renderer->charWidth(_str[i])) == 0) {
						xpos += 1;
					} else {
						xpos += d;
					}
				}

				if (xpos >= _x + _width) {
					_shift += 1;
				}

				if (xpos == 0) {
					_shift -= 1;
				}

				if (_shift < 0) {
					_shift = 0;
				}
			} while ((xpos == 0 || xpos >= _x + _width) && cursor);

			if (j) {
				redraw_object(this);
			}

			xor_rectangle(_locx + xpos, _locy, 1, _height);
		}
		break;

	case E_MOUSE:
		{
			MS_EVENT *ms;
			int msx;

			va_copy(args, msg->data);
			ms = va_arg(args, MS_EVENT*);
			va_end(args);
			msx = ms->x - _locx;

			if (ms->event_type & 2) {
				int xpos;

				xpos = 0;

				for (cursor = _shift; cursor < slen; cursor++) {
					xpos += renderer->charWidth(_str[cursor]);
					if (xpos > msx) {
						break;
					}
				}

				redraw_object(this);
			}
		}
		break;

	case E_KEYBOARD:
		{
			int key;

			cancel_event();
			va_copy(args, msg->data);
			key = va_arg(args, int);
			va_end(args);

			if (!(key & 0xff)) {
				switch (key >> 8) {
				case 'M':
					if (cursor < slen) {
						cursor++;
					}
					break;

				case 'K':
					if (cursor > 0) {
						cursor--;
					}
					break;

				case 'S':
					if (cursor < slen) {
						strcpy(&_str[cursor], &_str[cursor+1]);
						slen--;
					}
					break;

				case 'G':
					cursor = 0;
					break;

				case 'O':
					cursor = slen;
					break;
				}
			} else {
				key &= 0xff;

				switch (key) {
				case 8:
					if (cursor > 0) {
						strcpy(&_str[cursor-1], &_str[cursor]);
						cursor--;
					}
					break;

				case 0:
					break;

				case 13:
					break;

				case 27:
					strcpy(_str, save);
					slen = strlen(_str);
					if (cursor > slen) {
						cursor = slen;
					}
					break;

				default:
					if (key >= ' ' && (slen < _length || clear_context)) {
						int i;

						if (clear_context) {
							*_str = '\0';
							cursor = 0;
							slen = 0;
						}

						for (i = slen + 1; i > cursor; i--) {
							_str[i] = _str[i-1];
						}

						_str[cursor++] = key;
					}
					break;
				}
			}

			if (!cursor) {
				_shift = 0;
			}

			redraw_object(this);
			msg->msg = E_CURSOR_TICK;
			event(msg);
			clear_context = 0;
			msg->msg = -1;
		}
	}
}

//-------------------------------------------------------------

Label::Label(int id, int x, int y, int width, int height, int align,
	const char *text) : GUIObject(id, x, y, width, height, align),
	_text(NULL) {

	_text = new char[strlen(text) + 1];
	strcpy(_text, text);
}

Label::~Label(void) {
	delete[] _text;
}

void Label::draw(int x, int y, int width, int height) {
	renderer->drawText(x, y, _text);
}
