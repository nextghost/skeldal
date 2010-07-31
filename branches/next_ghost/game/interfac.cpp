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
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "libs/pcx.h"
#include <inttypes.h>
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/memman.h"
#include "libs/sound.h"
#include "libs/strlite.h"
#include <cctype>
#include "libs/basicobj.h"
#include <ctime>
#include "libs/mgfplay.h"
#include <fcntl.h>
#include <errno.h>
#include "game/globals.h"
#include "game/engine1.h"
#include "game/interfac.h"
#include "libs/system.h"

#define MES_MAXSIZE 500
#define CHECK_BOX_ANIM 6
#define COL_SIZE 776

//static const char *error_hack = "....Source compiled.";
static char shadow_enabled=1;

//uint16_t color_topbar[7]={0,RGB555(22,14,4),RGB555(24,16,6),RGB555(25,17,7)};
uint8_t color_topbar[7][3] = {
	{0,0,0},
	{181,115,33},
	{198,132,49},
	{206,140,57}
};

int input_txtr=H_LOADTXTR;

PalBlock::PalBlock(SeekableReadStream &stream, uint8_t r, uint8_t g, uint8_t b)
	: _pals(NULL), _size(0) {

	int i, j, k;

	_size = stream.size() / COL_SIZE;
	assert(_size && "Invalid palette data");
	_pals = new fadepal_t[_size];

	for (i = 0; i < _size; i++) {
		stream.readUint32LE();
		stream.readUint32LE();

		for (j = 0; j < PAL_SIZE; j++) {
			_pals[i][0][j] = stream.readUint8();
		}

		palette_shadow(_pals[i], r, g, b);
	}
}

PalBlock::~PalBlock(void) {
	delete[] _pals;
}

const pal_t *PalBlock::getPal(unsigned pal) const {
	assert(pal < _size && "Invalid palette index");
	return _pals[pal];
}

void create_frame(int x, int y, int xs, int ys, char clear) {
	int i;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_RAMECEK));

	x -= VEL_RAMEC;
	y -= VEL_RAMEC;
	xs = (xs + VEL_RAMEC - 1) / VEL_RAMEC + 1;
	ys = (ys + VEL_RAMEC - 1) / VEL_RAMEC + 1;
	renderer->rectBlit(*tex, x, y, 0, 0, VEL_RAMEC, VEL_RAMEC, tex->palette());

	for (i = 1; i < xs; i++) {
		renderer->rectBlit(*tex, x + i * VEL_RAMEC, y, 0, VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());
	}

	renderer->rectBlit(*tex, x + xs * VEL_RAMEC, y, 0, 2 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());

	for (i = 1; i < ys; i++) {
		renderer->rectBlit(*tex, x, y + i * VEL_RAMEC, 0, 3 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());
		renderer->rectBlit(*tex, x + xs * VEL_RAMEC, y + i * VEL_RAMEC, 0, 4 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());
	}

	renderer->rectBlit(*tex, x, y + ys * VEL_RAMEC, 0, 5 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());

	for (i = 1; i < xs; i++) {
		renderer->rectBlit(*tex, x + i * VEL_RAMEC, y + ys * VEL_RAMEC, 0, 6 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());
	}

	renderer->rectBlit(*tex, x + xs * VEL_RAMEC, y + ys * VEL_RAMEC, 0, 7 * VEL_RAMEC, VEL_RAMEC, VEL_RAMEC, tex->palette());

	if (clear) {
		uint8_t tmp[3] = {COL_RAMEC};
		memcpy(curcolor, tmp, 3 * sizeof(uint8_t));
		x += VEL_RAMEC;
		y += VEL_RAMEC;
		xs = (xs - 1) * VEL_RAMEC;
		ys = (ys - 1) * VEL_RAMEC;
		bar(x, y, x + xs - 1, y + ys - 1);
	}
}


void show_textured_button(int x, int y, int xs, int ys, int texture, CTL3D *border3d, int xofs, int yofs) {
	int i;
	const Texture *tex;

	i = 0;

	if (texture) {
		tex = dynamic_cast<const Texture*>(ablock(texture));
		put_textured_bar(*tex, x, y, xs, ys, xofs, yofs);
	}

	for (i = border3d->bsize - 1; i >= 0; i--) {
		if (border3d->ctldef & (1 << i)) {
			trans_line_x(x + i, y + i, xs - (i << 1), 0, 0, 0);
			trans_line_x(x + i, y + ys - i - 1, xs - (i << 1), 255, 255, 255);
			trans_line_y(x + i, y + i, ys - (i << 1), 0, 0, 0);
			trans_line_y(x + xs - i - 1, y + i, ys - (i << 1), 255, 255, 255);
		} else {
			trans_line_x(x + i, y + i, xs - (i << 1), 255, 255, 255);
			trans_line_x(x + i, y + ys - i - 1, xs - (i << 1), 0, 0, 0);
			trans_line_y(x + i, y + i, ys - (i << 1), 255, 255, 255);
			trans_line_y(x + xs - i - 1, y + i, ys - (i << 1), 0, 0, 0);
		}
	}
}
void show_textured_win(struct window *w) {
	show_textured_button(w->x, w->y, w->xs - 10, w->ys - 10, w->txtid, &w->border3d, w->txtx, w->txty);

	if (shadow_enabled) {
		trans_bar((w->x + 10), w->y + w->ys - 10, (w->xs - 10), 10, 0, 0, 0);
		trans_bar(w->x + w->xs - 10, w->y + 10, 10, w->ys - 20, 0, 0, 0);
	} else {
		shadow_enabled = 1;
	}
}

