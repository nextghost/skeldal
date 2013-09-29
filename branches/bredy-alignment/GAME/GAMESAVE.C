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
#include <skeldal_win.h>
#include <debug.h>
#include <dos.h>
#include <bios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <mem.h>
#include <pcx.h>
#include <types.h>
#include <bgraph.h>
#include <event.h>
#include <strlite.h>
#include <devices.h>
#include <bmouse.h>
#include <memman.h>
#include <fcntl.h>
#include <io.h>
#include <zvuk.h>
#include <SYS\STAT.H>
#include "globals.h"
#include <libs/memfile.h>
#include "gamesave.h"

#define STATE_CUR_VER 1

#define _GAME_ST "_GAME.TMP"
#define _GLOBAL_ST "_GLOBEV.TMP"
#define _SLOT_SAV "slot%02d.SAV"
#define SLOTS_MAX 10

#define GM_MAPENABLE 0x1

#define SAVE_SLOT_S 34
#define LOAD_SLOT_S (372+34)
#define SAVE_SLOT_E (34+203)
#define LOAD_SLOT_E (372+34+203)

#define SSAVE_VERSION 0

static PMEMFILE story =NULL;
static char load_another;
char reset_mobiles=0;


typedef struct s_save
  {
  int viewsector;
  char viewdir;
  short version;
  char not_used;
  int gold;
  short cur_group;
  char autosave;
  char enable_sort;
  char shownames;
  char showlives;
  char zoom_speed;
  char turn_speed;
  char autoattack;
  char music_vol;
  char sample_vol;
  char xbass;
  char bass;
  char treble;
  char stereing;
  char swapchans;
  char out_filter;
  long glob_flags;
  long game_time;
  char runes[5];
  char level_name[12];
  short picks;  //pocet_sebranych predmetu v mysi
  short items_added; //pocet_pridanych predmetu
  int sleep_long;
	int game_flags;
  }S_SAVE;

#define ZAKLAD_CRC 0xC005

static int get_list_count();



int load_org_map(char *filename,void **sides,void **sectors,void **coords,int *mapsize)
  {
  FILE *f;
  void *temp;
  int sect;
  long size,r;
  char nmapend=1;
  char *c;

	c=find_map_path(filename);
	f=fopen(c,"rb");free(c);
  if (f==NULL) return -1;
  do
     {
     r=load_section(f,&temp,&sect,&size);
     if (r==size)
        switch (sect)
         {
         case A_SIDEMAP:
                  *sides=temp;
                  break;
         case A_SECTMAP:
                  *sectors=temp;
                  if (mapsize!=NULL) *mapsize=size/sizeof(TSECTOR);
                  break;
         case A_MAPINFO:
                  if (coords!=NULL) *coords=temp;else free(temp);
                  break;
         case A_MAPGLOB:
                  //memcpy(&mglob,temp,min(sizeof(mglob),size));
                  free(temp);
                  break;
         case A_MAPEND :
                  nmapend=0;
                  free(temp);
                  break;
         default: free(temp);
         }
     else
        {
        if (temp!=NULL)free(temp);
        fclose(f);
        return -1;
        }
     }
  while (nmapend);
  fclose(f);
  return 0;
  }

void save_daction(PMEMFILE *f,int count,D_ACTION *ptr)
  {
  if (ptr!=NULL)
     {
     save_daction(f,count+1,ptr->next);
     writeMemFile(f,ptr,sizeof(D_ACTION));
     }
  else {
	  writeMemFile(f,&count,2);
  }
}

void load_daction(PMEMFILE f, int *seekPos)
  {
  int i,j;
  i=0;
  while (d_action!=NULL) //vymaz pripadne delaited actions
     {
     D_ACTION *p;
     p=d_action; d_action=p->next;free(p);
     }
  readMemFile(f,seekPos,&i,2);
  d_action=NULL;
  for(j=0;j<i;j++)
     {
     D_ACTION *p;

     p=(D_ACTION *)getmem(sizeof(D_ACTION));
     readMemFile(f,seekPos,p,sizeof(D_ACTION));
     p->next=d_action;
     d_action=p;
     }
  }

void save_items(PMEMFILE *f)
  {
  int i,j;
  short *c;

  for(i=0;i<mapsize*4;i++)
     if (map_items[i]!=NULL)
        {
        for(j=1,c=map_items[i];*c;c++,j++);
		writeMemFile(f,&i,2);
		writeMemFile(f,&j,2);
		writeMemFile(f,map_items[i],j*2);
        }
  i=-1;
  writeMemFile(f,&i,2);
  }

void restore_items(PMEMFILE f, int *seekPos)
  {
  short i,j;

  for(i=0;i<mapsize*4;i++) if (map_items[i]!=NULL) free(map_items[i]);
  memset(map_items,0,mapsize*16);
  while(readMemFile(f,seekPos,&i,2) && i!=-1)
     {
		readMemFile(f,seekPos,&j,2);
	    map_items[i]=(short *)getmem(j*2);
		readMemFile(f,seekPos,map_items[i],2*j);
     }
  }

extern TSTR_LIST texty_v_mape;

void save_map_description(PMEMFILE *f)
  {
  int count,max;
  int i;

  if (texty_v_mape==NULL) max=0;else max=str_count(texty_v_mape);
  for(i=0,count=0;i<max;i++) if (texty_v_mape[i]!=NULL) count++;
  writeMemFile(f,&count,sizeof(count));
  for(i=0;i<max;i++) if (texty_v_mape[i]!=NULL)
     {
     int len;
     len=strlen(texty_v_mape[i]+12)+12+1;
     writeMemFile(f,&len,2);
     writeMemFile(f,texty_v_mape[i],len);
     }
  }

void load_map_description(PMEMFILE f, unsigned int *seekPos)
  {
  int count;
  int i;
  word len;

  if (texty_v_mape!=NULL)release_list(texty_v_mape);
  readMemFile(f,seekPos,&count,sizeof(count));
  if (!count)
     {
     texty_v_mape=NULL;
     return;
     }
  texty_v_mape=create_list(count);
  for(i=0;i<count;i++)
     {
     readMemFile(f,seekPos,&len,2);

        {
        char *s;
        s=(char *)alloca(len);
        memset(s,1,len-1);
        s[len-1]=0;
        str_replace(&texty_v_mape,i,s);
        }
     readMemFile(f,seekPos,texty_v_mape[i],len);
     }
  }

