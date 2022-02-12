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
#include <cmath>
#include <cstring>
#include <cassert>
#include <inttypes.h>
#include "libs/event.h"
#include "libs/memman.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/strlite.h"
#include "game/engine1.h"
#include "libs/pcx.h"
#include "game/globals.h"
#include "game/version.h"
#include "libs/system.h"

#define ZIVOTY_S 60
#define ZIVOTY_E 62
#define KONDIC_S 64
#define KONDIC_E 66
#define MANA_S   68
#define MANA_E   70
#define BARS_S   13
#define BARS_E   85
#define BARS_YS  (BARS_E-BARS_S)
#define PIC_X   3
#define PIC_Y   12
#define SEL_COLOR 0,255,255,255

#define ZASAHB_X 8
#define ZASAHB_Y 20
#define ZASAHT_X 30
#define ZASAHT_Y 38


#define SHOW {swap_buffs();showview(0,0,0,0);swap_buffs();getche();`}

#define HUMAN_ADJUST 97

/*
uint16_t barvy_skupin[POCET_POSTAV+1]=
         {
         RGB555(8,8,8),
         RGB555(31,28,00),
         RGB555(00,23,06),
         RGB555(31,11,13),
         RGB555(22,16,31),
         RGB555(28,13,31),
         RGB555(00,29,26)
         };
*/
uint8_t barvy_skupin[POCET_POSTAV + 1][3] = {
	{66,66,66},
	{255,231,0},
	{0,189,49},
	{255,90,107},
	{181,132,255},
	{231,107,255},
	{0,139,214}
};

char reverse_draw=0;
int viewsector=1,viewdir=1;
char norefresh=0,cancel_render=0,map_state=0;
int cur_sector; //sektor aktualni pozice
uint8_t back_color[3];
uint8_t global_anim_counter=0;
char set_halucination=0;
int hal_sector;
int hal_dir;
char see_monster=0;
char lodka=0;
int bgr_distance=0; //vzdalenost pozadi od pohledu
int bgr_handle=0;

int spell_handle=0,spell_phase,spell_xicht;

THE_TIMER *bott_timer=NULL;
char *bott_text=NULL;
char bott_display;
char true_seeing=0;
char obl_anim_counter=0;
char obl_max_anim=1;
char anim_mirror=0;
static char showrune=0;
static int showruneitem=0;



char dirs[10];
uint16_t minimap[VIEW3D_Z+1][VIEW3D_X*2+1];



//debug - !!!!

int dhit=0;
int ddef=0;
int ddostal=0;
int dlives=0;
int dmzhit=0;
int dsee=0;
char show_debug=0;
const char *debug_text;
char marker=0;

static void draw_spectxtrs(uint16_t sector, int celx, int cely) {
	int i;
	char a = 1;

	if (!(gameMap.coord()[sector].flags & MC_SPECTXTR)) {
		return;
	}

	for (i = 0; i < MAX_SPECTXTRS; i++) {
		if (gameMap.spectxtr()[i].repeat && gameMap.spectxtr()[i].sector == sector) {
			const SPECTXTR *sp = gameMap.spectxtr() + i;
			const Texture *tex;

			tex = dynamic_cast<const Texture*>(ablock(sp->handle + sp->pos));
			draw_spectxtr(*tex, celx, cely, sp->xpos);
			a = 0;
		}
	}

	if (a) {
		gameMap.clearCoordFlags(sector, MC_SPECTXTR);
	}
}

void ukaz_kompas(unsigned char mode) {
	static int phase = 0, speed = 0;
	int direct;

	if (mode == 1) {
		direct = (3 - viewdir);
		direct = phase - direct * 10;

		if (direct < -20) {
			direct += 40;
		}

		if (direct >= 20) {
			direct -= 40;
		}

		speed = ((direct > 0) - (direct < 0));
	}

	if (mode != 255) {
		const Texture *tex = dynamic_cast<const Texture*>(ablock(H_KOMPAS));
		renderer->rectBlit(*tex, 476, 0, 0, phase * 16, 102, 16, tex->palette());
	}

	if (mode == 1) {
		phase -= speed;

		if (phase > 39) {
			phase=0;
		}

		if (phase < 0) {
			phase = 39;
		}
	}

	if (mode == 255) {
		showview(476, 0, 102, 15);
	}
}

void show_money() {
	char c[20];
	const Font *font = dynamic_cast<const Font*>(ablock(H_FONT7));

	renderer->setFont(font, 1, 231, 231, 173);
	sprintf(c, "%ld", money);
	renderer->drawAlignedText(460, 13, HALIGN_RIGHT, VALIGN_BOTTOM, c);
}


void anim_sipky(int h, int mode) {
	static int phase = 0;
	static char drw = 0;
	static short handle = 0;
	const Texture *tex;

	if (mode == 1) {
		handle = h;
		phase = 6;
		return;
	}

	if (phase && mode != 255) {
		int i;

		if (phase > 4) {
			i = 8 - phase;
		} else {
			i = phase;
		}

		i = (i - 2) * 110;

		if (i >= 0) {
			tex = dynamic_cast<const Texture*>(ablock(handle));
			renderer->rectBlit(*tex, 498, 378, 0, i, 142, 102, tex->palette());
		} else {
			tex = dynamic_cast<const Texture*>(ablock(H_SIPKY_END));
			renderer->blit(*tex, 498, 378, tex->palette());
		}

		drw = 1;

		if (mode != -1) {
			phase--;
		}
	}

	if (mode == 255 && drw) {
		showview(498, 378, 142, 102);
		drw = 0;
	}
}

void chveni(int i) {
	static int pos = 0;
	static int count = 0;

	if (!count && !i) {
		return;
	}

	if (i) {
		count = i;
		pos = 1;
	}

	count--;

	if (!count) {
		pos = 0;
	}

	wait_retrace();
	renderer->xshift(8 * pos);
	renderer->drawRect(0, 0, renderer->width(), renderer->height());
	pos = -pos;
}

void objekty_mimo(the_timer *arg) {
	ukaz_kompas(1);
	anim_sipky(0, 0);
	ukaz_kompas(255);
	anim_sipky(0, 255);
	chveni(0);
}