void add_window(int x, int y, int xs, int ys, int texture, int border, int txtx, int txty) {
	CTL3D wb;
	WINDOW *p;

	xs &= ~1;
	wb.bsize = abs(border);
	wb.ctldef = -1 * (border < 0);
	p = create_window(x, y, xs, ys, 0, 0, 0, &wb);
	p->txtx = txtx;
	p->txty = txty;
	p->txtid = texture;
	p->draw_event = show_textured_win;
	desktop_add_window(p);
}


void zalamovani(const char *source, char *target, int maxxs, int *xs, int *ys) {
	strcpy(target, source);
	xs[0] = 0;
	ys[0] = 0;

	if ((xs[0] = renderer->textWidth(target)) > maxxs) {
		char c[2] = " ";
		char *ls, *ps, *cs;
		int sum;

		cs = ps = target;

		do {
			ls = NULL;
			sum = 0;

			while (sum < maxxs || ls == NULL) {
				c[0] = *ps++;

				if (c[0] == 0) {
					ls = NULL;
					break;
				}

				if (c[0] == 32) {
					ls = ps - 1;
				}

				sum += renderer->textWidth(c);
			}

			if (ls != NULL) {
				*ls = 0;
				ps = ls + 1;
				ls = NULL;
			}

			ys[0] += renderer->textHeight(cs);
			cs = ps;
		} while (c[0]);

		xs[0] = maxxs;
		*ps = 0;
	} else {
		char *c;

		c = target;
		c = strchr(c, 0);
		c++;
		*c = 0;
		ys[0] = renderer->textHeight(target);
	}
}

static T_CLK_MAP message_win[]=
  {
  {-1,0,0,0,0,NULL,0,H_MS_DEFAULT},
  {-1,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };

#define MSG_COLOR1 1,247,247,189

void open_message_win(int pocet_textu, va_list args) {
	int maxxs;
	int maxys, maxws, wscelk, wsx, wsy, wsys, y;
	int x1, y1, x2, y2;
	int i;
	char *text;
	const char *ptr;
	va_list locargs;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FBOLD));

	renderer->setFont(font, MSG_COLOR1);
	GUIObject::setDefaultFont(font, MSG_COLOR1);
	va_copy(locargs, args);
	ptr = va_arg(locargs, const char*);
	text = (char*)alloca(strlen(ptr) + 2);
	zalamovani(ptr, text, MES_MAXSIZE, &maxxs, &maxys);
	maxws = 0;
	wsys = 0;

	for (i = 1; i < pocet_textu; i++) {
		ptr = va_arg(locargs, const char*);
		int z1 = renderer->textWidth(ptr);
		int z2 = renderer->textHeight(ptr);

		if (z1 > maxws) {
			maxws = z1;
		}

		if (z2 > wsys) {
			wsys = z2;
		}
	}

	va_end(locargs);
	maxws += 10;

	if (maxws < 50) {
		maxws = 50;
	}

	wscelk = (pocet_textu - 1) * (maxws + 20);

	if (wscelk > maxxs) {
		maxxs = wscelk;
	}

	wsy = 10 + wsys;
	maxys += wsy + 40;

	if (maxys < 50) {
		maxys = 50;
	}

	maxxs += 20;
	add_window(x1 = 320 - (maxxs >> 1), y1 = 180 - (maxys >> 1), x2 = maxxs + 10, y2 = maxys + 10, H_WINTXTR, 2, 0, 0);
	message_win[0].xlu = x1;
	message_win[0].ylu = y1;
	message_win[0].xrb = x1 + x2;
	message_win[0].yrb = y1 + y2;
	change_click_map(message_win, 2);
	set_window_modal();
	y = 10;

	while (text[0]) {
		define(new Label(-1, 10, y, 1, 1, 0, text));
		y += renderer->textHeight(text);
		text = strchr(text, 0) + 1;
	}

	wsx = (maxxs - wscelk) >> 1;
	va_copy(locargs, args);
	ptr = va_arg(locargs, const char*);

	for (i = 1; i < pocet_textu; i++) {
		ptr = va_arg(locargs, const char*);
		define(new Button(i - 1, wsx + 10, wsy, maxws + 10, wsys + 10, 3, ptr));
		property(def_border(5, BAR_COLOR), font, 0, flat_color(247, 247, 189), single_color(BAR_COLOR));
		on_change(terminate);
		wsx += maxws + 20;
	}

	va_end(locargs);
	redraw_window();
}

static char default_action,cancel_action;

EVENT_PROC(message_keyboard) {
	char *str;
	va_list args;

	switch(GET_MSG()) {
	case E_INIT:
		va_copy(args, msg->data);
		str = va_arg(args, char*);
		va_end(args);

		SAVE_USER_PTR(NewArr(char, strlen(str)+1));
		strcpy(GET_USER(char *), str);
		break;
	case E_DONE:
		free(*GET_USER_PTR());
		SAVE_USER_PTR(NULL);
		break;
	case E_KEYBOARD:
		{
			char *keys = GET_USER(char *);
			char code, *p;
			int key;

			va_copy(args, msg->data);
			code = va_arg(args, int);
			va_end(args);

			if (code == 0) {
				return;
			}

			code = toupper(code);

			if (code == 13) {
				key = default_action;
			} else if (code == 27) {
				key = cancel_action;
			} else if ((p = strchr(keys, code)) != NULL) {
				key = p - keys;
			} else {
				key = -1;
			}

			if (key!=-1) {
				goto_control(key);
				terminate();
			}
		}
	}
}