void save_vyklenky(PMEMFILE *fsta)
  {
  writeMemFile(fsta,&vyk_max,sizeof(vyk_max));
  if (vyk_max)
     writeMemFile(fsta,map_vyk,vyk_max*sizeof(TVYKLENEK));
  }

int load_vyklenky(PMEMFILE fsta, unsigned int *seekPos)
  {
  int i=0;
  readMemFile(fsta,seekPos,&i,sizeof(vyk_max));
  if (vyk_max)
     {
     if (i>vyk_max) return -2;
     readMemFile(fsta,seekPos,map_vyk,vyk_max*sizeof(TVYKLENEK));
     }
  return 0;
  }


void save_all_fly(PMEMFILE *fsta)
  {
  LETICI_VEC *f;

  f=letici_veci;
  writeMemFile(fsta,&f,sizeof(f));
  while (f!=NULL)
     {
     short *c;
     writeMemFile(fsta,f,sizeof(*f));
     c=f->items;
     if (c!=NULL) do writeMemFile(fsta,c,2); while (*c++);
     f=f->next;
     }
  }

int load_all_fly(PMEMFILE fsta, int *seekPos)
  {
  LETICI_VEC *f=NULL,*n,*p;
  readMemFile(fsta,seekPos,&f,sizeof(f));
  p=letici_veci;
  while (f!=NULL)
     {
     short items[100],*c;
     n=New(LETICI_VEC);
     c=items;memset(items,0,sizeof(items));
     if (readMemFile(fsta,seekPos,n,sizeof(*n))!=sizeof(*n))
        {
        free(n);
        if (p!=NULL) p->next=NULL;
        return -2;
        }
     if (n->items!=NULL)
        {
        do
          readMemFile(fsta,seekPos,c,2);
        while (*c++);
        n->items=NewArr(short,c-items);
        memcpy(n->items,items,(c-items)*sizeof(short));
        }
     if (p==NULL) p=letici_veci=n;else p->next=n;
     p=n;
     f=n->next;
     n->next=NULL;
     }
  return 0;
  }


int save_map_state() //uklada stav mapy pro savegame (neuklada aktualni pozici);
  {
  char sta[200];
  char *bf;
  PMEMFILE fsta;
  int i;
  long siz;
 TSTENA *org_sides;
 TSECTOR *org_sectors;
  short res=-1;
  int ver=0;

  restore_sound_names();
  strcpy(sta,level_fname);
  fsta=createMemFile(sta,16);
  SEND_LOG("(SAVELOAD) Saving map state for current map",0,0);
  if (load_org_map(level_fname,&org_sides,&org_sectors,NULL,NULL)) goto err;
  siz=(mapsize+7)/8;
  bf=(char *)getmem(siz);
  ver=0;
  writeMemFile(&fsta,&ver,sizeof(ver));  //<-------------------------
  ver=STATE_CUR_VER;
  writeMemFile(&fsta,&ver,sizeof(ver));  //<-------------------------
  writeMemFile(&fsta,&mapsize,sizeof(mapsize));  //<-------------------------
  memset(bf,0,siz);
  writeMemFile(&fsta,&siz,sizeof(siz));          //<-------------------------
  for(i=0;i<mapsize;i++)  //save automap
    if (map_coord[i].flags & MC_AUTOMAP) bf[i>>3]|=1<<(i & 7);
  writeMemFile(&fsta,bf,siz);
  for(i=0;i<mapsize;i++)  //save disclosed
    if (map_coord[i].flags & MC_DISCLOSED) bf[i>>3]|=1<<(i & 7);
  writeMemFile(&fsta,bf,siz);
  save_map_description(&fsta);
  for(i=0;i<mapsize*4;i++)  //save changed sides
     if (memcmp(map_sides+i,org_sides+i,sizeof(TSTENA)))
        {
        writeMemFile(&fsta,&i,2);
        writeMemFile(&fsta,map_sides+i,sizeof(TSTENA));
        }
  i=-1;
  writeMemFile(&fsta,&i,2);
  for(i=0;i<mapsize;i++)   //save changed sectors
     if (memcmp(map_sectors+i,org_sectors+i,sizeof(TSECTOR)))
        {
        writeMemFile(&fsta,&i,2);
        writeMemFile(&fsta,map_sectors+i,sizeof(TSECTOR));
        }
  i=-1;
  writeMemFile(&fsta,&i,2);
  for(i=0;i<MAX_MOBS;i++) if (mobs[i].vlajky & MOB_LIVE)
     {
     writeMemFile(&fsta,&i,2);
     writeMemFile(&fsta,mobs+i,sizeof(TMOB)); //save_mobmap
     }
  i=-1;
  writeMemFile(&fsta,&i,2);
  i=mapsize*4;
  writeMemFile(&fsta,&i,4); //save flag maps //<-------------------------
  writeMemFile(&fsta,flag_map,i);
  save_daction(&fsta,0,d_action); //save dactions//<-------------------------
  writeMemFile(&fsta,&macro_block_size,4);
  if (macro_block_size) writeMemFile(&fsta,macro_block,macro_block_size);//save_macros
  if (save_codelocks(&fsta)) goto err;
  save_items(&fsta);
  save_vyklenky(&fsta);
  save_all_fly(&fsta);
  save_enemy_paths(&fsta);
  res=0;
  SEND_LOG("(SAVELOAD) State of current map saved (err:%d)",res,0);
  commitMemFile(fsta);
  free(org_sectors);
  free(org_sides);
  free(bf);
  return res;
err:
  res = 1;
  SEND_LOG("(SAVELOAD) Can't load original map to make difference (err:%d)",res,0);
  return 1;
  }