void draw_blood(char mode, int mob_dostal, int mob_dostal_pocet) {
	static int phase = 100;
	static int block;
	static int dostal;
	char s[20];
	uint16_t *adr;
	int i;
	const Texture *tex;
	const Font *font;

	mob_dostal_pocet;

	if (mode) {
		if (phase < 3) {
			dostal += mob_dostal_pocet;
		} else {
			dostal = mob_dostal_pocet;
		}

		phase = 0;
		block = mob_dostal;
		return;
	}

	if (phase > 7) {
		return;
	}

	i = 105 * phase;
	phase++;
	tex = dynamic_cast<const Texture*>(ablock(H_KREVMIN + block - 1));
	renderer->rectBlit(*tex, 520, 378, 0, i, 120, 102, tex->palette());
//	itoa(dostal,s,10);
	font = dynamic_cast<const Font*>(ablock(H_FTINY));
	renderer->setFont(font, 1, 255, 255, 255);
	sprintf(s, "%d", dostal);
	renderer->drawAlignedText(60 + 520, 51 + 378, HALIGN_CENTER, VALIGN_CENTER, s);
}

static void draw_small_icone(int num, int x, int y) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_POSTUP));

	put_textured_bar(*tex, x + num * 13, y, 13, 13, num * 13, 0);
}

static DataBlock *bott_fletna_normal(void) {
	uint16_t *bott_scr;
	int i,x;
	SoftRenderer *backrend;
	const Texture *tex;
	const Font *font;

	backrend = new SoftRenderer(renderer->width(), 104);
	tex = dynamic_cast<const Texture*>(ablock(H_FLETNA_BAR));
	backrend->blit(*tex, 0, 0, tex->palette());
	font = dynamic_cast<const Font*>(ablock(H_FTINY));
	backrend->setFont(font, 1, 255, 255, 0);
	backrend->drawAlignedText(103, 52, HALIGN_CENTER, VALIGN_CENTER, texty[174]);

	for (i = 0, x = 156; i < 12; i++, x += 22) {
		backrend->drawAlignedText(x, 32, HALIGN_CENTER, VALIGN_CENTER, texty[180 + i]);
	}

	return backrend;
}

static DataBlock *bott_draw_normal(void) {
	int i, j;
	int x, xs = 0, y;
	uint16_t *bott_scr, *ptr;
	THUMAN *p;
	SoftRenderer *backrend, *tmp;
	const Texture *tex;
	const Font *font;
	backrend = new SoftRenderer(renderer->width(), 104);

	// FIXME: rewrite directly
	tmp = renderer;
	renderer = backrend;

	if (battle && cur_mode == MD_INBATTLE) {
		tex = dynamic_cast<const Texture*>(ablock(H_BATTLE_BAR));
		backrend->blit(*tex, 0, 0, tex->palette());
	} else {
		tex = dynamic_cast<const Texture*>(ablock(H_DESK));
		backrend->blit(*tex, 0, 0, tex->palette());
	}

	tex = dynamic_cast<const Texture*>(ablock(H_OKNO));
	xs = tex->width();
	x = 54;

	for (j = 0; j < POCET_POSTAV; j++) {
		if ((p = &postavy[i = group_sort[j]])->used) {
			if (cur_mode != MD_PRESUN || i == moving_player) {
				char c[] = " ";
				int z, lv, llv;

				tex = dynamic_cast<const Texture*>(ablock(H_OKNO));
				backrend->blit(*tex, x, 0, tex->palette());
				lv = p->lives;
				llv = p->vlastnosti[VLS_MAXHIT];

				if (lv || p->used & 0x80) {
					z = 3 - ((lv - 1) * 4 / llv);

					if (lv == llv) {
						z = 0;
					}

					z *= 75;

					if (p->xicht >= 0) {
						tex = dynamic_cast<const Texture*>(ablock(H_XICHTY+i));
						if (tex) {
							backrend->rectBlit(*tex, PIC_X + x, PIC_Y, 0, z, 54, 75, tex->palette());
						}
					}

					if (p->bonus) {
						draw_small_icone(0, PIC_X + x + 1, PIC_Y + 1);
					}

					if (p->spell) {
						draw_small_icone(1, PIC_X + x + 1, PIC_Y + 1);
					}

					if (!p->voda) {
						draw_small_icone(2, PIC_X + x + 1, PIC_Y + 1);
					}

					if (!p->jidlo) {
						draw_small_icone(3, PIC_X + x + 1, PIC_Y + 1);
					}
				} else {
					tex = dynamic_cast<const Texture*>(ablock(H_LEBKA));
					backrend->blit(*tex, PIC_X + x, PIC_Y, tex->palette());
				}

				memset(curcolor, 0, 3 * sizeof(uint8_t));
				y = BARS_YS - p->lives * BARS_YS / p->vlastnosti[VLS_MAXHIT];

				if (y) {
					bar(x + ZIVOTY_S, BARS_S, x + ZIVOTY_E, BARS_S + y);
				}

				y = BARS_YS - p->kondice * BARS_YS / p->vlastnosti[VLS_KONDIC];

				if (y) {
					bar(x + KONDIC_S, BARS_S, x + KONDIC_E, BARS_S + y);
				}

				if (p->vlastnosti[VLS_MAXMANA]) {
					y = BARS_YS - p->mana * BARS_YS / p->vlastnosti[VLS_MAXMANA];
				} else {
					y = BARS_YS;
				}

				if (y < 0) {
					y = 0;
				}

				if (y) {
					bar(x + MANA_S, BARS_S, x + MANA_E, BARS_S + y);
				}

				if (p->sektor != viewsector) {
					trans_bar25(x, 0, 74, 102);
				}

				font = dynamic_cast<const Font*>(ablock(H_FLITT));

				if (p->groupnum == cur_group && !battle) {
					renderer->setFont(font, SEL_COLOR);
				} else {
					renderer->setFont(font, 1, barvy_skupin[p->groupnum][0], barvy_skupin[p->groupnum][1], barvy_skupin[p->groupnum][2]);
				}

				renderer->drawAlignedText(x + 36, 92, HALIGN_CENTER, VALIGN_TOP, p->jmeno);
				c[0] = p->groupnum + 48;
				renderer->drawAlignedText(x + 5, 86, HALIGN_LEFT, VALIGN_BOTTOM, c);

				if (cur_mode == MD_INBATTLE) {
					char s[20];
					signed char dir;

					font = dynamic_cast<const Font*>(ablock(H_FBOLD));
					renderer->setFont(font, 1, 255, 255, 0);
					sprintf(s, texty[40], p->actions);
					renderer->drawAlignedText(x + 56, 86, HALIGN_RIGHT, VALIGN_BOTTOM, s);
					dir = viewdir - p->direction;

					if (abs(dir) == 2) {
						c[0] = 2;
					} else if (dir == -1 || dir == 3) {
						c[0] = 1;
					} else if (dir == -3 || dir == 1) {
						c[0] = 3;
					}

					if (dir) {
						font = dynamic_cast<const Font*>(ablock(H_FSYMB));
						if (p->groupnum == cur_group && !battle) {
							renderer->setFont(font, SEL_COLOR);
						} else {
							renderer->setFont(font, 1, barvy_skupin[p->groupnum][0], barvy_skupin[p->groupnum][1], barvy_skupin[p->groupnum][2]);
						}

						c[0] += 4;
						renderer->drawAlignedText(x + 10, 86, HALIGN_LEFT, VALIGN_BOTTOM, c);
					}
				}

				if (i == select_player) {
					rectangle(x + 3, 12, x + 3 + 54, 12 + 75, 255, 255, 255);
				}

				if (p->dostal) {
					char s[20];

					tex = dynamic_cast<const Texture*>(ablock(H_PZASAH));
					backrend->blit(*tex, x + ZASAHB_X, ZASAHB_Y, tex->palette());
					font = dynamic_cast<const Font*>(ablock(H_FBOLD));
					renderer->setFont(font, 1, 255, 255, 0);
					sprintf(s,"%d", p->dostal);
					renderer->drawAlignedText(x + ZASAHT_X, ZASAHT_Y, HALIGN_CENTER, VALIGN_CENTER, s);
				}

				x += xs;
			}
		}
	}

	if (cur_mode == MD_PRESUN || cur_mode == MD_UTEK) {
		char s[40];

		font = dynamic_cast<const Font*>(ablock(H_FBOLD));
		renderer->setFont(font, 1, 255, 255, 144);
		renderer->drawText(150, 20, texty[cur_mode == MD_PRESUN ? 42 : 44]);
		sprintf(s, texty[60 + cislovka(postavy[moving_player].actions)], postavy[moving_player].actions);
		renderer->drawText(150, 35, s);
		renderer->drawText(150, 50, texty[63]);
	}
/*  if (mob_dostal)
        {
        uint16_t *w;
        char s[40];

        w=ablock(H_MZASAH1+mob_dostal-1);
        put_picture(580-(w[0]>>1),55-(w[1]>>1),w);
        itoa(mob_dostal_pocet,s,10);
        set_font(H_FLITT5,0xffff);
        set_aligned_position(580,55,1,1,s);
        outtext(s);
        }
 */
	renderer = tmp;
	return backrend;
}


