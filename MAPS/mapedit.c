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
#include "mapedit_pch.h"
#include "dump.h"
#include "mapy.h"
#include "edit_map.h"
#include "globals.h"
#include "steny.h"
#include "save_map.h"
#include "wiz_tool.h"
#include "mob_edit.h"
#include "../crashdump.h"
#include "../mapedit/version.h"
#include "../mapedit/editor.h"

//#define FONTPATH "d:\\tp\\vga\\"
#define FONTPATH "..\\font\\"
extern T_EVENT_ROOT *ev_tree;

static char newmap=0;
static char nosound=0;
TSTR_LIST config_file;

const word *icones,*vga_font;
const char *sample_path;
const char *script_name;
extern word ikones;
extern word boldcz;
extern word font8x5;
word icone_color[7]={RGB555(8,8,8),RGB555(31,31,31),RGB555(0,0,15),RGB555(16,16,16),RGB555(0x1b,0x1b,0x1b)}; 
extern word sipka;
static const char *error_texts[]=
     {
     "Progamov� chyba",
     "Pr�v� b���c� ud�lost zp�sobila neplatnou referenci pam��i!",
     "Pr�v� b���c� ud�lost zp�sobila zpracov�n� neplatn� instrukce!",
     "Syst�m zachytil neo�ek�vanou chybu programu!"
     };

static char ask_exit_status;

word menu_win=-1;
char hicolor=1;
char sekceid[8]="<BLOCK>";
long pc_counter=0;
long pc_max=1;
long pc_result=0;
long pc_tick=0;
int test_mode=4;
char *background_file=NULL;
int editor_win=-1;
char color16=0;

void close_test()
  {
  close_window(waktual);
  }

void logo(void)
  {
  CTL3D ctl;
  word *p;

  ctl.light=RGB555(31,31,31);
  ctl.shadow=RGB555(16,16,16);
  ctl.bsize=2;
  ctl.ctldef=0;
  draw_border(120,120,0x18f,0x6a,&ctl);
  p=(word *)LoadResourceFont("MAPEDIT.HI");
  put_picture(120,120,p);
//  free(p);
  showview(0,0,0,0);
  }

void about();

void movesize_test()
  {
  WINDOW *w;

  w=find_window(map_win);
  if (w==NULL) return;
  movesize_win(w,0,0,100,100);
  redraw_desktop();
  }

void forced_map_test()
  {
  int i;

  for(i=0;i<maplen;i++)
     {
     if (minfo[i].flags & 0x1)
        {
        call_testmap(i);
        break;
        }
     }
  }

EVENT_PROC(exit_key)
  {
  int c;

  (void)GET_USER_PTR();
  WHEN_MSG(E_INIT)
     return;
  WHEN_MSG(E_KEYBOARD)
     {
     c=GET_DATA(int);
     if ((c & 0xff)==0 && (c>>8)=='D') gui_terminate();
     if ((c & 0xff)==0 && (c>>8)==';') about();
     if ((c & 0xff)==0 && (c>>8)=='?') save_dump();
     if ((c & 0xff)==0 && (c>>8)=='<') movesize_test();
     if ((c & 0xff)=='t' || (c & 0xff)=='T' || (c & 0xff)==' ' ) forced_map_test();
     }
  }

EVENT_PROC(wait_ms_key)
  {
  MS_EVENT *ms;

  (void)GET_USER_PTR();
  WHEN_MSG(E_INIT)
     return;
  WHEN_MSG(E_MOUSE)
     {
     ms=get_mouse(GET_MSG_VAR());
     if (ms->event_type & 4) gui_terminate();
     }
  }

EVENT_PROC(main_menu)
  {
  MS_EVENT *ms;

  (void)GET_USER_PTR();
  WHEN_MSG(E_INIT)
     return;
  WHEN_MSG(E_MOUSE)
     {
     ms=get_mouse(GET_MSG_VAR());
     if (ms->event_type & 32) select_window(menu_win),redraw_window();
     }
  }


