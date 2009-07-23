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
#include "libs/system.h"

char *strupr(char *str) {
	int i;
	for (; str[i]; i++) {
		str[i] = toupper(str[i]);
	}
	return str;
}

// TODO: implement these
void Mouse_GetEvent(MS_EVENT *event) {

}

void Mouse_MapWheel(char up, char down) {

}

int Input_Kbhit(void) {

}

int Input_ReadKey(void) {

}

void Sys_ErrorBox(const char *msg) {

}

void Sys_WarnBox(const char *msg) {

}

void Sys_InfoBox(const char *msg) {

}

void Sys_Shutdown(void) {

}

void Sys_SetEnv(const char *name, const char *value) {

}

void Sys_Init(void) {

}

void Sys_Mkdir(const char *path) {

}

int Sys_LatestFile(char *mask, int offset) {
	return 0;
}

void Sys_PurgeTemps(char z) {

}

int Sys_PackStatus(FILE *f) {
	return 0;
}

char Screen_Init(char windowed, int zoom, int monitor, int refresh) {
	return 0;
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
	return NULL;
}

long Screen_GetSize(void) {
	return 0;
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

void *Screen_PrepareWalk(int ypos) {
	return NULL;
}

void Screen_ZoomWalk(void *handle, int ypos, int *points,float phase, void *lodka) {

}

void Screen_DoneWalk(void *handle) {

}

void *Screen_PrepareTurn(int ypos) {
	return NULL;
}

void Screen_Turn(void *handle, char right, int ypos,int border, float phase, void *lodka) {

}

void Screen_DoneTurn(void *handle) {

}

void Screen_StripBlt(void *data, unsigned int startline, unsigned long wihtd) {

}

void Screen_Shift(int x, int y) {

}

void ShareCPU(void) {

}

char Sound_SetEffect(int filter, int data) {
	return 0;
}

int Sound_GetEffect(int filter) {
	return 0;
}

char Sound_CheckEffect(int filter) {
	return 0;
}

char Sound_IsActive(void) {
	return 0;
}

void Sound_SetVolume(int channel, int left, int right) {

}

void Sound_GetVolume(int channel, int *left, int *right) {

}

char Sound_GetChannelState(int channel) {
	return 0;
}

int Sound_MixBack(int synchro) {
	return 0;
}

void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type) {

}

void Sound_ChangeMusic(char *file) {

}

void Sound_BreakLoop(int channel) {

}

void Sound_BreakExt(int channel, void *sample, long size) {

}

void Sound_Mute(int channel) {

}

void Sound_StopMixing(void) {

}

void Sound_StartMixing(void) {

}

void Sound_SetMixer(int dev, int freq, ...) {

}

int Task_Add(int stack, TaskerFunctionName func, ...) {
	return 0;
}

void *Task_Sleep(void *data) {
	return NULL;
}

void *Task_WaitEvent(long event) {
	return NULL;
}

char Task_IsMaster(void) {
	return 0;
}

int Task_Count(void) {
	return 0;
}

void Task_Wakeup(EVENT_MSG *msg) {

}

void Task_Term(int id) {

}

char Task_IsRunning(int id) {
	return 0;
}

char Task_QuitMsg() {
	return 0;
}

void Task_Shutdown(int id) {

}

int Timer_GetValue(void) {
	return 0;
}

int Timer_GetTick(void) {
	return 0;
}

void Timer_Sleep(int msec) {

}

int get_control_state(void) {
	return 0;
}

int get_shift_state(void) {
	return 0;
}

void *LoadDefaultFont(void) {
	return NULL;
}

void *PrepareVideoSound(int mixfreq, int buffsize) {
	return NULL;
}

void DoneVideoSound(void *buffer) {

}

char LoadNextVideoFrame(void *buffer, char *data, int size, short *xlat, short *accnums, long *writepos) {
	return 0;
}

void *OpenMGFFile(const char *filename) {
	return NULL;
}

void CloseMGFFile(void *file) {

}

char OtevriUvodniOkno() {
	return 0;
}

char SelectAdventure() {
	return 0;
}

char *GetSelectedAdventure() {
	return NULL;
}

