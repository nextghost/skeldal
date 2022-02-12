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
#define MEMTEXT "Pam�t: "

#define E_STATUS_LINE 60

extern const word *msg_box_font;
extern const word *msg_icn_font;

int msg_box(char *title, char icone, char *text, ... );


void highlight(CTL3D *c,word color);
CTL3D *def_border(int btype,int color);
void xor_rectangle(int x,int y,int xs,int ys);

// status lines
void status_line(EVENT_MSG *msg,T_EVENT_ROOT **user_data);
void *status_mem_info(EVENT_MSG *msg);
void *mouse_xy(EVENT_MSG *msg);
void *show_time(EVENT_MSG *msg);

// objects
//void sample(OBJREC *o);
void button(OBJREC *o);
void win_label(OBJREC *o);
void check_box(OBJREC *o);
void radio_butts(OBJREC *o);
void toggle_button(OBJREC *o);
void input_line(OBJREC *o);
void label(OBJREC *o);
void mid_label(OBJREC *o);
void scroll_bar_v(OBJREC *o);
void scroll_button(OBJREC *o);
void scroll_support();
void scroll_bar_h(OBJREC *o);
void button2(OBJREC *o);
void resizer(OBJREC *o);


