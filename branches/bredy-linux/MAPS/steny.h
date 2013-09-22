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
#define T_LIST_SIZE 20
typedef char t_side_names[256][T_LIST_SIZE];


extern TSTR_LIST side_flgs;
extern TSTR_LIST side_names;
extern TSTR_LIST floors;
extern TSTR_LIST ceils;
extern TSTR_LIST oblouky;
extern TSTR_LIST vzhled_veci;
extern TSTR_LIST pohledy_veci;
extern TSTR_LIST dlg_names;
extern TSTR_LIST weapons;
extern TSTR_LIST weapons_pos;
extern TSTR_LIST mob_procs;
extern TSTR_LIST wall_procs;
extern int *dlg_pgfs;

#define NSID "SID"
#define NFLR "FLR"
#define NCEI "CEI"
#define NOBL "OBL"
#define SCR ".SCR"

int string_list(char *c,int akt);
void read_side_list(char *filename,TSTR_LIST *outpt,int relative,int structlen);
void read_dlg_list(char *filename,TSTR_LIST *outpt,int **nums);
TSTR_LIST build_static_list(char *c);
int pgf2name(int num);
void read_side_script_one(const char *filename,const char *nick,TSTR_LIST *outpt,int relative,int structlen);
void read_full_side_script(const char *filename);
void read_spec_procs();
void load_side_script(const char *filename);
int change_side(int script,int side,int pos,int frame,char *nw);
void discharge_side_script();
void save_side_script(const char *filename);
int delete_side(int list,int pos);
int add_side(int list,char *name,int frames);
char *get_side_name(int list,int pos,int field);
