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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <inttypes.h>
#include "libs/event.h"
#include "libs/memman.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/sound.h"
#include "libs/strlite.h"
#include "game/engine1.h"
#include "libs/pcx.h"
#include "game/globals.h"
#include "libs/system.h"


#define MUSIC "TRACK06.MUS"

#define H_ANIM_ORIGN (H_MENUS_FREE+0)
#define H_ANIM       (H_MENUS_FREE+1)
#define H_MENU_BAR   (H_MENUS_FREE+31)
#define H_MENU_MASK  (H_MENUS_FREE+32)
#define H_MENU_ANIM  (H_MENUS_FREE+33)
#define H_PICTURE    (H_MENUS_FREE+39)

#define SELECT 1
#define UNSELECT -1

#define SPEED 3
int speedscroll=3;
char low_mem=0;
static volatile char load_ok=0;

static int cur_pos[]={0,0,0,0,0};
static int cur_dir[]={UNSELECT,UNSELECT,UNSELECT,UNSELECT,UNSELECT};
static int cur_click = 0;

static int titlefont = H_FBIG;

#define TITLE_HEAD 1
#define TITLE_NAME 2
#define TITLE_TEXT 3
#define TITLE_CENTER 0
#define TITLE_KONEC 4

static int title_mode = 0;
static int title_line = 0;

#define CLK_MAIN_MENU 4

static char vymacknout(int id,int xa,int ya,int xr,int yr)
  {

  for(id=0;id<5;id++) cur_dir[id]=UNSELECT;
  xa,ya,xr,yr;
  return 1;
  }

static char promacknuti(int id, int xa, int ya, int xr, int yr) {
	uint8_t pix;
	const Texture *mask = dynamic_cast<const Texture*>(ablock(H_MENU_MASK));

	assert(xr < mask->width() && yr < mask->height() && mask->depth() == 1);
	pix = mask->pixels()[xr + yr * mask->width()];
	vymacknout(id, xa, ya, xr, yr);

	if (pix) {
		cur_dir[pix - 1] = SELECT;
	}

	return 1;
}

static char click(int id,int xa,int ya,int xr,int yr)
  {
  int i;

  id,xa,ya,xr,yr;
  for(i=0;i<5;i++) if (cur_dir[i]==SELECT) break;
	if (i != 5) {
		cur_click = i;
		send_message(E_MENU_SELECT,i);
	}
  return 1;
  }