void bott_timer_draw(struct the_timer *q)
  {
  q;
  bott_display=BOTT_NORMAL;
  zneplatnit_block(H_BOTTBAR);
  free(bott_text);
  bott_text=NULL;
  bott_timer=NULL;
  }

DataBlock *bott_disp_text_proc(void) {
	char *p, *text;
	int y = 20;
	int xx, yy;
	SoftRenderer *backrend, *tmp;
	const Texture *tex;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FBOLD));

	backrend = new SoftRenderer(renderer->width(), 104);
	// FIXME: rewrite directly
	tmp = renderer;
	renderer = backrend;

	text = (char*)alloca(strlen(bott_text) + 2);
	renderer->setFont(font, 0, 0, 0, 0);
	zalamovani(bott_text, text, 390, &xx, &yy);

	if (battle && cur_mode == MD_INBATTLE) {
		tex = dynamic_cast<const Texture*>(ablock(H_BATTLE_BAR));
		backrend->blit(*tex, 0, 0, tex->palette());
	} else {
		tex = dynamic_cast<const Texture*>(ablock(H_DESK));
		backrend->blit(*tex, 0, 0, tex->palette());
	}

	create_frame(70, 20, 400, 50, 1);
	p = text;

	do {
		renderer->drawText(70, y, p);
		y += renderer->textHeight(p);
		p = strchr(p, 0) + 1;
	} while (p[0]);

	renderer = tmp;
	return backrend;
}

void bott_disp_text(const char *text)
  {
  if (text==0) text="Chybi popisek!!";
  zneplatnit_block(H_BOTTBAR);
  if (bott_timer==NULL)
     bott_timer=add_to_timer(TM_BOTT_MESSAGE,MSG_DELAY,1,bott_timer_draw);
     else bott_timer->counter=MSG_DELAY;
  bott_display=BOTT_TEXT;
  if (bott_text!=NULL) free(bott_text);
  bott_text=(char *)getmem(strlen(text)+1);
  strcpy(bott_text,text);
  }

DataBlock *bott_draw_rune(void) {
	int sel_zivel = showrune / 10;
	int sel_rune = showrune % 10;
	int maskrune = runes[sel_zivel];
	int i;
	char buff[300];
	int spell = (sel_zivel * 7 + sel_rune) * 3;
	SoftRenderer *backrend, *tmp;
	const Texture *tex;
	const Font *font;

	backrend = new SoftRenderer(renderer->width(), 104);

	// FIXME: rewrite directly
	tmp = renderer;
	renderer = backrend;

	if (battle && cur_mode == MD_INBATTLE) {
		tex = dynamic_cast<const Texture*>(ablock(H_BATTLE_BAR));
		backrend->blit(*tex, 0, 0, tex->palette());
	} else {
		tex = dynamic_cast<const Texture*>(ablock(H_DESK));
		backrend->blit(*tex, 0, 0, tex->palette());
	}

	create_frame(70, 20, 280, 50, 1);
	tex = dynamic_cast<const Texture*>(ablock(H_RUNEBAR1 + sel_zivel));
	backrend->blit(*tex, 378, 0, tex->palette());

	for (i = 0; i < 7; i++) {
		if (!(maskrune & (1 << i))) {
			tex = dynamic_cast<const Texture*>(ablock(H_RUNEMASK));
			backrend->maskFill(*tex, 378, 0, i + 6, 0, 0, 0, 0);
		} else if (i != sel_rune) {
			tex = dynamic_cast<const Texture*>(ablock(H_RUNEMASK));
			backrend->maskFill(*tex, 378, 0, i + 6, 1, 0, 0, 0);
		}
	}

	if (sel_zivel) {
		trans_bar(378, 0, sel_zivel * 24, 22, 0, 0, 0);
	}

	if (sel_zivel != 4) {
		trans_bar(378 + 24 + sel_zivel * 24, 0, 96 - sel_zivel * 24, 22, 0, 0, 0);
	}

	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 0, 0, 0, 0);
	renderer->drawText(120, 30, glob_items[showruneitem].jmeno);  
	sprintf(buff, "%s %d, %s %d", texty[11], get_spell_um(spell), texty[16], get_spell_mana(spell));
	renderer->drawText(75, 60, buff);
	tex = dynamic_cast<const Texture*>(ablock(glob_items[showruneitem].vzhled + face_arr[0]));
	backrend->blit(*tex, 70, 30, tex->palette());
	renderer = tmp;
	return backrend;
}

