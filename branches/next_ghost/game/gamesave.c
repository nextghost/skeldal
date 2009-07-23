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
//#include <skeldal_win.h>
//#include <debug.h>
//#include <dos.h>
//#include "libs/bios.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <conio.h>
#include <malloc.h>
//#include "libs/mem.h"
#include "libs/pcx.h"
#include "libs/types.h"
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/strlite.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/memman.h"
//#include <io.h>
#include "libs/sound.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "game/globals.h"
#include "libs/system.h"

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

static FILE *story=NULL;
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

static word vypocet_crc(char *data,long delka)
  {
  unsigned long l=0;
  do
     {
     l=(l<<8)|(delka>0?*data++:0);delka--;
     l=(l<<8)|(delka>0?*data++:0);delka--;
     l%=ZAKLAD_CRC;
     }
  while(delka>-1);
  return l & 0xffff;
  }
static unable_open_temp(char *c)
  {
  char d[]="Unable to open the file : ",*e;

  concat(e,d,c);
  closemode();
  Sys_ErrorBox(e);
//  MessageBox(NULL,e,NULL,MB_OK|MB_ICONSTOP);
  SEND_LOG("(SAVELOAD) Open temp error detected (%s)",c,0);
  exit(1);
  }

static unable_write_temp(char *c)
  {
  char d[]="Unable to write to the temp file : ",*e;

  concat(e,d,c);
  closemode();
  Sys_ErrorBox(e);
//  MessageBox(NULL,e,NULL,MB_OK|MB_ICONSTOP);
  SEND_LOG("(SAVELOAD) Open temp error detected (%s)",c,0);
  exit(1);
  }

void expand_map_file_name(char *s) //prepise *.map na fullpath\*.TMP
  {
  char *c;
  char *st;
  c=strchr(s,0);
  while (c!=s && *c!='.' && *c!='\\') c--;
  if (*c=='.') strcpy(c,".TMP");
  concat(st,pathtable[SR_TEMP],s);
  strcpy(s,st);
  }

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

void save_daction(FILE *f,int count,D_ACTION *ptr)
  {
  if (ptr!=NULL)
     {
     save_daction(f,count+1,ptr->next);
     fwrite(ptr,1,sizeof(D_ACTION),f);
     }
  else
     fwrite(&count,1,2,f);
  }

void load_daction(FILE *fsta)
  {
  int i,j;
  i=0;
  while (d_action!=NULL) //vymaz pripadne delaited actions
     {
     D_ACTION *p;
     p=d_action; d_action=p->next;free(p);
     }
  fread(&i,1,2,fsta);d_action=NULL;
  for(j=0;j<i;j++)
     {
     D_ACTION *p;

     p=(D_ACTION *)getmem(sizeof(D_ACTION));
     fread(p,1,sizeof(D_ACTION),fsta);
     p->next=d_action;
     d_action=p;
     }
  }

void save_items(FILE *f)
  {
  int i,j;
  short *c;

  for(i=0;i<mapsize*4;i++)
     if (map_items[i]!=NULL)
        {
        for(j=1,c=map_items[i];*c;c++,j++);
        fwrite(&i,1,2,f);
        fwrite(&j,1,2,f);
        fwrite(map_items[i],2,j,f);
        }
  i=-1;
  fwrite(&i,1,2,f);
  }

void restore_items(FILE *f)
  {
  short i,j;

  for(i=0;i<mapsize*4;i++) if (map_items[i]!=NULL) free(map_items[i]);
  memset(map_items,0,mapsize*16);
  while(fread(&i,1,2,f) && i!=-1)
     {
     fread(&j,1,2,f);
     map_items[i]=(short *)getmem(j*2);
     fread(map_items[i],2,j,f);
     }
  }

extern TSTR_LIST texty_v_mape;

void save_map_description(FILE *f)
  {
  int count,max;
  int i;

  if (texty_v_mape==NULL) max=0;else max=str_count(texty_v_mape);
  for(i=0,count=0;i<max;i++) if (texty_v_mape[i]!=NULL) count++;
  fwrite(&count,1,sizeof(count),f);
  for(i=0;i<max;i++) if (texty_v_mape[i]!=NULL)
     {
     int len;
     len=strlen(texty_v_mape[i]+12)+12+1;
     fwrite(&len,1,2,f);
     fwrite(texty_v_mape[i],1,len,f);
     }
  }

