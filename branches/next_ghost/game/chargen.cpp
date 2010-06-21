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
//CHARACTER GENERATOR
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
#include "libs/sound.h"
#include "libs/gui.h"
#include "libs/basicobj.h"
#include "game/engine1.h"
#include "libs/pcx.h"
#include "game/globals.h"
#include <cstddef>

//there is defined procedures from source "INV.C"

void display_items_wearing(THUMAN *h);
void inv_display_vlastnosti();;
extern void (*inv_redraw)();
void write_human_big_name(char *c);

#define MSG_COLOR1 RGB555(30,30,23)


#define MAX_XICHTS 8
#define MAX_CHARS 6

#define XICHT_STEP 40

#define INV_DESK 266
#define INV_DESK_Y (SCREEN_OFFLINE)
#define INV_DESC_X 285
#define INV_DESC_Y (SCREEN_OFFLINE+20)
#define HUMAN_X 40
#define HUMAN_Y 328


#define X_SKIP_X 64
#define X_SKIP_Y 85

typedef struct staty
  {
  int32_t zivl;
  int32_t zivh;
  int32_t manl;
  int32_t manh;
  int32_t konl;
  int32_t konh;
  int32_t akc;
  }T_STATY;

static T_STATY cur_stats;

/*
uint16_t color_butt_on[]={0,RGB555(31,27,4),RGB555(30,26,4),RGB555(29,25,4)};
uint16_t color_butt_off[]={0,RGB555(10,10,10),RGB555(10,10,10),RGB555(10,10,10)};
*/
uint8_t color_butt_on[FONT_COLORS][3] = {
	{0,0,0},
	{255,222,33},
	{247,214,33},
	{239,206,33}
};

uint8_t color_butt_off[FONT_COLORS][3] = {
	{0,0,0},
	{82,82,82},
	{82,82,82},
	{82,82,82}
};

typedef struct vlasts
  {
  int32_t sill;
  int32_t silh;
  int32_t smgl;
  int32_t smgh;
  int32_t pohl;
  int32_t pohh;
  int32_t obrl;
  int32_t obrh;
  int32_t hpreg;
  int32_t mpreg;
  int32_t vpreg;
  }T_VLASTS;

static T_VLASTS cur_vls;
static T_VLASTS rohy[9]=
  {
  {17,22, 5,10,17,22, 9,14, 3, 2, 3},
  {13,18,10,15,20,25, 5,10, 3, 3, 2},
  { 9,14,15,20,17,22, 9,14, 3, 3, 2},
  { 5,10,20,25,13,18,13,18, 2, 4, 2},
  { 9,14,15,20, 9,14,17,22, 2, 3, 2},
  {13,18,10,15, 5,10,20,25, 2, 2, 4},
  {17,22, 5,10, 9,14,17,22, 3, 1, 2},
  {20,25, 0, 5,13,18,13,18, 4, 1, 2},
  {12,17,12,17,12,17,12,17, 2, 2, 2},
  };

#define CALC_DIFF(x,y,angl) ((x)+(((((y)-(x))<<4)*((angl)<<4)/720+8)>>4))
#define CALC_DIFF2(x,y,pol) ((x)+(((((y)-(x))<<4)*((pol)<<4)/(PERLA_MAXPOLOMER<<4)+8)>>4))
#define DIFF_RAND(x,y) ((x)+rnd((y)-(x)+1))

#define MAX_RACES 5
static char women[MAX_XICHTS]={0,0,0,0,1,1,1,1};
static char poradi[MAX_XICHTS]={0,2,3,4,1,5,6,7};
static char disable[MAX_XICHTS];
static int cur_edited=0;
static int cur_angle=90;
static int cur_polomer=0;
static int cur_xicht=-1;
static char shut_downing_text=0;
static int save_values[POCET_POSTAV][2];
static char del_mode=0;
static char was_enter=0;
static int close_ret = 0;

#define PERLA_STRED_X (INV_DESK+189)
#define PERLA_STRED_Y (INV_DESK_Y+217)


#define PERLA_POLOMER cur_polomer
#define PERLA_MAXPOLOMER 75
// FIXME: pod_perlou is never delete'd...
Texture *pod_perlou = NULL;
short pod_perlou_x=0;
short pod_perlou_y=0;
const char *error_text = NULL;



