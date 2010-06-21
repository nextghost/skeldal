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
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cassert>
#include <inttypes.h>
#include "libs/event.h"
#include <cctype>
#include "libs/memman.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/strlite.h"
#include "libs/mgifmem.h"
#include "game/engine1.h"
#include "libs/pcx.h"
#include "game/globals.h"
#include "libs/system.h"

typedef struct t_paragraph {
	unsigned num:15;
	unsigned alt:15;
	unsigned visited:1;
	unsigned first:1;
	int32_t position;
} T_PARAGRAPH;

class DialogList : public DataBlock {
public:
	struct Dialog {
		unsigned num, alt, visited, first;
		int position;
	};

private:
	unsigned _size;
	Dialog *_info;
	MemoryReadStream *_data;

	// do not implement
	DialogList(const DialogList &src);
	const DialogList &operator=(const DialogList &src);

public:
	DialogList(SeekableReadStream &stream);
	~DialogList(void);

	void setVisited(unsigned idx, unsigned value);
	void setFirst(unsigned idx, unsigned value);

	const Dialog &operator[](unsigned idx) const;
	unsigned size(void) const { return _size; }
	unsigned find(unsigned num) const;
	int findNum(unsigned pos) const;
	MemoryReadStream *data(void) const { return _data; }
};

#define STR_BUFF_SIZ 4096
#define SAVE_POSTS 20
#define P_STRING 1
#define P_SHORT 2
#define P_VAR 3

#define MAX_VOLEB 10
#define VAR_COUNT 20

#define VOLBY_X 85
#define VOLBY_Y 398
#define VOLBY_XS 450
#define VOLBY_YS 10

#define TEXT_X 17
#define TEXT_Y 270
#define TEXT_XS 606
#define TEXT_YS 94
#define TEXT_STEP 11

#define OPER_EQ 32
#define OPER_BIG 35
#define OPER_LOW 33
#define OPER_BIGEQ 36
#define OPER_LOWEQ 34
#define OPER_NOEQ 37

#define PIC_X 17
#define PIC_Y (17+SCREEN_OFFLINE)

#define DESC_COLOR1 1,231,231,173

static short varibles[VAR_COUNT];

static char sn_nums[SAVE_POSTS];
static char sn_nams[SAVE_POSTS][32];
static char sn_rods[SAVE_POSTS];

static Texture *back_pic;
static char back_pic_enable=0;

static char showed=0;
static MemoryReadStream *pstream = NULL;
static char *descript=NULL;
static char *string_buffer=NULL;
static char iff;

static char _flag_map[32];

static int local_pgf=0;

static char story_on=0;

static char pocet_voleb=0;
static char vyb_volba=0;
static short vol_n[MAX_VOLEB];
//static char *vol_s[MAX_VOLEB];
static short save_jump;

static StringList history;
static int his_line=0;
static int end_text_line=0;
static int last_his_line=0;

static int starting_shop=-1;

static char halt_flag=0;

static int dialog_mob=0;

static char code_page=1;

static char case_click(int id,int xa,int ya,int xr,int yr);
static char ask_who_proc(int id,int xa,int ya,int xr,int yr);


#define CLK_DIALOG 3
static T_CLK_MAP clk_dialog[CLK_DIALOG]=
  {
  {0,TEXT_X,TEXT_Y,TEXT_X+TEXT_XS,TEXT_Y+TEXT_YS,case_click,3,H_MS_DEFAULT},
  {-1,30,0,85,14,konec,2,H_MS_DEFAULT},
  {0,0,0,639,479,empty_clk,0xff,H_MS_DEFAULT},
  };

#define CLK_DLG_WHO 3
static T_CLK_MAP clk_dlg_who[CLK_DLG_WHO]=
  {
  {1,54,378,497,479,ask_who_proc,2,-1},
  {2,0,0,639,479,ask_who_proc,8,-1},
  {-1,0,0,639,479,empty_clk,0xff,-1},
  };


static int glob_y;

static int last_pgf;

static short task_num=-1;

static void dialog_anim(va_list args) {
//#pragma aux dialog_anim parm []
	char *block = va_arg(args, char *);
	int speed = va_arg(args, int);
	int rep = va_arg(args, int);

	void *anm;
	char *ch;
	char hid;
	int spdc = 0, cntr = rep, tm, tm2, ret = 1;
	File file;

	file.open(Sys_FullPath(SR_DIALOGS, block));
	free(block);

	do {
		file.seek(0, SEEK_SET);
		MGIFReader reader(file, 320, 180, 0);

		while (ret && Task_QuitMsg()) {
			Task_Sleep(NULL);

			if (!spdc) {
				if (ms_last_event.x <= PIC_X + 320 && ms_last_event.y <= PIC_Y + 180) {
					hid = 1;
					schovej_mysku();
				} else {
					hid = 0;
				}

				ret = reader.decodeFrame();
				spdc = speed;

				if (ret & FLAG_VIDEO) {
					renderer->blit(reader, PIC_X, PIC_Y, reader.palette());
				}

				if (hid) {
					ukaz_mysku();
				}

				showview(PIC_X, PIC_Y, 320, 180);
			}

			tm2 = Timer_GetValue();

			if (tm != tm2) {
				spdc--;
				tm = tm2;
			}
		}

		rep--;
	} while (!cntr && rep && !Task_QuitMsg());
}

static void stop_anim()
  {
  if (task_num!=-1) Task_Term(task_num);
  }

static void run_anim(char *name,int speed,int rep)
  {
  char *bl;
  stop_anim();
  bl = (char*)getmem(strlen(name)+1);
  strcpy(bl,name);
  task_num=Task_Add(8196,dialog_anim,bl,speed,rep);
  }

