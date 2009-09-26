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
#include "game/engine1.h"
#include <inttypes.h>
#include "libs/event.h"
#include "libs/memman.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/strlite.h"
#include "libs/pcx.h"
#include "globals.h"

#define AUTOMAP_BACK RGB555(8,4,0)
#define AUTOMAP_VODA RGB555(0,15,31)
#define AUTOMAP_LAVA RGB555(31,16,0)
#define AUTOMAP_FORE RGB555(18,17,14)
#define AUTOMAP_LINE1 RGB555(13,11,10)
#define AUTOMAP_LINE2 RGB555(31,22,6)
#define AUTOMAP_MOB RGB555(31,8,8)

#define MEDIUM_MAP 9
#define MEDIUM_MMAP 4
#define MEDIUM_MAP_COLOR RGB555(21,20,19)
#define MEDIUM_MAP_LINE1 RGB555(25,20,17)
#define MEDIUM_MAP_LINE2 RGB555(31,22,6)

/*
uint16_t stairs_colors[]=
  {AUTOMAP_LINE1,
   RGB555(14,12,11),
   RGB555(15,14,12),
   RGB555(16,15,12),
   RGB555(17,16,13)};

uint16_t arrow_colors[]=
  {
  AUTOMAP_LINE1,
  AUTOMAP_FORE
  };
*/
uint16_t stairs_colors[5];
uint16_t arrow_colors[2];

char shift_map(int id,int xa,int ya,int xr,int yr);
char psani_poznamek(int id,int xa,int ya,int xr,int yr);
char map_target_select(int id,int xa,int ya,int xr,int yr);
char map_target_cancel(int id,int xa,int ya,int xr,int yr);
char map_menu(int id,int xa,int ya,int xr,int yr);
static char map_menu_glob_map(int id,int xa,int ya,int xr,int yr);

char noarrows=0;
char enable_glmap=0;

static int map_xr,map_yr;
static int cur_depth;

#define BOTT 378
#define LEFT 520

static char cur_disables;

#define CLK_MAP_VIEW 5
T_CLK_MAP clk_map_view[]=
  {
  {MS_GAME_WIN,0,17,639,377,psani_poznamek,2,H_MS_DEFAULT},
  {-1,54,378,497,474,start_invetory,2+8,-1},
  {-1,337,0,357,14,go_map,2,H_MS_DEFAULT},
  {-1,LEFT,BOTT,639,479,map_menu,2,H_MS_DEFAULT},
  {-1,0,0,639,479,return_game,8,-1},
  };

#define CLK_GLOB_MAP 4
T_CLK_MAP clk_glob_map[]=
  {
  {-1,54,378,497,474,start_invetory,2+8,-1},
  {-1,337,0,357,14,go_map,2,H_MS_DEFAULT},
  {-1,LEFT,BOTT,639,479,map_menu,2,H_MS_DEFAULT},
  {-1,0,0,639,479,map_menu_glob_map,8,-1},
  };

#define CLK_TELEPORT_VIEW 4
T_CLK_MAP clk_teleport_view[]=
  {
  {MS_GAME_WIN,0,17,639,377,map_target_select,2,H_MS_DEFAULT},
  {-1,LEFT,BOTT,639,479,map_target_cancel,2,H_MS_DEFAULT},
  {-1,0,0,639,479,map_target_cancel,8,-1},
  {-1,0,0,639,479,empty_clk,0xff,-1},
  };

void Automap_Init(void) {
	stairs_colors[0] = AUTOMAP_LINE1;
	stairs_colors[1] = RGB555(14,12,11);
	stairs_colors[2] = RGB555(15,14,12);
	stairs_colors[3] = RGB555(16,15,12);
	stairs_colors[4] = RGB555(17,16,13);

	arrow_colors[0] = AUTOMAP_LINE1;
	arrow_colors[1] = AUTOMAP_FORE;
}

char testclip(int x,int y)
  {
  return (y>=16 && y<360+16 && x>=8 && x<630);
  }

