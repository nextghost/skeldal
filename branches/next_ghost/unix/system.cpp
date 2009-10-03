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

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <inttypes.h>

#include "libs/system.h"
#include "game/globals.h"
#include "libs/memman.h"

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

int Sys_LatestFile(const char *mask, int offset) {
	assert(0);
	return 0;
}

// FIXME: get rid of temps
void Sys_PurgeTemps(char all) {
	DIR *dir;
	struct dirent *ent;
	char *ptr, buf[PATH_MAX], *cat;

	strcpy(buf, Sys_FullPath(SR_TEMP, ""));
	cat = buf + strlen(buf);
	dir = opendir(buf);

	while ((ent = readdir(dir))) {
		ptr = strrchr(ent->d_name, '.');

		if (!ptr || strcasecmp(ptr, ".tmp")) {
			continue;
		}

		if ((ent->d_name[0] == '~') && !all) {
			continue;
		}

		strcpy(cat, ent->d_name);
		unlink(buf);
	}

	closedir(dir);
}

int pack_status_file(WriteStream&, const char*);

int Sys_PackStatus(WriteStream &stream) {
	DIR *dir;
	struct dirent *ent;
	char *ptr, buf[PATH_MAX], *cat;
	int ret;

	strcpy(buf, Sys_FullPath(SR_TEMP, ""));
	cat = buf + strlen(buf);
	dir = opendir(buf);

	while ((ent = readdir(dir))) {
		ptr = strrchr(ent->d_name, '.');

		if (!ptr || strcasecmp(ptr, ".tmp") || (ent->d_name[0] == '~')) {
			continue;
		}

		ret = pack_status_file(stream, ent->d_name);

		if (ret) {
			return ret;
		}
	}

	closedir(dir);

	memset(buf, 0, 12);
	stream.write(buf, 12);

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
	len = ftell(fr);
	fseek(fr, 0, SEEK_SET);

	ret = malloc(len);
	fread(ret, 1, len, fr);
	return ret;
}

void Screen_DrawRectZoom2(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys) {
	assert(0);

}

// FIXME: move this to generic renderer
void Screen_StripBlt(uint16_t *data, unsigned int startline, unsigned long width) {
	int i;
	uint16_t *dst = Screen_GetAddr() + startline * Screen_GetXSize();

	for (i = 0; i < width; i++) {
		memcpy(dst, data, 640 * sizeof(uint16_t));
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

// FIXME: rewrite properly
static int mgf = -1, mgf_len;
void *OpenMGFFile(const char *filename) {
	struct stat st;
	void *ret;

	assert(mgf == -1);
	mgf = open(filename, O_RDONLY);

	if (mgf < 0) {
		fprintf(stderr, "Could not open file %s\n", filename);
		exit(1);
	}

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

