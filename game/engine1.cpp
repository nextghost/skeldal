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
#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "libs/memman.h"
#include "libs/bgraph.h"
#include "libs/event.h"
#include "libs/mgifmem.h"
#include "game/globals.h"
#include "game/engine1.h"
#include "libs/system.h"

#define CTVR 128

t_points points;
struct all_view showtabs;
static char backgrnd_mode=0;

static int lclip,rclip;

char zooming_xtable[ZOOM_PHASES][VIEW_SIZE_X];
short zooming_ytable[ZOOM_PHASES][VIEW_SIZE_Y];
short zooming_points[ZOOM_PHASES][4]
  ={
     {620,349,10,3},
     {600,338,20,7},
     {580,327,30,11},
     {560,316,40,14},
     {540,305,50,18},
     {520,293,60,21},
     {500,282,70,25},
     {480,271,80,28},
     {460,259,90,31}
  };
int zooming_step=1;
int rot_phases=1;
int yreq;
int last_scale;
char secnd_shade=1;

/*void zooming_dx(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming_dx parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void zooming32(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming32 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void zooming32b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming32b parm [ESI][EDI][EAX][EBX][ECX] modify [EAX EDX]
void zooming_lo(void *source,void *target,void *xlat,long xysize);
//#pragma aux zooming_lo parm [ESI][EDI][EBX][ECX] modify [EAX EDX]
void zooming256(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming256 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void zooming256b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming256b parm [ESI][EDI][EAX][EBX][ECX] modify [EAX EDX]
void zooming64(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming64 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void zooming64b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux zooming64b parm [ESI][EDI][EAX][EBX][ECX] modify [EAX EDX]
void scroll_support_dx(void *lbuf,void *src1,void *src2,int size1);
//#pragma aux scroll_support_dx parm [EDI][ESI][EDX][ECX] modify [EAX]
void scroll_support_32(void *lbuf,void *src1,void *src2,int size1);
//#pragma aux scroll_support_32 parm [EDI][ESI][EDX][ECX] modify [EAX]
void scroll_support_32b(void *lbuf,void *src1,void *src2,int size1);
//#pragma aux scroll_support_32b parm [EDI][ESI][EDX][ECX] modify [EAX]
void scroll_support_256(void *lbuf,void *src1,void *src2,int size1,void *xlat);
//#pragma aux scroll_support_256 parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];
void scroll_support_256b(void *lbuf,void *src1,void *src2,int size1,void *xlat);
//#pragma aux scroll_support_256b parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];
void scroll_support_64(void *lbuf,void *src1,void *src2,int size1,void *xlat);
//#pragma aux scroll_support_64 parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];
void scroll_support_64b(void *lbuf,void *src1,void *src2,int size1,void *xlat);
//#pragma aux scroll_support_64b parm [EDI][ESI][EDX][ECX][EBX] modify [EAX];*/
void fcdraw(uint16_t *source,uint16_t *target, T_FLOOR_MAP *table);
//#pragma aux fcdraw parm [EDX][EBX][EAX] modify [ECX ESI EDI];

/*void lodka32(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka32 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void lodka_dx(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka_dx parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void lodka32b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka32b parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]

void lodka256(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka256 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void lodka256b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka256b parm [ESI][EDI][EAX][EBX][ECX] modify [EAX EDX]

void lodka64(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka64 parm [ESI][EDI][EAX][EBX][ECX] modify [EDX]
void lodka64b(void *source,void *target,void *background,void *xlat,long xysize);
//#pragma aux lodka64b parm [ESI][EDI][EAX][EBX][ECX] modify [EAX EDX]
*/

void *p,*p2,*pozadi,*podlaha,*strop,*sit;int i;
void (*zooming)(void *source,long target,uint16_t *background,void *xlat,long xysize);
void (*turn)(long lbuf,void *src1,void *src2,int size1);
uint16_t *Screen_GetBackAddr();
uint16_t *background;
char debug=0,nosides=0,nofloors=0,drwsit=0,show_names=0,show_lives=0;
static long old_timer;

static void wait_timer()
  {
  Timer_Sleep(10);
  }

/*void zooming1(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  wait_timer();
  if (backgrnd_mode)
    lodka_dx(source,Screen_GetAddr()+target,background+3,xlat,xysize);
  else
    zooming_dx(source,Screen_GetAddr()+target,background+3,xlat,xysize);
  showview(0,0,0,0);
  }
/*
void zooming2(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  uint16_t *lbuffer=LockDirectScreen();
  wait_timer();
  if (backgrnd_mode)
    lodka256(source,lbuffer+(target>>1),background+3,xlat,xysize);
  else
    zooming256(source,lbuffer+(target>>1),background+3,xlat,xysize);
  UnlockDirectScreen();
  }

void zooming3(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  source;target;background;xlat;xysize;
  }

/*void zooming4(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  uint16_t *lbuffer=LockDirectScreen();
  wait_timer();
  if (backgrnd_mode)
    lodka32b(source,(void *)(target*2),background+3,xlat,xysize);
  else
    zooming32b(source,(void *)(target*2),background+3,xlat,xysize);
  UnlockDirectScreen();
  }*/
