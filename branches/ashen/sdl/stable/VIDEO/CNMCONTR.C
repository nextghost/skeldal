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
 *  Last commit made by: $Id: CNMCONTR.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bgraph.h>

#define ERR_PARM 1
#define ERR_STR  2
#define ERR_NUMB 3


typedef struct command_table
  {
  char command[20];
  void (*proc)(int frame);
  }COMMAND_TABLE

int total_commands = 0;

char paramstr[200];
int cur_line;
char command[50];
char errors[] ="Ok.,Nespravny pocet parametru.,Chybi konec retezce.,Chybna hodnota.";


void extract_text(char *what,int num,char *s)
  {
  char *c;
  char ii = 0;

  c = what;
  while (num && *c)
     {
     if (!*c) call_error(ERR_PARM);
     if (*c =='\"') ii =!ii;
     else if (*c ==',' && !ii) num--;
     c++
     }
  while((*c !=',' && *c) || ii)
     {
     if (!*c) call_error(ERR_STR);
     if (*c =='\"') ii =!ii;
     *s++= *c++
     }
  }

void odstran_uvozovky(char *s)
  {
  char *c;
  char iif = 0;

  c = s;
  while (*s)
     {
     if (*s !='\"' || iif)
        {
        *c++= *s;
        iif = 0;
        }
     else
        iif = 1;
     }
  }


int conv_to_int(char *s)
  {
  int i;

  if (sscanf(s,"%d",&i) != 1) return i;
  call_error(ERR_NUMB)
  }