int message(int butts, char def, char canc, const char *keys, ...) {
	int id;
	void *clksav;
	int clksav2;
	va_list args;

	default_action = def;
	cancel_action = canc;
	mute_all_tracks(0);
	unwire_proc();
	save_click_map(&clksav, &clksav2);
	change_click_map(NULL, 0);
	va_start(args, keys);
	open_message_win(butts + 1, args);
	va_end(args);
	send_message(E_ADD, E_KEYBOARD, message_keyboard, keys);
	escape();
	id = o_aktual->id();
	send_message(E_DONE, E_KEYBOARD, message_keyboard, keys);
	close_current();
	restore_click_map(clksav, clksav2);
	return id;
}

//------------------

typedef void (*type_text_exit_proc)(char);

void type_text(EVENT_MSG *msg,void **data) {
	static int x, y, xs, ys;
	static char text[255], index, len;
	static char *source;
	static int max_size, max_chars;
	static int font, edit, tw;
	static uint8_t color[3];
	static type_text_exit_proc exit_proc;
	static Texture *back_pic;
	int i, px, ok = 0;
	const Font *fnt;
	va_list args;

	if (msg->msg == E_INIT) {
		int *p;
		char *c;

		va_copy(args, msg->data);
		c = va_arg(args, char*);
		x = va_arg(args, int);
		y = va_arg(args, int);
		max_size = va_arg(args, int);
		max_chars = va_arg(args, int);
		font = va_arg(args, int);
		color[0] = va_arg(args, int);
		color[1] = va_arg(args, int);
		color[2] = va_arg(args, int);
		exit_proc = va_arg(args, type_text_exit_proc);
		va_end(args);

		// prepare buffer
		for (i = 0; i < 254; i++) {
			text[i] = i + 1;
		}
	
		text[i] = 0;
		ys = renderer->textHeight(text) + 5;
		edit = 0;

		strcpy(text, c);
		source = c;
		fnt = dynamic_cast<const Font*>(ablock(font));
		renderer->setFont(fnt, 0, color[0], color[1], color[2]);
		tw = renderer->textWidth(text);
		len = index = strlen(text);
		xs = max_size + renderer->textWidth("_");
		back_pic = new SubTexture(*renderer, x, y, xs, ys);

		// render string
		renderer->drawText(x, y, text);
		renderer->drawText(tw + x, y + 3, "_");
		showview(x, y, xs, ys);
	} else if (msg->msg == E_KEYBOARD) {
		char sz[2] = " ";
		uint16_t c;

		va_copy(args, msg->data);
		c = va_arg(args, int);
		va_end(args);
		fnt = dynamic_cast<const Font*>(ablock(font));
		renderer->setFont(fnt, 0, color[0], color[1], color[2]);

		switch (c & 0xff) {
		case 8:
			if (index) {
				index--;
				memmove(text + index, text + index + 1, len-index);
				len--;
			}

			break;

		case 13:
			strcpy(source, text);
			ok = 1;

		case 27:
			send_message(E_DONE, E_KEYBOARD, type_text);
			exit_proc(ok);
			return;

		case 0:
			switch (c >> 8) {
			// left arrow
			case 0x4b:
				if (index > 0) {
					index--;
				}
				break;

			// right arrow
			case 0x4d:
				if (index < len) {
					index++;
				}
				break;

			// home
			case 0x47:
				index = 0;
				break;

			// ???
			case 0x4f:
				index = len;
				break;

			// delete
			case 0x53:
				if (len > index) {
					memmove(text + index, text + index + 1, len - index);
					len--;
				}
				break;

			// page down?
			case 0x74:
				while (index < len) {
					index++;
					if (text[index] == ' ') {
						break;
					}
				}
				break;

			// end?
			case 0x73:
				while (index > 0) {
					index--;
					if (text[index] == ' ') {
						break;
					}
				}
				break;
			}
			break;

		default:
			sz[0] = c & 0xff;

			if (edit) {
				if (sz[0] < 32 || tw + renderer->textWidth(sz) > max_size || len >= max_chars) {
					break;
				}

				memmove(&text[index+1], &text[index], len - index + 1);
				text[index] = sz[0];
				len++;
				index++;
			} else {
				text[0] = sz[0];
				text[1] = 0;
				len = 1;
				index = 1;
			}
			break;
		}

		tw = renderer->textWidth(text);
		edit = c ? 1 : edit;	// Shift makes a keypress now too

		renderer->blit(*back_pic, x, y, back_pic->palette());
		fnt = dynamic_cast<const Font*>(ablock(font));
		renderer->setFont(fnt, 0, color[0], color[1], color[2]);
		renderer->drawText(x, y, text);
		sz[0] = text[index];
		text[index] = 0;
		px = renderer->textWidth(text);
		text[index] = sz[0];
		renderer->drawText(px + x, y + 3, "_");
		ukaz_mysku();
		showview(x, y, xs, ys);
	} else if (msg->msg == E_DONE) {
		delete back_pic;
	}
}

DataBlock *col_load(SeekableReadStream &stream) {
	return new PalBlock(stream, gameMap.global().fade_r, gameMap.global().fade_g, gameMap.global().fade_b);
}