/*void shift_map_event(EVENT_MSG *msg,int *data)
  {
  static int smer;

  data;
  if (msg->msg==E_INIT) smer=*(int *)msg->data;
  else if (msg->msg==E_TIMER)
        {
        switch (smer)
           {
           case H_SIPKY_S:send_message(E_KEYBOARD,'H'*256);break;
           case H_SIPKY_J:send_message(E_KEYBOARD,'P'*256);break;
           case H_SIPKY_V:send_message(E_KEYBOARD,'M'*256);break;
           case H_SIPKY_Z:send_message(E_KEYBOARD,'K'*256);break;
           case H_SIPKY_SZ:send_message(E_KEYBOARD,'s'*256);msg->msg=-2;break;
           case H_SIPKY_SV:send_message(E_KEYBOARD,'t'*256);msg->msg=-2;break;
           }
        }
   else if (msg->msg==E_MOUSE)
     {
     MS_EVENT *ms;

     ms=get_mouse(msg);
     if (!ms->tl1 && !ms->tl2)
        {
        send_message(E_DONE,E_TIMER,shift_map_event);
        msg->msg=-2;
        schovej_mysku();
        other_draw();
        ukaz_mysku();
        showview(0,0,0,0);
        }
     }
  }
 */

void save_text_to_map(int32_t x, int32_t y, int32_t depth, char *text) {
	gameMap.addNote(x - 320 + map_xr, y - 197 + map_yr, depth, text);
}

static char check_for_layer(int layer) {
	int i;
	const TMAP_EDIT_INFO *m = gameMap.coord();

	for(i = 0; i < gameMap.coordCount(); i++, m++) {
		if (m->layer == layer && m->flags & MC_MARKED && m->flags & MC_AUTOMAP) {
			return 1;
		}
	}

	return 0;
}

void ukaz_vsechny_texty_v_mape() {
	int x, y;
	int i, j, cn;
	const MapText *ptr;

	if (!gameMap.notesCount()) {
		return;
	}

	set_font(H_FLITT5, NOSHADOW(0));
	cn = gameMap.notesSize();
	ptr = gameMap.notes();

	for(i = 0; i < cn; i++) {
		if (ptr[i].text) {
			x = ptr[i].x - map_xr;
			y = ptr[i].y - map_yr;
			x += 320;
			y += 197;

			if (ptr[i].depth == cur_depth) {
				if (testclip(x, y)) {
					int h;
					char *d, *e;

					h = strlen(ptr[i].text);
					d = new char[h + 1];
					strcpy(d, ptr[i].text);
					e = d + h;

					while((h = text_width(d) + x) > 640) {
						*e = '\0';

						if (e == d) {
							break;
						}

						e--;
					}

					position(x, y);
					outtext(d);
					delete[] d;
				} else if(x < 8 && x + text_width(ptr[i].text) > 10 && y > 16 && y < 376) {
					char cd[2] = " ";

					for (j = 0; x < 10 && ptr[i].text[j]; j++) {
						cd[0] = ptr[i].text[j];
						x += text_width(cd);
					}

					position(x, y);
					outtext(ptr[i].text + j);
				}
			}
		}
	}
}

