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
#include "mapy.h"
#include "globals.h"
#include "edit_map.h"
#include "steny.h"
#include "save_map.h"
#include "mob_edit.h"


#define ITEM_BUFFER 256
#define SV_ITLIST 0x8001
#define SV_SNDLIST 0x8002
#define SV_END    0x8000
#define SV_PASSW  0x1000

#define ITEM_IKON "..\\graphics\\items\\"
#define ITEM_MALE "..\\graphics\\basic\\char00.pcx"

#define DIALOG_MASK "..\\graphics\\dialogs\\*.hi"


#define IT_INSIDE 0x1
#define IT_IDENTF 0x2
#define IT_UNIKAT 0x4
#define IT_MAGIC  0x8

TITEM item_list[ITEM_SORTS];
short *item_place[MAPSIZE*4];
short item_buffer[ITEM_BUFFER];
short max_items;
int item_win=-1;
int vzor_win=-1;
int selected_place=-1;
int preview_win=-1;
int grep_num=-1;

static int shop_window=-1;

TSTR_LIST vzhled_veci;
TSTR_LIST pohledy_veci;

TSTR_LIST ls_buffer=NULL,ls_sorts=NULL;
TSTR_LIST itm_sounds=NULL;

TSHOP *shop_list=NULL;int max_shops=0;
TSTR_LIST shop_items=NULL;short shop_max_id=1;
TTRAINING *train_list=NULL;int max_trains=0;

void clear_it_buffer();

void init_item_system(void)
  {
  static char again=0;
  int i;

  if (again) for(i=0;i<sizeof(item_place)/4;i++) if (item_place[i]!=NULL) free(item_place[i]);
  memset(item_place,0,sizeof(item_place));
  max_items=0;
  clear_it_buffer();
  again=1;
  }


#define get_items(sector,pos) item_place[sector*4+pos];

void remove_items_from_sector(int sector)
  {
  int i;

  sector<<=2;for(i=0;i<4;i++)
     {
     free(item_place[i+sector]);
     item_place[i+sector]=NULL;
     }
  }

void update_items(int sector,int pos,short *new[],int pocet)
  {
  short **q;
  q=&item_place[sector*4+pos];
  free(*q);
  *q=(short *)getmem(pocet*sizeof(short));
  memcpy(*q,*new,pocet*sizeof(short));
  }

void ikris(int num,int x1,int y1,int x2,int y2)
 {
 curcolor=RGB555(31,0,0);
 if (num==selected_place) bar(x1,y1,x2,y2);
 if (item_place[num]==NULL) return;
 curcolor=RGB555(0,0,31);
 line(x1+1,y1+1,x2-1,y2-1);
 line(x2-1,y1+1,x1+1,y2-1);
 }

void ikona_display(int x1,int y1,int x2,int y2,OBJREC *o)
  {
  static char *ikonlib=NULL;
  static int lastikolib=-1;
  int nowlib;
  int ikn;
  char s[200];
  char n[13];

  ikn=*(int *)o->data;
  nowlib=ikn/18;
  if (lastikolib!=nowlib)
     {
     FILE *f;int i;void *z;long size;
     const char *c=get_text_field(config_file,"CESTA_ITEMY");
     if (c==NULL) c="";
     sprintf(n,"ikony%02d.lib",nowlib);
     sprintf(s,"%s%s",c,n);
     if (ikonlib==NULL) ikonlib=(char *)getmem(18*2993);
     f=fopen(s,"rb");
     if (f==NULL)
        if ((z=afile(n,read_group(0),&size))==NULL)
           for(i=0;i<18;i++)
              {
              memset(ikonlib+i*2993,0,2993);
              }
        else
           {
           memcpy(ikonlib,z,18*2993);
           free(z);
           }
       else
        {
        fread(ikonlib,1,18*2993,f);
        fclose(f);
        }
     lastikolib=nowlib;
     for(i=0;i<18;i++)
     {
       int j;
       word *z=(word *)(ikonlib+i*2993+6);
       for (j=0;j<256;j++) z[j]=z[j]+(z[j] & 0x7FE0);
     }

     }
  ikn%=18;
  bar(x1,y1,x2,y2);
  put_picture(x1,y1,ikonlib+ikn*2993);
  }

void ikona_click(EVENT_MSG *msg,OBJREC *o)
  {
  MS_EVENT *ms;

  (void)o;
  if (msg->msg==E_MOUSE)
     {
     ms=get_mouse(msg);
     if (ms->event_type==0x4)
         set_change();
     }
  }

void ikona(OBJREC *o)
  {
  o->runs[1]=ikona_display;
  o->runs[2]=ikona_click;
  o->datasize=4;
  }

int select_ikon_lib(int last)
  {
  int x,y,i,s;

  s=last;
  def_dialoge(50,200,500,264,"Knihovna ikon - vyber jednu ikonu");
  x=5;y=20;
  for (i=0;i<30;i++)
     {
     gui_define(i+10,x,y,45,55,0,ikona);c_default(last+i);
     gui_property(def_border(5,WINCOLOR),NULL,NULL,0);gui_on_change(gui_terminate);
     x+=48;if (x+48>490) {y+=58;x=5;}
     }
  gui_define(300,10,10,80,20,3,button,"<<");
  gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(gui_terminate);
  gui_define(310,10,10,80,20,2,button,">>");
  gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(gui_terminate);
  gui_define(320,175-40,10,80,20,3,button,"Zru�it");
  gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(gui_terminate);
  redraw_window();
  do
     {
     escape();
     if (o_aktual->id==300)
     {
     last-=30;if (last<0) last=0;
     for(i=0;i<30;i++) c_set_value(0,10+i,last+i);
     }
  if (o_aktual->id==310)
     {
     last+=30;
     for(i=0;i<30;i++) c_set_value(0,10+i,last+i);
     }
     }
  while (o_aktual->id==310 || o_aktual->id==300);
  i=o_aktual->id;
  close_current();
  if (i==320) i=s; else i=i-10+last;
  return i;
  }

void change_item_ikone()
  {
  c_set_value(0,130,select_ikon_lib(f_get_value(0,130)));
  }

char nvlast[][30]=
  {"S�la","Schopnost magie","Pohyblivost","Obratnost","Max zran�n�",
  "Kondice","Max mana","Obrana(doln�)","Obrana(Horn�)","�tok(Doln�)",
  "�tok(Horn�)","Ohe�","Voda","Zem�","Vzduch","Mysl","�ivoty Regen",
  "Mana Regen","Kondice Regen","Magick� s�la(D)", "Magick� s�la(H)","","��innek z�sahu","*"};

void ukaz_vlastnosti(int pocet,int x,int y,int id,short *it)
  {
  word i,ts;

  i=0;
  curfont=vga_font;
  default_font=curfont;
  while (nvlast[i][0]!='*' && i<pocet)
     {
     ts=text_width(nvlast[i]);
     if (y+15>250)
        {
        x+=200;
        y=20;
        }
     if (nvlast[i][0])
	{
     	gui_define(-1,x,y,ts,10,0,label,nvlast[i]);
     	gui_define(id++,x+130,y,60,12,0,input_line,7,-32767,+32767,"%6d");
     	gui_property(def_border(5,WINCOLOR),NULL,NULL,WINCOLOR);
     	set_default(strs(*it));
     	gui_on_exit(test_int);
     	y+=15;
	}
     it++;
     i++;
     }
  }