void load_map_description(FILE *f)
  {
  int count;
  int i;
  word len;

  if (texty_v_mape!=NULL)release_list(texty_v_mape);
  fread(&count,1,sizeof(count),f);
  if (!count)
     {
     texty_v_mape=NULL;
     return;
     }
  texty_v_mape=create_list(count);
  for(i=0;i<count;i++)
     {
     fread(&len,1,2,f);
        {
        char *s;
        s=(char *)alloca(len);
        memset(s,1,len-1);
        s[len-1]=0;
        str_replace(&texty_v_mape,i,s);
        }
     fread(texty_v_mape[i],1,len,f);
     }
  }

void save_vyklenky(FILE *fsta)
  {
  fwrite(&vyk_max,1,sizeof(vyk_max),fsta);
  if (vyk_max)
     fwrite(map_vyk,vyk_max,sizeof(TVYKLENEK),fsta);
  }

int load_vyklenky(FILE *fsta)
  {
  int i=0;
  fread(&i,1,sizeof(vyk_max),fsta);
  if (vyk_max)
     {
     if (i>vyk_max) return -2;
     fread(map_vyk,vyk_max,sizeof(TVYKLENEK),fsta);
     }
  return 0;
  }


void save_all_fly(FILE *fsta)
  {
  LETICI_VEC *f;

  f=letici_veci;
  fwrite(&f,1,sizeof(f),fsta);
  while (f!=NULL)
     {
     short *c;
     fwrite(f,1,sizeof(*f),fsta);
     c=f->items;
     if (c!=NULL) do fwrite(c,1,2,fsta); while (*c++);
     f=f->next;
     }
  }