static char select_xicht(int id,int xa,int ya,int xr,int yr);
static char vol_vlastnosti(int id,int xa,int ya,int xr,int yr);
static char go_next_page(int id,int xa,int ya,int xr,int yr);
char vls_click(int id,int xa,int ya,int xr,int yr);
static char view_another_click2(int id,int xa,int ya,int xr,int yr);
//char edit_another_click(int id,int xa,int ya,int xr,int yr);
char edit_another_click2(int id,int xa,int ya,int xr,int yr);
char gen_exit_editor(int id,int xa,int ya,int xr,int yr);

static void zobraz_staty(T_VLASTS *st);

#define CLK_PAGE1 6


static T_CLK_MAP clk_page1[]=
  {
  {0,28+INV_DESK,18+INV_DESK_Y,334+INV_DESK,55+INV_DESK_Y,select_xicht,2,-1},
  {0,78+INV_DESK,97+INV_DESK_Y,314+INV_DESK,340+INV_DESK_Y,vol_vlastnosti,2+1,-1},
  {-1,520,378,639,479,go_next_page,2,-1},
  //{-1,54,378,497,479,edit_another_click,2+8,-1},
  {2,0,0,639,479,gen_exit_editor,8,-1},
  {0,0,0,639,479,empty_clk,0xff,-1},
  };

#define CLK_PAGE2 5

static T_CLK_MAP clk_page2[]=
  {
  {-1,INV_DESK,INV_DESK_Y,639,378,vls_click,2,-1},
  {-1,520,378,639,479,go_next_page,2,-1},
  {-1,54,378,497,479,view_another_click2,2,-1},
  {1,0,0,639,479,gen_exit_editor,8,-1},
  {0,0,0,639,479,empty_clk,0xff,-1},
  };
#define CLK_PAGE3 4

/*static T_CLK_MAP clk_page3[]=
  {
  {-1,520,378,639,479,go_next_page,2,-1},
  {-1,54,378,497,479,view_another_click2,2+8,-1},
  {0,0,0,639,479,edit_another_click2,8,-1},
  {0,0,0,639,479,empty_clk,0xff,-1},
  };
*/


#define H_GENERACE  (H_MENUS_FREE+100)
#define H_GEN_PERLA (H_MENUS_FREE+101)
#define H_GEN_TOPBAR (H_MENUS_FREE+102)
#define H_GEN_OKBUTT (H_MENUS_FREE+103)
#define H_GEN_CHARGEN (H_MENUS_FREE+104)
#define H_GEN_CHARGENB (H_MENUS_FREE+105)
#define H_GEN_CHARGENM (H_MENUS_FREE+106)
#define H_GEN_XICHTY (H_MENUS_FREE+107)
#define H_GEN_POSTAVY (H_GEN_XICHTY+MAX_XICHTS)

TDREGISTERS char_gen_reg[]=
    {
    {H_GENERACE,"postavy.pcx",pcx_15bit_decomp,SR_BGRAFIKA},
    {H_GEN_PERLA,"perla.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_GEN_TOPBAR,"topbar_p.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_GEN_CHARGEN,"chargen.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_GEN_CHARGENB,"chargenb.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_GEN_CHARGENM,"chargenm.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    };

#define BOTT 378
#define LEFT 520

const char *b_texty[4];
char b_disables;

void displ_button(char disable, const char **text) {
	int posy[] = {0, 18, 37, 55};
	int sizy[] = {18, 20, 20, 21};
	int i;
	const Texture *tex;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FTINY));

	renderer->setFont(font, 1, 0, 0, 0);
	tex = dynamic_cast<const Texture*>(ablock(H_GEN_CHARGEN));
	renderer->blit(*tex, LEFT, BOTT, tex->palette());

	for (i = 0; i < 4; i++) {
		if (disable & 1) {
			tex = dynamic_cast<const Texture*>(ablock(H_GEN_CHARGENB));
			renderer->rectBlit(*tex, 524, 392 + posy[i], 0, posy[i], 96, sizy[i], tex->palette());
			renderer->setFont(font, 1, color_butt_off);
		} else {
			renderer->setFont(font, 1, color_butt_on);
		}

		disable >>= 1;
		renderer->drawAlignedText(LEFT + 50, BOTT + 14 + 13 + i * 19, HALIGN_CENTER, VALIGN_BOTTOM, *text);
		text++;
	}
}