#define Hi(x) ((x)>>16)
#define Lo(x) ((x)& 0xffff)


char labyrinth_find_path(uint16_t start, uint16_t konec, int flag, char (*proc)(uint16_t), uint16_t **cesta) {
	int32_t *stack;
	int32_t *stk_free;
	int32_t *stk_cur;
	char *ok_flags;
	char vysl;

	if (cesta != NULL) {
		*cesta = NULL;
	}

	stk_free = stk_cur = stack = (int32_t*)getmem((gameMap.coordCount() + 2) * sizeof(int32_t));
	memset(ok_flags = (char*)getmem((gameMap.coordCount() + 8) / 8), 0, (gameMap.coordCount() + 8) / 8);
	ok_flags[start >> 3] |= 1 << (start & 0x7);

	for (*stk_free++ = start; stk_free != stk_cur; stk_cur++) {
		char i;
		uint16_t s, d, ss;

		s = (ss = Lo(*stk_cur)) << 2;

		for (i = 0; i < 4; i++) {
			if (!(gameMap.sides()[s+i].flags & flag)) {
				char c;
				uint16_t w;

				d = gameMap.sectors()[ss].step_next[i];
				c = 1 << (d & 0x7);
				w = d >> 3;

				if (!(ok_flags[w] & c) && proc(d)) {
					ok_flags[w] |= c;
					*stk_free++ = d | ((stk_cur - stack) << 16);
				}

				if (d == konec) {
					break;
				}
			}
		}

		if (d == konec) {
			break;
		}
	}

	vysl = 0;

	if (stk_free != stk_cur) {
		if (cesta != NULL) {
			int count = 0;
			int32_t *p, *z;
			uint16_t *x;

			z = p = stk_free - 1;

			while (Lo(*p) != start) {
				int l;

				count++;
				l = *p;
				p = Hi(l) + stack;
				*z-- = Lo(l);
			}

			x = *cesta = (uint16_t*)getmem((count + 1) * sizeof(uint16_t));
			z++;

			while (count--) {
				*x++ = (uint16_t)*z++;
			}

			*x++ = 0;
		}

		vysl = 1;
	}

	free(stack);
	free(ok_flags);
	return vysl;
}

typedef struct radio_butt_data
  {
  uint16_t *picture;
  char *texty;
  }TRADIO_BUTT_DATA;

/*
char ask_test(char *text,char def)
  {
  char znak;
  SEND_LOG("(START CHECK) %s",text,0);
  cprintf("\n\r%s (A/N, Cokoliv=%c)\x7",text,def);
  znak=getche();
  if (znak>='a' && znak<='z') znak-='a'-'A';
  if (znak!='N' && znak!='A') znak=def;
  return znak=='A';
  }
*/

long get_disk_free(char disk)
  {
  return 10*1024*1024;
/*  struct diskfree_t ds;
  if (_dos_getdiskfree(disk,&ds)==0)
     return ds.avail_clusters*ds.sectors_per_cluster*ds.bytes_per_sector;
  return 0;*/
  }

void start_check()
  {
  /*
  char *c;
  unsigned drv;
  long siz;
  struct meminfo memory;
  get_mem_info(&memory);
  concat(c,pathtable[SR_TEMP],TEMP_FILE);
//  if (!access(c,F_OK))
//     if (ask_test("Skeldal nebyl spr vnˆ ukon‡en. Mˆl bys provest kontrolu disku\n\rMam spustit SCANDISK?",'A'))
//        system("SCANDISK /NOSUMMARY");
  if (pathtable[SR_TEMP][1]==':') drv=pathtable[SR_TEMP][0];else
        {
        _dos_getdrive(&drv);drv+='@';
        }
  if (drv>='a' && drv<='z') drv-='a'-'A';
  siz=get_disk_free(drv-'@')/1024;
  SEND_LOG("Checking system enviroment - Largest Free Block %u bytes/pages %d",memory.LargestBlockAvail,memory.NumPhysicalPagesFree);
  SEND_LOG("Checking system enviroment - Disk space on %c: %d Kb",drv,siz);
  c=alloca(1024);
  if (siz<1024)
     {
     sprintf(c,"Na disku %c: nen¡ pot©ebn‚ 1 MB pro ukl d n¡ pozic. Hroz¡ ‘e pozice nebude kam ukl dat\n\rP©esto spustit?",drv);
     if (!ask_test(c,'N')) exit(1);
     }
  else if (siz<50000 && level_preload==1 && memory.LargestBlockAvail<50000000)
     {
     sprintf(c,"Na disku %c: neni nutn˜ch 50 MB pro odkl d n¡ dat. Skeldal bude ¨et©it\n\r"
               "s pamˆt¡ a nahr vat jen pot©ebn  data. Hra se m–‘e rapidnˆ zpomalit!\n\r"
               "Mam to udˆlat?",drv);
     if (ask_test(c,'A')) level_preload=0;
     }
     */
  }
