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

void highlight(CTL3D *c,uint16_t color)
  {
  c->light=color<<1;
  if (c->light & 0x0020) {c->light&=~RGB555(0,0,31);c->light|=RGB555(0,0,31);}
  if (c->light & 0x0400) {c->light&=~RGB555(0,31,0);c->light|=RGB555(0,31,0);}
  if (c->light & 0x8000) {c->light&=~RGB555(31,0,0);c->light|=RGB555(31,0,0);}
  c->shadow=color;
  c->shadow&=RGB555(30,30,31);
  c->shadow>>=1;
  }


CTL3D *def_border(int btype,int color)
  {
  static CTL3D ctl;

  highlight(&ctl,color);
  switch (btype)
     {
     case 0:ctl.bsize=0;
     case 1:ctl.light=color;ctl.shadow=color;ctl.bsize=1;break;
     case 2:ctl.bsize=2;ctl.ctldef=0;break;
     case 3:ctl.bsize=2;ctl.ctldef=3;break;
     case 4:ctl.bsize=1;ctl.ctldef=0;break;
     case 5:ctl.bsize=1;ctl.ctldef=1;break;
     case 6:ctl.bsize=2;ctl.ctldef=1;break;
     case 7:ctl.bsize=2;ctl.ctldef=2;break;
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
	highlight(&ctl, _color);
	ctl.bsize = 2 - _data;
	ctl.ctldef = 3 * _data;
	draw_border(x + 2, y + 2, width - 4, height - 4, &ctl);
	set_aligned_position(x + width / 2 + _data * 2, y + height / 2 + _data * 2, 1, 1, _text);
	outtext(_text);
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

void draw_status_line(char *c)
  {
  static uint16_t *font;
  static FC_TABLE color;
  static uint16_t backgr;
  static uint16_t ysmax=0,y;
  uint16_t ysize;
  CTL3D ctl;

  if (c==NULL)
     {
     backgr=curcolor;
     memcpy(&color,&charcolors,sizeof(charcolors));
     font=curfont;
     return;
     }

  schovej_mysku();
  curfont=font;
  ysize=text_height(c);
  if (ysmax>ysize) ysize=ysmax;else
     ysmax=ysize;

  highlight(&ctl,backgr);
  ctl.bsize=2;
  ctl.ctldef=0;
  curcolor=backgr;
  memcpy(&charcolors,&color,sizeof(charcolors));
  y=Screen_GetYSize()-ysize-3;
  desktop_y_size=y-3;
  bar(0,y,Screen_GetXSize()-1,Screen_GetYSize()-1);
  draw_border(2,y,Screen_GetXSize()-5,ysize,&ctl);
  while (text_width(c)>Screen_GetXSize())
     {
     char *p;

     p=strchr(c,'\0');
     *(--p)='\0';
     if (p=c) break;
     }
  position(5,y);outtext(c);
  ukaz_mysku();
  showview(0,y-2,Screen_GetXSize()-1,ysize+5);
  }

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

void xor_rectangle(int x,int y,int xs,int ys)
  {
  curcolor=RGB555(31,31,31);
  if (x<0) x=0;
  if (y<0) y=0;
  if (x+xs>=Screen_GetXSize()) xs=Screen_GetXSize()-x-1;
  if (y+ys>=Screen_GetYSize()) ys=Screen_GetYSize()-y-1;
  schovej_mysku();
  hor_line_xor(x,y,x+xs);
  ver_line_xor(x,y,y+ys);
  if (xs && ys)
     {
  hor_line_xor(x,y+ys,x+xs);
  ver_line_xor(x+xs,y,y+ys);
     }
  ukaz_mysku();
  showview(x,y,x+xs,4);
  showview(x,y,4,ys+4);
  if (xs && ys)
     {
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
	set_aligned_position(x + 5, y + height / 2, 0, 1, _text);
	outtext(_text);
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
	int tw;
	int len;
	int shift;

	bar(x, y, x + width, y + height);
	position(x, y);

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
	tw = x + text_width(d);

	while (tw < x + width) {
		outtext(d);

		if (!*str) {
			break;
		}

		d[0] = *str++;
		tw = writeposx + text_width(d);
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
					if ((d = charsize(curfont, _str[i]) & 0xff) == 0) {
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

			xor_rectangle(_locx + xpos, _locy, 1, _y + _height);
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
					xpos += charsize(curfont, _str[cursor]) & 0xff;
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
	position(x, y);
	outtext(_text);
}

//-------------------------------------------------------------

#define MSG_SIZE (Screen_GetXSize()*3/4)
#define MSG_L_MARGIN 10
#define MSG_R_MARGIN 50
#define MSG_A_MARGIN (MSG_L_MARGIN+MSG_R_MARGIN)
#define MSG_COLOR RGB555(15,0,0)
#define MSG_F_COLOR RGB555(31,31,0)

uint16_t *msg_box_font;
uint16_t *msg_icn_font;


int msg_box(char *title, char icone, char *text, ... )
  {
  int winx,winy,xp,yp,txt_h;
  int txt_max,temp1,temp2,i;
  char buf[256];
  char *p;
  CTL3D *ctl;
  char **c;
  FC_TABLE cl;

  curfont=msg_box_font;
  ctl=def_border(2,MSG_COLOR);
  winx=text_width(text)+MSG_A_MARGIN;
  winy=30;
  desktop_add_window(create_window(0,0,winx,300,MSG_COLOR,ctl));
  buf[1]='\0';buf[0]=icone;
  xp=text_width(buf);
  define(new Label(-1, (MSG_R_MARGIN >> 1) - (xp >> 1), 20, xp, text_height(buf), 1, buf));
  cl[1]=ctl->light;
  cl[0]=ctl->shadow;
  property(NULL,msg_icn_font,&cl,MSG_COLOR);
  curfont=msg_box_font;
  default_font=curfont;
  if (winx>MSG_SIZE) winx=MSG_SIZE;
  c=&text;c++;
  temp1=0;temp2=0;
   while (*c)
     {temp1+=text_width(*c++)+10;temp2++;}
  if (temp1>winx-2*MSG_L_MARGIN) winx=temp1+2*MSG_L_MARGIN;
  txt_max=winx-MSG_A_MARGIN;
  txt_h=0;
  while (*text)
     {
     memset(buf,0,sizeof(buf));
     p=buf;
     while (text_width(buf)<txt_max && *text && *text!='\n') *p++=*text++;
     if (text_width(buf)>txt_max) while (*(--p)!=' ') text--;
     if (*text=='\n') text++;
     *p='\0';
     txt_h=text_height(buf);
     define(new Label(-1, MSG_L_MARGIN, winy, txt_max, txt_h, 0, buf));
     property(NULL,NULL,flat_color(MSG_F_COLOR),MSG_COLOR);
     o_end->setFColor(0, 0);
     winy+=txt_h;
     }
  winy+=40;
  xp=(Screen_GetXSize()>>1)-(winx>>1);
  yp=(Screen_GetYSize()>>1)-(winy>>1);
  waktual->x=xp;waktual->y=yp;waktual->xs=winx;waktual->ys=winy;
  define(new WindowLabel(0, 1, 1, winx - 2, text_height(title) + 2, 0, title));
  ctl=def_border(5,MSG_COLOR);
  property(ctl,NULL,flat_color(MSG_F_COLOR),0x10);
  ctl=def_border(1,0);
  c=&text;c++;
  for (i=1;i<=temp2;i++)
     {
     int sz;

     sz=(winx/(temp2+1))>>1;
     if (sz<text_width(*c)) sz=text_width(*c);
   define(new Button(i, i * winx / (temp2 + 1) - (sz >> 1), 10, sz + 5, 20, 3, *c));
   property(ctl,NULL,flat_color(0),RGB555(24,24,24));on_change(terminate);
     c++;
     }
  set_window_modal();
  redraw_window();
  escape();
  temp2 = o_aktual->id();
  close_window(waktual);
  return temp2;
  }