void psani_poznamek_event(EVENT_MSG *msg, void **data) {
	static int x, y;
	static char text[255], index;
	static char *save;
	static uint16_t *save_pic = NULL;
	va_list args;
	
	if (msg->msg == E_INIT) {
		char *c;

		set_font(H_FLITT5, NOSHADOW(0));

		va_copy(args, msg->data);
		x = va_arg(args, int);
		y = va_arg(args, int);
		c = va_arg(args, char*);
		va_end(args);

		strcpy(text, c);
		save = (char *)getmem(strlen(text) + 1);
		strcpy(save, text);
		index = strchr(text, 0) - text;
		if (save_pic == NULL) {
			schovej_mysku();
			save_pic = (uint16_t*)getmem(640 * 20 * 2 + 6);
			get_picture(0, y, 640, 20, save_pic);
			position(x, y);
			outtext(text);
			outtext("_");
			showview(0, 0, 640, 480);
		}
	} else if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);
		if (ms->event_type & 0x8) {
			send_message(E_KEYBOARD,27);
		}
		msg->msg = -1;
	} else if (msg->msg == E_KEYBOARD) {
		char c;

		va_copy(args, msg->data);
		c = va_arg(args, int) & 0xff;
		va_end(args);

		set_font(H_FLITT5, NOSHADOW(0));

		if (c) {
			switch (c) {
			case 8:
				if (index) {
					index--;
				}

				text[index]=0;
				break;

			case 27:
				strcpy(text,save);
			case 13:
				save_text_to_map(x, y, cur_depth, text);
				send_message(E_DONE, E_MOUSE, psani_poznamek_event);
				msg->msg = -2;
				return;

			default:
				if (c >= 32) {
					text[index] = c;
					text[index + 1] = 0;

					if (text_width(text) > (640 - x)) {
						text[index] = 0;
					} else {
						index++;
					}
				}
			}

			put_picture(0, y, save_pic);
			position(x, y);
			outtext(text);
			outtext("_");
			showview(x, y, 640, 20);
		}

		msg->msg = -1;
	} else if (msg->msg == E_DONE) {
		if (save_pic != NULL) {
			put_picture(0, y, save_pic);
			showview(x, y, 640, y + 20);
			free(save_pic);
			save_pic = NULL;
			send_message(E_AUTOMAP_REDRAW);
			ukaz_mysku();
		} if (save != NULL) {
		   free(save);
		   save = NULL;
		}
	}
}

int hledej_poznamku(int x, int y, int depth) {
	int i, count;
	const MapText *ptr;

	x = (x - 320) + map_xr;
	y = (y - 197) + map_yr;

	if (!gameMap.notesCount()) {
		return -1;
	}

	count = gameMap.notesSize();
	ptr = gameMap.notes();
	set_font(H_FLITT5, NOSHADOW(0));

	for (i = 0; i < count; i++) {
		if (ptr[i].text) {
			int xb, yb;
			int xas, yas, xbs, ybs, xs, ys;

			xs = text_width(ptr[i].text);
			ys = text_height(ptr[i].text);
			xb = ptr[i].x + xs;
			yb = ptr[i].y + ys;

			if (x >= ptr[i].x && y >= ptr[i].y && x <= xb && y <= yb && ptr[i].depth == depth) {
				xas = (ptr[i].x + 320) - map_xr;
				yas = (ptr[i].y + 197) - map_yr;
				xbs = xas + xs;
				ybs = yas + ys;

				if (xas > 0 && xbs < 640 && yas > 16 && ybs < 360 + 16) {
					return i;
				}

				return -2;
			}
		}
	}
	return -1;
}

char psani_poznamek(int id, int xa, int ya, int xr, int yr) {
	if (noarrows) {
		return 1;
	}

	if ((id = hledej_poznamku(xa, ya, cur_depth)) == -1) {
		xa &= ~7;
		xa += 2;
		ya &= ~7;
		ya -= 4;
		send_message(E_ADD, E_KEYBOARD, psani_poznamek_event, xa, ya, "");
		send_message(E_ADD, E_MOUSE, psani_poznamek_event, xa, ya, "");
	} else if (id != -2) {
		const MapText *ptr = gameMap.notes() + id;
		char *s;

		xa = (ptr->x + 320) - map_xr;
		ya = (ptr->y + 197) - map_yr;
		s = new char[strlen(ptr->text) + 1];
		strcpy(s, ptr->text);
		gameMap.removeNote(id);
		send_message(E_AUTOMAP_REDRAW);

		for (xr = 0; xr < 10; xr++) {
			do_events();
		}

		send_message(E_ADD, E_KEYBOARD, psani_poznamek_event, xa, ya, s);
		send_message(E_ADD, E_MOUSE, psani_poznamek_event, xa, ya, s);
		delete[] s;
	}

	return 0;
}

char shift_map(int id,int xa,int ya,int xr,int yr)
  {
  id;xa;ya;xr;yr;

  anim_sipky(id,1);
//  send_message(E_ADD,E_TIMER,shift_map_event,id);
//  send_message(E_ADD,E_MOUSE,shift_map_event,id);
  return 0;
  }