/*
typedef struct dos_extra_block
  {
  long sector;
  uint16_t pocet;
  uint16_t buffer_ofs;
  uint16_t buffer_seg;
  };


typedef struct disk_label
  {
  uint16_t nula;
  long serial;
  char label[11];
  char type[8];
  };
*/
/*static void read_1st_sector(char drive,char *sector)
  {
  uint16_t segment;
  uint16_t selector;
  uint16_t exseg;
  uint16_t exbuf;
  void *ptr;
  struct dos_extra_block *data;

  RMREGS regs;

  dosalloc(32,&segment,&selector);
  dosalloc(2,&exseg,&exbuf);
  ptr=(void *)(segment<<4);
  data=(void *)(exseg<<4);
  data->sector=0;
  data->pocet=1;
  data->buffer_ofs=0;
  data->buffer_seg=segment;
  regs.eax=drive;
  regs.ecx=0xffff;
  regs.ds=exseg;
  regs.ebx=0;
  rmint(0x25,0,0,&regs);
  memcpy(sector,ptr,512);
  dosfree(selector);
  dosfree(exbuf);
  }

  */
/*
long read_serial(char drive)
  {
  uint16_t segment;
  uint16_t selector;
  struct disk_label *p;
  RMREGS regs;
  long serial;

  dosalloc(32,&segment,&selector);
  regs.eax=0x6900;
  regs.ebx=drive;
  regs.ds=segment;
  regs.edx=0;
  rmint(0x21,0,0,&regs);
  p=(void *)(segment<<4);
  serial=p->serial;
  dosfree(selector);
  return serial;
  }

static void crash_event1(THE_TIMER *t)
  {
  long serial;
  int i;

  serial=read_serial(t->userdata[1]);
  serial=~serial;
  if (serial==t->userdata[0]) return;
  outp(0x64,0xfe);
  send_message(E_PRGERROR,&i);
  exit(0);
  }

static void crash_event2(THE_TIMER *t)
  {
  long serial;
  int i;

  serial=read_serial(t->userdata[1]);
  serial=~serial;
  if (serial==t->userdata[0]) return;
  outp(0x64,0xfe);
  send_message(E_PRGERROR,&i);
  exit(0);
  }

static void crash_event3(THE_TIMER *t)
  {
  long serial;
  int i;

  serial=read_serial(t->userdata[1]);
  serial=~serial;
  if (serial==t->userdata[0]) return;
  outp(0x64,0xfe);
  send_message(E_PRGERROR,&i);
  exit(0);
  }


#pragma aux check_number_1phase parm[];
void check_number_1phase(char *exename) //check serial number!
  {
  THE_TIMER *t;
  int h;
  char buffer[_MAX_PATH];
  unsigned short date,time;
  long serial;

  _fullpath(buffer,exename,_MAX_PATH);
  t=add_to_timer(TM_HACKER,2000,1,crash_event1);
  t->userdata[0]=*(long *)error_hack;
  t->userdata[1]=(long)buffer[0]-'@';
  t=add_to_timer(TM_HACKER,3000,1,crash_event2);
  t->userdata[0]=*(long *)error_hack;
  t->userdata[1]=(long)buffer[0]-'@';
  h=open(exename,O_RDONLY);
  _dos_getftime(h,&date,&time);
  serial=(date<<16) | time;
  t=add_to_timer(TM_HACKER,4000,1,crash_event3);
  t->userdata[0]=~serial;
  t->userdata[1]=(long)buffer[0]-'@';
  close(h);
  }

*/

void animate_checkbox(int first_id, int last_id, int step) {
	int i;

	for(i = first_id; i <= last_id; i += step) {
		WINDOW *w;
		SkeldalCheckBox *cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, i, &w));

		if (!cb) {
			continue;
		}

		cb->nextFrame();
		redraw_object(cb);
	}
}

SkeldalCheckBox::SkeldalCheckBox(int id, int x, int y, int width, int height,
	int align, int value) : GUIObject(id, x, y, width, height, align),
	_phase(value ? 1 : 0), _background(NULL) {

}

SkeldalCheckBox::~SkeldalCheckBox(void) {
	if (_background) {
		delete _background;
	}
}

void SkeldalCheckBox::draw(int x, int y, int width, int height) {
	int phase;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_CHECKBOX));

	if (!_background) {
		_background = new SubTexture(*renderer, x, y, tex->width(), tex->width());
	} else {
		renderer->blit(*_background, x, y, _background->palette());
	}

	phase = (CHECK_BOX_ANIM - (_phase >> 1)) * 20;
	renderer->rectBlit(*tex, x, y, 0, phase, tex->width(), tex->width());
}

void SkeldalCheckBox::event(EVENT_MSG *msg) {
	if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->event_type & 0x2) {
			_phase ^= 1;
			set_change();
		}
	}
}

void SkeldalCheckBox::nextFrame(void) {
	if ((_phase & 1) && (_phase >> 1) < CHECK_BOX_ANIM) {
		_phase += 2;
	} else if (~_phase & 1 && (_phase >> 1) > 0) {
		_phase -= 2;
	}
}

void SkeldalCheckBox::setValue(int value) {
	_phase = (_phase & ~1) | (value ? 1 : 0);
}

//------------------------------------------

SetupOkButton::SetupOkButton(int id, int x, int y, int width, int height,
	int align, const char *text) : GUIObject(id, x, y, width, height,
	align), _text(NULL), _background(NULL), _toggle(0) {

	_text = new char[strlen(text) + 1];
	strcpy(_text, text);
}

SetupOkButton::~SetupOkButton(void) {
	delete[] _text;

	if (_background) {
		delete _background;
	}
}

