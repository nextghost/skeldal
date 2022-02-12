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
 *  Last commit made by: $Id: BITLINE.H 7 2008-01-14 20:14:25Z bredysoft $
 */
void *cmode0(void *source,void *target,int count,int rovina);
#pragma aux cmode0 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cmode1(void *source,void *target,int count,int rovina);
#pragma aux cmode1 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cmode2(void *source,void *target,int count,int rovina);
#pragma aux cmode2 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cmode3(void *source,void *target,int count,int rovina);
#pragma aux cmode3 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cread0(void *source,void *target,int count,int rovina);
#pragma aux cread0 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cread1(void *source,void *target,int count,int rovina);
#pragma aux cread1 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cread2(void *source,void *target,int count,int rovina);
#pragma aux cread2 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
void *cread3(void *source,void *target,int count,int rovina);
#pragma aux cread3 parm [ESI][EDI][ECX][EBX] modify [EAX EDX] value [EDI];