static void error(const char *text)
  {
  char buff[256];
  sprintf(buff,"%s v odstavci %d\r\nLocal_pgf=%d / DIALOG : %d / SENTENCE : %d\r\n",text,last_pgf+local_pgf,local_pgf,local_pgf/128,last_pgf);
//  MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
  Sys_ErrorBox(buff);
  SEND_LOG("(DIALOGS) Dialog error detected at %d:%d",local_pgf/128,last_pgf);
  SEND_LOG("(DIALOGS) Error description: %s",text,0);  
  }

static void show_dialog_picture() {
	if (!showed) {
		const Texture *tex = dynamic_cast<const Texture*>(ablock(H_DIALOG));
		renderer->blit(*tex, 0, SCREEN_OFFLINE, tex->palette());
		showed = 1;
		glob_y = 250;
	}
}

DialogList::DialogList(SeekableReadStream &stream) : _size(0), _info(NULL),
	_data(NULL) {
	int i;
	unsigned tmp;

	_size = stream.readUint32LE();
	_info = new Dialog[_size];
	stream.readUint32LE();	// _data length, ignore and recalculate

	for (i = 0; i < _size; i++) {
		tmp = stream.readUint32LE();
		_info[i].num = tmp & 0x7fff;
		_info[i].alt = (tmp >> 15) & 0x7fff;
		_info[i].visited = (tmp >> 30) & 0x1;
		_info[i].first = (tmp >> 31) & 0x1;
		_info[i].position = stream.readSint32LE();
	}

	_data = stream.readStream(stream.size() - stream.pos());
}

DialogList::~DialogList(void) {
	delete[] _info;
	delete _data;
}

void DialogList::setVisited(unsigned idx, unsigned value) {
	assert(idx < _size && "Invalid dialog index");
	_info[idx].visited = value;
}

void DialogList::setFirst(unsigned idx, unsigned value) {
	assert(idx < _size && "Invalid dialog index");
	_info[idx].first = value;
}

const DialogList::Dialog &DialogList::operator[](unsigned idx) const {
	assert(idx < _size && "Invalid dialog index");
	return _info[idx];
}

unsigned DialogList::find(unsigned num) const {
	int i;

	for (i = 0; i < _size; i++) {
		if (_info[i].num == num) {
			return i;
		}
	}

	assert(0 && "Paragraph not found");
}

int DialogList::findNum(unsigned pos) const {
	int i, num = -1;

	for (i = 0; i < _size; i++) {
		if (_info[i].position > pos) {
			break;
		} else {
			num = _info[i].num;
		}
	}

	return num;
}

DataBlock *loadDialogs(SeekableReadStream &stream) {
	return new DialogList(stream);
}

static unsigned find_paragraph(int num) {
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));
	return list->find(num + local_pgf);
}

static int find_pgnum(unsigned pos) {
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));
	return list->findNum(pos) - local_pgf;
}

static void goto_paragraph(int prgf) {
	unsigned idx;
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	do {
		idx = find_paragraph(prgf);
		const DialogList::Dialog &dlg = (*list)[idx];

		if (dlg.visited) {
			list->setFirst(idx, 1);
		}

		if (dlg.alt == dlg.num || !dlg.visited) {
			pstream = list->data();
			pstream->seek(dlg.position, SEEK_SET);
			last_pgf = prgf;
			list->setVisited(idx, 1);
			return;
		}

		prgf = dlg.alt - local_pgf;
		do_events();
	} while (1);
}

static char *transfer_text(const char *source, char *target)
  {
  const char *orgn = source;
  char *ot=target;
  int num;
  while (*source)
     {
     if (*source=='%')
        {
        source++;
        switch(*source)
           {
           case '[':*target++='[';break;
           case ']':*target++=']';break;
           case 'a':*target++='\'';break;
           case 'p':
           case '%':*target++='%';break;
           case 'n':strcpy(target,sn_nams[0]);target+=strlen(sn_nams[0]);break;
           default: num=0;while (isdigit(*source)) num=10*num+*source++-'0';
                    if (*source=='l')
                       {
                       sn_nums[0]=sn_nums[num];
                       strcpy(sn_nams[0],sn_nams[num]);
                       sn_rods[0]=sn_rods[num];
                       }
                    break;
           }
           source++;
        }
     else if (*source=='[')
        {
        source++;
        num=sn_rods[0];
        while(num>0)
           {
           source=strchr(source,',');
           num--;
           if (source==NULL)
              {
			  char buff[256];
              closemode();
			  sprintf(buff,"%s\r\nChybny rod nebo maly pocet tvaru od jednoho slova",orgn);
              error(buff);
              exit(-1);
              }
           source++;
           }
        while (*source!=',' && *source!=']' && *source!=0) *target++=*source++;
        if (*source!=']')
           {
           source=strchr(source,']');
           if (source==NULL)
              {
			  char buff[256];
              closemode();
			  sprintf(buff,"%s\r\nOcekava se ]",orgn);
              error(buff);
              exit(-1);
              }
           }
        source++;
        }
     else *target++=*source++;
     }
  *target=0;
  if (code_page==2)
     prekodovat(ot);
  return target;
  }

static char *conv_text(const char *source)
  {
  if (string_buffer==NULL) string_buffer = (char*)getmem(STR_BUFF_SIZ);
  return transfer_text(source,string_buffer);
  }

static char *Get_string() {
	char *c, i;

	i = pstream->readUint8();

	if (i == P_STRING) {
		c = conv_text(pstream->readCString());

		for (i = pstream->readUint8(); i == P_STRING; i = pstream->readUint8()) {
			c = transfer_text(pstream->readCString(), c);
		}

		pstream->seek(-1, SEEK_CUR);
		return string_buffer;
	} else if (i == P_SHORT) {
		i = pstream->readSint16LE();

		if (i <= 0) {
			c = conv_text(texty[-i]);
		} else {
			c = conv_text(level_texts[i]);
		}

		return string_buffer;
	} else {
		assert(0 && "Expected string or string table index");
	}
}

