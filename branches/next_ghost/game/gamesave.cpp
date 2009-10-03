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
#include "libs/pcx.h"
#include <inttypes.h>
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/strlite.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/memman.h"
#include "libs/sound.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "game/globals.h"
#include "libs/system.h"

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

#pragma pack(1)
typedef struct s_save {
	int32_t viewsector;
	int8_t viewdir;
	int16_t version;
	int8_t not_used;
	int32_t gold;
	int16_t cur_group;
	int8_t autosave;
	int8_t enable_sort;
	int8_t shownames;
	int8_t showlives;
	int8_t zoom_speed;
	int8_t turn_speed;
	int8_t autoattack;
	uint8_t music_vol;
	uint8_t sample_vol;
	int8_t xbass;
	int8_t bass;
	int8_t treble;
	int8_t stereing;
	int8_t swapchans;
	int8_t out_filter;
	int32_t glob_flags;
	int32_t game_time;
	int8_t runes[5];
	char level_name[12];
	int16_t picks;  //pocet_sebranych predmetu v mysi
	int16_t items_added; //pocet_pridanych predmetu
	int32_t sleep_long;
	int32_t game_flags;
} S_SAVE;
#pragma option align=reset

#define ZAKLAD_CRC 0xC005

static int get_list_count();

static uint16_t vypocet_crc(uint8_t *data,long delka)
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
static void unable_open_temp(char *c)
  {
  char d[]="Unable to open the file: ",*e;

  concat(e,d,c);
  closemode();
  Sys_ErrorBox(e);
//  MessageBox(NULL,e,NULL,MB_OK|MB_ICONSTOP);
  SEND_LOG("(SAVELOAD) Open temp error detected (%s)",c,0);
  exit(1);
  }

static void unable_write_temp(const char *c)
  {
  char d[]="Unable to write to the temp file : ",*e;

  concat(e,d,c);
  closemode();
  Sys_ErrorBox(e);
//  MessageBox(NULL,e,NULL,MB_OK|MB_ICONSTOP);
  SEND_LOG("(SAVELOAD) Open temp error detected (%s)",c,0);
  exit(1);
  }

//prepise *.map na fullpath\*.TMP
void expand_map_file_name(char *s) {
	char *c;
	char *st;

	c = strchr(s, 0);

	while (c != s && *c != '.' && *c != '\\') {
		c--;
	}

	if (*c == '.') {
		strcpy(c,".TMP");
	}

//  concat(st,pathtable[SR_TEMP],s);
//  strcpy(s,st);
	strcpy(s, Sys_FullPath(SR_TEMP, s));
}

void save_daction(WriteStream &stream, int count, D_ACTION *ptr) {
	if (ptr) {
		save_daction(stream, count + 1, ptr->next);
		stream.writeUint16LE(ptr->action);
		stream.writeUint16LE(ptr->sector);
		stream.writeUint16LE(ptr->side);
		stream.writeUint16LE(ptr->flags);
		stream.writeUint16LE(ptr->nocopy);
		stream.writeUint16LE(ptr->delay);
		stream.writeUint32LE(ptr->next != NULL);
	} else {
		stream.writeUint16LE(count);
	}
}

void load_daction(ReadStream &stream) {
	int i, j;

	i = 0;

	//vymaz pripadne delaited actions
	while (d_action != NULL) {
		D_ACTION *p;
		p = d_action;
		d_action = p->next;
		free(p);
	}

	i = stream.readUint16LE();
	d_action = NULL;

	for (j = 0; j < i; j++) {
		D_ACTION *p;

		p = (D_ACTION *)getmem(sizeof(D_ACTION));
		p->action = stream.readUint16LE();
		p->sector = stream.readUint16LE();
		p->side = stream.readUint16LE();
		p->flags = stream.readUint16LE();
		p->nocopy = stream.readUint16LE();
		p->delay = stream.readUint16LE();
		stream.readUint32LE();	// p->next; the value is meaningless
		p->next = d_action;
		d_action = p;
	}
}

//uklada stav mapy pro savegame (neuklada aktualni pozici);
int save_map_state() {
	restore_sound_names();
	return gameMap.save();
}

