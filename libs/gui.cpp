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
//Gui system - object system + graphic
#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "libs/memman.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/gui.h"

#define E_REDRAW_DESKTOP 2010
#define E_REDRAW_WINDOW 2000

const Texture *gui_background = NULL;

extern T_EVENT_ROOT *ev_tree;

WINDOW *desktop={NULL},*waktual={NULL};;
GUIObject *o_aktual={NULL},*o_end={NULL},*o_start={NULL};
CTL3D noneborder={0,0,0,0};
FC_TABLE f_default;
uint16_t desktop_y_size;
char force_redraw_desktop=0;
static char change_flag=0,f_cancel_event=0;
uint16_t *default_font;

const Font *GUIObject::_dFont = NULL;
int GUIObject::_dShadow = 0;
uint8_t GUIObject::_dColor[FONT_COLORS][3] = {{0}};

void empty() {

}

void empty2(EVENT_MSG *msg) {

}

void GUIObject::setDefaultFont(const Font *font, int shadow, uint8_t r, uint8_t g, uint8_t b) {
	int i;

	_dFont = font;
	_dShadow = shadow;
	_dColor[0][0] = 0;
	_dColor[0][1] = 0;
	_dColor[0][2] = 0;

	for (i = 1; i < 5; i++) {
		_dColor[i][0] = r;
		_dColor[i][1] = g;
		_dColor[i][2] = b;
	}
}

GUIObject::GUIObject(int id, int x, int y, int width, int height, int align) :
	_id(id), _x(x), _y(y), _width(width), _height(height), _align(align),
	_autoResizeX(false), _autoResizeY(false), _enabled(true),
	_drawError(false), _font(_dFont), _onEvent(empty2), _gotFocus(empty),
	_lostFocus(empty), _onActivate(empty), _shadow(_dShadow) {

	memcpy(_color, waktual->color, sizeof(_color));
	memcpy(_fColor, _dColor, sizeof(_dColor));
	memcpy(&_border, &noneborder, sizeof(CTL3D));
}

GUIObject::~GUIObject(void) {

}

void GUIObject::event(EVENT_MSG *msg) {

}

void GUIObject::onEvent(EVENT_MSG *msg) {
	_onEvent(msg);
}

void GUIObject::gotFocus() {
	_gotFocus();
}

void GUIObject::lostFocus() {
	_lostFocus();
}

void GUIObject::onActivate(EVENT_MSG *msg) {
	_onActivate();
}

void GUIObject::setOnEvent(void (*proc)(EVENT_MSG *msg)) {
	_onEvent = proc;
}

void GUIObject::setGotFocus(void (*proc)()) {
	_gotFocus = proc;
}

void GUIObject::setLostFocus(void (*proc)()) {
	_lostFocus = proc;
}

void GUIObject::setOnActivate(void (*proc)()) {
	_onActivate = proc;
}

void (*GUIObject::getOnActivate(void))() {
	return _onActivate;
}

void GUIObject::render(window *w, int show) {
	int ok;

	if (_drawError) {
		return;
	}

	_drawError = true;
	ok = w->border3d.bsize;
	align(w, _locx, _locy);

	if (_width < 1 || _height < 1) {
		_drawError = false;
		return;
	}

	draw_border(_locx, _locy, _width, _height, &_border);
	memcpy(curcolor, _color, 3 * sizeof(uint8_t));

	// FIXME: I think this is never used
	renderer->setFont(_font, _shadow, _fColor);
	draw(_locx, _locy, _width, _height);

	if (!_enabled) {
		disable_bar(_locx, _locy, _width, _height, _color[0], _color[1], _color[2]);
	}

	if (show) {
		showview(_locx - ok, _locy - ok, _width + 2 * ok, _height + 2 * ok);
	}

	_drawError = false;
}

void GUIObject::setFColor(unsigned idx, uint8_t r, uint8_t g, uint8_t b) {
	assert(idx < 7);
	_fColor[idx][0] = r;
	_fColor[idx][1] = g;
	_fColor[idx][2] = b;
}