static int Get_short() {
	int i = pstream->readUint8();

	if (i == P_SHORT) {
		return pstream->readSint16LE();
	} else if (i == P_VAR) {
		i = pstream->readUint16LE();
		assert(i < VAR_COUNT && "Invalid variable index");
		return varibles[i];
	} else {
		assert(0 && "Expected number or variable index");
	}
}

static void show_desc() {
	char *c = descript;
	int y;
	const Font *font;

	showed = 0;
	show_dialog_picture();

	if (c == NULL) {
		return;
	}

	y = 34;
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, DESC_COLOR1);

	while (*c) {
		renderer->drawText(382, y, c);
		y += renderer->textHeight(c);
		c = strchr(c, 0) + 1;
	}
}

static void add_desc(char *c) {
	int xs, ys;
	const Font *font;

	if (story_on) {
		write_story_text(c);
	}

	if (descript != NULL) {
		free(descript);
	}

	descript = (char *)getmem(strlen(c) + 2);
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 1, 255, 255, 255);
	zalamovani(c, descript, 225, &xs, &ys);
}

static void show_emote(char *c) {
	int xs, ys;
	char *a;
	const Font *font;

	if (story_on) {
		write_story_text(c);
	}

	a = (char*)alloca(strlen(c) + 2);
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 1, 255, 255, 255);
	zalamovani(c, a, TEXT_XS, &xs, &ys);

	while (*a) {
		char z[100] = "M";
		strcat(z, a);
		end_text_line = history.insert(z) + 1;
		a = strchr(a, 0) + 1;
	}
}


static void echo(const char *c) {
	int xs, ys;
	char *a;
	const Font *font;

	if (story_on) {
		write_story_text(c);
	}

	a = (char*)alloca(strlen(c) + 2);
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 1, 0, 247, 0);
	zalamovani(c, a, TEXT_XS, &xs, &ys);

	while (*a) {
		char z[100]="E";

		strcat(z, a);
		end_text_line = history.insert(z) + 1;
		a=strchr(a, 0) + 1;
	}
}

#define TEXT_UNSELECT 254
#define TEXT_SELECT 255

static void redraw_text() {
	int y = TEXT_Y;
	int ys = TEXT_YS;
	int ls_cn, i;
	const uint8_t *pal;
	const Font *font;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_DIALOG));

	//create_frame(TEXT_X,TEXT_Y,TEXT_XS,TEXT_YS,1);
	// FIXME: this seems to break dialogs, is it needed?
//	put_textured_bar(*tex, TEXT_X, TEXT_Y, TEXT_XS, TEXT_YS, TEXT_X, TEXT_Y - SCREEN_OFFLINE);
	ls_cn = history.size();

	if (ls_cn <= his_line) {
		return;
	}

	for (i = his_line; i < ls_cn; i++) {
		if (history[i] != NULL) {
			const char *c = history[i];

			font = dynamic_cast<const Font*>(ablock(H_FBOLD));

			if (*c == 'E') {
				renderer->setFont(font, 0, 0, 0, 0);
			} else if(*c == 'M') {
				pal = tex->palette() + 3 * TEXT_UNSELECT;
				renderer->setFont(font, 0, pal[0], pal[1], pal[2]);
			} else if(*c - 48 == vyb_volba) {
				pal = tex->palette() + 3 * TEXT_SELECT;
				renderer->setFont(font, 0, pal[0], pal[1], pal[2]);
			} else {
				pal = tex->palette() + 3 * TEXT_UNSELECT;
				renderer->setFont(font, 0, pal[0], pal[1], pal[2]);
			}

			c++;
			renderer->drawText(TEXT_X, y, c);
			y += TEXT_STEP;
			ys -= TEXT_STEP;

			if (ys < TEXT_STEP) {
				break;
			}
		}
	}
}

static int get_last_his_line()
  {
  int i=0,cf;

  cf = history.size();
  while(i<cf && history[i]!=NULL) i++;
  return i;
  }

static void draw_all() {
	const Texture *c;

	show_desc();

	if (back_pic_enable) {
		c = back_pic;
	} else {
		c = dynamic_cast<const Texture*>(ablock(H_DIALOG_PIC));
	}

	other_draw();

	if (c) {
		renderer->blit(*c, PIC_X, PIC_Y, c->palette());
	}

	redraw_text();
}

static void lecho(char *c)
  {
  write_story_text(c);
  }

static void save_name(int pos)
  {
  sn_nums[pos]=sn_nums[0];
  strcpy(sn_nams[pos],sn_nams[0]);
  sn_rods[pos]=sn_rods[0];
  }

static void load_name(int pos)
  {
  sn_nums[0]=sn_nums[pos];
  strcpy(sn_nams[0],sn_nams[pos]);
  sn_rods[0]=sn_rods[pos];
  }

