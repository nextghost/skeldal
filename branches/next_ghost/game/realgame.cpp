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
#include <climits>
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
#include "libs/inicfg.h"
#include "libs/system.h"



#define A_OPEN_DOOR 1
#define A_CLOSE_DOOR 2
#define A_OPEN_CLOSE 3
#define A_RUN_PRIM 4
#define A_SHOW_PRIM 5
#define A_HIDE_PRIM 6
#define A_SHOW_HIDE_PRIM 7
#define A_RUN_SEC 8
#define A_SHOW_SEC 9
#define A_HIDE_SEC 10
#define A_SHOW_HIDE_SEC 11
#define A_HIDE_PRIM_SEC 12
#define A_DISPLAY_TEXT 13
#define A_CODELOCK_LOG 14
#define A_OPEN_TELEPORT 15
#define A_CLOSE_TELEPORT 16
#define A_CODELOCK_LOG2 17

#define MAX_FLY_SECT 4

#define DIS (char *)0x1
#define START_HANDLE hl_ptr

static StringList leaving_places;

char pass_zavora=0;

Map gameMap;
StringList level_texts;
int num_ofsets[10]; //tabulka ofsetu pro cisla sten k levelu
char sekceid[]="<BLOCK>";
char datapath;
D_ACTION *d_action={NULL};
int end_ptr;
char cur_group=1;
char group_select=1;
char cancel_pass=0;
char break_sleep=0;
char enable_sort=0;
int  sleep_ticks=MAX_SLEEP;
char side_touched=1;
char save_map=1;
//char formace=0;
char last_send_action;      //naposled vyslana akce
char insleep=0;
short moving_player=0;

char force_start_dialog=0;
int start_dialog_number=0;
int start_dialog_mob=0;

long money=0;

char runes[5]={0,0,0,0,0};

char group_sort[POCET_POSTAV]={0,1,2,3,4,5};

const int Map::_floorPanels[] = {1, 2, 2, 4, 4, 8, 2, 2};

static void loadMob(TMOB &mob, ReadStream &stream) {
	stream.read(mob.name, 30);
	mob.casting = stream.readSint16LE();

	for (int j = 0; j < 6 * 16; j++) {
		mob.adjusting[j] = stream.readSint16LE();
	}

	mob.sector = stream.readUint16LE();
	mob.dir = stream.readUint16LE();
	mob.locx = stream.readUint8();
	mob.locy = stream.readUint8();
	mob.headx = stream.readUint8();
	mob.heady = stream.readUint8();
	mob.anim_counter = stream.readSint16LE();

	for (int j = 0; j < 24; j++) {
		mob.vlastnosti[j] = stream.readSint16LE();
	}

	for (int j = 0; j < MOBS_INV; j++) {
		mob.inv[j] = stream.readSint16LE();
	}

	mob.lives = stream.readSint16LE();
	mob.cislo_vzoru = stream.readSint16LE();
	mob.speed = stream.readSint16LE();
	mob.dohled = stream.readSint16LE();
	mob.dosah = stream.readSint16LE();
	mob.stay_strategy = stream.readSint8();
	mob.walk_data = stream.readUint8();
	mob.bonus = stream.readUint16LE();
	mob.flee_num = stream.readSint8();

	for (int j = 0; j < 6; j++) {
		mob.anim_counts[j] = stream.readSint8();
	}

	stream.read(mob.mobs_name, 7);
	mob.experience = stream.readSint32LE();
	mob.vlajky = stream.readSint8();
	mob.anim_phase = stream.readSint8();
	mob.csektor = stream.readSint16LE();
	mob.home_pos = stream.readSint16LE();
	mob.next = stream.readSint16LE();
	mob.actions = stream.readSint8();
	mob.hit_pos = stream.readSint8();
	mob.sounds[0] = stream.readUint16LE();
	mob.sounds[1] = stream.readUint16LE();
	mob.sounds[2] = stream.readUint16LE();
	mob.sounds[3] = stream.readUint16LE();
	mob.palette = stream.readSint8();
	mob.mode = stream.readSint8();
	mob.dialog = stream.readSint16LE();
	mob.dialog_flags = stream.readSint8();
	mob.money = stream.readUint16LE();
	mob.specproc = stream.readUint16LE();
	mob.dostal = stream.readUint16LE();
	mob.user_data = stream.readUint8();
}

static void loadVyk(TVYKLENEK &vyk, ReadStream &stream) {
	vyk.sector = stream.readSint16LE();
	vyk.dir = stream.readSint16LE();
	vyk.xpos = stream.readSint16LE();
	vyk.ypos = stream.readSint16LE();
	vyk.xs = stream.readSint16LE();
	vyk.ys = stream.readSint16LE();

	for (int i = 0; i < 9; i++) {
		vyk.items[i] = stream.readSint16LE();
	}

	vyk.reserved = stream.readSint16LE();
}

static void loadSide(TSTENA &side, ReadStream &stream) {
	side.prim = stream.readSint8();
	side.sec = stream.readSint8();
	side.oblouk = stream.readSint8();
	side.side_tag = stream.readSint8();
	side.sector_tag = stream.readUint16LE();
	side.xsec = stream.readSint8();
	side.ysec = stream.readSint8();
	side.flags = stream.readUint32LE();
	side.prim_anim = stream.readSint8();
	side.sec_anim = stream.readSint8();
	side.lclip = stream.readSint8();
	side.action = stream.readSint8();
}

static void loadSector(TSECTOR &sector, ReadStream &stream) {
	sector.floor = stream.readSint8();
	sector.ceil = stream.readSint8();
	sector.flags = stream.readSint8();
	sector.sector_type = stream.readSint8();
	sector.action = stream.readSint8();
	sector.side_tag = stream.readSint8();
	sector.step_next[0] = stream.readUint16LE();
	sector.step_next[1] = stream.readUint16LE();
	sector.step_next[2] = stream.readUint16LE();
	sector.step_next[3] = stream.readUint16LE();
	sector.sector_tag = stream.readUint16LE();
}

static void loadCoord(TMAP_EDIT_INFO &coord, ReadStream &stream) {
	coord.x = stream.readSint16LE();
	coord.y = stream.readSint16LE();
	coord.layer = stream.readSint16LE();
	coord.flags = stream.readUint16LE();
}

static void loadMapText(MapText &note, ReadStream &stream) {
	unsigned size = stream.readUint16LE();

	note.x = stream.readSint32LE();
	note.y = stream.readSint32LE();
	note.depth = stream.readSint32LE();
	note.text = new char[size - 12];
	stream.read(note.text, size - 12);
}

long actn_flags(const TSTENA *q,long flags) {
	flags >>= 24;
	flags &= 0x1f;
	return q->flags ^ flags;
}

void check_codelock_log(int sector, unsigned long flags) {
	int i;
	const TSTENA *p;
	const TSECTOR *s;

	p = gameMap.sides() + sector * 4;
	s = gameMap.sectors() + sector;

	for (i = 0; i < 4; i++) {
		if (!(p[i].flags & SD_PRIM_VIS)) {
			break;
		}
	}

	if (i == 4) {
		gameMap.doAction(s->action, s->sector_tag, s->side_tag, flags, 0);
	}
}

Map::Map() : _coordCount(0), _vykCount(0), _sptPtr(0), _fileName(NULL),
	_sides(NULL), _sectors(NULL), _vyk(NULL), _coord(NULL), _flagMap(NULL),
	_items(NULL), _macros(NULL), _mapNotes(NULL), _notesSize(0),
	_notesCount(0) {

	memset(&_glob, 0, sizeof(_glob));
	memset(_spectxtr, 0, sizeof(_spectxtr));
}

Map::~Map() {
	close();
}

#define BLOCK_HEADER_SIZE 8

int Map::partialLoad(const char *filename) {
	char blockHeader[BLOCK_HEADER_SIZE], *path = find_map_path(filename);
	int i, end = 0;
	unsigned type, size;
	File mapFile(path);
	MemoryReadStream *stream;

	free(path);

	if (!mapFile.isOpen()) {
		return -1;
	}

	delete[] _sides;
	delete[] _sectors;
	delete[] _coord;

	do {
		mapFile.read(blockHeader, BLOCK_HEADER_SIZE);

		if (strcmp(blockHeader, "<BLOCK>")) {
			return -1;
		}

		type = mapFile.readUint32LE();
		size = mapFile.readUint32LE();
		mapFile.readUint32LE(); // offset of next block
		stream = mapFile.readStream(size);

		if (stream->size() != size) {
			delete stream;
			return -1;
		}

		switch (type) {
		case A_SIDEMAP:
			size /= 16;
			_sides = new TSTENA[size];

			for (i = 0; i < size; i++) {
				loadSide(_sides[i], *stream);
			}
			break;

		case A_SECTMAP:
			size /= 16;
			_sectors = new TSECTOR[size];

			for (i = 0; i < size; i++) {
				loadSector(_sectors[i], *stream);
			}
			break;

		case A_MAPINFO:
			size /= 8;
			_coordCount = size;
			_coord = new TMAP_EDIT_INFO[size];

			for (i = 0; i < size; i++) {
				loadCoord(_coord[i], *stream);
			}
			break;

		case A_MAPEND:
			end = 1;
			break;

		}

		delete stream;
	} while (!end);

	return 0;
}