int load_all_fly(FILE *fsta)
  {
  LETICI_VEC *f=NULL,*n,*p;
  fread(&f,1,sizeof(f),fsta);
  p=letici_veci;
  while (f!=NULL)
     {
     short items[100],*c;
     n=New(LETICI_VEC);
     c=items;memset(items,0,sizeof(items));
     if (fread(n,1,sizeof(*n),fsta)!=sizeof(*n))
        {
        free(n);
        if (p!=NULL) p->next=NULL;
        return -2;
        }
     if (n->items!=NULL)
        {
        do
          fread(c,1,2,fsta);
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
  FILE *fsta;
  int i;
  long siz;
 TSTENA *org_sides;
 TSECTOR *org_sectors;
  short res=-1;
  int ver=0;

  restore_sound_names();
  strcpy(sta,level_fname);
  expand_map_file_name(sta);
  fsta=fopen(sta,"wb");if (fsta==NULL) unable_open_temp(sta);
  SEND_LOG("(SAVELOAD) Saving map state for current map",0,0);
  if (load_org_map(level_fname,(void*)&org_sides,(void*)&org_sectors,NULL,NULL)) goto err;
  siz=(mapsize+7)/8;
  bf=(char *)getmem(siz);
  ver=0;
  fwrite(&ver,sizeof(ver),1,fsta);  //<-------------------------
  ver=STATE_CUR_VER;
  fwrite(&ver,sizeof(ver),1,fsta);  //<-------------------------
  fwrite(&mapsize,sizeof(mapsize),1,fsta);  //<-------------------------
  memset(bf,0,siz);
  fwrite(&siz,1,sizeof(siz),fsta);          //<-------------------------
  for(i=0;i<mapsize;i++)  //save automap
    if (map_coord[i].flags & MC_AUTOMAP) bf[i>>3]|=1<<(i & 7);
  if (!fwrite(bf,siz,1,fsta)) goto err;     //<-------------------------
  for(i=0;i<mapsize;i++)  //save disclosed
    if (map_coord[i].flags & MC_DISCLOSED) bf[i>>3]|=1<<(i & 7);
  if (!fwrite(bf,siz,1,fsta)) goto err;     //<-------------------------
  save_map_description(fsta);
  for(i=0;i<mapsize*4;i++)  //save changed sides
     if (memcmp(map_sides+i,org_sides+i,sizeof(TSTENA)))
        {
        fwrite(&i,1,2,fsta);
        if (fwrite(map_sides+i,1,sizeof(TSTENA),fsta)!=sizeof(TSTENA)) goto err;
        }
  i=-1;
  fwrite(&i,1,2,fsta);
  for(i=0;i<mapsize;i++)   //save changed sectors
     if (memcmp(map_sectors+i,org_sectors+i,sizeof(TSECTOR)))
        {
        fwrite(&i,1,2,fsta);
        if (fwrite(map_sectors+i,1,sizeof(TSECTOR),fsta)!=sizeof(TSECTOR)) goto err;
        }
  i=-1;
  fwrite(&i,1,2,fsta);
  for(i=0;i<MAX_MOBS;i++) if (mobs[i].vlajky & MOB_LIVE)
     {
     fwrite(&i,1,2,fsta);
     fwrite(mobs+i,1,sizeof(TMOB),fsta); //save_mobmap
     }
  i=-1;
  fwrite(&i,1,2,fsta);
  i=mapsize*4;
  fwrite(&i,1,sizeof(i),fsta); //save flag maps //<-------------------------
  if (fwrite(flag_map,1,i,fsta)!=(unsigned)i) goto err;   //<-------------------------
  save_daction(fsta,0,d_action); //save dactions//<-------------------------
  fwrite(&macro_block_size,1,sizeof(macro_block_size),fsta);
  if (macro_block_size)fwrite(macro_block,1,macro_block_size,fsta);//save_macros
  if (save_codelocks(fsta)) goto err;
  save_items(fsta);
  save_vyklenky(fsta);
  save_all_fly(fsta);
  save_enemy_paths(fsta);
  res=0;
  err:
  SEND_LOG("(SAVELOAD) State of current map saved (err:%d)",res,0);
  fclose(fsta);
  free(org_sectors);
  free(org_sides);
  free(bf);
  if (res)
     {
     remove(sta);
     unable_write_temp(sta);
     }
  return res;
  }

int load_map_state() //obnovuje stav mapy; nutno volat po zavolani load_map;
  {
  char sta[200];
  char *bf;
  FILE *fsta;
  int i;
  long siz;
  short res=-2;
  int ver=0;


  strcpy(sta,level_fname);
  expand_map_file_name(sta);
  fsta=fopen(sta,"rb");if (fsta==NULL) return -1;
  i=0;fread(&i,sizeof(mapsize),1,fsta);
  if (i==0)
  {
    if (!fread(&ver,sizeof(ver),1,fsta)) goto err;
    if (ver>STATE_CUR_VER) goto err;
    if (!fread(&i,sizeof(mapsize),1,fsta)) goto err;
    if (mapsize!=i) goto err;
    SEND_LOG("(SAVELOAD) Loading map state for current map",0,0);
    fread(&siz,1,sizeof(siz),fsta);
    bf=(char *)getmem(siz);
    if (!fread(bf,siz,1,fsta)) goto err;
    for (i=0;i<mapsize;i++)
      if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_AUTOMAP;
    if (!fread(bf,siz,1,fsta)) goto err;
    for (i=0;i<mapsize;i++)
      if ((bf[i>>3]>>(i & 7)) & 1) map_coord[i].flags|=MC_DISCLOSED;
   }
  else
  {
    if (mapsize!=i) return fclose(fsta);
    SEND_LOG("(SAVELOAD) Loading map state for current map",0,0);
    fread(&siz,1,sizeof(siz),fsta);
    bf=(char *)getmem(siz);
    if (!fread(bf,siz,1,fsta)) goto err;
    for (i=0;i<mapsize;i++)
       map_coord[i].flags|=(bf[i>>3]>>(i & 7)) & 1;
  }
  load_map_description(fsta);
  while (fread(&i,1,2,fsta) && i<=mapsize*4)
     if (fread(map_sides+i,1,sizeof(TSTENA),fsta)!=sizeof(TSTENA)) goto err;
  while (fread(&i,1,2,fsta) && i<=mapsize)
     if (fread(map_sectors+i,1,sizeof(TSECTOR),fsta)!=sizeof(TSECTOR)) goto err;
  if (reset_mobiles)  //reloads mobiles if flag present
    {
    char mm[MAX_MOBS];
    for(i=0;i<MAX_MOBS;mobs[i].vlajky &=~MOB_LIVE,i++)
      if (mobs[i].vlajky & MOB_LIVE) mm[i]=1;else mm[i]=0;
    while (fread(&i,1,2,fsta) && i<=MAX_MOBS)
      {
      if (mm[i]) mobs[i].vlajky |=MOB_LIVE;
      fseek(fsta,sizeof(TMOB),SEEK_CUR);
      }
    reset_mobiles=0;
    }
  else
    {
    for(i=0;i<MAX_MOBS;(mobs[i].vlajky &=~MOB_LIVE),i++);
    while (fread(&i,1,2,fsta) && i<=MAX_MOBS)
       if (fread(mobs+i,1,sizeof(TMOB),fsta)!=sizeof(TMOB)) goto err;
    }
  for(i=0;i<MAX_MOBS;i++) mobs[i].vlajky &=~MOB_IN_BATTLE;
  refresh_mob_map();
  fread(&i,1,sizeof(i),fsta);
  fread(flag_map,1,i,fsta);
  load_daction(fsta);
  fread(&i,1,sizeof(macro_block_size),fsta);
  if (macro_block_size && i==macro_block_size)fread(macro_block,1,macro_block_size,fsta);
  else
     {
     fseek(fsta,i,SEEK_CUR);
     SEND_LOG("(ERROR) Multiaction: Sizes mismatch %d != %d",i,macro_block_size);
     }
  if (load_codelocks(fsta)) goto err;
  restore_items(fsta);
  res=0;
  res|=load_vyklenky(fsta);
  res|=load_all_fly(fsta);
  res|=load_enemy_paths(fsta);
  err:
  SEND_LOG("(SAVELOAD) State of current map loaded (err:%d)",res,0);
  fclose(fsta);
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
  load_org_map(level_fname,(void*)&map_sides,(void*)&map_sectors,(void*)&map_coord,NULL); //nahrej originalni mapu
  load_map_state(); //nahrej ulozenou mapu
  for(i=1;i<mapsize*4;i++) call_macro(i,MC_STARTLEV);
  }

inline char rotate(char c)
  {
// FIXME: rewrite
/*  __asm
    {
    mov al,c
    rol al,1;
    }
*/
  }

//#pragma aux rotate parm [al] value [al]="rol al,1";

/* errors
     -1 end of file
      1 disk error
      2 internal error
      3 checksum error
 */
int pack_status_file(FILE *f,char *status_name)
  {
  int stt;
  char rcheck=0;
  long fsz;
  char *buffer,*c,*fullnam;
  word crc;

  SEND_LOG("(SAVELOAD) Packing status file '%s'",status_name,0);
  fullnam=alloca(strlen(status_name)+strlen(pathtable[SR_TEMP])+1);
  if (fullnam==NULL) return 2;
  strcpy(fullnam,pathtable[SR_TEMP]);
  strcat(fullnam,status_name);
// FIXME: O_BINARY is Windows-specific
//  stt=open(fullnam,O_RDONLY | O_BINARY);
  stt=open(fullnam,O_RDONLY);
//  fsz=filelength(stt);
  fsz = lseek(stt, 0, SEEK_END);
  lseek(stt, 0, SEEK_SET);
  c=buffer=getmem(fsz+12+4+2);
  strcpy(c,status_name);c+=12;
  *(long *)c=fsz+2;
  c+=sizeof(long);
  read(stt,c,fsz);
  close(stt);
  crc=vypocet_crc(c,fsz);
  c+=fsz;
  memcpy(c,&crc,sizeof(crc));
  fsz+=12+4+2;
  rcheck=(fwrite(buffer,1,fsz,f)!=(unsigned)fsz);
  free(buffer);
  return  rcheck;
  }

int unpack_status_file(FILE *f)
  {
  int stt;
  char rcheck=0;
  long fsz;
  char *buffer,*c,*fullnam;
  char name[13];
  word crc,crccheck;

  name[12]=0;
  name[11]=0;
  fread(name,1,12,f);
  SEND_LOG("(SAVELOAD) Unpacking status file '%s'",name,0);
  if (name[0]==0) return -1;
  fread(&fsz,1,4,f);
  c=buffer=(char *)getmem(fsz);
  if (fread(buffer,1,fsz,f)!=(unsigned)fsz) return 1;
  fullnam=alloca(strlen(name)+strlen(pathtable[SR_TEMP])+2);
  if (fullnam==NULL) return 2;
  strcpy(fullnam,pathtable[SR_TEMP]);
  strcat(fullnam,name);
  fsz-=2;
  crc=vypocet_crc(c,fsz);
  c+=fsz;memcpy(&crccheck,c,sizeof(crccheck));
  if (crc!=crccheck)
     {
     free(buffer);
     return 3;
     }
// FIXME: Windows-specific flags
//  stt=open(fullnam,O_BINARY | O_RDWR | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE);
  stt=open(fullnam, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (stt==-1)
     {
     free(buffer);
     return 1;
     }
  rcheck=(write(stt,buffer,fsz)!=fsz) ;
  free(buffer);
  close(stt);
  return rcheck;
  }

int pack_all_status(FILE *f)
  {
// FIXME: rewrite
/*
  char *c;
  WIN32_FIND_DATA inf;
  HANDLE res;

  concat(c,pathtable[SR_TEMP],"*.TMP");
  res=FindFirstFile(c,&inf);
  if (res!=INVALID_HANDLE_VALUE)
    do
     {
     int i;
     if (inf.cFileName[0]!='~')
        {
        i=pack_status_file(f,inf.cFileName);
        if (i) return i;
        }
     }
    while (FindNextFile(res,&inf));     
  FindClose(res);
  c[0]=0;
  fwrite(c,1,12,f);
*/
  return 0;
  }

int unpack_all_status(FILE *f)
  {
  int i;

  i=0;
  while (!i) i=unpack_status_file(f);
  if (i==-1) i=0;
  return i;
  }

int save_basic_info()
  {
  FILE *f;
  char *c;
  S_SAVE s;
  short *p;
  int i;
  char res=0;
  THUMAN *h;

  concat(c,pathtable[SR_TEMP],_GAME_ST);
  SEND_LOG("(SAVELOAD) Saving basic info for game (file:%s)",c,0);
  f=fopen(c,"wb");
  if (f==NULL) return 1;
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
  s.sample_vol=Sound_GetEffect(SND_GFX);
  s.music_vol=Sound_GetEffect(SND_MUSIC);
  s.xbass=Sound_GetEffect(SND_XBASS);
  s.bass=Sound_GetEffect(SND_BASS);
  s.treble=Sound_GetEffect(SND_TREBL);
  s.stereing=Sound_GetEffect(SND_LSWAP);
  s.swapchans=Sound_GetEffect(SND_SWAP);
  s.out_filter=Sound_GetEffect(SND_OUTFILTER);
  s.autosave=autosave_enabled;
	s.game_flags=(enable_glmap!=0);
  strncpy(s.level_name,level_fname,12);
  for(i=0;i<5;i++) s.runes[i]=runes[i];
  if (picked_item!=NULL)
     for(i=1,p=picked_item;*p;i++,p++);else i=0;
  s.picks=i;
  s.items_added=item_count-it_count_orgn;
  res|=(fwrite(&s,1,sizeof(s),f)!=sizeof(s));
  if (i)
     res|=(fwrite(picked_item,2,i,f)!=(unsigned)i);
  if (s.items_added)
     res|=(fwrite(glob_items+it_count_orgn,sizeof(TITEM),s.items_added,f)!=(unsigned)s.items_added);
  res|=save_spells(f);
  if (!res) res|=(fwrite(postavy,1,sizeof(postavy),f)!=sizeof(postavy));
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++) if (h->demon_save!=NULL)
     fwrite(h->demon_save,sizeof(THUMAN),1,f);       //ulozeni polozek s demony
  res|=save_dialog_info(f);
  fclose(f);
  SEND_LOG("(SAVELOAD) Done... Result: %d",res,0);
  return res;
  }

int load_basic_info()
  {
  FILE *f;
  char *c;
  S_SAVE s;
  int i;
  char res=0;
  TITEM *itg;
  THUMAN *h;

  concat(c,pathtable[SR_TEMP],_GAME_ST);
  SEND_LOG("(SAVELOAD) Loading basic info for game (file:%s)",c,0);
  f=fopen(c,"rb");
  if (f==NULL) return 1;
  res|=(fread(&s,1,sizeof(s),f)!=sizeof(s));
	if (s.game_flags & GM_MAPENABLE) enable_glmap=1;else enable_glmap=0;
  i=s.picks;
  if (picked_item!=NULL) free(picked_item);
  if (i)
     {
     picked_item=NewArr(short,i);
     res|=(fread(picked_item,2,i,f)!=(unsigned)i);
     }
  else picked_item=NULL;
  itg=NewArr(TITEM,it_count_orgn+s.items_added);
  memcpy(itg,glob_items,it_count_orgn*sizeof(TITEM));
  free(glob_items);glob_items=itg;
  if (s.items_added)
     res|=(fread(glob_items+it_count_orgn,sizeof(TITEM),s.items_added,f)!=(unsigned)s.items_added);
  item_count=it_count_orgn+s.items_added;
  res|=load_spells(f);
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++) if (h->demon_save!=NULL) free(h->demon_save);
  if (!res) res|=(fread(postavy,1,sizeof(postavy),f)!=sizeof(postavy));
  for(i=0,h=postavy;i<POCET_POSTAV;h++,i++)
        {
        h->programovano=0;
        h->provadena_akce=h->zvolene_akce=NULL;
        h->dostal=0;
        if (h->demon_save!=NULL)
          {
          h->demon_save=New(THUMAN);
          fread(h->demon_save,sizeof(THUMAN),1,f);//obnova polozek s demony
          }
        }
  res|=load_dialog_info(f);
  fclose(f);
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
  Sound_SetEffect(SND_GFX,s.sample_vol);
  Sound_SetEffect(SND_MUSIC,s.music_vol);
  Sound_SetEffect(SND_XBASS,s.xbass);
  Sound_SetEffect(SND_BASS,s.bass);
  Sound_SetEffect(SND_TREBL,s.treble);
  Sound_SetEffect(SND_LSWAP,s.stereing);
  Sound_SetEffect(SND_SWAP,s.swapchans);
  Sound_SetEffect(SND_OUTFILTER,s.out_filter);
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
  Sys_Mkdir(p);
}

static int save_global_events()
{
  FILE *f;
  char *c;
  concat(c,pathtable[SR_TEMP],_GLOBAL_ST );
  f=fopen(c,"wb");
  if (f==NULL) return 1;
  fwrite(GlobEventList,1,sizeof(GlobEventList),f);
  fclose(f);
  return 0;
}

static int load_global_events()
{
  FILE *f;
  char *c;
  memset(GlobEventList,0,sizeof(GlobEventList));

  concat(c,pathtable[SR_TEMP],_GLOBAL_ST );
  f=fopen(c,"rb");
  if (f==NULL) return 1;
  fread(GlobEventList,1,sizeof(GlobEventList),f);
  fclose(f);
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
	Sys_ErrorBox(buff);
//    MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);  
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
//  if (GetKeyState(VK_CONTROL) & 0x80) correct_level();
  if (get_control_state() & 0x80) correct_level();
  return r;
  }

static void load_specific_file(int slot_num,char *filename,void **out,long *size) //call it in task!
  {
  FILE *slot;
  char *c,*d;
  long siz;
  char fname[12];
  char succes=0;

  concat(c,pathtable[SR_SAVES],_SLOT_SAV);
  d=alloca(strlen(c)+2);
  sprintf(d,c,slot_num);
  slot=fopen(d,"rb");
  if (slot==NULL)
     {
     *out=NULL;
     return;
     }
  fseek(slot,SAVE_NAME_SIZE,SEEK_CUR);
  fread(fname,1,12,slot);
  while(fname[0] && !succes)
     {
     Task_Sleep(NULL);
     if (Task_QuitMsg()) break;
     fread(&siz,1,4,slot);
     if (!strncmp(fname,filename,12)) succes=1; else
           {
           fseek(slot,siz,SEEK_CUR);
           fread(fname,1,12,slot);
           }
     }
  if (succes)
     {
     *out=getmem(siz);
     fread(*out,1,siz,slot);
     *size=siz;
     }
  else *out=NULL;
  fclose(slot);
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
static void read_story_task(va_list args)
  {
  int slot=va_arg(args,int);

  TSTR_LIST ls;
  void *text_data;
  char *c,*d;
  long size;

  load_specific_file(slot,STORY_BOOK,&text_data,&size);
  if (text_data!=NULL)
     {
     ls=create_list(2);
     c=text_data;
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
     free(text_data);
     }
  else ls=NULL;
  if (story_text!=NULL) release_list(story_text);
  story_text=ls;
  cur_story_pos=get_list_count();if (cur_story_pos<0) cur_story_pos=0;
  redraw_story_bar(cur_story_pos);
  }

static void read_story(int slot)
  {
  static task_num=-1;

  if (task_num!=-1) Task_Term(task_num);
  if (slot!=-1)
     task_num=Task_Add(8196,read_story_task,slot);
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

static char updown_scroll(int id,int xa,int ya,int xr,int yr);

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
  clk_ask_name[0].id=Task_Add(16384,type_text_v2,global_gamename,x,y,SAVE_SLOT_E-SAVE_SLOT_S,SAVE_NAME_SIZE,H_FBOLD,RGB555(31,31,0),save_it);
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
  int err;

  concat(c,pathtable[SR_TEMP],STORY_BOOK);
  story=fopen(c,"a");
//  err=GetLastError();
  if (story==NULL) story=fopen(c,"w");
  if (story==NULL)
     unable_open_temp(c);
  SEND_LOG("(STORY) Story temp file is opened....",0,0);
  }


void write_story_text(char *text)
  {
  if (fputs(text,story)==EOF)
     unable_write_temp(STORY_BOOK);
  if (fputs("\n",story)==EOF)
     unable_write_temp(STORY_BOOK);
  }

void close_story_file()
  {
  if (story!=NULL)  fclose(story);
  story=NULL;
  SEND_LOG("(STORY) Story temp file is closed...",0,0);
  }

static int load_map_state_partial(char *level_fname,int mapsize) //obnovuje stav mapy; castecne
  {
  char sta[200];
  char *bf;
  FILE *fsta;
  int i;
  long siz;
  short res=-2;


  strcpy(sta,level_fname);
  expand_map_file_name(sta);
  fsta=fopen(sta,"rb");if (fsta==NULL) return -1;
  i=0;fread(&i,sizeof(mapsize),1,fsta);if (mapsize!=i)goto err;
  SEND_LOG("(SAVELOAD) Partial restore for map: %s (%s)",level_fname,"START");
  fread(&siz,1,sizeof(siz),fsta);
  bf=(char *)getmem(siz);
  if (!fread(bf,siz,1,fsta)) goto err;
  for (i=0;i<mapsize;i++)
     map_coord[i].flags|=(bf[i>>3]>>(i & 7)) & 1;
  load_map_description(fsta);
  while (fread(&i,1,2,fsta) && i<=mapsize*4)
     if (fread(map_sides+i,1,sizeof(TSTENA),fsta)!=sizeof(TSTENA)) goto err;
  while (fread(&i,1,2,fsta) && i<=mapsize)
     if (fread(map_sectors+i,1,sizeof(TSECTOR),fsta)!=sizeof(TSECTOR)) goto err;
  res=0;
  err:
  free(bf);
  fclose(fsta);
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
  load_org_map(mapfile,(void*)&map_sides,(void*)&map_sectors,(void*)&map_coord,(void*)&mapsize); //nahrej originalni mapu
  return load_map_state_partial(mapfile,mapsize); //nahrej ulozenou mapu
  }
//po teto akci se nesmi spustit TM_SCENE!!!! pokud mapfile!=level_fname