static void nahodne(int vls,int omz,char check)
  {
  char chk[POCET_POSTAV];
  int i,l,m;

  memset(chk,0,sizeof(chk));
  if (!check) for(i=0;i<SAVE_POSTS;i++) if (sn_nums[i]<POCET_POSTAV) chk[sn_nums[i]]|=1;
  for(i=0;i<POCET_POSTAV;i++) if (postavy[i].sektor!=viewsector || !postavy[i].lives || !postavy[i].used) chk[i]|=2;
  m=0;l=-1;
  for(i=0;i<POCET_POSTAV;i++)
     if (postavy[i].vlastnosti[vls]>=omz && chk[i]==0)
        {
        if (l>=0) chk[l]|=4;
        l=-2;iff=0;
        }
     else if  (l!=-2 && chk[i]==0 && postavy[i].vlastnosti[vls]>=m)
           {
           if (l>=0) chk[l]|=4;
           l=i;iff=1;
           m=postavy[i].vlastnosti[vls];
           }
        else chk[i]|=4;
  m=0;
  for(i=0;i<POCET_POSTAV;i++) if (!chk[i])m++;
  if (m==0 && !check)
     {
     for(i=0;i<POCET_POSTAV;i++) chk[i]&=~1;
     for(i=0;i<POCET_POSTAV;i++) if (!chk[i])m++;
     iff=1;
     if (m==0) return;
     }
  l=rnd(m)+1;
  for(i=0;l>0;i++) if (!chk[i]) l--;
  i--;
  sn_nums[0]=i;
  strcpy(sn_nams[0],postavy[i].jmeno);
  sn_rods[0]=postavy[i].female;
  }

static void pc_xicht(int xichtid)
{
  int i;
  for (i=0;i<POCET_POSTAV;i++)
    if (postavy[i].used && postavy[i].xicht==xichtid)
    {
      sn_nums[0]=i;
      strcpy(sn_nams[0],postavy[i].jmeno);
      sn_rods[0]=postavy[i].female;
      iff=1;      
      return;
    }
iff=0;
return;
}

static char visited(int prgf) {
	unsigned idx = find_paragraph(prgf);
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	return (*list)[idx].visited;
}

static void set_nvisited(int prgf) {
	unsigned idx = find_paragraph(prgf);
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	list->setVisited(idx, 0);
	list->setFirst(idx, 0);
}


void q_flag(int flag)
  {
  iff=_flag_map[flag>>3] & (1<<(flag & 0x7));
  }

static void set_flag(int flag)
  {
  q_flag(flag);
  _flag_map[flag>>3]|=(1<<(flag & 0x7));
  }

static void reset_flag(int flag)
  {
  q_flag(flag);
  _flag_map[flag>>3]&=~(1<<(flag & 0x7));
  }

void change_flag(int flag,char mode)
  {
  if (mode==0) reset_flag(flag);
  else if (mode==1) set_flag(flag);
  else _flag_map[flag>>3]^=(1<<(flag & 0x7));
  }

char test_flag(int flag)
  {
  return (_flag_map[flag>>3] & (1<<(flag & 0x7)))!=0;
  }

static void first_visited(int prgf) {
	unsigned idx = find_paragraph(prgf);
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	iff = !(*list)[idx].first;
}



void do_dialog();
static void remove_all_cases();

static void dialog_cont()
     {
     save_jump=vol_n[vyb_volba];
     remove_all_cases();
     echo(" ");
     his_line=get_last_his_line();
     if (halt_flag) goto_paragraph(save_jump);
     schovej_mysku();
     do_dialog();
     }


static void key_check(EVENT_MSG *msg, void **unused) {
	char c, d;
	
	if (msg->msg == E_KEYBOARD) {
		int tmp;
		va_list args;

		va_copy(args, msg->data);
		tmp = va_arg(args, int);
		c = tmp & 0xff;
		d = tmp >> 8;
		va_end(args);

		if (c == 0) {
			switch(d) {
			case 'H':
				if (vyb_volba == 0) {
					his_line -= (his_line > 0);
				} else {
					vyb_volba--;
				}
				break;

			case 'P':
				if (his_line < last_his_line) {
					his_line++;
				} else if (vyb_volba + 1 < pocet_voleb) {
					vyb_volba++;
				}
				break;
			}

			schovej_mysku();
			redraw_text();
			ukaz_mysku();
			showview(TEXT_X, TEXT_Y, TEXT_XS, TEXT_YS);
		} else if (c == 13 || c == 32) {
			dialog_cont();
			msg->msg = -1;
		} else if (c == 27) {
			konec(0,0,0,0,0);
		}
	}
}

void wire_dialog();
void wire_dialog_drw()
  {
  schovej_mysku();
  wire_dialog();
  draw_all();
  ukaz_mysku();
  showview(0,0,0,0);
  }
void unwire_dialog()
  {
  send_message(E_DONE,E_KEYBOARD,key_check);
  disable_click_map();
//  wire_proc=wire_dialog_drw;
  }

void wire_dialog()
  {
  cancel_render=1;
  send_message(E_ADD,E_KEYBOARD,key_check);
  change_click_map(clk_dialog,CLK_DIALOG);
  unwire_proc=unwire_dialog;
  showview(0,0,0,0);
  last_his_line=his_line;
  }


short *q_item_one(int i,int itnum)
     {
     int j;
     THUMAN *p=&postavy[i];
     for(j=0;j<p->inv_size;j++)
        if (p->inv[j]==itnum) return &p->inv[j];
     for(j=0;j<HUMAN_PLACES;j++)
        if (p->wearing[j]==itnum) return &p->wearing[j];
     for(j=0;j<HUMAN_RINGS;j++)
        if (p->prsteny[j]==itnum) return &p->wearing[j];
     return NULL;
     }

short *q_item(int itnum,int sector)
  {
  int i;
  short *p;

  itnum++;
  for(i=0;i<POCET_POSTAV;i++)
     if (postavy[i].sektor==sector)
       if ((p=q_item_one(i,itnum))!=NULL)return p;
  return NULL;
  }

void destroy_item(int itnum)
  {
  short *q=q_item(itnum,viewsector);
  if (q!=NULL) *q=0;
  }

void create_item(int itnum)
  {
  //short *p;

  poloz_vsechny_predmety();
  picked_item=(short *)getmem(4);
  picked_item[0]=itnum+1;
  picked_item[1]=0;
  pick_set_cursor();
  }

