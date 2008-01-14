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
#pragma once

#include "WString.h"

class IDDLFileEnumerator
{
public:
  virtual bool File(WString name, int group, unsigned long offset)=0;  
};

struct DDLData
{
  void *data;
  size_t sz;

  DDLData():data(0),sz(0) {}
  ~DDLData() {free(data);}
  DDLData(const DDLData &other):data(other.data),sz(other.sz) 
    {
      DDLData &dother=const_cast<DDLData &>(other);
      dother.data=0;dother.sz=0;
    }
  DDLData& operator =(const DDLData &other) 
    {
      DDLData &dother=const_cast<DDLData &>(other);
      data=other.data;sz=other.sz;
      dother.data=0;dother.sz=0;
      return *this;
    }
};

class DDLFile
{
    HANDLE _hFile;

    bool ReadFile(void *data, size_t sz);
public:
  DDLFile(void);
  ~DDLFile(void);

  bool OpenDDLFile(WString filename);
  bool EnumFiles(IDDLFileEnumerator &enmClass);
  DDLData ExtractFile(unsigned long offset);
  unsigned long GetFileSize(unsigned long offset);
};
