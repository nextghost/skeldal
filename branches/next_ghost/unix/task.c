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
#include "libs/event.h"

static long wait_target = 0;
static EVENT_MSG wait_ret;

void *Task_WaitEvent(long event) {
	static int counter = 0;

	assert(++counter <= 1 && "Only one task can wait for events");

	wait_target = event;
	do {
		do_events();
	} while (wait_target);

	--counter;
	return &wait_ret;
}

void Task_Wakeup(EVENT_MSG *msg) {
	if (msg->msg != wait_target) {
		return;
	}

	wait_ret = *msg;
	wait_target = 0;
}