static void print_symbol(int x,int y,char znak)
  {
  char c[2]=" ";
  position(x+1,y+1);c[0]=znak;outtext(c);
  }


static void draw_amap_sector(int x,int y,int sector,int mode,int turn,int line1,int line2) {
	int j, i, k;
	const TSTENA *q;
	const TSECTOR *ss;

	q = gameMap.sides() + (sector << 2);
	ss = gameMap.sectors() + sector;

	if (ss->sector_type == S_VODA || ss->sector_type == S_LODKA) {
		curcolor = AUTOMAP_VODA;
	} else if (ss->sector_type == S_LAVA) {
		curcolor = AUTOMAP_LAVA;
	} else {
		curcolor = AUTOMAP_FORE;
	}

	if (!mode) {
		trans_bar(x, y, 8, 8, curcolor);

		if ((k = gameMap.coord()[sector].flags & 0x600) != 0) {
			int i;

			i = gameMap.sectors()[sector].sector_type;
			set_font(H_FSYMB, 0);
			k >>= 9;

			switch (k) {
			case 3:
				break;

			case 2:
				set_font(H_FSYMB, 0);
				font_color(stairs_colors);
				print_symbol(x, y, 's');
				break;

			case 1:
				set_font(H_FSYMB, 0);
				font_color(arrow_colors);
				print_symbol(x, y, 4 + ((i - S_SMER + 4 - turn) & 0x3));
				break;
			}
		} else switch(gameMap.sectors()[sector].sector_type) {
			int i;
			const TSTENA *sd;

		case S_SCHODY:
			set_font(H_FSYMB, 0x3e0);
			memcpy(charcolors, stairs_colors, sizeof(stairs_colors));
			print_symbol(x, y, 's');
			break;

		case S_TELEPORT:
			for (i = 0, sd = gameMap.sides() + sector * 4; i < 4 && ~sd->flags & SD_SEC_VIS; i++, sd++);

			if (i != 4) {
				set_font(H_FSYMB, 0x3e0);
				print_symbol(x, y, 'T');
			}
			break;

		case S_DIRA:
			set_font(H_FSYMB, NOSHADOW(0));
			print_symbol(x, y, 'N');
			break;
		}
	} else {
		for(j=0;j<4;j++) {
			i = (j + turn) & 3;

			if (!(q[i].flags & SD_TRANSPARENT) ||(q[i].flags & SD_SECRET)) {
				curcolor = line1;
			} else if (q[i].flags & SD_PLAY_IMPS) {
				curcolor = line2;
			} else {
				curcolor = AUTOMAP_FORE;
			}

			if (q[i].flags & SD_INVIS) {
				curcolor = AUTOMAP_FORE;
			}

			if (curcolor != AUTOMAP_FORE) {
				switch (j) {
				case 0:
					hor_line(x, y, x + 8);
					break;

				case 1:
					ver_line(x + 8, y, y + 8);
					break;

				case 2:
					hor_line(x, y + 8, x + 8);
					break;

				case 3:
					ver_line(x, y, y+8);
					break;
				}
			}
		}
	}
}

void herni_cas(char *s)
  {
  int mes,den,hod,min;
  long cas;

  cas=game_time;
  mes=cas/(360*24*30);cas%=360*24*30;
  den=cas/(360*24);cas%=360*24;
  hod=cas/(360);cas%=360;
  min=cas/6;

  if (mes)
     {
     sprintf(s,texty[cislovka(mes)+149],mes);
     strcat(s," ");
     s=strchr(s,0);
     }
  if (den)
     {
     sprintf(s,texty[cislovka(den)+146],den);
     strcat(s," ");
     s=strchr(s,0);
     }
  sprintf(s,texty[152],hod,min);
  }

static void zobraz_herni_cas(void)
  {
  static char text[100];
  static long old_time=-1;
  char cas[100];

  if (old_time!=game_time)
     {
     herni_cas(cas);
     strcpy(text,texty[145]);
     strcat(text," ");
     strcat(text,cas);
     old_time=game_time;
     }
  set_font(H_FONT6,NOSHADOW(0));
  set_aligned_position(635,372,2,2,text);
  outtext(text);
  }