int Map::load(const char *filename) {
	char blockHeader[BLOCK_HEADER_SIZE], *path = find_map_path(filename);
	const char *ptr;
	int i, end = 0, snd_load = 0;
	int ofsts = START_HANDLE;
	unsigned type, size, pos, sector, mob_size;
	TMOB *mob_template = NULL;
	File mapFile(path);
	MemoryReadStream *stream;

	close();

	free(path);
	_fileName = new char[strlen(filename) + 1];
	strcpy(_fileName, filename);

	if (!mapFile.isOpen()) {
		return -1;
	}

	do {
		mapFile.read(blockHeader, BLOCK_HEADER_SIZE);

		if (strcmp(blockHeader, "<BLOCK>")) {
			return -3;
		}

		type = mapFile.readUint32LE();
		size = mapFile.readUint32LE();
		mapFile.readUint32LE(); // offset of next block
		stream = mapFile.readStream(size);

		if (stream->size() != size) {
			delete stream;
			return -3;
		}

		switch (type) {
		case A_SIDEMAP:
			size /= 16;
			_sides = new TSTENA[size];

			for (i = 0; i < size; i++) {
				loadSide(_sides[i], *stream);
			}
			break;

		case A_SECTMAP:
			size /= 16;
			_sectors = new TSECTOR[size];

			for (i = 0; i < size; i++) {
				loadSector(_sectors[i], *stream);
			}
			break;

		case A_MAPINFO:
			size /= 8;
			_coordCount = size;
			_coord = new TMAP_EDIT_INFO[size];

			for (i = 0; i < size; i++) {
				loadCoord(_coord[i], *stream);
			}

			_items = new short*[size * 4];
			memset(_items, 0, size * 4 * sizeof(short*));
			init_mobs();
			break;

		case A_MAPEND:
			end = 1;
			break;

		case A_STRMAIN:
			num_ofsets[0] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_fade_decomp, SR_GRAFIKA);
			break;

		case A_STRLEFT:
			num_ofsets[1] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_fade_decomp, SR_GRAFIKA);
			break;

		case A_STRRIGHT:
			num_ofsets[2] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_fade_decomp, SR_GRAFIKA);
			break;

		case A_STRCEIL:
			num_ofsets[3] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_15bit_autofade, SR_GRAFIKA);
			break;

		case A_STRFLOOR:
			num_ofsets[4] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_15bit_autofade, SR_GRAFIKA);
			break;

		case A_STRARC:
			num_ofsets[OBL_NUM] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_fade_decomp, SR_GRAFIKA);
			break;

		case A_STRARC2:
			num_ofsets[OBL2_NUM] = ofsts - 1;
			prepare_graphics(&ofsts, stream, pcx_fade_decomp, SR_GRAFIKA);
			break;

		case A_MAPGLOB:
			num_ofsets[BACK_NUM] = ofsts;
			memset(&_glob, 0, sizeof(MAPGLOBAL));
			stream->read(_glob.back_fnames[0], 13);
			stream->read(_glob.back_fnames[1], 13);
			stream->read(_glob.back_fnames[2], 13);
			stream->read(_glob.back_fnames[3], 13);
			_glob.fade_r = stream->readSint32LE();
			_glob.fade_g = stream->readSint32LE();
			_glob.fade_b = stream->readSint32LE();
			_glob.start_sector = stream->readSint32LE();
			_glob.direction = stream->readSint32LE();
			stream->read(_glob.mapname, 30);
			_glob.map_effector = stream->readSint8();
			_glob.local_monsters = stream->readSint8();

			assert(!stream->eos());

			_glob.map_autofadefc = stream->readSint8();

			if (stream->eos()) {
				_glob.map_autofadefc = 0;
			}

			for (i = 0; i < 4; i++) {
				def_handle(ofsts++, _glob.back_fnames[i], pcx_fade_decomp, SR_GRAFIKA);
			}

			back_color = RGB888(_glob.fade_r, _glob.fade_g, _glob.fade_b);
			break;

		case A_MAPITEM:
			SEND_LOG("(GAME) Loading items...", 0, 0);

			for (sector = stream->readUint32LE(); !stream->eos(); sector = stream->readUint32LE()) {
				pos = stream->pos();
				for (i = 1; stream->readSint16LE(); i++);
				stream->seek(pos, SEEK_SET);
				_items[sector] = new short[i];
				for (i = 0; _items[sector][i] = stream->readSint16LE(); i++);
			}
			break;

		case A_MAPMOBS:
			if (!snd_load) {
				create_sound_table_old();
			}

			SEND_LOG("(GAME) Loading enemies...", 0, 0);

			if (!mob_template) {
				unsigned char *block = (unsigned char*)ablock(H_ENEMY);
				TMOB *templ;
				MemoryReadStream enemies(block + 8, get_handle_size(H_ENEMY) - 8);

				templ = new TMOB[size = enemies.size() / 376];

				for (i = 0; i < size; i++) {
					loadMob(templ[i], enemies);
				}

				load_enemies(stream, &ofsts, templ, size);
				delete[] templ;
			} else {
				load_enemies(stream, &ofsts, mob_template, mob_size);
				delete[] mob_template;
				SEND_LOG("(GAME) Loading enemies from map template...", 0, 0);
			}
			break;

		case A_MAPMACR:
			SEND_LOG("(GAME) Loading multiactions...", 0, 0);
			_macros = new unsigned char*[_coordCount * 4];
			memset(_macros, 0, _coordCount * 4 * sizeof(unsigned char*));

			while (sector = stream->readUint32LE()) {
				pos = stream->pos();
				size = 0;

				do {
					i = stream->readUint32LE();
					stream->seek(i, SEEK_CUR);
					size += 4 + i;
				} while (i);

				stream->seek(pos, SEEK_SET);
				_macros[sector] = new unsigned char[size];
				stream->read(_macros[sector], size);
			}

			load_macros();
			break;

		case A_MAPVYK:
			size /= 32;
			_vykCount = size;
			_vyk = new TVYKLENEK[size];

			for (i = 0; i < size; i++) {
				loadVyk(_vyk[i], *stream);
			}
			break;

		case A_MOBS:
			size /= 376;
			mob_template = new TMOB[size];
			mob_size = size;

			for (i = 0; i < size; i++) {
				loadMob(mob_template[i], *stream);
			}

			break;

		case A_MOBSND:
			snd_load = 1;

			for (ptr = stream->readCString(), i = 0; !stream->eos(); ptr = stream->readCString(), i++) {
				if (*ptr) {
					sound_table.replace(i, ptr);
				}
			}
			break;
		}

		delete stream;
	} while (!end);

	end_ptr = ofsts;
	_flagMap = new uint8_t[_coordCount * 4];

	for (i = 0; i < _coordCount * 4; i++) {
		_flagMap[i] = _sides[i].flags & 0xff;
	}

	return 0;
}

void Map::close(void) {
	int i;

	for (i = 0; i < _coordCount * 4; i++) {
		if (_items) {
			delete[] _items[i];
		}

		if (_macros) {
			delete[] _macros[i];
		}
	}

	for (i = 0; i < _notesSize; i++) {
		delete[] _mapNotes[i].text;
	}

	delete[] _fileName;
	delete[] _sides;
	delete[] _sectors;
	delete[] _vyk;
	delete[] _coord;
	delete[] _flagMap;
	delete[] _items;
	delete[] _macros;
	delete[] _mapNotes;

	_fileName = NULL;
	_sides = NULL;
	_sectors = NULL;
	_vyk = NULL;
	_coord = NULL;
	_flagMap = NULL;
	_items = NULL;
	_macros = NULL;
	_mapNotes = NULL;

	_coordCount = 0;
	_vykCount = 0;
	_sptPtr = 0;
	_notesSize = 0;
	_notesCount = 0;
}

void Map::addSpecTexture(uint16_t sector, uint16_t fhandle, uint16_t count, uint16_t repeat, int16_t xpos) {
	SPECTXTR *sp = _spectxtr + _sptPtr;

	sp->sector = sector;
	sp->handle = fhandle;
	sp->count = count;
	sp->pos = 0;
	sp->repeat = repeat;
	sp->xpos = xpos;

	if (count && ++_sptPtr >= MAX_SPECTXTRS) {
		_sptPtr = 0;
	}

	_coord[sector].flags |= MC_SPECTXTR;
}

void Map::recalcSpecTextures(void) {
	int i;

	for (i = 0; i < MAX_SPECTXTRS; i++) {
		if (_spectxtr[i].repeat) {
			SPECTXTR *sp = _spectxtr + i;

			if (++sp->pos >= sp->count) {
				sp->pos = 0;

				if (sp->repeat > 0) {
					sp->repeat--;
				}
			}
		}
	}
}

// FIXME: rewrite?
extern long sound_side_flags;

void Map::calcAnimations(void) {
	int i;
	
	for (i = 0; i < _coordCount * 4; i++) {
		TSTENA *p;
		int pj, pk, sj, sk;
		
		p = _sides + i;
		sound_side_flags = p->flags;
		pj = p->prim_anim >> 4;
		pk = p->prim_anim & 15;
		sj = p->sec_anim >> 4;
		sk = p->sec_anim & 15;

		if (!pk && !sk) {
			continue;
		}

		if (p->flags & SD_PRIM_ANIM) {
			if (p->flags & SD_PRIM_GAB) {
				if (pj == 0 || pj == pk)
					p->flags ^= SD_PRIM_FORV;
			}

			if (p->flags & SD_PRIM_FORV) {
				pj++;
			} else {
				pj--;
			}

			if (pj > pk) {
				pj = 0;
			}

			if (pj < 0) {
				pj = pk;
			}
		} else {
			if (p->flags & SD_PRIM_FORV) {
				pj++;
			} else {
				pj--;
			}

			if (pk && (pj + (!(p->flags & SD_PRIM_FORV)) == pk)) {
				p->flags &= ~0xff;
				p->flags |= _flagMap[i];
			}

			if (pj > pk) {
				pj = pk;
			} else if (pj < 0) {
				pj = 0;
			} else {
				call_macro(i, MC_ANIM | ((pj & 1) ? MC_ANIM2 : 0) | (pj ? 0 : MC_CLOSEDOOR) | (pj == pk ? MC_OPENDOOR : 0));
			}

			if (pk == pj && p->flags & SD_PRIM_GAB) {
				p->flags &= ~SD_PRIM_FORV;
			}
		}

		if (p->flags & SD_SEC_ANIM) {
			if (p->flags & SD_SEC_GAB) {
				if (sj == 0 || sj == sk) p->flags ^= SD_SEC_FORV;
			}

			if (p->flags & SD_SEC_FORV) {
				sj++;
			} else {
				sj--;
			}

			if (sj > sk) {
				sj = 0;
			}

			if (sj < 0) {
				sj = sk;
			}
		} else {
			if (p->flags & SD_SEC_FORV) {
				sj++;
			} else {
				sj--;
			}

			if (!pk && sk && (sj + (!(p->flags & SD_SEC_FORV)) == sk)) {
				p->flags &= ~0xff;
				p->flags |= _flagMap[i];
			}

			if (sj > sk) {
				sj = sk;
			} else if (sj < 0) {
				sj = 0;
			} else if (!pk) {
				call_macro(i, MC_ANIM | ((sj & 1) ? MC_ANIM2 : 0) | (pj ? 0 : MC_CLOSEDOOR));
			}

			if (sk == sj && p->flags & SD_SEC_GAB) {
				p->flags &= ~SD_SEC_FORV;
			}
		}
		p->prim_anim = (pj << 4) + pk;
		p->sec_anim= (sj << 4) + sk;
	}

	sound_side_flags = 0;
}

void Map::resetSecAnim(unsigned side) {
	assert(side < _coordCount * 4);
	_sides[side].flags &= ~SD_SEC_VIS;
	_sides[side].sec_anim = 0;
	_sides[side].sec = 0;
}

void Map::setSecAnim(unsigned sector, unsigned templ) {
	assert(sector < _coordCount && templ < _coordCount);

	for (int i = 0; i < 4; i++) {
		_sides[sector * 4 + i].flags |= SD_SEC_VIS | SD_SEC_ANIM;
		_sides[sector * 4 + i].sec_anim = _sides[templ * 4].sec_anim;
		_sides[sector * 4 + i].sec = _sides[templ * 4].sec;
	}
}

void Map::clearTag(unsigned sector) {
	assert(sector < _coordCount);
	_sectors[sector].sector_tag = 0;
}

void Map::tag(unsigned s1, unsigned s2, int sideTag) {
	assert(s1 < _coordCount && s2 < _coordCount);
	_sectors[s1].sector_tag = s2;
	_sectors[s1].side_tag = _sectors[s2].side_tag;
	_sectors[s2].sector_tag = s1;
	_sectors[s2].side_tag = sideTag;
}

void Map::setCoordFlags(unsigned coord, uint16_t flags) {
	assert(coord < _coordCount);
	_coord[coord].flags |= flags;
}

void Map::clearCoordFlags(unsigned coord, uint16_t flags) {
	assert(coord < _coordCount);
	_coord[coord].flags &= ~flags;
}

void Map::setSideFlags(unsigned side, uint32_t flags) {
	assert(side < _coordCount * 4);
	_sides[side].flags |= flags;
}

void Map::clearSideFlags(unsigned side, uint32_t flags) {
	assert(side < _coordCount * 4);
	_sides[side].flags &= ~flags;
}

void Map::clearTeleport(unsigned sector) {
	assert(sector < _coordCount && _sectors[sector].sector_type >= S_USERTELEPORT);
	_sectors[sector].sector_type -= S_USERTELEPORT;

}

void Map::setTeleport(unsigned sector) {
	assert(sector < _coordCount && _sectors[sector].sector_type < S_USERTELEPORT);
	_sectors[sector].sector_type += S_USERTELEPORT;

}

short Map::removeItem(unsigned vyk) {
	assert(vyk < _vykCount && _vyk[vyk].items[0]);

	int i;
	short ret;

	for (i = 1; _vyk[vyk].items[i]; i++);
	ret = _vyk[vyk].items[--i];
	_vyk[vyk].items[i] = 0;
	return ret;
}

void Map::putItem(unsigned vyk, short item) {
	assert(vyk < _vykCount);

	int i;

	for (i = 0; _vyk[vyk].items[i]; i++);
	assert(i < 8);
	_vyk[vyk].items[i++] = item;
	_vyk[vyk].items[i] = 0;
}

void Map::pushItem(unsigned sector, short *items) {
	assert(items && sector < _coordCount * 4);

	int sCount, pCount;
	short *ptr;

	sCount = count_items_total(_items[sector]);
	pCount = count_items_total(items);

	if (sCount + pCount == 0) {
		return;
	}

	ptr = new short[sCount + pCount + 1];
	memcpy(ptr, _items[sector], sCount * sizeof(short));
	memcpy(ptr + sCount, items, pCount * sizeof(short));
	ptr[pCount + sCount] = 0;
	delete[] _items[sector];
	_items[sector] = ptr;
}