/*
void zooming5(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  wait_timer();
  if (backgrnd_mode)
     lodka256b(source,(void *)target,background+3,xlat,xysize);
  else
     zooming256b(source,(void *)target,background+3,xlat,xysize);
  }

void zooming6(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  uint16_t *lbuffer=LockDirectScreen();
  wait_timer();
  if (backgrnd_mode)
    lodka_dx(source,lbuffer+(target),background+3,xlat,xysize);
  else
    zooming_dx(source,lbuffer+(target),background+3,xlat,xysize);
  UnlockDirectScreen();
  }
/*
void zooming7(void *source,long target,uint16_t *background,void *xlat,long xysize)
  {
  wait_timer();
  if (backgrnd_mode)
     lodka64b(source,(void *)(target*2),background+3,xlat,xysize);
  else
     zooming64b(source,(void *)(target*2),background+3,xlat,xysize);
  }
*/

void turn1(long lbuf,void *src1,void *src2,int size1)
  {
//wait_timer();
//     scroll_support_dx(lbuf+Screen_GetAddr(),src1,src2,size1);
  showview(0,0,0,0);
  }

/*void turn2(long lbuf,void *src1,void *src2,int size1)
  {
  wait_timer();
     scroll_support_256((lbuf>>1)+lbuffer,src1,src2,size1,xlatmem);
  }
*/
void turn3(long lbuf,void *src1,void *src2,int size1)
  {
  lbuf;src1;src2;size1;
  }
/*
void turn4(long lbuf,void *src1,void *src2,int size1)
  {
  wait_timer();
     scroll_support_32b((void *)(lbuf*2),src1,src2,size1);
  }

void turn5(long lbuf,void *src1,void *src2,int size1)
  {
  wait_timer();
     scroll_support_256b((void *)lbuf,src1,src2,size1,xlatmem);
  }
*/
/*
void turn6(long lbuf,void *src1,void *src2,int size1)
  {
  uint16_t *lbuffer=LockDirectScreen();
  wait_timer();
     scroll_support_dx((lbuf)+lbuffer,src1,src2,size1,xlatmem);
  UnlockDirectScreen();
  }
/*
void turn7(long lbuf,void *src1,void *src2,int size1)
  {
  wait_timer();
     scroll_support_64b((void *)(lbuf*2),src1,src2,size1,xlatmem);
  }
*/


void calc_points(void)
  {
  int i,j,x1,y1,x2,y2;

  for (j=0;j<VIEW3D_X+1;j++)
     {
  x1=START_X1+2*START_X1*j;y1=START_Y1;
  x2=START_X2+2*START_X1*j;y2=START_Y2;
  for (i=0;i<VIEW3D_Z+1;i++)
     {
     points[j][0][i].x=x1;
     points[j][0][i].y=y1;
     points[j][1][i].x=x2;
     points[j][1][i].y=y2;
     x2=(int)(x2-x2/FACTOR_3D);
     y2=(int)(y2-y2/FACTOR_3D);
     x1=(int)(x1-x1/FACTOR_3D);
     y1=(int)(y1-y1/FACTOR_3D);
     }
     }
  }

void calc_x_buffer(int32_t *ptr,long txt_size_x, long len,long total,long scale1)
  {
  int i,j,old,z=-1;

  old=-1;
  for (i=0;i<total;i++)
     {
     if (old>txt_size_x)
        {
        if (z==-1) z=i-3;
        j=((i-z)*txt_size_x)/scale1+txt_size_x;
        }
        else j=(i*txt_size_x)/len;
     *ptr++=(j-old-1);
     old=j;
     }

  }

void calc_y_buffer(short *ptr,long txt_size_y, long len,long total)
  {
  int i,j,old;

  old=-1;
  for (i=0;i<total;i++)
     {
     j=(i*txt_size_y)/len;
     *(ptr++)=(j-old);
     old=j;
     }
  }