int load_map_state() //obnovuje stav mapy; nutno volat po zavolani load_map;
  {
  char *bf;
  PMEMFILE fsta;
  unsigned int fstaposaccu;
  unsigned int *fstapos = &fstaposaccu;
  int i = 0;
  long siz;
  short res=-2;
  int ver=0;


  fsta=openMemFile(level_fname);
  if (fsta==NULL) return -1;
  *fstapos = 0;
  i=0;
  readMemFile(fsta,fstapos,&i,sizeof(mapsize));
  if (i==0)
  {
    readMemFile(fsta,fstapos,&ver,sizeof(ver));
    if (ver>STATE_CUR_VER) goto err;
    readMemFile(fsta,fstapos,&i,sizeof(mapsize));
    if (mapsize!=i) goto err;
    SEND_LOG("(SAVELOAD) Loading map state for current map",0,0);
    readMemFile(fsta,fstapos,&siz,sizeof(siz));
    bf=(char *)getmem(siz);
    readMemFile(fsta,fstapos,bf,siz);
    for (i=0;i<mapsize;i++)
      if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_AUTOMAP;
    if (!readMemFile(fsta,fstapos,bf,siz)) goto err;
    for (i=0;i<mapsize;i++)
      if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_DISCLOSED;
   }
  else
  {
    if (mapsize!=i) goto err;
    SEND_LOG("(SAVELOAD) Loading map state for current map",0,0);
    readMemFile(fsta,fstapos,&siz,sizeof(siz));
    bf=(char *)getmem(siz);
    readMemFile(fsta,fstapos,bf,siz);
    for (i=0;i<mapsize;i++)
       map_coord[i].flags|=(bf[i>>3]>>(i & 7)) & 1;
  }
  load_map_description(fsta,fstapos);
  readMemFile(fsta,fstapos,&i,2);
  while (i != -1 && i<=mapsize*4) {
      readMemFile(fsta,fstapos,map_sides+i,sizeof(TSTENA));
	  readMemFile(fsta,fstapos,&i,2);
  }
  readMemFile(fsta,fstapos,&i,2);
  while (i != -1 && i<=mapsize) {
      readMemFile(fsta,fstapos,map_sectors+i,sizeof(TSECTOR));
	  readMemFile(fsta,fstapos,&i,2);
  
  }
  if (reset_mobiles)  //reloads mobiles if flag present
    {
    char mm[MAX_MOBS];
    for(i=0;i<MAX_MOBS;mobs[i].vlajky &=~MOB_LIVE,i++)
      if (mobs[i].vlajky & MOB_LIVE) mm[i]=1;else mm[i]=0;
	readMemFile(fsta,fstapos,&i,2);
    while (i!=-1 && i<=MAX_MOBS)
      {
      if (mm[i]) mobs[i].vlajky |=MOB_LIVE;
	  (*fstapos)+=sizeof(TMOB);
	  readMemFile(fsta,fstapos,&i,2);
      }
    reset_mobiles=0;
    }
  else
    {
		for(i=0;i<MAX_MOBS;(mobs[i].vlajky &=~MOB_LIVE),i++);
		readMemFile(fsta,fstapos,&i,2);
		while (i!=-1 && i<=MAX_MOBS) {
			readMemFile(fsta,fstapos,mobs+i,sizeof(TMOB));
			readMemFile(fsta,fstapos,&i,2);
		}
	}
  for(i=0;i<MAX_MOBS;i++) mobs[i].vlajky &=~MOB_IN_BATTLE;
  refresh_mob_map();  
  readMemFile(fsta,fstapos,&i,4);
  readMemFile(fsta,fstapos,flag_map,i);
  load_daction(fsta,fstapos);
  readMemFile(fsta,fstapos,&i,4);
  if (macro_block_size && i==macro_block_size) {
	  readMemFile(fsta,fstapos,macro_block,macro_block_size);
  } else {
     (*fstapos) += i;
     SEND_LOG("(ERROR) Multiaction: Sizes mismatch %d != %d",i,macro_block_size);
  }
  if (load_codelocks(fsta,fstapos)) goto err;
  restore_items(fsta,fstapos);
  res=0;
  res|=load_vyklenky(fsta,fstapos);
  res|=load_all_fly(fsta,fstapos);
  res|=load_enemy_paths(fsta,fstapos);
  err:
  SEND_LOG("(SAVELOAD) State of current map loaded (err:%d)",res,0);
  free(bf);
  return res;
  }

void restore_current_map() //pouze obnovuje ulozeny stav aktualni mapy
  {
  int i;

  SEND_LOG("(SAVELOAD) Restore map...",0,0);
  kill_all_sounds();
  for(i=0;i<mapsize;i++) map_coord[i].flags&=~0x7f; //vynuluj flags_info
  free(map_sides);        //uvolni informace o stenach
  free(map_sectors);      //uvolni informace o sektorech
  free(map_coord);       //uvolni minfo informace
  load_org_map(level_fname,&map_sides,&map_sectors,&map_coord,NULL); //nahrej originalni mapu
  load_map_state(); //nahrej ulozenou mapu
  for(i=1;i<mapsize*4;i++) call_macro(i,MC_STARTLEV);
  }


//#pragma aux rotate parm [al] value [al]="rol al,1";

/* errors
     -1 end of file
      1 disk error
      2 internal error
      3 checksum error
 */
int pack_status_file(FILE *f,PMEMFILE stt)
  {
  unsigned int sttpos = 0;
  char rcheck=0;
  long fsz;
  int nlen = strlen(stt->name);


  SEND_LOG("(SAVELOAD) Packing status file '%s'",stt->name,0);
  if (fwrite(&nlen,1,2,f) != 2) return 1;
  if (fwrite(stt->name,1,nlen,f) != nlen) return 1;
  fsz=stt->length;
  if (fwrite(&fsz,1,4,f) != 4) return 1;
  if (fwrite(stt->data,1,fsz,f) != fsz) return 1;
  return  0;
  }

int unpack_status_file(FILE *f,PMEMFILE *spc)
  {
  long fsz;
  int nlen = 0;
  char *fullnam;
  PMEMFILE mf;


  if (fread(&nlen,1,2,f) != 2) return 1;
  if (nlen == 0) return -1;
  fullnam = (char *)alloca(nlen+1);
  if (fread(fullnam,1,nlen,f) != nlen) return 1;
  fullnam[nlen] = 0;
  if (fread(&fsz,1,4,f) != 4) return 1;
  SEND_LOG("(SAVELOAD) Unpacking status file '%s' (size: %ld)",fullnam,fsz);
  if (spc) {
	  reuseMemFile(fullnam,spc,fsz);
	  mf = *spc;
  } else {
	  mf = createMemFile(fullnam,fsz);
  }
  if (fread(mf->data,1,fsz,f) != fsz) {
	  free(mf);
	  return 1;
  }
  mf->length =(unsigned int) fsz;
  if (spc == 0) 
	  commitMemFile(mf);
  else 
	  *spc = mf;

  return 0;
  }

