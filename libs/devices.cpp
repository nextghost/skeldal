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
#include <cstdio>
#include "libs/event.h"
#include "libs/devices.h"
#include "libs/system.h"
#include <ctime>

static char ms_keys;

char cz_table_1[]=" 1!3457≠908+,-./Ç+à®á©ëò†°\"ñ?=:_2ABCDEFGHIJKLMNOPQRSTUVWXYZ£\\)6=;abcdefghijklmnopqrstuvwxyz/|(; ";
char cz_table_2[]=" !\"#$%&'()*+,-./0123456789:;<=>?@èBCDêFGHãJKäMNïPQ´STóVWXùZ[\\]^_`†bcdÇfgh°jkçmn¢pq™st£vwxòz{|}~ ";
char cz_table_3[]=" !\"#$%&'()*+,-./0123456789:;<=>?@ABÄÖâFGHIJKúM•ßPQûõÜ¶VWXYí[\\]^_`abáÉàfghijkåm§ìpq©®üñvwxyë{|}~ ";
char *cz_key_tabs[]={cz_table_1,cz_table_2,cz_table_3};

void keyboard(EVENT_MSG *msg, void *user_data) {
	if (msg->msg == E_WATCH) {
		Sys_ProcessEvents();
	}
}

char ms_get_keycount()
  {
  return ms_keys;
  }