short *Map::popItem(unsigned sector, int picked) {
	assert(sector < _coordCount * 4);

	int sCount, pCount;
	short *ret;

	sCount = count_items_total(_items[sector]);
	assert(picked < sCount);
	pCount = count_items_inside(_items[sector] + picked);
	// FIXME: replace with new/delete
	ret = (short*)malloc((pCount + 1) * sizeof(short));
	memcpy(ret, _items[sector] + picked, pCount * sizeof(short));
	ret[pCount] = 0;
	memmove(_items[sector] + picked, _items + picked + pCount, (sCount - picked - pCount) * sizeof(short));
	_items[sector][sCount - pCount] = 0;
	return ret;
}

// FIXME: get rid of temps
void expand_map_file_name(char *s);

void save_daction(WriteStream &stream, int count, D_ACTION *ptr);

// FIXME: add write error checks
int Map::save(void) const {
	char buf[PATH_MAX];
	unsigned size, i, j;
	uint8_t byte = 0;
	LETICI_VEC *f;
	WriteFile file;
	Map hack;

	strcpy(buf, _fileName);
	expand_map_file_name(buf);
	file.open(buf);

	if (!file.isOpen()) {
		return -1;
	}

	SEND_LOG("(SAVELOAD) Saving map state for current map", 0, 0);

	if (hack.partialLoad(_fileName)) {
		return -1;
	}

	file.writeSint32LE(0);
	file.writeSint32LE(STATE_CUR_VER);	// game version
	file.writeSint32LE(_coordCount);	// map size
	file.writeUint32LE((_coordCount + 7) / 8);	// automap bitmap size

	// save automap bitmap
	for (i = 0; i < _coordCount; i++) {
		if (_coord[i].flags & MC_AUTOMAP) {
			byte |= 1 << (i & 0x7);
		}

		if ((i & 0x7) == 0x7) {
			file.writeUint8(byte);
			byte = 0;
		}
	}

	if (i & 0x7) {
		file.writeUint8(byte);
		byte = 0;
	}

	// save disclosed bitmap
	for (i = 0; i < _coordCount; i++) {
		if (_coord[i].flags & MC_DISCLOSED) {
			byte |= 1 << (i & 0x7);
		}

		if ((i & 0x7) == 0x7) {
			file.writeUint8(byte);
			byte = 0;
		}
	}

	if (i & 0x7) {
		file.writeUint8(byte);
	}

	// save automap notes
	file.writeSint32LE(_notesCount);

	for (i = 0; i < _notesSize; i++) {
		if (_mapNotes[i].text) {
			size = strlen(_mapNotes[i].text) + 1;
			file.writeUint16LE(size + 12);
			file.writeSint32LE(_mapNotes[i].x);
			file.writeSint32LE(_mapNotes[i].y);
			file.writeSint32LE(_mapNotes[i].depth);
			file.write(_mapNotes[i].text, size);
		}
	}

	// save changed sides
	for (i = 0; i < _coordCount * 4; i++) {
		if (memcmp(_sides + i, hack._sides + i, sizeof(TSTENA))) {
			file.writeUint16LE(i);	// side index
			// side data
			file.writeSint8(_sides[i].prim);
			file.writeSint8(_sides[i].sec);
			file.writeSint8(_sides[i].oblouk);
			file.writeSint8(_sides[i].side_tag);
			file.writeUint16LE(_sides[i].sector_tag);
			file.writeSint8(_sides[i].xsec);
			file.writeSint8(_sides[i].ysec);
			file.writeUint32LE(_sides[i].flags);
			file.writeSint8(_sides[i].prim_anim);
			file.writeSint8(_sides[i].sec_anim);
			file.writeSint8(_sides[i].lclip);
			file.writeSint8(_sides[i].action);
		}
	}

	file.writeUint16LE(0xffff);	// end of side list

	// save changed sectors
	for (i = 0; i < _coordCount; i++) {
		if (memcmp(_sectors + i, hack._sectors + i, sizeof(TSECTOR))) {
			file.writeUint16LE(i);	// sector index
			// sector data
			file.writeSint8(_sectors[i].floor);
			file.writeSint8(_sectors[i].ceil);
			file.writeSint8(_sectors[i].flags);
			file.writeSint8(_sectors[i].sector_type);
			file.writeSint8(_sectors[i].action);
			file.writeSint8(_sectors[i].side_tag);
			file.writeUint16LE(_sectors[i].step_next[0]);
			file.writeUint16LE(_sectors[i].step_next[1]);
			file.writeUint16LE(_sectors[i].step_next[2]);
			file.writeUint16LE(_sectors[i].step_next[3]);
			file.writeUint16LE(_sectors[i].sector_tag);
		}
	}

	file.writeUint16LE(0xffff);	// end of sector list

	// save mobs
	for (i = 0; i < MAX_MOBS; i++) {
		if (mobs[i].vlajky & MOB_LIVE) {
			file.writeUint16LE(i);	// mob index
			// mob data
			file.write(mobs[i].name, 30);
			file.writeSint16LE(mobs[i].casting);

			for (j = 0; j < 6 * 16; j++) {
				file.writeSint16LE(mobs[i].adjusting[j]);
			}

			file.writeUint16LE(mobs[i].sector);
			file.writeUint16LE(mobs[i].dir);
			file.writeUint8(mobs[i].locx);
			file.writeUint8(mobs[i].locy);
			file.writeUint8(mobs[i].headx);
			file.writeUint8(mobs[i].heady);
			file.writeSint16LE(mobs[i].anim_counter);

			for (j = 0; j < 24; j++) {
				file.writeSint16LE(mobs[i].vlastnosti[j]);
			}

			for (j = 0; j < MOBS_INV; j++) {
				file.writeSint16LE(mobs[i].inv[j]);
			}

			file.writeSint16LE(mobs[i].lives);
			file.writeSint16LE(mobs[i].cislo_vzoru);
			file.writeSint16LE(mobs[i].speed);
			file.writeSint16LE(mobs[i].dohled);
			file.writeSint16LE(mobs[i].dosah);
			file.writeSint8(mobs[i].stay_strategy);
			file.writeUint8(mobs[i].walk_data);
			file.writeUint16LE(mobs[i].bonus);
			file.writeSint8(mobs[i].flee_num);

			for (j = 0; j < 6; j++) {
				file.writeSint8(mobs[i].anim_counts[j]);
			}

			file.write(mobs[i].mobs_name, 7);
			file.writeSint32LE(mobs[i].experience);
			file.writeSint8(mobs[i].vlajky);
			file.writeSint8(mobs[i].anim_phase);
			file.writeSint16LE(mobs[i].csektor);
			file.writeSint16LE(mobs[i].home_pos);
			file.writeSint16LE(mobs[i].next);
			file.writeSint8(mobs[i].actions);
			file.writeSint8(mobs[i].hit_pos);
			file.writeUint16LE(mobs[i].sounds[0]);
			file.writeUint16LE(mobs[i].sounds[1]);
			file.writeUint16LE(mobs[i].sounds[2]);
			file.writeUint16LE(mobs[i].sounds[3]);
			file.writeSint8(mobs[i].palette);
			file.writeSint8(mobs[i].mode);
			file.writeSint16LE(mobs[i].dialog);
			file.writeSint8(mobs[i].dialog_flags);
			file.writeUint16LE(mobs[i].money);
			file.writeUint16LE(mobs[i].specproc);
			file.writeUint16LE(mobs[i].dostal);
			file.writeUint8(mobs[i].user_data);
		}
	}

	file.writeUint16LE(0xffff);	// end of mobs
	file.writeUint32LE(_coordCount * 4);	// flag map size

	// save flag map
	for (i = 0; i < _coordCount * 4; i++) {
		file.writeUint8(_flagMap[i]);
	}

	save_daction(file, 0, d_action);
	file.writeUint32LE(0);	// macro_block_size, never used

	if (save_codelocks(file)) {
		return -1;
	}

	// save items
	for (i = 0; i < _coordCount * 4; i++) {
		if (!_items[i]) {
			continue;
		}

		for (size = 0; _items[i][size++];);
		file.writeUint16LE(i);	// side index
		file.writeSint16LE(size);	// item count

		for (j = 0; j < size; j++) {
			file.writeSint16LE(_items[i][j]);
		}
	}

	file.writeUint16LE(0xffff);	// end of item list
	file.writeUint16LE(_vykCount);	// alcove count

	// save alcove list
	for (i = 0; i < _vykCount; i++) {
		file.writeSint16LE(_vyk[i].sector);
		file.writeSint16LE(_vyk[i].dir);
		file.writeSint16LE(_vyk[i].xpos);
		file.writeSint16LE(_vyk[i].ypos);
		file.writeSint16LE(_vyk[i].xs);
		file.writeSint16LE(_vyk[i].ys);

		for (j = 0; j < 9; j++) {
			file.writeSint16LE(_vyk[i].items[j]);
		}

		file.writeSint16LE(_vyk[i].reserved);
	}

	file.writeUint32LE(letici_veci != NULL);

	for (f = letici_veci; f; f = f->next) {
		file.writeUint32LE(f->next != NULL);
		file.writeSint32LE(f->sector);
		file.writeSint32LE(f->smer);
		file.writeSint32LE(f->xpos);
		file.writeSint32LE(f->ypos);
		file.writeSint32LE(f->zpos);
		file.writeSint16LE(f->item);
		file.writeUint32LE(f->items != NULL);
		file.writeSint32LE(f->counter);
		file.writeSint8(f->anim_pos);
		file.writeUint8(f->flags);
		file.writeSint16LE(f->owner);
		file.writeSint32LE(f->speed);
		file.writeSint32LE(f->velocity);
		file.writeSint16LE(f->hit_bonus);
		file.writeSint16LE(f->damage);
		file.writeSint16LE(f->lives);

		for (j = 0; f->items[j]; j++) {
			file.writeSint16LE(f->items[j]);
		}

		file.writeSint16LE(0);
	}

	save_enemy_paths(file);
	return 0;
}

// FIXME: verify and delete
extern char reset_mobiles;

// FIXME: rewrite MOB structure to class
void free_path(int mob);
void register_mob_path(int mob, uint16_t *path);

void load_daction(ReadStream &stream);