//obnovuje stav mapy; nutno volat po zavolani load_map;
int load_map_state() {
	return gameMap.restore();
}

//pouze obnovuje ulozeny stav aktualni mapy
void restore_current_map() {
	SEND_LOG("(SAVELOAD) Restore map...", 0, 0);
	kill_all_sounds();
	gameMap.quickRestore();
}

/*
inline char rotate(char c)
  {
  __asm
    {
    mov al,c
    rol al,1;
    }
  }
*/

//#pragma aux rotate parm [al] value [al]="rol al,1";

/* errors
     -1 end of file
      1 disk error
      2 internal error
      3 checksum error
 */
int pack_status_file(WriteStream &stream, const char *status_name) {
	char rcheck = 0;
	unsigned fsz;
	unsigned char *buffer;
	uint16_t crc;
	File file;

	SEND_LOG("(SAVELOAD) Packing status file '%s'", status_name, 0);
	file.open(Sys_FullPath(SR_TEMP, status_name));
	fsz = file.size();
	stream.write(status_name, 12);
	stream.writeSint32LE(fsz + 2);
	buffer = new unsigned char[fsz];
	file.read(buffer, fsz);
	crc = vypocet_crc(buffer, fsz);
	rcheck = stream.write(buffer, fsz) != fsz;
	stream.writeUint16LE(crc);
	free(buffer);
	return rcheck;
}

int unpack_status_file(FILE *f) {
	FILE *stt;
	char rcheck = 0;
	long fsz;
	char *fullnam;
	uint8_t *buffer, *c;
	char name[13];
	uint16_t crc, crccheck;
	size_t stfu;

	name[12] = 0;
	name[11] = 0;
	stfu = fread(name, 1, 12, f);
	SEND_LOG("(SAVELOAD) Unpacking status file '%s'", name, 0);

	if (name[0] == 0) {
		return -1;
	}

	stfu = fread(&fsz, 1, 4, f);
	c = buffer = (uint8_t*)getmem(fsz);

	if (fread(buffer, 1, fsz, f) != (unsigned)fsz) {
		return 1;
	}

	fsz -= 2;
	crc = vypocet_crc(c, fsz);
	c += fsz;
	memcpy(&crccheck, c, sizeof(crccheck));

	if (crc != crccheck) {
		free(buffer);
		return 3;
	}

	stt = fopen(Sys_FullPath(SR_TEMP, name), "wb+");

	if (!stt) {
		free(buffer);
		return 1;
	}

	rcheck = fwrite(buffer, 1, fsz, stt) != fsz;
	free(buffer);
	fclose(stt);
	return rcheck;
}

int unpack_all_status(FILE *f)
  {
  int i;

  i=0;
  while (!i) i=unpack_status_file(f);
  if (i==-1) i=0;
  return i;
  }