void GUIObject::align(WINDOW *w, int &x, int &y) const {
	switch (_align) {
	case 0:
	case 1:
		y = _y + w->y;
		break;

	case 2:
	case 3:
		y = (w->y + w->ys) - (_y + _height);
		break;
	}

	switch (_align) {
	case 0:
	case 3:
		x = _x + w->x;
		break;

	case 1:
	case 2:
		x = (w->x + w->xs) - (_x + _width);
		break;
	}
}

void GUIObject::property(CTL3D *ctl, const Font *font, int shadow, uint8_t fcolor[][3], uint8_t *color) {
	if (ctl) {
		memcpy(&_border, ctl, sizeof(CTL3D));
	}

	if (font) {
		_font = font;
	}

	_shadow = shadow;

	if (fcolor) {
		memcpy(_fColor, fcolor, sizeof(_fColor));
	}

	if (color) {
		memcpy(_color, color, 3 * sizeof(uint8_t));
	}
}

bool GUIObject::inside(WINDOW *w, int x, int y) const {
	int x1, y1;

	align(w, x1, y1);
	return x >= x1 && x <= x1 + _width && y >= y1 && y <= y1 + _height;
}

void GUIObject::autoResize(int xdiff, int ydiff) {
	if (_autoResizeX) {
		_width += xdiff;
	}

	if (_autoResizeY) {
		_height += ydiff;
	}
}

void GUIObject::setEnabled(bool value) {
	_enabled = value;
}

void draw_border(int16_t x, int16_t y, int16_t xs, int16_t ys, CTL3D *btype) {
	uint16_t i, j;
	uint8_t c[3];

	memcpy(c, curcolor, 3 * sizeof(uint8_t));
	j = btype->ctldef;

	for (i = 0; i < btype->bsize; i++) {
		x--;
		y--;
		xs += 2;
		ys += 2;

		if (j & 1) {
			memcpy(curcolor, btype->shadow, 3 * sizeof(uint8_t));
		} else {
			memcpy(curcolor, btype->light, 3 * sizeof(uint8_t));
		}

		hor_line(x, y, xs + x);
		ver_line(x, y, ys + y);

		if (j & 1) {
			memcpy(curcolor, btype->light, 3 * sizeof(uint8_t));
		} else {
			memcpy(curcolor, btype->shadow, 3 * sizeof(uint8_t));
		}

		hor_line(x, y + ys, xs + x);
		ver_line(x + xs, y, ys + y);
		j >>= 1;
	}

	memcpy(curcolor, c, 3 * sizeof(uint8_t));
}

void check_window(WINDOW *w) {
	if (w->x <= w->border3d.bsize) {
		w->x = w->border3d.bsize;
	}

	if (w->y <= w->border3d.bsize) {
		w->y=w->border3d.bsize;
	}

	if (w->x >= renderer->width() - w->border3d.bsize - w->xs) {
		w->x = renderer->width() - w->border3d.bsize - w->xs - 1;
	}

	if (w->y >= desktop_y_size - w->border3d.bsize - w->ys) {
		w->y = desktop_y_size - w->border3d.bsize - w->ys - 1;
	}
}

void show_window(WINDOW *w)
  {
  int ok;

  ok=w->border3d.bsize;
  showview(w->x-ok,w->y-ok,w->xs+(ok<<1),w->ys+(ok<<1));
  }

void draw_cl_window(WINDOW *o) {
	memcpy(curcolor, o->color, 3 * sizeof(uint8_t));
	bar32(o->x, o->y, o->x + o->xs, o->y + o->ys);
	draw_border(o->x, o->y, o->xs, o->ys, &o->border3d);
}



WINDOW *create_window(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b, CTL3D *okraj) {
	WINDOW *p;

	p=(WINDOW *)getmem(sizeof(WINDOW));
	p->x = x;
	p->y = y;
	p->xs = xs;
	p->ys = ys;
	p->color[0] = r;
	p->color[1] = g;
	p->color[2] = b;
	memcpy(&(p->border3d), okraj, sizeof(CTL3D));
	p->objects = NULL;
	p->modal = 0;
	p->popup = 0;
	p->minimized = 0;
	p->window_name = NULL;
	p->minsizx = MINSIZX;
	p->minsizy = MINSIZY;
	p->idlist = NULL;
	p->txtid = -1;
	p->txtx = 0;
	p->txty = 0;
	p->draw_event = draw_cl_window;
	check_window(p);
	return p;
}