void wait_mouse(void)
  {
  send_message(E_ADD,E_MOUSE,wait_ms_key);
  escape();
  send_message(E_DONE,E_MOUSE,wait_ms_key);
  }

EVENT_PROC(all_status)
  {
  char *c;
  char *fn;
//  char **p;

  (void)GET_USER_PTR();
  WHEN_MSG(E_INIT) return;
  WHEN_MSG(E_DONE) return;
  c=GET_DATA_PTR(char);
  c=(char *)msg->data;
  fn=strrchr(filename,'\\');
  if (fn) fn++;else fn=filename;
  sprintf(c,"CH:%d, N:%d, P:%d, file: %s ",maplen,0,max_items,fn);
  c=strchr(c,'\0');
  if (pc_result<0) sprintf(c,"(wait)");else sprintf(c,"CPU %03ld%%",pc_result);
  c=strchr(c,'\0');
  msg->data = c;
//  *p=c;
  pc_counter++;
  }

void *pc_metter(EVENT_MSG *msg)
  {
  static int last1,last2;
  if (msg->msg==E_INIT) return &pc_metter;
  if (msg->msg==E_DONE) return NULL;
  pc_tick++;
  if (pc_tick>10)
     {
     if (pc_counter>pc_max) {
        if (last1>pc_max && last2>pc_max) pc_max=pc_counter;
        else last2=last1;last1=pc_counter;
     }
     pc_result=100-pc_counter*100/pc_max;
     pc_counter=0;
     pc_tick=0;
     }
  return NULL;
  }

void graph_init(char windowed)
{
/*
int c;
static word *p=NULL;
   if (color16)
     {
     if (p==NULL)p=create_blw_palette16();
     initmode16(p);
     }
  else
     {
    if (hicolor) c=initmode32() ;else c=-1;
    if (c)
     {

    if (p==NULL)p=create_special_palette();
    if (initmode256(p))
        initmode_lo(p);
     }
     }
	 */
//  report_mode(1);

 const char *c=get_text_field(config_file,"MAPEDIT_WINDOW");
 int scale = 1;
 if (c != 0) 
 {
    int x,y;
    if (sscanf(c,"%dx%d",&x,&y) == 2 && x>=640 && y>=480)
        DxSetInitResolution(x,y);
 }
 ;
 c=get_text_field(config_file,"MAPEDIT_SCALE");
 if (c != 0)
 {
     sscanf(c,"%d",&scale);
 }
initmode_dx(windowed,scale,0,0);

}

void init_sound()
  {
  int a,b,c,d;
  if (nosound)
    {
    a=DEV_NOSOUND;
    }
  else if (sound_detect(&a,&b,&c,&d))
     {
     puts("No sound device found");
     a=DEV_NOSOUND;
     }
  set_mixing_device(a,22000,b,c,d);
  }

void prg_error(EVENT_MSG *msg,void **unused)
  {
#if 0
  int *err;
  char *c;

  (void)unused;
  if (msg->msg==E_PRGERROR)
     {
     err=(int *)(*(int **)msg->data);
     switch (*err)
       {
       case ERR_MEMREF:c=error_texts[1];break;
       case ERR_ILLEGI:c=error_texts[2];break;
       default: c=error_texts[3];break;
       }
    zobraz_mysku();
    showview(0,0,0,0);
    *err=2-msg_box(error_texts[0],' ',c,"Ignoruj","Konec","Ulo� mapu",NULL);
    if (*err==-1) save_all_map();
     }
#endif
  }

void raise_error_conv(int i)
  {
/*  if (i==SIGILL) raise_error(ERR_ILLEGI);
  else
  if (i==SIGSEGV) raise_error(ERR_MEMREF);
  else
     raise_error(ERR_MEMREF);
*/
  }

static void key_test(va_list args)
  {
  word c;
  do
     {
     c=*(word *)task_wait_event(E_KEYBOARD);
     }
  while ((c & 0xff)!='!');
  puts("\x7");
  }