int Map::restore(void) {
	char buf[PATH_MAX];
	int i;
	unsigned size;
	uint8_t tmp;
	File file;

	strcpy(buf, _fileName);
	expand_map_file_name(buf);
	file.open(buf);

	if (!file.isOpen()) {
		return -1;
	}

	i = file.readSint32LE();

	if (!i) {
		i = file.readSint32LE();	// game version

		if (file.eos() || i > STATE_CUR_VER) {
			return -2;
		}

		i = file.readSint32LE();	// map size

		if (file.eos() || i != _coordCount) {
			return -2;
		}

		SEND_LOG("(SAVELOAD) Loading map state for current map", 0, 0);
		file.readUint32LE();	// size of automap bitmap, ignore

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_AUTOMAP;
			}
		}

		if (file.eos()) {
			return -2;
		}

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_DISCLOSED;
			}
		}

		if (file.eos()) {
			return -2;
		}
	} else if (i != _coordCount) {
		return 0;
	} else {
		SEND_LOG("(SAVELOAD) Loading map state for current map", 0, 0);
		i = file.readUint32LE();	// size of automap bitmap, ignore

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_AUTOMAP;
			}
		}

		if (file.eos()) {
			return -2;
		}
	}

	// load automap notes
	for (i = 0; i < _notesSize; i++) {
		delete[] _mapNotes[i].text;
	}

	delete[] _mapNotes;
	_notesCount = _notesSize = file.readSint32LE();
	_mapNotes = new MapText[_notesSize];

	for (i = 0; i < _notesCount; i++) {
		loadMapText(_mapNotes[i], file);
	}

	for (i = file.readUint16LE(); !file.eos() && i < _coordCount * 4; i = file.readUint16LE()) {
		loadSide(_sides[i], file);

		if (file.eos()) {
			return -2;
		}
	}

	for (i = file.readUint16LE(); !file.eos() && i < _coordCount; i = file.readUint16LE()) {
		loadSector(_sectors[i], file);

		if (file.eos()) {
			return -2;
		}
	}

	// reset_mobiles is never set, therefore I'm not adding reset code here
	assert(!reset_mobiles);

	for (i = 0; i < MAX_MOBS; i++) {
		mobs[i].vlajky &= ~MOB_LIVE;
	}

	for (i = file.readUint16LE(); !file.eos() && i < MAX_MOBS; i = file.readUint16LE()) {
		loadMob(mobs[i], file);

		if (file.eos()) {
			return -2;
		}
	}

	for (i = 0; i < MAX_MOBS; i++) {
		mobs[i].vlajky &= ~MOB_IN_BATTLE;
	}

	refresh_mob_map();
	size = file.readUint32LE();

	for (i = 0; i < size; i++) {
		_flagMap[i] = file.readUint8();
	}

	load_daction(file);
	i = file.readUint32LE();
	file.seek(i, SEEK_CUR);	// macro_block is never actually used

	if (load_codelocks(file)) {
		return -2;
	}

	for (i = 0; i < _coordCount * 4; i++) {
		if (_items[i]) {
			delete[] _items[i];
			_items[i] = NULL;
		}
	}

	for (i = file.readUint16LE(); !file.eos() && i != 0xffff; i = file.readUint16LE()) {
		size = file.readSint16LE();
		_items[i] = new short[size];

		for (int j = 0; j < size; j++) {
			_items[i][j] = file.readSint16LE();
		}
	}

	size = file.readUint16LE();

	if (size > _vykCount) {
		return -2;
	}

	for (i = 0; i < size; i++) {
		loadVyk(_vyk[i], file);
	}

	LETICI_VEC *n, *p = letici_veci;

	for (i = file.readUint32LE(); !file.eos() && i;) {
		short items[100] = {0}, *ptr;
		int present;

		// FIXME: replace with new/delete
		n = (LETICI_VEC*)malloc(sizeof(LETICI_VEC));
		ptr = items;
		i = file.readUint32LE();	// n->next
		n->next = NULL;
		n->sector = file.readSint32LE();
		n->smer = file.readSint32LE();
		n->xpos = file.readSint32LE();
		n->ypos = file.readSint32LE();
		n->zpos = file.readSint32LE();
		n->item = file.readSint16LE();
		present = file.readUint32LE();	// n->items
		n->counter = file.readSint32LE();
		n->anim_pos = file.readSint8();
		n->flags = file.readUint8();
		n->owner = file.readSint16LE();
		n->speed = file.readSint32LE();
		n->velocity = file.readSint32LE();
		n->hit_bonus = file.readSint16LE();
		n->damage = file.readSint16LE();
		n->lives = file.readSint16LE();

		if (file.eos()) {
			free(n);

			return -2;
		}

		if (present) {
			for (ptr = items; *ptr++ = file.readSint16LE(););
			n->items = (short*)malloc((ptr - items) * sizeof(short));
			memcpy(n->items, items, (ptr - items) * sizeof(short));
		}

		if (!p) {
			p = letici_veci = n;
		} else {
			p->next = n;
		}

		p = n;
	}

	for (i = 0; i < MAX_MOBS; i++) {
		free_path(i);
	}

	for (i = file.readUint16LE(); !file.eos() && i != 0xffff; i = file.readUint16LE()) {
		uint16_t *ptr;

		size = file.readSint32LE();
		// FIXME: replace with new/delete
		ptr = (uint16_t*)malloc(size * sizeof(uint16_t));

		for (int j = 0; j < size; j++) {
			ptr[j] = file.readUint16LE();
		}

		register_mob_path(i, ptr);
	}

	if (file.eos()) {
		return 1;
	}

	return 0;
}

void Map::quickRestore(void) {
	int i;

	partialLoad(_fileName);
	restore();

	for (i = 0; i < _coordCount * 4; i++) {
		call_macro(i, MC_STARTLEV);
	}
}

// FIXME: replace this method with partial load in another instance of Map
int Map::automapRestore(const char *filename) {
	char buf[PATH_MAX];
	int i;
	unsigned size;
	uint8_t tmp;
	File file;

	partialLoad(filename);

	strcpy(buf, filename);
	expand_map_file_name(buf);
	file.open(buf);

	if (!file.isOpen()) {
		return -1;
	}

	i = file.readSint32LE();

	SEND_LOG("(SAVELOAD) Partial restore for map: %s (%s)", level_fname, "START");

	if (!i) {
		i = file.readSint32LE();	// game version

		if (file.eos() || i > STATE_CUR_VER) {
			return -2;
		}

		i = file.readSint32LE();	// map size

		if (file.eos() || i != _coordCount) {
			return -2;
		}

		file.readUint32LE();	// size of automap bitmap, ignore

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_AUTOMAP;
			}
		}

		if (file.eos()) {
			return -2;
		}

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_DISCLOSED;
			}
		}

		if (file.eos()) {
			return -2;
		}
	} else if (i != _coordCount) {
		return -2;
	} else {
		i = file.readUint32LE();	// size of automap bitmap, ignore

		for (i = 0; i < _coordCount; i++) {
			if ((i & 0x7) == 0) {
				tmp = file.readUint8();
			}

			if ((tmp >> (i & 0x7)) & 0x1) {
				_coord[i].flags |= MC_AUTOMAP;
			}
		}

		if (file.eos()) {
			return -2;
		}
	}

	// load automap notes
	for (i = 0; i < _notesSize; i++) {
		delete[] _mapNotes[i].text;
	}

	delete[] _mapNotes;
	_notesCount = _notesSize = file.readSint32LE();
	_mapNotes = new MapText[_notesSize];

	for (i = 0; i < _notesCount; i++) {
		loadMapText(_mapNotes[i], file);
	}

	for (i = file.readUint16LE(); !file.eos() && i != 0xffff; i = file.readUint16LE()) {
		loadSide(_sides[i], file);

		if (file.eos()) {
			return -2;
		}
	}

	for (i = file.readUint16LE(); !file.eos() && i != 0xffff; i = file.readUint16LE()) {
		loadSector(_sectors[i], file);

		if (file.eos()) {
			return -2;
		}
	}

	return 0;
}

void Map::swapSectors(unsigned s1, unsigned s2) {
	TSECTOR tmp1;
	TSTENA tmp2;
	int i;

	for (i = 0; i < 4; i++) {
		tmp2 = _sides[4 * s1 + i];
		_sides[4 * s1 + i] = _sides[4 * s2 + i];
		_sides[4 * s2 + i] = tmp2;
	}

	tmp1 = _sectors[s1];
	_sectors[s1] = _sectors[s2];
	_sectors[s2] = tmp1;
}

void Map::buttonActivate(unsigned sector) {
	assert(sector < _coordCount);
	_sectors[sector].floor += _floorPanels[_sectors[sector].flags & 7];
	_sectors[sector].sector_type = S_TLAC_ON;
}

void Map::buttonDeactivate(unsigned sector) {
	assert(sector < _coordCount);
	_sectors[sector].floor -= _floorPanels[_sectors[sector].flags & 7];
	_sectors[sector].sector_type = S_TLAC_OFF;
}

int Map::doAction(int action, unsigned sector, int dir, int flags, int nosend) {
	TSTENA *q;
	TSECTOR *s;
	uint8_t *c;
	int sid;
	char ok = 0;

	sid = sector * 4 + dir;
	s = _sectors + sector;
	q = _sides + sid;
	c = _flagMap + sid;

	switch (action) {
	case 0:
		q->flags = actn_flags(q, flags);
		break;

	case A_OPEN_DOOR:
		if (!(q->flags & SD_PRIM_FORV) && !(q->flags & SD_PRIM_ANIM)) {
			q->flags |= SD_PRIM_FORV;
			*c = (uint8_t)actn_flags(q, flags);
			ok = 1;
		}

		if (!(q->flags & SD_SEC_FORV)) {
			q->flags |= SD_SEC_FORV;
			ok = 1;
		}

		sound_side_flags = q->flags;
		break;

	case A_CLOSE_DOOR:
		if (q->flags & SD_PRIM_FORV && !(q->flags & SD_PRIM_ANIM)) {
			q->flags &= ~SD_PRIM_FORV;
			*c = (uint8_t)actn_flags(q, flags);
			ok = 1;
		}

		if (q->flags & SD_SEC_FORV) {
			q->flags &= ~SD_SEC_FORV;
			ok = 1;
		}

		sound_side_flags = q->flags;
		break;

	case A_OPEN_CLOSE:
		if (!(q->flags & SD_PRIM_ANIM)) {
			q->flags ^= SD_PRIM_FORV | SD_SEC_FORV;
		} else {
			q->flags ^= SD_SEC_FORV;
		}

		*c = (uint8_t)actn_flags(q, flags);
		ok = 1;
		sound_side_flags = q->flags;
		break;

	case A_RUN_PRIM:
		q->flags |= SD_PRIM_ANIM;
		ok = 1;
		break;

	case A_HIDE_PRIM:
		if (q->flags & SD_PRIM_VIS) {
			q->flags &= ~SD_PRIM_VIS;
			*c = (q->flags = actn_flags(q, flags));
			ok = 1;
		}
		break;

	case A_SHOW_PRIM:
		if (!(q->flags & SD_PRIM_VIS)) {
			q->flags |= SD_PRIM_VIS;
			*c = (q->flags = actn_flags(q, flags));
			ok = 1;
		}
		break;

	case A_SHOW_HIDE_PRIM:
		q->flags ^= SD_PRIM_VIS;
		*c = (q->flags = actn_flags(q, flags));
		ok = 1;
		break;

	case A_RUN_SEC:
		q->flags |= SD_SEC_ANIM;
		ok = 1;
		break;

	case A_HIDE_SEC:
		if (q->flags & SD_SEC_VIS) {
			q->flags &= ~SD_SEC_VIS;
			*c = (q->flags = actn_flags(q, flags));
			ok = 1;
		}
		break;

	case A_SHOW_SEC:
		if (!(q->flags & SD_SEC_VIS)) {
			q->flags |= SD_SEC_VIS;
			*c = (q->flags = actn_flags(q, flags));
			ok = 1;
		}
		break;

	case A_SHOW_HIDE_SEC:
		q->flags ^= SD_SEC_VIS;
		*c = (q->flags = actn_flags(q, flags));
		ok = 1;
		break;

	case A_HIDE_PRIM_SEC:
		if ((q->flags & SD_SEC_VIS)||(q->flags & SD_PRIM_VIS)) {
			q->flags &= ~(SD_SEC_VIS + SD_PRIM_VIS);
			*c = (q->flags = actn_flags(q, flags));
			ok = 1;
		}
		break;

	case A_DISPLAY_TEXT:
		bott_disp_text(level_texts[sector]);
		ok = 1;
		return 0;

	case A_CODELOCK_LOG2:
		check_codelock_log(sector, flags);
		q->flags ^= SD_PRIM_VIS;
		check_codelock_log(sector, flags);
		ok = 1;
		break;

	case A_CODELOCK_LOG:
		check_codelock_log(sector, 0x1f000000);
		q->flags ^= SD_PRIM_VIS;
		check_codelock_log(sector, 0x1f000000);
		ok = 1;
		break;

	case A_OPEN_TELEPORT:
		ok = (s->sector_type != S_TELEPORT);
		s->sector_type = S_TELEPORT;

		for (int i = 0; i < 4; i++) {
			_sides[(sector << 2) + i].flags |= SD_SEC_VIS;
		}

		nosend = 1;
		break;

	case A_CLOSE_TELEPORT:
		ok = (s->sector_type == S_TELEPORT);
		s->sector_type = S_NORMAL;

		for (int i = 0; i < 4; i++) {
			_sides[(sector << 2) + i].flags &= ~SD_SEC_VIS;
		}

		nosend = 1;
		break;
	}

	if (nosend) {
		return 0;
	}

	last_send_action = action;
	call_macro(sid, MC_INCOMING);

	if (ok) {
		ok = 0;
		call_macro(sid, MC_SUCC_DONE);
	}

	if (q->flags & SD_COPY_ACTION) {
		q->flags &= ~SD_COPY_ACTION;
		delay_action(action, q->sector_tag, q->side_tag, flags, 0, 0);
		q->flags |= SD_COPY_ACTION;
	}

	if (q->flags & SD_SEND_ACTION) {
		q->flags &= ~SD_SEND_ACTION;
		delay_action(q->action, q->sector_tag, q->side_tag, q->flags, 0, 0);
		q->flags |= SD_SEND_ACTION;
	}

	if (q->flags & SD_APPLY_2ND && s->step_next[dir]) {
		gameMap.doAction(action, s->step_next[dir], (dir + 2) & 3, flags, 1);
	}

	return 0;
}