static void draw_other_bar() {
	int i;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_BOTTBAR));

	renderer->blit(*tex, 0, 480 - 102, tex->palette());

	//put_8bit_clipped(ablock(H_GEN_OKBUTT),378*640+520+screen,0,120,102);
	displ_button(b_disables, b_texty);
	tex = dynamic_cast<const Texture*>(ablock(H_GEN_TOPBAR));
	renderer->blit(*tex, 0, 0, tex->palette());
	zobraz_staty(&cur_vls);
}


static void display_character(THUMAN *p, char i) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_IOBLOUK));

	renderer->blit(*tex, 4, SCREEN_OFFLINE, tex->palette());

	if (p->used) {
		tex = dynamic_cast<const Texture*>(ablock(H_GEN_POSTAVY + p->xicht));
		renderer->blit(*tex, HUMAN_X + 30, HUMAN_Y - tex->height(), tex->palette());
	}

	if (i) {
		draw_other_bar();
	}
}

static void zobraz_error_text() {
	if (error_text != NULL) {
		const Font *font = dynamic_cast<const Font*>(ablock(H_FONT6));

		trans_bar(0, 362, 640, 16, 0, 0, 0);
		renderer->setFont(font, 1, 255, 255, 0);
		renderer->drawText(10, 364, error_text);
	}
}




static void vypocet_perly(int angle,int xp,int yp,int *x,int *y)
  {
  float xr,yr;
  float rad;

  rad=angle*3.14159265f/180;
  xr=(float)cos(rad)*PERLA_POLOMER;
  yr=(float)sin(rad)*PERLA_POLOMER;
  *x=(int)xr+PERLA_STRED_X;
  *y=PERLA_STRED_Y-(int)yr;
  x[0]-=xp>>1;
  y[0]-=yp>>1;
  }

static void zobraz_perlu(void) {
	const Texture *perla;
	int x, y;
	uint16_t *scr, *sss;
	uint8_t *p;
	uint16_t *b;
	int xs, ys, xxs;

	alock(H_GEN_PERLA);
	perla = dynamic_cast<const Texture*>(ablock(H_GEN_PERLA));
	xs = perla->width();
	ys = perla->height();

	vypocet_perly(cur_angle, xs, ys, &x, &y);
	delete pod_perlou;
	pod_perlou = new SubTexture(*renderer, x, y, xs, ys);
	renderer->transparentBlit(*perla, x, y + perla->height(), perla->palette());
	//put_picture(x,y,perla);
	pod_perlou_x = x;
	pod_perlou_y = y;
	aunlock(H_GEN_PERLA);
}

static void schovej_perlu(void) {
	renderer->blit(*pod_perlou, pod_perlou_x, pod_perlou_y, pod_perlou->palette());
}

