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
#include <inttypes.h>
#include "libs/system.h"
#define line line32
#define hor_line hor_line32
#define ver_line ver_line32
#define bar bar32
#define point point32

#define RGB888(r, g, b) Screen_RGB((r) >> 3, (g) >> 3, (b) >> 3)
#define RGB555(r, g, b) Screen_RGB((r), (g), (b))

extern uint16_t curcolor,charcolors[7];
extern uint16_t *curfont,*writepos,writeposx;
extern uint8_t fontdsize;
extern uint8_t *palmem,*xlatmem;
extern void (*showview)(uint16_t,uint16_t,uint16_t,uint16_t);
extern char line480;
extern long screen_buffer_size;
extern char banking;
extern char __skip_change_line_test;
extern char no_restore_mode;

static __inline uint16_t *getadr32(int32_t x,int32_t y)
  {
  return Screen_GetAddr()+Screen_GetXSize()*y+x;
  }

static __inline void point32(int32_t x,int32_t y, uint16_t color)
  {
  *getadr32(x,y)=color;
  }
void bar32(int x1,int y1, int x2, int y2);
//#pragma aux bar32 parm [eAX] [eBX] [eCX] [eDX] modify [ESI EDI];
void hor_line32(int x1,int y1,int x2);
//#pragma aux hor_line32 parm [eSi] [eAX] [eCX] modify [eDI eDX];
void ver_line32(int x1,int y1,int y2);
//#pragma aux ver_line32 parm [eSi] [eAX] [eCX] modify [eDX];
void hor_line_xor(int x1,int y1,int x2);
//#pragma aux hor_line_xor parm [eSi] [eAX] [eCX] modify [eDI eDX];
void ver_line_xor(int x1,int y1,int y2);
//#pragma aux ver_line_xor parm [eSi] [eAX] [eCX] modify [eDX];
void line_32(int x,int y,int xs,int ys);
//#pragma aux line_32 parm [esi] [eax] [ecx] [ebx] modify [edx edi]
void char_32(uint16_t *posit,uint16_t *font,uint8_t znak);
//#pragma aux char_32 parm [edi] [esi] [eax] modify [eax ebx ecx edx]
void char2_32(uint16_t *posit,uint16_t *font,uint8_t znak);
//#pragma aux char2_32 parm [edi] [esi] [eax] modify [eax ebx ecx edx]
uint16_t charsize(uint16_t *font,uint8_t znak);
//#pragma aux charsize parm [esi] [eax]
void put_picture(uint16_t x,uint16_t y,uint16_t *p);
//#pragma aux put_picture parm [esi] [eax] [edi] modify [ebx ecx edx]
void get_picture(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys,uint16_t *p);
//#pragma aux get_picture parm [esi] [eax] [ebx] [ecx] [edi] modify [edx]
void setpal(void *paleta);
//#pragma aux setpal parm [esi] modify [eax edx]
void redraw_lo(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw_lo parm [esi][edi][ebx] modify[eax ecx edx]
void redraw256(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw256 parm [esi][edi][ebx] modify [eax ecx edx]
void redraw256b(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw256b parm [esi][edi][ebx] modify [eax ecx edx]
void redraw32(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw32 parm [esi][edi][ebx] modify [ecx]
void redraw32b(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw32b parm [esi][edi][ebx] modify [ecx eax]
void redraw64(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw64 parm [esi][edi][ebx] modify [ecx eax]
void redraw64b(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw64b parm [esi][edi][ebx] modify [ecx eax]
void redraw32bb(void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redraw32bb parm [esi][edi][ebx] modify [ecx eax]
void redrawbox_lo(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox_lo parm [ecx][edx][esi][edi][ebx] modify [eax edx]
void redrawbox256(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox256 parm [edx][ecx][esi][edi][ebx] modify [eax edx]
void redrawbox256b(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox256b parm [edx][ecx][esi][edi][ebx] modify [eax edx]
void redrawbox32(uint16_t xs,uint16_t ys,void *screen,void *lbuffer);
//#pragma aux redrawbox32 parm [ebx][edx][esi][edi] modify [ecx eax]
void redrawbox32b(uint16_t xs,uint16_t ys,void *screen,void *lbuffer);
//#pragma aux redrawbox32b parm [ebx][edx][esi][edi] modify [ecx eax]
void redrawbox64(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox64 parm [ecx][edx][esi][edi][ebx] modify [eax]
void redrawbox64b(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox64b parm [ecx][edx][esi][edi][ebx]modify [eax]
void redrawbox32bb(uint16_t xs,uint16_t ys,void *screen,void *lbuffer);
//#pragma aux redrawbox32bb parm [ebx][edx][esi][edi] modify [ecx]
void redraw16(void *screen,void *lbuffer,uint8_t *xlat);
void redrawbox16(uint16_t xs,uint16_t ys,void *screen,void *lbuffer,uint8_t *xlat);
//#pragma aux redrawbox16 parm [edx][ecx][esi][edi][ebx] modify [eax edx]
//#pragma aux redraw16 parm [esi][edi][ebx] modify [ecx]
void showview32(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys);
void showview256(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys);
void showview_lo(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys);
int initmode_dx(char inwindow, char zoom, char monitor, int refresh);
int initmode32();
int initmode32b();
int initmode256(void *paletefile);
int initmode256b(void *paletefile);
int initmode_lo(void *paletefile);
int initmode16(void *paletefile);
int initmode64(void *paletefile);
int initmode64b(void *paletefile);
void *create_hixlat();
void closemode();
void line32(uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2);
void position(uint16_t x,uint16_t y);
void outtext(const char *text);
void show_ms_cursor(int16_t x,int16_t y);
void *register_ms_cursor(uint16_t *cursor);
void move_ms_cursor(int16_t newx,int16_t newy,char nodraw);
void hide_ms_cursor();
int text_height(const char *text);
int text_width(const char *text);
void set_aligned_position(int x,int y,char alignx, char aligny,const char *text);
void wait_retrace();
void pal_optimize();
void rectangle(int x1,int y1,int x2,int y2,int color);
uint16_t *mapvesaadr1(uint16_t *a);
void *create_special_palette();
void *create_special_palette2();
void *create_blw_palette16();
void rel_position_x(uint16_t x);
int init_empty_mode();

void put_8bit_clipped(uint16_t *src,uint16_t *trg,int startline,int velx,int vely);
//#pragma aux put_8bit_clipped parm [ESI][EDI][EAX][EBX][EDX] modify [ECX];
void put_textured_bar_(uint16_t *src,uint16_t *trg,int xsiz,int ysiz,int xofs,int yofs);
//#pragma aux put_textured_bar_ parm [EBX][EDI][EDX][ECX][ESI][EAX];
void put_textured_bar(uint16_t *src,int x,int y,int xs,int ys,int xofs,int yofs);
void trans_bar(int x,int y,int xs,int ys,int barva);
//#pragma aux trans_bar parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];
void trans_bar25(int x,int y,int xs,int ys);
//#pragma aux trans_bar25 parm [EDI][ESI][EDX][ECX] modify [EAX EBX];
void trans_line_x(int x,int y,int xs,int barva);
//#pragma aux trans_line_x parm [EDI][ESI][ECX][EDX] modify [EAX];
void trans_line_y(int x,int y,int ys,int barva);
//#pragma aux trans_line_y parm [EDI][ESI][ECX][EDX] modify [EAX];
void draw_placed_texture(short *txtr,int celx,int cely,int posx,int posy,int posz,char turn);

void put_image(uint16_t *image,uint16_t *target,int start_line,int sizex,int sizey);
//#pragma aux put_image parm [ESI][EDI][EAX][EBX][EDX] modify [ECX]
void put_picture2picture(uint16_t *source,uint16_t *target,int xp,int yp);
//#pragma aux put_picture2picture parm [ESI][EDI][EAX][EDX] modify [ECX]



#define swap_int(a,b) do  {int c=a;a=b;b=c;} while (0);