void create_tables(void) {
	int x, y;

	for (y = 0; y < VIEW3D_Z + 1; y++) {
		showtabs.y_table[y].vert_size = points[1][0][y].y - points[1][1][y].y;
		showtabs.y_table[y].vert_total = (points[1][0][y].y + MIDDLE_Y);
		showtabs.y_table[y].drawline = ((points[1][0][y].y + MIDDLE_Y));
		calc_y_buffer(showtabs.y_table[y].zoom_table, TXT_SIZE_Y, showtabs.y_table[y].vert_size, TAB_SIZE_Y);
	}

	for (y = 0; y < VIEW3D_Z; y++) {
		for (x = 0; x < VIEW3D_X; x++) {
			int rozdil1, rozdil2, rozdil3;

			if (points[x][0][y+1].x > MIDDLE_X) {
				showtabs.z_table[x][y].used = 0;
			} else {
				showtabs.z_table[x][y].used = 1;
				rozdil1 = points[x][0][y].x - points[x][0][y+1].x;
				rozdil2 = rozdil1 - MIDDLE_X + points[x][0][y+1].x;
				rozdil3 = points[0][0][y].x - points[0][0][y+1].x;

				if (rozdil2 < 0) {
					showtabs.z_table[x][y].xpos = MIDDLE_X - points[x][0][y].x;
					showtabs.z_table[x][y].txtoffset = 0;
				} else {
				   showtabs.z_table[x][y].xpos = MIDDLE_X - points[x][0][y].x;
				   showtabs.z_table[x][y].txtoffset = (TXT_SIZE_X_3D * rozdil2 / rozdil1);
				}

				showtabs.z_table[x][y].point_total = rozdil1;
				calc_x_buffer(showtabs.z_table[x][y].zoom_table, TXT_SIZE_X_3D, rozdil1, VIEW_SIZE_X, rozdil3);
			}
		}
	}

	for (y = 0; y < VIEW3D_Z + 1;y++) {
		for (x = 0; x < VIEW3D_X; x++) {
			int rozdil1, rozdil2;

			if (x && points[x-1][0][y].x > TXT_SIZE_X) {
				showtabs.z_table[x][y].used = 0;
			} else {
				showtabs.x_table[x][y].used = 1;
				rozdil1 = points[1][0][y + 1].x - points[0][0][y + 1].x;
				rozdil2 = -MIDDLE_X + points[x][0][y + 1].x;

				if (rozdil2 < 0) {
					showtabs.x_table[x][y].xpos = MIDDLE_X - points[x][0][y + 1].x;
					showtabs.x_table[x][y].txtoffset = 0;
					showtabs.x_table[x][y].max_x = rozdil1;
				} else {
					showtabs.x_table[x][y].xpos = MIDDLE_X - points[x][0][y + 1].x;
					showtabs.x_table[x][y].txtoffset = (TXT_SIZE_X * rozdil2 / rozdil1);
					showtabs.x_table[x][y].max_x = MIDDLE_X - points[x - 1][0][y + 1].x;
				}

				if (x != 0) {
					showtabs.x_table[x][y].xpos2 = VIEW_SIZE_X - (showtabs.x_table[x][y].xpos + rozdil1);
				}

				showtabs.x_table[x][y].point_total = rozdil1;
				calc_x_buffer(showtabs.x_table[x][y].zoom_table, TXT_SIZE_X, rozdil1, VIEW_SIZE_X, rozdil1);
			}
		}
	}

	for (x = 0; x < CF_XMAP_SIZE; x++) {
		for (y = 0; y < F_YMAP_SIZE; y++) {
			int xl, xr, y1, yp, strd;

			strd = CF_XMAP_SIZE >> 1;
			y1 = (VIEW_SIZE_Y - y) - MIDDLE_Y;
			yp=1;

			while (points[0][0][yp].y > y1) {
				yp++;
			}

			if (x < strd) {
				xl = -points[strd - x][0][0].x;
				xr = -points[strd - x - 1][0][0].x;
			} else if (x == strd) {
				xl = -points[0][0][0].x;
				xr = +points[0][0][0].x;
			} else if (x > strd) {
				xl = +points[x - strd - 1][0][0].x;
				xr = +points[x - strd][0][0].x;
			}

			y1 = (VIEW_SIZE_Y-y) - MIDDLE_Y;
			xl = xl * (y1 + 1) / points[0][0][0].y + MIDDLE_X;
			xr = xr * (y1 + 1) / points[0][0][0].y + MIDDLE_X;

			if (xl < 0) {
				xl = 0;
			}

			if (xr < 0) {
				xr = 0;
			}

			if (xl > 639) {
				xl = 639;
			}

			if (xr > 639) {
				xr = 639;
			}

			showtabs.f_table[x][y].x = xl;
			showtabs.f_table[x][y].dsty = y1 + MIDDLE_Y;
			showtabs.f_table[x][y].srcy = y1 + MIDDLE_Y - VIEW_SIZE_Y + F_YMAP_SIZE;
			showtabs.f_table[x][y].linewidth = xr - xl + (xl != xr);
			showtabs.f_table[x][y].counter = y1 - points[0][0][yp].y;
		}
	}

	for (x = 0; x < CF_XMAP_SIZE; x++) {
		for (y = 0; y < C_YMAP_SIZE; y++) {
			int xl, xr, y1, yp, strd;

			strd = CF_XMAP_SIZE >> 1;
			y1 = y - MIDDLE_Y;
			yp = 1;

			while (points[0][1][yp].y < y1) {
				yp++;
			}

			if (x < strd) {
				xl = -points[strd - x][1][0].x;
				xr = -points[strd - x - 1][1][0].x;
			} else if (x == strd) {
				xl = -points[0][1][0].x;
				xr = +points[0][1][0].x;
			} else if (x > strd) {
				xl = +points[x - strd - 1][1][0].x;
				xr = +points[x - strd][1][0].x;
			}

			xl = xl * (y1 - 2) / points[0][1][0].y + MIDDLE_X;
			xr = xr * (y1 - 2) / points[0][1][0].y + MIDDLE_X;

			if (xl < 0) {
				xl = 0;
			}

			if (xr < 0) {
				xr = 0;
			}

			if (xl > 639) {
				xl = 639;
			}

			if (xr > 639) {
				xr = 639;
			}

			showtabs.c_table[x][y].x = xl;
			showtabs.c_table[x][y].srcy = y1 + MIDDLE_Y;
			showtabs.c_table[x][y].dsty = y1 + MIDDLE_Y;
			showtabs.c_table[x][y].linewidth = xr - xl + (xl != xr);
			showtabs.c_table[x][y].counter = points[0][1][yp].y - y1;
		}
	}
}

void calc_zooming(char *buffer,int dvojice,int oldsiz)
  {
  int poz,roz,i,x;

  poz=-2;
  for(i=0;i<dvojice;i++)
     {
     x=(i*oldsiz/dvojice);
     roz=x-poz;
     if (roz>2) roz=2;
     if (roz<1) roz=1;
     if (roz==1) *buffer++=1; else *buffer++=0;
     poz+=roz;
     }
  }

void create_zooming(void) {
	int i, j;

	for (j = 0; j < ZOOM_PHASES; j++) {
		calc_zooming(zooming_xtable[j], 320, zooming_points[j][0]);
		calc_y_buffer(zooming_ytable[j], zooming_points[j][1], 360, 360);

		for (i = 0; i < 360; i++) {
			zooming_ytable[j][i] *= renderer->width();
		}
	}
}