static void add_case(int num, char *text) {
	char *a;
	int xs, ys;
	const Font *font;

	vol_n[pocet_voleb] = num;

	if (pocet_voleb > MAX_VOLEB) {
		error("POZOR! Je priliz mnoho voleb");
		pocet_voleb = MAX_VOLEB;
	}

	a = (char*)alloca(strlen(text) + 2);
	font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	renderer->setFont(font, 1, 0, 247, 0);
	zalamovani(text, a, TEXT_XS, &xs, &ys);

	while (*a) {
		char z[100];
		z[0] = pocet_voleb + 48;
		z[1] = 0;
		strcat(z, a);
		history.insert(z);
		a = strchr(a, 0) + 1;
	}

	pocet_voleb++;
}

static void remove_all_cases() {
	int cf, i;

	pocet_voleb = 0;
	cf = history.size();

	for (i = end_text_line; i < cf; i++) {
		if (history[i]!=NULL) {
			if (history[i][0]-48 != vyb_volba) {
				history.remove(i);
			} else {
				char *ptr = new char[strlen(history[i]) + 1];
				strcpy(ptr, history[i]);
				ptr[0] = 'M';
				history.replace(i, ptr);
				delete[] ptr;
			}
		} else {
			break;
		}
	}

	history.pack();
	vyb_volba=0;
}

static char case_click(int id,int xa,int ya,int xr,int yr)
  {
  int cf;

  xa,ya,xr,yr;

  if (pocet_voleb>1)
     {
     id=yr/TEXT_STEP;
     cf = history.size();
     id+=his_line;
     if (id>=cf) return 0;
     if (history[id]==NULL) return 0;
     id=history[id][0];
     if (id=='E' || id=='M') return 0;
     id-=48;
  if (id!=vyb_volba)
     {
     vyb_volba=id;
     schovej_mysku();
     redraw_text();
     ukaz_mysku();
     showview(TEXT_X,TEXT_Y,TEXT_XS,TEXT_YS);
     }
  if (ms_last_event.event_type & 0x2)
     {
     dialog_cont();
     }
     }
  else
  if (ms_last_event.event_type & 0x2)
     {
     dialog_cont();
     }
  return 1;
  }

void dialog_select(char halt)
  {
  showed=0;
  unwire_proc();
  draw_all();
  ukaz_mysku();
  wire_dialog();
  halt_flag=halt && (pocet_voleb>0);
  }

void dialog_select_jump()
  {
  goto_paragraph(save_jump);
  }

static void exit_dialog()
  {
  _flag_map[0]|=0x1;
  stop_anim();
  if (dialog_mob>-1)
     {
     mobs[dialog_mob].dialog_flags=_flag_map[0];
     mobs[dialog_mob].stay_strategy=_flag_map[1];
     }
  unwire_proc();
  aunlock(H_DIALOGY_DAT);
  ukaz_mysku();
  free(descript);descript=NULL;
  free(string_buffer);string_buffer=NULL;
  remove_all_cases();
  history.clear();
	delete back_pic;
  undef_handle(H_DIALOG_PIC);
  if (starting_shop!=-1 && !battle)
     {
     enter_shop(starting_shop);
     ukaz_mysku();
     update_mysky();
     cancel_pass=0;
     }
  else
     {
     wire_proc();
     norefresh=0;
     }
  starting_shop=-1;
  SEND_LOG("(DIALOGS) Exiting dialog...",0,0);  
  }


static void picture(char *c)
  {
  undef_handle(H_DIALOG_PIC);
  if (strcmp(c,"SCREEN")) def_handle(H_DIALOG_PIC,c,hi_8bit_correct,SR_DIALOGS),back_pic_enable=0;
  else back_pic_enable=1;
  }



static void dlg_start_battle()
  {
  if (dialog_mob!=-1)
     {
     mobs[dialog_mob].vlajky|=MOB_IN_BATTLE;
     }
  battle=1;
  }

static void teleport_group(short sector,short dir)
  {
  int i;
  THUMAN *h=postavy;

  destroy_player_map();
  for(h=postavy,i=0;i<POCET_POSTAV;i++,h++)
     if (h->used && h->groupnum==cur_group)
        {
        recheck_button(h->sektor,1);
        recheck_button(sector,1);
        h->sektor=sector;
        h->direction=dir;
        }
  viewsector=sector;
  viewdir=dir;
  build_player_map();
  }

extern THUMAN postavy_2[];

char join_character(int i)
  {
  THUMAN *h;
  THUMAN *s=postavy_2+i;
  int j;

  SEND_LOG("(DIALOGS) Joining character '%s'",s->jmeno,0);
  for(j=0,h=postavy;j<POCET_POSTAV;j++,h++) if (!h->used)
     {
     memcpy(h,s,sizeof(THUMAN));
     h->sektor=viewsector;
     h->direction=viewdir;
     h->groupnum=cur_group;
     reg_grafiku_postav();
     bott_draw(1);
     return 0;
     }
  SEND_LOG("(DIALOGS) Join failed - no room for new character",0,0);
  return 1;
  }


static int selected_player;

char drop_character()
{
  int selected_player=sn_nums[0];
  if (selected_player<0 || selected_player>=POCET_POSTAV) return 1;
  memcpy(postavy+selected_player,postavy+selected_player+1,sizeof(*postavy)*(POCET_POSTAV-selected_player));
  postavy[POCET_POSTAV-1].used=0;
  reg_grafiku_postav();
  bott_draw(1);
  return 0;
}


static char dead_players=0;