void oprav_vlastnosti(TITEM *it)
  {
  int i;
  def_dialoge(120,80,410,310,"Vlastnosti");
  ukaz_vlastnosti(4,5,20,70,it->podminky);
  gui_define(-1,5,93,190,0,0,label,"");gui_property(def_border(2,WINCOLOR),NULL,NULL,WINCOLOR);
  ukaz_vlastnosti(99,5,105,80,it->zmeny);
  /*gui_define(130,5,200,100,10,0,check_box,"Na hlavu");
  gui_define(140,5,212,100,10,0,check_box,"Na krk");
  gui_define(150,5,224,100,10,0,check_box,"Na telo");
  gui_define(160,5,236,100,10,0,check_box,"Na ruce");
  gui_define(170,5,248,100,10,0,check_box,"Na lokte");
  gui_define(180,5,260,100,10,0,check_box,"Kolem pasu");
  gui_define(190,5,272,100,10,0,check_box,"Na nohy");
  gui_define(200,105,200,100,10,0,check_box,"Na chodidla");
  gui_define(210,105,212,100,10,0,check_box,"Na ramena");
  gui_define(220,105,224,100,10,0,check_box,"Na z�da");  */
  gui_define(300,10,10,80,20,2,button,"Zru�it");gui_on_change(gui_terminate);
   gui_property(def_border(1,0),NULL,NULL,WINCOLOR);
  gui_define(310,10,40,80,20,2,button,"Ok");gui_on_change(gui_terminate);
   gui_property(def_border(1,0),NULL,NULL,WINCOLOR);
  gui_define(-1,225,216,70,10,0,label,"Magick� �tok");
  gui_define(250,225,228,70,70,0,radio_butts,5,"Ohe�","Voda","Zem�","Vzduch","Mysl");
  c_default(it->zmeny[VLS_MGZIVEL]);
//  fill_bit_fields(0,130,it->place_map,11);
  redraw_window();
  escape();
  if (o_aktual->id==310)
     {
     for(i=0;i<4;i++) it->podminky[i]=vals(i+70);
     for(i=0;i<23;i++) it->zmeny[i]=vals(i+80);
//     it->place_map=get_bit_fields(0,130,10);
     it->zmeny[VLS_MGZIVEL]=f_get_value(0,250);
     }
  close_current();
  }

void otevri_vlastnosti()
  {
  TITEM *it;

  get_value(0,5,&it);
  oprav_vlastnosti(it);
  }

void item_special(TITEM *it)
  {
  int i,y;
  CTL3D *c;

  def_dialoge(140,100,410,320,"Special - obr�zky v letu");
  y=20;c=def_border(5,WINCOLOR);
  for(i=0;i<16;i++)
     {
     gui_define((i+1)*10,100,y,300,12,0,str_line,pohledy_veci);gui_property(c,NULL,NULL,WINCOLOR);
     c_default(it->v_letu[i]); gui_on_enter(string_list_sup);
     y+=15; if ((i & 3)==3) y+=5;
     }
  gui_define(-1,5,20,1,1,0,label,"Zezadu");
  gui_define(-1,5,85,1,1,0,label,"Ze strany");
  gui_define(-1,5,150,1,1,0,label,"Zep�edu");
  gui_define(-1,5,215,1,1,0,label,"V�buch");
  gui_define(300,10,10,80,20,2,button,"Zru�it");gui_on_change(gui_terminate);
  gui_define(310,100,10,80,20,2,button,"Ok");gui_on_change(gui_terminate);
  redraw_window();
  escape();
  if (o_aktual->id==310)
     for(i=0;i<16;i++) it->v_letu[i]=f_get_value(0,i*10+10);
  close_current();
  }

void otevri_special()
  {
  TITEM *it;

  get_value(0,5,&it);
  item_special(it);
  }

void items_adjusting()
  {
  int num;
  TSTR_LIST pcxs=NULL;
  char *filename;
  int x1,y1,x2,y2,yy1,yy2;
  word *vec = 0;
  WINDOW *w;
  char *c,*d;
  const char *cc,*dd;
  word *male,malexs,maleys;

  w=find_window(preview_win);
  cc=get_text_field(config_file,"CESTA_BGRAFIKA"); if (cc==NULL) cc="";
  concat(d,cc,"CHAR00.PCX");
  if (w!=NULL) close_window(w);
  if (open_pcx(d,A_8BIT,(void *)&male))
     {
     void *z;long s;
	 char *ss=strrchr(d,'\\');
	 if (ss==NULL) ss=d;else ss=ss+1;

     if ((z=afile(ss,read_group(0),&s))==NULL) return;
     load_pcx(z,s,A_8BIT,(void *)&male);free(z);
     }
  malexs=male[0];
  maleys=male[1];
  free(male);
  read_side_list(ITEMS_SCRIPT,&pcxs,2,4);
  num=f_get_value(0,190);
  if (num >= str_count(pcxs))
      num = 0;
  concat(c,"",pcxs[num]);
  dd=get_text_field(config_file,"CESTA_ITEMY");if (dd==NULL) dd="";
  concat(filename,dd,cc);
  release_list(pcxs);
  if (open_pcx(filename,A_8BIT,(void *)&vec))
     {
     void *z;long s;

     if ((z=afile(c,read_group(0),&s))==NULL)
        {
        msg_box("Soubor nenalezen!",'\x1',filename,"Ok",NULL);
        return;
        }
     load_pcx(z,s,A_8BIT,(void *)&vec);free(z);
     }
  x1=vals(140);
  y1=vals(150);
  x2=vals(160);
  y2=vals(170);
  yy1=350-y1-vec[1];
  yy2=350-y2-vec[1];
  if (yy1<0 || yy2<0)
     {
     msg_box("Chyba!",'\x1',"Nelze zobrazit, proto�e n�hled je z��sti mimo obrazovku!","Ok",NULL);
     return;
     }
  preview_win=def_window(300,400,"View");
  gui_define(10,150-malexs/2,350-maleys,malexs,maleys,0,pcx_view);set_default(ITEM_MALE);
  gui_define(20,150+x1-vec[0]/2,yy1,1,1,0,pcx_view);set_default(filename);
  gui_define(30,150+x2-vec[0]/2,yy2,1,1,0,pcx_view);set_default(filename);
  gui_define(40,5,5,80,20,2,button,"Ok");gui_on_change(close_current);
  movesize_win(waktual,0,0,300,400);
  redraw_window();
  free(vec);
  }


static int add_sound_to_table(char *s)
  {
  int cnt;
  int i;

  cnt=str_count(itm_sounds);
  for(i=1;i<cnt;i++) if (itm_sounds[i]!=NULL && !strcmp(itm_sounds[i],s)) return i;
  return str_add(&itm_sounds,s);
  }

static int delete_unused_sound(int index)
  {
  int i;

  for(i=0;i<ITEM_SORTS;i++) if (item_list[i].sound==index) return 0;
  str_replace(&itm_sounds,index,NULL);
  return 1;
  }


void item_sound_call(TITEM *it)
  {
  TSTR_LIST list;
  CTL3D b1,b2,b3;
  char *c;


  memcpy(&b1,def_border(1,0),sizeof(CTL3D));
  memcpy(&b2,def_border(5,WINCOLOR),sizeof(CTL3D));
  memcpy(&b3,def_border(6,WINCOLOR),sizeof(CTL3D));
  default_font=vga_font;
  memcpy(f_default,flat_color(0x0000),sizeof(charcolors));
  def_dialoge(100,50,300,250,"Zvuk p�i dopadu (destrukci)");
  concat(c,sample_path,"*.wav");
  list=read_directory(c,DIR_BREIF,_A_NORMAL);
   read_ddl_list_wav(&list);
  gui_define(9,10,20,200,126,0,listbox,list,0x7fff,0);
  gui_property(&b3,NULL,NULL,WINCOLOR);c_default(0);gui_on_change(mob_test_sound);
  gui_define(10,217,40,19,87,0,scroll_bar_v,0,10,1,SCROLLBARCOL);
  gui_property(&b2,NULL,NULL,WINCOLOR);
  gui_define(11,216,20,21,17,0,scroll_button,-1,0,"\x1e");
  gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
  gui_define(12,216,130,21,17,0,scroll_button,1,10,"\x1f");
  gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     {
     int z,y;
     char *s;
     z=it->sound; y=160+15;
     if (z==0) s=NULL;else s=itm_sounds[z];
     if (s==NULL) s="";
     gui_define(-1,5,y,1,1,0,label,"Zvuk:");
     gui_define(20,120,y,120,12,0,input_line,13);gui_property(&b2,NULL,NULL,WINCOLOR);
     set_default(s);
     gui_define(40,245,y,20,12,0,button,"<");
        gui_property(&b1,NULL,NULL,WINCOLOR);gui_on_change(mob_sound_copy);
     }
  gui_define(100,5,5,80,20,2,button,"Ok");gui_on_change(gui_terminate); gui_property(&b1,NULL,NULL,WINCOLOR);
  gui_define(110,90,5,80,20,2,button,"Zru�it");gui_on_change(gui_terminate); gui_property(&b1,NULL,NULL,WINCOLOR);
  redraw_window();
  escape();
  if (o_aktual->id==100)
     {
     char s[200];
     int z;
     get_value(0,20,s);
     strupr(s);
     z=it->sound;
     if (s[0])it->sound=add_sound_to_table(s);else it->sound=0;
     delete_unused_sound(z);
     }
  close_current();
  check_unused_sounds();
  }


