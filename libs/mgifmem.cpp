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
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include "libs/memman.h"
#include "libs/mgifmem.h"

#define MGIF "MGIF"
#define LZW_MAX_CODES  16384
#define LZW_BUFFER 64000

MGIF_PROC show_proc;
static int mgif_frames;
static int cur_frame;


typedef struct double_s
  {
  int16_t group,chr,first,next;
  }DOUBLE_S;

typedef DOUBLE_S CODE_TABLE[LZW_MAX_CODES];

DOUBLE_S *compress_dic=NULL;
static void *lzw_buffer=NULL;
static int clear_code;
static int end_code;
static int free_code;
static int nextgroup;
static int bitsize,init_bitsize;
char old_value=0;

void do_clear_code()
  {
  int i;

  old_value=0;
  nextgroup=free_code;
  bitsize=init_bitsize;
  for(i=0;i<clear_code;i++)
     {
     DOUBLE_S *p;
     p=&compress_dic[i];
     p->group=i;p->chr=-1;p->next=-1;p->first=-1;
     }
  }

void reinit_lzw()
  {
  do_clear_code();
  }

void init_lzw_compressor(int dic_size)
  {

	if (!compress_dic) {
		compress_dic = (DOUBLE_S *)getmem(sizeof(CODE_TABLE));
	}
  clear_code=1<<dic_size;
  end_code=clear_code+1;
  free_code=end_code+1;
  nextgroup=free_code;
  init_bitsize=bitsize=dic_size+1;
  do_clear_code();
  }


void done_lzw_compressor()
  {
  free(compress_dic);
  compress_dic=NULL;
  }

unsigned input_code(uint8_t *source,long *bitepos,int bitsize,int mask) {
	unsigned ret = *(unsigned *)(source + (*bitepos >> 3));
	ret >>= *bitepos & 0x7;
	*bitepos += bitsize;
	return ret & mask;
}


void de_add_code(int group, int chr) {
	DOUBLE_S *q;

	q = &compress_dic[nextgroup];
	q->group = group;
	q->chr = chr;
	q->first = compress_dic[group].first + 1;
	nextgroup++;

	if (nextgroup >= (1 << bitsize) - 1) {
		bitsize++;
	}
}


char fast_expand_code(int code, uint8_t **target) {
	if (code < 256) {
		old_value = **target = (code + old_value) & 0xff;
		++*target;
		return code;
	}

	*target += compress_dic[code].first;
	char ret;
	uint8_t *ptr = *target;
	int idx = code;

	do {
		*ptr-- = compress_dic[idx].chr & 0xff;
	} while ((idx = compress_dic[idx].group) >= 256);

	ret = idx;
	idx = (idx + old_value) & 0xff;
	*ptr++ = idx;
	code = compress_dic[code].first;

	do {
		idx = (*ptr + idx) & 0xff;
		*ptr++ = idx;
	} while (--code);

	old_value = idx;
	++*target;
	return ret;
}

void lzw_decode(ReadStream &source, uint8_t *target) {
	register int code;
	int old, i;
	//int group,chr;
	int old_first;
	BitStream bstream(source);

	for (i = 0; i < LZW_MAX_CODES; i++) {
		compress_dic[i].first = 0;
	}

clear:
	old_value = 0;
	nextgroup = free_code;
	bitsize = init_bitsize;
	code = bstream.readBitsLE(bitsize);
	old_first = fast_expand_code(code, &target);
//  old_first=expand_code(code,&target);
	old = code;

	while ((code = bstream.readBitsLE(bitsize)) != end_code) {
		if (code == clear_code) {
			goto clear;
		} else if (code < nextgroup) {
			old_first = fast_expand_code(code, &target);
//        old_first=expand_code(code,&target);
			//group=old;
			//chr=old_first;
			de_add_code(old, old_first);
			old = code;
		} else {
			//p.group=old;
			//p.chr=old_first;
			de_add_code(old, old_first);
			old_first = fast_expand_code(code, &target);
//        old_first=expand_code(code,&target);
			old = code;
		}
	}
}

//dekoduje a zobrazi frame
int mgif_play(ReadStream &stream) {
	uint8_t *pc, *ff;
	unsigned tmp;
	int acts, size, act, csize;
	MemoryReadStream *scr_sav = NULL;
	int scr_act = -1;
	
	tmp = stream.readUint32LE();
	acts = tmp & 0xff;
	size = tmp >> 8;

	for (pc = NULL; acts; acts--) {
		tmp = stream.readUint32LE();
		act = tmp & 0xff;
		csize = tmp >> 8;

		if ((act == MGIF_LZW) || (act == MGIF_DELTA)) {
			MemoryReadStream *tmp = stream.readStream(csize);
			ff = (uint8_t*)lzw_buffer;
			lzw_decode(*tmp, ff);
			delete[] scr_sav;
			scr_sav = new MemoryReadStream(ff, LZW_BUFFER);
			scr_act = act;
			delete tmp;
		} else if (act == MGIF_COPY) {
			delete[] scr_sav;
			scr_sav = new MemoryReadStream(ff, csize);
			scr_act = act;
		} else {
			delete[] pc;
			pc = new uint8_t[csize];
			stream.read(pc, csize);
			ff = pc;
			MemoryReadStream tmpstream(ff, csize);
			show_proc(act, tmpstream);
		}
	}

	if (scr_act != -1) {
		show_proc(scr_act, *scr_sav);
	}

	delete scr_sav;
	delete[] pc;
	cur_frame += 1;

	if (cur_frame == mgif_frames) {
		return 0;
	}

	return 1;
}