void bott_disp_rune(char rune, int item)
  {
  showrune=rune;
  showruneitem=item;
  bott_display=BOTT_RUNA;
  zneplatnit_block(H_BOTTBAR);
  if (bott_timer==NULL)
     bott_timer=add_to_timer(TM_BOTT_MESSAGE,MSG_DELAY,1,bott_timer_draw);
     else bott_timer->counter=MSG_DELAY;

  }

void bott_text_forever()
  {
  delete_from_timer(TM_BOTT_MESSAGE);
  bott_timer=NULL;
  }

DataBlock *bott_draw_proc(SeekableReadStream &stream) {
	switch (bott_display) {
	case BOTT_NORMAL:
		return bott_draw_normal();

	case BOTT_TEXT:
		return bott_disp_text_proc();

	case BOTT_FLETNA:
		return bott_fletna_normal();

	case BOTT_RUNA:
		return bott_draw_rune();
	}
}

void bott_draw(char priority)
  {
  if (priority)
     {
     if (bott_display!=BOTT_NORMAL)
        {
        delete_from_timer(TM_BOTT_MESSAGE);
        bott_timer=NULL;
        bott_display=BOTT_NORMAL;
        }
     }
  if (bott_display==BOTT_NORMAL)
     zneplatnit_block(H_BOTTBAR);
  }

void bott_draw_fletna()
  {
  bott_display=BOTT_FLETNA;
     zneplatnit_block(H_BOTTBAR);
  }

void draw_spell(int handle, int phase, int xicht) {
	int x, y, i;
	const Texture *tex;

	if (bott_display != BOTT_NORMAL) {
		bott_draw(1);
	}

	for (i = 0; i < POCET_POSTAV; i++) {
		if (xicht & (1 << group_sort[i])) {
			tex = dynamic_cast<const Texture*>(ablock(H_OKNO));
			x = 54 + tex->width() * i + PIC_X;
			y = PIC_Y + 378;
			tex = dynamic_cast<const Texture*>(ablock(handle));
			renderer->rectBlit(*tex, x, y, 0, phase * 75, 54, 75, tex->palette());
			neco_v_pohybu = 1;
		}
	}
}

void other_draw() {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_BOTTBAR));

//  if (cancel_render) return;
//  memcpy(Screen_GetAddr()+(480-102)*Screen_GetXSize(),ablock(H_BOTTBAR),Screen_GetXSize()*102*2);
	renderer->blit(*tex, 0, 480 - 102, tex->palette());

	if (spell_handle) {
		draw_spell(spell_handle, spell_phase, spell_xicht);
		spell_phase++;

		if (spell_phase >= 10) {
			spell_handle = 0;
			spell_xicht = 0;
		}
	}

	tex = dynamic_cast<const Texture*>(ablock(H_TOPBAR));
	renderer->blit(*tex, 0, 0, tex->palette());
	ukaz_kompas(0);
	show_money();
	anim_sipky(0, -1);
	draw_fx();
	renderer->bar(0, SCREEN_OFFLINE - 1, renderer->width(), 1, 0, 0, 0);
	renderer->bar(0, SCREEN_OFFLINE + 360, renderer->width(), 1, 0, 0, 0);
}

void display_spell_in_icone(int handle,int xicht)
  {
  spell_phase=0;
  spell_handle=handle;
  spell_xicht|=xicht;
  }

int fc_num(int anim_counter, int sector, char floor) {
	int mode;
	const TSECTOR *s;
	const TMAP_EDIT_INFO *m;
	int basic;

	s = gameMap.sectors() + sector;
	m = gameMap.coord() + cur_sector;

	if (floor) {
		basic = s->floor;
		mode = s->flags & 0xf;
	} else {
		basic = s->ceil;
		mode = s->flags >> 4;
	}

	if (mode & 0x8) {
		return basic + (anim_counter % ((mode & 0x7) + 1));
	}

	switch (mode) {
	case 0:
		return basic;

	case 1:
		return basic + ((m->x + m->y + dirs[1]) & 0x1);

	case 2:
		return basic + (dirs[1] & 0x1);

	case 3:
		return basic + (dirs[1] & 0x1) * 2 + ((m->x + m->y + (dirs[1] >> 1)) & 0x1);

	case 4: return
		basic + dirs[1];

	case 5:
		return basic + dirs[1] * 2 + ((m->x + m->y + (dirs[1] >> 1)) & 0x1);
	}

	return basic;
}

int enter_tab[VIEW3D_Z+1][VIEW3D_X*2+1]=
  {4,3,3,3,3,3,3,3,4,
   4,3,3,2,3,2,3,3,4,
   4,3,3,2,4,2,3,3,4,
   4,3,1,1,4,1,1,3,4,
   4,1,1,1,4,1,1,1,4};