static Texture *stepzoom(const Texture &tex, float phase, int *points) {
	int pts[4], i, j, width, height, x, y;
	float xcoef, ycoef;
	uint8_t *data;
	Texture *ret;

	width = tex.width();
	height = tex.height();
	data = new uint8_t[width * height * 3];
	phase = phase < 0 ? 0 : phase;
	phase = phase > 1 ? 1 : phase;
	pts[0] = phase * points[0];
	pts[1] = phase * points[1];
	pts[2] = width * (1.0f - phase) + phase * points[2];
	pts[3] = height * (1.0f - phase) + phase * points[3];

	xcoef = (float)(pts[2] - pts[0]) / (float)width;
	ycoef = (float)(pts[3] - pts[1]) / (float)height;

	for (i = 0; i < height; i++) {
		y = pts[1] + (int)(i * ycoef);

		for (j = 0; j < width; j++) {
			x = pts[0] + (int)(j*xcoef);
			data[3 * (j + i * width)] = tex.pixels()[3 * (x + y * width)];
			data[3 * (j + i * width) + 1] = tex.pixels()[3 * (x + y * width) + 1];
			data[3 * (j + i * width) + 2] = tex.pixels()[3 * (x + y * width) + 2];
		}
	}

	ret = new TextureHi(data, width, height);
	delete[] data;
	return ret;
}

void zooming_forward_backward(const Texture &tex, int back) {
	if (!zooming_step) {
		return;  
	}

	long tmp = Timer_GetValue();
	int tpoints[4] = {90, 31, 90 + 460, 31 + 259};
	int maxtime = 5 * zoom_speed(-1);
	int curtime;
	float phase;
	SubTexture sub(tex, 0, SCREEN_OFFLINE, tex.width(), 360);
	Texture *tex2;

	do {
		curtime = Timer_GetValue() - tmp;
		phase = (curtime) * (1.0f / (float)maxtime);
		//phase=(float)sin(3.14159265*0.5f*phase);

		if (back) {
			phase = 1.0 - phase;
		}

		tex2 = stepzoom(sub, phase, tpoints);
		renderer->blit(*tex2, 0, SCREEN_OFFLINE);
		renderer->drawRect(0, SCREEN_OFFLINE, renderer->width(), 360);
		delete tex2;
		do_events();
	} while (curtime < maxtime);
}

void turn_left_right(const Texture &ltex, const Texture &rtex, int right) {
	if (!rot_phases) {
		return;  
	}

	long tmp = Timer_GetValue();

	int maxtime = 5 * rot_phases;
	int curtime;
	int x, width;
	float phase;
	int last = 90;

	do {
		curtime = Timer_GetValue() - tmp;
		phase = (curtime) * (1.0f / (float)maxtime);

		if (right) {
			phase = 1.0f - phase;
		}

		//phase=(float)sin(3.14159265*0.5f*phase);
		// FIXME: rewrite
		width = ltex.width() - 180;
		x = width * phase;
		renderer->rectBlit(ltex, 0, SCREEN_OFFLINE, x, SCREEN_OFFLINE, width + 90 - x, 360);
		renderer->rectBlit(rtex, width + 90 - x, SCREEN_OFFLINE, 90, SCREEN_OFFLINE, width + 90, 360);
		renderer->drawRect(0, SCREEN_OFFLINE, renderer->width(), 360);
		do_events();
	} while (curtime < maxtime);
}

void show_cel(int celx, int cely, const Texture &tex, int xofs, int yofs, char rev) {
	T_INFO_X_3D *x3d, *x0d;
	T_INFO_Y *yd, *yp;
	int txtsx, txtsy, realsx, realsy, x, i, yss, ysd;
	char *p;
	int plac;

	plac = rev >> 5;
	rev &= 3;

	if (celx <= 0) {
		x3d = &showtabs.z_table[-celx][cely];
	} else {
		x3d = &showtabs.z_table[celx][cely];
	}

	x0d = &showtabs.z_table[0][cely];

	if (!x3d->used) {
		return;
	}

	yd = &showtabs.y_table[cely];
	yp = &showtabs.y_table[cely+1];
	txtsx = tex.width();
	txtsy = tex.height();

	if (rev < 2) {
		xofs -= (txtsx >> 1) * TXT_SIZE_X / TXT_SIZE_X_3D;
		yofs -= txtsy >> 1;
	}

	rev &= 1;
	yss = (points[0][0][cely].y - points[0][0][cely + 1].y) * xofs / TXT_SIZE_X;
	ysd = (points[0][1][cely].y - points[0][1][cely + 1].y) * xofs / TXT_SIZE_X;
	yofs = yofs * (yd->vert_size - yss + ysd) / TXT_SIZE_Y + yss;
	xofs = xofs * x3d->point_total / TXT_SIZE_X;

	if (txtsx > x3d->point_total && celx) {
		realsx = txtsx * x0d->point_total / TXT_SIZE_X_3D;
		realsx += x3d->point_total - x0d->point_total;
	} else {
		realsx = txtsx * x3d->point_total / TXT_SIZE_X_3D;
	}

	realsy = txtsy * yd->vert_size / TXT_SIZE_Y - 1;
	x = x3d->xpos + xofs;

	if (-x > realsx) {
		return;
	}

	if (x + realsx > 640) {
		realsx = 640 - x;
	}

	if (realsx <= 0) {
		return;
	}

	yofs = yd->drawline - yofs;
	yofs += (plac == 1) ? (-yp->vert_size + points[0][0][cely+1].y - points[0][0][cely].y) : ((plac == 2) ? (yd->vert_size) : 0);

	if (yofs - realsy < 0) {
		realsy = yofs;
	}

	renderer->wallBlit(tex, x, yofs + SCREEN_OFFLINE, x3d->zoom_table, realsx, yd->zoom_table, realsy + 1, tex.palette(cely + (secnd_shade ? SHADE_STEPS : 0)), rev);
}


