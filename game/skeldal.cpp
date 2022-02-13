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
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <climits>
#include <unistd.h>
#include "libs/pcx.h"
#include <inttypes.h>
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/memman.h"
#include "libs/sound.h"
#include "libs/strlite.h"
#include "libs/gui.h"
#include "libs/basicobj.h"
#include <ctime>
#include "libs/mgfplay.h"
#include "libs/inicfg.h"
#include "game/globals.h"
#include "game/engine1.h"
#include "game/wizard.h"
#include "game/version.h"
#include "libs/pcx.h"
#include "libs/system.h"

#define CONFIG_NAME SKELDALINI

#define INI_TEXT 1
#define INI_INT 2

#define ERR_GENERAL 1
/*
char def_path[]="";
char graph_path[]="graphics\\";
char basc_graph[]="graphics\\basic\\";
char item_graph[]="graphics\\items\\";
char sample_path[]="samples\\";
char font_path[]="font\\";
char map_path[]="maps\\";
char music_path[]="music\\";
char org_music_path[]="music\\";
char temp_path[]="?";
char enemies_path[]="graphics\\enemies\\";
char video_path[]="video\\";
char dialogs_path[]="graphics\\dialogs\\";
char saves_path[]="";
char work_path[]="";
char cd_path[]="";
char map2_path[]="";
char plugins_path[]="";
char *pathtable[]={def_path,graph_path,sample_path,font_path,map_path,music_path,temp_path,basc_graph,item_graph,enemies_path,video_path,dialogs_path,saves_path,work_path,cd_path,map2_path,plugins_path,org_music_path};
*/

/*
char *pathtable[]=
  {"",
  "graphics\\",
  "graphics\\basic\\",
  "graphics\\items\\",
  "samples\\",
  "font\\",
  "maps\\",
  "music\\",
  "",
  "graphics\\enemies\\",
  "video\\",
  "graphics\\dialogs\\"
  };
*/

StringList texty;

char skip_intro=0;
char autosave_enabled=0;
long game_time=0;
int charmin=3;
int charmax=3;

int autoopenaction=0;
int autoopendata=0;

void redraw_desktop_call(EVENT_MSG*, void**);

TMA_LOADLEV loadlevel;

typedef struct inis
  {
//  char heslo[15];
  char heslo[20];
  int8_t parmtype;
  }INIS;

THE_TIMER timer_tree;


int hl_ptr=H_FIRST_FREE;
int debug_enabled=0;
char sound_detection=1;
int snd_devnum,snd_parm1,snd_parm2,snd_parm3,snd_mixing=22050;
char gamespeed=6;
char gamespeedbattle=0;
char level_preload=1;
int game_extras=0;

char default_map[20]="LESPRED.MAP";

THUMAN postavy[POCET_POSTAV],postavy_save[POCET_POSTAV];
void (*unwire_proc)();
void (*wire_proc)();
char cur_mode,battle_mode;
static int init_music_vol=127,init_gfx_vol=255;
static char full_video=0;
static char titles_on=0;
static char windowed=0;
static char windowedzoom=1;
static char monitor=0;
static int refresh=0;

char *texty_knihy;
static char *patch_file=NULL;
int cur_page=0;

StringList cur_config;

