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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "libs/pcx.h"
#include "libs/memman.h"
#include "libs/system.h"

#define SHADE_PAL (SHADE_STEPS*512*2)

//void *get_palette_ptr=NULL;

Texture::Texture(uint8_t *data, unsigned xs, unsigned ys, unsigned bpp) :
	_pixels(data), _width(xs), _height(ys), _depth(bpp) {
	assert(_pixels);
}

void Texture::setData(uint8_t *data, unsigned xs, unsigned ys, unsigned bpp) {
	_pixels = data;
	_width = xs;
	_height = ys;
	_depth = bpp;
}

TextureHi::TextureHi(const uint8_t *data, unsigned xs, unsigned ys) :
	Texture(new uint8_t[xs * ys * 3], xs, ys, 3) {

	memcpy(_pixels, data, xs * ys * 3 * sizeof(uint8_t));
}

TextureHi::TextureHi(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys) :
	Texture(new uint8_t[xs * ys * 3], xs, ys, 3) {

	for (int i = 0; i < xs * ys; i++) {
		_pixels[3 * i] = pal[3 * data[i]];
		_pixels[3 * i + 1] = pal[3 * data[i] + 1];
		_pixels[3 * i + 2] = pal[3 * data[i] + 2];
	}
}

TextureHi::TextureHi(ReadStream &stream) : Texture() {
	unsigned xs, ys;
	uint16_t src;

	xs = stream.readUint16LE();
	ys = stream.readUint16LE();
	assert(stream.readUint16LE() == 15 && "Only RGB555 raw image supported");
	setData(new uint8_t[xs * ys * 3], xs, ys, 3);

	for (int i = 0; i < xs * ys; i++) {
		src = stream.readUint16LE();
		_pixels[3 * i] = (src & 0x7c00) >> 7;
		_pixels[3 * i + 1] = (src & 0x3e0) >> 2;
		_pixels[3 * i + 2] = (src & 0x1f) << 3;
	}
}

TextureHi::~TextureHi(void) {
	delete[] _pixels;
}

TexturePal::TexturePal(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys) :
	Texture(new uint8_t[xs * ys], xs, ys, 1), _pal(NULL) {

	memcpy(_pixels, data, xs * ys * sizeof(uint8_t));

	if (pal) {
		_pal = new uint8_t[PAL_SIZE];
		memcpy(_pal, pal, PAL_SIZE * sizeof(uint8_t));
	}
}

TexturePal::TexturePal(ReadStream &stream) : Texture(),
	_pal(new uint8_t[PAL_SIZE]) {
	unsigned xs, ys;
	uint16_t src;
	int i;

	xs = stream.readUint16LE();
	ys = stream.readUint16LE();
	assert(stream.readUint16LE() == 8 && "Only 8bit raw image supported");
	setData(new uint8_t[xs * ys], xs, ys, 1);

	for (i = 0; i < 256; i++) {
		src = stream.readUint16LE();
		_pal[3 * i] = (src & 0x7c00) >> 7;
		_pal[3 * i + 1] = (src & 0x3e0) >> 2;
		_pal[3 * i + 2] = (src & 0x1f) << 3;
	}

	for (i = 0; i < xs * ys; i++) {
		_pixels[i] = stream.readUint8();
	}
}

TexturePal::~TexturePal(void) {
	delete[] _pixels;
	delete[] _pal;
}

TextureFade::TextureFade(const uint8_t *pal, unsigned xs, unsigned ys, uint8_t r, uint8_t g, uint8_t b)
	: Texture(new uint8_t[xs * ys], xs, ys, 1) {

	memset(_pixels, 0, xs * ys * sizeof(uint8_t));
	memcpy(_pal[0], pal, PAL_SIZE * sizeof(uint8_t));
	palette_shadow(_pal, r, g, b);
}
	
TextureFade::TextureFade(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys, int tr, int tg, int tb)
	: Texture(new uint8_t[xs * ys], xs, ys, 1) {
	int i, j;

	assert(pal);
	memcpy(_pixels, data, xs * ys * sizeof(uint8_t));
	memcpy(_pal[0], pal, PAL_SIZE * sizeof(uint8_t));
	palette_shadow(_pal, tr, tg, tb);
}

TextureFade::~TextureFade(void) {
	delete[] _pixels;
}

const uint8_t *TextureFade::palette(unsigned fade) const {
	assert(fade < 2 * SHADE_STEPS);

	return _pal[fade];
}

SubTexture::SubTexture(const Texture &src, unsigned x, unsigned y, unsigned xs,
	unsigned ys) : Texture(), _pal(NULL) {
	int i, tx, ty;

	tx = min(xs, src.width() - x);
	ty = min(ys, src.height() - y);

	assert(tx > 0 && ty > 0 && "Invalid subtexture coordinates");

	setData(new uint8_t[tx * ty * src.depth()], tx, ty, src.depth());

	for (i = 0; i < ty; i++) {
		memcpy(_pixels + i * tx * src.depth(), src.pixels() + src.depth() * (x + (y + i) * src.width()), tx * src.depth() * sizeof(uint8_t));
	}

	if (src.palette()) {
		_pal = new uint8_t[PAL_SIZE];
		memcpy(_pal, src.palette(), PAL_SIZE * sizeof(uint8_t));
	}
}