static char ask_who_proc(int id,int xa,int ya,int xr,int yr) {
	THUMAN *p;
	int i;
	const Texture *tex;

	if (id == 2) {
		selected_player = -1;
		exit_wait = 1;
		return 1;
	}

	tex = dynamic_cast<const Texture*>(ablock(H_OKNO));
	i = xr / tex->width();

	if (i < POCET_POSTAV) {
		i = group_sort[i];
		p = &postavy[i];

		if (p->used && ((p->lives != 0) ^ (dead_players))) {
			if (p->sektor == viewsector) {
				selected_player = i;
				exit_wait = 1;
			}
		}
	}

	return 1;
}


static int dlg_ask_who()
  {
  draw_all();
  mouse_set_default(H_MS_WHO);
  ukaz_mysku();
  showview(0,0,0,0);
  *otevri_zavoru=1;
  change_click_map(clk_dlg_who,CLK_DLG_WHO);
  escape();
  dead_players=0;
  schovej_mysku();
  mouse_set_default(H_MS_DEFAULT);
  if (selected_player==-1) return 1;
  strcpy(sn_nams[0],postavy[selected_player].jmeno);
  sn_rods[0]=postavy[selected_player].female;
  sn_nums[0]=selected_player;
  change_click_map(NULL,0);
  return 0;
  }

extern uint16_t weapon_skill[];

static void pract(int h, int vls, int how, int max)
  {
   iff=0;
   if (vls>=100)
     {
     vls-=100;
     if (postavy[h].bonus_zbrani[vls]>=max) iff=1;
     else
        {
        postavy[h].bonus_zbrani[vls]+=how;
        if (postavy[h].bonus_zbrani[vls]>max) postavy[h].bonus_zbrani[vls]=max,iff=1;
        postavy[h].weapon_expy[vls]=weapon_skill[postavy[h].bonus_zbrani[vls]];
        }
     }
   else
     {
     if (postavy[h].vlastnosti[vls]>=max) iff=1;
     else
        {
        postavy[h].stare_vls[vls]+=how;
        prepocitat_postavu(postavy+h);
        if (postavy[h].vlastnosti[vls]>max)
           {
           postavy[h].stare_vls[vls]-=postavy[h].vlastnosti[vls]-max;
           postavy[h].vlastnosti[vls]=max;
           iff=1;
           }
        }
     }
  }

static void pract_to(int h, int vls, int how)
  {
   iff=0;
   if (vls>=100)
     {
     vls-=100;
     if (postavy[h].bonus_zbrani[vls]<how)
        {
        postavy[h].bonus_zbrani[vls]=how;
        postavy[h].weapon_expy[vls]=weapon_skill[how];
        }
     else iff=1;
     }
   else
     {
     if (postavy[h].vlastnosti[vls]<how)
        postavy[h].stare_vls[vls]+=how-postavy[h].vlastnosti[vls];else iff=1;
     prepocitat_postavu(postavy+h);
     }
  }


static char oper_balance(int val1,int val2,int oper)
  {
  switch (oper)
     {
     case OPER_EQ:return val1==val2;
     case OPER_BIG:return val1>val2;
     case OPER_LOW:return val1<val2;
     case OPER_BIGEQ:return val1>=val2;
     case OPER_LOWEQ:return val1<=val2;
     case OPER_NOEQ:return val1!=val2;
     default:error("Chybn˜ operator porovn v n¡");
     }
  return 0;
  }

static char test_vls(int h, int vls, int oper, int num)
  {
  int val;
   if (vls>=100)
     {
     vls-=100;
     val=postavy[h].bonus_zbrani[vls];
     }
  else
     val=postavy[h].stare_vls[vls];
  return oper_balance(val,num,oper);
  }

static char atsector(int oper,int sector)
  {
  return oper_balance(viewsector,sector,oper);
  }


static void dark_screen(int time, int gtime) {
	int z, i;
	THUMAN *h;

	i = Timer_GetValue() + time * 50;
	memset(curcolor, 0, 3 * sizeof(uint8_t));
	bar(0, 17, 639, 377);
	showview(0, 0, 0, 0);

	while (Timer_GetValue() < i) {
		do_events();
	}

	game_time += gtime * HODINA;

	for (i = 0, h = postavy; i < POCET_POSTAV; i++, h++) {
		if (h->used && h->lives) {
			z = h->vlastnosti[VLS_HPREG] * gtime;
			z += h->lives;

			if (z > h->vlastnosti[VLS_MAXHIT]) {
				z = h->vlastnosti[VLS_MAXHIT];
			}

			h->lives = z;
			z = h->vlastnosti[VLS_MPREG] * gtime;
			z += h->mana;

			if (z > h->vlastnosti[VLS_MAXMANA]) {
				z = h->vlastnosti[VLS_MAXMANA];
			}

			h->mana = z;
			z = h->vlastnosti[VLS_VPREG] * gtime;
			z += h->kondice;

			if (z > h->vlastnosti[VLS_KONDIC]) {
				z = h->vlastnosti[VLS_KONDIC];
			}

			h->kondice=z;
		}
	}

	bott_draw(0);
}

static char najist_postavy(int cena)
  {
  int i,s=0;
  THUMAN *h=postavy;

  for(i=0;i<POCET_POSTAV;i++,h++) if (h->used && h->sektor==viewsector && h->lives) s=s+cena;
  if (s>money) return 1;
  money-=s;
  for(i=0,h=postavy;i<POCET_POSTAV;i++,h++) if (h->used && h->sektor==viewsector && h->lives)
     {
     h->jidlo=MAX_HLAD(h);
     h->voda=MAX_ZIZEN(h);
     }
  return 0;
  }

static char isall()
  {
  THUMAN *h=postavy;
  int i;

  for(i=0,h=postavy;i<POCET_POSTAV;i++,h++) if (h->sektor!=viewsector && h->used && h->lives) return 0;
  return 1;
  }

