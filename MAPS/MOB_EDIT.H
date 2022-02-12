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
#define MOB_DIR mob_dir
#define MOB_VER 0x100


extern char *mob_dir;
void enemy_window();
void init_mob_list();
extern int enemy_win;
extern short mob_map[MAPSIZE];
void select_enemy(int at_sector);
void save_mobs();
void load_mobs();
void *save_mob_map(long *size);
void load_mob_map(void *data,int size);
void pcx_view(OBJREC *o);
void load_mobs_to_map(void *p,long size);
void *save_mobs_to_map(long *size);
void load_sound_dat(void *p,long siz);
int save_sound_dat(FILE *f,int id);