void Map::moveBoat(unsigned from, unsigned to) {
	if (_sectors[from].sector_type == S_LODKA && _sectors[to].sector_type == S_VODA) {
		_sectors[from].sector_type = S_VODA;
		_sectors[to].sector_type = S_LODKA;
	}
}

void Map::addNote(int x, int y, int depth, const char *str) {
	unsigned i;

	if (!*str) {
		return;
	}

	if (_notesCount >= _notesSize) {
		MapText *ptr = new MapText[_notesSize + 32];

		memcpy(ptr, _mapNotes, _notesSize * sizeof(MapText));
		memset(ptr + _notesSize, 0, 32 * sizeof(MapText));
		delete[] _mapNotes;
		_mapNotes = ptr;
		_notesSize += 32;
	}

	for (i = 0; i < _notesSize && _mapNotes[i].text; i++);

	_mapNotes[i].x = x;
	_mapNotes[i].y = y;
	_mapNotes[i].depth = depth;
	_mapNotes[i].text = new char[strlen(str) + 1];
	strcpy(_mapNotes[i].text, str);
	_notesCount++;
}

void Map::removeNote(unsigned idx) {
	unsigned i;

	if (idx >= _notesSize || !_mapNotes[idx].text) {
		return;
	}

	// free the note
	delete[] _mapNotes[idx].text;
	_mapNotes[idx].text = NULL;
	_notesCount--;

	// pack the array
	for (i = idx + 1; i < _notesSize; i++) {
		if (_mapNotes[i].text) {
			_mapNotes[idx++] = _mapNotes[i];
			_mapNotes[i].text = NULL;
		}
	}
}

long load_section(FILE *f, void **section, int *sct_type, long *sect_size) {
	long s;
	char c[20];
	size_t stfu;

	*section = NULL;
	stfu = fread(c, 1, sizeof(sekceid), f);

	if (strcmp(c, sekceid)) {
		return -1;
	}

	stfu = fread(sct_type, 1, sizeof(*sct_type), f);
	stfu = fread(sect_size, 1, sizeof(*sect_size), f);
	stfu = fread(&s, 1, sizeof(s), f);
	*section = getmem(*sect_size);
	s = fread(*section, 1, *sect_size, f);
	return s;
}


void prepare_graphics(int *ofs, MemoryReadStream *names, void (*decomp)(void**, long*), int cls) {
	const char *ptr;

	for (ptr = names->readCString(); !names->eos(); ptr = names->readCString()) {
		def_handle((*ofs)++, ptr, decomp, cls);
	}
}

static void preload_percent(int cur,int max)
  {
  int pos;
  pos=cur*640/max;
  if (pos>640) pos=640;
  curcolor=RGB555(16,16,16);hor_line(0,476,pos);
  curcolor=RGB555(8,8,8);hor_line(0,477,pos);
  curcolor=RGB555(31,31,31);hor_line(0,475,pos);
  showview(0,460,640,20);
  do_events();
  }

//#pragma preload_objects parm [];
void preload_objects(int ofsts) {
	int i;
	char lodka = 1;
	char c[200];

	for (i = 1; i < gameMap.coordCount(); i++) {
		if (gameMap.sectors()[i].sector_type == S_LODKA) {
			break;
		}
	}

	if (i == gameMap.coordCount()) {
		lodka = 0;
	}

//  sprintf(c,"%sLOADING.MUS",pathtable[SR_DATA]);
//  Sound_ChangeMusic(c);
//  Sound_ChangeMusic(Sys_FullPath(SR_DATA, "LOADING.MUS"));
	trans_bar(0, 460, 640, 20, 0);
	position(0, 460);
	set_font(H_FBOLD, RGB555(0, 31, 0));
	sprintf(c, texty[TX_LOAD], gameMap.global().mapname);
	outtext(c);

	for (i = 0; i < ofsts; i++) {
		if (i < H_LODKA0 || i > H_LODKA7 || lodka) {
			apreload_sign(i, ofsts);
		}
	}

	apreload_start(preload_percent);
/*  for(i=0;i<ofsts;i++)
     {
     ablock(ofsts-i-1);
     preload_percent(i+ofsts,ofsts);
     }
  */
}

int load_level_texts(char *filename)
  {
  int err;

  err=load_string_list_ex(level_texts,filename);
  return err;
  }

char *pripona(const char *filename,const char *pripona)
  {
  char *buff;
  char *c;

  buff = (char*)getmem(strlen(filename)+strlen(pripona)+2);
  strcpy(buff,filename);
  c=strrchr(buff,'\\');
  if (c==NULL) c=buff;
  c=strchr(c,'.');
  if (c!=NULL) *c=0;
  strcat(buff,pripona);
  return buff;
  }

void show_loading_picture(const char *filename)
  {
  uint16_t *p;
  long s;

  p = (uint16_t*)afile(filename,SR_BGRAFIKA,&s);
  Screen_FixPalette(p + 3, s/sizeof(uint16_t) - 3);
  put_picture(0,0,p);
  showview(0,0,0,0);
  free(p);
  #ifdef LOGFILE
  display_ver(639,0,2,0);
  #endif
  cancel_pass=1;
  }

extern int snd_devnum;

int load_map(char *filename) {
	int ret;
	char *c, *d;

	schovej_mysku();

	if (level_preload) {
		show_loading_picture("LOADING.HI");
	}

	Sound_ChangeMusic("?");
	zobraz_mysku();
	ret = gameMap.load(filename);

	if (ret < 0) {
		return ret;
	}

	memset(minimap, 0, sizeof(minimap));

	if (level_preload) {
		preload_objects(end_ptr);
	}

	c = find_map_path(filename);
	d = pripona(find_map_path(filename), ".TXT");
	ret = load_level_texts(d);
	free(d);
	free(c);

	if (!ret) {
		create_playlist(level_texts[0]);
	}

	init_tracks();
	play_next_music(&d);
	Sound_ChangeMusic(d);

	if (!doNotLoadMapState && gameMap.restore() == -2) {
		closemode();
		Sys_ErrorBox("Bug in temp file. Please purge some status blocks in last load savegame file.");
		exit(0);
	}

	doNotLoadMapState = 0;
	return ret;
}

void add_leaving_place(int sector) {
	add_field_num(leaving_places, gameMap.fname(), sector);
}

void save_leaving_places(void)
  {
  char *s;

  if (!leaving_places.count()) return;
//  concat(s,pathtable[SR_TEMP],"_LPLACES.TMP");
//  save_config(leaving_places,s);
	save_config(leaving_places, Sys_FullPath(SR_TEMP, "_LPLACES.TMP"));
  }

void load_leaving_places(void)
  {
  char *s;

  leaving_places.clear();
//  concat(s,pathtable[SR_TEMP],"_LPLACES.TMP");
//  leaving_places=read_config(s);
	read_config(leaving_places, Sys_FullPath(SR_TEMP, "_LPLACES.TMP"));
  }

int set_leaving_place(void) {
	return get_leaving_place(gameMap.fname());
}

int get_leaving_place(const char *level_name) {
	int s = 0, i;

	if (!leaving_places.count()) {
		return 0;
	}

	if (get_num_field(leaving_places, level_name,&s) || ~gameMap.coord()[s].flags & 1 || gameMap.sectors()[s].sector_type != S_LEAVE || s == 0) {
		for (i = 1; i < gameMap.coordCount(); i++) {
			if (gameMap.sectors()[i].sector_type == S_LEAVE && gameMap.coord()[i].flags & 0x1) {
				return i;
			}
		}

		return s;
	}

	return s;
}


void leave_current_map() {
	int i;
	TFLY *p;

	SEND_LOG("(GAME) Leaving current map ... start", 0, 0);
	add_leaving_place(viewsector);
	kill_all_sounds();
	restore_sound_names();
	remove_all_mob_spells();
	regen_all_mobs();

	if (save_map) {
		save_map_state(); //do tempu se zabali status mapy
	}

	destroy_fly_map();
	stop_all_fly();
	save_map = 1;

	while (d_action != NULL) {
		void *p = d_action;
		d_action = d_action->next;
		free(p);
	}

	level_texts.clear();

	if (mob_map != NULL) {
		free(mob_map);
	}

	for (i = 0; i < hl_ptr; i++) {
		THANDLE_DATA *h;

		h = get_handle(i);

		if (h->loadproc == pcx_fade_decomp) {
			zneplatnit_block(i);
		}
	}

	for (i = hl_ptr; i < end_ptr; i++) {
		undef_handle(i);
	}

	build_all_players();
	purge_playlist();
	hold_timer(TM_BACK_MUSIC, 0);
	gameMap.close();
}

void recheck_button(int sector, char auto_action) {
	const TSECTOR *ts;
	short * const *ms;
	char swch;

	ts = gameMap.sectors() + sector;

	if (ts->sector_type != S_TLAC_OFF && ts->sector_type != S_TLAC_ON) {
		return;
	}

	ms = gameMap.items() + (sector << 2);
	swch = (gameMap.coord()[sector].flags & MC_DPLAYER || mob_map[sector] || ms[0] != NULL || ms[1] != NULL || ms[2] != NULL || ms[3] != NULL);

	switch (ts->sector_type) {
	case S_TLAC_OFF:
		if (swch) {
			gameMap.buttonActivate(sector);

			if (auto_action) {
				gameMap.doAction(ts->action, ts->sector_tag, ts->side_tag, 0, 0);
			}
		}
		break;

	case S_TLAC_ON:
		if (!swch) {
			gameMap.buttonDeactivate(sector);

			if (auto_action) {
				gameMap.doAction(ts->action, ts->sector_tag, ts->side_tag, 0, 0);
			}
		}
		break;
	}
}

void stop_fly(LETICI_VEC *p,char zvuk)
  {
  if (zvuk && ~p->flags & FLY_NEDULEZITA)sirit_zvuk(p->sector);
  if (!(p->flags & FLY_DESTROY))
     {
     item_sound_event(p->items==NULL?p->item:p->items[0],p->sector);
     if (p->xpos>=0) push_item(p->sector,(p->smer+(p->ypos>0))&3,p->items);
                else push_item(p->sector,(p->smer+(p->ypos<0)+2)&3,p->items);
     }
  p->flags |= FLY_BURNT;
  }


void stop_all_fly() {
  LETICI_VEC *p=letici_veci;
  while (p!=NULL) {stop_fly(p,0);p=p->next;}
  p=letici_veci;
  while (p!=NULL) {p->flags |= FLY_UNUSED;p=p->next;}
  while (letici_veci) calc_fly(NULL);
}

void interrupt_fly(LETICI_VEC *p)
  {
           if (p->flags & FLY_DESTROY_SEQ) return;
           p->speed=0;
           if (p->flags & FLY_DESTROY)
              {
              p->flags|=FLY_DESTROY_SEQ|FLY_NEHMOTNA;
              p->anim_pos=3;
              p->velocity=0;
              item_sound_event(p->items==NULL?p->item:p->items[0],p->sector);
              }
           else
              if (p->velocity==0) stop_fly(p,1);else if (p->velocity<10) p->velocity=10;
  }

LETICI_VEC *create_fly()
  {
  LETICI_VEC *p,*q;

  p=letici_veci;
  q=NULL;
  while (p!=NULL) if (p->flags & FLY_UNUSED)
     {
     if (q==NULL) letici_veci=p->next;else q->next=p->next;
     free(p->items);
     break;
     }
     else
        {
        q=p;
        p=p->next;
        }
  if (p==NULL) p=New(LETICI_VEC);
  p->flags=0;
  p->items=NULL;
  p->item=0;
  p->lives=0;
  return p;
  }

TFLY *duplic_fly(TFLY *p)
  {
  TFLY *q;

  q=create_fly();
  *q=*p;
  if (q->items!=NULL)
     {
//     int s=_msize(q->items);
     int s = 0;
     for (; q->items[s]; s++);
     q->items = (int16_t*)getmem(s*sizeof(int16_t));
     memcpy(q->items,p->items,s*sizeof(int16_t));
     }
  return q;
  }

