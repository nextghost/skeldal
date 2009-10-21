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
#include "libs/strlite.h"

StringList::StringList(unsigned prealloc) : _data(NULL),
	_size(prealloc ? prealloc : 1), _count(0) {
	_data = new char*[_size];
	memset(_data, 0, _size * sizeof(char*));
}

StringList::~StringList(void) {
	for (int i = 0; i < _size; i++) {
		if (_data[i]) {
			delete[] _data[i];
		}
	}

	delete[] _data;
}

unsigned StringList::insert(const char *str) {
	unsigned i;

	for (i = 0; i < _size && _data[i]; i++);

	replace(i, str);
	return i;
}

void StringList::replace(unsigned idx, const char *str) {
	if (idx >= _size) {
		char **tmp = new char*[idx + _size];
		memcpy(tmp, _data, _size * sizeof(char*));
		memset(tmp + _size, 0, idx * sizeof(char*));
		delete[] _data;
		_data = tmp;
		_size += idx;
	}

	if (_data[idx]) {
		delete[] _data[idx];
	} else {
		_count++;
	}

	_data[idx] = new char[strlen(str) + 1];
	strcpy(_data[idx], str);
}

void StringList::remove(unsigned idx) {
	if (idx < _size && _data[idx]) {
		delete[] _data[idx];
		_data[idx] = NULL;
		_count--;
	}
}

void StringList::pack(void) {
	char **dst, **ptr;
	unsigned i, size = _count ? _count : 1;

	ptr = dst = new char*[size];

	for (i = 0; i < _size; i++) {
		if (_data[i]) {
			assert(dst != ptr + _count && "Broken string counter");
			*dst++ = _data[i];
		}
	}

	memset(dst, 0, (size - _count) * sizeof(char*));

	delete[] _data;
	_data = ptr;
	_size = size;
}

void StringList::clear(void) {
	for (unsigned i = 0; i < _size; i++) {
		if (_data[i]) {
			delete[] _data[i];
			_data[i] = NULL;
		}
	}

	_count = 0;
}