T_CLK_MAP clk_main_menu[]=
  {
  {-1,220,300,220+206,300+178,promacknuti,1,-1},
  {-1,220,300,220+206,300+178,click,2,-1},
  {-1,0,0,639,479,vymacknout,1,-1},
  {-1,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };

static DataBlock *nahraj_rozdilovy_pcx(SeekableReadStream &stream) {
	uint8_t *data;
	int i;
	Texture *ret, *diff = load_pcx(stream, A_8BIT);
	const Texture *orig = dynamic_cast<const Texture*>(ablock(H_ANIM_ORIGN));

	assert(orig->width() == diff->width() && orig->height() == diff->height() && orig->depth() == 1);

	data = new uint8_t[orig->width() * orig->height()];

	for (i = 0; i < orig->width() * orig->height(); i++) {
		data[i] = orig->pixels()[i] ^ diff->pixels()[i];
	}

	ret = new TextureHi(data, diff->palette(), orig->width(), orig->height());
	delete diff;
	delete[] data;
	return ret;
}

static void init_menu_entries(void) {
	int i;
	char *a;

	def_handle(H_ANIM_ORIGN, "LOGO00.PCX", pcx_8bit_decomp, SR_BGRAFIKA);
	def_handle(H_ANIM, "LOGO00.PCX", pcx_15bit_decomp, SR_BGRAFIKA);
	a = (char*)alloca(15);

	for (i = 1; i < 30; i++) {
		sprintf(a, "LOGO%02d.PCX", i);
		def_handle(H_ANIM + i, a, nahraj_rozdilovy_pcx, SR_BGRAFIKA);
	}

	def_handle(H_MENU_BAR, "MAINMENU.PCX", pcx_8bit_decomp, SR_BGRAFIKA);
	def_handle(H_MENU_MASK, "MENUVOL5.PCX", pcx_8bit_decomp, SR_BGRAFIKA);

	for (i = 0; i < 5; i++) {
		sprintf(a, "MENUVOL%d.PCX", i);
		def_handle(H_MENU_ANIM + i, a, pcx_15bit_decomp, SR_BGRAFIKA);
	}
}

static void zobraz_podle_masky(char barva, char anim) {
	unsigned xs, ys;
	const Texture *mask, *data;

	alock(H_MENU_MASK);
	mask = dynamic_cast<const Texture*>(ablock(H_MENU_MASK));
	data = dynamic_cast<const Texture*>(ablock(H_MENU_ANIM + anim));
	renderer->maskBlit(*data, *mask, 220, 300, barva, data->palette());
	aunlock(H_MENU_MASK);
}

static void prehraj_animaci_v_menu(EVENT_MSG *msg, char **unused) {
	static int counter = 0;
	const Texture *tex;

	if (msg->msg == E_TIMER) {
		if (counter % SPEED == 0) {
			int i = counter / SPEED;
			char show = 0;

			if (!low_mem || ~i & 1) {
				tex = dynamic_cast<const Texture*>(ablock(H_ANIM + i));
				renderer->blit(*tex, 0, 56, tex->palette());
			}

			do_events();

			for (i = 0; i < 5; i++) {
				cur_pos[i] += cur_dir[i];

				if (cur_pos[i] < 0) {
					cur_pos[i] = 0;
				} else if (cur_pos[i] > 4) {
					cur_pos[i] = 4;
				} else {
					zobraz_podle_masky(i + 1, cur_pos[i]);
					do_events();
					show = 1;
				}
			}

			update_mysky();
			showview(0, 56, 640, 250);

			if (show) {
				showview(220, 300, 206, 178);
			}
		}

		counter++;

		if (counter >= (SPEED * 30)) {
			counter = 0;
		}
	}
}


//static void preload_anim(va_list args)
static void preload_anim(void) {
  int i;

  low_mem=0;
  ablock(H_ANIM+29);
  for(i=0;i<30;i+=2)
     {
     apreload(H_ANIM+i);
//     Task_Sleep(NULL);
     }
  for(i=1;i<30;i+=2)
     {
     THANDLE_DATA *h;

     h=get_handle(H_ANIM+29);
     if (h->status!=BK_PRESENT)
        {
        low_mem=1;
        break;
        }
     apreload(H_ANIM+i);
//     Task_Sleep(NULL);
     }
  for(i=0;i<5;i++)
     {
     apreload(H_MENU_ANIM+i);
//     Task_Sleep(NULL);
     }
  apreload(H_MENU_MASK);
//  Task_WaitEvent(E_TIMER);
  load_ok=1;
  }

static void klavesnice(EVENT_MSG *msg, void **unused) {
	int cursor, i;

	if (msg->msg == E_KEYBOARD) {
		char c;
		va_list args;

		for (cursor = 0; cursor < 5; cursor++) {
			if (cur_dir[cursor] == SELECT) {
				break;
			}
		}

		if (cursor == 5) {
			cursor=-1;
		}

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		switch (c) {
		case 'H':
			cursor--;
			if (cursor < 0) {
				cursor = 0;
			}
			break;

		case 'P':
			cursor++;
			if (cursor > 4) {
				cursor=4;
			}
			break;

		case 28:
		case 57:
			click(0,0,0,0,0);
			return;
		}

		for(i = 0; i < 5; i++) {
			if (i == cursor) {
				cur_dir[i] = SELECT;
			} else {
				cur_dir[i] = UNSELECT;
			}
		}
	}
}

int enter_menu(char open) {
	int c;
	char *d;
	const Texture *tex;

	init_menu_entries();
//  Task_Add(2048,preload_anim);
//  load_ok=0;
//  while(!load_ok) Task_Sleep(NULL);
	preload_anim();

	if (!open) {
		play_next_music(&d);
		Sound_ChangeMusic(d);
	}

	update_mysky();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	tex = dynamic_cast<const Texture*>(ablock(H_MENU_BAR));
	renderer->blit(*tex, 0, 0, tex->palette());
	tex = dynamic_cast<const Texture*>(ablock(H_ANIM));
	renderer->blit(*tex, 0, 56, tex->palette());

	if (open) {
		effect_show(NULL);
	} else {
		showview(0, 0, 0, 0);
	}

	change_click_map(clk_main_menu, CLK_MAIN_MENU);
	send_message(E_ADD, E_TIMER, prehraj_animaci_v_menu);
	send_message(E_ADD, E_KEYBOARD, klavesnice);
	ms_last_event.event_type = 0x1;
	send_message(E_MOUSE, &ms_last_event);
	Task_WaitEvent(E_MENU_SELECT);
	c = cur_click;
	disable_click_map();
	send_message(E_DONE, E_KEYBOARD, klavesnice);
	cur_dir[c] = UNSELECT;

	while (cur_pos[c]) {
		Task_WaitEvent(E_TIMER);
	}

	Task_WaitEvent(E_TIMER);
	send_message(E_DONE, E_TIMER, prehraj_animaci_v_menu);
	return c;
}

typedef struct line_s {
	int x, y, height, mode, font;
	uint8_t color[4], color2[4];
	char *text;
	struct line_s *next;
} line_t;

line_t *load_titles(const char *filename) {
	SeekableReadStream *titles = NULL;
	line_t head = {0}, tpl = {0}, *ptr;
	char buf[81], *c;
	unsigned minheight = 0;
	const Font *font;

	titles = enc_open(Sys_FullPath(SR_MAP, filename));

	if (!titles) {
		titles = enc_open(Sys_FullPath(SR_DATA, filename));
	}

	if (!titles) {
		fprintf(stderr, "Could not open file %s\n", filename);
		fprintf(stderr, "Search path: %s; ", Sys_FullPath(SR_MAP, ""));
		fprintf(stderr, "%s\n", Sys_FullPath(SR_DATA, ""));
		exit(1);
	}

	speedscroll = 4;
	ptr = &head;
	tpl.x = 320;
	tpl.mode = TITLE_CENTER;
	tpl.font = H_FBIG;
	tpl.color[0] = 1;
	tpl.color[1] = 158;
	tpl.color[2] = 210;
	tpl.color[3] = 25;
	tpl.color2[0] = 1;
	tpl.color2[1] = 0;
	tpl.color2[2] = 0;
	tpl.color2[3] = 0;

	do {
		titles->readLine(buf, 80);
		c = strchr(buf, '\n');

		if (c) {
			*c = '\0';
		}

		c = strchr(buf, 0x1a);

		if (c) {
			*c = '\0';
		}

		if (buf[0] == '*') {
			strupr(buf);

			if (!strcmp(buf + 1, "HEAD")) {
				tpl.x = 50;
				tpl.mode = TITLE_HEAD;
				tpl.color[0] = 1;
				tpl.color[1] = 146;
				tpl.color[2] = 187;
				tpl.color[3] = 142;
				tpl.color2[0] = 0;
				tpl.color2[1] = 0;
				tpl.color2[2] = 0;
				tpl.color2[3] = 8;
			} else if (!strcmp(buf + 1, "NAME")) {
				tpl.x = 100;
				tpl.mode = TITLE_NAME;
				tpl.color[0] = 1;
				tpl.color[1] = 186;
				tpl.color[2] = 227;
				tpl.color[3] = 182;
				tpl.color2[0] = 0;
				tpl.color2[1] = 0;
				tpl.color2[2] = 0;
				tpl.color2[3] = 8;
			} else if (!strcmp(buf + 1, "CENTER")) {
				tpl.x = 320;
				tpl.mode = TITLE_CENTER;
				tpl.color[0] = 1;
				tpl.color[1] = 255;
				tpl.color[2] = 248;
				tpl.color[3] = 240;
				tpl.color2[0] = 0;
				tpl.color2[1] = 0;
				tpl.color2[2] = 0;
				tpl.color2[3] = 8;
			} else if (!strcmp(buf + 1, "TEXT")) {
				tpl.x = 50;
				tpl.mode = TITLE_TEXT;
				tpl.color[0] = 1;
				tpl.color[1] = 255;
				tpl.color[2] = 248;
				tpl.color[3] = 240;
				tpl.color2[0] = 0;
				tpl.color2[1] = 0;
				tpl.color2[2] = 0;
				tpl.color2[3] = 8;
			} else if (!strcmp(buf + 1, "KONEC")) {
				break;
			} else if (!strncmp(buf + 1, "LINE", 4)) {
				sscanf(buf + 5, "%d", &minheight);
			} else if (!strncmp(buf + 1, "SMALL", 5)) {
				tpl.font = H_FBOLD;
			} else if (!strncmp(buf + 1, "BIG", 3)) {
				tpl.font = H_FBIG;
			} else if (!strncmp(buf + 1, "SPEED", 5)) {
				// FIXME: Does speed change during slide?
				sscanf(buf + 6, "%d", &speedscroll);
			}
		} else {
			ptr->next = new line_t(tpl);
			ptr->next->y = ptr->y + (ptr->height < minheight ? minheight : ptr->height);
			ptr = ptr->next;
			ptr->text = new char[strlen(buf) + 1];
			strcpy(ptr->text, buf);
			font = dynamic_cast<const Font*>(ablock(ptr->font));
			assert(font && "Invalid font");
			ptr->height = font->textHeight(ptr->text);

			if (!ptr->height) {
				ptr->height = font->textHeight("W");
			}
		}
	} while (!titles->eos());

	return head.next;
}

void titles(int send_back, const char *filename) {
	int counter, newc, y, skip;
	line_t *lines, *ptr;
	const Font *font;
	const Texture *tex;

	lines = load_titles(filename);
	def_handle(H_PICTURE, "titulky.pcx", pcx_15bit_decomp, SR_BGRAFIKA);
	tex = dynamic_cast<const Texture*>(ablock(H_PICTURE));
	SubTexture top(*tex, 0, 0, tex->width(), 60);
	SubTexture mid(*tex, 0, 60, tex->width(), 360);
	SubTexture bott(*tex, 0, 420, tex->width(), 60);
	schovej_mysku();
	effect_show(NULL);
	newc = counter = Timer_GetValue();
	y = 420;

	do {
		counter = Timer_GetValue();
		skip = (counter - newc) / speedscroll;

		if (skip > 10) {
			skip = 10;
		}

		y -= skip;
		newc += skip * speedscroll;

		while (lines && (lines->y + lines->height + y < 0)) {
			ptr = lines->next;
			delete lines;
			lines = ptr;
		}

		renderer->blit(mid, 0, 60, mid.palette());

		for (ptr = lines; ptr && (ptr->y + y < 480); ptr = ptr->next) {
			font = dynamic_cast<const Font*>(ablock(ptr->font));
			renderer->setFont(font, ptr->color[0], ptr->color[1], ptr->color[2], ptr->color[3]);
			renderer->setFontColor(ptr->color2[0], ptr->color2[1], ptr->color2[2], ptr->color2[3]);

			switch (ptr->mode) {
			case TITLE_TEXT:
			case TITLE_HEAD:
				renderer->drawText(ptr->x, y + ptr->y, ptr->text);
				break;

			case TITLE_NAME:
				renderer->drawAlignedText(ptr->x, y + ptr->y, HALIGN_LEFT, VALIGN_TOP, ptr->text);
				break;

			case TITLE_CENTER:
				renderer->drawAlignedText(ptr->x, y + ptr->y, HALIGN_CENTER, VALIGN_TOP, ptr->text);
				break;

			default:
				assert(0 && "case not implemented");
			}
		}

		renderer->blit(top, 0, 0, top.palette());
		renderer->blit(bott, 0, 420, bott.palette());
		showview(0, 0, renderer->width(), renderer->height());

		// quit on keypress
		if (Input_Kbhit()) {
			Input_ReadKey();
			send_back = 1;
			break;
		}

		ShareCPU();
	} while (lines);

	ukaz_mysku();

	if (!send_back) {
		Task_WaitEvent(E_KEYBOARD);
	}
}

void run_titles(void)
  {
/*
  int task_id;
  task_id=Task_Add(8196,titles,1,"titulky.TXT");
  Task_WaitEvent(E_KEYBOARD);
  Task_Term(task_id);
*/
	titles(1, "TITULKY.TXT");
  }

void konec_hry() {
	int task_id;
	int timer;
	char *d;

	schovej_mysku();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	effect_show(NULL);
	create_playlist(texty[205]);
	play_next_music(&d);
	Sound_ChangeMusic(d);
	timer = Timer_GetValue();

	while (Timer_GetValue() - timer < 150) {
		Task_Sleep(NULL);
	}
/*
  task_id=Task_Add(8196,titles,1,"ENDTEXT.TXT");
  Task_WaitEvent(E_KEYBOARD);
  if (Task_IsRunning(task_id)) Task_Term(task_id);
*/
	titles(1, "ENDTEXT.TXT");
	Task_WaitEvent(E_TIMER);
	Task_WaitEvent(E_TIMER);
/*
  task_id=Task_Add(8196,titles,0,"TITULKY.TXT");
  Task_WaitEvent(E_KEYBOARD);
  if (Task_IsRunning(task_id)) Task_Term(task_id);
*/
	titles(0, "TITULKY.TXT");
	Sound_ChangeMusic("?");
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	ukaz_mysku();
	effect_show(NULL);
	timer = Timer_GetValue();

	while (Timer_GetValue() - timer < 150) {
		Task_Sleep(NULL);
	}
}