void calc_fly(the_timer *arg) {
	LETICI_VEC *p, *q;
	short ss;

	for (p = letici_veci; p != NULL; p = q) {
		p->xpos += p->speed;

		if (p->flags & FLY_BURNT) {
			if (p->flags & FLY_UNUSED) {
				short ds[2];

				if (letici_veci == p) {
					letici_veci = p->next;
				} else {
					for (q = letici_veci; q->next != p; q = q->next);
					q->next = p->next;
				}

				q = p->next;
				ds[0] = p->item;
				ds[1] = 0;

				if (p->items != NULL) {
					destroy_items(p->items);
				} else {
					destroy_items(ds);
				}

				free(p->items);
				p->items = NULL;
				free(p);
				continue;
			} else {
				q = p->next;
				continue;
			}
		}

		q = p->next;

		if (!(p->flags & FLY_NEHMOTNA)) {
			p->velocity += 1;              
		}

		p->zpos -= p->velocity;
		ss = p->sector;

		if (!(p->flags & FLY_NEDULEZITA)) {
			neco_v_pohybu = 1;
		}

		if (p->zpos > 110) {
			p->zpos = 110;
		}

		if (p->zpos <= 20) {
			if (gameMap.sectors()[p->sector].sector_type == S_DIRA) {
				p->speed = 0;

				if (p->zpos <= -128) {
					p->sector = gameMap.sectors()[p->sector].sector_tag;
					p->zpos = 256;
				}
			} else {
				stop_fly(p, 1);
			}

			continue;
		}     

		if (p->flags & FLY_IN_FLY && p->xpos > -16 && zasah_veci(p->sector, p)) {
			interrupt_fly(p);
			continue;
		}

		if (p->xpos > 63) {
			p->flags |= FLY_IN_FLY;
			p->xpos -= 127;
			ss = gameMap.sectors()[p->sector].step_next[p->smer];

			if (p->sector != 0 && !(gameMap.sides()[p->sector * 4 + p->smer].flags & SD_THING_IMPS)) {
				p->sector = ss;

				if (ISTELEPORT(gameMap.sectors()[p->sector].sector_type)) {
					p->sector = gameMap.sectors()[p->sector].sector_tag;
				}
			} else {
				p->xpos += 127 - p->speed;

				if (!zasah_veci(p->sector, p) && p->lives) {
					p->lives--;
				}

				if (p->lives) {
					TFLY *q,*z;

					q = duplic_fly(p);
					q->smer = p->smer + 1 & 3;
					z = duplic_fly(p);
					z->smer = p->smer - 1 & 3;
					add_fly(q);
					add_fly(z);
				}

				interrupt_fly(p);
			}
		} else if (p->counter++ > 100) {
			zasah_veci(p->sector, p);
			interrupt_fly(p);
			continue;
		}
	}
}

void delay_action(int action_numb,int sector,int direct,int flags,int nosend,int delay)
  {
  D_ACTION *d;
  if (!sector && !direct) return;
  if (!delay) gameMap.doAction(action_numb,sector,direct,flags,nosend);
  else
     {
     d=(D_ACTION *)getmem(sizeof(D_ACTION));
     d->action=action_numb;
     d->sector=sector;
     d->side=direct;
     d->flags=flags>>16;
     d->nocopy=nosend;
     d->delay=delay;
     d->next=d_action;
     d_action=d;
     }
  }
/*
int get_action_delay(TSTENA *q)
  {
  if (q->flags & SD_DELAY)
     {
     if (q->sec) return q->sec_anim & 0xf;
     return q->prim_anim & 0xf;
     }
  else
     return 0;
  }
*/

/*
void black(int i,int sector,int dir)
  {
  int xl,y1,xr,y2;

  if (!(map_sides[sector*4+dir].flags & SD_INTERIER)) return;
     y2=MIDDLE_Y+points[i][0][VIEW3D_Z].y+16;
     y1=MIDDLE_Y+points[i][1][VIEW3D_Z].y+16;
  if (i<0)
     {
     xl=MIDDLE_X-points[i][0][VIEW3D_Z].x;
     xr=MIDDLE_X-points[i+1][0][VIEW3D_Z].x;
     }
  else
  if (i>0)
     {
     xl=MIDDLE_X+points[i-1][0][VIEW3D_Z].x;
     xr=MIDDLE_X+points[i][0][VIEW3D_Z].x;
     }
  else
     {
     xl=MIDDLE_X-points[0][0][VIEW3D_Z].x;
     xr=MIDDLE_X+points[0][0][VIEW3D_Z].x;
     }
  if (xl<0) xl=0;
  if (xr>639) xr=639;
  curcolor=back_color;
  bar(xl,y1,xr,y2);
  }
*/

void do_delay_actions()
  {
  D_ACTION *d,*p,*q;

  d=d_action;p=NULL;
  while (d!=NULL)
     {
     if (!(--d->delay))
        {
        q=d;
        if (q==d_action)
           {
           d_action=d->next;
           d=d_action;
           }
        else
           {
           p=d_action;
           while (p->next!=d) p=p->next;
           p->next=d->next;
           d=p->next;
           }
       gameMap.doAction(q->action,q->sector,q->side,q->flags<<16,q->nocopy);
       free(q);
       }
     else
        {
        d=d->next;
        }
     }
  }

void mrtva_skupina()
  {
  int i=0;

  for(i=0;!postavy[i].groupnum || !postavy[i].used;i++);
  zmen_skupinu(&postavy[i]);
  }
/*
static void check_pod_vodou(char mode)
  {
  short sectors[POCET_POSTAV];
  short psec;
  short pp=0;
  int i,j;


  for(i=0;i<POCET_POSTAV;i++)
     {
     psec=postavy[i].sektor;
     for(j=0;j<pp;j++) if (sectors[j]==psec) break;
     if (j==pp)
        {

        }
     }
  }
*/
static void akce_voda(THUMAN *h,int mode)
  {
  int ziv=mode?10:1;
  if (h->kondice) h->kondice-=ziv*2;
     else
        {
        player_hit(h,h->lives,0);
        }
  if (h->kondice<0) h->kondice=0;
  bott_draw(0);
  }

void check_players_place(char mode) {
	int i;
	THUMAN *h = postavy;
	char vir_eff = 0;
	static char vir_zavora = 0;
	char levitat;

	for (i = 0; i < POCET_POSTAV; i++, h++) {
		if (h->used && h->lives) {
			int sect;
			int u1;

			levitat = (h->vlastnosti[VLS_KOUZLA] & SPL_LEVITATION) != 0;
			sect = h->sektor;

			if (sect >= gameMap.coordCount()) {
				continue;
			}

			switch (gameMap.sectors()[sect].sector_type) {
			case S_ACID:
				if (!levitat) {
					if (h->lives > 3) {
						h->lives -= 3;
						bott_draw(0);
					} else {
						player_hit(h, 3 + 7 * mode, 0);
					}
				}
				break;

			case S_VIR:
				if (!levitat) {
					if (mode == 0 && vir_zavora == 0) {
						int i, smer;

						vir_zavora = 1;
						pass_zavora = 0;
						smer = rnd(100) < 50 ? -1 : 1;

						if (vir_eff == 0) {
							for (i = 0; i < 8; i++) {
								turn_zoom(smer);
							}
						}

						vir_eff = 1;
						vir_zavora = 0;
						cancel_pass = 1;
					} else {
						break;
					}
				}

			case S_LAVA:
				if (!levitat) {
					u1 = h->lives;
					player_hit(h, u1, 0);
					bott_draw(0);
				} else {
					if (h->lives > 3) {
						h->lives -= 3;
						bott_draw(0);
					} else {
						player_hit(h, 3 + 7 * mode, 0);
					}
				}
				break;

			case S_SSMRT:
				u1 = h->lives;
				player_hit(h, u1, 0);
				bott_draw(0);
				break;

			case S_VODA:
				if (!levitat) {
					akce_voda(h, mode);
				}
				break;

			case S_DIRA:
				if (!pass_zavora) {
					postavy_propadnout(sect);
				}
				break;

			case S_LODKA:
				if (lodka != 1 && mode) {
					set_backgrnd_mode(1);
					lodka = 1;
				}
				break;
			}

			if (gameMap.global().map_effector == ME_PVODA && ~gameMap.coord()[h->sektor].flags & MC_SAFEPLACE) {
				akce_voda(h, mode);
			}

			if (gameMap.sectors()[sect].sector_type != S_LODKA && lodka) {
				set_backgrnd_mode(0);
				lodka = 0;
			}
		}
	}
}

static void move_lodka(int oldsect, int newsect) {
	gameMap.moveBoat(oldsect, newsect);
}

void calc_game()
  {
  int d;
	gameMap.calcAnimations();
  if (d_action!=NULL) do_delay_actions();
  calc_mobs();
  if (force_start_dialog && !norefresh && !cancel_render)
     {
     force_start_dialog=0;
     call_dialog(start_dialog_number,start_dialog_mob);
     }
  check_players_place(0);
  if ((d=check_end_game())!=0)
     if (d==1) wire_end_game();else mrtva_skupina();
  if (battle && cur_mode!=MD_INBATTLE)
      {
      start_battle();
      }
  else if(!battle && select_player!=-1)
     {
     select_player=-1;
     bott_draw(0);
     }
  }

void a_touch(int sector, int dir) {
	const TSTENA *q;
	int sid;

	sid = sector * 4 + dir;
	q = gameMap.sides() + sid;

	if (q->flags & SD_PASS_ACTION) {
		return;
	}

	if (q->sec && ~q->flags & SD_SEC_VIS) {
		return;
	}

	side_touched = 1;

	if (!q->sec) {
		delay_action(q->action, q->sector_tag, q->side_tag, q->flags, 0, 0);
	} else if (q->flags & SD_SEC_VIS) {
		if (q->flags & SD_AUTOANIM) {
			gameMap.doAction(A_OPEN_CLOSE, sector, dir, 0, 1);
		}

		delay_action(q->action, q->sector_tag, q->side_tag, q->flags, 0, 0);
	}

	call_macro(sid, MC_TOUCHSUC);
}

void a_pass(int sector, int dir) {
	const TSTENA *q;

	q = gameMap.sides() + sector * 4 + dir;
	gameMap.clearSideFlags(sector * 4 + dir, SD_SECRET);

	if (q->flags & SD_ALARM) {
		if (GlobEvent(MAGLOB_ALARM, viewsector, viewdir)) {
			sirit_zvuk(gameMap.sectors()[sector].step_next[dir]);
		}
	}

	if (!(q->flags & SD_PASS_ACTION)) {
		return;
	}

	delay_action(q->action, q->sector_tag, q->side_tag, q->flags, 0, 0);
}

void zmen_skupinu(THUMAN *p)
  {
  int i;

  if (p->groupnum==0) {bott_draw(0);return;}
  cur_group=p->groupnum;
  for(i=0;i<POCET_POSTAV;i++) if (!postavy[i].lives && postavy[i].groupnum!=cur_group) postavy[i].groupnum=0;
  viewsector=p->sektor;
  viewdir=p->direction;
  schovej_mysku();
  bott_draw(0);
  other_draw();
  ukaz_mysku();
  showview(0,378,640,480);
  ukaz_mysku();
  cancel_render=1;
  }

void sort_groups()
  {
  int i,j,t=0;

  if (cur_mode==MD_PRESUN) group_sort[0]=moving_player;
  if (enable_sort)
     {
     for(i=0;i<POCET_POSTAV && t<POCET_POSTAV;i++)
       {
       for(j=0;j<POCET_POSTAV;j++) if (postavy[j].groupnum==i+1) group_sort[t++]=j;
       }
    j=0;
    while (t<POCET_POSTAV)
       {
       while (postavy[j].groupnum) j++;
       group_sort[t++]=j++;
       }
     }
  else
     for(i=0;i<POCET_POSTAV;i++) group_sort[i]=i;
  }

void add_to_group(int num)
  {
  if (lodka)return;
  if (!postavy[num].used) return;
  if (postavy[num].groupnum!=cur_group) postavy[num].groupnum=cur_group;
  else if (!postavy[num].lives) postavy[num].groupnum=0;else
     {
     char cisla[7];
     int i;

     memset(cisla,0,sizeof(cisla));
     for(i=0;i<POCET_POSTAV;i++) cisla[postavy[i].groupnum]=1;
     for(i=1;i<7 && cisla[i]!=0;i++);
     if (i==7) return;
     postavy[num].groupnum=i;
     }
  }