int save_basic_info() {
	FILE *f;
	char *c;
	S_SAVE s;
	short *p;
	int i;
	char res = 0;
	THUMAN *h;
	size_t stfu;

	c = Sys_FullPath(SR_TEMP, _GAME_ST);
	SEND_LOG("(SAVELOAD) Saving basic info for game (file:%s)", c, 0);
	f = fopen(c, "wb");

	if (f == NULL) {
		return 1;
	}

	s.viewsector = viewsector;
	s.viewdir = viewdir;
	s.version = SSAVE_VERSION;
	s.not_used = 0;
	s.gold = money;
	s.cur_group = cur_group;
	s.shownames = show_names;
	s.showlives = show_lives;
	s.autoattack = autoattack;
	s.turn_speed = turn_speed(-1);
	s.zoom_speed = zoom_speed(-1);
	s.game_time = game_time;
	s.enable_sort = enable_sort;
	s.sleep_long = sleep_ticks;
	s.sample_vol = Sound_GetEffect(SND_GFX);
	s.music_vol = Sound_GetEffect(SND_MUSIC);
	s.xbass = Sound_GetEffect(SND_XBASS);
	s.bass = Sound_GetEffect(SND_BASS);
	s.treble = Sound_GetEffect(SND_TREBL);
	s.stereing = Sound_GetEffect(SND_LSWAP);
	s.swapchans = Sound_GetEffect(SND_SWAP);
	s.out_filter = Sound_GetEffect(SND_OUTFILTER);
	s.autosave = autosave_enabled;
	s.game_flags = (enable_glmap != 0);
	strncpy(s.level_name, gameMap.fname(), 12);

	for (i = 0; i < 5; i++) {
		s.runes[i] = runes[i];
	}

	if (picked_item != NULL) {
		for (i = 1, p = picked_item; *p; i++, p++);
	} else {
		i = 0;
	}

	s.picks = i;
	s.items_added = item_count - it_count_orgn;
	res |= (fwrite(&s, 1, sizeof(s), f) != sizeof(s));

	if (i) {
		res |= (fwrite(picked_item, 2, i, f) != (unsigned)i);
	}

	if (s.items_added) {
		res |= (fwrite(glob_items + it_count_orgn, sizeof(TITEM), s.items_added, f) != (unsigned)s.items_added);
	}

	res |= save_spells(f);

	if (!res) {
		res |= (fwrite(postavy, 1, sizeof(postavy), f) != sizeof(postavy));
	}

	for (i = 0, h = postavy; i < POCET_POSTAV; h++, i++) {
		if (h->demon_save != NULL) {
			stfu = fwrite(h->demon_save, sizeof(THUMAN), 1, f);       //ulozeni polozek s demony
		}
	}

	res |= save_dialog_info(f);
	fclose(f);
	SEND_LOG("(SAVELOAD) Done... Result: %d", res, 0);
	return res;
}

static void loadHuman(THUMAN &human, ReadStream &stream) {
	int i;

	human.used = stream.readSint8();
	human.spell = stream.readSint8();
	human.groupnum = stream.readSint8();
	human.xicht = stream.readSint8();
	human.direction = stream.readSint8();
	human.sektor = stream.readSint16LE();

	for (i = 0; i < VLS_MAX; i++) {
		human.vlastnosti[i] = stream.readSint16LE();
	}

	for (i = 0; i < TPW_MAX; i++) {
		human.bonus_zbrani[i] = stream.readSint16LE();
	}

	human.lives = stream.readSint16LE();
	human.mana = stream.readSint16LE();
	human.kondice = stream.readSint16LE();
	human.actions = stream.readSint16LE();
	human.mana_battery = stream.readSint16LE();

	for (i = 0; i < VLS_MAX; i++) {
		human.stare_vls[i] = stream.readSint16LE();
	}

	for (i = 0; i < HUMAN_PLACES; i++) {
		human.wearing[i] = stream.readSint16LE();
	}

	for (i = 0; i < HUMAN_RINGS; i++) {
		human.prsteny[i] = stream.readSint16LE();
	}

	human.sipy = stream.readSint16LE();
	human.inv_size = stream.readSint16LE();

	for (i = 0; i < MAX_INV; i++) {
		human.inv[i] = stream.readSint16LE();
	}

	human.level = stream.readSint16LE();

	for (i = 0; i < TPW_MAX; i++) {
		human.weapon_expy[i] = stream.readSint16LE();
	}

	human.exp = stream.readSint32LE();
	human.female = stream.readSint8();
	human.utek = stream.readSint8();
	stream.readUint32LE();	// pointers, ignore
	stream.readUint32LE();
	human.zvolene_akce = NULL;
	human.provadena_akce = NULL;
	human.programovano = stream.readSint8();
	stream.read(human.jmeno, 15);
	human.zasah = stream.readSint16LE();
	human.dostal = stream.readSint16LE();
	human.bonus = stream.readSint16LE();
	human.jidlo = stream.readSint32LE();
	human.voda = stream.readSint32LE();
	human.demon_save = stream.readUint32LE() ? &human : NULL;
}

// FIXME: move this function here
void loadItem(TITEM &item, ReadStream &stream);