void ddl_init()
  {
  const char *c;
  char *path,*temp;

  c=get_text_field(config_file,"CESTA_DATA");
  concat(path,c,"SKELDAL.DDL");
  c=get_text_field(config_file,"CESTA_TEMP");
  if (c!=NULL) {
	  concat(temp,c,"~MAPEDIT.TMP");
	  c = temp;
  }
  printf("Hledam soubor %s\n",path);
  init_manager(path,c);
  }

void init(void)
  {
    mapy_init();
    ddl_init();
    vga_font=LoadResourceFont("BOLDCZ");
    default_font=vga_font;
    icones=LoadResourceFont("IKONY");
    graph_init(1);
    curcolor=RGB555(24,24,24);memcpy(charcolors,flat_color(0x0000),sizeof(charcolors));
    init_events(100);
    curfont=default_font;
    register_ms_cursor(LoadResourceFont("SIPKA.HI"));
    init_mysky();
//    hranice_mysky(0,0,639,479);
    add_task(1024,key_test);
    send_message(E_ADD,E_STATUS_LINE,status_line);
    send_message(E_STATUS_LINE,E_ADD,E_IDLE,all_status);
    send_message(E_STATUS_LINE,E_ADD,E_IDLE,status_mem_info);
    send_message(E_STATUS_LINE,E_ADD,E_IDLE,show_time);
    send_message(E_ADD,E_TIMER,pc_metter);
    send_message(E_ADD,E_KEYBOARD,exit_key);
    send_message(E_ADD,E_PRGERROR,prg_error);
/*    signal(SIGILL,raise_error_conv);
    signal(SIGSEGV,raise_error_conv);*/
    install_gui();
    redraw_desktop();do_events();
    send_message(E_ADD,E_MOUSE,main_menu);
    msg_box_font=vga_font;
    msg_icn_font=icones;
    ukaz_mysku();
    update_mysky();
    }
/*
 w=create_window(100,100,400,200,0x6318,&x);
  id=desktop_add_window(w);
  gui_define(10,20,50,30,0,sample,"Test");
  gui_property(&x,NULL,flat_color(0x7000),0xffff);
  gui_define(10,20,70,30,3,button,"Tlacitko");
  gui_property(NULL,NULL,flat_color(0x000f),0x01c0);gui_on_change(close_test);
  w=create_window(5,5,200,200,0x6318,&x);
  id=desktop_add_window(w);
  gui_define(50,50,70,30,3,button,"Tlacitko");
  gui_property(NULL,NULL,flat_color(0x7fff),0x000f);gui_on_change(close_test);
  w=create_window(300,150,300,200,0x6318,&x);
  id=desktop_add_window(w);
  gui_define(50,50,70,30,3,button,"Tlacitko");
  gui_property(NULL,NULL,flat_color(0x7fff),0x000f);gui_on_change(close_test);
 */

long def_window(word xs,word ys,char *name)
  {
  word x=0,y=0;
  WINDOW *p;
  CTL3D ctl;
  FC_TABLE fc;
  long q;

  if (waktual!=NULL)
     {
     x=waktual->x;
     y=waktual->y;
     }

  highlight(&ctl,WINCOLOR);
  ctl.bsize=2;ctl.ctldef=0;
  x+=20;y+=20;
  memcpy(fc,flat_color(RGB555(31,31,0)),sizeof(FC_TABLE));
  fc[0]=0x0000;
  if (x+xs>SCR_WIDTH_X-2) x=SCR_WIDTH_X-2-xs;
  if (y+ys>SCR_WIDTH_Y-2) y=SCR_WIDTH_Y-2-ys;
     p=create_window(x,y,xs,ys,WINCOLOR,&ctl);
     q=desktop_add_window(p);
  gui_define(0,2,2,xs-5-20*(xs>=70),14,0,win_label,name);
     ctl.bsize=1;ctl.ctldef=1;
     o_end->autoresizex=1;
     gui_property(&ctl,vga_font,&fc,LABELCOLOR);
  if (xs>=70)
     {
  gui_define(1,1,1,19,16,1,button,"\x0f");
     gui_property(NULL,icones,&icone_color,WINCOLOR);gui_on_change(close_test);
     }
  return q;
  }