static void spat(int hodin)
  {
  sleep_ticks=hodin*HODINA;
//  Task_Add(16384,sleep_players);
  sleep_players();
  insleep=1;
  while (insleep) do_events();
  }

static char test_volby_select(int balance,int value)
  {
  return oper_balance(pocet_voleb,value,balance);
  }

static void cast_spell(int spell)
  {
  int cil=1+sn_nums[0];

  add_spell(spell,cil,cil,1);
  }


void do_dialog() {
	int i, p1, p2, p3;
	char *c;

	do {
		i = Get_short();
		p3 = 0;

		switch(i) {
		case 128:
			add_desc(Get_string());
			break;

		case 129:
			show_emote(Get_string());
			break;

		case 130:
			save_name(Get_short());
			break;

		case 131:
			iff = !iff;
			// FIXME: break here?

		case 132:
			load_name(Get_short());
			break;

		case 133:
			nahodne(0, 0, Get_short());
			break;

		case 134:
			p1 = Get_short();
			p2 = Get_short();
			nahodne(VLS_SMAGIE, p1, p2);
			break;

		case 135:
			p1 = Get_short();
			p2 = Get_short();
			nahodne(VLS_SILA, p1, p2);
			break;

		case 136:
			p1 = Get_short();
			p2 = Get_short();
			nahodne(VLS_OBRAT, p1, p2);
			break;

		case 137:
			c = Get_string();
			p1 = Get_short();
			strncpy(sn_nams[0], c, 32);
			sn_rods[0] = p1;
			break;

		case 138:
			iff = Get_short();
			break;

		case 139:
			goto_paragraph(Get_short());
			break;

		case 140:
			p1 = Get_short();

			if (iff) {
				goto_paragraph(p1);
			}
			break;

		case 141:
			p1 = Get_short();

			if (!iff) {
				goto_paragraph(p1);
			}
			break;

		case 142:
			p1 = Get_short();
			add_case(p1, Get_string());
			break;

		case 143:
			p1 = Get_short();
			p2 = Get_short();
			c = Get_string();

			if (iff == p1) {
				add_case(p2, c);
			}
			break;

		case 144:
			dialog_select(1);
			return;

		case 145:
			iff = visited(Get_short());
			break;

		case 146:
			p1 = Get_short();
			c = Get_string();
			iff = visited(p1);

			if (iff == 0) {
				add_case(p1, c);
			}
			break;

		case 147:
			picture(Get_string());
			break;

		case 148:
			echo(Get_string());
			break;

		case 149:
			cur_page = count_pages();
			cur_page &= ~0x1;
			cur_page++;
			add_to_book(Get_short());
			play_fx_at(FX_BOOK);

			if (game_extras & EX_AUTOOPENBOOK) {
				autoopenaction = 1;
			}
			break;

		case 150:
			set_nvisited(Get_short());
			break;

		case 151:
			iff = rnd(100) <= Get_short();
			break;

		case 152:
			iff = q_item(Get_short(), viewsector) != NULL;
			break;

		case 153:
			create_item(Get_short());
			break;

		case 154:
			destroy_item(Get_short());
			break;

		case 155:
			money += Get_short();
			break;

		case 156:
			p1 = Get_short();

			if (p1 > money) {
				iff = 1;
			} else {
				money -= p1;
			}
			break;

		case 157:
			dlg_start_battle();
			break;

		case 158:
			p1 = Get_short();
			p2 = Get_short();
			delay_action(0, p1, p2, 0, 0, 0);
			break;

		case 160:
			p1 = Get_short();
			p2 = Get_short();
			teleport_group(p1, p2);
			break;

		case 161:
			c = Get_string();
			p1 = Get_short();
			p2 = Get_short();
			run_anim(c, p1, p2);
			break;

		case 162:
			lecho(Get_string());
			break;

		case 163:
			q_flag(Get_short());
			break;

		case 164:
			dialog_select(0);
			return;

		case 165:
			dialog_select_jump();
			break;

		case 166:
			first_visited(Get_short());
			break;

		case 167:
			local_pgf = Get_short();
			break;

		case 168:
			starting_shop = Get_short();
			break;

		case 169:
			p1 = Get_short();

			if (!iff) {
				pstream->seek(p1, SEEK_CUR);
			}
			break;

		case 170:
			p1 = Get_short();

			if (iff) {
				pstream->seek(p1, SEEK_CUR);
			}
			break;

		case 171:
			p1 = Get_short();
			pstream->seek(p1, SEEK_CUR);
			break;

		case 172:
			code_page = Get_short();
			break;

		case 173:
			break; //ALT_SENTENCE

		case 174:
			iff = join_character(Get_short());
			break;

		case 189:
			dead_players = 1;

		case 175:
			echo(Get_string());
			p1 = Get_short();

			if (dlg_ask_who()) {
				if (p1) {
					goto_paragraph(p1);
				} else {
					iff = 1;
				}
			} else {
				iff = 0;
			}
			break;

		case 176:
			p1 = Get_short();
			p2 = Get_short();
			pract_to(sn_nums[0], p1, p2);
			break;

		case 177:
			p1 = Get_short();
			p2 = Get_short();
			p3 = Get_short();
			iff = test_vls(sn_nums[0], p1, p2, p3);
			break;

		case 178:
			p1 = Get_short();
			runes[p1 / 10] |= 1 << (p1 % 10);
			break;

		case 179:
			p1 = Get_short();
			iff = ((runes[p1 / 10] & (1 << (p1 % 10))) != 0);
			break;

		case 180:
			p1 = Get_short();
			iff = (money >= p1);
			break;

		case 181:
			p1 = Get_short();
			p2 = Get_short();
			p3 = Get_short();
			pract(sn_nums[0], p1, p2, p3);
			break;

		case 182:
			p1 = Get_short();
			p2 = Get_short();
			dark_screen(p1, p2);
			break;

		case 183:
			spat(Get_short());
			break;

		case 184:
			p1 = Get_short();
			iff = najist_postavy(p1);
			break;

		case 185:
			iff = isall();
			break;

		case 186:
			enable_glmap = Get_short();
			break;

		case 187:
			p1 = Get_short();
			p2 = Get_short();
			iff = atsector(p1, p2);
			break;

		case 188:
			p1 = Get_short();
			cast_spell(p1);
			break;

		case 190:
			spell_sound(Get_string());
			break;

		case 191:
			p1 = Get_short();
			p2 = Get_short();
			iff = test_volby_select(p1, p2);
			break;

		case 192:
			p1 = Get_short();
			p2 = Get_short();
			varibles[p1] = p2;
			break;

		case 193:
			p1 = Get_short();
			p2 = Get_short();
			varibles[p1] += p2;
			break;

		case 194:
			p1 = Get_short();
			p2 = Get_short();
			p3 = Get_short();
			iff = oper_balance(varibles[p1], p3, p2);
			break;

		case 195:
			p2 = find_pgnum(pstream->pos());
			p1 = Get_short();
			varibles[p1] = p2;
			break;

		case 196:
			p1 = Get_short();
			varibles[p1] = iff;
			break;

		case 197:
			p1 = Get_short();
			add_case(varibles[p1], Get_string());
			break;

		case 198:
			p1 = Get_short();
			p2 = Get_short();
			c = Get_string();

			if (iff == p1) {
				add_case(varibles[p2], c);
			}
			break;

		case 199:
			goto_paragraph(varibles[Get_short()]);
			break;

		case 200:
			iff = drop_character();
			break;

		case 201:
			pc_xicht(Get_short());
			break;

		case 518:
			set_flag(Get_short());
			break;

		case 519:
			reset_flag(Get_short());
			break;

		case 255:
			exit_dialog();
			return;

		default: {
			char s[80];
			sprintf(s,"Nezn m  instrukce: %d",i);
			error(s);
			break;
		}
		}
	} while(1);
}