int load_basic_info() {
	char *c, level_name[13];
	int i, size, items_added, game_flags;
	TITEM *itg;
	THUMAN *h;
	File file;

	c = Sys_FullPath(SR_TEMP, _GAME_ST);
	SEND_LOG("(SAVELOAD) Loading basic info for game (file:%s)", c, 0);
	file.open(c);

	if (!file.isOpen()) {
		return 1;
	}

	viewsector = file.readSint32LE();
	viewdir = file.readSint8();
	file.readSint16LE();	// version, ignore
	file.readSint8();	// not used, ignore
	money = file.readSint32LE();
	cur_group = file.readSint16LE();
	autosave_enabled = file.readSint8();
	enable_sort = file.readSint8();
	show_names = file.readSint8();
	show_lives = file.readSint8();
	zoom_speed(file.readSint8());
	turn_speed(file.readSint8());
	autoattack = file.readSint8();

	Sound_SetEffect(SND_MUSIC, file.readUint8());
	Sound_SetEffect(SND_GFX, file.readUint8());
	Sound_SetEffect(SND_XBASS, file.readSint8());
	Sound_SetEffect(SND_BASS, file.readSint8());
	Sound_SetEffect(SND_TREBL, file.readSint8());
	Sound_SetEffect(SND_LSWAP, file.readSint8());
	Sound_SetEffect(SND_SWAP, file.readSint8());
	Sound_SetEffect(SND_OUTFILTER, file.readSint8());
	file.readSint32LE();	// global flags, ignore
	game_time = file.readSint32LE();
	runes[0] = file.readSint8();
	runes[1] = file.readSint8();
	runes[2] = file.readSint8();
	runes[3] = file.readSint8();
	runes[4] = file.readSint8();
	file.read(level_name, 12);
	level_name[12] = '\0';
	size = file.readSint16LE();
	items_added = file.readSint16LE();
	sleep_ticks = file.readSint32LE();
	game_flags = file.readSint32LE();

	if (game_flags & GM_MAPENABLE) {
		enable_glmap = 1;
	} else {
		enable_glmap = 0;
	}

	if (picked_item != NULL) {
		free(picked_item);	// FIXME: rewrite to new/delete
	}

	if (size) {
		picked_item = NewArr(short, size);	// FIXME: rewrite to new/delete

		for (i = 0; i < size; i++) {
			picked_item[i] = file.readSint16LE();
		}
	} else {
		picked_item = NULL;
	}

	// FIXME: rewrite to new/delete
	itg = NewArr(TITEM, it_count_orgn + items_added);
	memcpy(itg, glob_items, it_count_orgn * sizeof(TITEM));
	free(glob_items);
	glob_items = itg;

	for (i = 0; i < items_added; i++) {
		loadItem(glob_items[it_count_orgn + i], file);
	}

	item_count = it_count_orgn + items_added;
	load_spells(file);

	for (i = 0, h = postavy; i < POCET_POSTAV; h++, i++) {
		if (h->demon_save != NULL) {
			free(h->demon_save);
		}
	}

	if (!file.eos()) {
		for (i = 0; i < POCET_POSTAV; i++) {
			loadHuman(postavy[i], file);
		}
	}

	for (i = 0, h = postavy; i < POCET_POSTAV; h++, i++) {
		h->programovano = 0;
		h->provadena_akce = h->zvolene_akce = NULL;
		h->dostal = 0;

		if (h->demon_save != NULL) {
			h->demon_save = New(THUMAN);	// FIXME: rewrite to new/delete
			loadHuman(*h->demon_save, file);
		}
	}

	load_dialog_info(file);

	if (!gameMap.fname() || strcmp(level_name, gameMap.fname())) {
		strcpy(loadlevel.name, level_name);
		loadlevel.start_pos = viewsector;
		loadlevel.dir = viewdir;
		send_message(E_CLOSE_MAP);
		load_another = 1;
	} else {
		load_another = 0;
	}

	for (i = 0; i < POCET_POSTAV; i++) {
		postavy[i].dostal = 0;
	}

	SEND_LOG("(SAVELOAD) Done... Result: %d", file.eos(), 0);
	return file.eos();
}

/*
static void MakeSaveGameDir(const char *name)
{
  char *p=(char *)alloca(strlen(name)+1);
  strcpy(p,name);
  p[strlen(p)-1]=0;
  Sys_Mkdir(p);
}
*/

