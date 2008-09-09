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
 *  Last commit made by: $Id: IWStringEffect.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#if !defined(AFX_IWSTRINGEFFECT_H__863ADE82_7789_4E54_BE7D_B8F0740FD81B__INCLUDED_)
#define AFX_IWSTRINGEFFECT_H__863ADE82_7789_4E54_BE7D_B8F0740FD81B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wchar.h>

class IWStringEffect
{
public:
  ///function returns extra size that effect needs
  /**
  @param curSize size of string that enters to the effect
  @return count of extra characters needs to effects;
  */
  virtual unsigned long GetEffectExtraSize(unsigned long curSize) {return 0;}

  ///function renders begin of string. 
  /** Function returns number of characters rendered, and must be <= then size returned by GetEffectExtraSize()
  @param renderPtr pointer to render buffer
  @param curSize size of string that enters to the effect
  @return number of characters rendered. Entered string will be rendered behind.
  */
  virtual unsigned long PreRenderString(wchar_t *renderPtr,unsigned long curSize) {return 0;}

  ///function renders effect. 
  /** 
  @param renderPtr pointer to begin of render buffer.
  @param rendered number of characters rendered by previous effect. Value doesn't point to the end
  of buffer, function must add result of PreRenderString */
  virtual void RenderString(wchar_t *renderPtr, unsigned long rendered)=0;
};

#endif
