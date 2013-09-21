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
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>

char copy_path[500];
char *adv_name;

char _change_disk(unsigned znak)
  {
  unsigned total;
  znak-='@';
  _dos_setdrive(znak,&total);
  _dos_getdrive(&total);
  return total==znak;
  }

char disk_finder()
  {
  static struct find_t ft;
  int err;

  if (!access("SKELDAL.EXE",F_OK) && !access("ADV",F_OK)) return 1;
  err=_dos_findfirst("*.*",_A_SUBDIR,&ft);
  while (!err)
    {
    if (ft.attrib & _A_SUBDIR && strcmp(ft.name,".") && strcmp(ft.name,".."))
      {
      chdir(ft.name);
      if (disk_finder()) return 1;
      chdir("..");
      }
    err=_dos_findnext(&ft);
    }
  return 0;
  }

char find_path(char *path)
  {
  char *oldpath;
  unsigned pismeno='C';

  cputs("Hledam...\r");
  oldpath=getcwd(NULL,PATH_MAX);
  for(pismeno='C';pismeno<='Z';pismeno++)
    {
    _change_disk(pismeno);
    chdir("..");
    if (disk_finder()==1)
      {
      getcwd(path,PATH_MAX);
      chdir(oldpath);_change_disk(oldpath[0]);
      free(oldpath);
      return 1;
      }
    }
  chdir(oldpath);_change_disk(oldpath[0]);
  free(oldpath);
  return 1;
  }


void main(int argc,char **argv)
  {
  char temp[550];
  char rep;
  if (argc<2)
    {
    puts("Nespravne parametry!\n"
         "\n"
         "Pouziti ADVINST [jmeno]\n"
         "\n"
         "jmeno = Nazev dobrodruzstvi bez pripony");
    return;
    }
  adv_name=argv[1];
  sprintf(temp,"%s.adv",adv_name);
  if (access(adv_name,F_OK) && access(temp,F_OK))
    {
    printf("Nemohu najit zadne dobrodruzstvi s timto jmenem (%s)!\n",adv_name);
    return;
    }
  do
    {
    printf("Vypiste celou cestu, kde lezi hra (napr: c:\\hry\\skeldal)\n"
           "Pokud vlozte otaznik (?), instalator se pokusi hru na disku vyhledat\n"
           "Pokud stisknete pouze <ENTER>, instalator se ukonci\n"
           ">");
    gets(copy_path);
    if (copy_path[0]=='?')
      {
      find_path(copy_path);
      printf("\nInstalator nasel hru na ceste: %s\n\n",copy_path);
      }
    if (copy_path[0]==0) return;
    sprintf(temp,"%s\\skeldal.exe",copy_path);
    rep=access(temp,F_OK);
    if (rep) puts("Vami vlozena cesta neni spravna!\n");
    else
      {
      sprintf(temp,"%s\\adv\\%s",copy_path,adv_name);
      if (access(temp,F_OK))if (mkdir(temp)!=0) printf("Nedokazal jsem vytvorit adresar %s\n\n\n",temp),rep=1;
      }
    }
  while(rep);
  sprintf(temp,"copy %s.adv %s > nul",adv_name,copy_path);
  puts(temp);
  system(temp);
  sprintf(temp,"copy %s\\*.* %s\\adv\\%s > nul",adv_name,copy_path,adv_name);
  puts(temp);
  system(temp);
  sprintf(temp,"%s.bat",adv_name);
  if (access(temp,F_OK)==0)
    {
    sprintf(temp,"%s.bat %s",adv_name,copy_path);
    puts(temp);
    system(temp);
    }
  chdir(copy_path);
  puts("Instalace uspesna!");
  printf("Nove dobrodruzstvi spustis prikazem SKELDAL %s.adv\n",adv_name);
  puts("Chces si nyni nove dobrodruzstvi vyzkouset? ANO/NE (cokoliv/N)");
  if (toupper(getche())=='N') return;
  sprintf(temp,"SKELDAL %s.adv",adv_name);
  system(temp);
  }



