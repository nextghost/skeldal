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
 *  Last commit made by: $Id: WStringMemory.h 7 2008-01-14 20:14:25Z bredysoft $
 */
// WStringMemory.h: interface for the WStringMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WSTRINGMEMORY_H__79693029_4788_4099_9A97_92AF310A7AD5__INCLUDED_)
#define AFX_WSTRINGMEMORY_H__79693029_4788_4099_9A97_92AF310A7AD5__INCLUDED_

#include <WCHAR.H>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class WStringProxy;

class WStringMemory
{
public:

	static WStringProxy * AllocString(const wchar_t *text, size_t size);
	static WStringProxy * AllocProxy(const WStringProxy &templateProxy);
	static void FreeProxy(WStringProxy *proxy);
	static void LockProxy(WStringProxy *proxy);
	static void UnlockProxy(WStringProxy *proxy);

	//Releases extra memory leaved for future fast allocations
	static void FreeExtra();

	//Support addref for proxy - Single- or Multi- thread support
	static void AddRefProxy(WStringProxy *proxy);

	//Support addref for proxy - Single- or Multi- thread support
	//returns true, when counter reached zero
	static bool ReleaseRefProxy(WStringProxy *proxy);

    ///Gets statistics about memory manager
    /** 
    @param details optional pointer to 32 size_t items. Array will be filled
      with sizes of each string fastalloc group.
    @return function returns total bytes allocated for string fast allocation.
      (This number should be below 256KB)
    */
    static size_t GetStatistics(size_t *details=NULL);

    ///Function allows sharing the strings
	/** This function is called by WString::Share function
	Function mark proxy shared. If string is same as shared,
	it returns pointer to proxy with proxy of string that contains
	the same text.
	Read description of WString::Share for more informations
	*/
	static WStringProxy *ShareString(WStringProxy *proxy);

	///Function disables sharing of the string
	static void UnshareString(WStringProxy *proxy);

};



#endif // !defined(AFX_WSTRINGMEMORY_H__79693029_4788_4099_9A97_92AF310A7AD5__INCLUDED_)
