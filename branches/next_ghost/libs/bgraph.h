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

#ifndef _BGRAPH_H_
#define _BGRAPH_H_

#include <inttypes.h>
#include "libs/pcx.h"
#include "libs/system.h"
#define hor_line(x, y, xs) bar32(x, y, xs, y)
#define ver_line(x, y, ys) bar32(x, y, x, ys)
#define bar bar32
#define point point32

#define RGB888(r, g, b) Screen_RGB((r) >> 3, (g) >> 3, (b) >> 3)
#define RGB555(r, g, b) Screen_RGB((r), (g), (b))
#define R(r,g,b) (r)
#define G(r,g,b) (g)
#define B(r,g,b) (b)

#define HALIGN_LEFT 0
#define HALIGN_CENTER 1
#define HALIGN_RIGHT 2

#define VALIGN_TOP 0
#define VALIGN_CENTER 1
#define VALIGN_BOTTOM 2

#define GLYPH_COUNT 256
#define FONT_COLORS 7

typedef struct t_floor_map {
	int x, srcy, dsty, linewidth, counter;
} T_FLOOR_MAP;

class Font : public DataBlock {
public:
	struct Glyph {
		unsigned width, height;
		uint8_t *data;
	};

private:
	Glyph _glyphs[GLYPH_COUNT];

	// Do not implement
	Font(const Font &src);
	const Font &operator=(const Font &src);

public:
	explicit Font(SeekableReadStream &stream);
	~Font(void);

	const Glyph &glyph(unsigned idx) const;
	unsigned textWidth(const char *text) const;
	unsigned textHeight(const char *text) const;
};

class SoftRenderer : public Texture {
private:
	const Font *_font;
	int _shadow;
	uint8_t _fontPal[FONT_COLORS][3];

public:
	SoftRenderer(unsigned xs, unsigned ys);
	~SoftRenderer(void);

	// texture blit with simple index transparency (0 = invisible)
	// scale: 320 means 1:1 scale, >320 to expand, <320 to shrink
	void blit(const Texture &tex, int x, int y, const uint8_t *pal = NULL, unsigned scale = 320, int mirror = 0);
	void rotBlit(const Texture &tex, int x, int y, float angle, const uint8_t *pal = NULL);
	// texture blit with index transparency (0 = invisible, 1 = blend)
	void enemyBlit(const Texture &tex, int x, int y, const uint8_t *pal = NULL, unsigned scale = 320, int mirror = 0);
	// texture blit with index transparency (0 = invisible, >=128 = blend)
	void transparentBlit(const Texture &tex, int x, int y, const uint8_t *pal = NULL, unsigned scale = 320, int mirror = 0);
	// texture blit with index transparency (0 = invisible, 1 = skip
	// to next line)
	void wallBlit(const Texture &tex, int x, int y, int32_t *xtable, unsigned xlen, int16_t *ytable, unsigned ylen, const uint8_t *pal = NULL, int mirror = 0);
	// fullscreen video frame blit (cutscene only)
	void videoBlit(unsigned ypos, const Texture &tex, const uint8_t *pal = NULL);
	// blit only a part of the texture, no scaling or transparency
	void rectBlit(const Texture &tex, int x, int y, unsigned tx, unsigned ty, unsigned w, unsigned h, const uint8_t *pal = NULL);
	// Fill a shape with selected color (actions: 0 = copy, 1 = blend,
	// 2 = subtract fraction)
	void maskFill(const Texture &tex, int x, int y, uint8_t mask, int action, uint8_t r, uint8_t g, uint8_t b);
	// Blit a texture using mask
	void maskBlit(const Texture &tex, const Texture &mask, int x, int y, uint8_t shape, const uint8_t *pal);
	// Draw a rectangle using selected color (actions: 0 = copy,
	// 1 = blend, 2 = subtract fraction, 3 = xor)
	void bar(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t r, uint8_t g, uint8_t b, int action = 0);
	void fcBlit(const Texture &tex, unsigned y, const T_FLOOR_MAP *celmask, const uint8_t *fog, const uint8_t *pal = NULL);