static void item_sound()
  {
  TITEM *it;

  get_value(0,5,&it);
  item_sound_call(it);
  }

void save_edited_item()
  {
  TITEM *it;
  WINDOW *w;

  get_value(0,5,&it);
  get_value(0,15,it->jmeno);
  get_value(0,20,it->popis);
  it->hmotnost=vals(40);
  it->nosnost=vals(50);
  it->keynum=vals(60);
  it->user_value=vals(70);
  it->use_event=vals(80);
  it->druh=f_get_value(0,90);
  it->spell=vals(100);
  it->magie=vals(110);
  it->cena=vals(120);
  it->ikona=f_get_value(0,130);
  it->polohy[0][0]=vals(140);
  it->polohy[0][1]=vals(150);
  it->polohy[1][0]=vals(160);
  it->polohy[1][1]=vals(170);
  it->umisteni=f_get_value(0,180);
  it->vzhled=f_get_value(0,190);
  it->flags=get_bit_fields(0,210,2);
  it->typ_zbrane=f_get_value(0,200);
  it->zmeny[VLS_KOUZLA]=f_get_value(0,700);
  it->animace=f_get_value(0,400);
  it->hitpos=vals(410);
  it->shiftup=0xff;
  w=find_window(preview_win);
  if (w!=NULL) close_window(w);
  gui_terminate();
  }


void spell_affections();

static void update_hitpos()
  {
  int i;

  i=f_get_value(0,400);
  if (i==0) set_value(0,410,"0");
  else
  if (weapons_pos[i]!=NULL)
     {
     set_value(0,410,weapons_pos[i]);
     }
  }





void adjust_enter(EVENT_MSG *msg,OBJREC *o)
  {
  char update=0;

  if (msg->msg==E_KEYBOARD)
  {
     int s;
      
     char save=0;

     s=vals(o_aktual->id);
     switch (*(int *)msg->data>>8)
        {
        case 'Q':s++;save=1;break;
        case 'I':s--;save=1;break;
        }
     if (save)
     {
      char buff[256];
      set_value(0,o_aktual->id,itoa(s,buff,10));
      update=1;
     }
  }

  if (msg->msg==E_KEYBOARD && *(char *)(msg->data)==13) update=1;
  if (update)
  {
    if (find_window(preview_win)!=NULL)
        {
        WINDOW *w;
        int c;
        w=waktual;c=o->id;
        items_adjusting();
        select_window(w->id);
        goto_control(c);
        }
  }
  }