void apply_delta(uint8_t *target, ReadStream &stream, const uint8_t *palette) {
	unsigned int i, j, k, size, idx;
	uint8_t *tmp, *map, *oldmap;

	size = stream.readUint32LE();
	oldmap = map = new uint8_t[size+1];

	for (i = 0; i < size; i++) {
		map[i] = stream.readUint8();
	}

	for (i = 0; i < 180; i += j) {
		tmp = target;

		for (j = *map++; (j & 0xc0) != 0xc0; j = *map++) {
			target += 8 * j;

			for (k = 0; k < *map; k++) {
				idx = stream.readUint8();
				*target++ = palette[4 * idx];
				*target++ = palette[4 * idx + 1];
				*target++ = palette[4 * idx + 2];
				*target++ = palette[4 * idx + 3];
				idx = stream.readUint8();
				*target++ = palette[4 * idx];
				*target++ = palette[4 * idx + 1];
				*target++ = palette[4 * idx + 2];
				*target++ = palette[4 * idx + 3];
			}
			map++;
		}

		j = (j & 0x3f) + 1;
		target = tmp + j * 1280;
	}

	delete[] oldmap;
}

MGIFReader::MGIFReader(ReadStream &stream, unsigned width, unsigned height, unsigned forceAlpha) :
	Texture(new uint8_t[4 * width * height], width, height, 4),
	_stream(stream), _text(NULL), _audio(NULL), _textSize(0), _audioSize(0),
	_audioLength(0), _frame(0), _forceAlpha(forceAlpha) {
	int i;

	accnums[0] = accnums[1] = 0;
	memset(_pal, 0, 4 * 256 * sizeof(uint8_t));
	memset(_pixels, 0, 4 * width * height * sizeof(uint8_t));

	_stream.read(_header.sign, 4);
	_stream.read(_header.year, 2);
	_header.eof = _stream.readSint8();
	_header.ver = _stream.readUint16LE();
	_header.frames = _stream.readSint32LE();
	_header.snd_chans = _stream.readUint16LE();
	_header.snd_freq = _stream.readSint32LE();

	for (i = 0; i < 256; i++) {
		_header.ampl_table[i] = _stream.readSint16LE();
	}

	for (i = 0; i < 32; i++) {
		_header.reserved[i] = _stream.readSint16LE();
	}

	// FIXME: rewrite to use decompressor class
	init_lzw_compressor(8);
}

MGIFReader::~MGIFReader(void) {
	delete[] _pixels;
	delete[] _text;
	delete[] _audio;
}

unsigned MGIFReader::decodeFrame(void) {
	int acts, action, size, i, j, val;
	unsigned tmp, ret = FLAG_PLAYING;
	uint8_t *buf = NULL;
	MemoryReadStream *tmpstream, *delta = NULL;

	if (_frame++ >= _header.frames) {
		return 0;
	}

	// frame header: 1st byte = actions count, remaining 3 bytes = total
	// frame size
	acts = _stream.readUint32LE() & 0xff;

	for (i = 0; i < acts; i++) {
		tmp = _stream.readUint32LE();
		action = tmp & 0xff;
		size = tmp >> 8;
		tmpstream = _stream.readStream(size);

		switch (action) {
		case MGIF_LZW:
			assert(!(ret & FLAG_VIDEO) && "Multiple video actions");
			buf = new uint8_t[LZW_BUFFER];
			lzw_decode(*tmpstream, buf);

			for (j = 0; j < width() * height(); j++) {
				_pixels[4 * j] = _pal[4 * buf[j]];
				_pixels[4 * j + 1] = _pal[4 * buf[j] + 1];
				_pixels[4 * j + 2] = _pal[4 * buf[j] + 2];
				_pixels[4 * j + 3] = _pal[4 * buf[j] + 3];
			}

			delete[] buf;
			ret |= FLAG_VIDEO;
			break;

		case MGIF_DELTA:
			assert(!(ret & FLAG_VIDEO) && "Multiple video actions");
			buf = new uint8_t[LZW_BUFFER];
			lzw_decode(*tmpstream, buf);
			delta = new MemoryReadStream(buf, LZW_BUFFER);
			delete[] buf;
			ret |= FLAG_VIDEO;
			break;

		case MGIF_PAL:
			for (j = 0, tmp = tmpstream->readUint16LE(); j < 4 * 256 && !tmpstream->eos(); j += 4, tmp = tmpstream->readUint16LE()) {
				_pal[j] = (tmp >> 15) & 1;
				_pal[j + 1] = ((tmp >> 7) & 0xf1) | ((tmp >> 12) & 0x7);
				_pal[j + 2] = ((tmp >> 2) & 0xf1) | ((tmp >> 7) & 0x7);
				_pal[j + 3] = ((tmp << 3) & 0xf1) | ((tmp >> 2) & 0x7);
			}

			if (_forceAlpha) {
				_pal[0] = 1;
			}

			break;

		case MGIF_SOUND:
			if (_audioSize < size) {
				delete[] _audio;
				_audio = new int16_t[size];
				_audioSize = size;
			}

			_audioLength = size;

			for (j = 0; j < size; j++) {
				val = accnums[j % 2] + _header.ampl_table[tmpstream->readUint8()];
				val = val > 32767 ? 32767 : val;
				val = val < -32767 ? -32767 : val;
				_audio[j] = accnums[j % 2] = val;
			}

			ret |= FLAG_AUDIO;
			break;

		case MGIF_COPY:
			assert(!(ret & FLAG_VIDEO) && "Multiple video actions");
			ret |= FLAG_VIDEO;
			break;

		default:
			assert(0 && "Unsupported frame action");
		}

		delete tmpstream;
	}

	if (delta) {
		apply_delta(_pixels, *delta, _pal);
		delete delta;
	}

	return ret;
}
