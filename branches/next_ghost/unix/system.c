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
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "libs/system.h"
#include "game/globals.h"

char *strupr(char *str) {
	int i;
	for (i = 0; str[i]; i++) {
		str[i] = toupper(str[i]);
	}
	return str;
}

// FIXME: implement mouse support
void Mouse_MapWheel(char up, char down) {
//	assert(0);

}

// FIXME: implement keyboard support
int Input_Kbhit(void) {
//	assert(0);
	return 0;
}

// TODO: implement these
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

// FIXME: move this to generic renderer
void Screen_StripBlt(word *data, unsigned int startline, unsigned long width) {
	int i;
	word *dst = Screen_GetAddr() + startline * Screen_GetXSize();

	for (i = 0; i < width; i++) {
		memcpy(dst, data, 640 * sizeof(word));
		data += 640;
		dst += Screen_GetXSize();
	}
}

void Screen_Shift(int x, int y) {
	assert(0);

}

// FIXME: rewrite
void ShareCPU(void) {
//	assert(0);

}

// FIXME: implement sound backend
char Sound_SetEffect(int filter, int data) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
int Sound_GetEffect(int filter) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
char Sound_CheckEffect(int filter) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
char Sound_IsActive(void) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
void Sound_SetVolume(int channel, int left, int right) {
//	assert(0);

}

void Sound_GetVolume(int channel, int *left, int *right) {
	assert(0);

}

// FIXME: implement sound backend
char Sound_GetChannelState(int channel) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
int Sound_MixBack(int synchro) {
//	assert(0);
	return 0;
}

// FIXME: implement sound backend
void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type) {
//	assert(0);

}

// FIXME: implement music backend
void Sound_ChangeMusic(char *file) {
//	assert(0);

}

void Sound_BreakLoop(int channel) {
	assert(0);

}

void Sound_BreakExt(int channel, void *sample, long size) {
	assert(0);

}

// FIXME: implement sound backend
void Sound_Mute(int channel) {
//	assert(0);

}

// FIXME: implement sound backend
void Sound_StopMixing(void) {
//	assert(0);

}

// FIXME: implement sound backend
void Sound_StartMixing(void) {
//	assert(0);

}

// FIXME: implement sound backend
void Sound_SetMixer(int dev, int freq, ...) {
//	assert(0);

}

int Task_Add(int stack, TaskerFunctionName func, ...) {
	assert(0);
	return 0;
}

void *Task_Sleep(void *data) {
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

// FIXME: implement keyboard input
int get_control_state(void) {
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

// FIXME: implement sound backend
void *PrepareVideoSound(int mixfreq, int buffsize) {
//	assert(0);
	return NULL;
}

// FIXME: implement sound backend
void DoneVideoSound(void *buffer) {
//	assert(0);

}

// FIXME: implement sound backend
char LoadNextVideoFrame(void *buffer, char *data, int size, short *xlat, short *accnums, long *writepos) {
//	assert(0);
	return 1;
}

// FIXME: rewrite properly
static int mgf = -1, mgf_len;
void *OpenMGFFile(const char *filename) {
	struct stat st;
	void *ret;

	assert(mgf == -1);
	mgf = open(filename, O_RDONLY);
	assert(mgf >= 0);

	fstat(mgf, &st);
	mgf_len = st.st_size;
	ret = mmap(NULL, mgf_len, PROT_READ, MAP_SHARED, mgf, 0);

	return ret;
}

void CloseMGFFile(void *file) {
	munmap(file, mgf_len);
	close(mgf);
	mgf = -1;
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