void SetupOkButton::draw(int x, int y, int width, int height) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_SETUPOK));

	if (!_background) {
		_background = new SubTexture(*renderer, x, y, tex->width(), tex->height());
	}

	if (_toggle) {
		renderer->blit(*tex, x, y, tex->palette());
	} else {
		renderer->blit(*_background, x, y, _background->palette());
	}

	x = x + (10 + width) / 2;
	y = y + height / 2;
	renderer->drawAlignedText(x, y, HALIGN_CENTER, VALIGN_CENTER, _text);
}

void SetupOkButton::event(EVENT_MSG *msg) {
	if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->event_type & 0x2) {
			_toggle = 1;
			redraw_object(this);
		} else if (ms->event_type & 0x4) {
			_toggle = 0;
			redraw_object(this);
			set_change();
		}
	} else if (msg->msg == E_LOST_FOCUS) {
		_toggle = 0;
		redraw_object(this);
	}
}

//-----------------------------------------------------

SkeldalSlider::SkeldalSlider(int id, int x, int y, int width, int height,
	int align, int range, int value) : GUIObject(id, x, y, width, height,
	align), _range(range), _background(NULL), _value(value) {

	_value = _value < 0 ? 0 : _value;
	_value = _value > _range ? _range : _value;
}

SkeldalSlider::~SkeldalSlider(void) {
	if (_background) {
		delete _background;
	}
}

void SkeldalSlider::draw(int x, int y, int width, int height) {
	int total;
	int xpos;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_SOUPAK));

	total = height - tex->height();
	xpos = y + height - tex->height() - _value * total / _range;

	if (!_background) {
		_background = new SubTexture(*renderer, x, y, width + 1, height + 1);
	} else {
		renderer->blit(*_background, x, y, _background->palette());
	}

	renderer->blit(*tex, x, xpos, tex->palette());
}

void SkeldalSlider::event(EVENT_MSG *msg) {
	if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;
		int total;
		int ypos;
		const Texture *tex;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->tl1) {
			tex = dynamic_cast<const Texture*>(ablock(H_SOUPAK));
			total = _height - tex->height();
			ypos = ms->y - _locy;
			ypos += tex->height();
			_value = (_height - ypos) * _range / total;

			if (_value < 0) {
				_value = 0;
			}

			if (_value > _range) {
				_value = _range;
			}

			redraw_object(this);
			set_change();
		}
	}
}

static char fletna_str[13];
static char pos=0;

// C  C# D  D# E  F  F# G  G# A  A# B  C
// A  B  C  D  E  F  G  H  I  J  K  L  M
static char smery[4][13]=
  {
  "CHLJLH", //DGBABG
  "BDEDGE", //C#D#ED#F#E
  "LHJGHE",
  "CIGILI"
  };

void fletna_pridej_notu(char note)
  {
  note+=65;
  fletna_str[pos]=note;
  if (pos==13) memcpy(fletna_str,fletna_str+1,pos);
  else pos++;
  }

static void play_wav(int wav,int sector)
  {
  if (Sound_CheckEffect(SND_GFX))
     {
     play_sample_at_sector(wav,sector,sector,0,0);
     }
/*  else
     {
     struct t_wave *p;
     char *sample;
     int siz;

     p=ablock(wav);
     sample=(char *)p+sizeof(struct t_wave);
     memcpy(&siz,sample,4);sample+=4;
     pc_speak_play_sample(sample,siz,(p->freq!=p->bps?2:1),p->freq);
     }*/
  }

static void play_random_sound(int sector,int dir,int pos)
  {
  int seed;
  int v;

  seed=rand();
  srand(sector+dir);
  while (pos--)
     do
        {
        v=rnd(8);
        }
     while (v==dir);
  play_wav(H_SND_SEVER+v,sector);
  srand(seed);
  }

static void play_correct_sound(int sector,int dir)
  {
  play_wav(H_SND_SEVER+dir,sector);
  }

void check_fletna(THE_TIMER *t) {
	char len;
	char *s;
	int sec;
	int dir;

	if (!pos) {
		return;
	}

	sec = t->userdata[0];
	dir = t->userdata[1];
	s = smery[dir];
	len = strlen(s);

	if (len == pos) {
		if (!strncmp(s, fletna_str, pos) && gameMap.sectors()[sec].sector_type == dir + S_FLT_SMER) {
			play_correct_sound(sec, dir);
		} else {
			play_random_sound(sec, dir, pos);
		}
	} else {
		play_random_sound(sec, dir, pos);
	}

	pos = 0;
}

char fletna_get_buffer_pos()
  {
  return pos;
  }

static char globFletnaStr[256]="";
static char globNotes[][3]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","H","C"};

void fletna_glob_add_note(char note)
{
  if (strlen(globFletnaStr)<250) strcat(globFletnaStr,globNotes[note]);
}

static char compareMelody(const char *m1,const char *m2)
{
  while (*m1 && *m2)
  {
    if (!isalpha(*m1) && *m1!='#') m1++;
    else if (!isalpha(*m2) && *m2!='#') m2++;
    else if (toupper(*m1)!=toupper(*m2)) break;
    else 
    {
      m1++;
      m2++;
    }
    
  }
  if (*m1>*m2) return 1;
  if (*m1<*m2) return -1;
  return 0;
}