void crt_minimap_itr(int sector, int smer, int itrx, int itry, int automap) {
	static int sector_temp;
	static long sideflags;
	static short enter = 0;
	short savee;

	if (itrx == VIEW3D_X && itry == 0) {
		enter = 0;
	}

	if (itrx < 0 || itrx > VIEW3D_X * 2 || itry > VIEW3D_Z) {
		return;
	}

	if (minimap[itry][itrx]) {
		return;
	}

	minimap[itry][itrx] = sector;

	if (!set_halucination) {
		gameMap.setCoordFlags(sector, automap & (itrx <= VIEW3D_X + 1 && itrx >= VIEW3D_X - 1));
	}

	if (itrx <= VIEW3D_X) {
		sector_temp = gameMap.sectors()[sector].step_next[dirs[0]];
		sideflags = gameMap.sides()[sector * 4 + dirs[0]].flags;

		if (sector_temp && sideflags & 0x80 && enter >= 0) {
			savee = enter;
			enter = -enter_tab[itry][itrx];
			crt_minimap_itr(sector_temp, smer, itrx - 1, itry, automap & (sideflags & 1));
			enter = savee;
		}
	}

	if (itrx >= VIEW3D_X) {
		sector_temp = gameMap.sectors()[sector].step_next[dirs[2]];
		sideflags = gameMap.sides()[sector * 4 + dirs[2]].flags;
		if (sector_temp && sideflags & 0x80 && enter <= 0) {
			savee = enter;
			enter = enter_tab[itry][itrx];
			crt_minimap_itr(sector_temp, smer, itrx + 1, itry, automap & (sideflags & 1));
			enter = savee;
		}
	}

	sector_temp = gameMap.sectors()[sector].step_next[dirs[1]];
	sideflags = gameMap.sides()[sector * 4 + dirs[1]].flags;

	if (sector_temp && sideflags & 0x80) {
		savee = enter;
		enter -= (enter > 0) - (enter < 0);
		crt_minimap_itr(sector_temp, smer, itrx, itry + 1, automap & (sideflags & 1));
		enter = savee;
	}
}


void create_minimap(int sector,int smer)
  {
  memset(minimap,0,sizeof(minimap));
  dirs[1]=smer;
  dirs[0]=(smer-1) & 3;
  dirs[2]=(smer+1) & 3;
  crt_minimap_itr(sector,smer,VIEW3D_X,0,1);
  }

static const float Inv2=0.5;
static const float Snapper=3<<22;

#define GET_OBLOUK(p) ((p->oblouk & 0xf)+(p->prim_anim>>4))

static int left_shiftup,right_shiftup;

int draw_basic_floor(int celx, int cely, int sector) {
	const TSECTOR *s;
	int dark;
	const Texture *tex;

	s = gameMap.sectors() + sector;
	dark = gameMap.coord()[sector].flags & MC_SHADING;

	if (s->floor) {
		tex = dynamic_cast<const Texture*>(ablock(num_ofsets[FLOOR_NUM] + fc_num(global_anim_counter, sector, 1)));
		draw_floor_ceil(celx, cely, 0, dark, tex);
	}

	if (s->ceil) {
		tex = dynamic_cast<const Texture*>(ablock(num_ofsets[CEIL_NUM] + fc_num(global_anim_counter, sector, 0)));
		draw_floor_ceil(celx, cely, 1, dark, tex);
	}

	return 0;
}

static int calc_item_shiftup(TITEM *it) {
	const uint8_t *c;
	int y, x, xs, ys, t;
	const Texture *tex;

	t = 0;
	tex = dynamic_cast<const Texture*>(ablock(it->vzhled + face_arr[0]));
	assert(tex->depth() == 1 && "Invalid item texture bit depth");
	xs = tex->width();
	ys = tex->height();
	c = tex->pixels() + xs * ys - 1;
	y = 0;

	while (y < ys && t < 254) {
		x = 0;

		while (x < xs) {
			if (*c != 0) {
				return t;
			} else {
				c--;
				x++;
			}
		}

		y++;
		t++;
	}

	return t;
}

void draw_vyklenek(int celx, int cely, int sector, int dir) {
	int i, j;
	const TVYKLENEK *v;
	const Texture *tex;
	
	for (i = 0; i < gameMap.vykCount(); i++) {
		if (gameMap.vyk()[i].sector == sector && gameMap.vyk()[i].dir == dir) {
			break;
		}
	}

	if (i == gameMap.vykCount()) {
		return;
	}

	v = gameMap.vyk() + i;

	for (i = 0; (j = v->items[i]) != 0; i++) {
		TITEM *it = &glob_items[j - 1];

		if (it->shiftup == 0xff) {
			it->shiftup = calc_item_shiftup(it);
		}

		tex = dynamic_cast<const Texture*>(ablock(it->vzhled + face_arr[0]));
		draw_item2(celx, cely, v->xpos, v->ypos - it->shiftup, tex, i);
	}
}


static int draw_basic_sector(int celx, int cely, int sector) {
	const TSTENA *w, *q;
	int obl;
	const Texture *tex;

	w = gameMap.sides() + sector * 4;
	q = w + dirs[1];
	obl = GET_OBLOUK(q);
	if (cely < VIEW3D_Z) {
		if (q->flags & SD_LEFT_ARC && obl) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[OBL_NUM] + obl));
			show_cel2(celx, cely, *tex, 0, 0, 1);
		}

		if (q->flags & SD_RIGHT_ARC && q->oblouk  & 0x0f) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[OBL2_NUM] + obl));
			show_cel2(celx, cely, *tex, 0, 0, 2);
		}

		if (q->flags & SD_PRIM_VIS && q->prim) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->prim + (q->prim_anim >> 4)));
			show_cel2(celx, cely, *tex, 0, 0, 1 + (q->oblouk & SD_POSITION));
		}

		if (q->flags & SD_SEC_VIS && q->sec) {
			if (q->side_tag & SD_SHIFTUP) {
				if (cely != 0) {
					tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->sec + (q->sec_anim >> 4)));
					show_cel2(celx, cely - 1, *tex, 0, 0, 1);
				}
			} else {
				tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->sec + (q->sec_anim >> 4)));
				show_cel2(celx, cely, *tex, q->xsec << 1, q->ysec << 1, 0);
			}
		}

		if (q->oblouk & 0x10) {
			draw_vyklenek(celx, cely, sector, dirs[1]);
		}
	}

	if (celx <= 0) {
		q = &w[dirs[0]];

		if (left_shiftup) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[LEFT_NUM] + left_shiftup));
			show_cel(celx, cely, *tex, 0, 0, 2);
			left_shiftup = 0;
		}

		if (q->flags & SD_PRIM_VIS && q->prim) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[LEFT_NUM] + q->prim + (q->prim_anim >> 4)));
			show_cel(-celx, cely, *tex, 0, 0, 2 + (q->oblouk & SD_POSITION));
		}

		if (q->flags & SD_SEC_VIS && q->sec) {
			if (q->side_tag & SD_SHIFTUP) {
				if (celx != 0) {
					left_shiftup = q->sec + (q->sec_anim >> 4);
				} else {
					left_shiftup = 0;
				}
			} else if (q->flags & SD_SPEC) {
				tex = dynamic_cast<const Texture*>(ablock(num_ofsets[LEFT_NUM] + q->sec + (q->sec_anim >> 4)));
				show_cel(celx, cely, *tex, 0, 0, 2);
			} else {
				tex = dynamic_cast<const Texture*>(ablock(num_ofsets[LEFT_NUM] + q->sec + (q->sec_anim >> 4)));
				show_cel(celx, cely, *tex, q->xsec << 1, q->ysec << 1, 0);
			}
		}
	}

	if (celx >= 0) {
		q = &w[dirs[2]];

		if (right_shiftup) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[RIGHT_NUM] + right_shiftup));
			show_cel(celx, cely, *tex, 0, 0, 3);
			right_shiftup = 0;
		}

		if (q->flags & SD_PRIM_VIS && q->prim) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[RIGHT_NUM] + q->prim + (q->prim_anim >> 4)));
			show_cel(celx, cely, *tex, 0, 0, 3 + (q->oblouk & SD_POSITION));
		}

		if (q->flags & SD_SEC_VIS && q->sec) {
			if (q->side_tag & SD_SHIFTUP) {
				if (celx != 0) {
					right_shiftup = q->sec + (q->sec_anim >> 4);
				} else {
					right_shiftup = 0;
				}
			} else if (q->flags & SD_SPEC) {
				tex = dynamic_cast<const Texture*>(ablock(num_ofsets[RIGHT_NUM] + q->sec + (q->sec_anim >> 4)));
				show_cel(celx, cely, *tex, 0, 0, 3);
			} else {
				tex = dynamic_cast<const Texture*>(ablock(num_ofsets[RIGHT_NUM] + q->sec + (q->sec_anim >> 4)));
				show_cel(celx, cely, *tex, 500 - (q->xsec << 1), q->ysec << 1, 1);
			}
		}
	}

	return 0;
}