static int save_global_events() {
	FILE *f;
	char *c;
	size_t stfu;

	f = fopen(Sys_FullPath(SR_TEMP, _GLOBAL_ST), "wb");

	if (f == NULL) {
		return 1;
	}

	stfu = fwrite(GlobEventList, 1, sizeof(GlobEventList), f);
	fclose(f);
	return 0;
}

static int load_global_events() {
	FILE *f;
	char *c;
	size_t stfu;

	memset(GlobEventList, 0, sizeof(GlobEventList));
	f = fopen(Sys_FullPath(SR_TEMP, _GLOBAL_ST), "rb");

	if (f == NULL) {
		return 1;
	}

	stfu = fread(GlobEventList, 1, sizeof(GlobEventList), f);
	fclose(f);
	return 0;
}

int save_game(int slotnum, const char *gamename) {
	char *sn, *ssn, *gn;
	int r;
	WriteFile file;

	SEND_LOG("(SAVELOAD) Saving game slot %d", slotnum, 0);
	save_map_state();
	sn = Sys_FullPath(SR_SAVES, _SLOT_SAV);
	ssn = (char*)alloca(strlen(sn) + 3);
	sprintf(ssn, sn, slotnum);
	gn = (char*)alloca(SAVE_NAME_SIZE);
	strncpy(gn, gamename, SAVE_NAME_SIZE);

	if ((r = save_shops()) != 0) {
		return r;
	}

	if ((r = save_basic_info()) != 0) {
		return r;
	}

	save_leaving_places();
	save_book();
	save_global_events();
	file.open(ssn);

	if (!file.isOpen()) {
		char buff[256];
		sprintf(buff, "Nelze ulozit pozici na cestu: %s", ssn);
		Sys_ErrorBox(buff);
	} else {
		file.write(gn, SAVE_NAME_SIZE);
		close_story_file();
		r = Sys_PackStatus(file);
		open_story_file();
	}

	SEND_LOG("(SAVELOAD) Game saved.... Result %d", r, 0);
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
  Sys_PurgeTemps(0);
//  concat(sn,pathtable[SR_SAVES],_SLOT_SAV);
	sn = Sys_FullPath(SR_SAVES, _SLOT_SAV);
  ssn = (char*)alloca(strlen(sn)+3);
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
  if (get_control_state()) correct_level();
  return r;
  }

//call it in task!
static void load_specific_file(int slot_num, const char *filename, void **out, long *size) {
	FILE *slot;
	char *c, *d;
	long siz;
	char fname[12];
	char succes = 0;
	size_t stfu;

	c = Sys_FullPath(SR_SAVES, _SLOT_SAV);
	d = (char*)alloca(strlen(c) + 2);
	sprintf(d, c, slot_num);
	slot = fopen(d, "rb");

	if (slot == NULL) {
		*out = NULL;
		return;
	}

	fseek(slot, SAVE_NAME_SIZE, SEEK_CUR);
	stfu = fread(fname, 1, 12, slot);

	while(fname[0] && !succes) {
//		Task_Sleep(NULL);
//		if (Task_QuitMsg()) break;
		stfu = fread(&siz, 1, 4, slot);

		if (!strncmp(fname, filename, 12)) {
			succes = 1;
		} else {
			fseek(slot, siz, SEEK_CUR);
			stfu = fread(fname, 1, 12, slot);
		}
	}

	if (succes) {
		*out = getmem(siz);
		stfu = fread(*out, 1, siz, slot);
		*size = siz;
	} else {
		*out = NULL;
	}

	fclose(slot);
}

//------------------------ SAVE LOAD DIALOG ----------------------------
static char force_save;
static StringList slot_list;
static int last_select=-1;
static char used_pos[SLOTS_MAX];
static StringList story_text;
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