TDREGISTERS registred[]=
  {
    {H_DESK,"desk.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_TOPBAR,"topbar.pcx",pcx_15bit_decomp,SR_BGRAFIKA},
    {H_OKNO,"okno.pcx",pcx_15bit_decomp,SR_BGRAFIKA},
    {H_MS_DEFAULT,"msc_sip.pcx", pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MS_SOUBOJ,"msc_x.pcx", pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MS_WHO,"msc_who.pcx", pcx_8bit_decomp,SR_BGRAFIKA},
//    {H_MS_LIST,"msc_list.pcx", pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MS_ZARE,"msc_zare.pcx", pcx_8bit_decomp,SR_BGRAFIKA},
    {H_KOMPAS,"kompas.pcx", pcx_15bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_S,"sipky_s.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_SV,"sipky_sv.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_SZ,"sipky_sz.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_V,"sipky_v.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_Z,"sipky_z.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_J,"sipky_j.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_BACKMAP,"backmap.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_FBOLD,"sada16.fon",loadFont,SR_FONT},
    {H_FSYMB,"ikones.fon",loadFont,SR_FONT},
    {H_FLITT,"font4x8.fon",loadFont,SR_FONT},
    {H_FLITT5,"font5x8.fon",loadFont,SR_FONT},
    {H_FONT6,"font6x9.fon",loadFont,SR_FONT},
    {H_FONT7,"sada7.fon",loadFont,SR_FONT},
    {H_FTINY,"tiny.fon",loadFont,SR_FONT},
    {H_FKNIHA,"kniha.fon",loadFont,SR_FONT},
    {H_FBIG,"timese.fon",loadFont,SR_FONT},
    {H_IOBLOUK,"ioblouk.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_LODKA,"lodka.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_IDESKA,"ideska.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_IMRIZ1,"imriz1.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RAMECEK,"ramecek.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_BATTLE_BAR,"souboje.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_BATTLE_MASK,"m_souboj.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MZASAH1,"mzasah1.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MZASAH2,"mzasah2.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_MZASAH3,"mzasah3.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_PZASAH,"pzasah.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_BATTLE_CLICK,"souboje2.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SIPKY_END,"sipky.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_LEBKA,"death.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_KOSTRA,"bones.pcx",pcx_fade_decomp,SR_BGRAFIKA},
    {H_RUNEHOLE,"runehole.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RUNEMASK,"runemask.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_POWERBAR,"powerbar.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_POWERLED,"powerled.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SND_SWHIT1,"swd_hit0.wav",wav_load,SR_ZVUKY},
    {H_SND_SWHIT2,"swd_hit1.wav",wav_load,SR_ZVUKY},
    {H_SND_SWMISS1,"swd_mis0.wav",wav_load,SR_ZVUKY},
    {H_SND_SWMISS2,"swd_mis1.wav",wav_load,SR_ZVUKY},
    {H_SND_SIP1,"sip2.wav",wav_load,SR_ZVUKY},
    {H_SND_SIP2,"sip1.wav",wav_load,SR_ZVUKY},
    {H_SND_KNIHA,"kniha.wav",wav_load,SR_ZVUKY},
    {H_SND_OBCHOD,"obchod.wav",wav_load,SR_ZVUKY},
    {H_SND_LEKTVAR,"lektvar.wav",wav_load,SR_ZVUKY},
    {H_SND_TELEPIN,"telepin.wav",wav_load,SR_ZVUKY},
    {H_SND_TELEPOUT,"telepout.wav",wav_load,SR_ZVUKY},
    {H_SND_HEK1M,"jauu1m.wav",wav_load,SR_ZVUKY},
    {H_SND_HEK2M,"jauu2m.wav",wav_load,SR_ZVUKY},
    {H_SND_HEK1F,"jauu1f.wav",wav_load,SR_ZVUKY},
    {H_SND_HEK2F,"jauu2f.wav",wav_load,SR_ZVUKY},
    {H_SND_EAT,"jidlo.wav",wav_load,SR_ZVUKY},
    {H_SND_WEAR,"obleci.wav",wav_load,SR_ZVUKY},
    {H_SND_PUTINV,"put_inv.wav",wav_load,SR_ZVUKY},
    {H_RUNEBAR1,"r_ohen.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RUNEBAR2,"r_voda.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RUNEBAR3,"r_zeme.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RUNEBAR4,"r_vzduch.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_RUNEBAR5,"r_mysl.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SPELLDEF,"spelldef.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_KNIHA,"kniha.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_WINTXTR,"wintxtr.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SAVELOAD,"saveload.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SVITEK,"svitek.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_LOADTXTR,"loadtxtr.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_DIALOG,"dialog.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_DIALOGY_DAT,"dialogy.dat",loadDialogs,SR_MAP},
    {H_SHOP_PIC,"shop.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_TELEPORT,"teleport.mgf",preloadStream,SR_BGRAFIKA},
    {H_FX,"fx.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_CHECKBOX,"checkbox.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SETUPBAR,"volbades.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SOUPAK,"volbasou.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_SETUPOK,"volbazpe.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_POSTUP,"postup.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_LODKA0,"lesda21a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA1,"lesda22a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA2,"lesda23a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA3,"lesda24a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA4,"lesda25a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA5,"lesda26a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA6,"lesda27a.pcx",pcx_fade_decomp,SR_GRAFIKA},
    {H_LODKA7,"lesda28a.pcx",pcx_fade_decomp,SR_GRAFIKA},
//    {H_FLETNA,"fletna.wav",wav_load,SR_ZVUKY},
    {H_FLETNA_BAR,"stupnice.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_FLETNA_MASK,"stupni_m.pcx",pcx_8bit_nopal,SR_BGRAFIKA},
    {H_SND_SEVER,"sever.wav",wav_load,SR_ZVUKY},
    {H_SND_VYCHOD,"vychod.wav",wav_load,SR_ZVUKY},
    {H_SND_JIH,"jih.wav",wav_load,SR_ZVUKY},
    {H_SND_ZAPAD,"zapad.wav",wav_load,SR_ZVUKY},
    {H_SND_RAND1,"random1.wav",wav_load,SR_ZVUKY},
    {H_SND_RAND2,"random2.wav",wav_load,SR_ZVUKY},
    {H_SND_RAND3,"random3.wav",wav_load,SR_ZVUKY},
    {H_SND_RAND4,"random4.wav",wav_load,SR_ZVUKY},
    {H_CHARGEN,"chargen.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_CHARGENB,"chargenb.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
    {H_CHARGENM,"chargenm.pcx",pcx_8bit_nopal,SR_BGRAFIKA},
		{H_KREVMIN,"krevmin.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
		{H_KREVMID,"krevmid.pcx",pcx_8bit_decomp,SR_BGRAFIKA},
		{H_KREVMAX,"krevmax.pcx",pcx_8bit_decomp,SR_BGRAFIKA},

	};

INIS sinit[]=
  {
  {"VMODE",INI_INT},
  {"ZOOM_SPEED",INI_INT},
  {"TURN_SPEED",INI_INT},
  {"MUSIC_VOLUME",INI_INT},
  {"SOUND_VOLUME",INI_INT},
  {"SOUND_DEVICE",INI_TEXT},
  {"SOUND_MIXFREQ",INI_INT},
  {"DEFAULT_MAP",INI_TEXT},
  {"GAME_SPEED",INI_INT},
  {"PRELOAD",INI_INT},
  {"INSTALL",INI_TEXT},
  {"PATCH",INI_INT},
  {"SKIP_INTRO",INI_INT},
  {"AUTOSAVE",INI_INT},
  {"DEBUG",INI_INT},
  {"FULLRESVIDEO",INI_INT},
  {"PATCH_FILE",INI_TEXT},
  {"TITLES",INI_INT},
  {"CHAR_MIN",INI_INT},
  {"CHAR_MAX",INI_INT},
  {"EXTRAS",INI_INT},
  {"WINDOWED", INI_INT},
  {"BATTLE_ACCEL",INI_INT},
  {"WINDOWEDZOOM", INI_INT},
  {"MONITOR",INI_INT},
  {"VERSION",INI_INT},
  {"REFRESHRATE",INI_INT},
  {"CESTA_DATA",INI_TEXT},
  {"CESTA_GRAFIKA",INI_TEXT},
  {"CESTA_ZVUKY",INI_TEXT},
  {"CESTA_FONTY",INI_TEXT},
  {"CESTA_MAPY",INI_TEXT},
  {"CESTA_MUSIC",INI_TEXT},
  {"CESTA_TEMPY",INI_TEXT},
  {"CESTA_BGRAFIKA",INI_TEXT},
  {"CESTA_ITEMY",INI_TEXT},
  {"CESTA_ENEMY",INI_TEXT},
  {"CESTA_VIDEO",INI_TEXT},
  {"CESTA_DIALOGY",INI_TEXT},
  {"CESTA_POZICE",INI_TEXT},
  {"CESTA_HLAVNI",INI_TEXT},
  {"CESTA_CD",INI_TEXT},
  {"CESTA_MAPY2",INI_TEXT},
  {"CESTA_PLUGINS",INI_TEXT},
  {"CESTA_ORIG_MUSIC",INI_TEXT}
  };


#define CESTY_POS 27
int last_ms_cursor=-1;
int vmode=2;
int map_ret = 0;

int set_video(int mode) {
	int er = 0;

	report_mode(1);
	er = initmode_dx(windowed, windowedzoom, monitor, refresh);
	return er;
}

/*
int ask_video()
  {
  int c;
  printf("\nJaky videomode?:\n"
         "  1) 640x480x256 \n"
         "  2) 640x480xHiColor \n");
  c=_bios_keybrd(_KEYBRD_READ)>>8;
  if (c==1) exit(0);
  return c-1;
  }
*/

DataBlock *pcx_fade_decomp(SeekableReadStream &stream) {
	return load_pcx(stream, A_FADE_PAL, gameMap.global().fade_r, gameMap.global().fade_g, gameMap.global().fade_b);
}

DataBlock *pcx_15bit_decomp(SeekableReadStream &stream) {
	return load_pcx(stream, A_16BIT);
}

DataBlock *pcx_15bit_autofade(SeekableReadStream &stream) {
	return load_pcx(stream, A_16BIT);
}

DataBlock *pcx_8bit_nopal(SeekableReadStream &stream) {
	return load_pcx(stream, A_8BIT_NOPAL);
}


DataBlock *pcx_8bit_decomp(SeekableReadStream &stream) {
	return load_pcx(stream, A_8BIT);
}

DataBlock *hi_8bit_correct(SeekableReadStream &stream) {
	return new TextureHi(stream);
}

void mouse_set_cursor(int cursor) {
	static const Texture *ms_item = NULL;

	if (cursor == last_ms_cursor) {
		return;
	}

	if (last_ms_cursor > 0) {
		aunlock(last_ms_cursor);
	}

	if (cursor > 0) {
		alock(cursor);
		schovej_mysku();
		register_ms_cursor(dynamic_cast<const Texture*>(ablock(cursor)));
		last_ms_cursor = cursor;
		set_ms_finger(0, 0);
		ukaz_mysku();
	} else {
		const IconLib *tmp;

		cursor = -cursor;

		delete ms_item;
		tmp = dynamic_cast<const IconLib*>(ablock(cursor / 18 + ikon_libs));
		const Texture &cur = (*tmp)[cursor % 18];
		ms_item = new SubTexture(cur, 0, 0, cur.width(), cur.height());
		schovej_mysku();
		register_ms_cursor(ms_item);
		set_ms_finger(45 / 2, 55 / 2);
		last_ms_cursor = -cursor;
		ukaz_mysku();
	}
}

void mouse_set_default(int cursor)
  {
  default_ms_cursor=cursor;
  mouse_set_cursor(cursor);
  }

void music_init()
  {
//  char *path;
/*  if (sound_detection)
     {
     SEND_LOG("(SOUND) SOUND_DETECT Detecting sound card",0,0);
     if (sound_detect(&snd_devnum,&snd_parm1,&snd_parm2,&snd_parm3)) snd_devnum=DEV_NOSOUND;
     }*/
//  SEND_LOG("(SOUND) SOUND_SET Setting Sound: Device '%s' Port: %3X",device_name(snd_devnum),snd_parm1);
//  SEND_LOG("(SOUND) SOUND_SET Setting Sound: IRQ: %X DMA: %X",snd_parm2,snd_parm3);
  Sound_SetMixer(snd_devnum,snd_mixing,snd_parm1,snd_parm2,snd_parm3);
  SEND_LOG("(SOUND) SOUND_INIT Starting mixing",0,0);
  Sound_StartMixing();
  Sound_SetEffect(SND_GFX,init_gfx_vol);
  Sound_SetEffect(SND_MUSIC,init_music_vol);
/*
  path=plugins_path;
  if (path==0 || path[0]==0)  
    path=AutodetectWinAmp();
  if (path!=0 && path[0]!=0)
  {
    SEND_LOG("(SOUND) Installing plugins, path: %s",path,0);
    init_winamp_plugins(path);
    if (path!=plugins_path) free(path);
  }  
*/
  SEND_LOG("(SOUND) SOUND_DONE Sound Engine should work now",0,0);

  }

void clrscr()
  {

  }

void back_music(the_timer *arg)
  {
  Sound_MixBack(0);
  }

/*void *anim_idle(EVENT_MSG *msg,void **usr)
  {
  usr;
  if (msg->msg==E_TIMER) gameMap.calcAnimations();
  return &anim_idle;
  }*/

/*void timer_error()
  {
  puts("\x7");
  }
*/
void *timming(EVENT_MSG *msg,void **data) {
	THE_TIMER *p, *q;
	int i, j;
	va_list args;

	if (msg->msg == E_INIT) {
		return (void*)&timming;
	}

	*otevri_zavoru = 1;
	va_copy(args, msg->data);
	j = va_arg(args, int);
	va_end(args);

	for (i = 0; i < j; i++) {
		p = &timer_tree;
		q = p->next;

		while (q != NULL) {
			p = q->next;

			if (!(--q->counter))
			if (q->zavora && i==(j-1)) {
				q->zavora = 0;

				if (q->calls != -2) {
					q->proc(q);
				}

				p = q->next;
				q->zavora = 1;
				q->counter = q->count_max;
				if (q->calls != -1) {
					if (--q->calls<1) {
						for(p = &timer_tree; p->next != q; p = p->next);
						p->next = q->next;

						if (q->next == NULL) {
							SEND_LOG("(TIMER) Self remove for timer id: %d, next-><NULL>", q->id, 0);
						} else {
							SEND_LOG("(TIMER) Self remove for timer id: %d, next->%d", q->id, q->next->id);
						}

						free(q);
						q = p;
					}
				}
			} else {
				q->counter=1;
			}

			if (q->next!=p && q!=p) {
				THE_TIMER *z;
				SEND_LOG("(TIMER) Timer integrity corrupted", 0, 0);
				z = &timer_tree;

				while(z->next != p && z->next != NULL) {
					z = z->next;
				}

				if (z->next == NULL) {
					return NULL;
				}
			}

			q = p;
		}
	}
	return NULL;
}

void delete_from_timer(int id)
  {
  THE_TIMER *p,*q;

  p=&timer_tree;
  q=p->next;
  while (q!=NULL)
     {
     if (q->id==id)
              {
              if (q->zavora)
                 {
                 if (q->next==NULL)
                    SEND_LOG("(TIMER) Removing timer id: %d, next-><NULL>",id,0);
                 else
                    SEND_LOG("(TIMER) Removing timer id: %d, next->%d",id,q->next->id);

                 p->next=q->next;
                 free(q);
                 q=p;
                 }
              else
                 {
                 SEND_LOG("(TIMER) Can't remove timer! id: %d. Currently in use.",id,0);
                 q->calls=-2;
                 q->counter=1;
                 }
              }
      p=q;q=q->next;
     }
  }

THE_TIMER *find_timer(int id)
  {
  THE_TIMER *p;

  p=timer_tree.next;
  while (p!=NULL && p->id!=id) p=p->next;
  return p;
  }


void hold_timer(int id,char hld)
  {
  THE_TIMER *q;

  q=timer_tree.next;
  while (q!=NULL && q->id!=id) q=q->next;
  if (q!=NULL) q->counter=1-(hld<<1);
  SEND_LOG("(TIMER) Timer hold id: %d status: %s",id,hld?"Hold":"Unhold");
  }

THE_TIMER *add_to_timer(int id,int delay,int maxcall,void (*proc)(the_timer*))
  {
  THE_TIMER *q;

//  if (id==2 && marker && timer_tree.next->id==2)
//     MARKER_HIT(timer_error());
  q=(THE_TIMER *)getmem(sizeof(THE_TIMER));
  q->counter=q->count_max=delay;
  q->calls=maxcall;
  q->proc=proc;
  q->id=id;
  q->next=timer_tree.next;
  q->zavora=1;
  q->zero=0;
  timer_tree.next=q;
  SEND_LOG("(TIMER) Adding to timer id: %d delay: %d",id,delay);
  return q;
  }

static void kill_timer()
  {
  THE_TIMER *t;

  t=timer_tree.next;
  while (t!=NULL)
     {
     THE_TIMER *p;

     p=t;t=t->next;free(p);
     }
  timer_tree.next=NULL;
  }

void *user_timer(EVENT_MSG *msg,void **usr)
  {
  int x;
  static int lastvalue=0;
  usr;
  if (msg->msg==E_WATCH)
     {
     *otevri_zavoru=1;
     x=Timer_GetValue();
     x-=lastvalue;
     lastvalue+=x;
     if (x) send_message(E_TIMER,x);
     }
  return (void*)&user_timer;
  }

void do_timer()
  {
  EVENT_MSG msg;
  int8_t x;

  msg.msg=E_IDLE;
  otevri_zavoru=&x;
  user_timer(&msg,NULL);
  }

void done_skeldal(void) {
	SEND_LOG("(GAME) Video returned to textmode", 0, 0);
	close_manager();
	close_story_file();
	Sys_PurgeTemps(1);
	Sound_StopMixing();
	texty.clear();
	cur_config.clear();
	kill_timer();
	SEND_LOG("NORMAL TERMINATING--------------------------", 0, 0);
	delete syslog;
	syslog = NULL;
}


int cislovka(int i)
  {
  if (i==1) return 0;
  if (i>1 && i<5) return 1;
  return 2;
  }

void register_basic_data() {
	size_t i, s;
	TDREGISTERS *p;
	char xname[16];

	s = sizeof(registred) / sizeof(TDREGISTERS);
	p = registred;

	for (i = 0; i < s; i++, p++) {
		def_handle(p->h_num, p->name, p->proc, p->path);
	}

	def_handle(H_BOTTBAR, "", bott_draw_proc, 0);

	for (i = 0; i < H_TELEP_CNT; i++) {
		sprintf(xname, "TELEP%02d.PCX", (int)i);
		def_handle(H_TELEP_PCX + i, xname, pcx_fade_decomp, SR_BGRAFIKA);
	}

	for (i = 0; i < H_ARMA_CNT; i++) {
		sprintf(xname, "ARMA%02d.PCX", (int)i);
		def_handle(H_ARMAGED + i, xname, pcx_fade_decomp, SR_BGRAFIKA);
	}

	for (i = 0; i < H_KILL_MAX; i++) {
		sprintf(xname, "KILL%02d.PCX", (int)i);
		def_handle(H_KILL + i, xname, pcx_fade_decomp, SR_BGRAFIKA);
	}

	for (i = 0; i < H_FLETNA_CNT; i++) {
		sprintf(xname, "FLETNA%02d.WAV", (int)i);
		def_handle(H_FLETNA + i, xname, wav_load, SR_ZVUKY);
	}
}

void reg_grafiku_postav()
  {
  int i;
  char xname[16];

  for(i=0;i<POCET_POSTAV;i++)
     {
     undef_handle(i+H_POSTAVY);
     undef_handle(i+H_XICHTY);
     undef_handle(i+H_CHARS);
     }
  for(i=0;i<POCET_POSTAV;i++) def_handle(i+H_POSTAVY,"",build_items_called,0);
  for(i=0;i<POCET_POSTAV;i++)
     {
     sprintf(xname,XICHT_NAME,postavy[i].xicht);
     def_handle(i+H_XICHTY,xname,pcx_8bit_decomp,SR_BGRAFIKA);
     sprintf(xname,CHAR_NAME,postavy[i].xicht);
     def_handle(i+H_CHARS,xname,pcx_fade_decomp,SR_BGRAFIKA);
     }
  }

void cti_texty()
  {
	char *path;
	int err;

	path = Sys_FullPath(SR_DATA, TEXTY);
  if ((err = load_string_list_ex(texty, path)) !=0 )
     {
	 char buff[256];
     closemode();
     switch (err)
        {
        case -1:sprintf(buff,"Can't load string table. File %s has not been found\n",path);break;
        case -2:sprintf(buff,"Missing end mark (-1) at the end of string table\n");break;
        case -3:sprintf(buff,"Memory very low (need min 4MB)\n");break;
        default:sprintf(buff,"Error in string table at line %d\n",err);break;
        }
    Sys_ErrorBox(buff);
//    MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP);
     exit(1);
     }
  }


void global_kbd(EVENT_MSG *msg, void **usr) {
	char c;
	
	if (msg->msg == E_KEYBOARD) {
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		if (c == ';') {
			save_dump();
		}
	}
	return;
}

void add_game_window() {
	WINDOW *p;
	CTL3D *c;

	c = def_border(0, 0, 0, 0);
	p = create_window(0, 0, 0, 0, 0, 0, 0, c);
	desktop_add_window(p);
}



void error_exception(EVENT_MSG *msg,void **unused)
  {
  if (msg->msg==E_PRGERROR)
     {
     unused;
     SEND_LOG("(ERROR) Runtime error detected ... Game terminator lunched.",0,0);
     SEND_LOG("(ERROR) Log: Now dump of useful informations:",0,0);
     SEND_LOG("(ERROR) Log: Map name '%s'",gameMap.fname()==NULL?"<NULL>":gameMap.fname(),0);
     SEND_LOG("(ERROR) Log: Sector %d Direction %d",viewsector,viewdir);
     SEND_LOG("(ERROR) Log: Last 'memman' handle: %x",memman_handle,0);
     SEND_LOG("(ERROR) Log: Battle: %d Select_player %d",battle,select_player);
     closemode();
     printf("Program zpñsobil bàhovou chybu a bude ukonáen\n"
            "Posledn° zpracov†van† data màla rukojeü á°slo %xh\n",memman_handle);
     printf("Map: %s Sector %d Direction %d\n",gameMap.fname()==NULL?"<unknown>":gameMap.fname(),viewsector,viewdir);
     printf("Nyn° se program pokus° uloëit hru...\n\n");
     autosave_enabled=1;
     autosave();
     printf("Hra byla £spà®nà uloëena pod n†zvem AUTOSAVE\n");
     exit(0);
     }
  }

Font *boldcz;

#define ERR_WINX 320
#define ERR_WINY 100

/*

char device_error(int chyba,char disk,char info)
  {
  char c;
  void *old;

  old=_STACKLOW;
  _STACKLOW=NULL;
  chyba,disk,info;
  curfont=&boldcz;
  charcolors[0]=0xffff;
  for(c=1;c<5;c++) charcolors[c]=0x7fff;
  memcpy(buffer_2nd,screen,screen_buffer_size);
  trans_bar(320-ERR_WINX/2,240-ERR_WINY/2,ERR_WINX,ERR_WINY,0);
  curcolor=0x7fff;
  rectangle(320-ERR_WINX/2,240-ERR_WINY/2,320+ERR_WINX/2,240+ERR_WINY/2,0x7fff);
  set_aligned_position(320,230,1,1,texty[8]);outtext(texty[8]);
  set_aligned_position(320,250,1,1,texty[9]);outtext(texty[9]);
  showview(0,0,0,0);
  do
     {
     c=getche();
     }
  while (c!=13 && c!=27);
  memcpy(screen,buffer_2nd,screen_buffer_size);
  showview(0,0,0,0);
  _STACKLOW=old;
  return (c==13?_ERR_RETRY:_ERR_FAIL);
  }
*/
static void patch_error(int err) {
	uint8_t fontPal[FONT_COLORS][3];

	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 460, 640, 479);
	memcpy(fontPal, flat_color(255, 255, 255), sizeof(fontPal));
	renderer->setFont(boldcz, 1, fontPal);

	switch(err) {
	case 0:
		renderer->drawText(0, 460, "File has been patched: ");
		renderer->drawText(renderer->textWidth("File has been patched: "), 460, patch_file);
		break;

	case 1:
		renderer->drawText(0, 460, "Patch error within file: ");
		renderer->drawText(renderer->textWidth("Patch error within file: "), 460, patch_file);
		break;

	case 2:
		renderer->drawText(0, 460, "Cannot patch");
		break;

	case 3:
		renderer->drawText(0, 460, "Missing or error in main data file, patching ingnored!");
		break;
	break;
	}

	showview(0, 460, 640, 20);
}

Font *LoadDefaultFont(void) {
	SeekableReadStream *stream = afile("BOLDCZ.FON", SR_FONT);
	Font *ret = new Font(*stream);
	delete stream;
	return ret;
}

void init_skeldal(void)
  {
//  char c[200],d[200];
	char *c, *d;
  int verr;

SEND_LOG("(INIT) Reading texts.",0,0);
  cti_texty();
  timer_tree.next=NULL;
SEND_LOG("(INIT) Setting random seed.",0,0);
  srand(clock());
SEND_LOG("(INIT) Init message system - event handler",0,0);
  init_events();
SEND_LOG("(INIT) Setting videomode.",0,0);
  verr=set_video(vmode);
  if (verr)
     {
     exit(ERR_GENERAL);
     }
SEND_LOG("(INIT) Initializing engine.",0,0);
  general_engine_init();
  atexit(done_skeldal);
/*SEND_LOG("(INIT) Loading DOS error handler.",0,0);
  install_dos_error(device_error,(char *)getmem(4096)+4096);*/
//  sprintf(c,"%s%s",SWAPPATH,TEMP_FILE);
//  sprintf(d,"%s%s",pathtable[SR_DATA],"skeldal.ddl");
	d = Sys_FullPath(SR_TEMP, TEMP_FILE);
	c = (char*)alloca((strlen(d) + 1) * sizeof (char));
	strcpy(c, d);
	d = Sys_FullPath(SR_DATA, "SKELDAL.DDL");
SEND_LOG("(INIT) Initializing memory manager",0,0);
  init_manager(d,c);
  boldcz=LoadDefaultFont();
SEND_LOG("(GAME) Memory manager initialized. Using DDL: '%s' Temp dir: '%s'",d,c);
	texty_knihy=find_map_path("KNIHA.TXT");
SEND_LOG("(INIT) Installing GUI",0,0);
  install_gui();
SEND_LOG("(INIT) Attaching patch.",0,0);
  if (patch_file!=NULL) patch_error(add_patch_file(patch_file));
SEND_LOG("(INIT) Registring basic data.",0,0);
  register_basic_data();
SEND_LOG("(INIT) Timer event handler.",0,0);
  send_message(E_DONE,E_WATCH,timer);
  send_message(E_DONE,E_IDLE,redraw_desktop_call);
  send_message(E_ADD,E_TIMER,timming);
SEND_LOG("(INIT) User timer.",0,0);
  send_message(E_ADD,E_WATCH,user_timer);
SEND_LOG("(INIT) Mouse clicking maps.",0,0);
  send_message(E_ADD,E_MOUSE,ms_clicker);
SEND_LOG("(INIT) Global keyboard event handler.",0,0);
  send_message(E_ADD,E_KEYBOARD,global_kbd);
SEND_LOG("(INIT) Error exception event handler.",0,0);
  send_message(E_ADD,E_PRGERROR,error_exception);
SEND_LOG("(INIT) Wizard handler.",0,0);
// TODO: rewrite
//  if (debug_enabled) install_wizard();
SEND_LOG("(INIT) Background music timer.",0,0);
  add_to_timer(TM_BACK_MUSIC,5,-1,back_music);
SEND_LOG("(INIT) Creating game window.",0,0);
  add_game_window();
SEND_LOG("(INIT) Music.",0,0);
  music_init();
SEND_LOG("(INIT) Mouse interrupt handler.",0,0);
  if ((verr=init_mysky())!=0)
     {
     closemode();
     puts(texty[174-verr]);
     SEND_LOG("(ERROR) %s (%d)",texty[174-verr],verr);
     SEND_LOG("(ERROR) Mouse not found, shutting down.",0,0);
     exit(0);
     }
SEND_LOG("(INIT) Mouse initialized.",0,0);
SEND_LOG("(INIT) Loading mouse cursor.",0,0);
  mouse_set_default(H_MS_DEFAULT);
  ukaz_mysku();
  konec_skladby=play_next_music;
SEND_LOG("(INIT) Loading spells.",0,0);
  kouzla_init();
SEND_LOG("(INIT) Loading items.",0,0);
  load_items();
SEND_LOG("(INIT) Loading shops.",0,0);
  load_shops();
  Mouse_MapWheel('H','P');
  }

void wire_main_functs();
void unwire_main_functs()
  {
  SEND_LOG("(SYS) Wire main functions",0,0);
  delete_from_timer(TM_FLY);
  delete_from_timer(TM_SCENE);
  delete_from_timer(TM_REGEN);
  send_message(E_DONE,E_KEYBOARD,game_keyboard);
  send_message(E_DONE,E_KROK,real_krok);
  disable_click_map();
  cancel_render=1;
  wire_proc=wire_main_functs;
  }


void wire_main_functs()
  {
  SEND_LOG("(SYS) unWire main functions",0,0);
  add_to_timer(TM_SCENE,gamespeed,-1,refresh_scene);
  add_to_timer(TM_FLY,gamespeed,-1,calc_fly);
  add_to_timer(TM_REGEN,500,-1,real_regeneration);
  send_message(E_ADD,E_KEYBOARD,game_keyboard);
  send_message(E_ADD,E_KROK,real_krok);
  change_click_map(clk_main_view,CLK_MAIN_VIEW);
  unwire_proc=unwire_main_functs;
  cur_mode=MD_GAME;
  running_battle=0;
  recalc_volumes(viewsector,viewdir);
  cancel_pass=1;
  }


void init_game()
  {
  SEND_LOG("(INIT) Inventory.",0,0);
  init_inventory();
  SEND_LOG("(INIT) Characters.",0,0);
  reg_grafiku_postav();
  build_all_players();
  }

void *map_keyboard(EVENT_MSG *msg,void **usr);

char doNotLoadMapState=0;

static void reload_map_handler(EVENT_MSG *msg, void **usr) {
	extern char running_battle;

	if (msg->msg == E_RELOADMAP) {
		int i;
		va_list list;
		const char *fname;
		int sektor;

		va_copy(list, msg->data);
		fname = va_arg(list, const char *);
		sektor = va_arg(list, int);
		va_end(list);

		strncpy(loadlevel.name, fname, sizeof(loadlevel.name));
		loadlevel.start_pos = sektor;

		for (i = 0; i < POCET_POSTAV; i++) {
			postavy[i].sektor = loadlevel.start_pos;
		}

		SEND_LOG("(WIZARD) Load map '%s' %d",loadlevel.name,loadlevel.start_pos);
		unwire_proc();    

		if (battle) {
			konec_kola();
		}

		battle = 0;
		running_battle = 0;
		doNotLoadMapState = 1;
		hl_ptr = ikon_libs;
		destroy_fly_map();
		load_items();
		kill_block(H_SHOP_PIC);
		kill_block(H_DIALOGY_DAT);
		load_shops();
		send_message(E_CLOSE_MAP);
	}
}

void enter_game()
  {
  int end;
  chod_s_postavama(0);
  bott_draw(1);
  norefresh=0;
  Task_WaitEvent(E_TIMER);
  redraw_scene();
  wire_main_functs();
  add_to_timer(TM_FAST_TIMER,2,-1,objekty_mimo);
  cancel_pass=0;
  autosave();
  set_game_click_map();
  SEND_LOG("(GAME) --------- Waiting for E_CLOSE_MAP ------------\n",0,0);
  send_message(E_ADD,E_RELOADMAP,reload_map_handler);
  Task_WaitEvent(E_CLOSE_MAP);
	end = map_ret;
  send_message(E_DONE,E_RELOADMAP,reload_map_handler);
  SEND_LOG("(GAME) --------- E_CLOSE_MAP triggered, leaving map------------\n",0,0);
  unwire_main_functs();
  delete_from_timer(TM_FAST_TIMER);
  cancel_pass=1;
  Task_WaitEvent(E_TIMER);
  Task_WaitEvent(E_TIMER);
  unwire_main_functs();
  mute_all_tracks(1);
  if (end==255) konec_hry();
  }

/*int dos58(int mode);
#pragma aux dos58=\
  "mov  al,1"\
  "mov  ah,58h"\
  "int  21h"\
 parm[ebx] value [eax]
*/

static int do_config_skeldal(int num, int numdata, char *txt) {
	int defdir;

	switch (num) {
	case 0:
		vmode = numdata;
		break;

	case 1:
		zoom_speed(numdata);
		break;

	case 2:
		turn_speed(numdata);
		break;

	case 3:
		init_music_vol = numdata;
		break;

	case 4:
		init_gfx_vol = numdata;
		break;

	case 5:
		sscanf(txt, "%d %x %d %d", &snd_devnum, &snd_parm1, &snd_parm2, &snd_parm3);
		sound_detection = 0;
		break;

	case 6:
		snd_mixing = numdata;
		break;

	case 7:
		strncpy(default_map, txt, 20);
		default_map[19] = '\0';
		SEND_LOG("(GAME) Start map sets as '%s'", default_map, 0);
		break;

	case 8:
		gamespeed = numdata;
		break;

	case 9:
		level_preload = numdata;
		break;

//     case 10:system(txt);break;
	case 11:
		mman_patch = numdata;
		break;

	case 12:
		skip_intro = numdata;
		break;

	case 13:
		autosave_enabled = numdata;
		break;

	case 14:
		debug_enabled = numdata;
		break;

	case 15:
		full_video = numdata;
		break;

	case 16:
		txt = Sys_DOSPath(SR_DEFAULT, txt);
		patch_file = (char*)getmem(strlen(txt) + 1);
		strcpy(patch_file, txt);
		txt = strchr(patch_file, '\n');

		if (txt != NULL) {
			txt[0] = 0;
		}
		break;

	case 17:
		titles_on = numdata;
		break;

	case 18:
		charmin = numdata;
		break;

	case 19:
		charmax = numdata;
		break;

	case 20:
		game_extras = numdata;
		break;

	case 21:
		windowed = numdata;
		break;

	case 22:
		gamespeedbattle = numdata;
		break;

	case 23:
		windowedzoom = numdata;
		break;

	case 24:
		monitor = numdata;
		break;

	case 25:
		if (VERSIONNUM < numdata) {
			Sys_InfoBox("Pozor! Hra je starsi verze, nez vyzaduje dobrodruzstvi. Ve vlastnim zajmu si stahnete novou verzi, protoze toto dobrodruzstvi nemusi byt s aktualni verzi dohratelne");
//               MessageBox(NULL,"Pozor! Hra je starsi verze, nez vyzaduje dobrodruzstvi. Ve vlastnim zajmu si stahnete novou verzi, protoze toto dobrodruzstvi nemusi byt s aktualni verzi dohratelne","Chybna verze hry",MB_OK);
		}
		break;

	case 26:
		refresh = numdata;
		break;

	default:
		num -= CESTY_POS;
		defdir = num == SR_SAVES || num == SR_TEMP || num == SR_WORK ? SR_HOME : SR_DEFAULT;
		Sys_SetPath(num, Sys_DOSPath(defdir, txt));
/*
          mman_pathlist[num]=(char *)getmem(strlen(txt)+1);
          strcpy(mman_pathlist[num],txt);
*/
		SEND_LOG("(GAME) Directory '%s' has been assigned to group nb. %d",txt,num);
		break;
	}

	return 0;
}


static void config_skeldal(const char *line)
  {
  int ndata=0;
  size_t i,maxi;

  char *data=0;
  const char *c;

  c=strchr(line,' ');if (c==NULL) return;
  c++;
  maxi=strlen(c);
  data = (char*)alloca(maxi+1);
  strcpy(data,c);
  if (data[maxi-1]=='\n') data[maxi-1]=0;
  maxi=(sizeof(sinit)/sizeof(INIS));
  for(i=0;i<maxi;i++) if (comcmp(line,sinit[i].heslo)) break;
  if (i==maxi)
     {
     char s[256];
     i = c - line - 1;

     strcpy(s,"Chyba v INI souboru: Neznama promenna - ");
     strncat(s,line,i);
     SEND_LOG("(ERROR) %s",s,NULL);
     }
  else
     {
     if (sinit[i].parmtype==INI_INT) if (sscanf(data,"%d",&ndata)!=1)
        {
        char s[256];

        sprintf(s,"Chyba v INI souboru: Ocekava se ciselna hodnota\n%s\n",line);
        SEND_LOG("(ERROR) %s",s,NULL);
        }
     do_config_skeldal(i,ndata,data);
     }
  }

static void configure(StringList &lines, char *filename) {
	SEND_LOG("(GAME) Reading config. file '%s'", filename, NULL);

	if (!read_config(lines, filename)) {
		char s[256];

		sprintf(s,"\nNemohu precist konfiguracni soubor \"%s\".\n",filename);
		SEND_LOG("(ERROR) %s",s,NULL);
		puts(s);
//     exit(1);
	}

	SEND_LOG("(GAME) Configuring game...", 0, 0);
	process_ini(lines, config_skeldal);
	SEND_LOG("(GAME) Done config.", 0, 0);
}

static void update_config()
  {
  SEND_LOG("(GAME) Updating config. file '%s'",CONFIG_NAME,NULL);
  add_field_num(cur_config, sinit[1].heslo, zoom_speed(-1));
  add_field_num(cur_config, sinit[2].heslo, turn_speed(-1));
// FIXME: rewrite
//  if (Sound_CheckEffect(SND_MUSIC)) add_field_num(&cur_config,sinit[3].heslo,Sound_GetEffect(SND_MUSIC));
//  if (Sound_CheckEffect(SND_GFX)) add_field_num(&cur_config,sinit[4].heslo,Sound_GetEffect(SND_GFX));
  add_field_num(cur_config, sinit[9].heslo, level_preload);
  add_field_num(cur_config, sinit[13].heslo, autosave_enabled);
  save_config(cur_config,Sys_FullPath(SR_WORK, CONFIG_NAME));
  SEND_LOG("(GAME) Config. file was saved",0,0);
  }

void help()
  {
  printf("Pouziti:\n\n   S <filename.MAP> <start_sector>\n\n"
         "<filename.MAP> jmeno mapy\n"
         "<start_sector> Cislo startovaciho sektoru\n"
         );
  exit(0);
  }

extern char nofloors;

/*
void set_verify(char state);
#pragma aux set_verify parm [eax]=\
                             "mov   ah,2eh"\
                             "int   21h"
*/
void play_movie_seq(char *s, int y) {
	int hic = full_video ? SMD_HICOLOR + 128 : SMD_HICOLOR;
	int cc = full_video ? SMD_256 + 128 : SMD_256;

	switch (vmode) {
	case 1:
		if (!banking) {
			play_animation(s, cc, y, Sound_IsActive());
		} else {
			play_animation(s, hic, y, Sound_IsActive());
		}
		break;
	case 5:
	case 2:
		play_animation(s, hic, y, Sound_IsActive());
		break;
	default:
		play_animation(s, hic, y, Sound_IsActive());
		break;
	}
}


void play_anim(int anim_num) {
	char *s;
	char *t, *z;
	StringList titl;
	const Font *font;

//     concat(s,pathtable[SR_VIDEO],texty[anim_num]);
	s = Sys_FullPath(SR_VIDEO, texty[anim_num]);

	if (!Sound_IsActive() || titles_on) {
		concat(t, s, "   ");
		z = strrchr(t, '.');

		if (z != NULL) {
			strcpy(z, ".TXT");
			load_string_list_ex(titl, t);
		}
	}

	set_title_list(&titl);
	font = dynamic_cast<const Font*>(ablock(H_FBIG));
	renderer->setFont(font, 1,200,200,200);
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 0, 639, 459);
	showview(0, 0, 0, 0);
	play_movie_seq(s, 60);
	set_title_list(NULL);
}


/*main(int argc,char *argv[])
  {
  int err;int sect;int dir=0;

  //if (argc<2) help();

  dir;
  set_verify(0);
  mman_pathlist=&pathtable;
  //nofloors=1;
  zoom_speed(1);
  turn_speed(1);
  configure("skeldal.ini",config_skeldal);
  Sys_PurgeTemps(1);
  textmode_effekt();
  clrscr();
  init_skeldal();
  enter_menu();
  if (argc<2)
     {
     invex_anim();
   //  send_message(E_ADD,E_MOUSE,waiter);
   //  send_message(E_ADD,E_KEYBOARD,waiter);
   //  add_to_timer(TM_WAITER,1,-1,timer_waiter);
     strncpy(loadlevel.name,default_map,12);
     }
  else strncpy(loadlevel.name,argv[1],12);
  err=load_map(loadlevel.name);
  if (argc>=3) sscanf(argv[2],"%d",&sect);
  else
     {
     sect=mglob.start_sector;
     dir=mglob.direction;
     }
  loadlevel.start_pos=sect;
  loadlevel.dir=dir;
  init_game();
  while (loadlevel.name[0])
     {
     if (err)
       {
       closemode();
       switch (err)
          {
          case -1: printf("Error while loading map....file not found\n");break;
          case -2: printf("Missing -1 at the end of map string table");break;
          case -3: printf("Map file is corrupted!\n");break;
          default: printf("Error in string table at line %d",err);break;
          }
       exit(1);
       }
    viewsector=loadlevel.start_pos;
    viewdir=loadlevel.dir;
    loadlevel.name[0]=0;
    enter_game();
    leave_current_map();
    if (loadlevel.name[0]!=0)err=load_map(loadlevel.name);
     }
  closemode();
  }

*/

#define V_NOVA_HRA 0
#define V_OBNOVA_HRY 1
#define V_UVOD 2
#define V_AUTORI 3
#define V_KONEC 4

#define H_ETOPBAR (H_MENUS_FREE+100)
#define H_EDESK (H_MENUS_FREE+101)
static void game_big_circle(char enforced) {
	int err;
	int r;
	char s[13];

	SEND_LOG("\n(GAME) --------- Entering big loop ------------", 0, 0);
	purge_playlist();
	s[12] = 0;
	strncpy(s, loadlevel.name, 12);
	strupr(s);
	err = load_map(s);

	if (!enforced) {
		loadlevel.start_pos = gameMap.global().start_sector;
		loadlevel.dir = gameMap.global().direction;
	}

	while (loadlevel.name[0]) {
		if (err) {
			char buff[256];

			closemode();

			switch (err) {
			case -1:
				sprintf(buff, "Error while loading map (%s) ....file not found\n", s);
				break;

			case -2:
				sprintf(buff, "Missing -1 at the end of map string table");
				break;

			case -3:
				sprintf(buff, "Map file is corrupted!\n");
				break;

			default:
				sprintf(buff, "Error in string table at line %d", err);
				break;
			}

			Sys_ErrorBox(buff);
			exit(1);
		}

		viewsector = loadlevel.start_pos;
		viewdir = loadlevel.dir;

		if (viewsector == 0) {
			viewsector = set_leaving_place();

			if (viewsector == 0) {
				viewsector = gameMap.global().start_sector;
				viewdir = gameMap.global().direction;
			} else {
				int i;

				viewdir = 0;

				for (i = 0; i < 4; i++) {
					if (~gameMap.sides()[i + (viewsector << 2)].flags & (SD_PLAY_IMPS | SD_PRIM_VIS)) {
						viewdir = i;
						break;
					}
				}
			}
		}

		for (r = 0; r < gameMap.coordCount() * 4; r++) {
			call_macro(r, MC_STARTLEV);
		}

		recalc_volumes(viewsector, viewdir);
		loadlevel.name[0] = 0;
		SEND_LOG("\n(GAME) --------- Entering game ------------", 0, 0);
		enter_game();
		SEND_LOG("(GAME) --------- Leaving game ------------\n", 0, 0);
		leave_current_map();
		s[12] = 0;
		strncpy(s, loadlevel.name, 12);
		strupr(s);

		if (s[0] != 0) {
			err = load_map(s);
		}

		memset(GlobEventList, 0, sizeof(GlobEventList));
	}

	SEND_LOG("(GAME) --------- Leaving big loop ------------\n", 0, 0);
}

extern THUMAN postavy_2[];

static void new_game(int argc, char *argv[])
  {
  int sect,dir;
  char enforce=0;

  Sys_PurgeTemps(0);
  game_time=0;
  reinit_kouzla_full();
  load_shops();
  open_story_file();
  if (argc<2)
     strncpy(loadlevel.name,default_map,12);
  else
     strncpy(loadlevel.name,argv[1],12);
  if (argc>2)
     {
     sscanf(argv[2],"%d",&sect);
     enforce=1;
     dir=0;
     }
  loadlevel.start_pos=sect;
  loadlevel.dir=dir;
  init_game();
  if (argc>=2)
     {
     memcpy(postavy,postavy_2,sizeof(THUMAN)*6);
     memset(runes,0x7f,sizeof(runes));
     }
  reg_grafiku_postav();
  memset(GlobEventList,0,sizeof(GlobEventList));
  game_big_circle(enforce);
  }

static void undef_menu()
  {
  int i;
  for(i=0;i<255;i++) undef_handle(0x8000+i);
  }


static EVENT_PROC(load_error_report)
  {
  user_ptr;
  WHEN_MSG(E_IDLE)
     {
     message(1,0,0,"",texty[79],texty[80]);
     exit_wait=0;
     send_message(E_CLOSE_MAP);
     }
  }

static void wire_load_saved()
  {
	map_ret = -1;
  send_message(E_CLOSE_MAP,-1);
  }

static void load_saved_game(void) {
	signed char game;
	const Texture *tex;

err:
	loadlevel.name[0] = 0;
	def_handle(H_ETOPBAR, "topbar_e.pcx", pcx_15bit_decomp, SR_BGRAFIKA);
	wire_proc = wire_load_saved;
	tex = dynamic_cast<const Texture*>(ablock(H_ETOPBAR));
	renderer->blit(*tex, 0, 0, tex->palette());
	tex = dynamic_cast<const Texture*>(ablock(H_DESK));
	renderer->blit(*tex, 0, 378, tex->palette());
	wire_save_load(4);
	update_mysky();
	Task_WaitEvent(E_CLOSE_MAP);
	game = map_ret;
	unwire_proc();
	disable_click_map();
	Task_WaitEvent(E_TIMER);

	if (game != -1) {
		reinit_kouzla_full();
		open_story_file();
		memset(GlobEventList, 0, sizeof(GlobEventList));

		if (load_game(game)) {
			send_message(E_ADD, E_IDLE, load_error_report);
			Task_WaitEvent(E_CLOSE_MAP);
			send_message(E_DONE, E_IDLE, load_error_report);
			exit_wait = 0;
			goto err;
		}

		pick_set_cursor();
		undef_menu();
		init_game();
		build_all_players();
		game_big_circle(1);
		exit_wait = 1;
	}
}

//static void start(va_list args)
static void start()
  {
  int volba;
  char /*d,*/openning;

  zde:
   openning=0;
   update_mysky();
   schovej_mysku();
   if (!skip_intro)
    {
    show_jrc_logo("LOGO.PCX");
    play_anim(7);
    }
   skip_intro=0;
   create_playlist(texty[1]);
   //play_next_music(&d);
   Sound_ChangeMusic(NULL);
   zobraz_mysku();
   showview(0,0,0,0);
  do
  {
     volba=enter_menu(openning);openning=1;
     switch (volba)
       {
       case V_KONEC:exit_wait=1;break;
       case V_NOVA_HRA: if (!enter_generator())
                          {
                          undef_menu();
                          new_game(0,NULL);
                          exit_wait=1;
                          }
                        break;
       case V_UVOD:
       		bar(0,0,639,479);
		while (Input_Kbhit()) Input_ReadKey();
		goto zde;
		break;
       case V_OBNOVA_HRY:load_saved_game();break;
       case V_AUTORI:run_titles();break;
        }
     }
  while (!exit_wait);
  }

static void start_from_mapedit(va_list args)
//#pragma aux start_from_mapedit parm[]
  {
  int argc=va_arg(args,int);
  char **argv=va_arg(args,char **);
  new_game(argc-1,argv+1);
  exit_wait=1;
  }

void disable_intro()
  {
  add_field_num(cur_config, sinit[12].heslo, 1);
  update_config();
  }

//#include "crashdump.h"

int main(int argc, char *argv[]) {
	char *c, rm;
	int i;

	Sys_Init();
//  InitCrashDump();

	if (argc >= 3) {
		rm = !strcmp(argv[1], "12345678");
	} else {
		rm = 0;
	}

	if (!rm) {
		if (OtevriUvodniOkno() == 0) {
			return 0;
		}
	}

	// Uncoment to enable logging
//	delete syslog;
//	syslog = new StdLogger;
	SEND_LOG("START --------------------------", 0, 0);
//  c=getcwd(NULL,_MAX_PATH+1);
/*
  c=getcwd(NULL,PATH_MAX+1);
  pathtable[SR_SAVES]=getmem(strlen(c)+2);
  strcpy(pathtable[SR_SAVES],c);
  strcat(pathtable[SR_SAVES],"\\");
  free(c);
  SEND_LOG("(GAME) Save directory sets to '%s'",pathtable[SR_SAVES],0);
*/
//  set_verify(0);
//  mman_pathlist=pathtable;
	zoom_speed(1);
	turn_speed(1);
	Sys_SetEnv("BSVER", VERSION);
//  configure(CONFIG_NAME);
	configure(cur_config, Sys_FullPath(SR_WORK, CONFIG_NAME));

// FIXME: implement game launcher
/*
  if ((argc>=2 || SelectAdventure()) && !rm )
    {
	char *adventure;
    char **config=cur_config;

	// FIXME: ugly hack, is this really needed?
	Sys_SetPath(SR_ORGMUSIC, Sys_FullPath(SR_MUSIC, ""));

	if (argc<2) adventure=GetSelectedAdventure();
	else adventure=argv[1];
    cur_config=NULL;
    SEND_LOG("(GAME) Starting new adventure: %s",adventure,0);
    configure(adventure);
    release_list(cur_config);
    cur_config=config;
    }
*/
	if ((argc >= 2 || SelectAdventure()) && !rm) {
		char *adventure, *ptr, *tmp;
		StringList conf;

		adventure = argc >= 2 ? argv[1] : GetSelectedAdventure();
		tmp = new char[strlen(adventure) + 1];
		strcpy(tmp, adventure);
		ptr = strrchr(tmp, '/');

		if (ptr) {
			ptr[1] = '\0';
		} else {
			tmp[0] = '\0';
		}

		Sys_SetPath(SR_DEFAULT, tmp);
		delete[] tmp;
		Sys_SetPath(SR_ORGMUSIC, Sys_FullPath(SR_MUSIC, ""));
		SEND_LOG("(GAME) Starting new adventure: %s", adventure, 0);
		configure(conf, adventure);
	}

	for (i = 0; i <= SR_ORGMUSIC; i++) {
		SEND_LOG("(GAME) LOG: Using directory '%s' as '%s'", Sys_FullPath(i, ""), sinit[i+CESTY_POS].heslo);
	}

	Sys_PreparePaths();
	start_check();
	Sys_PurgeTemps(1);
//  textmode_effekt();
	clrscr();
	SEND_LOG("\n(GAME) Init----------------", 0, 0);
	init_skeldal();

	//Task_Add(32768,check_number_1phase,argv[0]);
	SEND_LOG("(INIT) Starting game thread.", 0, 0);
/*
  if (argc>=3 && rm)
     {
     Task_Add(65536,start_from_mapedit,argc,argv);
     }
  else
     Task_Add(65536,start);
*/
	start();
	SEND_LOG("(INIT) Main thread goes to sleep.", 0, 0);
/*  position(200,200);
  set_font(H_FBIG,RGB(200,200,200));
  outtext("Ahoj lidi");
  showview(0,0,0,0);*/  
//  escape();
	update_config();
	closemode();
	return 0;
}

#ifdef WIN32
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
  {
  return main(__argc, __argv);
  }
#endif
  
#include "game/version.h"


int GetExeVersion()
  {
	return VERSIONNUM;
  }
