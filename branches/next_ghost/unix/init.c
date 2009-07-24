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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "libs/system.h"
#include "game/globals.h"

// TODO: set this from configure script
#define DATA_PATH "/usr/share/games/skeldal/"

#define PATHTABLE_SIZE 18

static char *pathtable[18] = {0};

void Sys_SetPath(unsigned idx, const char *path) {
	assert(idx < PATHTABLE_SIZE);

	if (pathtable[idx]) {
		free(pathtable[idx]);
	}

	pathtable[idx] = malloc((strlen(path) + 1) * sizeof(char));
	strcpy(pathtable[idx], path);
}

char *Sys_FullPath(unsigned idx, const char *file) {
	static char ret[PATH_MAX];

	strcpy(ret, pathtable[idx]);
	strcat(ret, file);
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
	Sys_SetPath(SR_MAP, DATA_PATH "maps/");
	Sys_SetPath(SR_MUSIC, DATA_PATH "music/");
	Sys_SetPath(SR_WORK, home);
	Sys_SetPath(SR_TEMP, Sys_FullPath(SR_WORK, "temp/"));
	Sys_SetPath(SR_BGRAFIKA, DATA_PATH "graphics/basic/");
	Sys_SetPath(SR_ITEMS, DATA_PATH "graphics/items/");
	Sys_SetPath(SR_ENEMIES, DATA_PATH "graphics/enemies/");
	Sys_SetPath(SR_VIDEO, DATA_PATH "video/");
	Sys_SetPath(SR_DIALOGS, DATA_PATH "graphics/dialogs/");
	Sys_SetPath(SR_SAVES, Sys_FullPath(SR_WORK, "save/"));
	Sys_SetPath(SR_CD, DATA_PATH);
	Sys_SetPath(SR_MAP2, DATA_PATH);
	Sys_SetPath(SR_ORGMUSIC, DATA_PATH);
}
