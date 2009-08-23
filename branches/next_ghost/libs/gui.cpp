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
#include "libs/memman.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/gui.h"

#define E_REDRAW_DESKTOP 2010
#define E_REDRAW_WINDOW 2000

uint16_t *gui_background=NULL;

extern T_EVENT_ROOT *ev_tree;

WINDOW *desktop={NULL},*waktual={NULL};;
OBJREC *o_aktual={NULL},*o_end={NULL},*o_start={NULL};
CTL3D noneborder={0,0,0,0};
FC_TABLE f_default;
uint16_t desktop_y_size;
char force_redraw_desktop=0;
static char change_flag=0,f_cancel_event=0;
uint16_t *default_font;
void empty()
  {
  }


void empty1(OBJREC *o)
  {
  o;
  }

void empty3(EVENT_MSG *ms,OBJREC *o)
  {
  o;ms;
  }


void empty2(int x1,int y1,int x2,int y2,OBJREC *o)
  {
  o;x1;y1;x2;y2;
  }



void draw_border(int16_t x,int16_t y,int16_t xs,int16_t ys,CTL3D *btype)
  {
  uint16_t i,j,c;

  c=curcolor;
  j=btype->ctldef;
  for (i=0;i<btype->bsize;i++)
     {
     x--;y--;xs+=2;ys+=2;
     if (j & 1) curcolor=btype->shadow; else curcolor=btype->light;
     hor_line(x,y,xs+x);
     ver_line(x,y,ys+y);
     if (j & 1) curcolor=btype->light; else curcolor=btype->shadow;
     hor_line(x,y+ys,xs+x);
     ver_line(x+xs,y,ys+y);
     j>>=1;
     }
  curcolor=c;
  }
void check_window(WINDOW *w)
  {
  if (w->x<=w->border3d.bsize) w->x=w->border3d.bsize;
  if (w->y<=w->border3d.bsize) w->y=w->border3d.bsize;
  if (w->x>=Screen_GetXSize()-w->border3d.bsize-w->xs) w->x=Screen_GetXSize()-w->border3d.bsize-w->xs-1;
  if (w->y>=desktop_y_size-w->border3d.bsize-w->ys) w->y=desktop_y_size-w->border3d.bsize-w->ys-1;
  }

void show_window(WINDOW *w)
  {
  int ok;

  ok=w->border3d.bsize;
  showview(w->x-ok,w->y-ok,w->xs+(ok<<1),w->ys+(ok<<1));
  }

void draw_cl_window(WINDOW *o)
  {
  curcolor=o->color;
  bar32(o->x,o->y,o->x+o->xs,o->y+o->ys);
  draw_border(o->x,o->y,o->xs,o->ys,&o->border3d);
  }



WINDOW *create_window(int x,int y, int xs, int ys, uint16_t color, CTL3D *okraj)
  {
  WINDOW *p;

  p=(WINDOW *)getmem(sizeof(WINDOW));
  p->x=x;p->y=y;p->xs=xs;p->ys=ys;
  p->color=color;memcpy(&(p->border3d),okraj,sizeof(CTL3D));
  p->objects=NULL;
  p->modal=0;
  p->popup=0;
  p->minimized=0;
  p->window_name=NULL;
  p->minsizx=MINSIZX;
  p->minsizy=MINSIZY;
  p->idlist=NULL;
  p->draw_event=draw_cl_window;
  check_window(p);
   return p;
  }