extern uint16_t color_butt_on[];
extern uint16_t color_butt_off[];

static void displ_button(char disable, const char **text)
  {
  int posy[]={0,18,37,55};
  int sizy[]={18,20,20,21};
  int i;

  cur_disables=disable;
  set_font(H_FTINY,0);
  put_picture(LEFT, BOTT, (uint16_t*)ablock(H_CHARGEN));
  for(i=0;i<4;i++)
     {
     if (disable & 1)
        {
        put_8bit_clipped((uint16_t*)ablock(H_CHARGENB),(392+posy[i])*Screen_GetXSize()+524+Screen_GetAddr(),posy[i],96,sizy[i]);
        font_color(color_butt_off);
        }
     else
        {
        font_color(color_butt_on);
        }
     disable>>=1;
     set_aligned_position(LEFT+50,BOTT+14+13+i*19,1,2,*text);
     outtext(*text);
     text++;
     }
  }


void draw_automap(int xr, int yr) {
	int i, k, x, y, xp, yp;
	int depth;
	const TSTENA *q;
	const uint16_t *s;

	update_mysky();
	schovej_mysku();
	put_textured_bar((uint16_t*)ablock(H_BACKMAP), 0, 17, 640, 360, -xr * 8, -yr * 8);
	curcolor = AUTOMAP_BACK;
	xp = gameMap.coord()[viewsector].x * 8;
	yp = gameMap.coord()[viewsector].y * 8;
	depth = cur_depth;
	map_xr = xp - xr * 8;
	map_yr = yp - yr * 8;
	for(k = 0; k < 2; k++) {
		for(i = 1; i < gameMap.coordCount(); i++) {
			int flagmask = MC_AUTOMAP | (k != 0 ? MC_DISCLOSED : 0);

			if ((gameMap.coord()[i].flags & flagmask) && (gameMap.coord()[i].flags & MC_MARKED) && gameMap.coord()[i].layer == depth) {
				x = (gameMap.coord()[i].x * 8 + xr * 8);
				y = (gameMap.coord()[i].y * 8 + yr * 8);
				q = gameMap.sides() + (i * 4);
				s = gameMap.sectors()[i].step_next;
				x -= xp;
				y -= yp;
				if (y >= -178 && y < 170 && x >= -312 && x < 310) {
					x += 320;
					y += 197;
					draw_amap_sector(x, y, i, k, 0, AUTOMAP_LINE1, AUTOMAP_LINE2);
					if (gameMap.coord()[i].flags & MC_PLAYER && !noarrows) {
						int j, l = -1;

						for (j = 0; j < POCET_POSTAV; j++) {
							if (postavy[j].used && postavy[j].sektor==i) {
								if (postavy[j].groupnum == cur_group) {
									break;
								} else {
									l = j;
								}
							}
						}

						if (j == POCET_POSTAV) {
							j = l;
						}

						if (j != -1) {
							char c[2];

							position(x + 1, y + 1);
							set_font(H_FSYMB, postavy[j].groupnum == cur_group ? RGB888(255, 255, 255) : barvy_skupin[postavy[j].groupnum]);
							c[0] = postavy[j].direction + 4;
							c[1] = 0;
							outtext(c);
						}
					}
				}
			}
		}
	}

	ukaz_vsechny_texty_v_mape();
	zobraz_herni_cas();

	char str[50];
	sprintf(str, texty[153], gameMap.global().mapname, depth);
	set_aligned_position(5, 372, 0, 2, str);
	outtext(str);

	ukaz_mysku();
	wait_retrace();
	showview(0, 16, 640, 360);
}

void *map_keyboard(EVENT_MSG *msg,void **usr);

void enable_all_map(void) {
	unsigned i;

	for (i = 1; i < gameMap.coordCount(); i++) {
		gameMap.setCoordFlags(i, MC_MARKED);
	}
}

void disable_all_map(void) {
	int i;

	for (i = 1; i < gameMap.coordCount(); i++) {
		gameMap.clearCoordFlags(i, MC_MARKED);
	}
}


