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
#include "libs/pcx.h"
#include "libs/devices.h"
#include "libs/bgraph.h"

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
#define DESK_TOP_COLOR 0,123,123
#define MINSIZX 60
#define MINSIZY 40


//#define SCR_WIDTH_X DxGetResX()
//#define SCR_WIDTH_Y DxGetResY()


typedef struct ctl3d {
	uint8_t light[3], shadow[3];
	uint16_t bsize, ctldef;
} CTL3D;

typedef uint8_t FC_TABLE[7][3];
typedef uint8_t FC_PTR[3];
typedef FC_TABLE FC_PALETTE[16];

// align urcuje vzhledem ke ktermu rohu je vypocet souradnic vztazen
/* align = 0  levy horni roh
   align = 1  pravy horni roh
   align = 2  pravy dolni roh
   align = 3  levy dolni roh

   autoresize=1 znamena, ze object zmeni svou velikost tak aby jeho
   pravy dolni roh sledoval pravy dolni roh okna a levy horni roh sledoval
   levy horni roh okna
*/

struct window;

class GUIObject {
private:
	int _id;
	static const Font *_dFont;
	static int _dShadow;
	static uint8_t _dColor[FONT_COLORS][3];

protected:
	int _x, _y, _width, _height, _locx, _locy, _align;
	bool _autoResizeX, _autoResizeY, _enabled, _drawError;
	const Font *_font;
	CTL3D _border;
	uint8_t _fColor[FONT_COLORS][3], _color[3];
	int _shadow;

	void (*_onEvent)(EVENT_MSG *msg);
	void (*_gotFocus)();
	void (*_lostFocus)();
	void (*_onActivate)();

	// Do not implement
	GUIObject(const GUIObject &src);
	const GUIObject &operator=(const GUIObject &src);

public:
	GUIObject *_next;

	static void setDefaultFont(const Font *font, int shadow, uint8_t r, uint8_t g, uint8_t b);

	GUIObject(int id, int x, int y, int width, int height, int align);
	virtual ~GUIObject(void) = 0;

	virtual void draw(int x, int y, int width, int height) = 0;
	virtual void event(EVENT_MSG *msg);

	void onEvent(EVENT_MSG *msg);
	void gotFocus();
	void lostFocus();
	void onActivate(EVENT_MSG *msg);

	void setOnEvent(void (*proc)(EVENT_MSG *msg));
	void setGotFocus(void (*proc)(void));
	void setLostFocus(void (*proc)(void));
	void setOnActivate(void (*proc)(void));
	void (*getOnActivate(void))(void);

	void render(window *w, int show);
	void setFColor(unsigned idx, uint8_t r, uint8_t g, uint8_t b);
	void align(window *w, int &x, int &y) const;
	void property(CTL3D *ctl, const Font *font, int shadow, uint8_t fcolor[][3], uint8_t *color);
	bool inside(window *w, int x, int y) const;
	void autoResize(int xdiff, int ydiff);

	inline int id(void) const { return _id; }
	// always return false if you have empty draw()
	virtual bool isActive(void) const { return _enabled; }
	bool isEnabled(void) const { return _enabled; }
	void setEnabled(bool value);
};

typedef struct tidlist
  {
  struct tidlist *next;
	GUIObject *obj;
  }TIDLIST;

// FIXME: rewrite to proper class
typedef struct window {
	int16_t x, y, xs, ys;
	CTL3D border3d;
	uint8_t color[3];
	GUIObject *objects;
	uint32_t id;
	int8_t modal, minimized, popup;
	uint16_t minsizx, minsizy;
	int8_t *window_name;
	int txtid, txtx, txty;
	void (*draw_event)(struct window *);
	struct window *next;
	TIDLIST *idlist;
} WINDOW;

extern WINDOW *desktop,*waktual;
extern GUIObject *o_aktual, *o_end, *o_start;
extern CTL3D noneborder;
extern FC_TABLE f_default;
extern uint16_t desktop_y_size;
//extern char change_flag;
extern uint16_t *default_font;
extern const Texture *gui_background;



void draw_border(int16_t x,int16_t y,int16_t xs,int16_t ys,CTL3D *btype);
WINDOW *create_window(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b, CTL3D *okraj);
long desktop_add_window(WINDOW *w);
void select_window(long id);
WINDOW *find_window(long id);
void redraw_object(GUIObject *o);
void redraw_window();
void define(GUIObject *o);
CTL3D *border(uint16_t light,uint16_t shadow, uint16_t bsize, uint16_t btype);
void property(CTL3D *ctl, const Font *font, int shadow, uint8_t fcolor[][3], uint8_t *color);
FC_PTR *flat_color(uint8_t r, uint8_t g, uint8_t b);
uint8_t *single_color(uint8_t r, uint8_t g, uint8_t b);
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
void on_event(void (*proc)(EVENT_MSG *msg));
void terminate(void);
void set_change(void);
void cancel_event();
GUIObject *find_object(WINDOW *w,int id);
GUIObject *find_object_desktop(int wid, int id, WINDOW **wi);
void set_window_modal(void);
void set_enable(int win_id,int obj_id,int condition);
void run_background(void (*p)());
void disable_bar(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b);
void movesize_win(WINDOW *w, int newx,int newy, int newxs, int newys);
void goto_control(int obj_id);

#endif