int send_lost()
  {
  EVENT_MSG msg;
   msg.msg=E_LOST_FOCUS;
  if (o_aktual!=NULL)
     {
     o_aktual->lostFocus();
     if (f_cancel_event) return -1;
     o_aktual->event(&msg);
     o_aktual=NULL;
     }
  return 0;
  }

void select_window(long id)
  {
  WINDOW *p,*q;

  if (waktual->id==id) return;
  if (waktual->modal) return;
  if (send_lost()) return;
  p=desktop;
  while (p!=NULL && p->id!=id) p=p->next;
  if (p==NULL) return;
  q=desktop;
  if (p!=desktop)
     {
        while (q->next!=p) q=q->next;
        q->next=p->next;
     }
  else
     {
     desktop=p->next;
     }
  p->next=NULL;
  waktual->next=p;
  waktual=p;
  if (waktual->objects!=NULL)
     {
     o_start = waktual->objects;
     o_aktual=NULL;
     o_end=o_start;
     while(o_end->_next!=NULL) o_end=o_end->_next;
     }
  else
     {o_start=NULL;o_aktual=NULL;o_end=NULL;}

  }

long desktop_add_window(WINDOW *w)
  {
  static long id_counter=0;

  w->id=id_counter++;
  w->next=NULL;
  if (desktop==NULL)
     {
     waktual=w;
     desktop=w;
      }
  else
     {
     if (o_aktual!=NULL)
        {
        EVENT_MSG msg;

        msg.msg=E_LOST_FOCUS;
        o_aktual->lostFocus();
        o_aktual->event(&msg);
        }
     waktual->next=w;
     waktual=w;
     }
  o_aktual=NULL;
  o_end=NULL;
  o_start=NULL;
  return w->id;
  }


WINDOW *find_window(long id)
  {
  WINDOW *p;

  p=desktop;
  while (p!=NULL && p->id!=id) p=p->next;
  return p;
  }


void disable_bar(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b) {
	renderer->bar(x, y, xs, ys, r, g, b, 1);
	renderer->bar(x, y, xs, ys, r, g, b, 1);
}

void redraw_object(GUIObject *o) {
	o->render(waktual, 1);
}


void redraw_window_call() {
	GUIObject *p;

	waktual->draw_event(waktual);
	p = waktual->objects;

	while (p != NULL) {
		p->render(waktual, 0);
		p = p->_next;
	}

	show_window(waktual);
	return;
}

void add_to_idlist(GUIObject *o)
  {
  TIDLIST *p,*q;

  p=(TIDLIST *)getmem(sizeof(TIDLIST));
  p->obj=o;
  if (waktual->idlist==NULL)
     {
     p->next=NULL;
     waktual->idlist=p;
     return;
     }
  if (((waktual->idlist)->obj)->id() > o->id())
     {
     p->next=waktual->idlist;
     waktual->idlist=p;
     return;
     }
  q=waktual->idlist;
  while (q->next!=NULL && ((q->next)->obj)->id() < o->id()) q=q->next;
  p->next=q->next;
  q->next=p;
  }


void define(GUIObject *o) {
	o->_next = NULL;
	if (!o_start) {
		o_start = o;
		o_end = o;
		o_aktual = NULL;
		waktual->objects = o;
	} else {
		o_end->_next = o;
		o_end = o;
	}

	add_to_idlist(o);
}

void property(CTL3D *ctl, const Font *font, int shadow, uint8_t fcolor[][3], uint8_t *color) {
	o_end->property(ctl, font, shadow, fcolor, color);
}

FC_PTR *flat_color(uint8_t r, uint8_t g, uint8_t b) {
	static uint8_t table[FONT_COLORS][3] = {{0}};
	int i;

	for (i = 1; i < FONT_COLORS; i++) {
		table[i][0] = r;
		table[i][1] = g;
		table[i][2] = b;
	}

	return table;
}

uint8_t *single_color(uint8_t r, uint8_t g, uint8_t b) {
	static uint8_t table[3] = {0};

	table[0] = r;
	table[1] = g;
	table[2] = b;
	return table;
}