static void vypocet_vlastnosti(int angle,T_VLASTS *vls)
  {
  T_VLASTS *low,*hi;
  div_t rm;
  int p,test;

  rm=div(angle,45);
  p=rm.quot;
  low=rohy+p;p++;if (p>=8) p=0;
  hi=rohy+p;
  test=CALC_DIFF(8,12,44);
  vls->sill=CALC_DIFF(low->sill,hi->sill,rm.rem);
  vls->silh=CALC_DIFF(low->silh,hi->silh,rm.rem);
  vls->smgl=CALC_DIFF(low->smgl,hi->smgl,rm.rem);
  vls->smgh=CALC_DIFF(low->smgh,hi->smgh,rm.rem);
  vls->obrl=CALC_DIFF(low->obrl,hi->obrl,rm.rem);
  vls->obrh=CALC_DIFF(low->obrh,hi->obrh,rm.rem);
  vls->pohl=CALC_DIFF(low->pohl,hi->pohl,rm.rem);
  vls->pohh=CALC_DIFF(low->pohh,hi->pohh,rm.rem);
  vls->hpreg=CALC_DIFF(low->hpreg,hi->hpreg,rm.rem);
  vls->mpreg=CALC_DIFF(low->mpreg,hi->mpreg,rm.rem);
  vls->vpreg=CALC_DIFF(low->vpreg,hi->vpreg,rm.rem);
  low=&rohy[8];hi=vls;rm.rem=PERLA_POLOMER;
  vls->sill=CALC_DIFF2(low->sill,hi->sill,rm.rem);
  vls->silh=CALC_DIFF2(low->silh,hi->silh,rm.rem);
  vls->smgl=CALC_DIFF2(low->smgl,hi->smgl,rm.rem);
  vls->smgh=CALC_DIFF2(low->smgh,hi->smgh,rm.rem);
  vls->obrl=CALC_DIFF2(low->obrl,hi->obrl,rm.rem);
  vls->obrh=CALC_DIFF2(low->obrh,hi->obrh,rm.rem);
  vls->pohl=CALC_DIFF2(low->pohl,hi->pohl,rm.rem);
  vls->pohh=CALC_DIFF2(low->pohh,hi->pohh,rm.rem);
  vls->hpreg=CALC_DIFF2(low->hpreg,hi->hpreg,rm.rem);
  vls->mpreg=CALC_DIFF2(low->mpreg,hi->mpreg,rm.rem);
  vls->vpreg=CALC_DIFF2(low->vpreg,hi->vpreg,rm.rem);
  }

/*static void vypocet_statu(T_VLASTS *vls,T_STATY *sts)
  {
  sts->zivl=vls->sill*3/2;
  sts->zivh=vls->silh*3/2;
  sts->manl=vls->smgl*2;
  sts->manh=vls->smgh*2;
  sts->konl=vls->obrl*2;
  sts->konh=vls->obrh*2;
  sts->akc=(((vls->pohl+vls->pohh)/2)+14)/15;
  }
*/

static void zobraz_staty(T_VLASTS *st) {
	char s[100];
	const Font *font = dynamic_cast<const Font*>(ablock(H_FTINY));

	renderer->setFont(font, 1, color_topbar);
	sprintf(s, texty[2], st->sill, st->silh);
	renderer->drawText(230, 2, s);
	sprintf(s, texty[3], st->smgl, st->smgh);
	renderer->drawText(330, 2, s);
	sprintf(s, texty[4], st->pohl, st->pohh);
	renderer->drawText(430, 2, s);
	sprintf(s, texty[5], st->obrl, st->obrh);
	renderer->drawText(530, 2, s);
}


void redraw_generator(int show) {
	T_STATY z;
	int i;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_GENERACE));

	memset(&z, 20, sizeof(z));
	schovej_mysku();
	renderer->blit(*tex, INV_DESK, SCREEN_OFFLINE, tex->palette());

	if (cur_xicht == -1) {
		tex = dynamic_cast<const Texture*>(ablock(H_IOBLOUK));
		renderer->blit(*tex, 4, SCREEN_OFFLINE, tex->palette());
		bott_draw(1);
		draw_other_bar();
	} else {
		display_character(postavy + cur_edited, 1);
	}

	zobraz_error_text();
	zobraz_perlu();

	for (i = 0; i < MAX_XICHTS; i++) {
		if (disable[poradi[i]]) {
			trans_bar(28 + INV_DESK + i * XICHT_STEP, 18 + INV_DESK_Y, 26, 38, 0, 0, 0);
		}
	}

	ukaz_mysku();

	if (show) {
		showview(0, 0, 0, 0);
	}
}

static void redraw_gen_wrap() {
	redraw_generator(1);
}

static void set_xicht(int player,int xicht)
  {
  THUMAN *h=postavy+player;
     h->used=1;
     h->xicht=xicht;
     h->vlastnosti[VLS_MAXHIT]=1;
     h->vlastnosti[VLS_KONDIC]=1;
     h->lives=1;
     h->kondice=0;
     h->mana=0;
     h->sektor=viewsector;
     h->groupnum=1;
     h->jidlo=1;
     h->voda=1;
     h->bonus=0;
     h->spell=0;
  }