	void drawText(int x, int y, const char *text);
	void drawAlignedText(int x, int y, int halign, int valign, const char *text);
	void drawChar(int x, int y, unsigned char c);

	void setFont(const Font *font, int shadow, uint8_t r, uint8_t g, uint8_t b);
	void setFont(const Font *font, int shadow, uint8_t pal[][3]);
	void setFontColor(unsigned idx, uint8_t r, uint8_t g, uint8_t b);
	unsigned textWidth(const char *text) const;
	unsigned textHeight(const char *text) const;
	unsigned charWidth(char text) const;
	unsigned charHeight(char text) const;

	virtual void drawRect(unsigned x, unsigned y, unsigned xs, unsigned ys) { }
	virtual void xshift(int shift) { }
	virtual void setMouseCursor(const Texture &tex) { }
	virtual void showMouse(void) { }
	virtual void hideMouse(void) { }
	virtual void moveMouse(int x, int y) { }
};

class FadeRenderer : public TextureFade {
public:
	FadeRenderer(const uint8_t *pal, unsigned width, unsigned height, uint8_t r, uint8_t g, uint8_t b);
	~FadeRenderer(void) { }

	void blit(const Texture &tex, int x, int y);
};

class IconLib : public DataBlock {
private:
	TexturePal **_icons;
	unsigned _count;

public:
	IconLib(ReadStream &stream, unsigned count);
	~IconLib(void);

	const TexturePal &operator[](unsigned idx) const;
	unsigned size(void) const { return _count; }
};

// TODO: write a complete video backend instead
extern SoftRenderer *renderer, *backbuffer;

extern uint8_t curcolor[3];
extern uint16_t *curfont,*writepos,writeposx;
extern uint8_t *palmem,*xlatmem;
extern void (*showview)(uint16_t,uint16_t,uint16_t,uint16_t);
extern char line480;
extern char banking;
extern char __skip_change_line_test;
extern char no_restore_mode;

DataBlock *loadFont(SeekableReadStream &stream);

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
void show_ms_cursor(int x, int y);
void register_ms_cursor(const Texture *cursor);
void move_ms_cursor(int16_t newx,int16_t newy,char nodraw);
void hide_ms_cursor();
void wait_retrace();
void pal_optimize();
void rectangle(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b);
uint16_t *mapvesaadr1(uint16_t *a);
void *create_special_palette();
void *create_special_palette2();
void *create_blw_palette16();
int init_empty_mode();

void put_8bit_clipped(uint16_t *src,uint16_t *trg,int startline,int velx,int vely);
//#pragma aux put_8bit_clipped parm [ESI][EDI][EAX][EBX][EDX] modify [ECX];
void put_textured_bar(const Texture &tex, int x, int y, int xs, int ys, int xofs, int yofs);
void trans_bar(int x, int y, int xs, int ys, uint8_t r, uint8_t g, uint8_t b);
//#pragma aux trans_bar parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];
void trans_bar25(int x,int y,int xs,int ys);
//#pragma aux trans_bar25 parm [EDI][ESI][EDX][ECX] modify [EAX EBX];
void trans_line_x(int x, int y, int xs, uint8_t r, uint8_t g, uint8_t b);
//#pragma aux trans_line_x parm [EDI][ESI][ECX][EDX] modify [EAX];
void trans_line_y(int x, int y, int ys, uint8_t r, uint8_t g, uint8_t b);
//#pragma aux trans_line_y parm [EDI][ESI][ECX][EDX] modify [EAX];
void draw_placed_texture(const Texture *tex, int celx, int cely, int posx, int posy, int posz, char turn);

void put_image(uint16_t *image,uint16_t *target,int start_line,int sizex,int sizey);
//#pragma aux put_image parm [ESI][EDI][EAX][EBX][EDX] modify [ECX]

#define swap_int(a,b) do  {int c=a;a=b;b=c;} while (0);

#endif