int pack_all_status(FILE *f)
  {
  PMEMFILE stt = getFirstMemFile();
  while (stt != 0) {
	  pack_status_file(f,stt);
	  stt = getNextMemFile(stt);
  }
  fwrite(&stt,1,2,f);
  return 0;
  }

int unpack_all_status(FILE *f)
  {
  int i;

  i=0;
  while (!i) i=unpack_status_file(f,0);
  if (i==-1) i=0;
  return i;
  }



int save_basic_info()
  {
  PMEMFILE f;
  S_SAVE s;
  short *p;
  int i;
  char res=0;
  THUMAN *h;
  
  SEND_LOG("(SAVELOAD) Saving basic info for game ",_GAME_ST,0);
  f=createMemFile(_GAME_ST,0);
  s.viewsector=viewsector;
  s.viewdir=viewdir;
  s.version=SSAVE_VERSION;
  s.not_used=0;
  s.gold=money;
  s.cur_group=cur_group;
  s.shownames=show_names;
  s.showlives=show_lives;
  s.autoattack=autoattack;
  s.turn_speed=turn_speed(-1);
  s.zoom_speed=zoom_speed(-1);
  s.game_time=game_time;
  s.enable_sort=enable_sort;
  s.sleep_long=sleep_ticks;
  s.sample_vol=get_snd_effect(SND_GFX);
  s.music_vol=get_snd_effect(SND_MUSIC);
  s.xbass=get_snd_effect(SND_XBASS);
  s.bass=get_snd_effect(SND_BASS);
  s.treble=get_snd_effect(SND_TREBL);
  s.stereing=get_snd_effect(SND_LSWAP);
  s.swapchans=get_snd_effect(SND_SWAP);
  s.out_filter=get_snd_effect(SND_OUTFILTER);
  s.autosave=autosave_enabled;
	s.game_flags=(enable_glmap!=0);
  strncpy(s.level_name,level_fname,12);
  for(i=0;i<5;i++) s.runes[i]=runes[i];
  if (picked_item!=NULL)
     for(i=1,p=picked_item;*p;i++,p++);else i=0;
  s.picks=i;
  s.items_added=item_count-it_count_orgn;
  writeMemFile(&f,&s,sizeof(s));
  if (i)
     writeMemFile(&f,picked_item,2*i);
  if (s.items_added)
     writeMemFile(&f,glob_items+it_count_orgn,sizeof(TITEM)*s.items_added);
  save_spells(&f);
  writeMemFile(&f,postavy,sizeof(postavy));
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++) if (h->demon_save!=NULL)
     writeMemFile(&f,h->demon_save,sizeof(THUMAN));       //ulozeni polozek s demony
  save_dialog_info(&f);
  commitMemFile(f);
  SEND_LOG("(SAVELOAD) Done... Result: %d",res,0);
  return res;
  }

int load_basic_info()
  {
  PMEMFILE f;
  unsigned int fpos = 9;
  S_SAVE s;
  int i;
  char res=0;
  TITEM *itg;
  THUMAN *h;

  SEND_LOG("(SAVELOAD) Loading basic info for game (file:%s)",_GAME_ST,0);
  f = openMemFile(_GAME_ST);
  fpos =0;
  if (f==NULL) return 1;
  readMemFile(f,&fpos,&s,sizeof(s));
	if (s.game_flags & GM_MAPENABLE) enable_glmap=1;else enable_glmap=0;
  i=s.picks;
  if (picked_item!=NULL) free(picked_item);
  if (i)
     {
     picked_item=NewArr(short,i);
     readMemFile(f,&fpos,picked_item,2*i);
     }
  else picked_item=NULL;
  itg=NewArr(TITEM,it_count_orgn+s.items_added);
  memcpy(itg,glob_items,it_count_orgn*sizeof(TITEM));
  free(glob_items);glob_items=itg;
  if (s.items_added)
     readMemFile(f,&fpos,glob_items+it_count_orgn,sizeof(TITEM)*s.items_added);
  item_count=it_count_orgn+s.items_added;
  res|=load_spells(f,&fpos);
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++) 
	  if (h->demon_save!=NULL) free(h->demon_save);
  readMemFile(f,&fpos,postavy,sizeof(postavy));
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++)
        {
        h->programovano=0;
        h->provadena_akce=h->zvolene_akce=NULL;
        h->dostal=0;
        if (h->demon_save!=NULL)
          {
          h->demon_save=New(THUMAN);
          readMemFile(f,&fpos,h->demon_save,sizeof(THUMAN));
          }
        }
  load_dialog_info(f,&fpos);  
  viewsector=s.viewsector;
  viewdir=s.viewdir;
  cur_group=s.cur_group;
  show_names=s.shownames;
  show_lives=s.showlives;
  autoattack=s.autoattack;
  turn_speed(s.turn_speed);
  zoom_speed(s.zoom_speed);
  game_time=s.game_time;
  sleep_ticks=s.sleep_long;
  enable_sort=s.enable_sort;
  autosave_enabled=s.autosave;
  money=s.gold;
  for(i=0;i<5;i++) runes[i]=s.runes[i];
  set_snd_effect(SND_GFX,s.sample_vol);
  set_snd_effect(SND_MUSIC,s.music_vol);
  set_snd_effect(SND_XBASS,s.xbass);
  set_snd_effect(SND_BASS,s.bass);
  set_snd_effect(SND_TREBL,s.treble);
  set_snd_effect(SND_LSWAP,s.stereing);
  set_snd_effect(SND_SWAP,s.swapchans);
  set_snd_effect(SND_OUTFILTER,s.out_filter);
  if (level_fname==NULL || strncmp(s.level_name,level_fname,12))
     {
     strncpy(loadlevel.name,s.level_name,12);
     loadlevel.start_pos=viewsector;
     loadlevel.dir=viewdir;
     send_message(E_CLOSE_MAP);
     load_another=1;
     }
  else load_another=0;
  for(i=0;i<POCET_POSTAV;i++) postavy[i].dostal=0;
  SEND_LOG("(SAVELOAD) Done... Result: %d",res,0);
  return res;
  }

static void MakeSaveGameDir(const char *name)
{
  char *p=(char *)alloca(strlen(name)+1);
  strcpy(p,name);
  p[strlen(p)-1]=0;
  CreateDirectory(p,NULL);
}