static char select_xicht(int id, int xa, int ya, int xr, int yr) {
	int i, j, k;
	char s[20];

	i = xr / XICHT_STEP;
	j = xr % XICHT_STEP;

	if (j > 27) {
		return 0;
	}

	k = poradi[i];

	if (disable[k]) {
		return 1;
	}

	memset(disable, 0, sizeof(disable));
	set_xicht(cur_edited, cur_xicht = k);
	postavy[cur_edited].female = women[i];
	b_disables = 0x6;

	if (was_enter) {
		send_message(E_KEYBOARD, 13);
	} else {
		strcpy(postavy[cur_edited].jmeno, texty[160+i]);
		send_message(E_KEYBOARD, 27);
		was_enter = 0;
	}

	sprintf(s, XICHT_NAME, k);
	def_handle(H_XICHTY + cur_edited, s, pcx_8bit_decomp, SR_BGRAFIKA);
	sprintf(s, CHAR_NAME, k);
	def_handle(H_POSTAVY + cur_edited, s, pcx_8bit_decomp, SR_BGRAFIKA);

	for (j = 0; j < MAX_XICHTS; j++) {
		if (postavy[j].used) {
			disable[postavy[j].xicht] = 1;
		}
	}

	error_text = NULL;
	bott_draw(1);
	redraw_generator(1);
	displ_button(b_disables, b_texty);
	showview(0, 0, 0, 0);
	id,xa,ya,xr,yr;
	return 1;
}

static char vol_vlastnosti(int id, int xa, int ya, int xr, int yr) {
	int a;
	static char cancel = 1;
	const Texture *tex;

	if (!ms_last_event.tl1) {
		cancel = 1;
		return 0;
	}

	if (ms_last_event.event_type & 0x2) {
		if (xa < pod_perlou_x || ya < pod_perlou_y || xa > pod_perlou_x + pod_perlou->width() ||  ya > pod_perlou_y + pod_perlou->height()) {
			cancel = 1;
			return 0;
		} else {
			cancel = 0;
		}
	}

	if (cancel) {
		return 0;
	}

	xr = xa - PERLA_STRED_X;
	yr = -(ya - PERLA_STRED_Y);
	a = (int)(atan2((double)yr, (double)xr) * 180 / 3.14159265);
	cur_polomer = (int)sqrt(xr * xr + yr * yr);

	if (cur_polomer > PERLA_MAXPOLOMER) {
		cur_polomer = PERLA_MAXPOLOMER;
	}

	xa = pod_perlou_x;
	ya = pod_perlou_y;
	schovej_mysku();
	schovej_perlu();

	if (a < 0) {
		a += 360;
	}

	vypocet_vlastnosti(a, &cur_vls);
	cur_angle = a;
	zobraz_perlu();
	ukaz_mysku();
	update_mysky();
	showview(pod_perlou_x, pod_perlou_y, pod_perlou->width(), pod_perlou->height());
	showview(xa, ya, pod_perlou->width(), pod_perlou->height());
	tex = dynamic_cast<const Texture*>(ablock(H_GEN_TOPBAR));
	renderer->blit(*tex, 0, 0, tex->palette());
	zobraz_staty(&cur_vls);
	showview(250, 0, 640, 17);
	return 1;
}

//static int edit_task=-1;

static void edit_name() {
	if (shut_downing_text) {
		return;
	}

	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(120, 2, 120 + 104, 16);
//  edit_task=Task_Add(16384,type_text_v2,postavy[cur_edited].jmeno,120,2,104,
//     sizeof(postavy[cur_edited].jmeno)-1,H_FONT6,RGB555(31,31,0),edit_name);
	send_message(E_ADD, E_KEYBOARD, type_text,postavy[cur_edited].jmeno, 120, 2, 104, sizeof(postavy[cur_edited].jmeno) - 1, H_FONT6, 255, 255, 0, edit_name);
}

static void stop_edit_name()
  {
  shut_downing_text=1;
  send_message(E_KEYBOARD,13);
//  Task_Sleep(NULL);
//  if (edit_task>0 && Task_IsRunning(edit_task)) 
//    Task_Shutdown(edit_task);
  shut_downing_text=0;
  }