void check_global_fletna(THE_TIMER *t)
  {
  int sec;
  int dir;
  int i;
  int other=-1;  

  t;
  sec=t->userdata[0];
  dir=t->userdata[1];
  if (globFletnaStr[0] && globFletnaStr[strlen(globFletnaStr)-1]==32)
  {
    globFletnaStr[strlen(globFletnaStr)-1]=0;
  }
  for (i=MAGLOB_ONFLUTE1;i<=MAGLOB_ONFLUTE8 ;i++) 
    if (GlobEventList[i].param!=0)
    {
      {
        const char *cmp=level_texts[GlobEventList[i].param];
        if (compareMelody(cmp,globFletnaStr)==0)
        {
          GlobEvent(i,sec,dir);
          globFletnaStr[0]=0;
          return;
        }
      }
    }
    else if (GlobEventList[i].sector || GlobEventList[i].side) other=i;
  if (other!=-1)
    GlobEvent(other,sec,dir);
  globFletnaStr[0]=0;
  }


//---------------------------------------

char *find_map_path(const char *filename) {
	char *p1, *p;

	p1 = Sys_FullPath(SR_MAP2, filename);

	if (!Sys_FileExists(p1)) {
		p1 = Sys_FullPath(SR_MAP, filename);
	}

	p = (char*)malloc((strlen(p1) + 1) * sizeof (char));
	strcpy(p, p1);
	return p;
}

SeekableReadStream *enc_open(const char *filename) {
	File *f;
	MemoryReadStream *ret;
	char *c, *d, *enc, *temp;
	int i, j, last = 0;

	f = new File(filename);

	if (f->isOpen()) {
		return f;
	}

	enc = (char*)alloca(strlen(filename) + 5);
	strcpy(enc, filename);
	c = strrchr(enc, '.');
	if (!c) {
		c = strchr(enc, 0);
	}

	strcpy(c, ".ENC");
	f->open(enc);

	if (!f->isOpen()) {
		delete f;
		return NULL;
	}

	i = f->size();
	c = new char[i];
	f->read(c, i);

	for (j = 1; j < i; j++) {
		c[j] += c[j-1];
	}

	ret = new MemoryReadStream(c, i);
	delete[] c;
	delete f;

	return ret;
}

int load_string_list_ex(StringList &list, const char *filename) {
	char c[1024], *p;
	int i, j, lin = 0;
	SeekableReadStream *f;

	f = enc_open(filename);

	if (!f) {
		return -1;
	}

	do {
		lin++;
		do {
			do {
				j = f->readUint8();
			} while ((unsigned)j <= ' ');

			if (j == ';') {
				while ((j = f->readUint8()) != '\n' && !f->eos());
			}

			if (j == '\n') {
				lin++;
			}
		} while (j == '\n');

		c[0] = j;
		for (i = 0; (c[i] >= '0' && c[i] <= '9') || (i == 0 && c[0] == '-');) {
			c[++i] = (char)f->readUint8();
		}
		c[i] = '\0';
		j = sscanf(c, "%d", &i);

		if (f->eos()) {
			delete f;
			return -2;
		}

		if (j != 1) {
			delete f;
			return lin;
		}

		if (i == -1) {
			break;
		}

		while ((unsigned)(j = f->readUint8()) <= ' ' && !f->eos());

		if (!f->eos()) {
			f->seek(-1, SEEK_CUR);
		}

		if (f->readLine(c, 1022) == NULL) {
			delete f;
			return lin;
		}

		p = strchr(c, '\n');

		if (p) {
			*p=0;
		}

		for(p = c; *p; p++) {
			*p = *p == '|' ? '\n' : *p;
		}

		list.replace(i, c);
	} while (1);

	delete f;
	return 0;
}

//------------------------------------------------------------
int smlouvat_nakup(int cena,int ponuka,int posledni,int puvod,int pocet)
  {
  int min=cena-((cena-puvod)*2*(pocet-1)/(pocet+1));
  int d_ok=posledni<min?cena-min:cena-(min+posledni)/2;
  int p_ok=(ponuka-min)*100/(d_ok+1);
  int r_ok=rnd(100);

  if (ponuka==0) return 1;
  if (pocet==1 && ponuka<cena) return 6;
  if (ponuka>=cena) return 0;
  if (ponuka<=posledni || ponuka<min) return 1;
  if (p_ok>r_ok) return 0;
  if (p_ok>75) return 2;
  if (p_ok>50) return 3;
  if (p_ok>25) return 4;
  return 5;
  }

int smlouvat_prodej(int cena,int ponuka,int posledni,int puvod,int pocet)
  {
  int min=cena+((puvod-cena)*2/pocet);
  int d_ok=posledni==0?min-cena:(min+posledni)/2-cena;
  int p_ok=(min-ponuka)*100/(d_ok+1);
  int r_ok=rnd(100);

  if (ponuka==0) return 0;
  if (ponuka<=cena) return 0;
  if (posledni!=0) if (ponuka>=posledni || ponuka>min) return 1;
  if (p_ok>r_ok) return 0;
  if (p_ok>75) return 2;
  if (p_ok>50) return 3;
  if (p_ok>25) return 4;
  return 5;
  }

static void smlouvat_enter(EVENT_MSG *msg) {
	if (msg->msg == E_KEYBOARD) {
		char c;
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) & 0xff;
		va_end(args);

		switch (c) {
			case 13:
				goto_control(30);
				terminate();
				break;

			case 27:
				goto_control(20);
				terminate();
				break;
		}
	}
}