void show_cel2(int celx, int cely, const Texture &tex, int xofs, int yofs, char rev) {
	T_INFO_X *x3d;
	T_INFO_Y *yd;
	int txtsx, txtsy, realsx, realsy, x, i;
	char *p;
	int plac;

	plac = rev >> 5;
	rev &= 3;

	if (celx == 2) {
		x = celx;
	}

	if (rev == 2) {
		celx = -celx;
	}

	if (celx <= 0) {
		x3d = &showtabs.x_table[-celx][cely];
	} else {
		x3d = &showtabs.x_table[celx][cely];
	}

	if (!x3d->used) {
		return;
	}

	yd = &showtabs.y_table[cely + 1];
	txtsx = tex.width();
	txtsy = tex.height();

	if (!rev) {
		xofs -= txtsx >> 1;
		yofs -= txtsy >> 1;
	}

	yofs = yofs * yd->vert_size / TXT_SIZE_Y;
	xofs = xofs * x3d->point_total / TXT_SIZE_X;
	realsx = txtsx * x3d->point_total / TXT_SIZE_X;
	realsy = txtsy * yd->vert_size / TXT_SIZE_Y;

	if (celx <= 0) {
		x = x3d->xpos + xofs;
	} else {
		x = x3d->xpos2 + xofs;
	}

	if (-x > realsx) {
		return;
	}

	if (x + realsx > 640) {
		realsx = 640 - x;
	}

	if (realsx <= 0) {
		return;
	}

	yofs = yd->drawline - yofs;
	yofs += (plac == 1) ? (-yd->vert_size) : ((plac == 2) ? (yd->vert_size) : 0);

	if (yofs - realsy < 0) {
		realsy = yofs;
	}

	renderer->wallBlit(tex, x, yofs + SCREEN_OFFLINE, x3d->zoom_table, realsx, yd->zoom_table, realsy + 1, tex.palette(cely + (secnd_shade ? SHADE_STEPS : 0)), rev == 2);
}

void draw_floor_ceil(int celx, int cely, char f_c, int dark, const Texture *tex) {
	int y;
	uint8_t *fog, black[3] = {0};

	if (nofloors) {
		return;
	}

	if (gameMap.global().map_autofadefc == 1) {
		fog = dark ? black : back_color;
	} else {
		fog = NULL;
	}

	//podlaha
	if (f_c == 0) {
		y = (VIEW_SIZE_Y - MIDDLE_Y) - points[0][0][cely].y + 1;

		if (y < 1) {
			y = 1;
		}

		renderer->fcBlit(*tex, SCREEN_OFFLINE, &showtabs.f_table[celx+3][y], fog);
/*     if (debug)
        {
         memcpy(Screen_GetAddr(),Screen_GetBackAddr(),512000);
         showview(0,0,0,0);
        }*/
	} else {
		y = points[0][1][cely].y + MIDDLE_Y + 1;

		if (y < 0) {
			y = 0;
		}

		renderer->fcBlit(*tex, SCREEN_OFFLINE, &showtabs.c_table[celx+3][y], fog);
/*     if (debug)
        {
         memcpy(Screen_GetAddr(),Screen_GetBackAddr(),512000);
         showview(0,0,0,0);
        }*/
	}
}

  /*void chozeni(void)
  {
  char c;  char dir=0;uint16_t sector=22;

  zooming_forward();
  swap_buffs();
  showview(0,0,0,0);
  do
     {
     while (_bios_keybrd(_KEYBRD_READY)) _bios_keybrd(_KEYBRD_READ);
     c=_bios_keybrd(_KEYBRD_READ) >> 8;
     switch (c)
        {
        case 'H':if (mapa[sector][dir]!=-1)
                       {
                       sector=mapa[sector][dir];
                       render_scene(sector,dir);
                       if (!debug) zooming_forward();
                       swap_buffs();
                       showview(0,0,0,0);
                       }break;
        case 'P':if (mapa[sector][(dir+2)&3]!=-1)
                    {
                    sector=mapa[sector][(dir+2)&3];
                    render_scene(sector,dir);
                    swap_buffs();
                    if (!debug) zooming_backward();
                    showview(0,0,0,0);
                    }break;
        case 'M':dir=(dir+1)&3;
                 render_scene(sector,dir);
                 if (!debug) turn_left();
                 swap_buffs();
                 showview(0,0,0,0);
                 break;
        case 'K':dir=(dir-1)&3;
                 render_scene(sector,dir);
                 swap_buffs();
                 if (!debug) turn_right();
                 showview(0,0,0,0);
                 break;
        case ';':debug=!debug;break;
        case '<':nosides=!nosides;break;
        case '=':nofloors=!nofloors;break;
        case '>':drwsit=!drwsit;break;
        }
     }
  while (c!=1);
  }
*/
/*void ask_video(video)
  {
  char c,ok,er;
  printf("\nJaky videomode?:\n"
         "  1) 640x480x256 Pomale pocitace\n"
         "  2) 640x480xHiColor Pomale pocitace\n"
         "  3) 640x480x256 Rychle pocitace\n"
         "  4) 640x480xHiColor Rychle pocitace\n");
  screen_buffer_size=640*480*2;
    do
     {
     if (!video) c=_bios_keybrd(_KEYBRD_READ)>>8;else c=video+1;
     ok=1;er=0;
     line480=1;
     switch (c)
        {
        case 1:exit(0);
        case 4:line480=1;er=initmode256(load_file("xlat256.pal"));
               zooming=zooming2;ok=0;
               turn=turn2;
               if (banking)
                 {
                 turn=turn5;
                 zooming=zooming5;
                 }
               break;
        case 5:line480=1;er=initmode32();
               zooming=zooming1;ok=0;
               turn=turn1;
               if (banking)
                 {
                 turn=turn4;
                 zooming=zooming4;
                 }
               break;
        case 2:line480=1;er=initmode256(load_file("xlat256.pal"));
               zooming=zooming2;ok=0;zooming_step=2;rot_phases=2;rot_step=140;
               turn=turn2;
               if (banking)
                 {
                 turn=turn5;
                 zooming=zooming5;
                 }
               break;
        case 3:line480=1;er=initmode32();
               zooming=zooming1;ok=0;zooming_step=2;rot_phases=2;rot_step=140;
               turn=turn1;
               if (banking)
                 {
                 turn=turn4;
                 zooming=zooming4;
                 }
               break;
        }
     if (er)
        {
        ok=1;
        if (er==-1)
        printf("Rezim zrejme neni podporovan. Zkuste nainstalovat univbe\n");
        else
          printf("Graficka karta asi nepodporuje Linear Frame Buffer. \n"
          "Pokud tomu tak neni, zkontrolujte zda neni vypnuty.\n");
        }
     }
  while (ok);
    }

 */
