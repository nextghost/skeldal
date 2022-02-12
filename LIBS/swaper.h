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
 *  Last commit made by: $Id: SWAPER.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>

#ifndef SKELDAL_SWAPER_H_
#define SKELDAL_SWAPER_H_



  void swap_init(void);
  int swap_find_block(long size);
  int swap_add_block(long size);
  void swap_find_seek(long seek1,long seek2,int *pos1,int *pos2);
  void alloc_swp_block(long seek,long size);
  void swap_free_block(long seek,long size);







#endif /* SKELDAL_SWAPER_H_ */