void unwire_automap()
              {
              send_message(E_DONE,E_KEYBOARD,map_keyboard);
              send_message(E_DONE,E_AUTOMAP_REDRAW,map_keyboard);
              send_message(E_DONE,E_IDLE,map_keyboard);
              hold_timer(TM_FAST_TIMER,0);
              disable_all_map();
              noarrows=0;
              set_select_mode(0);
              pick_set_cursor();
			  GlobEvent(MAGLOB_AFTERMAPOPEN,viewsector,viewdir);
              }

void *map_keyboard(EVENT_MSG *msg, void **usr) {
	char c;
	static int draw = 0;
	static int xr, yr;
	
	if (msg->msg == E_INIT) {
		xr = yr = 0;
	}

	if (msg->msg == E_IDLE && draw == 1) {
		draw_automap(xr, yr);
		draw = 0;
	} else {
		draw--;
	}

	if (msg->msg == E_AUTOMAP_REDRAW) {
		draw = 4;
	}

	if (msg->msg == E_KEYBOARD) {
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		switch (c) {
		case 'H':
			yr++;
			draw = 4;
			break;

		case 'P':
			yr--;
			draw = 4;
			break;

		case 'M':
			xr--;
			draw = 4;
			break;

		case 'K':
			xr++;
			draw = 4;
			break;

		case 'Q':
		case 's':
			if (check_for_layer(cur_depth - 1)) {
				cur_depth--;
			}

			draw = 4;
			break;

		case 'I':
		case 't':
			if (check_for_layer(cur_depth + 1)) {
				cur_depth++;
			}

			draw = 4;
			break;

		case 15:
		case 50:
		case 1:
			// FIXME: rewrite and beat the original writer senseless
			//(*(int *)msg->data)=0;
			unwire_proc();
			wire_proc();
			break;
		}
	}

	return (void*)&map_keyboard;
}

void show_automap(char full) {
	const char *btexts[4] = {texty[210], texty[211], texty[212], texty[213]};

	mute_all_tracks(0);
	unwire_proc();

	if (full) {
		enable_all_map();
	}

	hold_timer(TM_FAST_TIMER, 1);
	unwire_proc = unwire_automap;
	schovej_mysku();

	if (cur_mode != MD_ANOTHER_MAP) {
		bott_draw(1);
		cur_mode=MD_MAP;
	}

	other_draw();

	if (!battle && full && enable_glmap) {
		displ_button(cur_mode == MD_ANOTHER_MAP ? 4 : 6, btexts);
	} else {
		displ_button(7, btexts);
	}

	ukaz_mysku();
	showview(0, 376, 640, 480);
	cur_depth = gameMap.coord()[viewsector].layer;
	draw_automap(0, 0);
	send_message(E_ADD, E_KEYBOARD, map_keyboard);
	send_message(E_ADD, E_AUTOMAP_REDRAW, map_keyboard);
	send_message(E_ADD, E_IDLE, map_keyboard);
	change_click_map(clk_map_view, CLK_MAP_VIEW);
}

static char mob_not_invis(int sector)
  {
  int m;
  m=mob_map[sector];
  while (m)
     {
     m--;if (mobs[m].vlastnosti[VLS_KOUZLA] & SPL_INVIS) return 0;
     m=mobs[m].next;
     }
  return 1;
  }