void group_all(void)
  {
  char c=1;
  int i;
  THUMAN *h;

  while (chod_s_postavama(1)) c=0;
  if (c)
     {
     if (cur_group!=1)
        {
        for(i=0,h=postavy;i<POCET_POSTAV;i++,h++) if (h->used && h->groupnum==1 && h->sektor!=viewsector) break;
        if (i==POCET_POSTAV) cur_group=1;
        }
     for(i=0,h=postavy;i<POCET_POSTAV;i++,h++)
        if (h->used && h->lives && h->sektor==viewsector) h->groupnum=cur_group;
     }

  bott_draw(0);
  }

//je nutne volat pred presunem postav
void destroy_player_map() {
	int i;

	for (i = 0; i < POCET_POSTAV; i++) {
		gameMap.clearCoordFlags(postavy[i].sektor, MC_DPLAYER | MC_SAFEPLACE);
	}
}

//je nutne volat po presunu postav
void build_player_map() {
	int i;
	THUMAN *p;

	for (i = 0; p = &postavy[i], i < POCET_POSTAV; i++) {
		gameMap.setCoordFlags(p->sektor, p->lives ? MC_PLAYER : MC_DEAD_PLR);

		if (gameMap.global().map_effector == ME_PVODA) {
			if (q_item_one(i, water_breath + 1)) {
				gameMap.setCoordFlags(p->sektor, MC_SAFEPLACE);
			}
		}
	}
}


char chod_s_postavama(char sekupit)
  {
  int i,/*j,*/lastsec=-1;
  char marks[6];
  char gatt=0;
  signed char group_nums[7];

  destroy_player_map();
  if (cur_mode==MD_PRESUN)
     {
        postavy[select_player].sektor=viewsector;
        postavy[select_player].direction=viewdir;
     }
  else
     {
     memset(group_nums,0xff,sizeof(group_nums));
     group_nums[0]=0;
     for(i=0;i<POCET_POSTAV;i++)
        {
        if (postavy[i].groupnum==cur_group)
           {
           int wh;
           int wf;
           lastsec=postavy[i].sektor;
           postavy[i].sektor=viewsector;
           postavy[i].direction=viewdir;
           postavy[i].utek=1;
           postavy[i].kondice-=weigth_defect(postavy+i);
           if (postavy[i].kondice<0) postavy[i].kondice=0;
           marks[i]=1;
           group_nums[cur_group]=1;
           }
        else
           if (sekupit && lastsec==viewsector) marks[i]=postavy[i].sektor==viewsector;
           else marks[i]=0;
        }
/*     if (formace && sekupit)
        {
        int gr=2;
        for(;;)
           {
           THUMAN *h1,*h2;
           int dir;
           char attach=0;
           for(i=0;h1=postavy+group_sort[i],i<POCET_POSTAV;i++) if (h1->used && h1->lives && !marks[group_sort[i]])
             {
             int sc;
             int ss=(sc=h1->sektor)<<2;
             for(dir=0;dir<4;dir++) if (map_sectors[sc].step_next[dir]==lastsec && ~map_sides[ss+dir].flags & SD_PLAY_IMPS) break;
             if (dir!=4)
                {
                for(j=0,h2=postavy;j<POCET_POSTAV;j++,h2++) if (h2->used && h2->lives && h2->groupnum==h1->groupnum)
                   {
                   h2->sektor=lastsec;
                   h2->direction=dir;
                   h2->utek=1;
                   marks[j]=1;
                   }
                lastsec=sc;
                attach=1;
                gatt=1;
                group_nums[h1->groupnum]=gr++;
                }
             }
           if (!attach) break;
           }
        for(i=1;i<7;i++) if (group_nums[i]==-1) group_nums[i]=gr++;
        for(i=0;i<POCET_POSTAV;i++) if (postavy[i].used) postavy[i].groupnum=group_nums[postavy[i].groupnum];
        cur_group=1;
        }
  */
     }
  build_player_map();
  SEND_LOG("(GAME) New position %d:%d",viewsector,viewdir);
  return gatt;
  }

void shift_zoom(char smer)
  {
  if (cancel_pass) return;
  hold_timer(TM_BACK_MUSIC,1);
  switch (smer & 3)
     {
     case 0:if (lodka)zooming_forward((uint16_t*)ablock(H_LODKA));
             else
            zooming_forward((uint16_t*)ablock(H_BGR_BUFF));break;
     case 1:turn_left();break;
     case 2:if (lodka)zooming_backward((uint16_t*)ablock(H_LODKA));
             else
            zooming_backward((uint16_t*)ablock(H_BGR_BUFF));break;
     case 3:turn_right();break;
     }
  hold_timer(TM_BACK_MUSIC,0);
  }

void hide_ms_at(int line)
  {
  if (ms_last_event.y<line)
     {
     update_mysky();
     schovej_mysku();
     }
  }

void real_krok(EVENT_MSG *msg,void **data)
  {
  if (msg->msg==E_INIT || msg->msg==E_DONE) return;
  check_all_mobs();
  calc_game();msg;data;
  SEND_LOG("(GAME) STEP",0,0);
  }

void do_halucinace() {
	hal_sector = rnd(gameMap.coordCount() - 1) + 1;
	hal_dir = rnd(4);
}

void sector0(void)
  {
  int i;

  for(i=0;i<POCET_POSTAV;i++) if (postavy[i].sektor==0 && postavy[i].used)
     {
     postavy[i].lives=0;
     player_check_death(postavy+i,0);
     }
  }

static void tele_redraw()
  {
  if (!running_anm) return;
  redraw_scene();
	gameMap.calcAnimations();
  showview(0,17,640,360);
  }

static void tele_redraw_wrap(the_timer *arg) {
	tele_redraw();
}

void postavy_teleport_group(int sector, int dir, int postava, char ef_mode) {
	int i;

	i = mob_map[sector];

	if (i) {
		mobs[--i].vlajky &= ~MOB_LIVE;

		if ((i = mobs[i].next) != 0) {
			mobs[--i].vlajky &= ~MOB_LIVE;
		}

		mob_map[sector] = 0;
	}

	destroy_player_map();

	for (i = 0; i < POCET_POSTAV; i++) {
		if (postava & (1<<i)) {
			if (ef_mode) {
				gameMap.addSpecTexture(postavy[i].sektor, H_TELEP_PCX, 14, 1, 0);
				play_sample_at_sector(H_SND_TELEPOUT, viewsector, viewsector, 0, 0);
			}

			postavy[i].sektor = sector;
			postavy[i].direction = dir;

			if (!sector) {
				player_hit(postavy + i, postavy[i].lives, 0);
			}

			if (ef_mode) {
				gameMap.addSpecTexture(sector, H_TELEP_PCX, 14, 1, 0);
				play_sample_at_sector(H_SND_TELEPOUT, viewsector, sector, 0, 0);
			}

			ef_mode = 0;
		}
	}

	build_player_map();
}

void postavy_teleport_effect(int sector, int dir, int postava, char effect) {
	char kolo = global_anim_counter;
	int counter = 0;
	static int zavora = 0;

	if (zavora) {
		return;
	}

	if (effect) {
		void (*c)();

		zavora = 1;
		c = wire_proc;
		unwire_proc();
		recalc_volumes(viewsector, viewdir);
		add_to_timer(-1, gamespeed, -1, tele_redraw_wrap);
		gameMap.addSpecTexture(viewsector, H_TELEP_PCX, 14, 1, 0);

		while (running_anm) {
			do_events();
		}

		play_sample_at_channel(H_SND_TELEPIN, 2, 100);
		play_big_mgif_animation(H_TELEPORT);

		while (!running_anm) {
			Task_Sleep(NULL);
		}

		kolo = global_anim_counter;

		if (norefresh) {
			norefresh = 0;
			cancel_pass = 1;
		}

		while (running_anm) {
			do_events();

			if (kolo != global_anim_counter) {
				counter += (char)(global_anim_counter - kolo);
				kolo = global_anim_counter;

				if (counter == 6) {
					postavy_teleport_group(sector, dir, postava, 0);
					viewsector = sector;
					viewdir = dir & 0x3;
					recalc_volumes(viewsector, viewdir);
				}
			}
		}

		kolo = global_anim_counter;

		while (kolo != global_anim_counter) {
			do_events();
		}

		delete_from_timer(-1);
		wire_proc();
		wire_proc = c;
		cancel_render = 1;
	} else {
		postavy_teleport_group(sector, dir, postava, 1);
		viewsector = sector;
		viewdir = dir & 0x3;
	}

	zavora = 0;
}

void check_postavy_teleport() {
	int nsect = viewsector;

	if (ISTELEPORT(gameMap.sectors()[viewsector].sector_type)) {
		int i, ss = 0, sid = viewsector * 4 + viewdir;

		for (i = 0; i < POCET_POSTAV; i++) {
			if (postavy[i].sektor == viewsector && postavy[i].used) {
				ss |= 1 << i;
			}
		}

		if (gameMap.sides()[sid].flags & SD_SEC_VIS) {
			postavy_teleport_effect(gameMap.sectors()[viewsector].sector_tag, gameMap.sectors()[nsect].side_tag, ss, 1);
		} else {
			postavy_teleport_group(viewsector = gameMap.sectors()[viewsector].sector_tag, viewdir = gameMap.sectors()[nsect].side_tag, ss, 0);
		}
	}
}

static char test_can_walk(int grp)
  {
  int i;
  THUMAN *h;

  for(i=0,h=postavy;i<POCET_POSTAV;i++,h++)
     if (h->used && h->groupnum==grp && h->lives &&
        (h->vlastnosti[VLS_POHYB]==0 || h->kondice==0)) return 0;
  return 1;
  }

void step_zoom(char smer) {
	char nopass, drs;
	int sid, nsect, sect;
	char can_go=1;
	
	if (running_anm) return;
	if (pass_zavora) return;
	if (lodka && (smer == 1 || smer == 3)) return;

	cancel_pass = 0;
	drs = (viewdir + smer) & 3;
	sid = viewsector * 4 + drs;
	sect = viewsector;
	call_macro(sid, MC_EXIT);
	nopass = (gameMap.sides()[sid].flags & SD_PLAY_IMPS);

	if (nopass) {
		call_macro(sid, MC_PASSFAIL);
	} else {
		call_macro(sid, MC_PASSSUC);
	}

	if (cur_mode == MD_PRESUN) {
		select_player = moving_player;
		if (!postavy[select_player].actions) {
			nopass = 1;
		}
	} else {
		can_go = test_can_walk(cur_group);
	}

	if (!can_go) {
		bott_disp_text(texty[220]);
		return;
	}

	if (force_start_dialog) cancel_pass = 1;
	if (cancel_pass) return;
	if (!GlobEvent(MAGLOB_ONSTEP, viewsector, viewdir)) return;

	if (viewsector != sect) {
		nsect = viewsector;
		viewsector = sect;
	} else {
		nsect = gameMap.sectors()[viewsector].step_next[drs];
	}

	if (gameMap.sectors()[nsect].sector_type == S_SCHODY) {
		int i;
		viewdir = (viewdir + gameMap.sectors()[nsect].side_tag) & 3;
		nsect = gameMap.sectors()[nsect].sector_tag;
		i = mob_map[nsect];

		while (i != 0) {
			i--;
			mobs[i].sector=mobs[i].home_pos;
			i=mobs[i].next;
		}

		mob_map[nsect]=0;
	} else if (mob_map[nsect] && !nopass) {
		if (!battle) {
			if (!mob_alter(nsect)) return;
		} else {
			return;
		}
	}

	if (gameMap.sectors()[nsect].sector_type == S_LODKA) {
		int i;
		THUMAN *h;
		group_all();
		can_go=1;

		for (i = 0, h = postavy; i < POCET_POSTAV; i++, h++) {
			if (h->groupnum != cur_group && h->lives) break;
		}

		if (i != POCET_POSTAV) {
			bott_disp_text(texty[66]);
			return;
		}
	}

	pass_zavora=1;
	norefresh=1;
	schovej_mysku();
	anim_sipky(H_SIPKY_S+smer,1);
	anim_sipky(0,255);
	hide_ms_at(385);
	ukaz_mysku();

	if (set_halucination) do_halucinace();
	if (loadlevel.name[0]) {
		if (!battle) {
			pass_zavora=0;
			return;
		}

		nopass=1;
		loadlevel.name[0]=0;
		exit_wait=0;
	}

	if (!can_go) {
		nopass=1;
	}

	if (!nopass) {
		a_pass(viewsector, drs);
		viewsector = nsect;
		move_lodka(sect, nsect);
		chod_s_postavama(1);
		send_message(E_KROK);
	}

	if (!cancel_pass) {
		render_scene(viewsector, viewdir);
		if (smer == 2) {
			OutBuffer2nd();
			if (!nopass) shift_zoom(smer);
		} else {
			shift_zoom(smer);
			OutBuffer2nd();
			if (nopass) shift_zoom(smer + 2);
		}
		if (battle || (game_extras & EX_ALWAYS_MINIMAP)) draw_medium_map();
		sort_groups();
		bott_draw(0);
		other_draw();
	}

	update_mysky();
	ukaz_mysku();

	if (!cancel_render) showview(0,0,0,0);

	norefresh=0;
	cancel_render=1;
	Sound_MixBack(0);
	viewsector = postavy_propadnout(viewsector);
	check_postavy_teleport();
	recheck_button(sect, 1);
	recheck_button(viewsector, 1);
	check_players_place(1);
	cancel_pass = 0;
	pass_zavora = 0;
	if (force_start_dialog) {
		force_start_dialog = 0;
		call_dialog(start_dialog_number, start_dialog_mob);
	}
	if (cur_mode == MD_GAME) recalc_volumes(viewsector, viewdir);
}

