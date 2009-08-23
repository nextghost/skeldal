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
#ifndef __ENGINE1_H
#define __ENGINE1_H

#include <inttypes.h>

#define VIEW_SIZE_X 640
#define VIEW_SIZE_Y 360
#define TAB_SIZE_X 640
#define TAB_SIZE_Y 600
#define MIDDLE_X 320
#define MIDDLE_Y 112
#define TXT_SIZE_Y 320
#define TXT_SIZE_X_3D 74
#define TXT_SIZE_X 500
#define VIEW3D_X 4
#define VIEW3D_Z 5
#define START_X1 357
#define START_Y1 305
#define START_X2 357
#define START_Y2 -150
#define FACTOR_3D 3.33
#define ZOOM_PHASES 9
#define SCREEN_OFFLINE (17)
#define SCREEN_OFFSET (Screen_GetXSize()*SCREEN_OFFLINE)
#define C_YMAP_SIZE 90
#define F_YMAP_SIZE 199
#define CF_XMAP_SIZE 7

#define SHADE_STEPS 5
#define SHADE_PAL (SHADE_STEPS*512*2)

#pragma pack(1)

void general_engine_init(void);
void draw_floor_ceil(int celx,int cely,char f_c,uint16_t *txtr);
void show_cel2(int celx,int cely,void *stena,int xofs,int yofs,char rev);
  //zobrazi primou stenu ktera lezi pred nebo napravo od pozorovatele
void show_cel(int celx,int cely,void *stena,int xofs,int yofs,char rev);
  void turn_left();
void turn_right();
void zooming_backward(uint16_t *background);
void zooming_forward(uint16_t *background);
void OutBuffer2nd(void);
void CopyBuffer2nd(void);
void report_mode(int);
void draw_item(int celx,int cely,int posx,int posy,uint16_t *pic,int index);
void draw_item2(int celx,int cely,int xpos,int ypos,void *texture,int index);
//void textmode_effekt();
//#pragma aux textmode_effekt modify[eax ebx ecx edx edi];


void clear_buff(uint16_t *background,uint16_t backcolor,int lines);

typedef struct zoominfo
  {
     void *startptr, *texture;
     int32_t texture_line,line_len;
     int32_t *xtable;
     int16_t *ytable;
     uint16_t *palette;
     uint16_t ycount;
     uint16_t xmax;
  }ZOOMINFO;


typedef struct t_info_y
  {
  int32_t drawline; //ukazatel na radku na ktere bude stena zacinat
  uint16_t vert_size; //konecna velikost steny, pokud ma pocatecni velikost TXT_SIZE_Y
  uint16_t vert_total; //maximalni velikost textury aby jeste nepresahla obrazovku
  int16_t zoom_table[TAB_SIZE_Y];  //tabulka pro zoomovaci rutiny
  }T_INFO_Y;

typedef struct t_info_x_3d
  {
  int8_t used;  // 1 pokud je tato strana videt
  int16_t xpos;      //bod od leveho okraje
  uint16_t txtoffset; //posunuti x vuci texture
  uint16_t point_total; //rozdil mezi levym prednim a levym zadnim okrajem postranni steny (v adresach)
  int32_t zoom_table[VIEW_SIZE_X]; //zoomovaci tabulka pro osu x pro postranni steny
  }T_INFO_X_3D;

typedef struct t_info_x
  {
  int8_t used;  // 1 pokud je tato strana videt
  int16_t xpos;  //bod od leveho okraje
  int16_t xpos2; //totez ale pro pravou stranu
  uint16_t txtoffset; //posunuti x vuci texture
  uint16_t max_x; //pocet viditelnych bodu z textury
  uint16_t point_total;  //celkovy pocet adres mezi levym a pravym okrajem
  int32_t zoom_table[VIEW_SIZE_X]; //zoomovaci tabulka pro osu x pro kolme steny
  }T_INFO_X;

typedef struct t_floor_map
  {
  int32_t lineofs,linesize,counter,txtrofs;
  }T_FLOOR_MAP;

typedef struct all_view
  {
  T_INFO_Y y_table[VIEW3D_Z+1];
  T_INFO_X_3D z_table[VIEW3D_X][VIEW3D_Z];
  T_INFO_X x_table[VIEW3D_X][VIEW3D_Z+1];
  T_FLOOR_MAP f_table[CF_XMAP_SIZE][F_YMAP_SIZE];
  T_FLOOR_MAP c_table[CF_XMAP_SIZE][C_YMAP_SIZE];

  }ALL_VIEW;

typedef struct t_point
  {
  int32_t x,y;
  }T_POINT;

typedef T_POINT t_points[VIEW3D_X+1][2][VIEW3D_Z+1];
extern uint16_t *background;
extern t_points points;
extern int zooming_step;
extern int rot_phases;
extern int rot_step;
extern uint16_t *buffer_2nd;
extern char show_names;
extern char show_lives;
extern char secnd_shade;

typedef uint16_t palette_t[256];

typedef struct drw_enemy_struct
  {
  void *txtr;
  int32_t celx,cely,posx,posy,adjust,shiftup,num;
  int8_t mirror;
  int8_t stoned;
  palette_t *palette;
  }DRW_ENEMY;




void enemy_draw(uint8_t *src,uint16_t *trg,int shade,int scale,int maxspace,int clip);
//#pragma aux enemy_draw parm[ESI][EDI][EBX][EDX][EAX][ECX]
void enemy_draw_transp(uint8_t *src,uint16_t *trg,palette_t shade,int scale,int maxspace,int clip);
//#pragma aux enemy_draw_transp parm[ESI][EDI][EBX][EDX][EAX][ECX]
void enemy_draw_mirror_transp(uint8_t *src,uint16_t *trg,palette_t shade,int scale,int maxspace,int clip);
//#pragma aux enemy_draw_mirror_transp parm[ESI][EDI][EBX][EDX][EAX][ECX]
void enemy_draw_mirror(uint8_t *src,uint16_t *trg,int shade,int scale,int maxspace,int clip);
//#pragma aux enemy_draw_mirror parm[ESI][EDI][EBX][EDX][EAX][ECX]
//clip je v poradi vpravo - vlevo (HiLo)

void draw_enemy(DRW_ENEMY *drw);
void draw_player(uint16_t *txtr,int celx,int cely,int posx,int posy,int adjust,char *name);
void double_zoom_xicht(uint16_t x,uint16_t y,uint16_t *source);

void set_lclip_rclip(int celx,int cely,int lc,int rc);
void draw_spectxtr(short *txtr,int celx,int cely,int xpos);

int turn_speed(int turnspeed); //oba je nutne volat na zacatku
int zoom_speed(int zoomspeed);

void scroll_and_copy(uint16_t *pic,uint16_t *slide, uint16_t *scr, int size,int shift, int lineinfo[][2]);
//#pragma aux scroll_and_copy parm[esi][ebx][edi][ecx][edx][eax]

void set_backgrnd_mode(int mode);

int get_item_top(int celx,int cely,int posx,int posy,uint16_t *txtr,int index);
 //vraci nejnizsi souradnici y predmetu leziciho na zemi v celx, cely na pozici posx,posy;

void play_big_mgif_animation(int block);
void play_big_mgif_frame(void);

#pragma option align=reset

#endif