static int draw_lodku(int celx, int cely) {
	const Texture *tex;

	if (cely == 0) {
		return 1;
	}

	tex = dynamic_cast<const Texture*>(ablock(H_LODKA0 + (global_anim_counter & 7)));
	show_cel2(celx, cely - 1, *tex, 250, 80, 0);
	return 0;
}


int p_place_table[4][5]=
     {
     {0,1,4,2,3},
     {1,2,4,3,0},
     {2,3,4,0,1},
     {3,0,4,1,2}
     };
int p_positions_x[5]={-32,32,32,-32,0};
int p_positions_y[5]={32,32,-32,-32,0};

void draw_players(int sector, int dir, int celx, int cely) {
	if (gameMap.coord()[sector].flags & MC_DPLAYER) {
		int i;
		THUMAN *p;
		char freep[5];
		int j, d, pp = 0, f;
		const Font *font;
		const Texture *tex;

		memset(freep, 0, sizeof(freep));

		for (i = 0, pp = 0; i < POCET_POSTAV; i++) {
			if ((p = &postavy[i])->sektor == sector && ((!p->lives && p->groupnum == 0) || p->sektor != viewsector)) {
				pp++;
				d = (p->direction - dir) & 0x3;

				if (p->lives) {
				        for (j = 0; j < 5 && freep[p_place_table[d][j]]; j++);
				} else {
				        for (j = 4; j >= 0 && freep[p_place_table[d][j]]; j--);
				}

				if (j == 5 || j == -1) {
					break;
				}

				freep[f = p_place_table[d][j]] = i + 1;
			}
		}

		if (pp == 1 && freep[f] & 1) {
			pp = f + 1 & 3;
			freep[pp] = freep[f];
			freep[f] = 0;
		}

		for (i = 0; i < 5; i++) {
			if ((j = freep[d = p_place_table[0][i]]) != 0) {
				if (postavy[j-1].lives) {
					uint8_t *color = barvy_skupin[postavy[j-1].groupnum];
					font = dynamic_cast<const Font*>(ablock(H_FLITT5));
					renderer->setFont(font, 0, color[0], color[1], color[2]);
					tex = dynamic_cast<const Texture*>(ablock(H_POSTAVY + j - 1));
					draw_player(*tex, celx, cely, p_positions_x[d], p_positions_y[d], HUMAN_ADJUST, postavy[j-1].jmeno);
				} else {
					tex = dynamic_cast<const Texture*>(ablock(H_KOSTRA));
					draw_player(*tex, celx, cely, p_positions_x[d], p_positions_y[d] - 32, HUMAN_ADJUST, NULL);
				}
			}
		}
	}
}



int draw_sloup_sector(int celx, int cely, int sector) {
	const TSTENA *w, *q;
	int obl;
	const Texture *tex;

	w = gameMap.sides() + sector * 4;
	q = w + dirs[1];
	obl = GET_OBLOUK(q);

	if (q->flags & SD_LEFT_ARC && q->oblouk) {
		tex = dynamic_cast<const Texture*>(ablock(num_ofsets[OBL_NUM] + obl));
		show_cel2(celx, cely, *tex, 0, 0, 1);
	}

	if (q->flags & SD_RIGHT_ARC && q->oblouk) {
		tex = dynamic_cast<const Texture*>(ablock(num_ofsets[OBL2_NUM] + obl));
		show_cel2(celx, cely, *tex, 0, 0, 2);
	}

	if (q->flags & SD_PRIM_VIS && q->prim) {
		tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->prim + (q->prim_anim >> 4)));
		show_cel2(celx, cely, *tex, 0, 0, 1 + (q->oblouk & SD_POSITION));
	}

	if (celx<=0) {
		q = &w[dirs[0]];
		if (q->flags & SD_PRIM_VIS && q->prim) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[LEFT_NUM] + q->prim + (q->prim_anim >> 4)));
			show_cel(-celx, cely, *tex, 0, 0, 2 + (q->oblouk & SD_POSITION));
		}
	}

	if (celx >= 0) {
		q = &w[dirs[2]];
		if (q->flags & SD_PRIM_VIS && q->prim) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[RIGHT_NUM] + q->prim + (q->prim_anim >> 4)));
			show_cel(celx, cely, *tex, 0, 0, 3 + (q->oblouk & SD_POSITION));
		}
	}

	q = &w[dirs[1]];

	if (q->flags & SD_SEC_VIS && q->sec && cely != 0) {
		if (q->flags & SD_SPEC) {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->sec + (q->sec_anim >> 4)));
			show_cel2(celx, cely - 1, *tex, 0, 0, 2);
		} else {
			tex = dynamic_cast<const Texture*>(ablock(num_ofsets[MAIN_NUM] + q->sec + (q->sec_anim >> 4)));
			show_cel2(celx, cely - 1, *tex, (q->xsec << 1) + celx * (points[0][0][cely].x - points[0][0][cely - 1].x) / 2, q->ysec << 1, 0);
		}
	}

	return 0;
}


