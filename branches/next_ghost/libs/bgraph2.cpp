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
#include <inttypes.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "libs/bgraph.h"
#include "libs/memman.h"

//uint16_t *screen;
//uint16_t curcolor,charcolors[7] = {0x0000,RGB555(0,31,0),RGB555(0,28,0),RGB555(0,24,0),RGB555(0,20,0),0x0000,0x0000};
uint8_t curcolor[3];

//long scr_linelen;
//long scr_linelen2;
//long dx_linelen;
uint16_t *curfont,*writepos,writeposx;
uint8_t *palmem=NULL,*xlatmem=NULL;
void (*showview)(uint16_t,uint16_t,uint16_t,uint16_t);
char line480=0;
char banking=0;
char screenstate=0;
char __skip_change_line_test=0;
char no_restore_mode=0;

char write_window=0;
long pictlen; // Tato promenna je pouze pouzita v BGRAPH1.ASM

SoftRenderer *renderer = NULL, *backbuffer = NULL;

void text_mode();

void wait_retrace();

Font::Font(SeekableReadStream &stream) {
	int i, j, pix, bufsize = 0;
	long pos;
	unsigned seek;
	uint8_t *buf = NULL;

	for (i = 0; i < GLYPH_COUNT; i++) {
		seek = stream.readUint16LE();
		pos = stream.pos();
		stream.seek(seek, SEEK_SET);
		_glyphs[i].width = stream.readUint8();
		_glyphs[i].height = stream.readUint8();

		if (bufsize < _glyphs[i].width * _glyphs[i].height) {
			delete[] buf;
			buf = new uint8_t[_glyphs[i].width * _glyphs[i].height];
		}

		for (j = 0, pix = 0; pix < _glyphs[i].width * _glyphs[i].height; j++) {
			buf[j] = stream.readUint8();

			// end of glyph mark
			if (buf[j] == 0xff) {
				j++;
				break;
			// blank mark
			} else if (buf[j] >= 8) {
				pix += buf[j] - 6;
			// color pixel
			} else {
				pix++;
			}
		}

		if (_glyphs[i].width * _glyphs[i].height == 0) {
			_glyphs[i].data = new uint8_t[1];
			_glyphs[i].data[0] = 0xff;
		} else {
			_glyphs[i].data = new uint8_t[j];
			memcpy(_glyphs[i].data, buf, j * sizeof(uint8_t));
		}

		stream.seek(pos, SEEK_SET);
	}

	delete[] buf;
}

Font::~Font(void) {
	for (int i = 0; i < GLYPH_COUNT; i++) {
		delete[] _glyphs[i].data;
	}
}

const Font::Glyph &Font::glyph(unsigned idx) const {
	assert(idx < GLYPH_COUNT && "Invalid glyph");
	return _glyphs[idx];
}

unsigned Font::textWidth(const char *text) const {
	unsigned i, sum;

	for (i = 0, sum = 0; text[i]; i++) {
		sum += glyph((unsigned char)text[i]).width;
	}

	return sum;
}

unsigned Font::textHeight(const char *text) const {
	unsigned i, max, tmp;

	for (i = 0, max = 0; text[i]; i++) {
		tmp = glyph((unsigned char)text[i]).height;
		max = tmp > max ? tmp : max;
	}

	return max;
}

size_t Font::memsize(void) const {
	unsigned i, ret;

	for (i = 0, ret = 0; i < GLYPH_COUNT; i++) {
		if (_glyphs[i].width * _glyphs[i].height) {
			ret += _glyphs[i].width * _glyphs[i].height;
		} else {
			ret += 1;
		}
	}

	return ret + sizeof(*this);
}

SoftRenderer::SoftRenderer(unsigned xs, unsigned ys) :
	Texture(new uint8_t[xs * ys * 3], xs, ys, 3), _font(NULL) {

	memset(_pixels, 0, xs * ys * 3 * sizeof(uint8_t));
	memset(_fontPal, 0, sizeof(_fontPal));
	// initial charcolors non-zero values
	_fontPal[1][1] = 255;
	_fontPal[2][1] = 231;
	_fontPal[3][1] = 198;
	_fontPal[4][1] = 165;
}

SoftRenderer::~SoftRenderer(void) {
	delete[] _pixels;
}

void SoftRenderer::blit(const Texture &tex, int x, int y, const uint8_t *pal, unsigned scale, int mirror) {
	int i, j, tx, ty, basex = 0, basey = 0, tdepth = tex.depth();
	int twidth = tex.width();
	uint8_t *dst;
	const uint8_t *src, *basesrc;
	int *transtab;

	if (x < 0) {
		basex = -x;
	}

	if (y < 0) {
		basey = -y;
	}

	if (tdepth != 3 && !pal) {
		pal = tex.palette();
	}

	assert(tdepth == 3 || tdepth == 4 || (tdepth == 1 && pal));

	transtab = new int[width() + basex];

	for (i = 0; i < width() + basex; i++) {
		transtab[i] = i * 320 / scale;
	}

	for (i = basey; i + y < height(); i++) {
		ty = i *  320 / scale;

		if (ty >= tex.height()) {
			break;
		}

		dst = _pixels + 3 * (basex + x + (i + y) * width());

		if (tdepth == 3) {
			basesrc = tex.pixels() + 3 * ty * twidth;
		} else if (tdepth == 4) {
			basesrc = tex.pixels() + 4 * ty * twidth + 1;
		} else {
			basesrc = tex.pixels() + ty * twidth;
		}

		for (j = basex; j + x < width(); j++) {
			tx = transtab[j];

			if (tx >= twidth) {
				break;
			}

			if (mirror) {
				tx = twidth - tx;
			}

			if (tdepth == 3) {
				src = basesrc + 3 * tx;
			} else if (tdepth == 4) {
				src = basesrc + 4 * tx;

				// alpha channel > 0 => transparent
				if (src[-1]) {
					dst += 3;
					continue;
				}
			} else if (!basesrc[tx]) {
				dst += 3;
				continue;
			} else {
				src = pal + 3 * basesrc[tx];
			}

			*dst++ = src[0];
			*dst++ = src[1];
			*dst++ = src[2];
		}
	}

	delete[] transtab;
}