void def_dialoge(word x,word y,word xs, word ys, char *name)
  {
  CTL3D ctl;
  FC_TABLE fc;
  WINDOW *p;


  memcpy(fc,flat_color(RGB555(31,31,0)),sizeof(FC_TABLE));
  memcpy(&ctl,def_border(2,WINCOLOR),sizeof(CTL3D));
  p=create_window(x,y,xs,ys,WINCOLOR,&ctl);
  desktop_add_window(p);
  set_window_modal();
  ctl.bsize=1;ctl.ctldef=1;
  gui_define(0,1,1,xs-2,14,0,win_label,name);
  o_end->autoresizex=1;
  gui_property(&ctl,vga_font,&fc,LABELCOLOR);
  }

void dtext_init(OBJREC *o,char *title)
  {
  title=get_title(title);
  o->userptr=(void *)getmem(strlen(title)+1);
  strcpy((char *)o->userptr,title);
  }


void dtext_draw(int x1,int y1,int x2, int y2,OBJREC *o)
  {
  (void)x2;(void)y2;
  fontdsize=1;
  position(x1,y1);
  outtext((char *)o->userptr);
  fontdsize=0;
  }

void dtext(OBJREC *o)
  {
  o->runs[0]=dtext_init;
  o->runs[1]=dtext_draw;
  //o->runs[2]=sample_event;
  //o->runs[3]=sample_done;
  }

void about()
  {
  FC_TABLE c={0,RGB555(31,31,0),RGB555(28,28,28),RGB555(24,24,24),RGB555(20,20,20)};
  FC_TABLE c2={0,RGB555(31,16,0),RGB555(28,16,0),RGB555(24,16,0),RGB555(20,16,0)};
  FC_TABLE c3={0,RGB555(0,24,0),RGB555(0,20,0),RGB555(0,16,0),RGB555(0,10,0)};
  static int about_win=-1;

  if (find_window(about_win)==NULL)
  {
  about_win=def_window(300,200,"About");
  waktual->x=320-150;
  waktual->y=140;
  memcpy(f_default,flat_color(0x0),sizeof(FC_TABLE));
  waktual->modal=1;
  default_font=vga_font;
  gui_define(-1,5,25,29,29,0,dtext,"\x8");gui_property(NULL,icones,&c,WINCOLOR);
  gui_define(-1,75,25,100,29,0,dtext,"MAPEDIT");gui_property(NULL,NULL,&c2,WINCOLOR);
  gui_define(-1,200,35,60,29,0,label,"verze 2.0");gui_property(NULL,NULL,&c3,WINCOLOR);
  gui_define(-1,20,80,200,10,0,label,"Naps�no pro hru \"Br�ny Skeldalu\"");
  gui_define(-1,20,92,200,10,0,label,"(C) 1997 Napoleon gameS ");
  gui_define(-1,20,104,200,10,0,label,"Naprogamoval: Ond�ej Nov�k ");
  gui_define(-1,20,116,200,10,0,label,"Tento software sm� b�t pou�it jen");
  gui_define(-1,20,128,200,10,0,label,"ve spojen� s v�vojem hry \"Br�ny");
  gui_define(-1,20,140,200,10,0,label,"Skeldalu\" (a p��padn� dal�� verze)");
  gui_define(-1,20,152,200,10,0,label,"a to pouze �leny v�vojov�ho t�mu.");
  gui_define(10,110,170,80,20,0,button,"Ok");gui_on_change(close_test);
  }
  else select_window(about_win);
  redraw_window();
  }

void fog_bar_draw(int x1,int y1,int x2,int y2,OBJREC *o)
  {
  disable_bar(x1,y1,x2-x1,y2-y1,o->color);
  }

void fog_bar(OBJREC *o)
  {
  o->runs[1]=fog_bar_draw;
  }