SubTexture::~SubTexture(void) {
	delete[] _pixels;
	delete[] _pal;
}

void palette_shadow(pal_t *pal, int tr, int tg, int tb) {
	int i, j;

	for (i = 1; i < SHADE_STEPS; i++) {
		for (j = 0; j < PAL_SIZE / 3; j++) {
			pal[i][3 * j] = tr + (pal[0][3 * j] - tr) * (3 * (SHADE_STEPS - i) - 1) / (3 * SHADE_STEPS - 1);
			pal[i][3 * j + 1] = tg + (pal[0][3 * j + 1] - tg) * (3 * (SHADE_STEPS - i) - 1) / (3 * SHADE_STEPS - 1);
			pal[i][3 * j + 2] = tb + (pal[0][3 * j + 2] - tb) * (3 * (SHADE_STEPS - i) - 1) / (3 * SHADE_STEPS - 1);
		}
	}

	for (; i < 2 * SHADE_STEPS; i++) {
		for (j = 0; j < PAL_SIZE; j++) {
			pal[i][j] = (unsigned)pal[0][j] * (2 * SHADE_STEPS - i) / SHADE_STEPS;
		}
	}
}

Texture *load_pcx(SeekableReadStream &stream, int conv_type, uint8_t tr, uint8_t tg, uint8_t tb) {
	int i, j;
	PCXHEADER pcxdata;
	int xsize, ysize;
	uint8_t tmp, palette[3*256], *data;
	Texture *ret;

	pcxdata.id = stream.readUint16LE();
	pcxdata.encoding = stream.readUint8();
	pcxdata.bitperpixel = stream.readUint8();
	pcxdata.xmin = stream.readUint16LE();
	pcxdata.ymin = stream.readUint16LE();
	pcxdata.xmax = stream.readUint16LE();
	pcxdata.ymax = stream.readUint16LE();
	pcxdata.hdpi = stream.readUint16LE();
	pcxdata.vdpi = stream.readUint16LE();

	for (i = 0; i < 48; i++) {
		pcxdata.colormap[i] = stream.readUint8();
	}

	pcxdata.reserved = stream.readUint8();
	pcxdata.mplanes = stream.readUint8();
	pcxdata.bytesperline = stream.readUint16LE();
	pcxdata.paleteinfo = stream.readUint16LE();
	pcxdata.hscreen = stream.readUint16LE();
	pcxdata.vscreen = stream.readUint16LE();

	for (i = 0; i < 54; i++) {
		pcxdata.filler[i] = stream.readUint8();
	}

	xsize = pcxdata.xmax - pcxdata.xmin + 1;
	ysize = pcxdata.ymax - pcxdata.ymin + 1;
	data = new uint8_t[pcxdata.bytesperline * ysize];

	for (i = 0; i < ysize; i++) {
		for (j = 0; j < pcxdata.bytesperline; j++) {
			tmp = stream.readUint8();

			// really i*xsize to get rid of padding bytes
			if (tmp >= 0xc0) {
				memset(data + i * xsize + j, stream.readUint8(), tmp - 0xc0);
				j += tmp - 0xc1; // fix j++
			} else {
				data[i * xsize + j] = tmp;
			}
		}
	}

	tmp = stream.readUint8();

	// strip trailing blank pixels
	while (!stream.eos() && !tmp) {
		stream.readUint8();
		tmp = stream.readUint8();
	}

	if (!stream.eos() && tmp == 0xc) {
		for (i = 0; i < 3 * 256; i++) {
			palette[i] = stream.readUint8();
		}
	} else {
		tmp = 0;
	}

	switch (conv_type) {
	case A_8BIT:
	case A_NORMAL_PAL:
		ret = new TexturePal(data, tmp ? palette : NULL, xsize, ysize);
		break;

	case A_8BIT_NOPAL:
		ret = new TexturePal(data, NULL, xsize, ysize);
		break;

	case A_16BIT:
		ret = new TextureHi(data, tmp ? palette : NULL, xsize, ysize);
		break;

	case A_FADE_PAL:
		ret = new TextureFade(data, tmp ? palette : NULL, xsize, ysize, tr, tg, tb);
		break;
	}

	delete[] data;
	return ret;
}

Texture *open_pcx(const char *filename, int type, uint8_t tr, uint8_t tg, uint8_t tb) {
	File file(filename);

	if (!file.isOpen()) {
		return NULL;
	}

	return load_pcx(file, type);
}

/*void initmode32b();

main()
  {
  int8_t *buf;

  initmode32b();
  open_pcx("DESK.pcx",A_8BIT,&buf,0,0,0);
  put_picture(0,480-102,buf);
  showview(0,0,0,0);
  getchar();
  return 0;
  }


*/