int send_lost()
  {
  EVENT_MSG msg;
   msg.msg=E_LOST_FOCUS;
  if (o_aktual!=NULL)
     {
     o_aktual->events[2]();
     if (f_cancel_event) return -1;
     ((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg,o_aktual);
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
     o_start=waktual->objects;
     o_aktual=NULL;
     o_end=o_start;
     while(o_end->next!=NULL) o_end=o_end->next;
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
        o_aktual->events[2]();
        ((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg,o_aktual);
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



void absolute_window(WINDOW *w,OBJREC *o, int *x, int *y)
  {
    switch (o->align)
     {
     case 0:
     case 1:*y=o->y+w->y;break;
     case 2:
     case 3:*y=(w->y+w->ys)-(o->y+o->ys);break;
     }
  switch (o->align)
     {
     case 0:
     case 3:*x=o->x+w->x;break;
     case 1:
     case 2:*x=(w->x+w->xs)-(o->x+o->xs);break;
     }
  }

  void disable_bar(int x,int y,int xs,int ys,uint16_t color)
     {
     int i,j;
     uint16_t *a;

     for (i=y;i<=y+ys;i++)
        {
        a=Screen_GetAddr()+Screen_GetXSize()*i+x;
        for(j=x;j<=x+xs;j++)
           {
           *a=((*a & RGB555(30,30,30))+(color & RGB555(30,30,30)))>>1;
           *a=((*a & RGB555(30,30,30))+(color & RGB555(30,30,30)))>>1;
           a++;
           }
        }
     }

  void draw_object(WINDOW *w,OBJREC *o,char show)
  {
  int x, y;
  int ok;
//  WINDOW *ws;

  if (o->draw_error==1) return;
  o->draw_error=1;
  ok=w->border3d.bsize;
  absolute_window(w,o,&x,&y);
  o->locx=x;o->locy=y;
  if (o->xs<1 || o->ys<1)
     {
     o->draw_error=0;
     return;
     }
  schovej_mysku();
  draw_border(x,y,o->xs,o->ys,&o->border3d);
  curcolor=o->color;
  curfont=o->font;position(x,y);
  memcpy(&charcolors,&o->f_color,sizeof(charcolors));
//  ws=waktual;
//  waktual=w;
  ((void (*)(int, int, int, int, OBJREC*))o->runs[1])(x, y, x+o->xs, y+o->ys, o);
//  waktual=ws;
  if (!o->enabled) disable_bar(x,y,o->xs,o->ys,o->color);
  ukaz_mysku();
  if (show) showview(x-ok,y-ok,o->xs+(ok<<1),o->ys+(ok<<1));
  o->draw_error=0;
  }


void redraw_object(OBJREC *o)
  {
  draw_object(waktual,o,1);
  }


void redraw_window_call()
  {
  OBJREC *p;

  schovej_mysku();
  waktual->draw_event(waktual);
  p=waktual->objects;
  while (p!=NULL)
     {
     draw_object(waktual,p,0);
     p=p->next;
     }
  ukaz_mysku();
  show_window(waktual);
  return;
  }

void add_to_idlist(OBJREC *o)
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
  if (((waktual->idlist)->obj)->id>o->id)
     {
     p->next=waktual->idlist;
     waktual->idlist=p;
     return;
     }
  q=waktual->idlist;
  while (q->next!=NULL && ((q->next)->obj)->id<o->id) q=q->next;
  p->next=q->next;
  q->next=p;
  }


void define(int id,int x,int y,int xs,int ys,char align,void (*initproc)(OBJREC *),...)
  {
  OBJREC *o;
  long *p;

  o=(OBJREC *)getmem(sizeof(OBJREC));
  o->x=x;o->y=y;o->xs=xs;o->ys=ys;
  o->id=id;
  o->runs[0] = (void (*)())empty1;
  o->runs[1] = (void (*)())empty2;
  o->runs[2] = (void (*)())empty3;
  o->runs[3] = (void (*)())empty1;
  o->autoresizex=0;
  o->autoresizey=0;
  o->events[0]=empty;
  o->events[1]=empty;
  o->events[2] = (void (*)())empty1;
  o->events[3] = (void (*)())empty3;
  o->enabled=1;
  o->draw_error=0;
  o->color=waktual->color;memcpy(o->f_color,f_default,sizeof(f_default));
  memcpy(&(o->border3d),&noneborder,sizeof(CTL3D));
  o->userptr=NULL;
  o->align=align;
  o->font=default_font;
  o->datasize=0;
  initproc(o);
  if (o->datasize) o->data=(void *)getmem(o->datasize); else o->data=NULL;
  p=(long *)&initproc;p++;
  ((void (*)(OBJREC*, void*))o->runs[0])(o,p);
  if (o->datasize && o->data==NULL) o->data=(void *)getmem(o->datasize);
  o->next=NULL;
  if (o_start==NULL)
     {
     o_start=o;
     o_end=o;
     o_aktual=NULL;
     waktual->objects=o;
     }
  else
     {
     o_end->next=o;
     o_end=o;
     }
  add_to_idlist(o);
  }

CTL3D *border(uint16_t light,uint16_t shadow, uint16_t bsize, uint16_t btype)
  {
  static CTL3D p;

  p.light=light;p.shadow=shadow;p.bsize=bsize;p.ctldef=btype;
  return &p;
  }

void property(CTL3D *ctl,uint16_t *font,FC_TABLE *fcolor,uint16_t color)
  {
  if (ctl!=NULL) memcpy(&o_end->border3d,ctl,sizeof(CTL3D));
  if (font!=NULL) o_end->font=font;
  if (fcolor!=NULL) memcpy(&o_end->f_color,fcolor,sizeof(FC_TABLE));
  if (color!=0xffff) o_end->color=color;
  }

FC_TABLE *flat_color(uint16_t color)
  {
  static FC_TABLE p;

  p[0]=0xffff;
  p[1]=color;p[2]=color;p[3]=color;p[4]=color;p[5]=color;p[6]=color;
  return &p;
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


void redraw_desktop_call(EVENT_MSG *msg,void **data)
  {
  WINDOW *w;
  int8_t *oz;

  data;
  if (msg->msg==E_INIT || msg->msg==E_DONE) return;
  if (!force_redraw_desktop) return;
  force_redraw_desktop=0;
  schovej_mysku();
  if (gui_background==NULL)
     {
     curcolor=DESK_TOP_COLOR;
     bar(0,0,Screen_GetXSize(),Screen_GetYSize()-1);
     }
  else
     put_picture(0,0,gui_background);
  oz=otevri_zavoru;
  do_events();
  *oz=1;
  w=desktop;
  while (w!=NULL)
     {
     OBJREC *p;
           w->draw_event(w);
           p=w->objects;
           while (p!=NULL)
           {
              draw_object(w,p,0);
              p=p->next;
           }
     w=w->next;
     *oz=0;
     do_events();
     *oz=1;
     }
  send_message(E_REDRAW);
  ukaz_mysku();
  showview(0,0,0,0);
  move_ms_cursor(0,0,1);
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
  OBJREC *q;

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
     w->objects=q->next;
     ((void (*)(OBJREC*))q->runs[3])(q);
     if (q->userptr!=NULL) free(q->userptr);
     if (q->data!=NULL) free(q->data);
     free(q);
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

char mouse_in_object(MS_EVENT *ms,OBJREC *o, WINDOW *w)
  {
  int x1, y1, x2, y2;

  absolute_window(w,o,&x1,&y1);
  x2=x1+o->xs;
  y2=y1+o->ys;
  if (ms->x>=x1 && ms->x<=x2 && ms->y>=y1 && ms->y<=y2)
     return 1;
  else
     return 0;
  }

OBJREC *get_last_id()
  {
  TIDLIST *p;

  p=waktual->idlist;
  while (p->next!=NULL) p=p->next;
  return p->obj;
  }

OBJREC *get_next_id(OBJREC *o)
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
     if (o->enabled && o->runs[2] != (void (*)())empty3) return o;
     }
  while (1);
  }

OBJREC *get_prev_id(OBJREC *o)
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
     if (o->enabled && o->runs[2] != (void (*)())empty3) return o;
     }
  while (1);
  }


void do_it_events(EVENT_MSG *msg, void **user_data) {
	MS_EVENT *msev;
	EVENT_MSG msg2;
	char b;
	static uint16_t cursor_tick = CURSOR_SPEED;
	OBJREC *p;
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
		((void (*)(EVENT_MSG*, OBJREC*))o_aktual->events[0])(msg, o_aktual);
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
				while (o_aktual != NULL && (!o_aktual->enabled || !mouse_in_object(msev, o_aktual, waktual) || o_aktual->runs[2] == (void (*)())empty3)) {
					o_aktual = o_aktual->next;
				}

				if (o_aktual == NULL) {
					return;
				}

				msg2.msg = E_GET_FOCUS;
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg2, o_aktual);
				o_aktual->events[1]();
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);
			} else {
				return;
			}
		} else {
			if (o_aktual->enabled) {
				b = mouse_in_object(msev, o_aktual, waktual);
			} else {
				b = 0;
			}

			if (b) {
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);
			}

			if ((msev->tl1 || msev->tl2 || msev->tl3) && !b) {
				o_aktual->events[2]();

				if (f_cancel_event) {
					return;
				}

				msg2.msg = E_LOST_FOCUS;
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg2, o_aktual);
				p = o_start;

				while (p != NULL && (!p->enabled || !mouse_in_object(msev, p, waktual) || p->runs[2] == (void (*)())empty3)) {
					p = p->next;
				}

				if (p != NULL) {
					o_aktual = p;
				}

				msg2.msg = E_GET_FOCUS;
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg2, o_aktual);
				o_aktual->events[1]();

				if (p != NULL) {
					((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);
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
			((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);
		}

		if (c >> 8 == 0xf && waktual->idlist != NULL) {
			if (o_aktual == NULL) {
				o_aktual = get_last_id();
			}

			if (o_aktual != NULL) {
				f_cancel_event = 0;
				o_aktual->events[2]();

				if (f_cancel_event) {
					return;
				}

				msg2.msg = E_LOST_FOCUS;
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg2, o_aktual);
			}

			if(c & 0xff == 9) {
				o_aktual = get_next_id(o_aktual);
			} else {
				o_aktual = get_prev_id(o_aktual);
			}

			if (o_aktual != NULL) {
				msg2.msg = E_GET_FOCUS;
				((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg2, o_aktual);
				o_aktual->events[1]();
			}
		}
	}

	if (msg->msg == E_TIMER && o_aktual != NULL) {
		((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);

		if (!(cursor_tick--)) {
			msg->msg = E_CURSOR_TICK;
			((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(msg, o_aktual);
			((void (*)(EVENT_MSG*, OBJREC*))o_aktual->events[0])(msg, o_aktual);
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
		run_background(o_aktual->events[3]);
	}

	if (change_flag) {
		send_message(E_CHANGE);
	}
}




void install_gui(void)
  {
  desktop_y_size = Screen_GetYSize();
  send_message(E_ADD,E_MOUSE,do_it_events);
  send_message(E_ADD,E_KEYBOARD,do_it_events);
  send_message(E_ADD,E_CHANGE,do_it_events);
  send_message(E_ADD,E_TIMER,do_it_events);
  send_message(E_ADD,E_GUI,do_it_events);
  send_message(E_ADD,E_IDLE,redraw_desktop_call);
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


void on_change(void (*proc)())
  {
  o_end->events[3]=proc;
  }

void on_enter(void (*proc)())
  {
  o_end->events[1]=proc;
  }
void on_leave(void (*proc)())
{
  o_end->events[2]=proc;
  }
void on_event(void (*proc)())
{
  o_end->events[0]=proc;
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

void set_object_value(char redraw,OBJREC *o,const void *value)
  {
  if (memcmp(o->data,value,o->datasize))
     {
     memcpy(o->data,value,o->datasize);
     if (redraw) redraw_object(o);
     }
  }

OBJREC *find_object(WINDOW *w,int id)
  {
  OBJREC *p;

  p=w->objects;
  if (p==NULL) return NULL;
  while (p!=NULL && p->id!=id) p=p->next;
  return p;
  }

OBJREC *find_object_desktop(int win_id,int obj_id,WINDOW **wi)
{
  WINDOW *w;
  OBJREC *o;

  if (win_id<0) return NULL;
  if (win_id==0) w=waktual;
  else
     w=find_window(win_id);
  if (w==NULL) return NULL;
  o=find_object(w,obj_id);
  *wi=w;
  return o;
}
void set_value(int win_id,int obj_id,void *value)
  {
  OBJREC *o;
  WINDOW *w;

  if ((o=find_object_desktop(win_id,obj_id,&w))==NULL)return;
  set_object_value((w==waktual),o,value);
  }

void set_default(const void *value)
  {
  set_object_value(0,o_end,value);
  }

void goto_control(int obj_id)
  {
  EVENT_MSG msg;
  if (send_lost()) return;
  o_aktual=find_object(waktual,obj_id);
  msg.msg=E_GET_FOCUS;
  ((void (*)(EVENT_MSG*, OBJREC*))o_aktual->events[0])(&msg,o_aktual);
  ((void (*)(EVENT_MSG*, OBJREC*))o_aktual->runs[2])(&msg,o_aktual);
  }

void c_set_value(int win_id,int obj_id,int cnst)
  {
  OBJREC *o;
  WINDOW *w;

  if ((o=find_object_desktop(win_id,obj_id,&w))==NULL)return;
  set_object_value((w==waktual),o,&cnst);
  }

void c_default(int cnst)
  {
  set_object_value(0,o_end,&cnst);
  }



int f_get_value(int win_id,int obj_id)
  {
  OBJREC *o;
  WINDOW *w;
  int v;

  if ((o=find_object_desktop(win_id,obj_id,&w))==NULL) return 0;
  memcpy(&v,o->data,o->datasize);
  return v;
  }

void get_value(int win_id,int obj_id,void *buff)
  {
  OBJREC *o;
  WINDOW *w;

  if ((o=find_object_desktop(win_id,obj_id,&w))==NULL)return;
  memcpy(buff,o->data,o->datasize);
  }

void cancel_event()
  {
  f_cancel_event=1;
  }

void set_enable(int win_id,int obj_id,int condition)
{
 OBJREC *o;
 WINDOW *w;

 condition=(condition!=0);
 if ((o=find_object_desktop(win_id,obj_id,&w))==NULL) return;
 if (o==o_aktual)
     if (send_lost()) return;
     else
      o_aktual=NULL;
 if (o->enabled!=condition)
  {
  o->enabled=condition;
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

void movesize_win(WINDOW *w, int newx,int newy, int newxs, int newys)
  {
  int xsdif,ysdif;
  OBJREC *p;


  if (newxs<w->minsizx) newxs=w->minsizx;
  if (newys<w->minsizy) newys=w->minsizy;
  if (newxs>=(Screen_GetXSize()-2*w->border3d.bsize)) newxs=(Screen_GetXSize()-2*w->border3d.bsize)-1;
  if (newys+2>=(desktop_y_size-2*w->border3d.bsize)) newys=(desktop_y_size-2*w->border3d.bsize)-2;
  xsdif=newxs-w->xs;
  ysdif=newys-w->ys;
  p=w->objects;
  while (p!=NULL)
     {
     if (p->autoresizex) p->xs+=xsdif;
     if (p->autoresizey) p->ys+=ysdif;
     p=p->next;
     }
  w->x=newx;
  w->y=newy;
  w->xs=newxs;
  w->ys=newys;
  check_window(w);
  }