void close_app(void)
  {
  WINDOW *w;
  CTL3D x={0,0,0,0};

  int mx =  DxGetResX() - 1;
  int my =  DxGetResY() - 1;

  w=create_window(0,0,1,1,0,&x);
  desktop_add_window(w);
  gui_define(-1,0,0,mx,my,0,fog_bar);gui_property(NULL,NULL,NULL,RGB555(16,0,0));
  redraw_desktop();
  if ((ask_exit_status=msg_box("Dotaz?",'\x2',"Chce� program ukon�it, nebo nahr�t jinou mapu?","Jinou mapu","Ukon�it","Ne",NULL))!=3) gui_terminate();
  close_window(w);
  do_events();
  }

void edit_basic_maze()
  {
  if (tool_sel>40) tool_sel=30;
  create_map_win(-1);
  if (newmap)
     {
     edit_side(0,0);
     sector_details_call(0);
     if (str_count(side_names)>2 || side_names[1]!=NULL) newmap=0;
     }
  }

void spawn_editor(char *dosline)
  {
//  deinstall_mouse_handler();
  closemode();
  system(dosline);
  graph_init(1);
//  install_mouse_handler();
//  hranice_mysky(0,0,639,479);
  redraw_desktop();
  }

void edit_script_file()
  {
  char s[255]="";
  char *pr;WINDOW *w;


  w=find_window(sektor_win);
  if (w!=NULL) close_window(w);
  switch (o_aktual->id)
     {
     case 10:strcat(s,pripona(filename,SCR));break;
     case 20:strcat(s,pripona(filename,".TXT"));break;
     case 30:strcat(s,ITEMS_SCRIPT);break;
     case 40:strcat(s,ITEMS_PICS);break;
     }
  EditSkeldalFile(s);
  pr=pripona(filename,SCR);
  read_full_side_script(pr);
  read_side_list(ITEMS_SCRIPT,&vzhled_veci,0,4);
  read_side_list(ITEMS_PICS,&pohledy_veci,0,2);
  read_side_list(WEAPONS_SCRIPT,&weapons,0,3);
  read_side_list(WEAPONS_SCRIPT,&weapons_pos,2,3);
  }

void call_animator()
  {
  spawn_editor("ani");
  }


void open_editor_win()
  {
  if (find_window(editor_win)==NULL)
     {
     editor_win=def_window(100,150,"Editory");
     gui_define(10,10,25,80,20,0,button,"map script");gui_on_change(edit_script_file);
     gui_define(20,10,50,80,20,0,button,"map texty");gui_on_change(edit_script_file);
     gui_define(30,10,75,80,20,0,button,"items.scr");gui_on_change(edit_script_file);
     gui_define(40,10,100,80,20,0,button,"items.pic");gui_on_change(edit_script_file);
     gui_define(50,10,125,80,20,0,button,"animator");gui_on_change(call_animator);
     redraw_window();
     }
  else
     {
     select_window(editor_win);
     redraw_window();
     }
  }

void create_menu(void)
  {
  FC_TABLE c={0,RGB555(31,31,31),RGB555(24,24,24),RGB555(16,16,16),RGB555(4,4,4)};
  menu_win=def_window(400,150,"Map Edit v2.0 for Windows - " MAPEDIT_VERSION );
  waktual->x=120;
  waktual->y=250;
  gui_on_change(close_app);
  curcolor=WINCOLOR;
  default_font=icones;
  gui_define(10,5,25,29,29,0,button,"\x8");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(edit_basic_maze);
  gui_define(20,5,55,29,29,0,button,"\x9");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(editor_veci);
  gui_define(30,5,85,29,29,0,button,"\xA");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(enemy_window);
  c[1]=RGB555(20,0,0);c[2]=RGB555(0,0,16);
  gui_define(40,200,25,29,29,0,button,"e");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(shop_train_edit);
  c[1]=RGB555(31,31,31);c[2]=RGB555(24,24,24);
  c[3]=RGB555(20,0,0);c[4]=RGB555(0,0,16);
  gui_define(50,200,55,29,29,0,button,"E");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(open_editor_win);
  gui_define(60,200,85,29,29,0,button,"S");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(save_all_map_nr);
  gui_define(70,5,115,29,29,0,button,"P");gui_property(NULL,NULL,&c,WINCOLOR);gui_on_change(pcxviewer);
  memcpy(c,flat_color(RGB555(0,0,15)),sizeof(FC_TABLE));
  gui_define(-1,45,35,90,20,0,label,"Kreslen� mapy");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,45,65,90,20,0,label,"Pokl�d�n� p�edm�t�");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,45,95,90,20,0,label,"Um�s�ov�n� nestv�r");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,240,35,90,20,0,label,"Obchody");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,240,65,90,20,0,label,"Editor script�");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,240,95,90,20,0,label,"Ulo�en� mapy");gui_property(NULL,vga_font,&c,WINCOLOR);
  gui_define(-1,45,125,90,20,0,label,"Prohl��e� PCX");gui_property(NULL,vga_font,&c,WINCOLOR);
  default_font=vga_font;
  memcpy(f_default,flat_color(0x0000),sizeof(charcolors));
  //set_enable(0,20,0);
  //set_enable(0,30,0);
  }

