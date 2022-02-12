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
#ifndef _STRLITE_H_
#define _STRLITE_H_

#include <cstdlib>

class StringList {
private:
	char **_data;
	unsigned _size, _count;

	// Do not implement
	StringList(const StringList &src);
	const StringList &operator=(const StringList &src);
public:
	explicit StringList(unsigned prealloc = 8);
	~StringList(void);

	/// Insert string into first empty space
	unsigned insert(const char *str);
	/// Replace string on position idx
	void replace(unsigned idx, const char *str);
	/// Delete selected string
	void remove(unsigned idx);
	/// Remove all NULL pointers
	void pack(void);
	/// Replace all strings with NULL pointers
	void clear(void);

	/// Get string
	inline const char *operator[](unsigned idx) const {
		return idx < _size ? _data[idx] : NULL;
	}

	/// Total count of non-NULL strings, if you want to pass through
	/// the entire list, use size() instead
	inline unsigned count(void) const {
		return _count;
	}

	/// Allocated size of string table, including empty space
	inline unsigned size(void) const {
		return _size;
	}
};

#endif 