void call_dialog(int entr, int mob) {
	int i;
	void (*old_wire_proc)()=wire_proc;

	memset(curcolor, 0, 3 * sizeof(uint8_t));
	back_pic = new SubTexture(*renderer, 0, SCREEN_OFFLINE, 340, 200);
	bar(0, SCREEN_OFFLINE, 639, SCREEN_OFFLINE + 359);
	SEND_LOG("(DIALOGS) Starting dialog...", 0, 0);

	for (i = 0; i < POCET_POSTAV; i++) {
		if (isdemon(postavy + i)) {
			unaffect_demon(i);
		}
	}

	mute_all_tracks(0);
	dialog_mob = mob;

	if (mob > -1) {
		_flag_map[0] = mobs[mob].dialog_flags;
		_flag_map[1] = mobs[mob].stay_strategy;
	}

	local_pgf = 0;
	poloz_vsechny_predmety();
	cancel_render = 1;
	norefresh = 1;
	his_line = 0;
	memset(sn_nums, 0xff, sizeof(sn_nums));
	goto_paragraph(entr);
	schovej_mysku();
	alock(H_DIALOGY_DAT);
	aswap(H_DIALOGY_DAT);
	selected_player = -1;
	do_dialog();
}

char save_dialog_info(WriteStream &stream) {
	int pgf_pocet;
	int i;
	size_t siz;
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	SEND_LOG("(DIALOGS)(SAVELOAD) Saving dialogs info...", 0, 0);
	pgf_pocet = list->size();
	stream.writeSint32LE(pgf_pocet);
	siz = (pgf_pocet + 3) / 4;


	if (siz) {
		unsigned char c = 0;

		for (i = 0; i < pgf_pocet; i++) {
			int j = (i & 3) << 1;
			c |= ((*list)[i].visited << j) | ((*list)[i].first << (j + 1));

			if ((i & 0x3) == 0x3) {
				stream.writeUint8(c);
				c = 0;
			}
		}

		// finish partial byte
		if (i & 0x3) {
			stream.writeUint8(c);
		}
	}

	for (i = 0; i < 32; i++) {
		stream.writeSint8(_flag_map[i]);
	}

	SEND_LOG("(DIALOGS)(SAVELOAD) Done...", 0, 0);
	return 0;
}

char load_dialog_info(SeekableReadStream &stream) {
	int pgf_pocet;
	int i;
	size_t siz;
	DialogList *list = dynamic_cast<DialogList*>(ablock(H_DIALOGY_DAT));

	SEND_LOG("(DIALOGS)(SAVELOAD) Loading dialogs info...",0,0);
	aswap(H_DIALOGY_DAT);
	pgf_pocet = stream.readSint32LE();
	siz = (pgf_pocet + 3) / 4;

	if (pgf_pocet != list->size()) {
		SEND_LOG("(ERROR) Dialogs has different sizes %d!=%d (can be skipped)", pgf_pocet, list->size());
		stream.seek(siz + 32, SEEK_CUR);
		return 0;
	}

	if (siz) {
		for (i = 0; i < pgf_pocet; i++) {
			int j = (i & 3) << 1;
			unsigned char c;
			
			if ((i & 0x3) == 0) {
				c = stream.readUint8();
			}

			list->setVisited(i, c >> j);
			list->setFirst(i, c >> (j + 1));
		}
	}

	for (i = 0; i < 32; i++) {
		_flag_map[i] = stream.readSint8();
	}

	SEND_LOG("(DIALOGS)(SAVELOAD) Done...", 0, 0);
	return stream.eos();
}