void aktivate_window(MS_EVENT *ms)
  {
  WINDOW *p,*q;
  if (ms->event_type==1) return;
  p=desktop;q=NULL;
  while (p!=NULL)
     {
     if (ms->x>=p->x && ms->x<=p->x+p->xs && ms->y>=p->y && ms->y<=p->y+p->ys)
        q=p;
     p=p->next;
     }
  if (q==NULL) return;
  if (q!=waktual)
     {
     select_window(q->id);
     redraw_window();
     return;
     }
  return;
  }


void redraw_desktop_call(EVENT_MSG *msg, void **data) {
	WINDOW *w;
	int8_t *oz;

	if (msg->msg == E_INIT || msg->msg == E_DONE) {
		return;
	}

	if (!force_redraw_desktop) {
		return;
	}

	force_redraw_desktop = 0;

	if (gui_background == NULL) {
		uint8_t tmp[3] = {DESK_TOP_COLOR};
		memcpy(curcolor, tmp, 3 * sizeof(uint8_t));
		bar(0, 0, renderer->width(), renderer->height() - 1);
	} else {
		renderer->blit(*gui_background, 0, 0, gui_background->palette());
	}

	oz = otevri_zavoru;
	do_events();
	*oz = 1;
	w = desktop;

	while (w != NULL) {
		GUIObject *p;
		w->draw_event(w);
		p = w->objects;

		while (p != NULL) {
			p->render(w, 0);
			p = p->_next;
		}

		w = w->next;
		*oz = 0;
		do_events();
		*oz = 1;
	}

	send_message(E_REDRAW);
	showview(0, 0, 0, 0);
}

void redraw_desktop()
  {
  force_redraw_desktop=1;
  }

void redraw_window()
  {
  redraw_window_call();
  }

void close_window(WINDOW *w)
  {
  WINDOW *p;
  GUIObject *q;

  if (waktual==w && send_lost()) return;
  if (w==waktual && waktual!=desktop)
     {
     p=desktop;
     while (p->next!=waktual) p=p->next;
     w->modal=0;
     select_window(p->id);
     }
  if (w!=desktop)
     {
     p=desktop;
     while (p->next!=w) p=p->next;
     p->next=w->next;
     }
  else
     {
     desktop=w->next;
     if (desktop==NULL) waktual=NULL;
     }
  while (w->objects!=NULL)
     {
     q=w->objects;
     w->objects=q->_next;
	delete q;
     }
  while (w->idlist!=NULL)
     {
     TIDLIST *p;

     p=w->idlist;
     w->idlist=p->next;
     free(p);
     }

  free(w);
  if (desktop==NULL) exit_wait=1;
  redraw_desktop();
  }




//-------------------------------- GUI EVENTS -------------------------

GUIObject *get_last_id()
  {
  TIDLIST *p;

  p=waktual->idlist;
  while (p->next!=NULL) p=p->next;
  return p->obj;
  }

GUIObject *get_next_id(GUIObject *o)
  {
  TIDLIST *p;

  p=waktual->idlist;
  while (p!=NULL && p->obj!=o) p=p->next;
  if (p==NULL) return NULL;
  do
     {

     p=p->next;
     if (p==NULL) p=waktual->idlist;
     o=p->obj;
     if (o->isActive()) return o;
     }
  while (1);
  }

GUIObject *get_prev_id(GUIObject *o)
  {
  TIDLIST *p,*q;

  p=waktual->idlist;
  while (p!=NULL && p->obj!=o) p=p->next;
  if (p==NULL) return NULL;
  do
     {
     if (p==waktual->idlist)
        {
        p=waktual->idlist;
        while (p->next!=NULL) p=p->next;
        }
     else
        {
        q=waktual->idlist;
        while (q->next!=p) q=q->next;
        p=q;
        }
     o=p->obj;
     if (o->isActive()) return o;
     }
  while (1);
  }