static int save_global_events()
{
  PMEMFILE f;
  f = createMemFile(_GLOBAL_ST,sizeof(GlobEventList));
  if (f==NULL) return 1;
  writeMemFile(&f,GlobEventList,sizeof(GlobEventList));
  commitMemFile(f);
  return 0;
}

static int load_global_events()
{
  PMEMFILE f;
  unsigned int fpos = 0;
  memset(GlobEventList,0,sizeof(GlobEventList));

  f = openMemFile(_GLOBAL_ST);
  if (f==NULL) return 1;
  readMemFile(f,&fpos,GlobEventList,sizeof(GlobEventList));
  return 0;
}

int save_game(int slotnum,char *gamename)
  {
  char *sn,*ssn,*gn;
  FILE *svf;
  int r;

  SEND_LOG("(SAVELOAD) Saving game slot %d",slotnum,0);
  save_map_state();
  concat(sn,pathtable[SR_SAVES],_SLOT_SAV);
  MakeSaveGameDir(pathtable[SR_SAVES]);
  ssn=alloca(strlen(sn)+3);
  sprintf(ssn,sn,slotnum);
  gn=alloca(SAVE_NAME_SIZE);
  strncpy(gn,gamename,SAVE_NAME_SIZE);
  if ((r=save_shops())!=0) return r;
  if ((r=save_basic_info())!=0) return r;
  save_leaving_places();
  save_book();
  save_global_events();
  svf=fopen(ssn,"wb");
  if (svf==NULL) 
  {
	char buff[256];
	sprintf(buff,"Nelze ulozit pozici na cestu: %s", ssn);
    MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);  
  }
  else
  {
	fwrite(gn,1,SAVE_NAME_SIZE,svf);
	close_story_file();
	r=pack_all_status(svf);
	open_story_file();
	fclose(svf);
  }
  SEND_LOG("(SAVELOAD) Game saved.... Result %d",r,0);
  disable_intro();
  return r;
  }

extern char running_battle;

int load_game(int slotnum)
  {
  char *sn,*ssn;
  FILE *svf;
  int r,t;

  SEND_LOG("(SAVELOAD) Loading game slot %d",slotnum,0);
  if (battle) konec_kola();
  battle=0;
  close_story_file();
  purge_temps(0);
  concat(sn,pathtable[SR_SAVES],_SLOT_SAV);
  ssn=alloca(strlen(sn)+3);
  sprintf(ssn,sn,slotnum);
  svf=fopen(ssn,"rb");
  if (svf==NULL) return 1;
  fseek(svf,SAVE_NAME_SIZE,SEEK_CUR);
  r=unpack_all_status(svf);
  load_leaving_places();
  fclose(svf);
  open_story_file();
  if (r>0)
     {
     SEND_LOG("(ERROR) Error detected during unpacking game... Loading stopped (result:%d)",r,0);
     return r;
     }
  load_book();
  load_global_events();
  if ((t=load_saved_shops())!=0) return t;
  if ((t=load_basic_info())!=0) return t;
  running_battle=0;
  norefresh=0;
  if (!load_another) restore_current_map();
        else
           {
           save_map=0;
           norefresh=1;
           }
  for(t=0;t<POCET_POSTAV;t++) postavy[t].zvolene_akce=NULL;
  SEND_LOG("(SAVELOAD) Game loaded.... Result %d",r,0);
  if (GetKeyState(VK_CONTROL) & 0x80) correct_level();
  return r;
  }

static PMEMFILE load_specific_file(int slot_num,char *filename) //call it in task!
  {
  FILE *slot;
  char *c,*d;
  long siz;
  char fname[12];
  char succes=0;
  PMEMFILE out;
  

  concat(c,pathtable[SR_SAVES],_SLOT_SAV);
  d=alloca(strlen(c)+2);
  sprintf(d,c,slot_num);
  slot=fopen(d,"rb");
  if (slot==NULL)
     return 0;

  fseek(slot,SAVE_NAME_SIZE,SEEK_CUR);
  out = createMemFile("",0);
   
  while (unpack_status_file(slot,&out) == 0) {
	  if (strcmp(out->name,filename) == 0) {
		  fclose(slot);
		  return out;
	  }
  }
  fclose(slot);
  return 0;
  }

//------------------------ SAVE LOAD DIALOG ----------------------------
static char force_save;
static TSTR_LIST slot_list=NULL;
static int last_select=-1;
static char used_pos[SLOTS_MAX];
static TSTR_LIST story_text=NULL;
static void *back_texture=NULL;
static int cur_story_pos=0;
static char load_mode;

#define SLOT_SPACE 33
#define SELECT_COLOR RGB555(31,31,31)
#define NORMAL_COLOR RGB555(10,31,10)
#define STORY_X 57
#define STORY_Y 50
#define STORY_XS (298-57)
#define STORY_YS (302-50)

void read_slot_list()
  {
  int i;
  char *mask,*name;
  char slotname[SAVE_NAME_SIZE];
  if (slot_list==NULL) slot_list=create_list(SLOTS_MAX);
  concat(mask,pathtable[SR_SAVES],_SLOT_SAV);
  name=alloca(strlen(mask)+1);
  for(i=0;i<SLOTS_MAX;i++)
     {
     FILE *f;
     sprintf(name,mask,i);
     f=fopen(name,"rb");
     if (f!=NULL)
        {
        fread(slotname,1,SAVE_NAME_SIZE,f);
        fclose(f);
        used_pos[i]=1;
        }
     else
        {
        strcpy(slotname,texty[75]);
        used_pos[i]=0;
        }
     str_replace(&slot_list,i,slotname);
     }
  }

static void place_name(int c,int i,char show)
  {
  int z,x;
  if (c) x=SAVE_SLOT_S;else x=LOAD_SLOT_S;
  if (show) schovej_mysku();
  position(x,z=i*SLOT_SPACE+21+SCREEN_OFFLINE);outtext(slot_list[i]);
  if (show)
     {
     ukaz_mysku();
     showview(x,z,204,18);
     }
  }

static void redraw_save()
  {
  int i;
  schovej_mysku();
  put_picture(0,SCREEN_OFFLINE,ablock(H_SAVELOAD));
  put_picture(274,SCREEN_OFFLINE,ablock(H_SVITEK));
  set_font(H_FBOLD,NORMAL_COLOR);
  for(i=0;i<SLOTS_MAX;i++) place_name(1,i,0);
  ukaz_mysku();
  }

