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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
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

static titlefont=H_FBIG;

#define TITLE_HEAD 1
#define TITLE_NAME 2
#define TITLE_TEXT 3
#define TITLE_CENTER 0
#define TITLE_KONEC 4

static title_mode=0;
static title_line=0;

#define CLK_MAIN_MENU 4

static char vymacknout(int id,int xa,int ya,int xr,int yr)
  {

  for(id=0;id<5;id++) cur_dir[id]=UNSELECT;
  xa,ya,xr,yr;
  return 1;
  }

static char promacknuti(int id,int xa,int ya,int xr,int yr)
  {
  char *z;
  uint16_t *w;

  z=ablock(H_MENU_MASK);w=(uint16_t *)z;
  z+=6+512;
  z+=xr+yr*w[0];
  vymacknout(id,xa,ya,xr,yr);
  if (*z!=0) cur_dir[*z-1]=SELECT;
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

void rozdily(uint8_t *orign,uint8_t *obr,uint16_t *hicolor,uint16_t *xtab,int pocet)
//#pragma aux rozdily parm[EDX][ESI][EDI][EBX][ECX]=
{
/*
__asm
  {
  mov edx,orign
  mov esi,obr
  mov edi,hicolor
  mov ebx,xtab
  mov ecx,pocet

jp1:lodsb 
  xor  al,[edx]
  movzx eax,al
  inc   edx
  movzx eax,short ptr[ebx+eax*2]
  stosw
  dec  ecx
  jnz  jp1
  }
*/

	// TODO: needs testing
	int i;
	for (i = 0; i < pocet; i++) {
		hicolor[i] = xtab[obr[i] ^ orign[i]];
	}
}

static void nahraj_rozdilovy_pcx(void **pp,long *s)
  {
  char *org,*pos;
  int8_t *vysl;
  uint16_t *size,*paltab;
  uint16_t *hicolor,*p;
  void *origin;
  int siz;

  load_pcx((char *)*pp,*s,A_8BIT,&vysl);
  size=(uint16_t *)vysl;
  free(*pp);
  siz=size[0]*size[1];
  *s = siz * 2 + 12;
//  p=hicolor=getmem(siz*2+12);
  p=hicolor=getmem(*s);
  *p++=size[0];
  *p++=size[1];
  *p++=16;
  origin=ablock(H_ANIM_ORIGN);
  org=(char *)origin+6+512;
  pos=(char *)vysl+6+512;
  paltab=(uint16_t *)vysl+3;
  rozdily(org,pos,hicolor+3,paltab,siz);
  free(vysl);
  *pp=hicolor;
//  *s=_msize(*pp);
  }


static void init_menu_entries(void)
  {
  int i;
  char *a;
  def_handle(H_ANIM_ORIGN,"LOGO00.PCX",pcx_8bit_decomp,SR_BGRAFIKA);
  def_handle(H_ANIM,"LOGO00.PCX",pcx_15bit_decomp,SR_BGRAFIKA);
  a=alloca(15);
  for(i=1;i<30;i++)
     {
     sprintf(a,"LOGO%02d.PCX",i);
     def_handle(H_ANIM+i,a,nahraj_rozdilovy_pcx,SR_BGRAFIKA);
     }
  def_handle(H_MENU_BAR,"MAINMENU.PCX",pcx_8bit_decomp,SR_BGRAFIKA);
  def_handle(H_MENU_MASK,"MENUVOL5.PCX",pcx_8bit_decomp,SR_BGRAFIKA);
  for(i=0;i<5;i++)
     {
     sprintf(a,"MENUVOL%d.PCX",i);
     def_handle(H_MENU_ANIM+i,a,pcx_15bit_decomp,SR_BGRAFIKA);
     }
  }

void zobraz_podle_masky_asm(uint8_t barva,uint16_t *scr,uint16_t *data, uint8_t *maska,int xs,int ys)
//#pragma aux zobraz_podle_masky_asm parm[al][edi][esi][ebx][edx][ecx]=
  {
/*
  __asm
    {
    mov  al,barva
    mov  edi,scr
    mov  esi,data
    mov  ebx,maska
    mov  edx,xs
    mov  ecx,ys
    push ebp
    mov  ebp,edx
    jp3: cmp  al,[ebx]
    jnz  jp1
    movsw   
    jmp  jp2
jp1: add  edi,2
    add  esi,2
jp2: inc  ebx
    dec  edx
    jnz  jp3
    mov  edx,ebp
    add  edi,scr_linelen
    sub  edi,edx
    sub  edi,edx
    dec  ecx
    jnz  jp3
    pop  ebp
    }
*/

	// TODO: needs testing
	int x, y;
	for (y = 0; y < ys; y++) {
		for (x = 0; x < xs; x++) {
			if (barva == maska[x+y*xs]) {
				scr[x+y*Screen_GetXSize()] = data[x+y*xs];
			}
		}
	}
  }

static void zobraz_podle_masky(char barva,char anim)
  {
  char *maska;
  uint16_t *data;
  uint16_t *obr=Screen_GetAddr()+300*Screen_GetXSize()+220;
  uint16_t xs,ys;

  alock(H_MENU_MASK);
  maska=ablock(H_MENU_MASK);
  data=ablock(H_MENU_ANIM+anim);
  xs=data[0];
  ys=data[1];
  zobraz_podle_masky_asm(barva,obr,data+3,maska+6+512,xs,ys);
  aunlock(H_MENU_MASK);
  }

static void prehraj_animaci_v_menu(EVENT_MSG *msg,char **unused)
  {
  static counter=0;
  unused;
  if (msg->msg==E_TIMER)
     {
     if (counter % SPEED==0)
        {
        int i=counter/SPEED;char show=0;

        schovej_mysku();
        if (!low_mem || ~i & 1)put_picture(0,56,ablock(H_ANIM+i));
        do_events();
        for(i=0;i<5;i++)
           {
           cur_pos[i]+=cur_dir[i];
           if (cur_pos[i]<0) cur_pos[i]=0;
           else if (cur_pos[i]>4) cur_pos[i]=4;
           else
              {
              zobraz_podle_masky(i+1,cur_pos[i]);
              do_events();
              show=1;
              }
           }
        ukaz_mysku();
        update_mysky();
        showview(0,56,640,250);
        if (show) showview(220,300,206,178);
        }
     counter++;
     if (counter>=(SPEED*30)) counter=0;
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

static void klavesnice(EVENT_MSG *msg,void **unused)
  {
  short cursor,i;
  unused;


  if (msg->msg==E_KEYBOARD)
     {
     for(cursor=0;cursor<5;cursor++) if (cur_dir[cursor]==SELECT) break;
     if (cursor==5) cursor=-1;

     switch(*((char *)msg->data+1))
        {
        case 'H':cursor--;if (cursor<0) cursor=0;break;
        case 'P':cursor++;if (cursor>4) cursor=4;break;
        case 28:
        case 57:click(0,0,0,0,0);return;
        }
     for(i=0;i<5;i++) if (i==cursor) cur_dir[i]=SELECT;else cur_dir[i]=UNSELECT;
     }
  }

int enter_menu(char open)
  {
  int c;
  char *d;
  init_menu_entries();
//  Task_Add(2048,preload_anim);
//  load_ok=0;
//  while(!load_ok) Task_Sleep(NULL);
  preload_anim();
  if (!open)
    {
    play_next_music(&d);
    Sound_ChangeMusic(d);
    }
  update_mysky();
  schovej_mysku();
  curcolor=0;bar(0,0,639,479);
  put_picture(0,0,ablock(H_MENU_BAR));
  put_picture(0,56,ablock(H_ANIM));
  ukaz_mysku();
  if (open) effect_show(NULL);else showview(0,0,0,0);
  change_click_map(clk_main_menu,CLK_MAIN_MENU);
  send_message(E_ADD,E_TIMER,prehraj_animaci_v_menu);
  send_message(E_ADD,E_KEYBOARD,klavesnice);
  ms_last_event.event_type=0x1;
  send_message(E_MOUSE,&ms_last_event);
  Task_WaitEvent(E_MENU_SELECT);
	c = cur_click;
  disable_click_map();
  send_message(E_DONE,E_KEYBOARD,klavesnice);
  cur_dir[c]=UNSELECT;
  while (cur_pos[c]) Task_WaitEvent(E_TIMER);
  Task_WaitEvent(E_TIMER);
  send_message(E_DONE,E_TIMER,prehraj_animaci_v_menu);
  return c;
  }

char *get_next_title(char control,char *filename)
  {
  static FILE *titles=NULL;
  static ENCFILE fl;
  static char buffer[81];
	char *path, *c, *p1, *p2;

  switch(control)
     {
     case 1:
//            concat(path,pathtable[SR_MAP],filename);
//            titles=enc_open(path,&fl);
		p2 = Sys_FullPath(SR_MAP, filename);
		p1 = alloca((strlen(p2) + 1) * sizeof(char));
		strcpy(p1, p2);
		titles = enc_open(p1, &fl);
		if (!titles) {
//              concat(path,pathtable[SR_DATA],filename);
//              titles=enc_open(path,&fl);
			p2 = Sys_FullPath(SR_DATA, filename);
			titles = enc_open(p2, &fl);
		}

		if (!titles) {
			char popis[300];
			closemode();
			sprintf(popis, "Soubor nenalezen: %s ani %s\n", p1, p2);
			Sys_ErrorBox(popis);
//			MessageBox(NULL,popis,NULL,MB_OK|MB_ICONSTOP);
			exit(1);
		}

		// FIXME: this is disgusting
		return (char *)titles;
     case 0:
     	if (titles != NULL) {
		fgets(buffer, 80, titles);
	}

        c = strchr(buffer, '\n');
	if (c != NULL) {
		if (c != buffer && c[-1] == '\r') {
			c[-1] = 0;
		} else {
			*c = 0;
		}
	}

	// fix some garbage at the end of credits ENC file
        c = strchr(buffer, 0x1a);
	if (c != NULL) {
		*c = 0;
	}

        return buffer;
     case -1:if (titles!=NULL)enc_close(&fl);
            break;
     }
  return NULL;
  }

static int title_lines[640][2];

static int insert_next_line(int ztrata) {
	char *c;
	int ll = -1;
	Screen_SetBackAddr();

	do {
		if (title_mode != TITLE_KONEC) {
			c = get_next_title(0, NULL);
		} else {
			c[0] = 0;
		}

		if (c[0] == '*') {
			strupr(c);
			if (!strcmp(c+1, "HEAD")) {
				title_mode = TITLE_HEAD;
				set_font(titlefont, RGB(146, 187, 142));
			} else if (!strcmp(c+1, "NAME")) {
				title_mode = TITLE_NAME;
				set_font(titlefont, RGB(186, 227, 182));
			} else if (!strcmp(c+1, "CENTER")) {
				title_mode = TITLE_CENTER;
				set_font(titlefont, RGB(255, 248, 240));
			} else if (!strcmp(c+1, "TEXT")) {
				title_mode = TITLE_TEXT;
				set_font(titlefont, RGB(255, 248, 240));
			} else if (!strcmp(c+1, "KONEC")) {
				title_mode = TITLE_KONEC;
			} else if (!strncmp(c+1, "LINE", 4)) {
				sscanf(c+5, "%d", &title_line);
			} else if (!strncmp(c+1, "SMALL", 5)) {
				titlefont = H_FBOLD;
			} else if (!strncmp(c+1, "BIG", 3)) {
				titlefont = H_FBIG;
			} else if (!strncmp(c+1, "SPEED", 5)) {
				sscanf(c+6,"%d",&speedscroll);
			}
		} else {
			ll = text_height(c);
			if (ll == 0) ll = text_height("W");
			if (ll < title_line) ll = title_line;
			curcolor = BGSWITCHBIT;
			charcolors[0] = RGB555(0, 0, 1);
			bar(0, 360 + ztrata, 639, 360 + ztrata + ll);
			switch (title_mode) {
			case TITLE_TEXT:
			case TITLE_HEAD:
				position(50, 360 + ztrata);
				break;

			case TITLE_NAME:
				set_aligned_position(100, 360 + ztrata, 0, 0, c);
				break;

			case TITLE_CENTER:
				set_aligned_position(320, 360 + ztrata, 1, 0, c);
				break;
			}

			outtext(c);
		}
	} while (c[0] == '*');

	Screen_Restore();
	if (title_mode == TITLE_KONEC) ll = -1;
	return ll;
}

static void scan_lines(uint16_t *buffer,int start,int poc) {
	int first, last, i, pocet = poc;
	uint16_t *buf;
	while (pocet--) {
		buf = buffer + start * Screen_GetXSize();
		first = 0;
		last = 0;
		for (i = 0; i < 640; i++) {
			if (!(buf[i] & BGSWITCHBIT)) {
				break;
			}
		}

		if (i != 640) {
			first = i;
			last = i;
			for(; i < 640; i++) {
				if (!(buf[i] & BGSWITCHBIT)) {
					last = i;
				}
			}
		}

		first &= ~1;

		if (last) {
			last += 2;
			last &= ~1;
		}

		title_lines[start][0] = first;
		title_lines[start][1] = last;
		start++;
	}
}

static void get_max_extend(int *l,int *r) {
	int left = 640;
	int right = 0;
	int i;

	for(i = 0; i < 360; i++) {
		left = min(title_lines[i][0], left);
		right = max(title_lines[i][1], right);
	}

	*l = left;
	*r = right;
}

/*
void titles(va_list args)
//#pragma aux titles parm[]
  {
  int send_back=va_arg(args,int);
  char *textname=va_arg(args,char *);
*/

void titles(int send_back, char *textname) {
	void *picture;
	uint16_t *scr, *buff;
	int counter, newc;
	int lcounter = 1;
	char end = 0;
	int l, r;
	
	title_mode = TITLE_CENTER;
	if (get_next_title(1, textname) == NULL) return;
	schovej_mysku();
	speedscroll = 4;
	curcolor = BGSWITCHBIT;
	bar(0,0,639,479);
	Screen_SetBackAddr();
	bar(0,0,639,479);
	Screen_Restore();
	memset(title_lines, 0, sizeof(title_lines));
	def_handle(H_PICTURE, "titulky.pcx", pcx_15bit_decomp, SR_BGRAFIKA);
	alock(H_PICTURE);
	picture = ablock(H_PICTURE);
	put_picture(0, 0, picture);
	effect_show(NULL);
	titlefont = H_FBIG;
	set_font(titlefont, RGB(158, 210, 25));
	charcolors[1] = 0;
	counter = Timer_GetValue();
	newc = counter;
	do {
		int skip;
		scr = Screen_GetXSize() * 60 + Screen_GetAddr();
		buff = Screen_GetBackAddr();
		counter = Timer_GetValue();
		skip = (counter - newc) / speedscroll;
		if (skip > 0) {
			if (skip > 10) skip = 10;

			newc += skip * speedscroll;
			scan_lines(buff, 360, skip);
			scroll_and_copy((uint16_t *)picture + 640 * 60 + 3, buff, scr, 360, skip, title_lines);
			//memcpy(Screen_GetAddr(),buff,480*Screen_GetScan());
			get_max_extend(&l, &r);
			memmove(title_lines, &title_lines[skip], sizeof(title_lines) - skip * sizeof(int) * 2);
			//showview(l,60,r-l+1,360);
			showview(0, 60, 639, 360);
			buff += Screen_GetXSize() * 359;
			memcpy(buff, buff + Screen_GetXSize() * skip, 40 * Screen_GetScan());
			showview(0, 0, 640, 40);
			Task_WaitEvent(E_TIMER);
			counter += skip;
			lcounter -= skip;
		} else if (skip < 0) {
			counter = skip;
		}

		while (lcounter <= 0 && !end) {
			int c;
			c = insert_next_line(lcounter);
			scan_lines(Screen_GetBackAddr(), 360 + lcounter, -lcounter);
			if (c == -1) {
				end = 1;
				lcounter = 360;
			} else {
				lcounter += c;
			}
		}

		// quit on keypress
		if (Input_Kbhit()) {
			Input_ReadKey();
			send_back = 1;
			break;
		}
	} while (!(end && lcounter<=0));
//  while (!(Task_QuitMsg() || (end && lcounter<=0)));
	ukaz_mysku();
	get_next_title(-1, NULL);
	aunlock(H_PICTURE);

	if (!send_back) {
		Task_WaitEvent(E_KEYBOARD);
	}
//  if (send_back)send_message(E_KEYBOARD,27);
}

void run_titles(va_list args)
  {
/*
  int task_id;
  task_id=Task_Add(8196,titles,1,"titulky.TXT");
  Task_WaitEvent(E_KEYBOARD);
  Task_Term(task_id);
*/
	titles(1, "TITULKY.TXT");
  }

void konec_hry()
  {
  int task_id;
  int timer;
  char *d;

  schovej_mysku();
  curcolor=0;
  bar(0,0,639,479);
  effect_show(NULL);
  create_playlist(texty[205]);
  play_next_music(&d);
  Sound_ChangeMusic(d);
  timer=Timer_GetValue();
  while (Timer_GetValue()-timer<150) Task_Sleep(NULL);
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
  curcolor=0;
  bar(0,0,639,479);
  ukaz_mysku();
  effect_show(NULL);
  timer=Timer_GetValue();
  while (Timer_GetValue()-timer<150) Task_Sleep(NULL);
  }