int smlouvat(int cena, int puvod, int pocet, int money, char mode) {
	int ponuka = 0, posledni = 0;
	char text[255], buffer[20];
	const char *c;
	int y, yu, xu;
	int temp1, temp2;
	const Font *font;

	text[0] = 0;
	text[1] = 0;
	add_window(170, 130, 300, 150, H_IDESKA, 3, 20, 20);
	define(new Label(-1, 10, 15, 1, 1, 0, texty[241]));
//  set_font(H_FBOLD,RGB555(31,31,31));define(-1,150,15,100,13,0,label,itoa(cena,buffer,10));
	sprintf(buffer, "%d", cena);
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 1, 255, 255, 255);
	define(new Label(-1, 150, 15, 100, 13, 0, buffer));
	renderer->setFont(font, MSG_COLOR1);
	define(new Label(-1, 10, 30, 1, 1, 0, texty[238]));
	define(new InputLine(10, 150, 30, 100, 13, 0, 8));
	property(def_border(5, BAR_COLOR), NULL, -1, NULL, NULL);
	on_event(smlouvat_enter);
	define(new Button(20, 20, 20, 80, 20, 2, texty[239]));
	property(def_border(5, BAR_COLOR), NULL, -1, NULL, single_color(BAR_COLOR));
	on_change(terminate);
	define(new Button(30, 110, 20, 80, 20, 2, texty[230]));
	property(def_border(5, BAR_COLOR), NULL, -1, NULL, single_color(BAR_COLOR));
	on_change(terminate);

	do {
		redraw_window();
		font = dynamic_cast<const Font*>(ablock(H_FBOLD));
		renderer->setFont(font, 1, 255, 255, 255);
		c = text;
		yu = y = waktual->y + 50;
		xu = waktual->x + 10;

		do {
			renderer->drawText(xu, y, c);
			y += renderer->textHeight(c) + 1;
			c = strchr(c, 0) + 1;
		} while(*c);

		showview(xu, yu, 280, y - yu);
		goto_control(10);
		escape();
		temp1 = 1;

		if (o_aktual->id() == 20) {
			cena = -1;
		} else {
			WINDOW *wi;
			InputLine *line = dynamic_cast<InputLine*>(find_object_desktop(0, 10, &wi));
			assert(line);
			strcpy(buffer, line->getString());

			if (buffer[0] == 0) {
				c = texty[240];
			} else {
				if (sscanf(buffer, "%d", &ponuka) != 1) {
					c = texty[237];
				} else if (ponuka > money && mode == 1) {
					c = texty[104];
				} else {
					if (mode) {
						temp1 = smlouvat_nakup(cena, ponuka, posledni, puvod, pocet);
					} else {
						temp1 = smlouvat_prodej(cena, ponuka, posledni, puvod, pocet + 1);
					}

					posledni = ponuka;

					if (rnd(100) < 50) {
						c = texty[230 + temp1];
					} else {
						c = texty[250 + temp1];
					}
				}
			}

			shadow_enabled=0;
		}

		if (c) {
			zalamovani(c, text, 280, &temp2, &temp2);
		}
	} while (temp1 != 0 && cena != -1);

	if (temp1 == 0) {
		cena = ponuka;
	}

	close_current();
	shadow_enabled = 1;
	return cena;
}

//----------------- JRC LOGO ----------------------------------

#define SHOWDELAY 125
#define SHOWDEND (SHOWDELAY-32)

typedef struct _hicolpal
  {
  unsigned blue:5;
  unsigned green:5;
  unsigned red:5;
  }HICOLPAL;

void show_jrc_logo(const char *filename) {
	char *s;
	char bnk = 1;
	int xp, yp, i;
	uint8_t palette[3 * 256] = {0};
	int cntr, cdiff, ccc;
	unsigned cpalf;
	Texture *pcx;

	Sound_ChangeMusic("?");
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	showview(0, 0, 0, 0);
	Timer_Sleep(1000);
//  concat(s,pathtable[SR_VIDEO],filename);
//  if (open_pcx(s,A_8BIT,&pcx)) return;

	pcx = open_pcx(Sys_FullPath(SR_VIDEO, filename), A_8BIT);

	if (!pcx) {
		return;
	}

	xp = pcx->width();
	yp = pcx->height();
	assert(pcx->palette());
	xp /= 2;
	yp /= 2;
	xp = 320 - xp;
	yp = 240 - yp;
	cntr = Timer_GetValue();
	ccc = 0;

	do {
		cdiff = (Timer_GetValue() - cntr) / 2;

		if (cdiff < SHOWDEND && ccc != cdiff) {
			cpalf = cdiff;

			if (cpalf < 32) {
				cpalf = (cpalf << 3) | (cpalf >> 2);

				for (i = 0; i < 3 * 256; i++) {
					palette[i] = min(pcx->palette()[i], cpalf);
				}
			}
		} else if (ccc != cdiff) {
			cpalf = SHOWDELAY - cdiff;

			if (cpalf < 32) {
				cpalf = 0xff - ((cpalf << 3) | (cpalf >> 2));

				for (i = 0; i < 3 * 256; i++) {
					palette[i] = (pcx->palette()[i] > cpalf) ? pcx->palette()[i] - cpalf : 0;
				}
			}
		}

		if (!bnk) {
			wait_retrace();
		}

		renderer->blit(*pcx, xp, yp, palette);

		if (bnk) {
			wait_retrace();
			showview(xp, yp, pcx->width(), pcx->height());
		}

		ccc = cdiff;
		Sound_MixBack(0);
	} while (cdiff < SHOWDELAY && !Input_Kbhit());

	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	showview(0, 0, 0, 0);
	delete pcx;
}
