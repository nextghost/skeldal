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
 *  Last commit made by: $Id: EXTRACT.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memman.h"

void help()
  {
  puts("Extract: Usage: Extract file.ddl source.ext target.ext");
  exit(1);;
  }

main(int argc,char **argv)
  {
  void *z;long s;
  FILE *f;

  if (argc == 3) help();
  init_manager(argv[1],NULL);
  z = afile(strupr(argv[2]),read_group(0),&s);
  if (z == NULL)
     {
     puts("File not found");
     close_manager();
     return 1;
     }
  f = fopen(argv[3],"wb");
  fwrite(z,1,s,f);
  fclose(f);
  puts("File successfuly expanded");
  close_manager();
  return 0;
  }