void report_mode(int mode)
  {
/*  switch (mode)
     {
     case 1:zooming=zooming1; turn=turn1;break;
     case 2:STOP();break;
     case 3:STOP();break;
     case 4:STOP();break;
     case 5:STOP();break;
     case 6:STOP();break;
     case 7:STOP();break;
     }*/
  }

void clear_buff(const Texture *tex, uint8_t r, uint8_t g, uint8_t b, int lines) {
	if (tex) {
		renderer->blit(*tex, 0, SCREEN_OFFLINE, tex->palette());
	} else {
		lines = 0;
	}

	renderer->bar(0, SCREEN_OFFLINE + lines, renderer->width(), 360 - lines, r, g, b);
}

void general_engine_init() {
	calc_points();
	create_tables();
	create_zooming();
}

void map_pos(int celx,int cely,int posx,int posy,int posz,int *x,int *y)
  {
  char negate2=0;
  int xl,xr;
  int p1,p2,p;
  if (celx<0)
     {
     negate2=1;
     posx=CTVR-posx;
     celx=-celx;
     }
  p1=(points[0][0][cely].y-points[0][1][cely].y);
  p2=(points[0][0][cely+1].y-points[0][1][cely+1].y);
  last_scale=p=posy*(p2-p1)/CTVR+p1;
  *y=points[0][0][cely].y-(posy*(points[0][0][cely].y-points[0][0][cely+1].y)/CTVR)-p*posz/CTVR;
  xr=points[celx][0][cely].x-(posy*(points[celx][0][cely].x-points[celx][0][cely+1].x)/CTVR);
  if (celx) xl=points[celx-1][0][cely].x-(posy*(points[celx-1][0][cely].x-points[celx-1][0][cely+1].x)/CTVR);
  else xl=-xr;
  *x=xl+((xr-xl)*posx/CTVR);
  if (negate2) *x=-*x;
  *x+=MIDDLE_X;
  *y+=MIDDLE_Y;
  }

static int items_indextab[][2]={{0,0},{-1,3},{1,7},{-1,7},{1,10},{-1,10},{0,10},{-2,15}};
void draw_item(int celx, int cely, int posx, int posy, const Texture *tex, int index) {
	int x, y;
	int clipl, clipr;
	int randx, randy;

	if (!tex) {
		return;
	}

	randx = items_indextab[7 - (index & 0x7)][0];
	randy = items_indextab[7 - (index & 0x7)][1];
	map_pos(celx, cely, 42 * posx + 42 + randx, 72 * posy + randy, 0, &x, &y);
	x -= (tex->width() / 2 * last_scale) / 320;

	renderer->enemyBlit(*tex, x, y + SCREEN_OFFLINE, tex->palette(cely + (secnd_shade ? SHADE_STEPS : 0)), last_scale);
}


void put_textured_bar(const Texture &tex, int x, int y, int xs, int ys, int xofs, int yofs) {
	int i, j, rx, ry, rxs, rys;

	xofs %= (signed)tex.width();
	yofs %= (signed)tex.height();
	xofs += xofs < 0 ? tex.width() : 0;
	yofs += yofs < 0 ? tex.height() : 0;

	for (i = -yofs; i < ys; i += tex.height()) {
		ry = i < 0 ? -i : 0;
		rys = tex.height() < ys - i ? tex.height() : ys - i;

		for (j = -xofs; j < xs; j += tex.width()) {
			rx = j < 0 ? -j : 0;
			rxs = tex.width() < xs - j ? tex.width() : xs - j;
			renderer->rectBlit(tex, j < 0 ? x : x + j, i < 0 ? y : y + i, rx, ry, rxs, rys, tex.palette());
		}
	}
}

