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
int Sys_FileExists(const char *file);
void *Sys_ReadFile(const char *file);
void Sys_PreparePaths(void);
void Sys_ProcessEvents(void);

char Screen_Init(char windowed, int zoom, int monitor, int refresh);
int Screen_GetXSize(void);
int Screen_GetYSize(void);
uint16_t *Screen_GetAddr(void);
uint16_t *Screen_GetBackAddr(void);
long Screen_GetSize(void);
void Screen_SetAddr(unsigned short *addr);
void Screen_SetBackAddr();
void Screen_Restore(void);
void Screen_DrawRect(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys);
void Screen_DrawRectZoom2(unsigned short x, unsigned short y, unsigned short xs, unsigned short ys);
void *Screen_PrepareWalk(int ypos);
void Screen_ZoomWalk(void *handle, int ypos, int *points,float phase, void *lodka);
void Screen_DoneWalk(void *handle);
void Screen_Shutdown(void);
int Screen_GetScan(void);

void Screen_StripBlt(uint16_t *data, unsigned int startline, unsigned long width);
void Screen_Shift(int x, int y);

void Screen_FixPalette(uint16_t *pal, int size);
void Screen_FixMGIFPalette(uint16_t *pal, int size);
uint16_t Screen_RGB(unsigned r, unsigned g, unsigned b);
uint16_t Screen_ColorMin(uint16_t c1, uint16_t c2);
uint16_t Screen_ColorSub(uint16_t color, int sub);
uint16_t Screen_ColorAvg(uint16_t c1, uint16_t c2);
uint16_t Screen_ColorBlend(uint16_t c1, uint16_t c2, float factor);

unsigned Screen_ColorR(uint16_t c);
unsigned Screen_ColorG(uint16_t c);
unsigned Screen_ColorB(uint16_t c);


void ShareCPU(void);

char Sound_SetEffect(int filter, int data);
int Sound_GetEffect(int filter);
char Sound_CheckEffect(int filter);
char Sound_IsActive(void);
void Sound_SetVolume(int channel, int left, int right);
void Sound_GetVolume(int channel, int *left, int *right);
char Sound_GetChannelState(int channel);
int Sound_MixBack(int synchro);
void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type);
void Sound_ChangeMusic(const char *file);
void Sound_BreakLoop(int channel);
void Sound_BreakExt(int channel, void *sample, long size);
void Sound_Mute(int channel);
void Sound_StopMixing(void);
void Sound_StartMixing(void);
void Sound_SetMixer(int dev, int freq, ...);

int Task_Add(int stack, TaskerFunctionName func, ...);
void *Task_Sleep(void *data);
void *Task_WaitEvent(long event);
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

void *LoadDefaultFont(void);
void *PrepareVideoSound(int mixfreq, int buffsize);
void DoneVideoSound(void *buffer);
char LoadNextVideoFrame(void *buffer, uint8_t *data, int size, short *xlat, short *accnums, long *writepos);

char OtevriUvodniOkno();
char SelectAdventure();
char *GetSelectedAdventure();

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)>(b)?(b):(a))

#endif
