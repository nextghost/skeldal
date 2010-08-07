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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <sys/stat.h>
#include <sys/types.h>
#include "libs/system.h"
#include "game/globals.h"

// TODO: set this from configure script
#define DATA_PATH "/usr/local/games/skeldal/"

#define PATHTABLE_SIZE 20

static char *pathtable[PATHTABLE_SIZE] = {0};

void Sys_SetPath(unsigned idx, const char *path) {
	int len = strlen(path);
	assert(idx < PATHTABLE_SIZE);

	if (pathtable[idx]) {
		free(pathtable[idx]);
	}

	if (!*path) {
		pathtable[idx] = (char*)malloc(sizeof(char));
		pathtable[idx] = '\0';
		return;
	}

	if (path[len - 1] != '/') {
		len++;
	}

	pathtable[idx] = (char*)malloc((len + 1) * sizeof(char));
	strcpy(pathtable[idx], path);
	pathtable[idx][len-1] = '/';
	pathtable[idx][len] = '\0';
}

char *Sys_FullPath(unsigned idx, const char *file) {
	static char ret[PATH_MAX];

	strcpy(ret, pathtable[idx]);
	strncat(ret, file, PATH_MAX - strlen(ret) - 1);
	return ret;
}

char *Sys_DOSPath(unsigned defdir, const char *path) {
	static char ret[PATH_MAX];
	int i, length;

	strcpy(ret, pathtable[defdir]);
	length = strlen(ret);
	strncat(ret + length, path, PATH_MAX - length - 1);
	strupr(ret + length);

	for (i = length; ret[i]; i++) {
		if (ret[i] == '\\') {
			ret[i] = '/';
		}
	}

	return ret;
}

void Sys_Init(void) {
	char home[PATH_MAX];
	int len;

	strcpy(home, getenv("HOME"));
	len = strlen(home);

	if (home[len-1] != '/') {
		home[len++] = '/';
	}

	strcpy(home + len, ".skeldal/");

	Sys_SetPath(SR_DATA, DATA_PATH);
	Sys_SetPath(SR_GRAFIKA, DATA_PATH "graphics/");
	Sys_SetPath(SR_ZVUKY, DATA_PATH "samples/");
	Sys_SetPath(SR_FONT, DATA_PATH "font/");
	Sys_SetPath(SR_MAP, DATA_PATH "MAPS/");
	Sys_SetPath(SR_MUSIC, DATA_PATH "MUSIC/");
	Sys_SetPath(SR_WORK, home);
	Sys_SetPath(SR_TEMP, Sys_FullPath(SR_WORK, "temp/"));
	Sys_SetPath(SR_BGRAFIKA, DATA_PATH "graphics/basic/");
	Sys_SetPath(SR_ITEMS, DATA_PATH "graphics/items/");
	Sys_SetPath(SR_ENEMIES, DATA_PATH "graphics/enemies/");
	Sys_SetPath(SR_VIDEO, DATA_PATH "VIDEO/");
	Sys_SetPath(SR_DIALOGS, DATA_PATH "graphics/dialogs/");
	Sys_SetPath(SR_SAVES, Sys_FullPath(SR_WORK, "save/"));
	Sys_SetPath(SR_CD, DATA_PATH);
	Sys_SetPath(SR_MAP2, DATA_PATH);
	Sys_SetPath(SR_ORGMUSIC, DATA_PATH "MUSIC/");
	Sys_SetPath(SR_DEFAULT, "");
	Sys_SetPath(SR_HOME, home);
}

void Sys_MkDir(const char *path) {
	char buf[PATH_MAX], *ptr, *endptr;
	int length;
	struct stat tmp;

	strncpy(buf, path, PATH_MAX);
	buf[PATH_MAX] = '\0';

	for (length = strlen(buf); length > 1 && buf[length - 1] == '/'; length--);

	ptr = endptr = buf + length;
	*endptr = '\0';

	while (stat(buf, &tmp)) {
		ptr = strrchr(buf, '/');

		if (ptr) {
			*ptr = '\0';
		} else {
			return;
		}
	}

	while (ptr != endptr) {
		*ptr = '/';
		ptr += strlen(ptr);

		if (mkdir(buf, 0755)) {
			fprintf(stderr, "Could not create directory %s\n", buf);
			assert(0);
		}
	}
}

void Sys_PreparePaths(void) {
	Sys_MkDir(pathtable[SR_WORK]);
	Sys_MkDir(pathtable[SR_TEMP]);
	Sys_MkDir(pathtable[SR_SAVES]);
}