void draw_placed_texture(const Texture *tex, int celx, int cely, int posx, int posy, int posz, char turn) {
	int x, y;
	int clipl, clipr;

	if (!tex) {
		return;
	}

	map_pos(celx, cely, posx, posy, posz, &x, &y);
	x -= (tex->width() / 2 * last_scale) / 320;
	y += (tex->height() / 2 * last_scale) / 320;

	if (y > 400) {
		y = 400;
	}

	renderer->enemyBlit(*tex, x, y + SCREEN_OFFLINE, tex->palette(cely + (secnd_shade ? SHADE_STEPS : 0)), last_scale, turn);
}

void set_lclip_rclip(int celx,int cely,int lc,int rc)
  {
  int x,xs;
  lclip=0;
  rclip=640;
  if (celx>=0)
     {
     if (rc)
        {
        x=points[celx][0][cely].x+MIDDLE_X;
        xs=points[celx][0][cely].x-points[celx][0][cely+1].x;
        rclip=x-rc*xs/TXT_SIZE_X_3D;
        if (rclip>640) rclip=640;
        }
     if (celx>0 && lc)
        {
        lclip=points[celx-1][0][cely].x+MIDDLE_X;
        if (lclip>rclip) lclip=rclip;
        }

     }
  if (celx<=0)
     {
     if (lc)
        {
        int cc=-celx;
        x=-points[cc][0][cely].x+MIDDLE_X;
        xs=points[cc][0][cely].x-points[cc][0][cely+1].x;
        lclip=x+lc*xs/TXT_SIZE_X_3D;
        if (lclip<0) lclip=0;
        }
     if (celx<0 && rc)
        {
        rclip=-points[(-celx)-1][0][cely].x+MIDDLE_X;
        if (rclip<lclip) rclip=lclip;
        }
     }
  }

void draw_enemy(DRW_ENEMY *drw) {
	int x, y, lx, sd;
	int clipl, clipr;
	int posx, posy, cely;
	short *ys, yss, *xs, xss;
	int grcel;
	pal_t *stonepal = NULL;

	posx = drw->posx;
	posy = drw->posy;  
	cely = drw->cely;

	if (!drw->tex) {
		return;
	}

	posx += 64;

	if (!(drw->shiftup & 0x8)) {
		posy += 32;
	}

	if (drw->shiftup & 0x1) {
		posy += 128;
	}

	if (posy < 0 || posy > 127) {
		return;
	}

	map_pos(drw->celx, drw->cely, posx, posy, 0, &x, &y);
	xss = drw->tex->width() * last_scale / 320;

	if (xss > 640) {
		return;
	}

	if (drw->stoned) {
		stonepal = new fadepal_t;

		for (i = 0; i < PAL_SIZE / 3; i++) {
			uint8_t col = ((unsigned)drw->palette[0][3 * i] + (unsigned)drw->palette[0][3 * i + 1] + (unsigned)drw->palette[0][3 * i + 2]) / 3;
			stonepal[0][3 * i] = col;
			stonepal[0][3 * i + 1] = col;
			stonepal[0][3 * i + 2] = col;
		}

		palette_shadow(stonepal, gameMap.global().fade_r, gameMap.global().fade_g, gameMap.global().fade_b);
		drw->palette = stonepal;
	}

	yss = drw->tex->height() * last_scale / 320;
	lx = x;
	grcel = cely;

	if (posy > 64) {
		grcel++;
	}

	if (grcel) {
		grcel--;
	}

	if (cely) {
		cely -= 1;
	}

	x -= (drw->adjust * last_scale) / 320;

	renderer->transparentBlit(*drw->tex, x, y + SCREEN_OFFLINE, drw->palette[grcel + (secnd_shade ? SHADE_STEPS : 0)], last_scale, drw->mirror);

	if (show_lives) {
		char s[25];

		// FIXME: rewrite
		sprintf(s, "%d", drw->num);
		sd = renderer->textWidth(s) / 2;

		if (lx - sd > 0 && lx + sd < 639) {
			int ly = y + SCREEN_OFFLINE - last_scale * 5 / 6;
			trans_bar(lx - sd - 5, ly - 10, sd * 2 + 10, 10, 0, 0, 0);
			renderer->drawAlignedText(lx, ly, HALIGN_CENTER, VALIGN_BOTTOM, s);
		}
	}

	if (stonepal) {
		delete stonepal;
	}
}

void draw_player(const Texture &tex, int celx, int cely, int posx, int posy, int adjust, char *name) {
	int x, y, yc, lx, sd;

	map_pos(celx, cely, posx + 64, posy + 64, 0, &x, &y);
	lx = x;
	x -= (adjust * last_scale) / 320;
	yc = (20 * last_scale) / 320 + y;

	renderer->enemyBlit(tex, x, yc + SCREEN_OFFLINE, tex.palette(cely + (secnd_shade ? SHADE_STEPS : 0)), last_scale);

	if (show_names && name != NULL) {
		sd = renderer->textWidth(name) / 2;

		if (lx - sd > 0 && lx + sd < 639) {
			int ly = y + SCREEN_OFFLINE - last_scale * 5 / 6;
			trans_bar(lx - sd - 5, ly - 10, sd * 2 + 10, 10, 0, 0, 0);
			renderer->drawAlignedText(lx, ly, HALIGN_CENTER, VALIGN_BOTTOM, name);
		}
	}
}


void draw_spectxtr(const Texture &tex, int celx, int cely, int xpos) {
	int x, y, lx, clipl, clipr;

	map_pos(celx, cely, 64, 64, 0, &x, &y);
	lx = x;
	x -= ((tex.width() / 2 + xpos) * last_scale * 2) / 320;

	renderer->transparentBlit(tex, x, y + SCREEN_OFFLINE, tex.palette(cely + (secnd_shade ? SHADE_STEPS : 0)), last_scale * 2);
}