void item_edit(TITEM *it)
  {
  CTL3D *c;
  WINDOW *w;
  int anm=it->animace;

  if (anm>=str_count(weapons)) anm=0;
  c=def_border(5,WINCOLOR);
  def_dialoge(220,80,410,350,"Oprava �daj� o p�edm�tu");
  curfont=vga_font;
  default_font=curfont;
  gui_define(-1,5,20,50,10,0,label,"Jm�no (31 znak�)");
  gui_define(15,10,35,288,12,0,input_line,31);set_default(it->jmeno);
   gui_property(def_border(2,WINCOLOR),NULL,flat_color(RGB555(16,0,0)),WINCOLOR);
  gui_define(-1,5,50,50,10,0,label,"Popis (31 znak�)");
  gui_define(20,10,65,380,12,0,input_line,31);set_default(it->popis);
   gui_property(def_border(2,WINCOLOR),NULL,flat_color(0xF),WINCOLOR);
  gui_define(5,0,0,0,0,0,value_store,4);set_default(&it);
  gui_define(30,10,30,80,20,1,button,"Vlastnosti");
   gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(otevri_vlastnosti);
  gui_define(-1,5,80,100,10,0,label,"Hmotnost");
  gui_define(-1,5,95,100,10,0,label,"Nosnost");
  gui_define(-1,5,110,100,10,0,label,"ID Kl��e");
  gui_define(-1,5,125,100,10,0,label,"?Hodnota");
  gui_define(-1,5,140,100,10,0,label,"�.Ud�losti");
  gui_define(-1,5,155,100,10,0,label,"Druh");
  def_border(5,WINCOLOR);
  gui_define(40,105,80,60,12,0,input_line,7,-32767,+32767,"%6d");
   gui_property(c,NULL,NULL,WINCOLOR);gui_on_exit(test_int);set_default(strs(it->hmotnost));
  gui_define(50,105,95,60,12,0,input_line,7,-32767,+32767,"%6d");
   gui_property(c,NULL,NULL,WINCOLOR);gui_on_exit(test_int);set_default(strs(it->nosnost));
  gui_define(60,105,110,60,12,0,input_line,7,-32767,+32767,"%6d");
   gui_property(c,NULL,NULL,WINCOLOR);gui_on_exit(test_int);set_default(strs(it->keynum));
  gui_define(70,105,125,60,12,0,input_line,7,-32767,+32767,"%6d");
   gui_property(c,NULL,NULL,WINCOLOR);gui_on_exit(test_int);set_default(strs(it->user_value));
  gui_define(80,105,140,60,12,0,input_line,7,-32767,+32767,"%6d");
   gui_property(c,NULL,NULL,WINCOLOR);gui_on_exit(test_int);set_default(strs(it->use_event));
  gui_define(90,80,160,220,12,0,str_line,typy_veci);gui_property(c,NULL,NULL,WINCOLOR);
     gui_on_enter(string_list_sup);c_default(it->druh);
  gui_define(-1,105,80,100,10,1,label,"Kouzlo:");
  gui_define(-1,105,95,100,10,1,label,"Mana celk:");
  gui_define(-1,105,110,100,10,1,label,"Cena v�ci");
  gui_define(-1,105,125,100,10,1,label,"Poloha1(XY)");
  gui_define(-1,105,140,100,10,1,label,"Poloha2(XY)");
  gui_define(100,55,80,50,12,1,input_line,7,-32767,32767,"%6d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->spell));
  gui_define(110,55,95,50,12,1,input_line,7,-32767,32767,"%6d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->magie));
  gui_define(120,5,110,100,12,1,input_line,10,0,999999,"%10d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->cena));
  gui_define(140,55,125,50,12,1,input_line,7,-32767,32767,"%5d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->polohy[0][0]));gui_on_event(adjust_enter);
  gui_define(150,5,125,45,12,1,input_line,7,-32767,32767,"%5d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->polohy[0][1]));gui_on_event(adjust_enter);
  gui_define(160,55,140,50,12,1,input_line,7,-32767,32767,"%5d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->polohy[1][0]));gui_on_event(adjust_enter);
  gui_define(170,5,140,45,12,1,input_line,7,-32767,32767,"%5d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->polohy[1][1]));gui_on_event(adjust_enter);
  gui_define(175,5,155,60,20,1,button,"Zobraz");gui_on_change(items_adjusting);
  gui_define(180,80,175,220,12,0,str_line,umisteni_veci);gui_property(c,NULL,NULL,WINCOLOR);
      gui_on_enter(string_list_sup);c_default(it->umisteni);
  gui_define(200,80,190,220,12,0,str_line,typy_zbrani);gui_property(c,NULL,NULL,WINCOLOR);
      gui_on_enter(string_list_sup);c_default(it->typ_zbrane);
  gui_define(210,80,205,220,10,0,check_box,"P�i dopadu se zni��");
  gui_define(220,80,217,220,10,0,check_box,"Existuje dokud je pou��v�n");
  gui_define(130,10,180,45,55,0,ikona);c_default(it->ikona);
   gui_property(c,NULL,NULL,0);gui_on_change(change_item_ikone);
  gui_define(-1,150,25,120,12,2,label,"Obr�zkov� script");
  gui_define(190,150,10,120,12,2,str_line,vzhled_veci);gui_property(c,NULL,NULL,WINCOLOR);
     c_default(it->vzhled); gui_on_enter(string_list_sup);
  gui_define(-1,150,60,120,12,2,label,"Animace zbran�");
  gui_define(400,150,45,120,12,2,str_line,weapons);gui_property(c,NULL,NULL,WINCOLOR);
     c_default(anm); gui_on_enter(string_list_sup);gui_on_change(update_hitpos);
  gui_define(-1,10,45,1,12,3,label,"Hit Pos:");
  gui_define(410,90,45,40,12,3,input_line,7,0,255,"%4d");gui_on_exit(test_int);
   gui_property(c,NULL,NULL,WINCOLOR);set_default(strs(it->hitpos));
  gui_define(300,10,35,80,20,2,button,"Ulo�");gui_property(def_border(1,0),NULL,NULL,RGB555(0,20,0));gui_on_change(save_edited_item);
  gui_define(320,10,70,80,20,2,button,"Zvuk");gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(item_sound);
  gui_define(330,10,10,80,20,2,button,"Zru�it");gui_property(def_border(1,0),NULL,flat_color(RGB555(31,31,31)),RGB555(20,0,0));gui_on_change(gui_terminate);
  gui_define(310,10,10,80,20,3,button,"Special");gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(otevri_special);
  gui_define(340,10,95,80,20,2,button,"Aff");gui_property(def_border(1,0),NULL,NULL,WINCOLOR);gui_on_change(spell_affections);
  gui_define(700,0,0,0,0,0,value_store,4);c_default(it->zmeny[VLS_KOUZLA]);
  redraw_window();
  fill_bit_fields(0,210,it->flags,2);
  escape();
  w=find_window(preview_win);
  if (w!=NULL) close_window(w);
  close_current();
  }

void clear_it_buffer()
  {
  memset(item_buffer,0,sizeof(item_buffer));
  }

void create_ibuffer_list()
  {
  int i;
  if (ls_buffer!=NULL) release_list(ls_buffer);
  ls_buffer=create_list(32);
  for (i=0;item_buffer[i];i++)
     {
     char c[80];TITEM *q;
     int p;
     p=abs(item_buffer[i])-1;
     q=&item_list[p];
     if (item_buffer[i]<0) sprintf(c,"%3d ... %s",i,q->jmeno);
     else sprintf(c,"%3d %s",i,q->jmeno);
     if (str_replace(&ls_buffer,i,c)==NULL) return;
     }
  if (i==0) str_replace(&ls_buffer,0," -1 <�adn� p�edm�ty>");
  str_delfreelines(&ls_buffer);
  }

void create_isort_list(TSTR_LIST *ls_sorts,int filter)
  {
  int i;
  char cc=0;
  if (*ls_sorts!=NULL) release_list(*ls_sorts);
  *ls_sorts=create_list(32);
  for (i=0;i<max_items;i++)
     {
     char c[80];TITEM *q;
     q=&item_list[i];
     if (q->jmeno[0]!='?' && (filter==-1 || q->druh==filter))
        {
        sprintf(c,"%4d %s",i,q->jmeno);
        if (str_replace(ls_sorts,i,c)==NULL) return;
        cc=1;
        }
     }
  if (!cc) str_replace(ls_sorts,0," -1 <�adn� vzory>");
  str_delfreelines(ls_sorts);
  }

void save_items()
  {
  FILE *f;int i;
  TSTR_LIST names=NULL;
  char passw[50];

  for(i=0;i<max_items;i++) item_list[i].shiftup=0xff;
  if (max_items)
     {
     f=fopen(ITEMS_DAT,"wb");
     if (f==NULL)
        {
        msg_box("Chyba",'\x1',"Nemohu ulo�it vzory p�edm�t�","Panika",NULL);
        return;
        }
     save_section(f,item_list,SV_ITLIST,sizeof(TITEM)*max_items);
     for(i=1;i<4;i++)
        {
        read_side_list(ITEMS_SCRIPT,&names,i,4);
        save_scr_list(f,names,i);
        }
     read_side_list(ITEMS_PICS,&names,1,2);
     save_scr_list(f,names,4);
     read_side_list(WEAPONS_SCRIPT,&names,1,3);
     save_scr_list(f,names,5);
     save_scr_list(f,itm_sounds,SV_SNDLIST);
     strcpy(passw,set_data_password(NULL));
     encrypt(passw);
     if (passw[0]!=0) save_section(f,passw,SV_PASSW,strlen(passw)+1);
     save_section(f,&max_items,SV_END,sizeof(max_items));
     release_list(names);
     fclose(f);
     }
  }

void load_items()
  {
  FILE *f;void *p;long size,r;int sect;

   if (itm_sounds!=NULL) release_list(itm_sounds);
   f=fopen(ITEMS_DAT,"rb");
   if (f==NULL) return;
   do
     {
     r=load_section(f,&p,&sect,&size);
     //if (p!=NULL)size=r;
     if (r==size) {
        if (sect==SV_ITLIST)
        {
        memcpy(item_list,p,size);
        max_items=size/sizeof(TITEM);
        itm_sounds=create_list(256);
        str_add(&itm_sounds,"<nic>");
        }
        else if (sect==SV_PASSW)
          {
          encrypt(p);
          set_data_password(p);
          }
        else if (sect==SV_SNDLIST)
        {
        int i;
        char *c,*s;

        c=s=p;i=1;
        while(c-s<size)
           {
           str_replace(&itm_sounds,i,c);
           c=strchr(c,0);
           c++;i++;
           }
        }
     }
     free(p);
     }
   while (sect!=SV_END && r==size);
   fclose(f);
  }

short aloc_free_item()
  {
  int i;

  for(i=0;i<max_items && item_list[i].jmeno[0]!='?';i++);
  if (i==max_items) {
      if (max_items==ITEM_SORTS) return -1;
      else max_items++;
  }
  return i;
  }

void clone_item()
  {
  int i,j;

  if (check_data_password()==0) return;
  j=aloc_free_item();
  if (j==-1)
     {
     msg_box("Nelze!",'\x1',"MAPEDIT je omezen na po�et v�ech mo�n�ch vzor� ve h�e","Ok",NULL);
     return;
     }
  i=f_get_value(0,9);
  sscanf(ls_sorts[i],"%d",&i);
  if (i<0 || i>=max_items)
     memset(&item_list[j],0,sizeof(TITEM));
  else
     memcpy(&item_list[j],&item_list[i],sizeof(TITEM));
  item_edit(&item_list[j]);
  create_isort_list(&ls_sorts,grep_num);
  send_message(E_GUI,9,E_CONTROL,1,ls_sorts);
  }

void edit_selected_item()
  {
  int i;

  if (check_data_password()==0) return;
  i=f_get_value(0,9);
  sscanf(ls_sorts[i],"%d",&i);
  if (i<0 || i>=max_items) clone_item();
  else
     item_edit(&item_list[i]);
  create_isort_list(&ls_sorts,grep_num);
  send_message(E_GUI,9,E_CONTROL,1,ls_sorts);
  }

void delete_item_sort()
  {
  int i;

  if (check_data_password()==0) return;
  i=f_get_value(0,9);
  sscanf(ls_sorts[i],"%d",&i);
  if (i<0 || i>=max_items) return;
  if (msg_box("Dotaz?",'\x02',"Opravdu m�� v �myslu vymazat tento vzor? Je mo�n�,"
             " �e na tento vzor se odvol�v�j� p�edm�ty v jin� map�. MAPEDIT je "
             "nem��e vymazat, a mohou se dal�� pr�c� zm�nit v p�edm�t jin�!",
             "P�esto vymazat","Zru�it",NULL)==2) return;
  strcpy(item_list[i].jmeno,"??? Vymaz�n");
  while (max_items && item_list[max_items-1].jmeno[0]=='?') max_items--;
  create_isort_list(&ls_sorts,grep_num);
  send_message(E_GUI,9,E_CONTROL,1,ls_sorts);
  }

void refresh_itm()
  {
  create_ibuffer_list();
  send_message(E_GUI,9,E_CONTROL,1,ls_buffer);
  }

void it_kos()
  {
  int i;

  i=f_get_value(0,9);
  sscanf(ls_buffer[i],"%d",&i);
  if (i<0 || i>=ITEM_BUFFER) return;
  if (i==ITEM_BUFFER-1)
     {
     item_buffer[i]=-1;
        return;
     }
  memcpy(&item_buffer[i],&item_buffer[i+1],(ITEM_BUFFER-i-1)*2);
  create_ibuffer_list();
  send_message(E_GUI,9,E_CONTROL,1,ls_buffer);
  }

void open_item_win()
  {
  create_ibuffer_list();
  if (find_window(item_win)==NULL)
     {
     CTL3D b1,b2;
     FC_TABLE f_sel;

     memcpy(&b1,def_border(1,0),sizeof(CTL3D));
     memcpy(&b2,def_border(5,WINCOLOR),sizeof(CTL3D));
     default_font=vga_font;
     memcpy(f_default,flat_color(0x0000),sizeof(charcolors));
     memcpy(&f_sel,flat_color(RGB555(0,0,24)),sizeof(charcolors));
     item_win=def_window(200,300,"P�edm�ty");
     waktual->y=2;waktual->x=SCR_WIDTH_X-20-160-3;
     waktual->minsizx=120;
     waktual->minsizy=100;
     gui_on_change(close_current);
     gui_define(9,5,20,170,240,0,listbox,ls_buffer,RGB555(31,31,0),0);c_default(0);
     gui_property(&b2,NULL,&f_sel,WINCOLOR);
     o_end->autoresizex=1;
     o_end->autoresizey=1;
     gui_define(10,3,41,19,198,1,scroll_bar_v,0,10,1,SCROLLBARCOL);
     gui_property(&b2,NULL,NULL,WINCOLOR);gui_on_change(scroll_support);
     o_end->autoresizey=1;
     gui_define(11,3,20,19,19,1,scroll_button,-1,0,"\x1e");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(12,3,40,19,19,2,scroll_button,1,10,"\x1f");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(20,3,1,10,10,2,resizer);
     gui_define(30,5,10,50,20,3,button,"P�ekr.");
     gui_property(&b1,NULL,NULL,WINCOLOR);gui_on_change(refresh_itm);
     gui_define(30,60,10,50,20,3,button,"Vyma�");
     gui_property(&b1,NULL,NULL,WINCOLOR);gui_on_change(it_kos);
     movesize_win(waktual,waktual->x,waktual->y,waktual->xs,150);
     redraw_window();

     }
  else
     select_window(item_win);
  }

void umisti_item()
  {
  int i,j;

  if (selected_place==-1)
     {
     msg_box("Nic nen� vybr�no",'\x01',"Nejd��ve klikni my�� na map�, na m�sto kam chce� p�edm�t um�stit","Ok",NULL);
     return;
     }
  i=f_get_value(0,9);
  sscanf(ls_sorts[i],"%d",&i);
  for(j=0;j<ITEM_BUFFER && item_buffer[j];j++);
  if (j==ITEM_BUFFER)
     {
     msg_box("Omezeni.",' ',"MAPEDIT omezil po�et p�edm�t� na jednom poli�ku. Dal�� ji� nelze vlo�it","Ok",NULL);
     return;
     }
  item_buffer[j]=i+1;
  open_item_win();
  send_message(E_GUI,9,E_CONTROL,1,ls_buffer);
  }

void select_item(int itpos);


void vloz_do_item()
  {
  int wht,ins,i;

  wht=f_get_value(vzor_win,9);
  sscanf(ls_sorts[wht],"%d",&wht);
  ins=f_get_value(item_win,9);
  sscanf(ls_buffer[ins],"%d",&ins);
  for(i=ITEM_BUFFER-1;i>ins;item_buffer[i]=item_buffer[i-1],i--);
  item_buffer[ins+1]=-wht-1;
  open_item_win();
  send_message(E_GUI,9,E_CONTROL,1,ls_buffer);
  }

void inv_testmap()
  {
  if (selected_place<0)
     msg_box("Nic nen� vybr�no",'\x01',"Nejd��ve klikni my�� na map�, na m�sto kde chce� za��t","Ok",NULL);
  else
     call_testmap(selected_place/4);
  }

static void change_grep()
  {
  char grep_en;
  int grep_type;

  grep_en=f_get_value(0,100);
  grep_type=f_get_value(0,90);
  if (grep_en) grep_num=grep_type;else grep_num=-1;
  send_message(E_GUI,9,E_CONTROL,0,&ls_sorts);
  create_isort_list(&ls_sorts,grep_num);
  send_message(E_GUI,9,E_CONTROL,1,ls_sorts);
  c_set_value(0,9,0);
  send_message(E_GUI,9,E_CONTROL,2);

  }

void editor_vzoru()
  {
  if (find_window(vzor_win)==NULL)
     {
     CTL3D b1,b2;
     FC_TABLE f_sel;

     create_isort_list(&ls_sorts,grep_num);
     memcpy(&b1,def_border(1,0),sizeof(CTL3D));
     memcpy(&b2,def_border(5,WINCOLOR),sizeof(CTL3D));
     default_font=vga_font;
     memcpy(f_default,flat_color(0x0000),sizeof(charcolors));
     memcpy(&f_sel,flat_color(RGB555(0,0,20)),sizeof(charcolors));
     vzor_win=def_window(200,370,"Vzory");
     waktual->minsizx=130;
     waktual->minsizy=170;
     gui_on_change(close_current);
     gui_define(9,5,20,170,240,0,listbox,ls_sorts,RGB555(31,31,0),0);c_default(0);
     gui_property(&b2,NULL,&f_sel,WINCOLOR);
     o_end->autoresizex=1;
     o_end->autoresizey=1;
     gui_define(10,3,41,19,198,1,scroll_bar_v,0,10,1,SCROLLBARCOL);
     gui_property(&b2,NULL,NULL,WINCOLOR);gui_on_change(scroll_support);
     o_end->autoresizey=1;
     gui_define(11,3,20,19,19,1,scroll_button,-1,0,"\x1e");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(12,3,110,19,19,2,scroll_button,1,10,"\x1f");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(20,3,1,10,10,2,resizer);
     gui_define(30,5,35,50,20,3,button,"Vyma�");gui_on_change(delete_item_sort);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(40,5,60,50,20,3,button,"Klonuj");gui_on_change(clone_item);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(50,5,85,50,20,3,button,"Oprav");gui_on_change(edit_selected_item);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(60,10,35,60,20,2,button,"Um�stit");gui_on_change(umisti_item);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(70,10,60,60,20,2,button,"Vlo� do");gui_on_change(vloz_do_item);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(80,10,85,60,20,2,button,"Test");gui_on_change(inv_testmap);
     gui_property(&b1,NULL,NULL,WINCOLOR);
     gui_define(90,5,10,170,12,3,str_line,typy_veci);c_default(grep_num>-1?grep_num:0);
     gui_property(&b2,NULL,&f_sel,WINCOLOR);gui_on_enter(string_list_sup);gui_on_change(change_grep);
     o_end->autoresizex=1;
     gui_define(100,10,10,10,10,2,check_box,"");c_default(grep_num>-1);gui_on_change(change_grep);
     movesize_win(waktual,waktual->x,160,waktual->xs,SCR_WIDTH_Y-20-160);
     redraw_window();
     }
  else
     select_window(vzor_win);
  }

void select_item(int itpos)
  {
  if (selected_place!=-1)
     {
     int ps;
     for(ps=0;item_buffer[ps];ps++);
     if (item_place[selected_place]!=NULL) free(item_place[selected_place]);
     if (ps)
        {
        item_place[selected_place]=getmem((ps+1)*sizeof(**item_place));
        memcpy(item_place[selected_place],item_buffer,_msize(item_place[selected_place]));
        item_place[selected_place][ps]=0;
        }
     else item_place[selected_place]=NULL;
     }
  clear_it_buffer();
  if (itpos!=-1 && item_place[itpos]!=NULL)
     memcpy(item_buffer,item_place[itpos],_msize(item_place[itpos]));
  selected_place=itpos;
  if (itpos!=-1)
     {
     open_item_win();
     send_message(E_GUI,9,E_CONTROL,1,ls_buffer);
     }
  }

char save_item_map(FILE *f,int id)
  {
  int totalmem=0,i,j;
  short *c,*d;
  long r;

  select_item(-1);
  for(i=0;i<maplen*4;i++)
     {
     if (item_place[i]!=NULL)
        {
        totalmem+=sizeof(int)+sizeof(short);
        for(j=0;j<ITEM_BUFFER && item_place[i][j];j++,totalmem+=sizeof(short));
        }
     }
  if (!totalmem) return 1;
  c=(short *)getmem(totalmem);d=c;
  for(i=0;i<maplen*4;i++)
     if (item_place[i]!=NULL)
        {
        *d++=i & 0xffff;
        *d++=i >> 16;
        for(j=0;j<ITEM_BUFFER && item_place[i][j];j++) *d++=item_place[i][j];
        *d++=0;
        }
  r=save_section(f,c,id,totalmem);
  free(c);
  return r==totalmem;
  }

void load_item_map(void *data,int size)
  {
  short *d,*e;
  int mode=0;
  int sect;

  d=data;
  while (size)
     {
     size-=2;
     switch (mode)
        {
        case 0:sect=*d++;mode++;break;
        case 1:sect+=(*d++)<<16;
               mode++;
               selected_place=sect;
               clear_it_buffer();
               e=item_buffer;
               break;
        case 2:*e++=*d;
               if (!(*d++))
                 {
                 select_item(-1);
                 mode=0;
                 }
               break;
        }
     }
  }

void editor_veci()
  {
  tool_sel=50;
  create_map_win(5,5,SCR_WIDTH_X-220,SCR_WIDTH_Y-60);
  change_tools();
  open_item_win();
  editor_vzoru();
  }



void add_record(void **ptr,int *count,int sizef)
  {
  void *p;
  int siz;

  siz=*count*sizef;
  count[0]++;
  p=getmem(siz+sizef);
  if (siz) memcpy(p,*ptr,siz);
  if (*ptr!=NULL) free(*ptr);
  *ptr=p;
  }

void insert_record(void **ptr,int *count,int sizef,int pos)
  {
  int siz;
  char *c,*d;

  siz=*count*sizef;
  add_record(ptr,count,sizef);
  pos*=sizef;
  c=*ptr;c+=pos;d=c+sizef;
  if (siz!=pos)memmove(d,c,siz-pos);
  }


void delete_record(void **ptr,int *count,int sizef,int record)
  {
  char *c1,*c2,*c3;
  int siz;
  void *p;

  if (!count[0] || record>=count[0]) return;
  c3=c1=*ptr;
  c1+=record*sizef;
  c2=c1+sizef;
  siz=*count*sizef;
  c3+=siz;
  if (c3!=c2)memmove(c1,c2,(c3-c2));
  p=realloc(*ptr,siz-sizef);
  if (siz-sizef<=0) {*ptr=NULL;p=NULL;}
  if (p!=NULL) *ptr=p;
  count[0]--;
  }


void re_build_shop_list(TSTR_LIST *ls,TSHOP *p, int count)
  {
  if (*ls!=NULL) release_list(*ls);
  *ls=create_list(count+1);
  while(count--)
     {
     char s[100];

     sprintf(s,"%s  (%d)",p->keeper,p->products);
     str_add(ls,s);
     p++;
     }
  str_add(ls,"<nov� obchod>");
  }


static void refresh_shop_list()
  {
  TSTR_LIST ls;

  send_message(E_GUI,9,E_CONTROL,0,&ls);
  re_build_shop_list(&ls,shop_list,max_shops);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  }

void re_build_item_list(TSTR_LIST *ls,TPRODUCT *p,int count)
  {

  if (*ls!=NULL) release_list(*ls);
  *ls=create_list(16);
  if (!count || p==NULL)
     {
     str_add(ls,"<��dn� p�edm�t>");
     return;
     }
  while (count--)
     {
     char s[200];
     short d;

     d=p->item;
     if (p->trade_flags & SHP_TYPE)
        sprintf(s,"typ: <%s>",typy_veci[d]);
     else if (d>=max_items) strcpy(s,"<p�edm�t vymaz�n!>");
     else
        sprintf(s,"%c%c%c%c %s (%d)",(p->trade_flags & SHP_SELL?'P':219),
                                    (p->trade_flags & SHP_BUY?'N':219),
                                    (p->trade_flags & SHP_AUTOADD?'Z':219),
                                    (p->trade_flags & SHP_SPECIAL?'S':219),
                                    item_list[d].jmeno,p->pocet
                                    );
     p++;
     str_add(ls,s);
     }
  }

TSHOP *create_shop()
  {
  TSHOP *p;
  add_record(&shop_list,&max_shops,sizeof(TSHOP));
  p=shop_list+max_shops-1;
  memset(p,0,sizeof(*p));
  p->koef=10;
  p->shop_id=shop_max_id++;
  return p;
  }

static char *shopstate=NULL;

static void close_edit_shop()
  {
  TSHOP *p;
  TSTR_LIST ls;
  char s[20];
  int i;

  p=shop_list+f_get_value(0,500);
  get_value(0,200,s);
  get_value(0,210,p->picture);
  i=f_get_value(0,500);
  p->koef=vals(220);
  p->spec_max=vals(230);
  if (s[0]==0)
     {
     msg_box("Editor obchod�",'\x1',"Obchod mus� m�t sv�ho prodava�e (shopkeeppera). Obchod bude ozna�en jako smazan�.","Ok",NULL);
     strcpy(s,"*vymaz�n*");
     }
  if (s[0]=='<')
     {
     msg_box("Editor obchod�",'\x1',"Obchodn�k m� neplatn� jm�no","Ok",NULL);
     return;
     }
  strcpy(p->keeper,s);
  if (!p->products && s[0]!='*')
     msg_box("Editor obchod�",'\x1',"S ni��m se neobchoduje. Nejsou definov�ny ��dn� produkty. Tato situace m��e v�st ke krachu hry!","Beru na v�dom�",NULL);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  release_list(ls);
  close_current();
  select_window(shop_window);
  refresh_shop_list();
  shopstate[i]=0;
  }

static void add_item()
  {
  TSHOP *p;
  TSTR_LIST ls;
  TPRODUCT *pr;
  int i;

  p=shop_list+f_get_value(0,500);
  i=f_get_value(0,9);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  insert_record(&p->list,&p->products,sizeof(*pr),i);
  pr=p->list+i;
  pr->trade_flags=get_bit_fields(0,130,4);
  pr->cena=vals(110);
  pr->max_pocet=pr->pocet=vals(120);
  pr->item=f_get_value(0,100);
  sscanf(shop_items[pr->item],"%hd",&pr->item);
  re_build_item_list(&ls,p->list,p->products);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  redraw_window();
  }

static void rewrite_item()
  {
  TSHOP *p;
  TSTR_LIST ls;
  TPRODUCT *pr;
  int i;

  p=shop_list+f_get_value(0,500);
  i=f_get_value(0,9);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  if (i>=p->products) return;
  pr=p->list+i;
  if (i>=pr->max_pocet) return;
  pr->trade_flags=get_bit_fields(0,130,4);
  pr->cena=vals(110);
  pr->max_pocet=pr->pocet=vals(120);
  pr->item=f_get_value(0,100);
  sscanf(shop_items[pr->item],"%hd",&pr->item);
  re_build_item_list(&ls,p->list,p->products);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  redraw_window();
  }

static void show_selected()
  {
  TSHOP *p;
  TPRODUCT *pr;
  int i;
  char c[30];

  p=shop_list+f_get_value(0,500);
  i=f_get_value(0,9);
  if (i>=p->products) return;
  pr=p->list+i;
  if (pr->trade_flags & SHP_TYPE) return;
  sprintf(c,"%7d",pr->cena);
  set_value(0,110,c);
  sprintf(c,"%7d",pr->max_pocet);
  set_value(0,120,c);
  c_set_value(0,100,pr->item);
  fill_bit_fields(0,130,pr->trade_flags,4);
  }

static void add_type()
  {
  TSHOP *p;
  TSTR_LIST ls;
  TPRODUCT *pr;

  p=shop_list+f_get_value(0,500);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  add_record(&p->list,&p->products,sizeof(*pr));
  pr=p->list+p->products-1;
  pr->trade_flags=SHP_BUY|SHP_TYPE;
  pr->cena=-1;
  pr->max_pocet=pr->pocet=-1;
  pr->item=item_list[f_get_value(0,100)].druh;
  re_build_item_list(&ls,p->list,p->products);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  redraw_window();
  }

static void remove_item()
  {
  TSTR_LIST ls;
  int i;

  TSHOP *p;
  p=shop_list+f_get_value(0,500);
  i=f_get_value(0,9);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  delete_record(&p->list,&p->products,sizeof(TPRODUCT),i);
  re_build_item_list(&ls,p->list,p->products);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  redraw_window();
  }

static void zjisti_cenu()
  {
  int i=f_get_value(0,100);
  int j=9999999;

  sscanf(shop_items[i],"%d",&j);
  if (j<max_items)
     {
     char s[20];

     sprintf(s,"%7d",item_list[j].cena);
     set_value(0,110,s);
     }
  set_enable(0,20,j<max_items);
  set_enable(0,30,j<max_items);
  }

static void browse_dialogs()
  {
  char c[15]="";
  const char *d;
  char *e;

  d=get_text_field(config_file,"CESTA_DIALOGY");if (d==NULL) d="";
  concat(e,d,"*.hi");
  browser(e,c);
  if (c[0]) set_value(0,210,c);
  }

#undef ctl1

void edit_shop(int i)
  {
  
  TSTR_LIST ls=NULL;
  TSHOP *shp=shop_list+i;
  CTL3D ctl1,ctl2;

  re_build_item_list(&ls,shp->list,shp->products);
  memcpy(&ctl1,def_border(1,0),sizeof(ctl1));
  memcpy(&ctl2,def_border(2,WINCOLOR),sizeof(ctl2));
  def_window(400,300,"Editor obchod�");
  gui_on_change(close_edit_shop);
  gui_define(500,0,0,0,0,0,value_store,4);c_default(i);
  gui_define(9,10,20,200,166,0,listbox,ls,RGB555(31,31,31),0);
  gui_property(&ctl1,NULL,NULL,WINCOLOR);c_default(0);gui_on_change(show_selected);
  gui_define(10,217,40,19,127,0,scroll_bar_v,0,10,1,SCROLLBARCOL);
  gui_property(&ctl1,NULL,NULL,WINCOLOR);
  gui_define(11,216,20,21,17,0,scroll_button,-1,0,"\x1e");
  gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
  gui_define(12,216,170,21,17,0,scroll_button,1,10,"\x1f");
  gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
  gui_define(20,75,30,60,20,1,button,"Vo�it");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(add_item);
  gui_define(60,75,55,60,20,1,button,"P�epsat");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(rewrite_item);
  gui_define(30,10,30,60,20,1,button,"Typ");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(add_type);
  gui_define(40,10,55,60,20,1,button,"Odebrat");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(remove_item);
  gui_define(50,10,10,80,20,2,button,"Zav��t");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(close_edit_shop);
  gui_define(100,10,80,150,12,1,str_line,shop_items);gui_property(&ctl2,NULL,NULL,WINCOLOR);
     c_default(0); gui_on_enter(string_list_sup);gui_on_change(zjisti_cenu);
  gui_define(-1,155,100,1,1,1,label,"Cena:");
  gui_define(110,10,100,90,12,1,input_line,10,0,9999999,"%7d");gui_property(&ctl2,NULL,NULL,WINCOLOR);
     set_default(strs(item_list[0].cena));gui_on_exit(test_int);
  gui_define(-1,155,120,1,1,1,label,"Po�et:");
  gui_define(120,10,120,90,12,1,input_line,10,-1,9999999,"%7d");gui_property(&ctl2,NULL,NULL,WINCOLOR);
     set_default(strs(1));gui_on_exit(test_int);
  gui_define(130,10,135,150,10,1,check_box,"Prod�vat");c_default(1);
  gui_define(140,10,147,150,10,1,check_box,"Nakupovat");c_default(0);
  gui_define(150,10,159,150,10,1,check_box,"Zbo�� p�ib�v�");c_default(0);
  gui_define(160,10,171,150,10,1,check_box,"Nen� v�dy na sklad�");c_default(0);
  gui_define(-1,10,200,1,1,0,label,"Shopkeeper:");
  gui_define(200,100,200,200,12,0,input_line,15);set_default(shp->keeper);gui_property(&ctl2,NULL,NULL,WINCOLOR);
  gui_define(-1,10,220,1,1,0,label,"Obr�zek:");
  gui_define(210,100,220,100,12,0,input_line,12);set_default(shp->picture);gui_property(&ctl2,NULL,NULL,WINCOLOR);
  gui_define(215,220,218,80,14,0,button,"Nalistuj");gui_property(&ctl1,NULL,NULL,WINCOLOR);gui_on_change(browse_dialogs);
  gui_define(-1,10,240,1,1,0,label,"Rozptyl cen +/- [%]");
  gui_define(220,150,240,60,12,0,input_line,6,0,100,"%5d");set_default(strs(shp->koef));gui_on_exit(test_int);gui_property(&ctl2,NULL,NULL,WINCOLOR);
  gui_define(-1,220,240,1,1,0,label,"Po�et spec:");
  gui_define(230,320,240,60,12,0,input_line,6,0,100,"%5d");set_default(strs(shp->spec_max));gui_on_exit(test_int);gui_property(&ctl2,NULL,NULL,WINCOLOR);
     {
     char s[50];
     sprintf(s,"Shop ID: %d",shp->shop_id);
     gui_define(-1,10,260,1,1,0,label,s);
     }

  redraw_window();
  }

static void close_shop_list()
  {
  TSTR_LIST ls;
  int i;

  send_message(E_GUI,9,E_CONTROL,0,&ls);
  for(i=0;i<max_shops;i++) if (shopstate[i]==1)
        {
        msg_box("Toto okno nelze uzav��t!",'\x1',"Program �ek� na dokon�en� prace v n�kter�m z pod��zen�ch oken","Ok",NULL);
        return;
        }
  for(i=0;i<max_shops;)
     if (shop_list[i].keeper[0]=='*')
        {
        if (shop_list[i].list!=NULL) free(shop_list[i].list);
        delete_record(&shop_list,&max_shops,sizeof(TSHOP),i);
        }
     else i++;
  release_list(ls);
  release_list(shop_items);
  free(shopstate);
  close_current();
  }

void open_shop_editor()
  {
  int i;
  TSTR_LIST ls;
//  TSHOP *p;


  if (check_data_password()==0) return;
  i=f_get_value(0,9);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  if (i==max_shops)
     {
     create_shop();
     shopstate=(char *)grealloc(shopstate,max_shops);
     re_build_shop_list(&ls,shop_list,max_shops);
     shopstate[i]=1;
     send_message(E_GUI,9,E_CONTROL,1,ls);
     edit_shop(i);
     }
  else
     {
     if (i>=max_shops) return;
     if (ls[i]==NULL) return;
     if (shopstate[i]==1)
        {
        msg_box("Seznam obchod�",'\x1',"Tento obchod je ji� opravov�n v jin�m okn�. Nelze jej znovu otev��t","Ok",NULL);
        return;
        }
     shopstate[i]=1;
     send_message(E_GUI,9,E_CONTROL,1,ls);
     edit_shop(i);
     }
  }

static void erase_shop()
  {
  int i;
  TSTR_LIST ls;
  TSHOP *p;

  if (check_data_password()==0) return;
  i=f_get_value(0,9);
  send_message(E_GUI,9,E_CONTROL,0,&ls);
  if (i>=max_shops) return;
  if (ls[i]==NULL) return;
  if (ls[i][0]=='<')
        {
        msg_box("Seznam obchod�",'\x1',"Tento obchod je opravov�n v jin�m okn�. Nelze jej vymazat","Ok",NULL);
        return;
        }
  if (msg_box("Vymazat obchod?",'\x2',"Opravdu chce� vymazat obchod?","Ano","Ne",NULL)==2) return;
  p=shop_list+i;
  strcpy(p->keeper,"*vymaz�n*");
  re_build_shop_list(&ls,shop_list,max_shops);
  send_message(E_GUI,9,E_CONTROL,1,ls);
  }


void open_shop_list()
  {
  TSTR_LIST ls=NULL;
  CTL3D ctl1;


  if (find_window(shop_window)==NULL)
     {
     create_isort_list(&ls_sorts,grep_num);
     shopstate=NewArr(char,max_shops);memset(shopstate,0,max_shops);
     shop_items=ls_sorts;
     ls_sorts=NULL;
     re_build_shop_list(&ls,shop_list,max_shops);
     memcpy(&ctl1,def_border(1,0),sizeof(ctl1));
     shop_window=def_window(250,220,"Seznam obchod�");
     gui_on_change(close_shop_list);
     gui_define(9,10,20,200,166,0,listbox,ls,RGB555(31,31,31),0);
     gui_property(&ctl1,NULL,NULL,WINCOLOR);c_default(0);//gui_on_change(edit_shop_item);
     gui_define(10,217,40,19,127,0,scroll_bar_v,0,10,1,SCROLLBARCOL);
     gui_property(&ctl1,NULL,NULL,WINCOLOR);
     gui_define(11,216,20,21,17,0,scroll_button,-1,0,"\x1e");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(12,216,170,21,17,0,scroll_button,1,10,"\x1f");
     gui_property(NULL,icones,NULL,WINCOLOR);gui_on_change(scroll_support);
     gui_define(200,5,5,70,20,3,button,"Oprava");gui_on_change(open_shop_editor);gui_property(&ctl1,NULL,NULL,WINCOLOR);
     gui_define(210,80,5,70,20,3,button,"Vymazat");gui_on_change(erase_shop);gui_property(&ctl1,NULL,NULL,WINCOLOR);
     movesize_win(waktual,10,10,250,220);
     }
  else select_window(shop_window);
  redraw_window();
  }

void shop_train_edit()
  {
  open_shop_list();
  }


void save_shop(TSHOP *p,FILE *f)
  {
  char *itm;
  int i,j,typs=0,lss;
  TPRODUCT *pstr,*pr,*nls,*npr;
  TSHOP *shp;

  shp=p;p=New(TSHOP);
  memcpy(p,shp,sizeof(*p));
  itm=getmem(max_items);
  memset(itm,0,max_items);
  pstr=p->list;pr=pstr;
  for(i=0;i<p->products;i++,pr++)
     {
     if (pr->trade_flags & SHP_TYPE)
        {
        typs++;
        for(j=0;j<max_items;j++) if (item_list[j].druh==pr->item) itm[j]|=1;
        }
     else
        if (itm[pr->item])
           {
           char s[300];

           sprintf(s,"Duplicitn� p�edm�t v definici v obchodu '%s'. Soubor obchodu bude ulo�en, ale vlastn� hra m��e na tomto p�edm�tu zkrachovat!",p->keeper);
           msg_box("Shop edit",'\x1',s,"Ok",NULL);
           typs++;
           }
        else
         itm[pr->item]|=2;
     }
  lss=0;
  for(i=0;i<max_items;i++) if (itm[i]) lss++;
  nls=NewArr(TPRODUCT,typs+lss);
  pr=pstr;npr=nls;
  for(i=0;i<p->products;i++,pr++)
     if (!(pr->trade_flags & SHP_TYPE)) memcpy(npr++,pr,sizeof(TPRODUCT));
  for(i=0;i<max_items;i++)
     if (itm[i]==1)
        {
        npr->item=i;
        npr->cena=item_list[i].cena;
        npr->trade_flags=SHP_BUY | SHP_NOEDIT | SHP_SELL;
        npr->pocet=npr->max_pocet=0;
        npr++;
        }
  pr=pstr;
  for(i=0;i<p->products;i++,pr++)
     if (pr->trade_flags & SHP_TYPE) memcpy(npr++,pr,sizeof(TPRODUCT));
  p->products=typs+lss;
  p->list_size=lss;
  fwrite(p,1,sizeof(TSHOP),f);
  fwrite(nls,p->products,sizeof(TPRODUCT),f);
  free(nls);
  free(itm);
  free(p);
  }

void save_all_shops()
  {
  FILE *f;
  int i;
  int pocet=max_shops;

  f=fopen(SHOP_NAME,"wb");
  fwrite(&pocet,1,4,f);
  for(i=0;i<pocet;i++) save_shop(shop_list+i,f);
  fclose(f);
  }


void load_shop(TSHOP *p,FILE *f)
  {
  int lss,i,el;
  TPRODUCT *nls,*ols,*pr,*tr;

  fread(p,1,sizeof(TSHOP),f);
  if (p->shop_id>=shop_max_id) shop_max_id=p->shop_id+1;
  lss=p->products;
  ols=NewArr(TPRODUCT,lss);
  fread(ols,lss,sizeof(TPRODUCT),f);
  el=0;
  for(i=0;i<lss;i++) if (!(ols[i].trade_flags & SHP_NOEDIT)) el++;
  nls=NewArr(TPRODUCT,el);
  pr=nls,tr=ols;
  for(i=0;i<lss;i++,tr++) if (!(tr->trade_flags & SHP_NOEDIT))  memcpy(pr++,tr,sizeof(TPRODUCT));
  free(ols);
  p->list=nls;
  p->products=el;
  }


void load_all_shops()
  {
  FILE *f;
  int i;
  int pocet=0;

  f=fopen(SHOP_NAME,"rb");
  if (f==NULL) return;
  fread(&pocet,1,4,f);
  if (!pocet)
     {
     fclose(f);
     return;
     }
  shop_list=NewArr(TSHOP,pocet);
  for(i=0;i<pocet;i++) load_shop(shop_list+i,f);
  max_shops=pocet;
  fclose(f);
  }
