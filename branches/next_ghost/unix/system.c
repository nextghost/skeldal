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
#include <assert.h>
#include <unistd.h>
#include "libs/system.h"
#include "game/globals.h"

char *strupr(char *str) {
	int i;
	for (i = 0; str[i]; i++) {
		str[i] = toupper(str[i]);
	}
	return str;
}

// TODO: implement these
void Mouse_GetEvent(MS_EVENT *event) {
	assert(0);
}

void Mouse_MapWheel(char up, char down) {
	assert(0);

}

int Input_Kbhit(void) {
	assert(0);

}

int Input_ReadKey(void) {
	assert(0);

}

// FIXME: Implement in GUI
void Sys_ErrorBox(const char *msg) {
	fprintf(stderr, "Error: %s", msg);
}

// FIXME: Implement in GUI
void Sys_WarnBox(const char *msg) {
	fprintf(stderr, "Warning: %s", msg);
}

// FIXME: Implement in GUI
void Sys_InfoBox(const char *msg) {
	fprintf(stderr, "Info: %s", msg);
}

void Sys_Shutdown(void) {
	Screen_Shutdown();
	assert(0);
}

void Sys_SetEnv(const char *name, const char *value) {
	setenv(name, value, 1);
}

void Sys_Mkdir(const char *path) {
	assert(0);

}

int Sys_LatestFile(char *mask, int offset) {
	assert(0);
	return 0;
}

// FIXME: get rid of temps
void Sys_PurgeTemps(char z) {
//	assert(0);
}

int Sys_PackStatus(FILE *f) {
	assert(0);
	return 0;
}

int Sys_FileExists(const char *file) {
	return !access(file, F_OK);
}

void *Sys_ReadFile(const char *file) {
	FILE *fr = fopen(file, "r");
	int len;
	void *ret;

	if (!fr) {
		fprintf(stderr, "Could not open file %s\n", file);
		exit(1);
	}

	len = fseek(fr, 0, SEEK_END);
	fseek(fr, 0, SEEK_SET);

	ret = malloc(len);
	fread(ret, 1, len, fr);
	return ret;
}

void Screen_SetAddr(unsigned short *addr) {
	assert(0);

}

void Screen_SetBackAddr() {
	assert(0);

}

void Screen_Restore(void) {
	assert(0);

}

void Screen_DrawRect(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {
	assert(0);

}

void Screen_DrawRectZoom2(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {
	assert(0);

}

void *Screen_PrepareWalk(int ypos) {
	assert(0);
	return NULL;
}

void Screen_ZoomWalk(void *handle, int ypos, int *points,float phase, void *lodka) {
	assert(0);

}

void Screen_DoneWalk(void *handle) {
	assert(0);

}

void *Screen_PrepareTurn(int ypos) {
	assert(0);
	return NULL;
}

void Screen_Turn(void *handle, char right, int ypos,int border, float phase, void *lodka) {
	assert(0);

}

void Screen_DoneTurn(void *handle) {
	assert(0);

}

void Screen_StripBlt(void *data, unsigned int startline, unsigned long wihtd) {
	assert(0);

}

void Screen_Shift(int x, int y) {
	assert(0);

}

void ShareCPU(void) {
	assert(0);

}

char Sound_SetEffect(int filter, int data) {
	assert(0);
	return 0;
}

int Sound_GetEffect(int filter) {
	assert(0);
	return 0;
}

char Sound_CheckEffect(int filter) {
	assert(0);
	return 0;
}

char Sound_IsActive(void) {
	assert(0);
	return 0;
}

void Sound_SetVolume(int channel, int left, int right) {
	assert(0);

}

void Sound_GetVolume(int channel, int *left, int *right) {
	assert(0);

}

char Sound_GetChannelState(int channel) {
	assert(0);
	return 0;
}

int Sound_MixBack(int synchro) {
	assert(0);
	return 0;
}

void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type) {
	assert(0);

}

void Sound_ChangeMusic(char *file) {
	assert(0);

}

void Sound_BreakLoop(int channel) {
	assert(0);

}

void Sound_BreakExt(int channel, void *sample, long size) {
	assert(0);

}

void Sound_Mute(int channel) {
	assert(0);

}

void Sound_StopMixing(void) {
	assert(0);

}

void Sound_StartMixing(void) {
	assert(0);

}

void Sound_SetMixer(int dev, int freq, ...) {
	assert(0);

}

int Task_Add(int stack, TaskerFunctionName func, ...) {
	assert(0);
	return 0;
}

void *Task_Sleep(void *data) {
	assert(0);
	return NULL;
}

void *Task_WaitEvent(long event) {
	assert(0);
	return NULL;
}

char Task_IsMaster(void) {
	assert(0);
	return 0;
}

int Task_Count(void) {
	assert(0);
	return 0;
}

void Task_Wakeup(EVENT_MSG *msg) {
	assert(0);

}

void Task_Term(int id) {
	assert(0);

}

char Task_IsRunning(int id) {
	assert(0);
	return 0;
}

char Task_QuitMsg() {
	assert(0);
	return 0;
}

void Task_Shutdown(int id) {
	assert(0);

}

int Timer_GetValue(void) {
	assert(0);
	return 0;
}

int Timer_GetTick(void) {
	assert(0);
	return 0;
}

void Timer_Sleep(int msec) {
	assert(0);

}

int get_control_state(void) {
	assert(0);
	return 0;
}

int get_shift_state(void) {
	assert(0);
	return 0;
}

void *LoadDefaultFont(void) {
//	assert(0);
//	return NULL;
	return Sys_ReadFile(Sys_FullPath(SR_FONT, "BOLDCZ.FON"));
}

void *PrepareVideoSound(int mixfreq, int buffsize) {
	assert(0);
	return NULL;
}

void DoneVideoSound(void *buffer) {
	assert(0);

}

char LoadNextVideoFrame(void *buffer, char *data, int size, short *xlat, short *accnums, long *writepos) {
	assert(0);
	return 0;
}

void *OpenMGFFile(const char *filename) {
	assert(0);
	return NULL;
}

void CloseMGFFile(void *file) {
	assert(0);

}

// FIXME: implement game launcher
char OtevriUvodniOkno() {
	return 1;
}

char SelectAdventure() {
	assert(0);
	return 0;
}

char *GetSelectedAdventure() {
	assert(0);
	return NULL;
}

