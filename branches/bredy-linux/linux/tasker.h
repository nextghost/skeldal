
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
 *  Last commit made by: $Id: FCS_Tasker.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef SKELDAL_LINUX_TASKER_H_
#define SKELDAL_LINUX_TASKER_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*TaskerFunctionName)(va_list);

void tasker(EVENT_MSG *msg,void **data);
int create_task();
int add_task(int stack,TaskerFunctionName fcname,...);
void term_task(int id_num);
char is_running(int id_num);
void suspend_task(int id_num,int msg);
void shut_down_task(int id_num);
void unsuspend_task(EVENT_MSG *msg);
void *task_sleep(void *data);
void *task_wait_event(long event_number);
int q_any_task();
char task_quitmsg();
char task_quitmsg_by_id(int id);
char q_is_mastertask();
int q_current_task();


#ifdef __cplusplus
}
#endif

#endif


