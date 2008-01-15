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
#ifndef __ANIPACK_H

#define __ANIPACK_H

int anim_line_len=1280;

void load_0bit(void *target,void *palette);
void *load_1bit(void *source,void *target,void *palette);
void *load_2bit(void *source,void *target,void *palette);
void *load_3bit(void *source,void *target,void *palette);
void *load_4bit(void *source,void *target,void *palette);
void *load_8bit(void *source,void *target,void *palette);
#pragma aux load_0bit parm [EDI][EBX] modify [EAX ECX EDX]
#pragma aux load_1bit parm [ESI][EDI][EBX] modify [EAX ECX EDX] value [esi]
#pragma aux load_2bit parm [ESI][EDI][EBX] modify [EAX ECX EDX] value [esi]
#pragma aux load_3bit parm [ESI][EDI][EBX] modify [EAX ECX EDX] value [esi]
#pragma aux load_4bit parm [ESI][EDI][EBX] modify [EAX ECX EDX] value [esi]
#pragma aux load_8bit parm [ESI][EDI][EBX] modify [EAX ECX] value [esi]

void *save_1bit(void *source,void *target);
#pragma aux save_1bit parm[esi][edi] modify [eax ebx] value[edi];

void *save_2bit(void *source,void *target);
#pragma aux save_2bit parm[esi][edi] modify [eax ebx] value[edi];

void *save_3bit(void *source,void *target);
#pragma aux save_3bit parm[esi][edi] modify [eax ebx] value[edi];

void *save_4bit(void *source,void *target);
#pragma aux save_4bit parm[esi][edi] modify [eax ebx] value[edi];


void display_interlaced_fused_lfb(void *buff1,void *buff2,void *lfb);
#pragma aux display_interlaced_fused_lfb parm [esi][ebx][edi]modify [ecx edx eax]

void display_interlaced_fused_bank(void *buff1,void *buff2,void *scr);
#pragma aux display_interlaced_fused_bank parm [esi][ebx][edi]modify [ecx edx eax]

#endif
