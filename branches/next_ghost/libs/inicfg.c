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
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libs/strlite.h"
#include "libs/inicfg.h"

TSTR_LIST read_config(const char *filename)
  {
  FILE *f;
  TSTR_LIST ls;
  char buff[256];

  f=fopen(filename,"r");
  if (f==NULL) return NULL;
  ls=create_list(256);
  while (!feof(f))
     {
     char *c;
     buff[0]=0;
     if (fgets(buff,256,f)==NULL) break;
     c=strchr(buff,'\n');if (c!=NULL) *c=0;
     if (ferror(f))
        {
        release_list(ls);
        fclose(f);
        return NULL;
        }
     str_add(&ls,buff);
     }
  fclose(f);
  return ls;
  }

TSTR_LIST merge_configs(TSTR_LIST target, TSTR_LIST source)
{
  char buff[256];
  int i;
  char *c;

  buff[255]=0;
  for (i=0;i<str_count(source);i++) if (source[i])
  {
	strncpy(buff,source[i],255);
	c=strchr(buff,' ');
	if (c!=NULL)
	{
	  int p;
	  *c=0;
	  p=find_ini_field(target,buff);
	  if (p==-1)
	  {
		str_add(&target,source[i]);
	  }
	  else
	  {
		str_replace(&target,p,source[i]);
	  }
	}
  }
  return target;
}

char comcmp(const char *text,const char *command)
  {
  while (toupper(*text)==toupper(*command)) text++,command++;
  if (*command==0 && *text==' ') return 1;
  return 0;
  }

int find_ini_field(TSTR_LIST ls,const char *name)
  {
  const char *a;
  char *b;
  int i,cnt=str_count(ls);

  for (i=0;i<cnt;i++) if (ls[i]!=NULL)
     {
     a=name;
     b=ls[i];
     while (*b==32) b++;
     if (*b==';') continue;
     if (comcmp(b,a)) return i;
     }
  return -1;
  }

void add_field_txt(TSTR_LIST *ls,const char *name,const char *text)
  {
  int i;
  char *d;

  d=alloca(strlen(name)+strlen(text)+2);
  sprintf(d,"%s %s",name,text);
  i=find_ini_field(*ls,name);
  if (i==-1) str_add(ls,d);
  else
     str_replace(ls,i,d);
  }

void add_field_num(TSTR_LIST *ls,const char *name,long number)
  {
  char buff[20];

//  itoa(number,buff,10);
  sprintf(buff, "%d", number);
  add_field_txt(ls,name,buff);
  }

int save_config(TSTR_LIST ls,const char *filename)
  {
  int i,cnt,err=0;
  FILE *f;
  f=fopen(filename,"w");
  if (f==NULL) return -1;
  cnt=str_count(ls);
  for(i=0;i<cnt && !err;i++) if (ls[i]!=NULL)
     {
     if (fputs(ls[i],f)==EOF) err=-1;
     putc('\n',f);
     }
  fclose(f);
  return err;
  }


const char *get_text_field(TSTR_LIST ls,const char *name)
  {
  int i;
  char *c;
  if (ls==NULL) return NULL;
  i=find_ini_field(ls,name);if (i==-1) return NULL;
  c=ls[i];
  c=strchr(c,' ');if (c!=NULL) while (*c==' ') c++;
  else c=strchr(ls[i],0);
  return c;
  }


int get_num_field(TSTR_LIST ls,const char *name,int *num)
  {
  const char *c;

  c=get_text_field(ls,name);
  if (c==NULL) return -1;
  if (sscanf(c,"%d",num)!=1) return -1;
  return 0;
  }

void process_ini(TSTR_LIST ls,void (*process)(const char *line))
  {
  char *c;
  int i,cnt;
  cnt=str_count(ls);

  for(i=0;i<cnt;i++) if (ls[i]!=NULL)
     {
     c=ls[i];
     while (*c==' ') c++;
     if (*c==';') continue;
     process(c);
     }
  }