void help(void)
  {
  printf("Parametry:\n\n");
  printf("mapedit filename [/4] [/2] [/c|t|v|m] [/?] \n"
         "\n"
         "/2 Prosadi 256 barev\n"
         "/1 Prosadi 16 barev\n"
         "/c Inicializace mysi se simulaci CGA karty (default)\n"
         "/t Inicializace mysi se simulaci textoveho rezimu (standard)\n"
         "/v Inicializace mysi se simulaci VGA\n"
         "/m Inicializace mysi se simulaci MCGA\n"
         "/? Tento help\n"
         "/s Bez zvuku (dobre pro WIN95)\n"
         );
  exit(0);
  }

void args_support(char count, char *sw[])
  {
  int i;
  char c;

  for(i=1;i<=count;i++)
    if (sw[i][0]=='/')
     {
     c=sw[i][1];
     switch (c)
     {
     case '?': help();break;
/*     case '4': line480=1;break;
     case '2': hicolor=0;break;
     case '1': color16=1;break;
     case 'c':
     case 'C': ms_fake_mode=0x6;break;
     case 't':
     case 'T': ms_fake_mode=0x3;break;
     case 'v':
     case 'V': ms_fake_mode=0x12;break;
     case 'm':
     case 'M': ms_fake_mode=0x13;break;
     case 'r':
     case 'R': test_mode=sw[i][2]-48;if (test_mode<0 || test_mode>4) test_mode=0;break;*/
     case 's': nosound=1;break;
     }
     }
   else
     strcpy(filename,sw[i]);
  }

char *pripona(char *fname,char *prip)
  {
  static char name[128],*s,*t;

  strncpy(name,fname,128);
  name[127]='\0';
  t=strchr(name,'.');
  s=t;
  while (s!=NULL)
     {
     t=s;
     s=strchr(t+1,'.');
     }
  if (t==NULL)
     {
     t=strchr(name,'\0');
     }
  strcpy(t,prip);
  return name;
  }


void load_background()
  {
  FILE *f;
  long siz;
  
  if (gui_background!=NULL)
     {
     free(gui_background);
     gui_background=NULL;
     }
  f=fopen(background_file,"rb");
  if (f==NULL) return;
  fseek(f,0,SEEK_END);siz=ftell(f);
  fseek(f,0,SEEK_SET);
  gui_background=getmem(siz);
  fread(gui_background,1,siz,f);
  redraw_desktop();
  fclose(f);
  }

static void shut_down()
  {
//  deinstall_mouse_handler();
  closemode();
  }

void init_maps(void);

static void ask_password_event(EVENT_MSG *msg,OBJREC *obj)
  {
  (void)obj;
  WHEN_MSG(E_KEYBOARD)
    {
    char c=GET_DATA(char);
    if (c==13) {goto_control(20);gui_terminate();}
    if (c==27) {goto_control(30);gui_terminate();}
    }
  }

