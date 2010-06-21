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
//!!!! POZOR, NUTNE LINKOVAT SOUBOR LZWA.ASM
#ifndef _MGIFMEM_H
#define _MGIFMEM_H

#include <inttypes.h>
#include "libs/pcx.h"

#pragma pack(1)
typedef void (*MGIF_PROC)(int, SeekableReadStream &); //prvni cislo akce, druhy data akce

#define MGIF "MGIF"
#define MGIF_Y "97"
#define VER 0x100
#define MGIF_EMPTY  0
#define MGIF_LZW    1
#define MGIF_DELTA  2
#define MGIF_PAL    3
#define MGIF_SOUND  4
#define MGIF_TEXT   5
#define MGIF_COPY   6
#define MGIF_SINIT  7

#define SMD_256 1
#define SMD_HICOLOR 2

typedef struct mgif_header {
	char sign[4];
	char year[2];
	int8_t eof;
	uint16_t ver;
	int32_t frames;
	uint16_t snd_chans;
	int32_t snd_freq;
	int16_t ampl_table[256];
	int16_t reserved[32];
} MGIF_HEADER_T;

#define FLAG_PLAYING 0x1
#define FLAG_VIDEO 0x2
#define FLAG_AUDIO 0x4
#define FLAG_TEXT 0x8

class MGIFReader : public Texture {
private:
	ReadStream &_stream;
	MGIF_HEADER_T _header;
	uint8_t _pal[4 * 256];
	char *_text;
	int16_t *_audio, accnums[2];
	unsigned _textSize, _audioSize, _audioLength, _frame, _forceAlpha;

public:
	MGIFReader(ReadStream &stream, unsigned width, unsigned height, unsigned forceAlpha = 0);
	~MGIFReader(void);

	unsigned decodeFrame(void);
	const char *getText(void) const { return _text; }
	const int16_t *getAudio(void) const { return _audio; }
	unsigned getAudioSize(void) const { return _audioLength; }
};

void mgif_install_proc(MGIF_PROC proc);
int open_mgif(const mgif_header &mgh);
int mgif_play(ReadStream &stream);
void close_mgif();           //dealokuje buffery pro prehravani
void loadMgifHeader(MGIF_HEADER_T &header, ReadStream &stream);

void show_full_lfb12e(uint16_t *target, ReadStream &stream, uint16_t *paleta);
void show_delta_lfb12e(uint16_t *target, ReadStream &stream, uint16_t *paleta);

#pragma option align=reset
#endif
