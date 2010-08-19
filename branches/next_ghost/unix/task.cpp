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
#include "libs/event.h"
#include "libs/system.h"

#define MAX_WAITS 10	// 10 event waits should be enough for everyone

static int wait_used[MAX_WAITS] = {0};
static long wait_target[MAX_WAITS] = {0};

void Task_WaitEvent(long event) {
	int counter;

	// find empty wait slot
	for (counter = 0; counter < MAX_WAITS && wait_used[counter]; counter++);

	assert(counter < MAX_WAITS && "Too many waits for event");

	wait_used[counter] = 1;
	wait_target[counter] = event;

	do {
		do_events();
		Timer_Sleep(TIMERSPEED/2);
	} while (wait_target[counter]);

	wait_used[counter] = 0;
}

void Task_Wakeup(EVENT_MSG *msg) {
	int i;

	for (i = 0; i < MAX_WAITS; i++) {
		if (wait_target[i] == msg->msg) {
			wait_target[i] = 0;
		}
	}
}