char ask_password(char *pass,char text)
  {
  char *c;

  switch (text)
    {
    case 0:c="Prov��en�";break;
    case 1:c="Zm�na hesla";break;
    case 2:c="Kontrola";break;
    }
  def_dialoge(320-100,240-50,200,90,c);
  gui_define(-1,10,20,1,1,0,label,"Vlo� heslo:");
  gui_define(10,10,40,180,12,0,input_line,49);
  gui_property(def_border(3,WINCOLOR),NULL,flat_color(RGB555(31,31,31)),RGB555(8,8,8));
  if (text==1)set_default(pass);else set_default("");gui_on_event(ask_password_event);
  gui_define(20,5,5,50,20,2,button,"OK");
  gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(gui_terminate);
  gui_define(30,5,5,50,20,3,button,"Zru�it");
  gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(gui_terminate);
  redraw_window();
  goto_control(10);
  escape();
  if (o_aktual->id==30) text=0;else
    {
    text=1;
    get_value(0,10,pass);
    }
  close_current();
  return text;
  }

static int data_edit_enabled=0;
static char data_password[50]="";

char check_data_password(void)
  {
  char text[50];

  if (data_password[0]==0) data_edit_enabled=1;
  if (data_edit_enabled) return 1;
  if (ask_password(text,0)==0) return 0;
  if (strcmp(data_password,text))
    {
    msg_box("Chyba!",1,"Chybn� heslo! P��stup zam�tnut!","OK",NULL);
    return 0;
    }
  else
    {
    data_edit_enabled=1;
    return 1;
    }
  }

char *set_data_password(char *text)
  {
  if (text!=NULL)
    {
    strncpy(data_password,text,sizeof(data_password));
    data_password[sizeof(data_password)-1]=0;
    }
  return data_password;
  }

/*TSTR_LIST read_ddl_dir(char *filter)
  {
  TSTR_LIST ls;
  int i=0;
  char *s;

  ls=create_list(256);
  s=read_next_entry(MMR_FIRST);
  while (s!=NULL)
     {
     char *d;
     char c[13];

     strncpy(c,s,12);
     c[12]=0;
     if (filter!=NULL)
        {
        d=strrchr(c,'.');
        if (d!=NULL && !strncmp(d+1,filter,3)) str_replace(&ls,i++,c);
        }
     else
        str_replace(&ls,i++,c);
     s=read_next_entry(MMR_NEXT);
     }
  if (i==0) str_add(&ls,"<chyba>");
  ls=sort_list(ls,-1);
  return ls;
  }
*/

SMapFiles mapFiles;

static char *strdup2(const char *a, const char *b)
{
  char *res=malloc(strlen(a)+strlen(b)+1);
  strcpy(res,a);
  strcat(res,b);
  return res;
}

static void InitMapFiles(const char *path)
{
  mapFiles.items_script=strdup2(path,XITEMS_SCRIPT);
  mapFiles.items_pics=strdup2(path,XITEMS_PICS);
  mapFiles.items_dat=strdup2(path,XITEM_FILE);
  mapFiles.dialogy_scr=strdup2(path,XDLG_SCRIPT);
  mapFiles.weapons_scr=strdup2(path,XWEAPONS_SCRIPT);
  mapFiles.shops_dat=strdup2(path,XSHOP_NAME);
  mapFiles.enemy=strdup2(path,XMOB_FILE);
  mapFiles.enemy_sound=strdup2(path,XMOB_SOUND);
}
static void ClearMapFiles()
{
  free(mapFiles.items_script);
  free(mapFiles.items_pics);
  free(mapFiles.items_dat);
  free(mapFiles.dialogy_scr);
  free(mapFiles.weapons_scr);
  free(mapFiles.shops_dat);
  free(mapFiles.enemy);
  free(mapFiles.enemy_sound);

}

#ifdef _WIN32
static BOOL WINAPI HandlerRoutine(
  DWORD dwCtrlType   //  control signal type
)
{
  return TRUE;
}
#endif