void swap_truesee(int ss) {
	int i;

	for (i = 0; i < 4; i++) {

		if (gameMap.sides()[i + ss].flags & SD_TRUESEE) {
			uint16_t old = gameMap.sides()[i + ss].flags & (SD_PRIM_VIS | SD_SEC_VIS | SD_AUTOMAP);

			gameMap.clearSideFlags(ss + i, old);
			gameMap.setSideFlags(ss + i, old ^ (SD_PRIM_VIS | SD_SEC_VIS | SD_AUTOMAP));
		}
	}
}

void draw_sector(int celx, int cely, int s) {
	int ss;

	ss = s << 2;

	if (true_seeing) {
		swap_truesee(ss);
	}

	switch (gameMap.sectors()[s].sector_type) {
	case S_SSMRT:
	case S_SLOUP:
	case S_TELEPORT:
		draw_sloup_sector(celx, cely, s);
		draw_placed_items_normal(celx, cely, s, viewdir);
		break;

	case S_LODKA:
		draw_basic_sector(celx, cely, s);

		if (cely == 0) {
			draw_placed_items_normal(celx, cely, s, viewdir);
		}

		draw_lodku(celx, cely);
		break;

	default:
		draw_basic_sector(celx, cely, s);
		draw_placed_items_normal(celx, cely, s, viewdir);
		break;
	}

	if (true_seeing) {
		swap_truesee(ss);
	}
}

void back_clear(int celx, uint8_t r, uint8_t g, uint8_t b) {
	int x1, y1, x2, y2, xc;

	y1 = points[0][0][VIEW3D_Z].y + MIDDLE_Y + SCREEN_OFFLINE;
	y2 = points[0][1][VIEW3D_Z].y + MIDDLE_Y + SCREEN_OFFLINE;
	x2 = points[0][1][VIEW3D_Z].x + MIDDLE_X;
	x1 = -points[0][1][VIEW3D_Z].x + MIDDLE_X;
	xc = (x2 - x1 + 2) * celx;
	x1 = x1 + xc - 1;
	x2 = x2 + xc;

	if (x1 < 0) {
		x1 = 0;
	}

	if (x2 > 640) {
		x2 = 640;
	}

	if (x1 < x2) {
		renderer->bar(x1, y1 < y2 ? y1 : y2, x2 - x1 + 1, (y1 < y2 ? y2 - y1 : y1 - y2) + 1, r, g, b);
	}
}

static  char set_blind(void)
  {
  int i;

  if (battle && postavy[select_player].sektor==viewsector) if (postavy[select_player].vlastnosti[VLS_KOUZLA] & SPL_BLIND) return 1;else return 0;
  for(i=0;i<POCET_POSTAV;i++)
     {
     THUMAN *p=postavy+i;
     if (p->sektor==viewsector && !(p->vlastnosti[VLS_KOUZLA] & SPL_BLIND)) return 0;
     }
  return 1;
  }

extern char folow_mode;

/*
static __inline void zobraz_lodku(uint16_t *data,uint16_t *scr,int _size)
  {
  __asm
    {
    mov ecx,_size
    mov esi,data
    mov edi,scr
    add esi,6
lp1:lodsw
    or   ax,ax
    jz   skp
    mov  [edi],ax
skp: add  edi,2h
    dec  ecx
    jnz  lp1
    }
  }
*/

/* TODO: not used, remove?
static void trace_for_bgr(int dir) {
	int s, i;
	static int olddist = 0;
	static int olddir = 0;
	const TSTENA *ss;

	bgr_handle = 0;
	bgr_distance = -1;

	for (i = 0, s = 0; i < VIEW3D_Z && minimap[i][VIEW3D_X];) {
		s = minimap[i++][VIEW3D_X];
		ss = gameMap.sides() + s * 4 + dir;

		if (ss->flags & SD_PRIM_VIS && !ss->prim) {
			bgr_distance = i - 1;
			bgr_handle = num_ofsets[BACK_NUM] + dir;
			break;
		}
	}

	if (olddist != bgr_distance || olddir != dir) {
		if (bgr_distance == -1) {
			uint16_t *w = Screen_GetAddr();
			int i = Screen_GetXSize() * 480;

			do {
				if (*w >= NOSHADOW(0)) {
					*w = back_color;
				}

				w++;
			} while(--i);
		}

		zneplatnit_block(H_BGR_BUFF);
	}

	olddist = bgr_distance;
	olddir = dir;
}
*/