void draw_medium_map() {
	int xr, yr;
	int xp, yp;
	int xc, yc, x, y;
	int j, i, k, layer;

	xp = MEDIUM_MMAP * 8 + 5;
	yp = MEDIUM_MMAP * 8 + 20;
	layer = gameMap.coord()[viewsector].layer;
	xr = gameMap.coord()[viewsector].x;
	yr = gameMap.coord()[viewsector].y;
	trans_bar(0, 17, MEDIUM_MAP * 8 + 6 * 2, MEDIUM_MAP * 8 + 4 * 2, 0);

	for (j = 0; j < 2; j++) {
		for (i = 1; i < gameMap.coordCount(); i++) {
			if (gameMap.coord()[i].flags & 1 && gameMap.coord()[i].layer == layer) {
				switch (viewdir & 3) {
				case 0:
					xc = gameMap.coord()[i].x - xr;
					yc = gameMap.coord()[i].y - yr;
					break;

				case 1:
					yc = -gameMap.coord()[i].x + xr;
					xc = gameMap.coord()[i].y - yr;
					break;

				case 2:
					xc = -gameMap.coord()[i].x + xr;
					yc = -gameMap.coord()[i].y + yr;
					break;

				case 3:
					yc = gameMap.coord()[i].x - xr;
					xc = -gameMap.coord()[i].y + yr;
					break;
				}

				if (xc >= -MEDIUM_MMAP && yc >= -MEDIUM_MMAP && yc <= MEDIUM_MMAP && xc <= MEDIUM_MMAP) {
					draw_amap_sector(x = xc * 8 + xp, y = yc * 8 + yp, i, j, viewdir & 3, MEDIUM_MAP_LINE1, MEDIUM_MAP_LINE2);

					if (j && mob_map[i] && mob_not_invis(i) && battle) {
						position(x + 1, y + 1);
						set_font(H_FSYMB, AUTOMAP_MOB);
						outtext("N");
					}

					if (gameMap.coord()[i].flags & MC_PLAYER) {
						int u = -1, z = -1;

						for (k = 0; k < POCET_POSTAV; k++) {
							if (postavy[k].sektor == i) {
								if (postavy[k].groupnum == cur_group) {
									z = k;
								} else {
									u=k;
								}
							}
						}

						if (z != -1) {
							u = z;
						}

						if (u != -1) {
							set_font(H_FSYMB, postavy[u].groupnum == cur_group && !battle ? RGB888(255, 255, 255) : barvy_skupin[postavy[u].groupnum]);
							position(x + 1, y + 1);
							outtext("M");
						}
					
					}
				}
			}
		}
	}
}

static char map_menu_glob_map(int id,int xa,int ya,int xr,int yr) {
	const char *btexts[4] = {texty[210], texty[211], texty[212], texty[213]};

	id = set_select_mode(0);

	if (id) {
		schovej_mysku();
		pick_set_cursor();
		displ_button(1, btexts);
		ukaz_mysku();
		showview(520, 378, 120, 120);
		return 1;
	}

	return 0;
}


static void wire_glob_map_control() {
	const char *btexts[4] = {texty[210], texty[211], texty[212], texty[213]};

	set_select_mode(0);
	schovej_mysku();
	other_draw();
	displ_button(1, btexts);
	wire_global_map();
	ukaz_mysku();
	update_mysky();
	change_click_map(clk_glob_map, CLK_GLOB_MAP);
}


char map_menu(int id,int xa,int ya,int xr,int yr) {
	char *s;
	uint16_t *c;
	const char *btexts[4] = {texty[210], texty[211], texty[212], texty[213]};

	id = set_select_mode(0);
	s = (char*)ablock(H_CHARGENM);
	c = (uint16_t*)ablock(H_CHARGENM);
	s += *c * yr + xr + 6;
	id = *s;

	if (!id) {
		return 1;
	}

	if (id>4) {
		return 1;
	}

	id--;

	if (cur_disables & (1 << id)) {
		return 1;
	}

	if (id) {
		pick_set_cursor();
		displ_button(1, btexts);
	}

	if (cur_mode == MD_ANOTHER_MAP) {
		unwire_proc();
		wire_proc();
	}

	switch (id) {
	case 0:
		wire_glob_map_control();
		break;

	case 1:
		unwire_proc();
		show_automap(1);
		break;

	case 2:
		set_select_mode(1);
		schovej_mysku();
		displ_button(5, btexts);
		mouse_set_default(H_MS_WHO);
		ukaz_mysku();
		showview(0, 0, 0, 0);break;

	case 3:
		unwire_proc();
		wire_proc();
		break;
	}

	return 1;
}


char exit_kniha(int id,int xa,int ya, int xr, int yr)
  {
  xa;ya;xr;yr;id;
  unwire_proc();
  wire_proc();
  return 1;
  }

int selected_page;


