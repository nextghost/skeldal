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

#ifndef __LIBS_GUI_H
#define __LIBS_GUI_H

#include <inttypes.h>
#include "libs/devices.h"

#pragma pack(1)

#define E_MS_CLICK 50
#define E_MS_MOVE 51
#define E_GET_FOCUS 52
#define E_LOST_FOCUS 53
#define E_KEY_PRESS 54
#define E_CHANGE 55
#define E_CURSOR_TICK 56
#define E_REDRAW 57 //redraw desktop
#define E_GUI 58    //direct enter to gui system
#define E_CONTROL 59 //User defined feature, enables direct controling desktop objects

#define CURSOR_SPEED 5
#define get_title(title) (char *)*(long *)(title)
#define DESK_TOP_COLOR RGB555(0,15,15)
#define MINSIZX 60
#define MINSIZY 40


//#define SCR_WIDTH_X DxGetResX()
//#define SCR_WIDTH_Y DxGetResY()


typedef struct ctl3d
  {
  uint16_t light,shadow,bsize,ctldef;
  }CTL3D;

typedef uint16_t FC_TABLE[7];
typedef FC_TABLE FC_PALETTE[16];

//Run_routs:
/*   0 - INIT
     1 - DRAW
     2 - EVENT
     3 - DONE

     0 - WHEN EVENT
     1 - AFTER GOT FOCUS
     2 - BEFORE LOST FOCUS
     3 - WHEN VALUE CHANGED / WHEN AKTIVATE BUTTON
*/

//objects
/*   prototypy jednotlivych funkci
     INIT(OBJREC *object,VOID *initparms);
     DRAW(int x1,int y1,int x2,int y2,OBJREC *object);
     EVENT(EVENT_MSG *msg, OBJREC *object);
     DONE(OBJREC *object);
*/

typedef void (*RUN_ROUTS[4])();

typedef struct objrec
  {
  int16_t x,y,xs,ys;
  CTL3D border3d;
  uint16_t color;
  uint16_t id;
  int8_t align,autoresizex,autoresizey;
  int8_t enabled;
  int16_t locx,locy;
  uint32_t datasize;
  void *data;
  FC_TABLE f_color;
  uint16_t *font;
  void *userptr;
  RUN_ROUTS runs;
  RUN_ROUTS events;
  int8_t draw_error;       //1 znamena ze objekt zpusobil chybu a nebude vykreslovan
  struct objrec *next;
  }OBJREC;

// align urcuje vzhledem ke ktermu rohu je vypocet souradnic vztazen
/* align = 0  levy horni roh
   align = 1  pravy horni roh
   align = 2  pravy dolni roh
   align = 3  levy dolni roh

   autoresize=1 znamena, ze object zmeni svou velikost tak aby jeho
   pravy dolni roh sledoval pravy dolni roh okna a levy horni roh sledoval
   levy horni roh okna
*/

typedef struct tidlist
  {
  struct tidlist *next;
  OBJREC *obj;
  }TIDLIST;


typedef struct window
  {
  int16_t x,y,xs,ys;
  CTL3D border3d;
  uint16_t color;
  OBJREC *objects;
  uint32_t id;
  int8_t modal,minimized,popup;
  uint16_t minsizx,minsizy;
  int8_t *window_name;
  void (*draw_event)(struct window *);
  struct window *next;
  TIDLIST *idlist;
  }WINDOW;

extern WINDOW *desktop,*waktual;
extern OBJREC *o_aktual,*o_end,*o_start;
extern CTL3D noneborder;
extern FC_TABLE f_default;
extern uint16_t desktop_y_size;
//extern char change_flag;
extern uint16_t *default_font;
extern void *gui_background;




void draw_border(int16_t x,int16_t y,int16_t xs,int16_t ys,CTL3D *btype);
WINDOW *create_window(int x,int y, int xs, int ys, uint16_t color, CTL3D *okraj);
long desktop_add_window(WINDOW *w);
void select_window(long id);
WINDOW *find_window(long id);
void redraw_object(OBJREC *o);
void redraw_window();
void define(int id,int x,int y,int xs,int ys,char align,void (*initproc)(OBJREC *),...);
CTL3D *border(uint16_t light,uint16_t shadow, uint16_t bsize, uint16_t btype);
void property(CTL3D *ctl,uint16_t *font,FC_TABLE *fcolor,uint16_t color);
FC_TABLE *flat_color(uint16_t color);
void aktivate_window(MS_EVENT *ms);
void redraw_desktop();
void close_window(WINDOW *w);
void close_current();
void check_window(WINDOW *w);
void install_gui(void);
void uninstall_gui(void);
void on_change(void (*proc)());
void on_enter(void (*proc)());
void on_leave(void (*proc)());
void on_event(void (*proc)());
void terminate(void);
void set_change(void);
void set_value(int win_id,int obj_id,void *value);
void set_default(void *value);
void c_set_value(int win_id,int obj_id,int cnst);
void c_default(int cnst);
int f_get_value(int win_id,int obj_id);
void get_value(int win_id,int obj_id,void *buff);
void cancel_event();
OBJREC *find_object(WINDOW *w,int id);
void set_window_modal(void);
void set_enable(int win_id,int obj_id,int condition);
void run_background(void (*p)());
void disable_bar(int x,int y,int xs,int ys,uint16_t color);
void movesize_win(WINDOW *w, int newx,int newy, int newxs, int newys);
void goto_control(int obj_id);

#pragma option align=reset

#endif
