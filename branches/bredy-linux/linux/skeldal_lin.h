/*
 * skeldal_lin.h
 *
 *  Created on: 22.9.2013
 *      Author: ondra
 */

#ifndef SKELDAL_LIN_H_
#define SKELDAL_LIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _tag_MEMORYSTATUS {

	unsigned long dummy;

} MEMORYSTATUS;

#define MAX_PATH 258

#define RGB888(r,g,b) ((unsigned short)((((r)<<8)&0xF800) | (((g)<<3) & 0x7C0) | ((b)>>3)))
#define RGB555(r,g,b) ((unsigned short)(((r)<<11) | ((g)<<6) | (b)))
#define RGB(r,g,b) (((r)>>3)*2048+((g)>>3)*64+((b)>>3))

#define _A_NORMAL 0

#define _KEYBRD_READY 0
#define _KEYBRD_READ 1
unsigned long _bios_keybrd(int mode);
unsigned char isKeyCTRLPressed();
void Beep(int freq, int delay);


int DxGetResX();
int DxGetResY();

void strupr(char *txt);
char *itoa(int value, char *buffer, int base);

size_t _msize(void *ptr);
const void *LoadResourceFont(const char *name);

void DxSetInitResolution(int x, int y);
int DXInit64(char inwindow, char zoom, char monitor, int refresh);
void DXCloseMode();
void DXCopyRects64(int x,int y,int xs,int ys);
int _access(const char *fname, int mode);

#endif /* SKELDAL_LIN_H_ */