int main(int argc,char *argv[])
  {
  char *s;
  char *pr;
  char test[50];
  char *mask;
  InitCrashDump();
#ifdef _WIN32
  SetConsoleCtrlHandler(HandlerRoutine,TRUE);
#endif
  filename[0]='\0';
  //  strcpy(filename,"TEST.MAP");
  args_support(argc-1,argv);
  printf("Hledam konfiguracni soubor\n");
  config_file=read_config("WSKELDAL.INI");
  if (config_file==NULL)
    {
    puts("...nemohu najit WSKELDAL.INI\n");
    return 1;
    }
  if (strlen(filename)>3 && strcasecmp(filename+strlen(filename)-3,"adv")==0)
  {
	TSTR_LIST adv_cfg=read_config(filename);
	config_file=merge_configs(config_file,adv_cfg);
	filename[0]=0;
  }  
  sample_path=get_text_field(config_file,"CESTA_ZVUKY");
  if (sample_path==NULL) sample_path="";
  mob_dir=get_text_field(config_file,"CESTA_ENEMY");
  if (mob_dir==NULL) mob_dir="";
  init_sound();
  init();

  concat(mask,get_text_field(config_file,"CESTA_MAPY"),"*.map");  
  atexit(shut_down);
//  signal(SIGABRT,shut_down);
  init_mob_list();
  InitMapFiles(get_text_field(config_file,"CESTA_MAPY"));
  do
     {
	  char *c;
     ask_exit_status=2;
     if (filename[0]=='\0') browser(mask,filename);
     if (filename[0]!='\0')
	 {
	   const char *mapy=get_text_field(config_file,"CESTA_MAPY");
	   memmove(filename+strlen(mapy),filename,strlen(filename)+1);
	   memcpy(filename,mapy,strlen(mapy));
	   s=pripona(filename,".HI");
	   background_file=(char *)getmem(strlen(s)+1);strcpy(background_file,s);
       load_background();
	 }
     do_events();
     logo();
     pr=pripona(filename,SCR);
     c=NewArr(char,strlen(pr)+1);
     strcpy(c,pr);
     script_name = c;
     read_full_side_script(pr);
     read_spec_procs();
     read_side_list(ITEMS_SCRIPT,&vzhled_veci,0,4);
     read_side_list(ITEMS_PICS,&pohledy_veci,0,2);
     read_dlg_list(DLG_SCRIPT,&dlg_names,&dlg_pgfs);
     read_side_list(WEAPONS_SCRIPT,&weapons,0,3);
     read_side_list(WEAPONS_SCRIPT,&weapons_pos,2,3);
     set_defaults();
     init_multiactions();
     memset(vyklenky,0,sizeof(vyklenky));
     init_item_system();
     if (filename[0]!='\0' )
       {
       int sel=1;

       init_maps();
       set_defaults();
       if (load_map(filename))
          msg_box(filename,'\01',"Tento soubor je bu� ne�iteln�, nebo po�kozen�","Pokra�ovat",NULL);
       if (check_password(NULL)==0)
          if (ask_password(test,0)==0 || check_password(test)==0)
            {
            filename[0]=0;
            ask_exit_status=1;
            goto preskoc;
            }
       if (maplen<2)
          {
          sel=msg_box(filename,' ',"Soubor neexistuje, bude vytvo�en nov�. Nyn� je nutn� nastavit z�kladn� st�ny"
                               " a jin� dal�� parametry pro tuto mapu","Pokra�ujem","Zav�it",NULL);
          if (sel==1)
             {
             newmap=1;
             }

          }
       if (sel==1)
           {
          create_menu();
          redraw_window();
          escape();
          filename[0]=0;
          close_current();
          }
        }
     preskoc:
     free(background_file);
     }
  while (ask_exit_status==1);
  ClearMapFiles();
  redraw_desktop();
  close_manager();
  return 0;
  }
#ifdef _WIN32
int __stdcall WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  return main(__argc,__argv);
}


  int GetExeVersion()
  {
	return MAPEDIT_NVERSION;
  }
#endif
