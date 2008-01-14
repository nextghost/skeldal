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
/*void install_dos_error(void *,void *);
#pragma aux install_dos_error parm [eax][edx] modify [ebx ecx esi edi]
*/

#define _ERR_WRITE 1
#define _ERR_SYS 0
#define _ERR_FAT 1
#define _ERR_DIR 2
#define _ERR_DATA 3
#define _ERR_EFAIL 8
#define _ERR_ERETRY 16
#define _ERR_EIGNORE 32
#define _ERR_NOTDISK 128

#define _ERR_IGNORE 0
#define _ERR_RETRY 1
#define _ERR_ABORT 2
#define _ERR_FAIL 3
