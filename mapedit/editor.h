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
 *  Last commit made by: $Id: editor.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef ___EDITOR__H___
#define ___EDITOR__H___

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_FORCESAVE (WM_APP+1)
#define MSG_FINDANDPOPUP (WM_APP+2)
#define MSG_CLOSEEDITOR (WM_APP+3)

void EditSkeldalFile(const char *filename);





#ifdef __cplusplus
  }
#endif

#endif