void SoftRenderer::rotBlit(const Texture &tex, int x, int y, float angle, const uint8_t *pal) {
	float sine = sin(angle), cosine = cos(angle);
	int i, j, sx, sy, tx, ty, basex, basey;
	uint8_t *dst;
	const uint8_t *src;

	basex = x >= 0 ? 0 : -x;
	basey = y >= 0 ? 0 : -y;

	if (tex.depth() != 3 && !pal) {
		pal = tex.palette();
	}

	for (i = basey; i + y < height(); i++) {
		ty = i - tex.height() / 2;
		dst = _pixels + 3 * (x + 2 * basex + (i + y) * width());

		for (j = basex; j + x + 2 * basex < width(); j++) {
			tx = j - tex.width() / 2;
			sx = tx * cosine - ty * sine + tex.width() / 2;
			sy = tx * sine + ty * cosine + tex.height() / 2;

			if (sx < 0 || sx >= tex.width() || sy < 0 || sy >= tex.height()) {
				*dst++ = 0;
				*dst++ = 0;
				*dst++ = 0;
				continue;
			}

			if (tex.depth() == 3) {
				src = tex.pixels() + 3 * (sx + sy * tex.width());
			} else {
				src = pal + 3 * tex.pixels()[sx + sy * tex.width()];
			}

			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
	}
}

void SoftRenderer::enemyBlit(const Texture &tex, int x, int y, const uint8_t *pal, unsigned scale, int mirror) {
	int i, j, tx, ty, basey = 0, basex = 0, tmp;
	unsigned xs = tex.width() * scale / 320;
	uint8_t *dst;
	const uint8_t *src;

	y -= tex.height() * scale / 320;

	if (y < 0) {
		basey = -y;
	}

	if (xs * 320 / scale >= tex.width()) {
		xs--;
	}

	if (tex.depth() != 3 && !pal) {
		pal = tex.palette();
	}

	assert(tex.depth() == 3 || (tex.depth() == 1 && pal));

	for (i = basey; i + y < height(); i++) {
		ty = i *  320 / scale;

		if (ty >= tex.height()) {
			break;
		}

		for (j = 0; ; j++) {
			tx = j * 320 / scale;

			if (tx >= tex.width()) {
				break;
			}

			tmp = mirror ? xs - j : j;

			if (tmp + x < 0 || tmp + x >= width()) {
				continue;
			}

			dst = _pixels + 3 * (tmp + x + (i + y) * width());

			if (tex.depth() == 3) {
				src = tex.pixels() + 3 * (tx + ty * tex.width());
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			} else if (tex.pixels()[tx + ty * tex.width()] == 0) {
				continue;
			} else if (tex.pixels()[tx + ty * tex.width()] == 1) {
				src = pal + 3 * tex.pixels()[tx + ty * tex.width()];
				// explicit cast is needed
				dst[0] = (src[0] + (unsigned)dst[0]) / 2;
				dst[1] = (src[1] + (unsigned)dst[1]) / 2;
				dst[2] = (src[2] + (unsigned)dst[2]) / 2;
			} else {
				src = pal + 3 * tex.pixels()[tx + ty * tex.width()];
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
		}
	}
}

void SoftRenderer::transparentBlit(const Texture &tex, int x, int y, const uint8_t *pal, unsigned scale, int mirror) {
	int i, j, tx, ty, basey = 0, tmp, tdepth = tex.depth();
	unsigned xs = tex.width() * scale / 320;
	uint8_t *dst, *basedst;
	const uint8_t *src, *basesrc;
	int *transtab;

	y -= tex.height() * scale / 320;

	if (y < 0) {
		basey = -y;
	}

	if (xs * 320 / scale >= tex.width()) {
		xs--;
	}

	if (tdepth != 3 && !pal) {
		pal = tex.palette();
	}

	assert(tdepth == 3 || (tdepth == 1 && pal));

	transtab = new int[xs + 2];

	for (i = 0; i < xs + 2; i++) {
		transtab[i] = i * 320 / scale;
	}

	for (i = basey; i + y < height(); i++) {
		ty = i *  320 / scale;

		if (ty >= tex.height()) {
			break;
		}

		basedst = _pixels + 3 * (x + (i + y) * width());

		if (tdepth == 3) {
			basesrc = tex.pixels() + 3 * ty * tex.width();
		} else {
			basesrc = tex.pixels() + ty * tex.width();
		}

		for (j = 0; j <= xs; j++) {
			tx = transtab[j];

			tmp = mirror ? xs - j : j;

			if (tmp + x < 0 || tmp + x >= width()) {
				continue;
			}

			dst = basedst + 3 * tmp;

			if (tdepth == 3) {
				src = basesrc + 3 * tx;
				*dst++ = src[0];
				*dst++ = src[1];
				*dst++ = src[2];
			} else if (basesrc[tx] == 0) {
				continue;
			} else if (basesrc[tx] & 0x80) {
				src = pal + 3 * basesrc[tx];
				// explicit cast is needed
				dst[0] = (src[0] + (unsigned)dst[0]) / 2;
				dst[1] = (src[1] + (unsigned)dst[1]) / 2;
				dst[2] = (src[2] + (unsigned)dst[2]) / 2;
				dst += 3;
			} else {
				src = pal + 3 * basesrc[tx];
				*dst++ = src[0];
				*dst++ = src[1];
				*dst++ = src[2];
			}
		}
	}

	delete[] transtab;
}

void SoftRenderer::wallBlit(const Texture &tex, int x, int y, int32_t *xtable, unsigned xlen, int16_t *ytable, unsigned ylen, const uint8_t *pal, int mirror) {
	int i, j, tx, ty, basey = 0, tmp, tdepth = tex.depth();
	int twidth = tex.width();
	uint8_t *dst, *basedst;
	const uint8_t *src, *basesrc;

	if (y < 0) {
		return;
	}

	if (tdepth != 3 && !pal) {
		pal = tex.palette();
	}

	assert(tdepth == 3 || (tdepth == 1 && pal));

	if (mirror) {
		x = width() - x - 1;
	}

	for (i = 0, ty = 0; y - i >= 0 && i < ylen; ty += ytable[i++]) {
		if (ty >= tex.height()) {
			break;
		}

		if (y - i >= height()) {
			continue;
		}

		basedst = _pixels + 3 * (y - i) * width();

		if (tdepth == 3) {
			basesrc = tex.pixels() + 3 * ty * twidth;
		} else {
			basesrc = tex.pixels() + ty * twidth;
		}

		for (j = 0, tx = 0; j < xlen; tx += xtable[j++] + 1) {
			if (tx >= twidth) {
				break;
			}

			tmp = mirror ? x - j : x + j;

			if (tmp < 0 || tmp >= width()) {
				continue;
			}

			dst = basedst + 3 * tmp;

			if (tdepth == 3) {
				src = basesrc + 3 * tx;
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			} else if (basesrc[tx] == 0) {
				continue;
			} else if (basesrc[tx] == 1) {
				break;
			} else {
				src = pal + 3 * basesrc[tx];
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
		}
	}
}

void SoftRenderer::videoBlit(unsigned ypos, const Texture &tex, const uint8_t *pal) {
	unsigned x, y, tmp1, tmp2;
	uint8_t *dst;
	const uint8_t *src1, *src2;

	assert(width() >= 2 * tex.width() && height() >= 2 * tex.height());

	if (!pal) {
		pal = tex.palette();
	}

	assert(tex.depth() == 3 || tex.depth() == 4 || (tex.depth() == 1 && pal));

	for (y = 0; y < tex.height() * 2 - 1; y++) {
		for (x = 0; x < tex.width() * 2 - 1; x++) {
			dst = _pixels + 3 * (x + (y + ypos) * width());
			tmp1 = tmp2 = (x >> 1) + (y >> 1) * tex.width();

			if (x & 1) {
				tmp2++;
			}

			if (y & 1) {
				tmp2 += tex.width();
			}

			if (tex.depth() == 3) {
				src1 = tex.pixels() + tmp1 * 3;
				src2 = tex.pixels() + tmp2 * 3;
			} else if (tex.depth() == 4) {
				src1 = tex.pixels() + tmp1 * 4 + 1;
				src2 = tex.pixels() + tmp2 * 4 + 1;
			} else {
				src1 = pal + 3 * tex.pixels()[tmp1];
				src2 = pal + 3 * tex.pixels()[tmp2];
			}

			if (tmp1 != tmp2) {
				// explicit cast is needed
				dst[0] = (src1[0] + (unsigned)src2[0]) / 2;
				dst[1] = (src1[1] + (unsigned)src2[1]) / 2;
				dst[2] = (src1[2] + (unsigned)src2[2]) / 2;
			} else {
				dst[0] = src1[0];
				dst[1] = src1[1];
				dst[2] = src1[2];
			}
		}
	}
}

void SoftRenderer::rectBlit(const Texture &tex, int x, int y, unsigned tx, unsigned ty, unsigned w, unsigned h, const uint8_t *pal) {
	unsigned i, j;
	uint8_t *dst;
	const uint8_t *src;

	if (!pal) {
		pal = tex.palette();
	}

	assert(tex.depth() == 3 || (tex.depth() == 1 && pal));

	for (i = 0; i < h && i + y < height() && i + ty < tex.height(); i++) {
		dst = _pixels + 3 * ((i + y) * width() + x);

		for (j = 0; j < w && j + x < width() && j + tx < tex.width(); j++) {
			if (tex.depth() == 3) {
				src = tex.pixels() + 3 * (j + tx + (i + ty) * tex.width());
			} else {
				src = tex.pixels() + j + tx + (i + ty) * tex.width();

				if (!*src) {
					dst += 3;
					continue;
				}

				src = pal + 3 * *src;
			}

			*dst++ = src[0];
			*dst++ = src[1];
			*dst++ = src[2];
		}
	}
}

void SoftRenderer::maskFill(const Texture &tex, int x, int y, uint8_t mask, int action, uint8_t r, uint8_t g, uint8_t b) {
	int i, j, basex = 0, basey = 0;
	uint8_t *dst;
	const uint8_t *src;

	if (x < 0) {
		basex = -x;
	}

	if (y < 0) {
		basey = -y;
	}

	assert(tex.depth() == 1);

	for (i = basey; i + y < height() && i < tex.height(); i++) {
		for (j = basex; j + x < width() && j < tex.width(); j++) {
			src = tex.pixels() + j + i * tex.width();
			dst = _pixels + 3 * (j + x + (i + y) * width());

			if (*src != mask) {
				continue;
			}

			switch (action) {
			case 0:
				dst[0] = r;
				dst[1] = g;
				dst[2] = b;
				break;

			case 1:
				dst[0] = (dst[0] + (unsigned)r) / 2;
				dst[1] = (dst[1] + (unsigned)g) / 2;
				dst[2] = (dst[2] + (unsigned)b) / 2;
				break;

			case 2:
				assert(r && g && b && "Invalid fraction");
				dst[0] -= dst[0] / r;
				dst[1] -= dst[1] / g;
				dst[2] -= dst[2] / b;
				break;

			}
		}
	}
}

void SoftRenderer::maskBlit(const Texture &tex, const Texture &mask, int x, int y, uint8_t shape, const uint8_t *pal) {
	int i, j, basex = 0, basey = 0;
	uint8_t *dst;
	const uint8_t *src, *msk;

	if (x < 0) {
		basex = -x;
	}

	if (y < 0) {
		basey = -y;
	}

	if (!pal) {
		pal = tex.palette();
	}

	assert(mask.depth() == 1 && (tex.depth() == 3 || (tex.depth() == 1 && pal)));
	assert(mask.width() == tex.width() && mask.height() == tex.height());

	for (i = basey; i + y < height() && i < tex.height(); i++) {
		for (j = basex; j + x < width() && j < tex.width(); j++) {
			msk = mask.pixels() + j + i * tex.width();
			dst = _pixels + 3 * (j + x + (i + y) * width());

			if (*msk != shape) {
				continue;
			}

			if (tex.depth() == 3) {
				src = tex.pixels() + 3 * (i * tex.width() + j);
			} else {
				src = pal + 3 * tex.pixels()[i * tex.width() + j];
			}

			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		}
	}
}

void SoftRenderer::bar(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t r, uint8_t g, uint8_t b, int action) {
	int i, j;
	uint8_t *dst;

	assert(action >= 0 && action < 4 && "Action not supported");

	for (i = 0; i < h && i + y < height(); i++) {
		dst = _pixels + 3 * ((i + y) * width() + x);

		for (j = 0; j < w && j + x < width(); j++) {
			if (action == 0) {
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
			} else if (action == 1) {
				dst[0] = (dst[0] + (unsigned)r) / 2;
				dst[1] = (dst[1] + (unsigned)g) / 2;
				dst[2] = (dst[2] + (unsigned)b) / 2;
				dst += 3;
			} else if (action == 2) {
				dst[0] -= dst[0] / r;
				dst[1] -= dst[1] / g;
				dst[2] -= dst[2] / b;
				dst += 3;
			} else if (action == 3) {
				*dst++ ^= r;
				*dst++ ^= g;
				*dst++ ^= b;
			}
		}
	}
}

void SoftRenderer::fcBlit(const Texture &tex, unsigned y, const T_FLOOR_MAP *celmask, const uint8_t *fog, const uint8_t *pal) {
	int i, tdepth = tex.depth();
	float factor;
	const uint8_t *src, *basesrc;
	uint8_t *dst;

	assert(celmask && "celmask not set");
	assert(tdepth == 3 || (tdepth == 1 && pal));

	do {
		dst = _pixels + depth() * ((y + celmask->dsty) * width() + celmask->x);
		factor = (float)celmask->srcy / (tex.height() - 1);

		if (tdepth == 3) {
			basesrc = tex.pixels() + 3 * (celmask->srcy * tex.width() + celmask->x);
		} else {
			basesrc = tex.pixels() + celmask->srcy * tex.width() + celmask->x;
		}

		for (i = 0; i < celmask->linewidth; i++) {
			if (tdepth == 3) {
				src = basesrc + 3 * i;
			} else {
				src = pal + 3 * basesrc[i];
			}

			if (fog) {
				*dst++ = src[0] + factor * (fog[0] - src[0]);
				*dst++ = src[1] + factor * (fog[1] - src[1]);
				*dst++ = src[2] + factor * (fog[2] - src[2]);
			} else {
				*dst++ = src[0];
				*dst++ = src[1];
				*dst++ = src[2];
			}
		}
	} while ((celmask++)->counter);
}

void SoftRenderer::drawText(int x, int y, const char *text) {
	unsigned char c;

	for (; *text; text++, x += charWidth(c)) {
		c = (unsigned char)*text;
		drawChar(x, y, c);
	}
}

void SoftRenderer::drawAlignedText(int x, int y, int halign, int valign, const char *text) {
	x -= textWidth(text) * halign / 2;
	y -= textHeight(text) * valign / 2;
	drawText(x, y, text);
}

void SoftRenderer::drawChar(int x, int y, unsigned char c) {
	int i, j;

	assert(_font && "Font not set");

	const Font::Glyph &glyph = _font->glyph(c);
	const uint8_t *ptr = glyph.data;
	uint8_t tmp = 0;

	for (i = 0; i < glyph.width && x + i < width(); i++) {
		for (j = 0; j < glyph.height && y + j < height(); j++) {
			if (tmp--) {
				continue;
			}

			tmp = *ptr++;

			if (!tmp) {
				continue;
			} else if (tmp == 0xff) {
				return;
			} else if (tmp >= 8) {
				tmp -= 7;
				continue;
			}

			if (x + i >= 0 && y + j >= 0 && (tmp != 1 || _shadow)) {
				_pixels[3 * (x + i + (y + j) * width())] = _fontPal[tmp - 1][0];
				_pixels[3 * (x + i + (y + j) * width()) + 1] = _fontPal[tmp - 1][1];
				_pixels[3 * (x + i + (y + j) * width()) + 2] = _fontPal[tmp - 1][2];
			}

			tmp = 0;
		}
	}
}

void SoftRenderer::setFont(const Font *font, int shadow, uint8_t r, uint8_t g, uint8_t b) {
	_font = font;
	_shadow = shadow;

	_fontPal[0][0] = 0;
	_fontPal[0][1] = 0;
	_fontPal[0][2] = 0;

	for (int i = 1; i < 5; i++) {
		_fontPal[i][0] = r;
		_fontPal[i][1] = g;
		_fontPal[i][2] = b;
	}
}

void SoftRenderer::setFont(const Font *font, int shadow, uint8_t pal[][3]) {
	_font = font;
	_shadow = shadow;
	memcpy(_fontPal, pal, sizeof(_fontPal));
}

void SoftRenderer::setFontColor(unsigned idx, uint8_t r, uint8_t g, uint8_t b) {
	assert(idx < FONT_COLORS);

	_fontPal[idx][0] = r;
	_fontPal[idx][1] = g;
	_fontPal[idx][2] = b;
}

unsigned SoftRenderer::textWidth(const char *text) const {
	assert(_font && "Font not set");
	return _font->textWidth(text);
}

unsigned SoftRenderer::textHeight(const char *text) const {
	assert(_font && "Font not set");
	return _font->textHeight(text);
}

unsigned SoftRenderer::charWidth(char text) const {
	assert(_font);
	return _font->glyph((unsigned char)text).width;
}

unsigned SoftRenderer::charHeight(char text) const {
	assert(_font);
	return _font->glyph((unsigned char)text).height;
}

FadeRenderer::FadeRenderer(const uint8_t *pal, unsigned width, unsigned height, uint8_t r, uint8_t g, uint8_t b) : TextureFade(pal, width, height, r, g, b) { }

void FadeRenderer::blit(const Texture &tex, int x, int y) {
	int i, j, basex, basey;
	const uint8_t *src;
	uint8_t *dst;
	assert(tex.depth() == 1 && "Invalid texture depth");

	basex = x < 0 ? -x : 0;
	basey = y < 0 ? -y : 0;

	for (i = basey; i < tex.height() && i + y < height(); i++) {
		src = tex.pixels() + basex + i * tex.width();
		dst = _pixels + basex + x + (i + y) * width();

		for (j = basex; j < tex.width() && j + x < width(); j++, src++, dst++) {
			if (*src) {
				*dst = *src;
			}
		}
	}
}

IconLib::IconLib(ReadStream &stream, unsigned count) : DataBlock(),
	_icons(NULL), _count(count) {
	int i;

	assert(count > 0);

	_icons = new TexturePal*[_count];

	for (i = 0; i < _count; i++) {
		_icons[i] = new TexturePal(stream);
	}
}

IconLib::~IconLib(void) {
	int i;

	for (i = 0; i < _count; i++) {
		delete _icons[i];
	}

	delete[] _icons;
}

const TexturePal &IconLib::operator[](unsigned idx) const {
	assert(idx < _count && "Index out of bounds");
	return *_icons[idx];
}

size_t IconLib::memsize() const {
	unsigned i;
	size_t ret;

	for (i = 0, ret = 0; i < _count; i++) {
		ret += _icons[i]->memsize();
	}

	return ret + sizeof(*this);
}

DataBlock *loadFont(SeekableReadStream &stream) {
	return new Font(stream);
}

/*MODEinfo vesadata[3];
SVGAinfo svgadata[3];
int lastbank=0;
int granuality=0;
int gran_mask=0;
uint16_t gr_page_end=0;
int gr_end_screen=0;

uint16_t *mapvesaadr(uint16_t *a);
#pragma aux mapvesaadr parm [edi] value [edi]

void write_vesa_info(int mode)
  {
  char c[20];

  getsvgainfo(&svgadata);
  printf("VIDEO mem   %5dKb\n"
         "Oem:        %s\n\n",
         svgadata[0].memory*64,
         svgadata[0].oemstr);
  getmodeinfo(&vesadata,mode);
  if (vesadata[0].modeattr & MA_SUPP)
     {
     if (vesadata[0].modeattr & MA_LINEARFBUF) sprintf(c,"%8Xh",(long)vesadata[0].linearbuffer); else strcpy(c,"None");
    printf("Mode:        %04X \n"
           "WinA           %02X\n"
           "WinB           %02X\n"
           "Granuality: %5dKB\n"
           "WinSize:    %5dKB\n"
           "Xres:       %5d\n"
           "Yres:       %5d\n"
           "Bppix:      %5d\n"
           "Lbuffer:    %s\n\n",
           mode,
           vesadata[0].winaattr,
           vesadata[0].winbattr,
           vesadata[0].wingran,
           vesadata[0].winsize,
           vesadata[0].xres,
           vesadata[0].yres,
           vesadata[0].bppix,
           c);
    }
  else printf("Mode %04X not currently supported!!!\n\n");
//  printf("--- Hit ENTER if values are correct or press CTRL+Break ---\n");
//  getche();
  delay(300);
  }
*/
static void showview_dx(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys);
//void showview64b(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys);
/*void showview32b(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  register longint a,b;

  if (x>640 || y>480) return;
  if (xs==0) xs=640;
  if (ys==0) ys=480;
  xs++;ys++;
  x&=~3;
  xs=(xs & ~3)+4;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw32bb(screen,NULL,NULL);
     return;
     }
  a=(x<<1)+linelen*y;
  b=y*1280+x*2;
  redrawbox32bb(xs,ys,(void *)((char *)screen+a),(void *)b);
  }
*/
/*
void set_scan_line(int newline);
#pragma aux set_scan_line=\
           "mov  eax,4f06h"\
           "xor  ebx,ebx"\
           "int 10h"\
   parm [ecx] modify [eax ebx];

int get_scan_line();
#pragma aux get_scan_line=\
           "mov  eax,4f06h"\
           "mov  bh,01"\
           "int  10h"\
   modify[eax ebx] value[ecx];

int initmode64b(void *paletefile)
  {
  int i;

  getmodeinfo(&vesadata,0x111);
  if (!(vesadata[0].modeattr & MA_SUPP)) return -1;
  //write_vesa_info(0x110);
  if (vesadata[0].winaattr & (WA_SUPP | WA_WRITE)) write_window=0;
  else if (vesadata[0].winbattr & (WA_SUPP | WA_WRITE)) write_window=1;
  else return -1;
  i=vesadata[0].wingran*1024;
  granuality=0;lastbank=0;
  while (i>>=1) granuality++;
  gran_mask=(1<<granuality)-1;
  gr_end_screen=0xa0000+gran_mask+1;
  gr_page_end=gran_mask+1;
  setvesamode(0x111,-1);
  lbuffer=(uint16_t *)0xa0000;
  screen=lbuffer;
  linelen=640*2;
  showview=showview64b;
  screen=(void *)malloc(screen_buffer_size);
  banking=1;
  screenstate=1;
  xlatmem=paletefile;
  return 0;

  }

int initmode32b()
  {
  int i;
  getmodeinfo(&vesadata,0x110);
  if (!(vesadata[0].modeattr & MA_SUPP)) return -1;
  //write_vesa_info(0x110);
  if (vesadata[0].winaattr & (WA_SUPP | WA_WRITE)) write_window=0;
  else if (vesadata[0].winbattr & (WA_SUPP | WA_WRITE)) write_window=1;
  else return -1;
  i=vesadata[0].wingran*1024;
  granuality=0;lastbank=0;
  while (i>>=1) granuality++;
  gran_mask=(1<<granuality)-1;
  gr_end_screen=0xa0000+gran_mask+1;
  gr_page_end=gran_mask+1;
  setvesamode(0x110,-1);
  lbuffer=(uint16_t *)0xa0000;
  screen=lbuffer;
  linelen=640*2;
  showview=showview32b;
  screen=(void *)malloc(screen_buffer_size);
  banking=1;
  screenstate=1;
  return 0;
  }



int initmode32bb()
  {
  int i;
  getmodeinfo(&vesadata,0x110);
  if (!(vesadata[0].modeattr & MA_SUPP)) return -1;
  //write_vesa_info(0x110);
  if (vesadata[0].winaattr & (WA_SUPP | WA_WRITE)) write_window=0;
  else if (vesadata[0].winbattr & (WA_SUPP | WA_WRITE)) write_window=1;
  else return -1;
  i=vesadata[0].wingran*1024;
  granuality=0;lastbank=0;
  while (i>>=1) granuality++;
  setvesamode(0x110,-1);
  set_scan_line(1024);
  if (get_scan_line()!=1024 && !__skip_change_line_test)
     {
     text_mode();
     return -10;
     }
  lbuffer=(uint16_t *)0xa0000;
  screen=lbuffer;
  linelen=640*2;
  showview=showview32b;
  screen=(void *)malloc(screen_buffer_size);
  banking=1;
  screenstate=1;
  return 0;
  }



uint16_t *mapvesaadr1(uint16_t *a)
  {
  uint16_t bank;

  bank=(long)a>>16;
  if (bank!=lastbank)
     {
     lastbank=bank;
     bank=bank;
          {
           union REGS regs;
           regs.w.ax = 0x4f05;
           regs.w.bx = write_window;
           regs.w.dx = bank;
           int386 (0x10,&regs,&regs); // window A
          }
     }
 return (uint16_t *)(((long)a & 0xffff)+0xa0000);
}

void switchvesabank(uint16_t bank)
#pragma aux switchvesabank parm [eax]
  {
           union REGS regs;
           regs.w.ax = 0x4f05;
           regs.w.bx = 0;
           regs.w.dx = bank;
           int386 (0x10,&regs,&regs); // window A
  }

*/
int initmode_dx(char inwindow, char zoom, char monitor, int refresh)
  {
//  if (!DXInit64(inwindow,zoom,monitor,refresh)) return -1;
  if (!Screen_Init(inwindow,zoom,monitor,refresh)) return -1;
  showview=showview_dx;
  screenstate=1;
//  scr_linelen2=scr_linelen/2;
  return 0;
  }


  /*
int initmode256(void *paletefile)
  {
  MODEinfo data;

  getmodeinfo(&data,0x100+line480);
  if (!(data.modeattr & MA_SUPP)) return initmode256b(paletefile);
  if (!(data.modeattr & MA_LINEARFBUF)) return initmode256b(paletefile);
  //write_vesa_info(0x101);
  setvesamode(0x4101,-1);
  if (lbuffer==NULL)lbuffer=(uint16_t *)physicalalloc((long)data.linearbuffer,screen_buffer_size>>1);
  screen=lbuffer;
  linelen=640*2;
  palmem=(char *)paletefile;
  xlatmem=palmem+768;
  setpal((void *)palmem);
  showview=showview256;
  screen=(void *)malloc(screen_buffer_size);
  screenstate=1;
  banking=0;
  return 0;
  }


void showview256b(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  register longint a,b;

  if (x>640 || y>480) return;
  if (xs==0) xs=640;
  if (ys==0) ys=480;
  xs++;ys++;
  x&=~3;
  xs=(xs & ~3)+4;
  y&=~1;
  ys=(ys & ~1)+2;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw256b(screen,0,xlatmem);
     return;
     }
  a=(x<<1)+linelen*y;
  b=y*640+x;
  redrawbox256b(xs,ys,(void *)((char *)screen+a),(void *)b,xlatmem);
  }


int initmode256b(void *paletefile)
  {
  int i;
  getmodeinfo(&vesadata,0x100);
  if (!(vesadata[0].modeattr & MA_SUPP)) return -1;
  //write_vesa_info(0x101);
  i=vesadata[0].wingran*1024;
  if (vesadata[0].winaattr & (WA_SUPP | WA_WRITE)) write_window=0;
  else if (vesadata[0].winbattr & (WA_SUPP | WA_WRITE)) write_window=1;
  else return -1;
  granuality=0;lastbank=0;
  while (i>>=1) granuality++;
  gran_mask=(1<<granuality)-1;
  gr_end_screen=0xa0000+gran_mask+1;
  gr_page_end=gran_mask+1;
  setvesamode(0x101,-1);
  lbuffer=(uint16_t *)0xa0000;
  screen=lbuffer;
  palmem=(char *)paletefile;
  xlatmem=palmem+768;
  setpal((void *)palmem);
  linelen=640*2;
  showview=showview256b;
  screen=(void *)malloc(screen_buffer_size);
  banking=1;
  screenstate=1;
  return 0;
  }


void init_lo();
#pragma aux init_lo modify[eax ebx ecx edx esi edi]

int initmode_lo(void *paletefile)
  {
  init_lo();
  palmem=(char *)paletefile;
  xlatmem=palmem+768;
  setpal((void *)palmem);
  linelen=640*2;
  lbuffer=0;
  showview=showview_lo;
  screen=(void *)malloc(screen_buffer_size);
  screenstate=1;
  banking=1;
  return 0;
  }
*/

void closemode()
  {
  if (screenstate)
     {
     palmem=NULL;
     Sys_Shutdown();
//     DXCloseMode();
     }
  screenstate=0;

  }

static void showview_dx(uint16_t x, uint16_t y, uint16_t xs, uint16_t ys) {
//  register longint a;

	if (x > renderer->width() || y > renderer->height()) {
		return;
	}

	if (xs == 0) {
		xs = renderer->width();
	}

	if (ys == 0) {
		ys = renderer->height();
	}

	xs += 2;
	ys += 2;

	if (x + xs > renderer->width()) {
		xs = renderer->width() - x;
	}

	if (y + ys > renderer->height()) {
		ys = renderer->height() - y;
	}

	renderer->drawRect(x, y, xs, ys);
}

/*
static void showview64b(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  register longint a;

  if (x>640 || y>480) return;
  if (xs==0) xs=640;
  if (ys==0) ys=480;
  xs+=2;ys+=2;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw64b(screen,NULL,xlatmem);
     return;
     }
  a=(x<<1)+linelen*y;
  redrawbox64b(xs,ys,(void *)((char *)screen+a),(void *)((char *)a),xlatmem);
  }


void showview256(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  register longint a;

  if (xs==0) xs=640;
  if (ys==0) ys=480;
  x&=0xfffe;y&=0xfffe;xs+=2;ys+=2;
  if (x>640 || y>480) return;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw256(screen,lbuffer,xlatmem);
     return;
     }
  a=(x<<1)+linelen*y;
  redrawbox256(xs,ys,(void *)((char *)screen+a),(void *)((char *)lbuffer+(a>>1)),xlatmem);
  }

void showview_lo(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  register longint a,b;

  if (xs==0) xs=640;
  if (ys==0) ys=480;
  if (ys==0) ys=480;
  x&=0xfffe;y&=0xfffe;xs+=2;ys+=2;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw_lo(screen,lbuffer,xlatmem);
     return;
     }
  a=(x<<1)+linelen*y;
  b=x+640*y;
  redrawbox_lo(xs,ys,(void *)((char *)screen+a),(void *)((char *)lbuffer+b),xlatmem);
  }
*/


void show_ms_cursor(int x, int y) {
	renderer->moveMouse(x, y);
	renderer->showMouse();
}

void hide_ms_cursor() {
	renderer->hideMouse();
}

void register_ms_cursor(const Texture *cursor) {
	renderer->setMouseCursor(*cursor);
}

void move_ms_cursor(int16_t newx, int16_t newy, char nodraw) {
	renderer->moveMouse(newx, newy);
}

/*void pal_optimize()
  {
  long *stattable;
  uint16_t *c;
  char *d;
  int i;
  long maxr,maxg,maxb,max;
  int j;

  if (palmem==NULL) return;
  stattable=(long *)getmem(32768*sizeof(long));
  memset(stattable,0,32768*sizeof(long));
  c=screen;
  for(i=0;i<screen_buffer_size;i++,c++)
     stattable[*c & 0x7fff]++;
  for(j=0;j<256;j++)
     {
     max=0;
     for (i=0;i<32768;i++)
        if (stattable[i]>max)
           {
           *((uint16_t *)xlatmem+j)=i;
           max=stattable[i];
           }
     stattable[*((uint16_t *)xlatmem+j)]=-1;
     }
  d=palmem;
  c=(uint16_t *)xlatmem;
  for(i=0;i<256;i++)
     {
     j=*c++;
     *d++=((j>>9)& 0x3e);
     *d++=((j>>4)& 0x3e);
     *d++=(j & 0x1f)<<1;
     }
  setpal((void *)palmem);
  memset(xlatmem,0,65536);
  for(j=0;j<32768;j++)
     {
     int r1,g1,b1;
     int r2,g2,b2,dif;
     char *c;
     maxr=maxg=maxb=999999999;
     r1=(j>>9)& 0x3e;g1=(j>>4)& 0x3e;b1=(j & 0x1f)<<1;
     c=palmem;
        for(i=0;i<256;i++)
        {
        r2=abs(r1-*c++);
        g2=abs(g1-*c++);
        b2=abs(b1-*c++);
        dif=r2+b2+g2;
        if (dif<=maxb)
           {
           if (dif<maxb) xlatmem[j*2]=i;
           else xlatmem[j*2]=xlatmem[j*2+1];
           xlatmem[j*2+1]=i;
           maxb=dif;
           }
        }
     }
  showview(0,0,0,0);
  free(stattable);
  }
*/
void rectangle(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b) {
	curcolor[0] = r;
	curcolor[1] = g;
	curcolor[2] = b;
	hor_line(x1,y1,x2);
	hor_line(x1,y2,x2);
	ver_line(x1,y1,y2);
	ver_line(x2,y1,y2);
}

/*
void showview16(uint16_t x,uint16_t y,uint16_t xs,uint16_t ys)
  {
  int x1,x2;
  if (x>640 || y>480) return;
  if (xs==0) xs=640;
  if (ys==0) ys=480;
  if (x+xs>640) xs=640-x;
  if (y+ys>480) ys=480-y;
  if (xs>550 && ys>400)
     {
     redraw16(screen,lbuffer,xlatmem);
     return;
     }
  x1=x & ~0x7;
  x2=(x+xs+7) & ~0x7;
  redrawbox16((x2-x1)/8,ys,screen+x1+640*y,(char *)lbuffer+x1/8+80*y,xlatmem);
  }

void init16colors();
#pragma aux init16colors modify [eax]=\
  "mov  eax,12h"\
  "int  10h"\

int initmode16(void *palette)
  {
  palette;
  init16colors();
  lbuffer=(uint16_t *)0xa0000;
  screen=lbuffer;
  linelen=640*2;
  showview=showview16;
  screen=(void *)malloc(screen_buffer_size);
  palmem=(char *)palette;
  xlatmem=palmem+768;
  setpal((void *)palmem);
  banking=0;
  screenstate=1;
  return 0;
  }

void empty_show_view(int x,int y,int xs,int ys)
  {
  x,y,xs,ys;
  }


int init_empty_mode()
  {
  screen=(void *)malloc(screen_buffer_size);
  showview=empty_show_view;
  banking=1;
  lbuffer=NULL;
  screenstate=1;
  return 0;
  }
*/
