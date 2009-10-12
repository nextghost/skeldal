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
#include "libs/bgraph.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include "libs/memman.h"
#include "libs/mgifmem.h"
#include "libs/sound.h"
#include "libs/system.h"
#include "libs/strlite.h"

static MGIF_HEADER_T mgif_header;

static short mgif_accnums[2];
static long mgif_writepos;

static uint16_t paleta[256];

static uint16_t *picture;
static uint16_t *anim_render_buffer;
static void *sound;

static void StretchImageHQ(uint16_t *src, uint16_t *trg, unsigned long linelen, char full) {
	uint16_t xs = src[0], ys = src[1];
	uint16_t *s, *t;
	int x, y;
	src += 3;  
	for (y = 0, s = src, t = trg; y < ys; y++, t += linelen * 2, s += xs) {
		for (x = 0; x < xs; x++) {
			uint16_t val;
//			t[x*2] = s[x] + (s[x] & 0x7fe0);
			t[x*2] = s[x];
			if (x) {
//				val = ((s[x-1] & 0x7bde) + (s[x] & 0x7bde)) >> 1;
//				t[x*2-1] = val + (val & 0x7fe0);
				t[x*2-1] = Screen_ColorAvg(s[x-1], s[x]);
			}
			if (full) {
				if (y) {
//					val = ((s[x-xs] & 0x7bde) + (s[x] & 0x7bde)) >> 1;
//					t[x*2-linelen] = val + (val & 0x7fe0);
					t[x*2-linelen] = Screen_ColorAvg(s[x-xs], s[x]);
				}
				if (x && y) {
//					val=((s[x-xs-1] & 0x7bde) + (s[x] & 0x7bde))>>1;
//					t[x*2-linelen-1] = val + (val & 0x7fe0);
					t[x*2-linelen-1] = Screen_ColorAvg(s[x-xs-1], s[x]);
				}
			}
		}
	}
}

void loadMgifHeader(MGIF_HEADER_T &header, ReadStream &stream) {
	int i;

	stream.read(header.sign, 4);
	stream.read(header.year, 2);
	header.eof = stream.readSint8();
	header.ver = stream.readUint16LE();
	header.frames = stream.readSint32LE();
	header.snd_chans = stream.readUint16LE();
	header.snd_freq = stream.readSint32LE();

	for (i = 0; i < 256; i++) {
		header.ampl_table[i] = stream.readSint16LE();
	}

	for (i = 0; i < 32; i++) {
		header.reserved[i] = stream.readSint16LE();
	}
}

static void PlayMGFFile(ReadStream &stream, MGIF_PROC proc, int ypos, char full) {
	mgif_install_proc(proc);
	sound = PrepareVideoSound(22050, 256 * 1024);
	mgif_accnums[0] = mgif_accnums[1] = 0;
	mgif_writepos = 65536;
	picture = new uint16_t[3 + 320 * 180];
	picture[0] = 320;
	picture[1] = 180;
	picture[2] = 15;
	memset(picture + 3, 0, 320 * 180 * 2);
	anim_render_buffer = picture + 3;
	loadMgifHeader(mgif_header, stream);

	if (!open_mgif(mgif_header)) {
		return;
	}

	while (mgif_play(stream)) {
		StretchImageHQ(picture, Screen_GetAddr() + ypos * Screen_GetXSize(), Screen_GetXSize(), full);
		showview(0, ypos, 0, 360);

		if (!Input_Kbhit()) {
			Sound_MixBack(0);
		} else {
			Input_ReadKey();
			break;
		}
	}

	close_mgif();  
	DoneVideoSound(sound);
	delete[] picture;
}

void show_full_lfb12e(uint16_t *target,uint8_t *buff,uint16_t *paleta);
void show_delta_lfb12e(uint16_t *target,uint8_t *buff,uint16_t *paleta);
void show_delta_lfb12e_dx(void *target,void *buff,void *paleta);
void show_full_lfb12e_dx(void *target,void *buff,void *paleta);


void BigPlayProc(int act, void *data, int csize) {
	switch (act) {
	case MGIF_LZW:
	case MGIF_COPY:
		show_full_lfb12e(anim_render_buffer, (uint8_t*)data, paleta);
		break;

	case MGIF_DELTA:
		show_delta_lfb12e(anim_render_buffer, (uint8_t*)data, paleta);
		break;

	case MGIF_PAL:
		memcpy(paleta, data, csize);
		Screen_FixMGIFPalette(paleta, csize / sizeof(uint16_t));
		break;

	case MGIF_SOUND: 
		while (LoadNextVideoFrame(sound, (uint8_t*)data, csize, mgif_header.ampl_table, mgif_accnums, &mgif_writepos) == 0) {
			Timer_Sleep(5);
		}
	}
}

void play_animation(char *filename, char mode, int posy, char sound) {
	File mgf(filename);

	Sound_ChangeMusic(NULL);

	if (!mgf.isOpen()) {
		return;
	}

	PlayMGFFile(mgf, BigPlayProc, posy, mode & 0x80);
}

void set_title_list(const StringList *titles)
  {

  }
void set_play_attribs(void *screen,char rdraw,char bm,char colr64)
  {

  }