/*static char edit_another_click(int id,int xa,int ya,int xr,int yr)
  {
  short *w;

  id,xa,ya,xr,yr;
  w=ablock(H_OKNO);
  id=xr/w[0];
  if (!postavy[id].used) return 0;
  shut_downing_text=1;
  send_message(E_KEYBOARD,13);
  shut_downing_text=0;
  save_values[cur_edited][0]=cur_angle;
  save_values[cur_edited][1]=cur_polomer;
  select_player=cur_edited=id;
  cur_angle=save_values[cur_edited][0];
  cur_polomer=save_values[cur_edited][1];
  vypocet_vlastnosti(cur_angle,&cur_vls);
  cur_xicht=postavy[cur_edited].xicht;
  edit_name();
  bott_draw(1);
  redraw_generator();
  return 1;
  }
*/


static void def_entries()
  {
  int i;
  int s;
  TDREGISTERS *p;

  s=sizeof(char_gen_reg)/sizeof(TDREGISTERS);
  p=char_gen_reg;
  for(i=0;i<s;i++,p++) def_handle(p->h_num,p->name,p->proc,p->path);
  for(i=0;i<MAX_XICHTS;i++)
     {
     char s[15];

     sprintf(s,CHAR_NAME,i);
     def_handle(H_GEN_POSTAVY+i,s,pcx_8bit_decomp,SR_BGRAFIKA);
     apreload(H_GEN_POSTAVY+i);
     Sound_MixBack(0);
     }
  for(i=0;i<MAX_XICHTS;i++)
     {
     char s[15];

     sprintf(s,XICHT_NAME,i);
     def_handle(H_GEN_XICHTY+i,s,pcx_8bit_decomp,SR_BGRAFIKA);
     apreload(H_GEN_XICHTY+i);
     Sound_MixBack(0);
     }
  }

static char go_next_page(int id, int xa, int ya, int xr, int yr) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_GEN_CHARGENM));

	assert(xr < tex->width() && yr < tex->height() && tex->depth() == 1);
	id = tex->pixels()[xr + yr * tex->width()];

	if (!id) {
		return 1;
	}

	id--;

	if (b_disables & (1 << id)) {
		return 1;
	}

	close_ret = id;
	send_message(E_CLOSE_GEN, id);
	return 1;
}


void generuj_postavu(THUMAN *h)
  {
  short *v;
  h->used=1;
  h->groupnum=1;
  v=h->stare_vls;
  v[VLS_SILA]=DIFF_RAND(cur_vls.sill,cur_vls.silh);
  v[VLS_SMAGIE]=DIFF_RAND(cur_vls.smgl,cur_vls.smgh);
  v[VLS_POHYB]=DIFF_RAND(cur_vls.pohl,cur_vls.pohh);
  v[VLS_OBRAT]=DIFF_RAND(cur_vls.obrl,cur_vls.obrh);
  v[VLS_HPREG]=cur_vls.hpreg;
  v[VLS_MPREG]=cur_vls.mpreg;
  v[VLS_VPREG]=cur_vls.vpreg;
//  v[VLS_THIEF]=cur_vls.vpreg;
  h->lives=v[VLS_MAXHIT]=(v[VLS_SILA]*3+v[VLS_POHYB])/2;
  h->mana=v[VLS_MAXMANA]=v[VLS_SMAGIE]*2;
  h->kondice=v[VLS_KONDIC]=v[VLS_OBRAT]*2;
  v[VLS_OBRAN_L]=1;
  v[VLS_OBRAN_H]=2;
  v[VLS_UTOK_L]=1;
  v[VLS_UTOK_H]=2;
  prepocitat_postavu(h);
  h->inv_size=6;
  h->level=1;
  h->exp=0;
  h->bonus=5;
  h->jidlo=MAX_HLAD(h);
  h->voda=MAX_ZIZEN(h);
  h->mana_battery=32767;
  //postava je vygenerovana
  }

static void redraw_page3() {
	update_mysky();
	schovej_mysku();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	inv_display_vlastnosti();
	display_character(postavy + cur_edited, 1);
	write_human_big_name(postavy[cur_edited].jmeno);
	draw_other_bar();
	displ_button(b_disables, b_texty);
	ukaz_mysku();
	showview(0, 0, 0, 0);
	inv_redraw = redraw_page3;
}


