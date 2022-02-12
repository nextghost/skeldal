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
 *  Last commit made by: $Id: extras.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#define EX_RANDOM_BACKFIRES 1  //hotovo
#define EX_RESPAWN_MONSTERS 2  //hotovo
#define EX_SMART_ROGUE 4
#define EX_RECOVER_DESTROYED_ITEMS 8  //hotovo
#define EX_MULTIPLE_ITEMS_IN_CURSOR 16  //zruseno
#define EX_BAG_EXTENDED 32		//hotovo
#define EX_SHIELD_BLOCKING 64
#define EX_FAST_TRADE 128       //hotovo
#define EX_ALWAYS_MINIMAP 256   //hotovo
#define EX_GROUP_FLEE 512
#define EX_NOHUNGRY 1024
#define EX_AUTOOPENBOOK 2048
#define EX_AUTOSHOWRUNE 4096
extern int game_extras;