static void redraw_load()
  {
  int i;
  schovej_mysku();
  put_picture(0,SCREEN_OFFLINE,ablock(H_SVITEK));
  put_picture(372,SCREEN_OFFLINE,ablock(H_SAVELOAD));
  set_font(H_FBOLD,NORMAL_COLOR);
  for(i=0;i<SLOTS_MAX;i++) place_name(0,i,0);
  ukaz_mysku();
  }

static void redraw_story_bar(int pos)
  {
  int i,y,ys,x,count;
  schovej_mysku();
  if (force_save) x=STORY_X+274;else x=STORY_X;
  if (back_texture==NULL)
     {
     back_texture=getmem(STORY_XS*STORY_YS*2+6);
     get_picture(x,STORY_Y+SCREEN_OFFLINE,STORY_XS,STORY_YS,back_texture);
     }
  else
     put_picture(x,STORY_Y+SCREEN_OFFLINE,back_texture);
  if (story_text!=NULL)
     {
     y=SCREEN_OFFLINE+STORY_Y;
     ys=STORY_YS;
     count=str_count(story_text);
       set_font(H_FONT6,NOSHADOW(0));
     for(i=pos;i<count;i++) if (story_text[i]!=NULL)
       {
       int h;

       h=text_height(story_text[i]);
       if (ys<2*h) break;
       position(x,y);outtext(story_text[i]);
       ys-=h;
       y+=h;
       }
     }
  ukaz_mysku();
  showview(x,STORY_Y+SCREEN_OFFLINE,STORY_XS,STORY_YS);
  }

//#pragma aux read_story_task parm []
static read_story_task(int slot)
  {

  TSTR_LIST ls;
  char *c,*d;
  long size;
  PMEMFILE text_data;

  text_data = load_specific_file(slot,STORY_BOOK);
  if (text_data!=NULL)
     {
	 size = text_data->length;
     ls=create_list(2);
     c=text_data->data;
     set_font(H_FONT6,0);
     while (size>0)
       {
       int xs,ys;
       d=c;
       while (size && *d!='\r' && *d!='\n') {d++;size--;};
       if (!size) break;
       *d=0;
       {
       char *e,*or;
       or=e=getmem(strlen(c)+2);
       zalamovani(c,e,STORY_XS,&xs,&ys);
       while (*e)
          {
          str_add(&ls,e);
          if (text_width(e)>STORY_XS) abort();
          e=strchr(e,0)+1;
          }
        c=d+1;size--;
       if (*c=='\n' || *c=='\r') {c++;size--;};
       free(or);
       }
       }
     closeMemFile(text_data);
     }
  else ls=NULL;
  if (story_text!=NULL) release_list(story_text);
  story_text=ls;
  cur_story_pos=get_list_count();if (cur_story_pos<0) cur_story_pos=0;
  redraw_story_bar(cur_story_pos);
  }

static void read_story(int slot)
  {
     read_story_task(slot);
  }


static int get_list_count()
  {
  int count,i,max=0;

  if (story_text==NULL) return 0;
  count=str_count(story_text);
  for(i=0;i<count;i++) if (story_text[i]!=NULL) max=i;
  return max-20;
  }

static int bright_slot(int yr)
  {
  int id;

  id=yr/SLOT_SPACE;
  if ((yr % SLOT_SPACE)<18 && yr>0)
     {
     if (id!=last_select)
     {
     set_font(H_FBOLD,NORMAL_COLOR);
     if (last_select!=-1) place_name(force_save,last_select,1);
     set_font(H_FBOLD,SELECT_COLOR);
     place_name(force_save,id,1);
     last_select=id;
     read_story(id);
     }
     }
  else
     id=-1;
  return id;
  }

char updown_scroll(int id,int xa,int ya,int xr,int yr);

static char updown_noinst=0;

static EVENT_PROC(updown_scroll_hold)
  {
  user_ptr;
  WHEN_MSG(E_MOUSE)
    {
    MS_EVENT *ms;

    ms=get_mouse(msg);
    if (ms->event_type==0x4 || !ms->tl1 || ms->tl2 || ms->tl3)
      {
      send_message(E_DONE,E_MOUSE,updown_scroll_hold);
      send_message(E_DONE,E_TIMER,updown_scroll_hold);
      updown_noinst=0;
      }
    }
  WHEN_MSG(E_TIMER)
    {
    MS_EVENT *ms;

    updown_noinst=1;
    ms=&ms_last_event;
    ms->event_type=0x2;
    send_message(E_MOUSE,ms);
    if (updown_noinst)
      {
      send_message(E_DONE,E_MOUSE,updown_scroll_hold);
      send_message(E_DONE,E_TIMER,updown_scroll_hold);
      updown_noinst=0;
      }
    else
      updown_noinst=1;
    }
  }

static char updown_scroll(int id,int xa,int ya,int xr,int yr)
  {
  int count;
  xr,yr,xa,ya;
  if (story_text==NULL) return 0;
  cur_story_pos+=id;
  count=get_list_count();
  if (cur_story_pos>count) cur_story_pos=count;
  if (cur_story_pos<0) cur_story_pos=0;
  redraw_story_bar(cur_story_pos);
  if (updown_noinst)
    {
    updown_noinst=0;
    return 1;
    }
  send_message(E_ADD,E_MOUSE,updown_scroll_hold);
  send_message(E_ADD,E_TIMER,updown_scroll_hold);
  return 1;
  }

static char close_saveload(int id,int xa,int ya,int xr,int yr)
  {
  xa;ya;xr;yr;id;
  if (ms_last_event.event_type & 0x8)
     {
     unwire_proc();
     wire_proc();
     }
  return 1;
  }

char clk_load_konec(int id,int xa,int ya,int xr,int yr)
  {
  id;xa;ya;xr;yr;
  send_message(E_CLOSE_MAP,-1);
  return 1;
  }

static char clk_load_proc(int id,int xa,int ya,int xr,int yr);