/*
__inline void prumeruj(void *target,void *source1, void *source2)
//#pragma aux prumeruj parm [edi][eax][edx]=
  {
// this is obviously wrong but nobody uses it
  _asm
    {
    mov  edi,target
    mov  eax,source1
    mov  ebx,source2	; mov edx, source2?
    mov  eax,[eax]
    mov  edx,[edx]
    and  eax,7bde7bdeh
    and  edx,7bde7bdeh
    add  eax,edx
    shr  eax,1
    stos
    }
  }

void double_zoom_xicht(uint16_t x,uint16_t y,uint16_t *source)
  {
  uint16_t *xpal;
  uint16_t *sline,*slline;
  char *pline;
  char *sr;
  int xx,yy;

  xpal=source+3;
  sr=(char *)(xpal+256);
  for(yy=0;yy<75;yy++)
     {
     sline=Screen_GetAddr()+(y+(yy<<1))*Screen_GetXSize()+x;
     slline=sline;
     pline=sr+yy*54;
     for(xx=0;xx<54;xx++)
        {
        int zz=xx<<1;
        sline[zz]=xpal[*pline++];
        if (xx)
           {
           sline[zz-1]=((sline[zz] & RGB555(30,30,30))+(sline[zz-2] & RGB555(30,30,30)))>>1;
           if (yy) prumeruj(sline-640+zz-1,sline+zz-1,sline-Screen_GetScan()+zz-1);
           }
        else
           if (yy) prumeruj(sline-640,sline,sline-Screen_GetScan());
        }
     }
  }
*/


void draw_item2(int celx, int cely, int xpos, int ypos, const Texture *tex, int index) {
	int x, y, xs, ys, ysc, abc, asc;// clipl, clipr;
	static int indextab[][2] = {{0, 0}, {0, 1}, {1, 0}, {-1, 0}, {1, 2}, {-1, 1}, {-2, 1}, {2, 1}};

	celx--;
	asc = (celx < 0);
	abc = abs(celx);

	if (asc) {
		abc--;
	}

	x = points[abc][0][cely + 1].x;
	y = points[abc][0][cely + 1].y;
	xs = showtabs.x_table[0][cely].max_x;
	ys = showtabs.y_table[cely + 1].vert_size;
	ysc = showtabs.y_table[cely].vert_size;
	xpos += indextab[7 - index][0];
	ypos += indextab[7 - index][1];
	xpos -= tex->width() / 2;
	xpos = xs * xpos / 500;
	ypos = ys * ypos / 320;

	if (asc) {
		x = -x;
	}

	x += MIDDLE_X;
	y += MIDDLE_Y;
	x += xpos;
	y -= ypos;

	renderer->enemyBlit(*tex, x, y + SCREEN_OFFLINE, tex->palette(cely + (secnd_shade ? SHADE_STEPS : 0)), ys);
}

/*
void main()
  {
  printf("%d\n",sizeof(showtabs));
  p=load_file("konvert\\bredy.hi");
  calc_points();
  create_tables();
  create_zooming();
  ask_video();
  put_picture(0,100,p);showview(0,0,0,0);free(p);
  p2=load_file("konvert\\stena2.hi");
  p=load_file("konvert\\stena2bl.hi");
  strop=load_file("konvert\\strop1.hi");
  podlaha=load_file("konvert\\podlaha1.hi");
  sit=load_file("konvert\\sit.hi");
  build_map();
  render_scene(22,0);showview(0,0,0,0);
  chozeni();
  closemode();
  }
*/


int zoom_speed(int zoomspeed)
  {
  switch (zoomspeed)
     {
     case 0:zooming_step=0;break;
     case 1:zooming_step=2;break;
     case 2:zooming_step=1;break;
     case -1: switch (zooming_step)
        {
        case 0:return 0;
        case 1:return 2;
        case 2:return 1;
        }
     }
  return zoomspeed;
  }


int turn_speed(int turnspeed)
  {
  switch (turnspeed)
     {
     case 0:rot_phases=0;break;
     case 1:rot_phases=1;break;
     case 2:rot_phases=2;break;
     case -1: return rot_phases;
     }
  return rot_phases;
  }

void set_backgrnd_mode(int mode)
  {
  backgrnd_mode=mode;
  }

int get_item_top(int celx, int cely, int posx, int posy, const Texture *tex, int index) {
	int x, y;
	int randx, randy;

	randx = items_indextab[7 - (index & 0x7)][0];
	randy = items_indextab[7 - (index & 0x7)][1];
	map_pos(celx, cely, 42 * posx + 42 + randx, 72 * posy + randy, 0, &x, &y);

	if (tex) {
		return y - (tex->height() * last_scale) / 320 + SCREEN_OFFLINE;
	} else {
		return y + SCREEN_OFFLINE;
	}
}

#define ANIM_SIZE (320*180)

MGIFReader *anim_reader = NULL;

void play_big_mgif_animation(int block) {
	MemoryReadStream *stream;

	assert(!anim_reader && "Another animation is being played");

	stream = dynamic_cast<MemoryReadStream*>(ablock(block));
	stream->seek(0, SEEK_SET);
	anim_reader = new MGIFReader(*stream, 320, 180, 1);
	play_big_mgif_frame();
}

void play_big_mgif_frame(void) {
	unsigned state;

	if (!anim_reader) {
		return;
	}

	if (state = anim_reader->decodeFrame()) {
		neco_v_pohybu = 1;
	} else {
		delete anim_reader;
		anim_reader = NULL;
	}
}