void turn_zoom(int smer)
  {
  if (running_anm) return;
  if (pass_zavora) return;else pass_zavora=1;
  if (!GlobEvent(MAGLOB_ONTURN,viewsector,viewdir)) return;
  if (set_halucination) do_halucinace();
  norefresh=1;
  hold_timer(TM_BACK_MUSIC,1);
                 viewdir=(viewdir+smer)&3;
                 render_scene(viewsector,viewdir);
                 hide_ms_at(387);
                 if (smer==1)
                    {
                    anim_sipky(H_SIPKY_SV,1);
                    anim_sipky(0,255);
                    turn_left();
                    }
                    else
                    {
                    anim_sipky(H_SIPKY_SZ,1);
                    anim_sipky(0,255);
                    turn_right();
                    }
  chod_s_postavama(0);
  OutBuffer2nd();
  if (battle || (game_extras & EX_ALWAYS_MINIMAP)) draw_medium_map();
  other_draw();
  update_mysky();
  ukaz_mysku();
  showview(0,0,0,0);
  recalc_volumes(viewsector,viewdir);
  if (!battle) calc_game();
  norefresh=0;
  cancel_render=1;
  hold_timer(TM_BACK_MUSIC,0);
  Sound_MixBack(0);
  pass_zavora=0;
  }

int check_path(uint16_t **path, uint16_t tosect) {
	uint16_t *p, *n, ss;
	char ok;
	int i;

	p = *path;
	n = p + 1;
	ok = 0;

	while (*p != tosect) {
		if (gameMap.sectors()[*n].sector_type != S_DIRA && ISTELEPORT(gameMap.sectors()[*n].sector_type)) {
			for (i = 0; i < 4 && gameMap.sectors()[*p].step_next[i] != *n; i++) {
				if (i == 4) {
					ss = *p;
					free(*path);
					*path = NULL;
					return ss;
				}
			}

			if (!gameMap.sides()[(*p << 2) + i].flags & SD_PLAY_IMPS) {
				ok = 1;
			}
		}

		if (!ok) {
			ss = *p;
			free(*path);
			*path = NULL;
			najdi_cestu(*p, tosect, SD_PLAY_IMPS, path, 0);

			if (*path == NULL) {
				return ss;
			}

			p = *path;
			n = p + 1;
			a_touch(ss, i);
		}

		p++;
		n++;
		ok = 0;
	}

	free(*path);
	*path = NULL;
	return tosect;
}

static unsigned path_length(uint16_t *path) {
	unsigned i;
	for (i = 0; path[i]; i++);
	return i;
}

void recall()
  {
  int tosect,max,i,j;
  uint16_t *paths[POCET_POSTAV];

  for(i=0;i<POCET_POSTAV;i++)
     {
     if (postavy[i].groupnum==cur_group) tosect=postavy[i].sektor;
     paths[i]=NULL;
     }
  for(i=0;i<POCET_POSTAV;i++)
     if (postavy[i].sektor!=tosect) najdi_cestu(postavy[i].sektor,tosect,SD_PLAY_IMPS,&paths[i],0);
  do
     {
     max=0xffff;j=-1;
//     for(i=0;i<POCET_POSTAV;i++) if (paths[i]!=NULL && _msize(paths[i])<(unsigned)max)
     for(i=0;i<POCET_POSTAV;i++) if (paths[i]!=NULL && path_length(paths[i])<(unsigned)max)
                                      {
//                                      max=_msize(paths[i]);
                                      max=path_length(paths[i]);
                                      j=i;
                                      }
     if (j!=-1)
        {
        postavy[j].sektor=check_path(&paths[j],tosect);
        free(paths[j]);paths[j]=NULL;
        bott_draw(1);
        other_draw();
        showview(0,0,0,0);
        }
     }
   while (j!=-1);
  }

void key_break_sleep(EVENT_MSG *msg,void **unused) {
	if (msg->msg == E_KEYBOARD) {
		break_sleep = 1;
	}

	if (msg->msg == E_MOUSE) {
		MS_EVENT *ms;
		va_list args;

		va_copy(args, msg->data);
		ms = va_arg(args, MS_EVENT*);
		va_end(args);

		if (ms->event_type & (0x2 | 0x8 | 0x20)) {
			break_sleep = 1;
		}
	}
}

//void sleep_players(va_list args) {
void sleep_players(void) {
	int i;
	int hours=0;
	char reg;
	char enablity;
	
	if (!sleep_ticks) return;
	if (!GlobEvent(MAGLOB_STARTSLEEP,viewsector,viewdir)) return;

	enablity=enable_sound(0);
	mute_all_tracks(0);
	autosave();
	insleep=1;
	update_mysky();
	schovej_mysku();
	curcolor=0;bar(0,17,639,360+16);
	send_message(E_ADD,E_KEYBOARD,key_break_sleep);
	ukaz_mysku();
	showview(0,0,0,0);
	unwire_proc();
	break_sleep=0;

	while (sleep_ticks && !break_sleep) {
		reg=0;
		if (!(sleep_ticks%6)) {
			if ((reg=(sleep_ticks%HODINA==0))==1) {
				char s[50];
				for(i=0;i<POCET_POSTAV;i++) {
					break_sleep |= sleep_regenerace(&postavy[i]);
				}

				update_mysky();
				schovej_mysku();
				bott_draw(0);
				curcolor=0;bar(0,120,639,140);
				sprintf(s,texty[71],hours++);
				set_font(H_FBOLD,RGB555(31,31,0));
				set_aligned_position(320,130,1,1,s);outtext(s);
				other_draw();
				ukaz_mysku();
				showview(0,120,640,20);
				showview(0,378,640,102);
				Task_WaitEvent(E_TIMER);
				Task_WaitEvent(E_TIMER);
				Task_WaitEvent(E_TIMER);
				Task_WaitEvent(E_TIMER);
			}

			sleep_enemy(reg);
			check_players_place(0);
		}

		if (battle) {
			break_sleep|=1;
		}

		for(i=0;i<POCET_POSTAV;i++) {
			break_sleep |= check_jidlo_voda(&postavy[i]) | check_map_specials(&postavy[i]);
		}

		send_message(E_KOUZLO_KOLO);
		sleep_ticks--;
		tick_tack(1);
		if (!TimerEvents(viewsector,viewdir,game_time)) {
			break;
		}
	}

	send_message(E_DONE,E_KEYBOARD,key_break_sleep);
	wire_proc();
	bott_draw(1);
	insleep=0;
	enable_sound(enablity);
	GlobEvent(MAGLOB_ENDSLEEP,viewsector,viewdir);
}


void *game_keyboard(EVENT_MSG *msg, void **usr) {
	char c;
	
	if (pass_zavora) {
		return NULL;
	}

	if (cur_mode == MD_END_GAME) {
		return NULL;
	}

	if (msg->msg == E_KEYBOARD) {
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) >> 8;
		va_end(args);

		while (Input_Kbhit()) Input_ReadKey();
		switch (c) {
		case 'H':
			step_zoom(0);
			break;

		case 'P':
			step_zoom(2);
			break;

		case 'M':
			if (get_control_state()) {
				step_zoom(1);
			} else {
				turn_zoom(1);
			}
			break;

		case 'K':
			if (get_control_state()) {
				step_zoom(3);
			} else {
				turn_zoom(-1);
			}
			break;

		case 79:
		case 's':
			step_zoom(3);
			break;

		case 81:
		case 't':
			step_zoom(1);
			break;

		case 0x21:
			if (q_item(flute_item, viewsector)) {
				bott_draw_fletna();
			}
			// FIXME: should there be a break here?

		case 57:
			a_touch(viewsector,viewdir);
			if (cur_mode == MD_PRESUN) {
				send_message(E_KEYBOARD, 28*256);
			}
			break;

		case 15:
		case 50:
			if (GlobEvent(MAGLOB_BEFOREMAPOPEN, viewsector, viewdir)) {
				show_automap(1);
			}
			break;

		case 0x17:
			unwire_proc();
			wire_inv_mode(human_selected);
			break;

/*
		case 'A':
			lodka = !lodka;
			set_backgrnd_mode(lodka);
			break;
*/
		case 1:
			konec(0, 0, 0, 0, 0);
			break;

/*
		case 25:
			GamePause();
			break;
*/
		case 28:
			enforce_start_battle();
			break;

		case 82:
			group_all();
			break;

		case '<':
			if (!battle && GlobEvent(MAGLOB_CLICKSAVE, viewsector, viewdir)) {
				unwire_proc();
				cancel_render = 1;
				wire_save_load(1);
			}
			break;

		case '=':
			unwire_proc();
			cancel_render = 1;
			wire_save_load(0);
			break;

		case '>':
			game_setup(0, 0, 0, 0, 0);
			break;

		CASE_KEY_1_6:
			c = group_sort[c - 2];
			if (postavy[c].sektor == viewsector && postavy[c].used) {
				add_to_group(c);
			}
			zmen_skupinu(postavy+c);
			bott_draw(1);
			break;

/*
		default:
			{
				char s[20];
				bott_disp_text(itoa(c, s, 10));
			}
			break;
*/
		}
	}

	return (void*)&game_keyboard;
}


void start_dialog(int dialog,int mob)
  {
  if (battle) 
  {
    call_dialog(dialog,mob);
  }
  else
  {
    force_start_dialog=1;
    start_dialog_number=dialog;
    start_dialog_mob=mob;
  }
//  call_dialog(dialog,mob);
  }


int postavy_propadnout(int sector) {
	char redraw = 0;
	int i, z = sector;

	mute_hit_sound = 0;

	if (gameMap.coord()[sector].flags & MC_DPLAYER && gameMap.sectors()[sector].sector_type == S_DIRA) {
		for (i = 0; i < POCET_POSTAV; i++) {
			if (postavy[i].sektor == sector && postavy[i].used && (postavy[i].vlastnosti[VLS_KOUZLA] & SPL_LEVITATION) == 0) {
				int l = postavy[i].vlastnosti[VLS_MAXHIT];

				z = postavy[i].sektor = gameMap.sectors()[sector].sector_tag;

				if (postavy[i].groupnum == cur_group) {
					viewsector = z;
				}

				if (z) {
					player_hit(postavy + i, rnd(l / 2) + l / 3, 0); 
				} else {
					player_hit(postavy + i, postavy[i].lives, 0);
				}

				redraw = 1;
			}
		}

		if (redraw) {
			bott_draw(1);
		}
	}

	return z;
}
