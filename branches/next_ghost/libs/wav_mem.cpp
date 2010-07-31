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
#include <cstring>
#include <cassert>
#include "wav_mem.h"

SoundSample::SoundSample(SeekableReadStream &stream) : _mode(0), _channels(0),
	_length(0), _freq(0), _bps(0), _data(NULL) {

	char buf[5] = "";
	unsigned size;

	stream.read(buf, 4);
	assert(!strncmp(buf, WAV_RIFF, 4) && "File not in RIFF format");
	stream.readUint32LE();	// file size, ignore
	stream.read(buf, 4);
	assert(!strncmp(buf, WAV_WAVE, 4) && "File does not contain WAVE data");
	stream.read(buf, 4);

	while (!stream.eos()) {
		if (!strncmp(buf, WAV_FMT, 4)) {
			size = stream.readUint32LE();
			assert(size >= 12 && "Chunk size assumption failed");
			_mode = stream.readUint16LE();
			_channels = stream.readUint16LE();
			_freq = stream.readUint32LE();
			_bps = stream.readUint32LE();
			// ignore any remaining format data
			stream.seek(size - 12, SEEK_CUR);
		} else if (!strncmp(buf, WAV_DATA, 4)) {
			assert(!_data && "Error: Multiple data chunks in file");
			_length = stream.readUint32LE();
			_data = new unsigned char[_length];
			stream.read(_data, _length);
		} else {
			size = stream.readUint32LE();
			stream.seek(size, SEEK_CUR);
		}

		stream.read(buf, 4);
	}

	assert(_data && _freq && "Error: Key chunks not found");
}

SoundSample::~SoundSample(void) {
	delete[] _data;
}
