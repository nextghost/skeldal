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
 *  Last commit made by: $Id: ENCRYPT.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

FILE *source,*target;

void encrypt_file(FILE *sr,FILE *tg)
  {
  int i,j,last = 0;

  i = getc(sr);
  while (i != EOF)
    {
    j = i-last;
    last = i;
    putc(j,tg);
    i = getc(sr);
    }
  }

void open_files(char *src,char *tgr)
  {
  if (tgr == NULL)
    {
    char *c,*d;
    tgr = alloca(strlen(src)+5);
    strcpy(tgr,src);
    c = strrchr(tgr,'\\');
    d = strrchr(tgr,'.');
    if (c>d) d = strchr(tgr,0);
    strcpy(d,".ENC");
    }
  source = fopen(src,"rb");
  target = fopen(tgr,"wb");
  }

void close_files()
  {
  fclose(source);
  fclose(target);
  }


char main(int argc,char **argv)
  {
  if (argc<2)
    {
    puts("Pouziti: ENCRYPT zdroj.ext [cil.ext] \n"
         "\n"
         "Pokud nezadas cil, doplni se zdroj s koncovkou .enc\n"
         "Nikdy nezadavej pouze cestu jako cil. Vzdycky uved jmeno!");
    return 0;
    }
  if (argc == 2) open_files(argv[1],NULL);
  else open_files(argv[1],argv[2]);
  if (source == NULL)
    {puts("Nemuzu najit zdrojovy soubor\n");return 1;};
  if (target == NULL)
    {puts("Nemuzu otevrit cil pro zapis\n");return 1;};
  encrypt_file(source,target);
  close_files();
  puts("Ok.");
  return 0;
  }
