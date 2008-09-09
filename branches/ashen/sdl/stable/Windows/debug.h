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
 *  Last commit made by: $Id: debug.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>

#define STOP() StopProgram(__FILE__,__LINE__);

static __inline void StopProgram(const char *text, int line) 
{
	char buff[256];
	sprintf(buff,"Stop at %s line %d",text,line);
	MessageBox(NULL,buff,NULL,MB_OK|MB_SYSTEMMODAL);
	__asm 
	{
		int 3;
	}  
}

#endif
