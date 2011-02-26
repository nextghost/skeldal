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
#ifndef _PCX_H_
#define _PCX_H_

#include <inttypes.h>
#include "libs/memman.h"

#define SHADE_STEPS 5
#define PAL_SIZE 768

typedef uint8_t pal_t[PAL_SIZE];
typedef pal_t fadepal_t[2 * SHADE_STEPS];

// simple base class and pixel array wrapper
class Texture : public DataBlock {
private:
	unsigned _width, _height, _depth;

protected:
	uint8_t *_pixels;

	Texture(void) {}
	void setData(uint8_t *data, unsigned xs, unsigned ys, unsigned bpp);

	// do not implement
	Texture(const Texture &src);
	const Texture &operator=(const Texture &src);

public:
	Texture(uint8_t *data, unsigned xs, unsigned ys, unsigned bpp);
	virtual ~Texture(void) {}

	const uint8_t *pixels(void) const { return _pixels; }
	virtual const uint8_t *palette(unsigned fade = 0) const { return NULL; }
	unsigned width(void) const { return _width; }
	unsigned height(void) const { return _height; }
	unsigned depth(void) const { return _depth; }
	unsigned memsize(void) const { return _width * _height * _depth * sizeof(uint8_t) + sizeof(*this); }
};

class TextureHi : public Texture {
public:
	// turn raw pixeldata to TextureHi
	TextureHi(const uint8_t *data, unsigned xs, unsigned ys);
	// expand 256 color image from palette
	TextureHi(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys);
	// load raw RGB555 from file
	explicit TextureHi(ReadStream &stream);
	~TextureHi(void);
	unsigned memsize(void) const { return Texture::memsize() + sizeof(*this) - sizeof(Texture); }
};

class TexturePal : public Texture {
private:
	uint8_t *_pal;

public:
	TexturePal(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys);
	explicit TexturePal(ReadStream &stream);
	~TexturePal(void);

	const uint8_t *palette(unsigned fade = 0) const { return _pal; }
	unsigned memsize(void) const { return Texture::memsize() + sizeof(*this) - sizeof(Texture) + _pal ? PAL_SIZE * sizeof(uint8_t) : 0; }
};

class TextureFade : public Texture {
protected:
	fadepal_t _pal;

	TextureFade(const uint8_t *pal, unsigned xs, unsigned ys, uint8_t r, uint8_t g, uint8_t b);

public:
	TextureFade(const uint8_t *data, const uint8_t *pal, unsigned xs, unsigned ys, int tr, int tg, int tb);
	~TextureFade(void);

	const uint8_t *palette(unsigned fade = 0) const;
	const pal_t *fadePal(void) const { return _pal; }
	unsigned memsize(void) const { return Texture::memsize() + sizeof(*this) - sizeof(Texture); }
};

// Subimage of another texture, does NOT support fade palette
class SubTexture : public Texture {
private:
	uint8_t *_pal;

public:
	SubTexture(const Texture &src, unsigned x, unsigned y, unsigned xs, unsigned ys);
	~SubTexture(void);

	const uint8_t *palette(unsigned fade = 0) const { return _pal; }
	unsigned memsize(void) const { return Texture::memsize() + sizeof(*this) - sizeof(Texture) + _pal ? PAL_SIZE * sizeof(uint8_t) : 0; }
};

#define A_8BIT 8
#define A_16BIT 16
#define A_FADE_PAL (256+8)
#define A_8BIT_NOPAL (512+8)
#define A_NORMAL_PAL (768+8)

typedef struct pcxrecord {
	uint16_t id;
	int8_t encoding;
	int8_t bitperpixel;
	uint16_t xmin, ymin, xmax, ymax;
	uint16_t hdpi, vdpi;
	int8_t colormap[48];
	int8_t reserved;
	int8_t mplanes;
	uint16_t bytesperline;
	uint16_t paleteinfo;
	uint16_t hscreen, vscreen;
	int8_t filler[54];
} PCXHEADER;


Texture *load_pcx(SeekableReadStream &stream, int conv_type, uint8_t tr = 0, uint8_t tg = 0, uint8_t tb = 0);
Texture *open_pcx(const char *filename, int type, uint8_t tr = 0, uint8_t tg = 0, uint8_t tb = 0);
void palette_shadow(pal_t *pal, int tr, int tg, int tb);
//extern void *get_palette_ptr;

#endif
