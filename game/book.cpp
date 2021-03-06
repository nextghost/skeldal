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
//#include <skeldal_win.h>
/*

 Popis jazyka pro psani textu do knihy


 [cislo]

 .... text
 ....

 [/cislo]


 Tagy

 <P> paragraph
 <BR> break line
 <IMG SRC="filename" ALIGN=/left,right,center/>
 <HR> horizontal rule
 <EP> page end



 -----------------------------

 Vnitrni zapis

 Escape sekvence

 ESC s <vynechat>  - mezera n bodu
 ESC p <filename> <x> <y> obrazek na souradnicich x a y
 ESC e - konec stranky (jako prvni v textu)
 ESC h - horizontalni rule (jako prvni v textu)
 ESC l <skip> - konec radky (skip je pocet vynechanych bodu)

 Zapis cisla - to je hexa cislo od 1-255 pokud se nevejde do rozsahu je po 256
 pridana dalsi hodnota, tj 255 se zapise jako 255,1,
                           350 se zapise jako 255,96,

*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "libs/strlite.h"
#include "libs/bgraph.h"
#include "libs/memman.h"
#include "libs/event.h"
#include "game/globals.h"
#include "libs/system.h"

#define XMAX 254
#define YMAX 390
#define XLEFT 34
#define YLEFT 50
#define XRIGHT 354

#define PARAGRAPH "P"
#define BREAKLINE "BR"
#define IMAGE "IMG"
#define HOR_RULE "HR"
#define CENTER1 "CENTER"
#define CENTER2 "/CENTER"
#define DISTEND1 "DISTEND"
#define DISTEND2 "/DISTEND"
#define ALIGN "ALIGN"
#define PIC_LINE "LINE"
#define PIC_LSIZ "LSIZE"
#define SRC "SRC"

#define ALEFT "LEFT"
#define ARIGHT "RIGHT"
#define ACENTER "CENTER"

#define ANUM_LEFT 1
#define ANUM_RIGHT 2
#define ANUM_CENTER 0

#define END_PAGE 'e'
#define SPACE 's'
#define PICTURE 'p'
#define HRUL  'h'
#define END_LINE 'l'

#define BOOK_FILE "_BOOK.TMP"

static int center=0;
static int distend=0;
static StringList all_text;
static char read_buff[256];
static char write_buff[256];
static int buff_pos=0;
static int buff_end=0;
static int total_width=XMAX;
static int left_skip=0;
static int linepos=0,last_skip=1;
static int picture_len=0;
static char winconv=0;
static int relpos=0;

static char xlat_table[128]="?__??_____?_____?__??__________________________________?___?__??__??_?__??____???__?___?___?__??__??_?__??____???__?";

static int insert_num(char *text,int pos,int num)
  {
  char c=0x80;
  do
     {
     c=num & 0x3f;num>>=6;
     if (num) c|=0x80;
     c|=0x40;
     text[pos++]=c;
     }
  while (num);
  return pos;
  }


static int read_num(const char *text,int *pos)
  {
  int num=0,shift=0;
  char c;

  do
     {
     c=text[pos[0]++];
     num|=(c & 0x3f)<<shift;
     shift+=6;
     }
  while (c & 0x80);
  return num;
  }

static void next_line(int step)
  {
  linepos+=step;
   if (linepos>YMAX)
     {
     char s[3];
     s[0]=27;
     s[1]=END_PAGE;
     s[2]=0;
     all_text.insert(s);
     linepos=0;
     picture_len=-1;
     }
  last_skip=step;
  }


static int insert_end_line_and_save(int p,int ys)
  {
  int size;
  while (read_buff[buff_pos]==' ' && buff_pos<buff_end) buff_pos++;
  size=buff_end-buff_pos;
  if (size)memcpy(read_buff,read_buff+buff_pos,size*sizeof(char));
  write_buff[p++]=27;
  write_buff[p++]=END_LINE;
  p=insert_num(write_buff,p,ys);
  write_buff[p++]=0;
  all_text.insert(write_buff);
  buff_pos=size;
  buff_end=buff_pos;
  if (picture_len>0) picture_len-=ys;
  if (picture_len<=0 && total_width!=XMAX)
     {
     picture_len=0;
     total_width=XMAX;
     left_skip=0;
     }
  return p;
  }

static int insert_left_skip(int p,int skip)
  {
  skip+=left_skip;
  if (skip)
     {
     write_buff[p++]=27;
     write_buff[p++]=SPACE;
     p=insert_num(write_buff,p,skip);
     }
  return p;
  }

static void save_line_oboustrane() {
	char space[] = " ";
	int xs, ys, ss, mez, mm = 0, mc = 0;
	int i, p;

	for (i = 0, xs = 0, mez = 0, ys = 0; i < buff_pos; i++) {
		space[0] = read_buff[i];

		if (space[0] == 32) {
			mez++;
		} else {
			xs += renderer->textWidth(space);
			p = renderer->textHeight(space);

			if (p > ys) {
				ys = p;
			}
		}
	}

	ss = total_width - xs;
	p = 0;

	if (!ys) {
		ys = last_skip;
	}

	next_line(ys);
	p = insert_left_skip(p, 0);

	for (i = 0, mc = 0; i < buff_pos; i++) {
		if (read_buff[i] == 32) {
			int sp;

			write_buff[p++] = 27;
			write_buff[p++] = SPACE;
			mc++;
			sp = ss * (mc) / mez;
			p = insert_num(write_buff, p, sp - mm);
			mm = sp;
		} else {
			write_buff[p++] = read_buff[i];
		}
	}

	insert_end_line_and_save(p, ys);
}

static void save_line_center() {
	char space[] = " ";
	int xs, ys, ss;
	int i, p;

	for (i = 0, xs = 0, ys = 0; i < buff_pos; i++) {
		space[0] = read_buff[i];
		xs += renderer->textWidth(space);
		p = renderer->textHeight(space);

		if (p > ys) {
			ys = p;
		}
	}

	p = 0;

	if (!ys) {
		ys = last_skip;
	}

	next_line(ys);
	ss = total_width - xs;
	p = insert_left_skip(p, ss / 2);
	memcpy(write_buff + p, read_buff, buff_pos * sizeof(char));
	p += buff_pos;
	write_buff[p] = 0;
	insert_end_line_and_save(p, ys);
}

static void save_line_left() {
	int p, z;
	int ys;

	p = 0;
	p = insert_left_skip(p, 0);
	z = p;

	if (buff_pos) {
		memcpy(write_buff + p, read_buff, buff_pos * sizeof(char));
	}

	p += buff_pos;
	write_buff[p] = 0;
	ys = renderer->textHeight(write_buff + z);

	if (!ys) {
		ys = last_skip;
	}

	next_line(ys);
	insert_end_line_and_save(p, ys);
}


static void save_buffer()
  {
  while (buff_end>buff_pos && read_buff[buff_end]==32) buff_end--;
  if (center) save_line_center();
     else if (buff_pos==buff_end || !distend) save_line_left(); else save_line_oboustrane();
  }

static void break_line()
  {
  buff_pos=buff_end;
  save_buffer();
  }

static char read_set(SeekableReadStream *txt, char *var, char *set) {
// TODO: this REALLY needs rewrite
	int c, i;
	char *cc;

	i = -1;
	do {
		var[++i] = (char)txt->readUint8();
	} while (var[i] != '=');
	var[i] = '\0';

	do {
		c = txt->readUint8();
	} while ((unsigned)c <= ' ');

	if (c == '"') {
		i = -1;
		do {
			set[++i] = (char)txt->readUint8();
		} while (set[i] != '"');
		c = (char)txt->readUint8();
		set[i] = '\0';
	} else if (c == '\'') {
		i = -1;
		do {
			set[++i] = (char)txt->readUint8();
		} while (set[i] != '\'');
		set[i] = '\0';
		c = (char)txt->readUint8();
	} else {
		for (i = 0, set[0] = c; set[i] != '>' && set[i] != ' ';) {
			set[++i] = (char)txt->readUint8();
		}
		c = set[i];
		set[i] = '\0';
	}

	while ((unsigned)c <= ' ' && !txt->eos()) {
		c = (char)txt->readUint8();
	}

	if (c != '>') {
		txt->seek(-1, SEEK_CUR);
	}

	cc = strchr(var, 0);

	while (cc != var) {
		cc--;

		if (*cc > 32) {
			cc++;
			break;
		}
	}

	*cc = 0;
	strupr(set);
	strupr(var);

	return c;
}

static int get_data_handle(const char *filename, DataBlock *(*dec)(SeekableReadStream&)) {
	int i;

	i = find_handle(filename, dec);

	if (i == -1) {
		i = end_ptr++;
		def_handle(i, filename, dec, SR_DIALOGS);
	}

	return i;
}

static void insert_picture(const char *filename, int align, int line, int lsize) {
	int x, y;
	char *c = write_buff;
	const Texture *tex = dynamic_cast<const Texture*>(ablock(get_data_handle(filename, pcx_8bit_decomp)));


	switch (align) {
	case ANUM_CENTER:
		x = (XMAX - tex->width()) / 2;
		y = linepos;
		linepos += tex->height();
		*c++ = 27;
		*c++ = END_LINE;
		c += insert_num(c, 0, tex->height() + last_skip);
		break;

	case ANUM_LEFT:
		x = 0;
		y = linepos;
		left_skip = tex->width() + 5;
		total_width = XMAX - left_skip;
		break;

	case ANUM_RIGHT:
		total_width = (x = XMAX - tex->width()) - 5;
		left_skip = 0;
		y = linepos;
		break;
	}

	if (!lsize) {
		lsize = tex->height() - line;
	}

	picture_len = lsize;
	*c++ = 27;
	*c++ = PICTURE;

	while (*filename) {
		*c++ = *filename++;
	}

	*c++ = ':';
	c += insert_num(c, 0, x);
	c += insert_num(c, 0, y);
	c += insert_num(c, 0, line);
	c += insert_num(c, 0, lsize);
	*c++ = 0;
	all_text.insert(write_buff);
}

static char read_tag(SeekableReadStream *txt) {
	char c, var[256], set[256];
	int i = -1;

	do {
		var[++i] = (char)txt->readUint8();
	} while (var[i] != '>' && var[i] != ' ');
	c = var[i];
	var[i] = '\0';

	while((unsigned)c <= ' ' && !txt->eos()) {
		c = i = (char)txt->readUint8();
	}

	if (c != '>') {
		txt->seek(-1, SEEK_CUR);
	}

	strupr(var);

	if (!strcmp(var, PARAGRAPH)) {
		break_line();
		break_line();
		return 1;
	}

	if (!strcmp(var, BREAKLINE)) {
		break_line();
		return 1;
	}

	if (!strcmp(var, IMAGE)) {
		char pic_name[50] = " ";
		char alig = 0;
		int line = 0, lsize = 0;

		while (c != '>') {
			c = read_set(txt, var, set);
			if (!strcmp(var, SRC)) {
				strncpy(pic_name, set, 49);
			} else if (!strcmp(var,ALIGN)) {
				if (!strcmp(set, ALEFT)) {
					alig = 1;
				} else if (!strcmp(set, ARIGHT)) {
					alig = 2;
				} else if (!strcmp(set, ACENTER)) {
					alig = 0;
				}
			} else if (!strcmp(var, PIC_LINE)) {
				sscanf(set, "%d", &line);
			} else if (!strcmp(var, PIC_LSIZ)) {
				sscanf(set, "%d", &lsize);
			}
		}
		if (pic_name[0] != 0) {
			insert_picture(pic_name, alig, line, lsize);
		}

		return 0;
	}

	if (!strcmp(var, CENTER1)) {
		center++;
	} else if (!strcmp(var, CENTER2)) {
		if (center > 0) {
			center--;
		}
	} else if (!strcmp(var, DISTEND1)) {
		distend++;
	} else if (!strcmp(var, DISTEND2)) {
		if (distend > 0) {
			distend--;
		}
	}

	return 0;
}


static char skip_section(SeekableReadStream *txt) {
	int c;
	char end = 1;

	c = txt->readUint8();

	while (c != ']' && !txt->eos()) {
		c = txt->readUint8();
		end = 0;
	}

	if (txt->eos()) {
		end = 1;
	}

	return end;
}

void prekodovat(char *c) {
	unsigned char *ptr = (unsigned char*)c;
	while (*ptr) {
		if (*ptr > 137) {
			*ptr = xlat_table[*ptr - 138];
		}

		ptr++;
	}
}

static void read_text(SeekableReadStream *txt) {
	int i;
	int xs = 0;
	char ss[2] = " ";
	char wsp = 1;

	buff_pos = 0;
	buff_end = 0;

	do {
		i = txt->readUint8();

		if (txt->eos()) {
			break;
		}

		if (i < 32) {
			i=32;
		}

		if (i == '<') {
			if (read_tag(txt)) {
				xs=0;
				wsp=1;
			}
			continue;
		}

		if (i == '[') {
			if (skip_section(txt)) {
				break;
			}

			continue;
		}

		if (i == 32) {
			if (wsp) {
				continue;
			}

			buff_pos = buff_end;
			wsp = 1;
		} else {
			wsp = 0;
		}

		if (i == '&') {
			i = txt->readUint8();
		}

		if (winconv && i > 137) {
			i = xlat_table[i-138];
		}

		ss[0] = i;
		xs += renderer->textWidth(ss);
		read_buff[buff_end++] = i;

		if (xs > total_width && !wsp) {
			save_buffer();
			read_buff[buff_end] = 0;
			xs = renderer->textWidth(read_buff);
		}
	} while (1);
}

void seek_section(SeekableReadStream *txt,int sect_number) {
	int i = EOF;
	char c = 0, buf[256];

	winconv = 0;

	do {
		while (c != '[' && !txt->eos()) {
			c = (char)txt->readUint8();
		}

		if (c == '[') {
			i = -1;
			do {
				buf[++i] = (char)txt->readUint8();
			} while (buf[i] >= '0' && buf[i] <= '9');
			buf[i+1] = '\0';
			i = -2;
			sscanf(buf, "%d%c", &i, &c);

			if (i == sect_number) {
				while(c != ']' && !txt->eos()) {
					if (c == 'W' || c == 'w') {
						winconv = 1;
					}

					if (c == 'K' || c == 'k') {
						winconv = 0;
					}

					c = (char)txt->readUint8();
				}
				return;
			}
		}
		c = 0;
	} while (i != EOF);

	closemode();
	sprintf(buf, "Nemohu najit odstavec s cislem %d.", sect_number);
	Sys_ErrorBox(buf);
	exit(1);
}

void add_text_to_book(const char *filename, int odst) {
	SeekableReadStream *txt;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FKNIHA));

	renderer->setFont(font, 0, 0, 0, 0);
	txt = enc_open(filename);

	if (txt == NULL) {
		return;
	}

	seek_section(txt, odst);
	read_text(txt);
	next_line(1000);
	delete txt;
}

static const char *displ_picture(const char *c) {
	char *d;
	int x, y, hn, z, ln, sl;
	const Texture *tex;

	d = write_buff;

	while (*c != ':') {
		*d++ = *c++;
	}

	*d++ = 0;
	c++;
	hn = get_data_handle(write_buff, pcx_8bit_decomp);
	x = read_num(c, (z = 0, &z));
	c += z;
	y = read_num(c, (z = 0, &z));
	c += z;
	ln = read_num(c, (z = 0, &z));
	c += z;
	sl = read_num(c, (z = 0, &z));
	c += z;
	tex = dynamic_cast<const Texture*>(ablock(hn));

	if (tex->height() + y > YMAX) {
		return c;
	}

	y += YLEFT;
	x += relpos;
	renderer->rectBlit(*tex, x, y, 0, ln, tex->width(), sl);
	return c;
}

void write_book(int page) {
	int i = 0, y = 0, z, zz, ps, pg, posx = 0, posy = 0;
	const char *c;
	char space[] = " ";
	const Font *font = dynamic_cast<const Font*>(ablock(H_FKNIHA));

	pg = page;
	renderer->setFont(font, 0, 0, 0, 0);
	relpos = XLEFT;
	zz = all_text.size();

	if (--page) {
		for (i = 0; i < zz && page; i++) {
			c = all_text[i];

			if (c != NULL && c[0] == 27 && c[1] == END_PAGE) {
				page--;
			}
		}
	}

	if (page) {
		return;
	}

	for (ps = 0; ps < 2; ps++) {
		posx = relpos;
		posy = YLEFT + y;

		do {
			if (i > zz) {
				break;
			}

			c = all_text[i];

			if (c == NULL) {
				break;
			}

			if (c[0] == 27 && c[1] == END_PAGE) {
				break;
			}

			while (*c) {
				z = 0;

				if (*c == 27) {
					c++;

					switch (*c++) {
					case SPACE:
						posx += read_num(c, &z);
						c += z;
						break;

					case END_LINE:
						y += read_num(c, &z);
						posx = relpos;
						posy = YLEFT + y;
						c += z;
						break;

					case PICTURE:
						c = displ_picture(c);
						break;
					}
				} else {
					space[0] = *c++;
					renderer->drawText(posx, posy, space);
					posx += renderer->textWidth(space);
				}
			}

			i++;
		} while (1);

		i++;
		y = 0;
		relpos = XRIGHT;

		if (ps == 0) {
			char s[20];
			sprintf(s, texty[135], pg);
			renderer->drawAlignedText(XLEFT, YLEFT + YMAX, HALIGN_LEFT, VALIGN_TOP, s);
		}

		if (ps == 1) {
			char s[20];
			sprintf(s, texty[136], pg + 1);
			renderer->drawAlignedText(XRIGHT + XMAX, YLEFT + YMAX, HALIGN_RIGHT, VALIGN_TOP, s);
		}
	}
}

int count_pages()
  {
  int i,cn,z;
  const char *c;

  z = all_text.size();
  for(i=0,cn=0;i<z;i++)
     {
     c=all_text[i];
     if (c!=NULL && c[0]==27 && c[1]==END_PAGE) cn++;
     }
  return cn;
  }

void save_book()
  {
  char *c;
  FILE *f;
  int i,ss;
  const char *tx;

  if (!all_text.count()) return;
/*
  concat(c,pathtable[SR_TEMP],BOOK_FILE);
  f=fopen(c,"w");if (f==NULL) return;
*/

	f = fopen(Sys_FullPath(SR_TEMP, BOOK_FILE), "w");

	if (!f) {
		return;
	}

  i=0;
  ss = all_text.size();
  while (i<ss && (tx=all_text[i++])!=NULL)
     {
     fputs(tx,f);
     fputc('\n',f);
     }
  fclose(f);
  }

void load_book()
  {
  char *c;
  FILE *f;
  char tx[512];

  all_text.clear();
	f = fopen(Sys_FullPath(SR_TEMP, BOOK_FILE), "r");

	if (!f) {
		return;
	}

  while (fgets(tx,510,f)!=NULL)
     {
     char *c;
     c=strchr(tx,0);c--;*c=0;
     all_text.insert(tx);
     }
  fclose(f);
  }


