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
extern char filename[MAX_PATH];

int load_map(char *filename);
int save_map(char *filename);
int save_all_map(void);

long save_section(FILE *f,void *section, int sct_type,long sect_size);
long load_section(FILE *f,void **section, int *sct_type,long *sect_size);
int save_scr_list(FILE *f,TSTR_LIST names,int scr_id);
char check_sound_map();
char *set_password(char *text);
char check_password(char *text);
void encrypt(char *password);