void read_slot_list() {
	int i;
	char *mask, *name;
	char slotname[SAVE_NAME_SIZE];
	size_t stfu;

	mask = Sys_FullPath(SR_SAVES, _SLOT_SAV);
	name = (char*)alloca(strlen(mask) + 1);

	for (i = 0; i < SLOTS_MAX; i++) {
		FILE *f;

		sprintf(name, mask, i);
		f = fopen(name, "rb");

		if (f != NULL) {
			stfu = fread(slotname, 1, SAVE_NAME_SIZE, f);
			fclose(f);
			used_pos[i] = 1;
		} else {
			strcpy(slotname, texty[75]);
			used_pos[i] = 0;
		}

		slot_list.replace(i, slotname);
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
  put_picture(0, SCREEN_OFFLINE, (uint16_t*)ablock(H_SAVELOAD));
  put_picture(274, SCREEN_OFFLINE, (uint16_t*)ablock(H_SVITEK));
  set_font(H_FBOLD,NORMAL_COLOR);
  for(i=0;i<SLOTS_MAX;i++) place_name(1,i,0);
  ukaz_mysku();
  }

static void redraw_load()
  {
  int i;
  schovej_mysku();
  put_picture(0, SCREEN_OFFLINE, (uint16_t*)ablock(H_SVITEK));
  put_picture(372, SCREEN_OFFLINE, (uint16_t*)ablock(H_SAVELOAD));
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
     get_picture(x, STORY_Y+SCREEN_OFFLINE, STORY_XS, STORY_YS, (uint16_t*)back_texture);
     }
  else
     put_picture(x, STORY_Y+SCREEN_OFFLINE, (uint16_t*)back_texture);

  y=SCREEN_OFFLINE+STORY_Y;
  ys=STORY_YS;
  count = story_text.size();
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
  ukaz_mysku();
  showview(x,STORY_Y+SCREEN_OFFLINE,STORY_XS,STORY_YS);
  }

//#pragma aux read_story_task parm []
//static void read_story_task(va_list args)
//  {
//  int slot=va_arg(args,int);
static void read_story_task(int slot) {

  void *text_data;
  char *c,*d;
  long size;

  load_specific_file(slot,STORY_BOOK,&text_data,&size);
  story_text.clear();
  if (text_data!=NULL)
     {
     c = (char*)text_data;
     set_font(H_FONT6,0);
     while (size>0)
       {
       int xs,ys;
       d=c;
       while (size && *d!='\r' && *d!='\n') {d++;size--;};
       if (!size) break;
       *d=0;
       {
       char *e,*orig;
       orig = e = (char*)getmem(strlen(c)+2);
       zalamovani(c,e,STORY_XS,&xs,&ys);
       while (*e)
          {
	  story_text.insert(e);
          if (text_width(e)>STORY_XS) abort();
          e=strchr(e,0)+1;
          }
        c=d+1;size--;
       if (*c=='\n' || *c=='\r') {c++;size--;};
       free(orig);
       }
       }
     free(text_data);
     }
  cur_story_pos=get_list_count();if (cur_story_pos<0) cur_story_pos=0;
  redraw_story_bar(cur_story_pos);
  }

static void read_story(int slot)
  {
  static int task_num = -1;

  if (task_num!=-1) Task_Term(task_num);
  if (slot!=-1)
//     task_num=Task_Add(8196,read_story_task,slot);
     read_story_task(slot);
  }


static int get_list_count()
  {
  int count,i,max=0;

  count = story_text.size();
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

static EVENT_PROC(updown_scroll_hold) {
	WHEN_MSG(E_MOUSE) {
		MS_EVENT *ms;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->event_type == 0x4 || !ms->tl1 || ms->tl2 || ms->tl3) {
			send_message(E_DONE, E_MOUSE, updown_scroll_hold);
			send_message(E_DONE, E_TIMER, updown_scroll_hold);
			updown_noinst = 0;
		}
	}

	WHEN_MSG(E_TIMER) {
		MS_EVENT *ms;

		updown_noinst = 1;
		ms = &ms_last_event;
		ms->event_type = 0x2;
		send_message(E_MOUSE, ms);
		if (updown_noinst) {
			send_message(E_DONE, E_MOUSE, updown_scroll_hold);
			send_message(E_DONE, E_TIMER, updown_scroll_hold);
			updown_noinst = 0;
		} else {
			updown_noinst = 1;
		}
	}
}