void render_scene(int sector, int smer) {
	int i, j, s;

	cancel_render = 0;
	see_monster = 0;
	destroy_fly_map();
	build_fly_map();

	if (set_blind() && cur_mode != MD_END_GAME && !folow_mode) {
		clear_buff(NULL, 0, 0, 0, 360);
		return;
	}

	if (set_halucination) {
		sector = hal_sector;
		smer = hal_dir;
	}

	cur_sector = sector;
	create_minimap(sector, smer);
//  trace_for_bgr(smer);
	i = VIEW3D_Z - 1;
	s = minimap[i][VIEW3D_X];

	if (s && !gameMap.sectors()[s].ceil) {
		clear_buff(NULL, back_color[0], back_color[1], back_color[2], 360);
	} else {
		clear_buff(NULL, back_color[0], back_color[1], back_color[2], 80);
	}

	for (i = -VIEW3D_X + 1; i < VIEW3D_X; i++) {
		if ((s = minimap[VIEW3D_Z-1][VIEW3D_X+i]) != 0) {
			if (gameMap.coord()[s].flags & MC_SHADING) {
				back_clear(i, 0, 0, 0);
			}
		}
	}

/*  if (reverse_draw)
  for(i=0;i<VIEW3D_Z;i++)
     for(j=0;j<VIEW3D_X;j++)
     {
     if (minimap[VIEW3D_X+j][i]) draw_sector(j,i,minimap[VIEW3D_X+j][i]);
     if (j && minimap[VIEW3D_X-j][i]) draw_sector(-j,i,minimap[VIEW3D_X-j][i]);
     do_events();
     if (cancel_render) return;
     }
  else*/
	for (i = VIEW3D_Z - 1; i >= 0; i--) {
		uint16_t *p;

		p = minimap[i];
		left_shiftup = 0;
		right_shiftup = 0;

		for (j = VIEW3D_X - 1; j >= 0; j--) {
			int s2;

			if ((s = p[VIEW3D_X + j]) != 0) {
				if (gameMap.coord()[s].flags & MC_SHADING) {
					secnd_shade = 1;
				} else {
					secnd_shade = 0;
				}

				draw_basic_floor(j, i, s);

				if (gameMap.sides()[(s << 2) + smer].flags & SD_TRANSPARENT) {
					s2 = gameMap.sectors()[s].step_next[smer];
					draw_mob(s2, smer, j, i, 1);
				}

				draw_sector(j, i, s);
			}

			if (j && (s = p[VIEW3D_X - j])) {
				if (gameMap.coord()[s].flags & MC_SHADING) {
					secnd_shade = 1;
				} else {
					secnd_shade = 0;
				}

				draw_basic_floor(-j, i, s);

				if (gameMap.sides()[(s << 2) + smer].flags & SD_TRANSPARENT) {
					s2 = gameMap.sectors()[s].step_next[smer];
					draw_mob(s2, smer, -j, i, 1);
				}

				draw_sector(-j,i,s);
			}
		}

		for (j = VIEW3D_X - 1; j >= 0; j--) {
			if ((s = p[VIEW3D_X + j]) != 0) {
				if (gameMap.coord()[s].flags & MC_SHADING) {
					secnd_shade = 1;
				} else {
					secnd_shade = 0;
				}

				draw_players(s, viewdir, j, i);
				draw_mob(s, viewdir, j, i, 0);
				draw_fly_items(j, i, s, viewdir);
				draw_spectxtrs(s, j, i);
			}

			if (j && (s = p[VIEW3D_X - j])) {
				if (gameMap.coord()[s].flags & MC_SHADING) {
					secnd_shade = 1;
				} else {
					secnd_shade = 0;
				}

				draw_players(s, viewdir, -j, i);
				draw_mob(s, viewdir, -j, i, 0);
				draw_fly_items(-j, i, s, viewdir);
				draw_spectxtrs(s, -j, i);
			}
		}

		// FIXME: solve cancel_render induced artifacts and uncomment
//		do_events();
		if (cancel_render) {
			return;
		}
	}

	gameMap.recalcSpecTextures();

	if (lodka) {
		const Texture *tex = dynamic_cast<const Texture*>(ablock(H_LODKA));
		renderer->blit(*tex, 0, LODKA_POS, tex->palette());
	}
}

void debug_print() {
	char s[256];
	static char indx = 50;
	static int counter = 0;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FBOLD));

	sprintf(s, "battle: %d waiting: %d lhit: %3d ldef: %3d dostal: %3d magic: %3d lives: %3d wpn: %d", battle, neco_v_pohybu, dhit, ddef, ddostal, dmzhit, dlives, vybrana_zbran);
	trans_bar(0, 17, 640, 15, 0, 0, 0);
	renderer->setFont(font, 0, 0, 123, 255);

	if (debug_text != NULL) {
		renderer->drawText(0, 17, debug_text);
		counter++;

		if (counter == 100) {
			counter = 0;
			debug_text = NULL;
		}
	} else {
		renderer->drawText(0, 17, s);
	}

	if (dsee) {
		indx--;
	}

	if (!indx) {
		dsee = 0;
		indx = 10;
	}
}

void redraw_scene() {
	if (norefresh) {
		return;
	}

	render_scene(viewsector, viewdir);

	if (cancel_render) {
		return;
	}

	if (anim_reader) {
		renderer->blit(*anim_reader, 0, SCREEN_OFFLINE, anim_reader->palette(), 640, anim_mirror);
	}

	update_mysky();

	if (battle || (game_extras & EX_ALWAYS_MINIMAP)) {
		draw_medium_map();
	}

	if (show_debug) {
		debug_print();
	}

	other_draw();
	global_anim_counter++;
	send_message(E_KOUZLO_ANM);
	// originally done by a task woken by the message above
	play_big_mgif_frame();
}

void refresh_scene(the_timer *arg)
  {
  redraw_scene();
  if (!cancel_render && !norefresh)
     {
     showview(0,0,0,0);
     calc_game();
	 if (autoopenaction==1) {go_book(0,0,0,0,0);autoopenaction=0;}
     }
  }

typedef struct fx_play
  {
  int32_t x,y,phase;
  struct fx_play *next;
  }FX_PLAY;

static FX_PLAY *fx_data=NULL;

void draw_fx() {
	FX_PLAY *fx;
	FX_PLAY **last;
	const Texture *tex;

	if (fx_data == NULL) {
		return;
	}

	fx = fx_data;
	last = &fx_data;
	tex = dynamic_cast<const Texture*>(ablock(H_FX));

	while (fx != NULL) {
		renderer->rectBlit(*tex, fx->x, fx->y, 0, fx->phase * 16, tex->width(), 16, tex->palette());

		if (++fx->phase > 14) {
			*last = NULL;
			free(fx);
			return;
		}

		last = &(fx->next);
		fx = *last;
	}
}


void play_fx(int x,int y)
  {
  FX_PLAY *fx;

  fx=New(FX_PLAY);
  fx->next=fx_data;
  fx->x=x;
  fx->y=y;
  fx->phase=0;
  fx_data=fx;
  }

void play_fx_at(int where)
  {
  static uint16_t polex[]={313,290,362};
  static uint16_t poley[]={1,1,1};

  play_fx(polex[where],poley[where]);
  }

void display_ver(int x, int y, int ax, int ay) {
	const char *ver = "Br ny Skeldalu version "VERSION" (C)1998";
	const Font *font = dynamic_cast<const Font*>(ablock(H_FTINY));

	renderer->setFont(font, 1, 255, 255, 255);
	renderer->drawAlignedText(x, y, ax, ay, ver);
	showview(0, 0, 0, 0);
}
