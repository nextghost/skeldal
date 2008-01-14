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
TSTR_LIST read_config(const char *filename);
void add_field_txt(TSTR_LIST *ls,const char *name,const char *text);
void add_field_num(TSTR_LIST *ls,const char *name,long number);
int save_config(TSTR_LIST ls,const char *filename);
const char *get_text_field(TSTR_LIST ls,const char *name);
int get_num_field(TSTR_LIST ls,const char *name,int *num);
void process_ini(TSTR_LIST ls,void (*process)(const char *line));
char comcmp(const char *text,const char *command);
TSTR_LIST merge_configs(TSTR_LIST target, TSTR_LIST source);
int find_ini_field(TSTR_LIST ls,const char *name);
