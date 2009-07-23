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

#include <stdlib.h>
#include <ctype.h>

char *strupr(char *str) {
	for (; *str; str++) {
		*str = toupper(*str);
	}
}

// TODO: implement these
void Sys_ErrorBox(const char *msg) {

}

void Sys_WarnBox(const char *msg) {

}

void Sys_InfoBox(const char *msg) {

}

void Sys_Shutdown(void) {

}

int Screen_GetXSize(void) {
	return 0;
}

int Screen_GetYSize(void) {
	return 0;
}

unsigned short *Screen_GetAddr(void) {
	return NULL;
}

unsigned short *Screen_GetBackAddr(void) {

}

void Screen_SetAddr(unsigned short *addr) {

}

void Screen_SetBackAddr() {

}

void Screen_Restore(void) {

}

void Screen_DrawRect(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {

}

void Screen_DrawRectZoom2(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {

}

char Sound_SetEffect(int filter, int data) {
	return 0;
}

int Sound_GetEffect(int filter) {
	return 0;
}

char Sound_IsActive(void) {
	return 0;
}

int Timer_GetValue(void) {
	return 0;
}

int Timer_GetTick(void) {
	return 0;
}

int get_control_state() {
	return 0;
}

int get_shift_state() {
	return 0;
}