char page_change(int id,int xa,int ya,int xr,int yr)
  {
  int oldp;
  xa,ya,xr,yr;

  oldp=cur_page;
  /*
  if (*(char *)0x417 & 0x3)
     {
     selected_page=cur_page+id;
     mouse_set_default(H_MS_LIST);
     mouse_set_cursor(H_MS_LIST);
     return 1;
     }
   */
  xa=count_pages();
  xa=((xa-1) & ~1)+1;
  cur_page+=id;
  if (cur_page<1) cur_page=1;
  if (cur_page>xa) cur_page=xa;
  if (cur_page!=oldp) play_sample_at_channel(H_SND_KNIHA,1,100);
  wire_kniha();
  return 1;
  }

#define CLK_KNIHA 3
T_CLK_MAP clk_kniha[]=
  {
  {2,320,0,639,479,page_change,2,-1},
  {-2,0,0,319,479,page_change,2,-1},
  {-1,0,0,639,479,exit_kniha,8,-1},
  };


void unwire_kniha()
{
  hold_timer(TM_FAST_TIMER,0);
  GlobEvent(MAGLOB_AFTERBOOK,viewsector,viewdir);
}


void wire_kniha()
  {
  int xa;
  if (!GlobEvent(MAGLOB_BEFOREBOOK,viewsector,viewdir))
  {
    return;
  }
  xa=count_pages();
  xa=((xa-1) & ~1)+1;
  if (cur_page<1) cur_page=1;
  if (cur_page>xa) cur_page=xa;
  mute_all_tracks(0);
  unwire_proc();
  schovej_mysku();
  put_picture(0, 0, (uint16_t*)ablock(H_KNIHA));
  change_click_map(clk_kniha,CLK_KNIHA);
  unwire_proc=unwire_kniha;
  set_font(H_FONT6,NOSHADOW(0));
  write_book(cur_page);
  ukaz_mysku();
  showview(0,0,0,0);
  hold_timer(TM_FAST_TIMER,1);
  }

static int last_selected;

char map_target_cancel(int id,int xa,int ya,int xr,int yr)
  {
  id,xa,ya,xr,yr;
  return exit_wait=1;
  }

void map_teleport_keyboard(EVENT_MSG *msg,void **usr) {
	if (msg->msg == E_KEYBOARD) {
		char c;
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		switch (c) {
		case 1:
		case 15:
		case 50:
			exit_wait = 1;
			msg->msg = -1;
			break;
		}
	}
}


static char path_ok(uint16_t sector) {
	gameMap.setCoordFlags(sector, MC_MARKED);
	return 1;
}

char map_target_select(int id, int xa, int ya, int xr, int yr) {
	int x1, y1, x2, y2;

	for (id = 1; id < gameMap.coordCount(); id++) {
		if (gameMap.coord()[id].flags & (MC_AUTOMAP | MC_DISCLOSED)) {
			x1 = (gameMap.coord()[id].x * 8 - map_xr);
			y1 = (gameMap.coord()[id].y * 8 - map_yr);
			x1 += 320;
			y1 += 197 - SCREEN_OFFLINE;
			x2 = x1 + 8;
			y2 = y1 + 8;

			if (xr >= x1 && xr <= x2 && yr >= y1 && yr <= y2) {
				if (!labyrinth_find_path(viewsector, id, SD_PLAY_IMPS, path_ok, NULL)) {
					return 0;
				}

				last_selected = id;
				exit_wait = 1;
				return 1;
			}
		}
	}

	return 0;
}


int select_teleport_target() {
	*otevri_zavoru = 1;
	unwire_proc();
	disable_all_map();
	labyrinth_find_path(viewsector, 65535, SD_PLAY_IMPS, path_ok, NULL);
	gameMap.setCoordFlags(viewsector, MC_MARKED);
	schovej_mysku();
	send_message(E_ADD, E_KEYBOARD, map_teleport_keyboard);
	show_automap(0);
	change_click_map(clk_teleport_view, CLK_TELEPORT_VIEW);
	last_selected = 0;
	ukaz_mysku();
	escape();
	send_message(E_DONE, E_KEYBOARD, map_teleport_keyboard);
	disable_all_map();
	return last_selected;
}
