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
 *  Last commit made by: $Id: EDIT_MAP.H 7 2008-01-14 20:14:25Z bredysoft $
 */
void edit_sector(int source);
void open_sector_win(void);
void info_sector(int sector);
void open_draw_win(void);
long get_draw_flags(void);
void edit_side(int source,int smer);
void sector_details_call(int sect_num);
void chozeni2(EVENT_MSG *msg,OBJREC *o);
void jdi_na_sektor(int s);


void string_list_sup();
void str_line(OBJREC *o);
char *strs(int num);
long vals(int num);
void test_int(void);
void fill_bit_fields(int win,int id_start,int source,int count);
long get_bit_fields(int win,int id_start,int count);
void value_store(OBJREC *o);