static void redraw_svitek() {
	if (postavy[cur_edited].bonus == 0) {
		char mode;

		mode = 7;

		if (postavy[charmin - 1].used) {
			mode &= ~2;
		}

		if (!postavy[charmax - 1].used) {
			mode &= ~1;
		}

		if (!del_mode) {
			mode &= ~4;
		}

		b_disables = mode;
		redraw_page3();
		return;
	}

	update_mysky();
	schovej_mysku();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 16, 30, 16 + 360);
	bar(620, 16, 640, 16 + 360);
	inv_display_vlastnosti();
	display_character(postavy + cur_edited, 0);
//  display_items_wearing(human_selected);
//  write_pocet_sipu();
//  display_rings();
	zobraz_error_text();
	displ_button(b_disables, b_texty);
	ukaz_mysku();
	showview(0, 0, 0, 0);
	inv_redraw = redraw_svitek;
}


static char validate_character(THUMAN *h)
  {
  if (h->jmeno[0]==0) error_text=texty[111];
  else error_text=NULL;
  return error_text==NULL;
  }

/*static char edit_another_click2(int id,int xa,int ya,int xr,int yr)
  {
  id,xa,ya,xr,yr;
  cur_edited=select_player;
  schovej_mysku();
  edit_name();
  draw_other_bar();
  send_message(E_CLOSE_GEN,0);
  ukaz_mysku();
  update_mysky();
  return 1;
  }
 */
static void empty_proc()
  {
  }

char potvrzeno(const char *text,void (*redraw)())
  {
  int i;
  unwire_proc=empty_proc;
  stop_edit_name();
  i=message(2,0,1,texty[118],text,texty[114],texty[115])==0;
  redraw();
  edit_name();
  return i;
  }

static char view_another_click2(int id, int xa, int ya, int xr, int yr) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_OKNO));

	id = xr / tex->width();

	if (!(~b_disables & 0x3)) {
		return 1;
	}

	if (!postavy[id].used) {
		return 0;
	}

	shut_downing_text = 1;
	send_message(E_KEYBOARD, 13);
	shut_downing_text = 0;
	select_player = id;
	bott_draw(1);
	human_selected = postavy + id;
	cur_edited = id;
	edit_name();
	redraw_page3();

	if (del_mode) {
		if (potvrzeno(texty[117], redraw_page3)) {
			del_mode = 0;
			postavy[cur_edited].used = 0;
			disable[postavy[cur_edited].xicht] = 0;
			close_ret = 0;
			send_message(E_CLOSE_GEN, 0);
		}
	}

	return 0;
}

void effect_show(va_list args)
  {
  int i;
  schovej_mysku();
  for(i=0;i<12;i++)
     {
     showview(0,240-i*20-20,640,20);
     showview(0,240+(i*20),640,20);
     Task_WaitEvent(E_TIMER);
     }
  ukaz_mysku();
  }


static void enter_reaction(EVENT_MSG *msg,void **unused) {
	if (msg->msg == E_KEYBOARD) {
		va_list args;
		char c;

		va_copy(args, msg->data);
		c = va_arg(args, int) & 0xff;
		va_end(args);

		if (c == 13 && !shut_downing_text) {
			send_message(E_KEYBOARD, 13);
			bott_draw(1);
			redraw_generator(1);
			msg->msg = -1;
		}

		if (c != 0) {
			was_enter = 1;
		}
	}
}

static void enter_reaction2(EVENT_MSG *msg,void **unused) {
	if (msg->msg == E_KEYBOARD) {
		va_list args;
		char c;

		va_copy(args, msg->data);
		c = va_arg(args, int) & 0xff;
		va_end(args);

		if (c == 13 && !shut_downing_text && ~b_disables & 0x3) {
			send_message(E_KEYBOARD, 13);
			bott_draw(1);
			redraw_page3();
			msg->msg = -1;
		}
	}
}


