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

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <cstdio>
#include <cstdarg>
#include <inttypes.h>
#include "libs/devices.h"
#include "libs/memman.h"

#ifdef WIN32
#include "windows/system.h"
#else
#include "unix/system.h"
#endif

// FIXME: get rid of alloca
#ifdef HAVE_ALLOCA_H
	# include <alloca.h>
#elif defined __GNUC__
	#undef alloca
	# define alloca __builtin_alloca
#elif defined _AIX
	#undef alloca
	# define alloca __alloca
#elif defined _MSC_VER
	#undef alloca
	# include <malloc.h>
	# define alloca _alloca
#endif

#define E_RELOADMAP 40
#define TIMERSPEED 20
#define SKELDALINI "skeldal.ini"

typedef void (*TaskerFunctionName)(va_list);

void Mouse_GetEvent(MS_EVENT *event);
void Mouse_MapWheel(char up, char down);

int Input_Kbhit(void);
// Note: do not use for reading keys, set event handler instead
int Input_ReadKey(void);

void Sys_ErrorBox(const char *msg);
void Sys_WarnBox(const char *msg);
void Sys_InfoBox(const char *msg);

void Sys_Shutdown(void);
void Sys_SetEnv(const char *name, const char *value);
void Sys_Init(void);
void Sys_Mkdir(const char *path);
int Sys_LatestFile(const char *mask, int offset);
void Sys_PurgeTemps(char z);
int Sys_PackStatus(WriteStream &stream);
void Sys_SetPath(unsigned idx, const char *path);
char *Sys_FullPath(unsigned idx, const char *file);
char *Sys_DOSPath(unsigned defdir, const char *path);
int Sys_FileExists(const char *file);
void Sys_PreparePaths(void);
void Sys_ProcessEvents(void);

char Screen_Init(char windowed, int zoom, int monitor, int refresh);
void Screen_Shutdown(void);

void ShareCPU(void);

char Sound_SetEffect(int filter, int data);
int Sound_GetEffect(int filter);
char Sound_CheckEffect(int filter);
char Sound_IsActive(void);
void Sound_SetVolume(int channel, int left, int right);
void Sound_GetVolume(int channel, int *left, int *right);
char Sound_GetChannelState(int channel);
int Sound_MixBack(int synchro);
void Sound_PlaySample(int channel, const void *sample, long size, long lstart, long lend, long sfreq, int type, int continuous = 0);
void Sound_ChangeMusic(const char *file);
void Sound_BreakLoop(int channel);
void Sound_BreakExt(int channel);
void Sound_Mute(int channel);
void Sound_StopMixing(void);
void Sound_StartMixing(void);
void Sound_SetMixer(int dev, int freq, ...);

int Task_Add(int stack, TaskerFunctionName func, ...);
void *Task_Sleep(void *data);
void Task_WaitEvent(long event);
char Task_IsMaster(void);
int Task_Count(void);
void Task_Wakeup(EVENT_MSG *msg);
void Task_Term(int id);
char Task_IsRunning(int id);
char Task_QuitMsg();
void Task_Shutdown(int id);

int Timer_GetValue(void);
int Timer_GetTick(void);
void Timer_Sleep(int msec);

int get_control_state(void);
int get_shift_state(void);

void *PrepareVideoSound(int mixfreq, int buffsize);
void DoneVideoSound(void *buffer);
int LoadNextAudioFrame(void *buff, const int16_t *data, unsigned length);

char OtevriUvodniOkno();
char SelectAdventure();
char *GetSelectedAdventure();

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)>(b)?(b):(a))

#endif