#define CLK_LOAD_ERROR 5
T_CLK_MAP clk_load_error[]=
  {
  {-1,59,14+SCREEN_OFFLINE,306,46+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {1,59,310+SCREEN_OFFLINE,306,332+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {-1,LOAD_SLOT_S,SCREEN_OFFLINE,LOAD_SLOT_E,350,clk_load_proc,3,H_MS_DEFAULT},
  {-1,30,0,85,14,clk_load_konec,2,H_MS_DEFAULT},
  {-1,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };


static char clk_load_proc_menu(int id,int xa,int ya,int xr,int yr)
  {
  id=bright_slot(yr-18);
  xa;ya;xr;yr;
  if (ms_last_event.event_type & 0x2 && id>=0 && used_pos[id])
     send_message(E_CLOSE_MAP,id);
  return 1;
  }

#define CLK_LOAD_MENU 5
T_CLK_MAP clk_load_menu[]=
  {
  {-1,59,14+SCREEN_OFFLINE,306,46+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {1,59,310+SCREEN_OFFLINE,306,332+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {-1,LOAD_SLOT_S,SCREEN_OFFLINE,LOAD_SLOT_E,350,clk_load_proc_menu,3,H_MS_DEFAULT},
  {-1,0,0,639,479,clk_load_konec,8,H_MS_DEFAULT},
  {-1,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };

static char clk_load_proc(int id,int xa,int ya,int xr,int yr)
  {
  id=bright_slot(yr-18);
  xa;ya;xr;yr;
  if (ms_last_event.event_type & 0x2 && id>=0 && used_pos[id])
     {
     if (load_game(id))
        {
        message(1,0,0,"",texty[79],texty[80]);
        redraw_load();
        showview(0,0,0,0);
        change_click_map(clk_load_error,CLK_LOAD_ERROR);
        }
     else
        {
        unwire_proc();
        wire_proc();
        if (battle) konec_kola();
        unwire_proc();
        if (!load_another)
		  {
		  wire_main_functs();
		  cur_mode=MD_GAME;
		  bott_draw(1);
		  pick_set_cursor();
		  for(id=0;id<mapsize;id++) map_coord[id].flags&=~MC_DPLAYER;
  		  build_player_map();
		  }
		reg_grafiku_postav();
		build_all_players();
  	    cancel_render=1;
        }
     }
  return 1;
  }

static char global_gamename[SAVE_NAME_SIZE];
static int slot_pos;

void save_step_next(EVENT_MSG *msg,void **unused)
  {
  char c;

  unused;
  if (msg->msg==E_KEYBOARD)
     {
     c=*(char *)msg->data;
     if (c==13)
        {
        send_message(E_KEYBOARD,c);
        save_game(slot_pos,global_gamename);
        wire_proc();
        read_slot_list();
        msg->msg=-2;
        }
     else if(c==27)
        {
        send_message(E_KEYBOARD,c);
        msg->msg=-2;
        wire_save_load(1);
        }
     }
  }
static char clk_askname_stop(int id,int xa,int ya,int xr,int yr)
  {
  id,xa,ya,xr,yr;
  if (ms_last_event.event_type & 0x2)
     {
     send_message(E_KEYBOARD,13);
     return 1;
     }
  else
     {
     send_message(E_KEYBOARD,27);
     return 1;
     }
  }


static void save_it(char ok)
  {
  if (ok)
     {
     save_game(slot_pos,global_gamename);
     read_slot_list();
     wire_proc();
	 GlobEvent(MAGLOB_AFTERSAVE,viewsector,viewdir);
     }
  else
     {
     wire_save_load(force_save);
     }
  }

#define CLK_ASK_NAME 2
T_CLK_MAP clk_ask_name[]=
  {
  {-1,0,0,639,479,clk_askname_stop,8+2,H_MS_DEFAULT},
  {-1,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };



void wire_ask_gamename(int id)
  {
  int x,y;

  x=SAVE_SLOT_S;
  y=id*SLOT_SPACE+21+SCREEN_OFFLINE;
  slot_pos=id;
  schovej_mysku();
  put_picture(x,y,ablock(H_LOADTXTR));
  strcpy(global_gamename,slot_list[id]);
  clk_ask_name[0].id=add_task(16384,type_text_v2,global_gamename,x,y,SAVE_SLOT_E-SAVE_SLOT_S,SAVE_NAME_SIZE,H_FBOLD,RGB555(31,31,0),save_it);
  change_click_map(clk_ask_name,CLK_ASK_NAME);
  ukaz_mysku();
  }


#define CLK_SAVELOAD 11
T_CLK_MAP clk_load[]=
  {
  {-1,LOAD_SLOT_S,SCREEN_OFFLINE,LOAD_SLOT_E,350,clk_load_proc,3,H_MS_DEFAULT},
  {-1,59,14+SCREEN_OFFLINE,306,46+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {1,59,310+SCREEN_OFFLINE,306,332+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {-1,54,378,497,479,start_invetory,2+8,-1},
  {-1,337,0,357,14,go_map,2,H_MS_DEFAULT},

  {-1,291,0,313,14,go_book,2,H_MS_DEFAULT},
  {-1,87,0,142,14,game_setup,2,H_MS_DEFAULT},
  {-1,30,0,85,14,konec,2,H_MS_DEFAULT},
  {1,147,0,205,14,clk_saveload,2,H_MS_DEFAULT},
  {-1,267,0,289,15,clk_sleep,2,H_MS_DEFAULT},
  {-1,0,0,639,479,close_saveload,9,H_MS_DEFAULT},
  };


static char clk_save_proc(int id,int xa,int ya,int xr,int yr)
  {
  id=bright_slot(yr-18);
  xa;ya;xr;yr;
  if (ms_last_event.event_type & 0x2 && id>=0)
     {
     unwire_proc();
     wire_ask_gamename(id);
     }
  return 1;
  }


T_CLK_MAP clk_save[]=
  {
  {-1,SAVE_SLOT_S,SCREEN_OFFLINE,SAVE_SLOT_E,350,clk_save_proc,3,H_MS_DEFAULT},
  {-1,59+274,14+SCREEN_OFFLINE,306+274,46+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {1,59+274,310+SCREEN_OFFLINE,306+274,332+SCREEN_OFFLINE,updown_scroll,2,H_MS_ZARE},
  {-1,54,378,497,479,start_invetory,2+8,-1},
  {-1,337,0,357,14,go_map,2,H_MS_DEFAULT},
  {-1,291,0,313,14,go_book,2,H_MS_DEFAULT},
  {-1,87,0,142,14,game_setup,2,H_MS_DEFAULT},
  {-1,30,0,85,14,konec,2,H_MS_DEFAULT},
  {0,207,0,265,14,clk_saveload,2,H_MS_DEFAULT},
  {-1,267,0,289,15,clk_sleep,2,H_MS_DEFAULT},
  {-1,0,0,639,479,close_saveload,9,H_MS_DEFAULT},
  };

static EVENT_PROC(saveload_keyboard)
  {
  user_ptr;
  WHEN_MSG(E_KEYBOARD)
     {
     switch (GET_DATA(word)>>8)
        {
        case 1:unwire_proc();wire_proc();break;
        case 'H':if (last_select>0) bright_slot((last_select-1)*SLOT_SPACE+1);break;
        case 'P':if (last_select<SLOTS_MAX-1) bright_slot((last_select+1)*SLOT_SPACE+1);break;
        case 28:ms_last_event.event_type|=0x2;
                if (force_save) clk_save_proc(0,0,0,0,last_select*SLOT_SPACE+1+18);
                else if (load_mode==4)
                  clk_load_proc_menu(0,0,0,0,last_select*SLOT_SPACE+1+18);
                else
                  clk_load_proc(0,0,0,0,last_select*SLOT_SPACE+1+18);
                break;
        }
     }
  }

void unwire_save_load()
  {
  if (back_texture!=NULL) free(back_texture);
  back_texture=NULL;
  if (story_text!=NULL)release_list(story_text);
  story_text=NULL;
  cancel_pass=0;
  send_message(E_DONE,E_KEYBOARD,saveload_keyboard);
  }

void wire_save_load(char save)
  {
  schovej_mysku();
  mute_all_tracks(0);
  force_save=save & 1;
  load_mode=save;
  if (slot_list==NULL) read_slot_list();
  curcolor=0;
  bar(0,17,639,17+360);
  if (force_save) redraw_save();else redraw_load();
  if (save==4) effect_show(NULL);else showview(0,0,0,0);
  redraw_story_bar(cur_story_pos);
  unwire_proc=unwire_save_load;
  send_message(E_ADD,E_KEYBOARD,saveload_keyboard);
  if (save==1)change_click_map(clk_save,CLK_SAVELOAD);
  else if (save==2) change_click_map(clk_load_error,CLK_LOAD_ERROR);
  else if (save==4) change_click_map(clk_load_menu,CLK_LOAD_MENU);
  else change_click_map(clk_load,CLK_SAVELOAD);
  cancel_pass=1;
  if (last_select!=-1)
     {
     int x=last_select*SLOT_SPACE+1;
     last_select=-1;
     bright_slot(x);
     }
  ukaz_mysku();
  update_mysky();
  }


void open_story_file()
  {
  char *c;

  story = openMemFile(STORY_BOOK);
  if (story == NULL) {
	  story = createMemFile(STORY_BOOK,0);
	  commitMemFile(story);
  }
  SEND_LOG("(STORY) Story temp file is opened....",0,0);
  }


void write_story_text(char *text)
  {
	  writeMemFile(&story,text,strlen(text));
	  writeMemFile(&story,"\n",1);
  }

void close_story_file()
  {
  story=NULL;
  SEND_LOG("(STORY) Story temp file is closed...",0,0);
  }

static int load_map_state_partial(char *level_fname,int mapsize) //obnovuje stav mapy; castecne
  {
  char *bf = 0;
  PMEMFILE fsta;
  unsigned int fpos = 0;
  int i;
  long siz;
  short res=-2;
  int ver = 0;


  fsta=openMemFile(level_fname);
  if (fsta==NULL) return -1;
  i=0;
  readMemFile(fsta,&fpos,&i,sizeof(mapsize));
  if (i==0)
  {
	  readMemFile(fsta,&fpos,&ver,sizeof(ver));
	  if (ver>STATE_CUR_VER) goto err;
	  readMemFile(fsta,&fpos,&i,sizeof(mapsize));
	  if (mapsize!=i) goto err;
	  SEND_LOG("(SAVELOAD) Loading map state for current map",0,0);
	  readMemFile(fsta,&fpos,&siz,sizeof(siz));
	  bf=(char *)getmem(siz);
	  readMemFile(fsta,&fpos,bf,siz);
	  for (i=0;i<mapsize;i++)
		  if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_AUTOMAP;
	  if (!readMemFile(fsta,&fpos,bf,siz)) goto err;
	  for (i=0;i<mapsize;i++)
		  if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_DISCLOSED;
  }
  else
  {
	  if (mapsize!=i) goto err;
	  SEND_LOG("(SAVELOAD) Partial restore for map: %s (%s)",level_fname,"START");
	  readMemFile(fsta,&fpos,&siz,sizeof(siz));
	  bf=(char *)getmem(siz);
	  readMemFile(fsta,&fpos,bf,siz);

	  for (i=0;i<mapsize;i++)
		 map_coord[i].flags|=(bf[i>>3]>>(i & 7)) & 1;
  }
  load_map_description(fsta,&fpos);
  readMemFile(fsta,&fpos,&i,2);
  while (i != -1 && i<=mapsize*4) {
	  readMemFile(fsta,&fpos,map_sides+i,sizeof(TSTENA));
	  readMemFile(fsta,&fpos,&i,2);
  }
  readMemFile(fsta,&fpos,&i,2);
  while (i != -1 && i<=mapsize) {
	  readMemFile(fsta,&fpos,map_sectors+i,sizeof(TSECTOR));
	  readMemFile(fsta,&fpos,&i,2);

  }
  res=0;
  err:
  free(bf);
  SEND_LOG("(SAVELOAD) Partial restore for map: %s (%s)",level_fname,"DONE");
  return res;
  }


int load_map_automap(char *mapfile)
  {
  int i;

  SEND_LOG("(SAVEGAME) CRITICAL SECTION - Swapping maps: %s <-> %s",level_fname,mapfile);
  kill_all_sounds();
  for(i=0;i<mapsize;i++) map_coord[i].flags&=~0x7f; //vynuluj flags_info
  free(map_sides);        //uvolni informace o stenach
  free(map_sectors);      //uvolni informace o sektorech
  free(map_coord);       //uvolni minfo informace
  load_org_map(mapfile,&map_sides,&map_sectors,&map_coord,&mapsize); //nahrej originalni mapu
  return load_map_state_partial(mapfile,mapsize); //nahrej ulozenou mapu
  }
//po teto akci se nesmi spustit TM_SCENE!!!! pokud mapfile!=level_fname