char gen_exit_editor(int id,int xa,int ya,int xr,int yr)
  {
  xa,ya,xr,yr,id;

  if (del_mode==1)
     {
      del_mode=0;
      mouse_set_default(H_MS_DEFAULT);
      b_disables&=~0x4;
      redraw_svitek();
      return 1;
     }
  unwire_proc=empty_proc;
  shut_downing_text=1;send_message(E_KEYBOARD,13);shut_downing_text=0;
  if (message(2,0,1,texty[118],texty[113],texty[114],texty[115])==0)
     {
	close_ret = 255;
     send_message(E_CLOSE_GEN,255);
     }
  else
     {
     if (id==2)redraw_generator(1);
     else redraw_svitek();
     edit_name();
     }
  return 1;
  }

char enter_generator() {
	int i;
	char rep = 0;

znova:
	del_mode = 0;
	stop_edit_name();
	b_texty[0] = texty[170];
	b_texty[1] = texty[171];
	b_texty[2] = texty[172];
	b_texty[3] = texty[173];
	def_entries();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	cur_angle = 315;
	cur_edited = 0;
	memset(postavy, 0, sizeof(postavy));
	memset(disable, 0, sizeof(disable));
	memset(save_values, 0, sizeof(save_values));

	do {
		mouse_set_default(H_MS_DEFAULT);
		cur_angle = save_values[cur_edited][0];
		cur_polomer = save_values[cur_edited][1];
		set_xicht(cur_edited, cur_xicht = -1);
		select_player = -1;
		memset(&cur_stats, 0, sizeof(cur_stats));
		vypocet_vlastnosti(cur_angle, &cur_vls);
		b_disables = 0x7;
		redraw_generator(rep);

		if (!rep) {
			effect_show(NULL);
		}

		rep = 1;
		edit_name();
		change_click_map(clk_page1, CLK_PAGE1);
		was_enter = 0;

		do {
			send_message(E_ADD, E_KEYBOARD, enter_reaction);
			Task_WaitEvent(E_CLOSE_GEN);
			i = close_ret;
			send_message(E_DONE, E_KEYBOARD, enter_reaction);

			if (i == 3 && potvrzeno(texty[116], redraw_gen_wrap)) {
				goto znova;
			}

			if (i == 255) {
				return 1;
			}
		} while (cur_xicht == -1 || i == 3);

		send_message(E_KEYBOARD, 13);
		save_values[cur_edited][0] = cur_angle;
		save_values[cur_edited][1] = cur_polomer;
		cur_xicht = 0;
		error_text = NULL;
		generuj_postavu(postavy + cur_edited);
		human_selected = postavy + cur_edited;
		b_disables = 0x7;

		do {
			redraw_svitek();
			change_click_map(clk_page2, CLK_PAGE2);

			do {
				send_message(E_ADD, E_KEYBOARD, enter_reaction2);
				Task_WaitEvent(E_CLOSE_GEN);
				i = close_ret;
				send_message(E_DONE, E_KEYBOARD, enter_reaction2);

				if (i == 3 && potvrzeno(texty[116], redraw_svitek)) {
					goto znova;
				}

				if (i == 2) {
					del_mode = 1;
					mouse_set_default(H_MS_WHO);
					b_disables |= 0x4;
					redraw_svitek();
					i = 3;
				}
			} while (i == 3);

			mouse_set_default(H_MS_DEFAULT);
			del_mode = 0;
			send_message(E_KEYBOARD, 13);

			if (i == 255) {
				return 1;
			}

			if (validate_character(postavy + cur_edited)) {
				for (cur_edited = 0; postavy[cur_edited].used; cur_edited++);
			}
		} while (error_text != NULL);

		stop_edit_name();
	} while (i != 1);

	disable_click_map();
	schovej_mysku();
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 479);
	showview(0, 0, 0, 0);
	ukaz_mysku();

	for (i = 0; i < 3; i++) {
		if (postavy[i].vlastnosti[VLS_SILA] > 20) {
			postavy[i].stare_vls[VLS_UTOK_H]++;
		}

		if (postavy[i].vlastnosti[VLS_OBRAT] > 20) {
			postavy[i].stare_vls[VLS_OBRAN_H]++;
		}
	}

	return 0;
}









