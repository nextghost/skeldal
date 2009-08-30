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

StringList::StringList(unsigned prealloc) : _data(new char*[prealloc]),
	_size(prealloc), _count(0) {
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

unsigned StringList::insert(const char *str, unsigned len) {
	unsigned i;

	for (i = 0; i < _size && _data[i]; i++);

	replace(i, str, len);
	return i;
}

void StringList::replace(unsigned idx, const char *str, unsigned len) {
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

	if (len) {
		_data[idx] = new char[len + 1];
		memcpy(_data[idx], str, len * sizeof(char));
		_data[idx][len] = '\0';
	} else {
		_data[idx] = new char[strlen(str) + 1];
		strcpy(_data[idx], str);
	}
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
	unsigned i;

	ptr = dst = new char*[_count];

	for (i = 0; i < _size; i++) {
		if (_data[i]) {
			assert(dst != ptr + _count && "Broken string counter");
			*dst++ = _data[i];
		}
	}

	delete[] _data;
	_data = ptr;
	_size = _count;
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
