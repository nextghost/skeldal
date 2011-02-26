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
#ifndef _WAV_H
#define _WAV_H

#include <inttypes.h>
#include "libs/memman.h"

#define WAV_RIFF "RIFF"
#define WAV_WAVE "WAVE"
#define WAV_FMT  "fmt "
#define WAV_DATA "data"

class SoundSample : public DataBlock {
private:
	unsigned _mode, _channels, _length;
	int _freq, _bps;
	unsigned char *_data;

	// do not implement
	SoundSample(const SoundSample &src);
	const SoundSample &operator=(const SoundSample &src);
public:
	SoundSample(SeekableReadStream &stream);
	~SoundSample(void);

	unsigned mode(void) const { return _mode; }
	unsigned channels(void) const { return _channels; }
	unsigned length(void) const { return _length; }
	int freq(void) const { return _freq; }
	int bps(void) const { return _bps; }
	const void *data(void) const { return _data; }
	unsigned memsize(void) const { return _length * sizeof(unsigned char) + sizeof(*this); }
};

#endif