void do_it_events(EVENT_MSG *msg, void **user_data) {
	MS_EVENT *msev;
	EVENT_MSG msg2;
	char b;
	static uint16_t cursor_tick = CURSOR_SPEED;
	GUIObject *p;
	int8_t *oz = otevri_zavoru;
	va_list args;
	
	if (msg->msg == E_INIT) {
		return;
	}

	if (desktop == NULL) {
		exit_wait = 1;
		return;
	}

	change_flag = 0;
	f_cancel_event = 0;

	if (o_aktual != NULL) {
		o_aktual->onEvent(msg);
	}

	if (msg->msg == E_MOUSE) {
		va_copy(args, msg->data);
		msev = va_arg(args, MS_EVENT*);
		va_end(args);

		*oz = 1;
		aktivate_window(msev);

		if (o_aktual == NULL) {
			if (o_start != NULL && (msev->tl1 || msev->tl2 || msev->tl3)) {
				o_aktual = o_start;
				while (o_aktual != NULL && (!o_aktual->isActive() || !o_aktual->inside(waktual, msev->x, msev->y))) {
					o_aktual = o_aktual->_next;
				}

				if (o_aktual == NULL) {
					return;
				}

				msg2.msg = E_GET_FOCUS;
				o_aktual->event(&msg2);
				o_aktual->gotFocus();
				o_aktual->event(msg);
			} else {
				return;
			}
		} else {
			if (o_aktual->isEnabled()) {
				b = o_aktual->inside(waktual, msev->x, msev->y);
			} else {
				b = 0;
			}

			if (b) {
				o_aktual->event(msg);
			}

			if ((msev->tl1 || msev->tl2 || msev->tl3) && !b) {
				o_aktual->lostFocus();

				if (f_cancel_event) {
					return;
				}

				msg2.msg = E_LOST_FOCUS;
				o_aktual->event(&msg2);
				p = o_start;

				while (p != NULL && (!p->isActive() || !p->inside(waktual, msev->x, msev->y))) {
					p = p->_next;
				}

				if (p != NULL) {
					o_aktual = p;
				}

				msg2.msg = E_GET_FOCUS;
				o_aktual->event(&msg2);
				o_aktual->gotFocus();

				if (p != NULL) {
					o_aktual->event(msg);
				}
			}
		}
	}

	if (msg->msg == E_KEYBOARD) {
		int c;

		va_copy(args, msg->data);
		c = va_arg(args, int);
		va_end(args);

		*oz = 1;

		if (o_aktual != NULL) {
			o_aktual->event(msg);
		}

		if (c >> 8 == 0xf && waktual->idlist != NULL) {
			if (o_aktual == NULL) {
				o_aktual = get_last_id();
			}

			if (o_aktual != NULL) {
				f_cancel_event = 0;
				o_aktual->lostFocus();

				if (f_cancel_event) {
					return;
				}

				msg2.msg = E_LOST_FOCUS;
				o_aktual->event(&msg2);
			}

			if(c & 0xff == 9) {
				o_aktual = get_next_id(o_aktual);
			} else {
				o_aktual = get_prev_id(o_aktual);
			}

			if (o_aktual != NULL) {
				msg2.msg = E_GET_FOCUS;
				o_aktual->event(&msg2);
				o_aktual->gotFocus();
			}
		}
	}

	if (msg->msg == E_TIMER && o_aktual != NULL) {
		o_aktual->event(msg);

		if (!(cursor_tick--)) {
			msg->msg = E_CURSOR_TICK;
			o_aktual->event(msg);
			o_aktual->onEvent(msg);
			cursor_tick = CURSOR_SPEED;
		}
	}
/* E_GUI message never gets sent
  if (msg->msg==E_GUI)
     {
     OBJREC *o;
     EVENT_MSG msg2;
     int *p;

     p=msg->data;
     o=find_object(waktual,*p++);
     if (o!=NULL)
        {
        msg2.msg=*p++;
        msg2.data=p;
        o->runs[2](&msg2,o);
        o->events[0](&msg,o_aktual);
        }

     }
*/
	if (msg->msg == E_CHANGE) {
		run_background(o_aktual->getOnActivate());
	}

	if (change_flag) {
		send_message(E_CHANGE);
	}
}




void install_gui(void) {
	desktop_y_size = renderer->width();
	send_message(E_ADD, E_MOUSE, do_it_events);
	send_message(E_ADD, E_KEYBOARD, do_it_events);
	send_message(E_ADD, E_CHANGE, do_it_events);
	send_message(E_ADD, E_TIMER, do_it_events);
	send_message(E_ADD, E_GUI, do_it_events);
	send_message(E_ADD, E_IDLE, redraw_desktop_call);
}