static char updown_scroll(int id,int xa,int ya,int xr,int yr)
  {
  int count;
  xr,yr,xa,ya;
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
	map_ret = -1;
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
	if (ms_last_event.event_type & 0x2 && id>=0 && used_pos[id]) {
		map_ret = id;
		send_message(E_CLOSE_MAP,id);
	}
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

static char clk_load_proc(int id, int xa, int ya, int xr, int yr) {
	id = bright_slot(yr - 18);

	if (ms_last_event.event_type & 0x2 && id >= 0 && used_pos[id]) {
		if (load_game(id)) {
			message(1, 0, 0, "", texty[79], texty[80]);
			redraw_load();
			showview(0, 0, 0, 0);
			change_click_map(clk_load_error, CLK_LOAD_ERROR);
		} else {
			unwire_proc();
			wire_proc();

			if (battle) {
				konec_kola();
			}

			unwire_proc();

			if (!load_another) {
				wire_main_functs();
				cur_mode = MD_GAME;
				bott_draw(1);
				pick_set_cursor();

				for (id = 0; id < gameMap.coordCount(); id++) {
					gameMap.clearCoordFlags(id, MC_DPLAYER);
				}

				build_player_map();
			}

			reg_grafiku_postav();
			build_all_players();
			cancel_render = 1;
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
  put_picture(x, y, (uint16_t*)ablock(H_LOADTXTR));
  strcpy(global_gamename,slot_list[id]);
//  clk_ask_name[0].id=Task_Add(16384,type_text_v2,global_gamename,x,y,SAVE_SLOT_E-SAVE_SLOT_S,SAVE_NAME_SIZE,H_FBOLD,RGB555(31,31,0),save_it);
  send_message(E_ADD, E_KEYBOARD, type_text,global_gamename,x,y,SAVE_SLOT_E-SAVE_SLOT_S,SAVE_NAME_SIZE,H_FBOLD,RGB555(31,31,0),save_it);
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

static EVENT_PROC(saveload_keyboard) {
	WHEN_MSG(E_KEYBOARD) {
		char c;
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		switch (c) {
		case 1:
			unwire_proc();
			wire_proc();
			break;

		case 'H':
			if (last_select > 0) {
				bright_slot((last_select - 1) * SLOT_SPACE + 1);
			}
			break;

		case 'P':
			if (last_select < SLOTS_MAX - 1) {
				bright_slot((last_select + 1) * SLOT_SPACE + 1);
			}
			break;

		case 28:
			ms_last_event.event_type |= 0x2;
		        if (force_save) {
				clk_save_proc(0, 0, 0, 0, last_select * SLOT_SPACE + 1 + 18);
			} else if (load_mode == 4) {
				clk_load_proc_menu(0, 0, 0, 0, last_select * SLOT_SPACE + 1 + 18);
			} else {
				clk_load_proc(0, 0, 0, 0, last_select * SLOT_SPACE + 1 + 18);
			}
			break;
		}
	}
}

void unwire_save_load()
  {
  if (back_texture!=NULL) free(back_texture);
  back_texture=NULL;
  story_text.clear();
  cancel_pass=0;
  send_message(E_DONE,E_KEYBOARD,saveload_keyboard);
  }

void wire_save_load(char save)
  {
  schovej_mysku();
  mute_all_tracks(0);
  force_save=save & 1;
  load_mode=save;
  if (!slot_list.count()) read_slot_list();
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

//  concat(c,pathtable[SR_TEMP],STORY_BOOK);
	c = Sys_FullPath(SR_TEMP, STORY_BOOK);
  story=fopen(c,"a");
//  err=GetLastError();
  if (story==NULL) story=fopen(c,"w");
  if (story==NULL)
     unable_open_temp(c);
  SEND_LOG("(STORY) Story temp file is opened....",0,0);
  }


void write_story_text(const char *text)
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

int load_map_automap(const char *mapfile) {
	SEND_LOG("(SAVEGAME) CRITICAL SECTION - Swapping maps: %s <-> %s", level_fname, mapfile);
	kill_all_sounds();
	return gameMap.automapRestore(mapfile);
}
//po teto akci se nesmi spustit TM_SCENE!!!! pokud mapfile!=level_fname