void uninstall_gui(void)
  {
  send_message(E_DONE,E_MOUSE,do_it_events);
  send_message(E_DONE,E_KEYBOARD,do_it_events);
  send_message(E_DONE,E_CHANGE,do_it_events);
  send_message(E_DONE,E_TIMER,do_it_events);
  send_message(E_DONE,E_GUI,do_it_events);
  send_message(E_DONE,E_IDLE,redraw_desktop_call);
  }

//send_message(E_GUI,cislo,E_UDALOST,data....)


void on_change(void (*proc)()) {
	o_end->setOnActivate(proc);
}

void on_enter(void (*proc)()) {
	o_end->setGotFocus(proc);
}

void on_leave(void (*proc)()) {
	o_end->setLostFocus(proc);
}

void on_event(void (*proc)(EVENT_MSG *msg)) {
	o_end->setOnEvent(proc);
}

void terminate(void)
  {
  exit_wait=1;
  }
void set_change(void)
{
 change_flag=1;
}

void set_window_modal(void)
  {
  waktual->modal=1;
  }

GUIObject *find_object(WINDOW *w,int id)
  {
  GUIObject *p;

  p=w->objects;
  if (p==NULL) return NULL;
  while (p!=NULL && p->id() != id) p=p->_next;
  return p;
  }

GUIObject *find_object_desktop(int win_id,int obj_id,WINDOW **wi)
{
  WINDOW *w;
  GUIObject *o;

  if (win_id<0) return NULL;
  if (win_id==0) w=waktual;
  else
     w=find_window(win_id);
  if (w==NULL) return NULL;
  o=find_object(w,obj_id);
  *wi=w;
  return o;
}

void goto_control(int obj_id)
  {
  EVENT_MSG msg;
  if (send_lost()) return;
  o_aktual=find_object(waktual,obj_id);
  msg.msg=E_GET_FOCUS;
  o_aktual->onEvent(&msg);
  o_aktual->event(&msg);
  }

void cancel_event()
  {
  f_cancel_event=1;
  }

void set_enable(int win_id,int obj_id,int condition)
{
 GUIObject *o;
 WINDOW *w;

 bool cond = (condition!=0);
 if ((o=find_object_desktop(win_id,obj_id,&w))==NULL) return;
 if (o==o_aktual)
     if (send_lost()) return;
     else
      o_aktual=NULL;
 if (o->isEnabled() != cond)
  {
  o->setEnabled(cond);
  if (w==waktual) redraw_object(o);
  }
}

void close_current()
  {
  close_window(waktual);
  }

void background_runner(EVENT_MSG *msg,void **prog) {
	void (*p)();
	char i = 1;

	if (msg->msg == E_INIT) {
		va_list args;
		va_copy(args, msg->data);
		*prog = (void*)va_arg(args, void (*)());
		va_end(args);
		return;
	} else if (msg->msg == E_DONE) {
		*prog = NULL;
		return;
	}

	p = (void (*)())*prog;
	p();
	i = 0;
	msg->msg = -2;
}

void run_background(void (*p)())
  {
  send_message(E_ADD,E_IDLE,background_runner,p);
  }

void movesize_win(WINDOW *w, int newx,int newy, int newxs, int newys) {
	int xsdif, ysdif;
	GUIObject *p;

	if (newxs < w->minsizx) {
		newxs = w->minsizx;
	}

	if (newys < w->minsizy) {
		newys = w->minsizy;
	}

	if (newxs >= (renderer->width() - 2 * w->border3d.bsize)) {
		newxs = (renderer->width() - 2 * w->border3d.bsize) - 1;
	}

	if (newys + 2 >= (desktop_y_size - 2 * w->border3d.bsize)) {
		newys = (desktop_y_size - 2 * w->border3d.bsize) - 2;
	}

	xsdif = newxs - w->xs;
	ysdif = newys - w->ys;
	p = w->objects;

	while (p != NULL) {
		p->autoResize(xsdif, ysdif);
		p = p->_next;
	}

	w->x = newx;
	w->y = newy;
	w->xs = newxs;
	w->ys = newys;
	check_window(w);
}

