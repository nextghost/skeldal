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
#include "skeldal_pch.h"
#include "types.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "memman.h"
#include <time.h>
#include "swaper.h"

#define DPMI_INT 0x31
#define LOAD_BUFFER 4096

#undef malloc

void bonz_table();

#define NON_GETMEM_RESERVED (4*1024)
char **mman_pathlist=NULL;
static char swap_status=0;

static int swap;
char mman_patch=0;

int memman_handle;
static int max_handle=0;
//static FILE *log;

void (*mman_action)(int action)=NULL;

long last_load_size;
void get_mem_info(MEMORYSTATUS *mem)
  {
/*  mem->dwLength=sizeof(*mem);
  GlobalMemoryStatus(mem); */
  }


void standard_mem_error(size_t size)
  {
  char buff[256];
  SEND_LOG("(ERROR) Memory allocation error detected, %u bytes missing",size,0);
  DXCloseMode();
  sprintf(buff,"Memory allocation error\n Application can't allocate %lu bytes of memory (%xh)\n",size,memman_handle);
  ShowError(buff);
  exit(1);
  }

void load_error(char *filename)
  {
  char buff[256];
  SEND_LOG("(ERROR) Load error detected, system can't load file: %s",filename,0);
  #ifdef LOGFILE
 //   bonz_table();
  #endif
  DXCloseMode();
  sprintf(buff,"Load error while loading file: %s", filename);
  ShowError(buff);
  exit(1);
  }

void standard_swap_error()
  {
  char buff[256];
  DXCloseMode();
  sprintf(buff,"Swap error. Maybe disk is full");
  ShowError(buff);
  exit(1);
  }


void (*mem_error)(size_t)=standard_mem_error;
void (*swap_error)()=standard_swap_error;

void *getmem(long size)
  {
  void *p,*res;

  if (!size) return NULL;
  do
     {
     res=malloc(NON_GETMEM_RESERVED);
     if (res!=NULL)
        {
        p=(void *)malloc(size);
        free(res);
        }
     else p=NULL;
     if (p==NULL) mem_error(size);
     }
  while (p==NULL);
//  SEND_LOG("(ALLOC) **** Alloc: %p size %d",p,*((long *)p-1));
  return p;
  }


void *load_file(char *filename)
  {
  int f;
  long size,*p;

  if (mman_action!=NULL) mman_action(MMA_READ);
  SEND_LOG("(LOAD) Loading file '%s'",filename,0);
  f=open(filename,O_BINARY | O_RDONLY);
  if (f==-1) load_error(filename);
  size=filelength(f);
  p=(void *)getmem(size);
  if (read(f,p,size)!=size) load_error(filename);
  close(f);
  last_load_size=size;
  return p;
  }

//--------------- BLOCK MASTER OPERATION SYSTEM --------------------------
//--------------- part: MEMORY MANAGER -----------------------------------
typedef struct tnametable
        {
        char name[12];
        long seek;
        }TNAMETABLE;


static long *grptable,grptabsiz;
static TNAMETABLE *nametable;
static int nmtab_size;
static int next_name_read=0;
static int last_group;

char *main_file_name=NULL;
handle_groups _handles;
static int bmf=-1;
static int patch=-1;
unsigned long bk_global_counter=0;
char *swap_path;

#ifdef LOGFILE
static void bonz_table()
  {
  int i;
  if (bmf==-1) return;
  for(i=0;i<nmtab_size;i++)
    {
    fprintf(stderr,"%-12.12s %12d\n",nametable[i].name,nametable[i].seek);
    }
  }
#endif

static int test_file_exist_DOS(int group,char *filename)
  {
     char *f;

     f=alloca(strlen(mman_pathlist[group])+strlen(filename)+1);
     strcpy(f,mman_pathlist[group]);
     strcat(f,filename);
     if (access(f,0)) return 0;
     return 1;
  }


void load_grp_table()
  {
  long i;

  SEND_LOG("(LOAD) Loading Group Table",0,0);
  lseek(bmf,4,SEEK_SET);
  read(bmf,&i,4);
  grptable=(long *)getmem(i+4);
  lseek(bmf,0,SEEK_SET);
  read(bmf,grptable,i);
  grptabsiz=i;
  for(i=0;i<(grptabsiz>>3);i++) grptable[i*2+1]=(grptable[i*2+1]-grptabsiz)>>4;
  SEND_LOG("(LOAD) Group Table Loaded",0,0);
  }

void load_file_table()
  {
  int strsize;
  void *p;

  SEND_LOG("(LOAD) Loading File Table",0,0);
  lseek(bmf,grptabsiz,SEEK_SET);
  lseek(bmf,12,SEEK_CUR);
  read(bmf,&strsize,4);
  strsize-=grptabsiz;
  lseek(bmf,grptabsiz,SEEK_SET);
  p=getmem(strsize);memcpy(&nametable,&p,4);
  read(bmf,nametable,strsize);
  nmtab_size=strsize/sizeof(*nametable);
  SEND_LOG("(LOAD) File Table Loaded",0,0);
  }



int find_name(int group,char *name)
  {
  int i;

  for(i=0;i<(grptabsiz>>2);i+=2)
     {
     if (grptable[i]==group) break;
     }
  if ((grptabsiz>>2)<=i) return -1;
  for(i=grptable[i+1];i<nmtab_size;i++)
     if (!strncmp(nametable[i].name,name,12)) break;
  if (i==nmtab_size) return -1;
  return i;
  }

int get_file_entry(int group,char *name)
  {
  int i;
  char ex;

  if (mman_patch) ex=test_file_exist_DOS(group,name);else ex=0;
  if (ex || bmf==0) return 0;
  i=find_name(group,name);
  if (i==-1) return 0;
  return nametable[i].seek;
  }
int swap_block(THANDLE_DATA *h)
  {
  long wsize,pos;

  if (mman_action!=NULL) mman_action(MMA_SWAP);
  if (swap==-1) return -1;
  if (h->flags & BK_HSWAP) pos=h->seekpos; else pos=swap_add_block(h->size);
  wsize = lseek(swap,0,SEEK_END);
  if (wsize<pos) write(swap,NULL,pos-wsize);
  lseek(swap,pos,SEEK_SET);
  SEND_LOG("(SWAP) Swaping block '%-.12hs'",h->src_file,0);
  wsize=write(swap,h->blockdata,h->size);
  swap_status=1;
  if ((unsigned)wsize==h->size)
     {
     h->seekpos=pos;
     if (h->flags & BK_PRELOAD) h->flags&=~BK_SWAPABLE;
     h->flags|=BK_HSWAP;
     return 0;
     }
  else
     {
     SEND_LOG("(SWAP) Swap failed!",0,0);
     swap_error();
     }
  swap_free_block(pos,h->size);
  return -1;
  }

THANDLE_DATA *get_handle(int handle)
  {
  int group,list;

  group=handle/BK_MINOR_HANDLES;
  list=handle % BK_MINOR_HANDLES;
  if (_handles[group]==NULL)
     {
     _handles[group]=(handle_list *)getmem(sizeof(handle_list));
     memset(_handles[group],0,sizeof(handle_list));
     }
  if (handle>max_handle) max_handle=handle;
  return ((THANDLE_DATA *)_handles[group])+list;
  }

void heap_error(size_t size) //heap system
  {
  int i,j;
//  char swaped=0;
  unsigned long maxcounter=0;
  THANDLE_DATA *sh;
  char repeat=0,did=0;
  THANDLE_DATA *lastblock=NULL;
  char *last_free=NULL;
//  int num;
  do
  {
  maxcounter=0;
  sh=NULL;
  repeat=0;did=0;
  for(i=0;i<BK_MAJOR_HANDLES;i++)
       if (_handles[i]!=NULL)
         {
         unsigned long c,max=0xffffffff,d;
         for (j=0;j<BK_MINOR_HANDLES;j++)
           {
           THANDLE_DATA *h;

           h=((THANDLE_DATA *)_handles[i]+j);
           c=bk_global_counter-h->counter;
           if (h->status==BK_PRESENT && ~h->flags & BK_LOCKED) {
              if (last_free!=NULL)
                 {
                 d=(char *)h->blockdata-last_free;
                 if (d<max) sh=h,max=d,did=1;//,num=i*BK_MINOR_HANDLES+j;
                 }
              else if (c>maxcounter)
                 {
                 maxcounter=c;
                 sh=h;
                 did=1;
                 //num=i*BK_MINOR_HANDLES+j;
                 }
           	   }
           }
         }
  if (lastblock==sh)
     {
     did=0;repeat=0;
     }
  if (did)
     {
     size-=sh->size;
     last_free=sh->blockdata;
     if (sh->flags & BK_SWAPABLE)
        {
        if (swap_block(sh))  //pri neuspechu o ulozeni je nalezen blok jiny
           {
           sh->counter=bk_global_counter;
           repeat=1;
           }
        else
           {
           free(sh->blockdata);
           sh->status=BK_SWAPED;
  //         swaped=1;
           }
        }
     else
        {
        if (sh->flags & BK_PRELOAD) sh->status=BK_SWAPED;
        else sh->status=BK_NOT_LOADED;
        free(sh->blockdata);
        if (mman_action!=NULL) mman_action(MMA_FREE);
        }
     }
  else
     standard_mem_error(size);
  lastblock=sh;
  }
  while (repeat || size>0);
//  if (swaped) _dos_commit(swap);
  }

THANDLE_DATA *kill_block(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);if (h->status==BK_NOT_USED) return h;
  if (h->flags & BK_LOCKED)
     {
     SEND_LOG("(ERROR) Unable to kill block! It is LOCKED! '%-.12hs' (%04X)",h->src_file,handle);
     return NULL;
     }
  SEND_LOG("(KILL) Killing block '%-.12hs' (%04X)",h->src_file,handle);
  if (h->status==BK_SAME_AS) return h;
  if (h->status==BK_PRESENT) free(h->blockdata);
  if (h->flags & BK_HSWAP) swap_free_block(h->seekpos,h->size);
  h->status=BK_NOT_LOADED;
  h->flags&=~BK_HSWAP;
  return h;
  }

THANDLE_DATA *zneplatnit_block(int handle)
  {
  THANDLE_DATA *h;

  h=kill_block(handle);
  if (h->status==BK_SAME_AS)
     return zneplatnit_block(h->seekpos);
  if (h->src_file[0]) h->seekpos=get_file_entry(h->path,h->src_file);
  return h;
  }

void init_manager(const char *filename,const char *swp) // filename= Jmeno datoveho souboru nebo NULL pak
                                  // se pouzije DOS
                                            // swp je cesta do TEMP adresare
  {
  next_name_read=0;
  last_group=0;
  memset(_handles,0,sizeof(_handles));
  if (filename!=NULL)
     {
     bmf=open(filename,O_BINARY | O_RDONLY);
     if (bmf!=-1)
        {
        main_file_name=(char *)getmem(strlen(filename)+1);
        strcpy(main_file_name,filename);
        load_grp_table();
        load_file_table();
        }
     else
        main_file_name=NULL;
     }
  else
     main_file_name=NULL;
  mem_error=heap_error;
  if (swp!=NULL)
     {
     swap=open(swp,O_BINARY | O_RDWR | O_CREAT | O_TRUNC,0777);
     swap_init();
     }
  else
     swap=-1;
  }

void *load_swaped_block(THANDLE_DATA *h)
  {
  void *i;

  if (mman_action!=NULL) mman_action(MMA_SWAP_READ);
  i=getmem(h->size);
  SEND_LOG("(LOAD)(SWAP) Loading block from swap named '%-.12hs'",h->src_file,0);
  lseek(swap,h->seekpos,SEEK_SET);
  read(swap,i,h->size);
  h->status=BK_PRESENT;
  return i;
  }


int find_same(char *name,void *decomp)
  {
  THANDLE_DATA *p;
  int i,j;

  (void)decomp;
  if (name[0]==0) return -1;
  for(i=0;i<BK_MAJOR_HANDLES;i++)
     if (_handles[i]!=NULL)
     {     
     p=(THANDLE_DATA *)(_handles[i]);
     for(j=0;j<BK_MINOR_HANDLES;j++)
        if ((!strncmp(p[j].src_file,name,12))&& (p[j].loadproc==decomp)) return i*BK_MINOR_HANDLES+j;
     }
  return -1;
  }

int find_handle(char *name,void *decomp)
  {
  return find_same(name,decomp);
  }

int test_file_exist(int group,char *filename)
  {
  if (find_name(group,filename)==-1) return test_file_exist_DOS(group,filename);
  return 1;
  }

THANDLE_DATA *def_handle(int handle,char *filename,void *decompress,char path)
  {
  THANDLE_DATA *h;
  int i;

  i=find_same(filename,decompress);
  h=get_handle(handle);
  if (i==handle) return h;
  if (kill_block(handle)==NULL)
     {
     SEND_LOG("(ERROR) File/Block can't be registred, handle is already in use '%-.12hs' handle %04X",filename,handle);
     return NULL;
     }
  if (i!=-1 && i!=handle)
     {
     h->status=BK_SAME_AS;
     h->seekpos=i;
     return h;
     }
  strncpy(h->src_file,filename,12);
  h->seekpos=0;
  strupr(h->src_file);
  h->loadproc=decompress;
  if (filename[0])
      h->seekpos=get_file_entry(path,h->src_file);
  SEND_LOG("(REGISTER) File/Block registred '%-.12hs' handle %04X",h->src_file,handle);
  SEND_LOG("(REGISTER) Seekpos=%d",h->seekpos,0);
  h->flags=0;
  h->path=path;
  if (h->status!=BK_DIRLIST) h->status=BK_NOT_LOADED;
  h->counter=bk_global_counter++;
  return h;
  }

void *afile(char *filename,int group,long *blocksize)
  {
  char *c,*d;
  long entr;
  void *p;

  d=alloca(strlen(filename)+1);
  strcpy(d,filename);
  strupr(d);
  if (mman_patch && test_file_exist_DOS(group,d)) entr=0;
  else entr=get_file_entry(group,d);
  if (entr!=0)
     {
		 int hnd;
		 SEND_LOG("(LOAD) Afile is loading file '%s' from group %d",d,group);
		 if (entr<0) entr=-entr,hnd=patch;else hnd=bmf;
     lseek(hnd,entr,SEEK_SET);
     read(hnd,blocksize,4);
     p=getmem(*blocksize);
     read(hnd,p,*blocksize);
     }
  else if (mman_pathlist!=NULL)
     {
     SEND_LOG("(LOAD) Afile is loading file '%s' from disk",d,group);
     c=alloca(strlen(filename)+strlen(mman_pathlist[group])+2);
     c=strcat(strcpy(c,mman_pathlist[group]),filename);
     p=load_file(c);
     *blocksize=last_load_size;
     }
  else return NULL;
  return p;
  }

void *ablock(int handle)
  {
  THANDLE_DATA *h;

  sem:
  h=get_handle(handle);
  if (memman_handle!=handle || h->status!=BK_PRESENT) h->counter=bk_global_counter++;
  memman_handle=handle;
  if (h->status==BK_SAME_AS)
     {
     handle=h->seekpos;
     goto sem;
     }
  if (h->status==BK_PRESENT) return h->blockdata;
  if (h->status==BK_DIRLIST)
     {
     free(h->blockdata);h->status=BK_NOT_LOADED;
     }
  if (h->status==BK_NOT_LOADED)
     {
        void *p;long s;
        char c[200];

        SEND_LOG("(LOAD) Loading file as block '%-.12hs' %04X",h->src_file,handle);
        if (h->seekpos==0)
           {
           if (h->src_file[0]!=0)
              {
              if (mman_action!=NULL) mman_action(MMA_READ);
              strcpy(c,mman_pathlist[h->path]);strcat(c,h->src_file);
              c[strlen(mman_pathlist[h->path])+12]='\0';
              p=load_file(c);
              s=last_load_size;
              }
           else
              {
              p=NULL;
              s=0;
              }
           if (h->loadproc!=NULL) h->loadproc(&p,&s);
           h->blockdata=p;
           h->status=BK_PRESENT;
           h->size=s;
           return p;
           }
        else
           {
					 int entr=h->seekpos,hnd;
           if (mman_action!=NULL) mman_action(MMA_READ);
					 if (entr<0) entr=-entr,hnd=patch;else hnd=bmf;
           lseek(hnd,entr,SEEK_SET);
           read(hnd,&s,4);
           p=getmem(s);
           read(hnd,p,s);
           if (h->loadproc!=NULL) h->loadproc(&p,&s);
           h->blockdata=p;
           h->status=BK_PRESENT;
           h->size=s;
           return p;
           }
        }
     //tato cast programu bude jeste dodelana - else ....
  if (h->status==BK_SWAPED)
     {
     return h->blockdata=load_swaped_block(h);
     }
  return NULL;
  }

void alock(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  if (!h->lockcount)
     {
     h->flags|=BK_LOCKED;
     if (h->status==BK_SAME_AS) alock(h->seekpos);
     }
  h->lockcount++;
  //SEND_LOG("(LOCK) Handle locked %04X (count %d)",handle,h->lockcount);
  }

void aunlock(int handle)
  {
  THANDLE_DATA *h;
  h=get_handle(handle);
  if (h->lockcount) h->lockcount--;else return;
  if (!h->lockcount)
     {
     h->flags&=~BK_LOCKED;
     if (h->status==BK_SAME_AS) aunlock(h->seekpos);
     }
  //SEND_LOG("(LOCK) Handle unlocked %04X (count %d)",handle,h->lockcount);
  }
void aswap(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  h->flags|=BK_SWAPABLE;
  if (h->status==BK_SAME_AS) aswap(h->seekpos);
  }

void aunswap(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  h->flags|=BK_SWAPABLE;
  if (h->status==BK_SAME_AS) aunswap(h->seekpos);
  }

void apreload(int handle)
  {
  THANDLE_DATA *h;


  h=get_handle(handle);
  if (h->src_file[0] && h->status!=BK_SAME_AS)
     {
     if (!(h->flags & BK_PRELOAD) || !(h->flags & BK_HSWAP)) h->flags|=BK_SWAPABLE | BK_PRELOAD;
     ablock(handle);
     }
  }

static long *apr_sign=NULL;
static long max_sign;

void apreload_sign(int handle,int max_handle)
  {
  THANDLE_DATA *h;
  if (apr_sign==NULL)
     {
     apr_sign=NewArr(long,max_handle);
     memset(apr_sign,0x7f,sizeof(long)*max_handle);
     max_sign=max_handle;
     }
  if (handle>=max_sign)
     {
     apreload(handle);
     return;
     }
  h=get_handle(handle);
  if (h->src_file[0] && h->status!=BK_SAME_AS && h->status!=BK_NOT_USED)
     if (!(h->flags & BK_PRELOAD) || !(h->flags & BK_HSWAP)) apr_sign[handle]=h->seekpos;
  }

static int apreload_sort(const void *val1,const void *val2)
  {
  long vl1,vl2;

  vl1=apr_sign[*(word *)val1];
  vl2=apr_sign[*(word *)val2];
  return (vl1>vl2)-(vl1<vl2);
  }

void apreload_start(void (*percent)(int cur,int max))
  {
  word *p;
  int i;
  int c,z;

  swap_status=0;
  p=NewArr(word,max_sign);
  for(i=0;i<max_sign;i++) p[i]=i;
  qsort(p,max_sign,sizeof(word),apreload_sort);
  for(i=0,c=0;i<max_sign;i++) if (apr_sign[p[i]]==0x7f7f7f7f)p[i]=-1;else c++;
  for(i=0,z=0;i<max_sign;i++) if (p[i]!=-1)
     {
     apreload(p[i]);
     percent(z++,swap_status?c+max_sign*2:c);
     }
  if (swap_status)
     for(i=0;i<max_sign;i++)
        {
        THANDLE_DATA *h=get_handle(p[i]);
        if (h->status==BK_PRESENT) swap_block(h);
        percent(c+i,c+max_sign);
        }
//  _dos_commit(swap);
  free(apr_sign);
  free(p);
  apr_sign=NULL;
  }

void undef_handle(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  if (h->status!=BK_NOT_USED)
     {
     if (kill_block(handle)==NULL) return;
     SEND_LOG("(REGISTER) File/Block unregistred %04X (%-.12hs)",handle,h->src_file);
     }
  h->src_file[0]=0;
  h->seekpos=0;
  h->flags=0;
  h->status=BK_NOT_USED;
  }

void close_manager()
  {
  int i,j;

  for(i=0;i<BK_MAJOR_HANDLES;i++) if (_handles[i]!=NULL)
     {
     for(j=0;j<BK_MINOR_HANDLES;j ++) undef_handle(i*BK_MINOR_HANDLES+j);
     free(_handles[i]);
     }
  free(main_file_name);
  close(bmf);
  if (swap!=-1) close(swap);
//  fclose(log);
  free(grptable); grptable=NULL;
  free(nametable); nametable=NULL;
  max_handle=0;
  }


//------------------------------------------------------------
/*static void block()
  {
  static MEMINFO inf;
  void *c;
  static counter=0;

  get_mem_info(&inf);
  c=malloc(inf.LargestBlockAvail-1024);
  counter++;
  printf("%d. %d\n",counter,inf.LargestBlockAvail);
  if (c!=NULL) block();
  counter--;
  free(c);
  }*/

void display_status()
  {
  int i,j;
  THANDLE_DATA *h;
  char names[][5]={"MISS","*ok*","SWAP","????","PTR "};
  char flags[]={"LS*PH"};
  char copys[6]="     ";
  char nname[14];
  long total_data=0;
  long total_mem=0;
//  MEMORYSTATUS mem;
  int ln=0;

  //block();
  //getchar();
  memset(nname,0,sizeof(nname));
  for(i=0;i<BK_MAJOR_HANDLES;i++)
     if (_handles[i]!=NULL)
     {
     for(j=0;j<BK_MINOR_HANDLES;j++)
        {
        h=get_handle(i*BK_MINOR_HANDLES+j);
        if (h->status!=BK_NOT_USED && h->status!=BK_SAME_AS)
           {
           int k;

           for(k=0;k<5;k++) copys[k]=h->flags & (1<<k)?flags[k]:'.';
           if (h->src_file[0]) strncpy(nname,h->src_file,12);else strcpy(nname,"<local>");
           printf("%04Xh ... %12s %s %s %08Xh %6d %10d %6d \n",i*BK_MINOR_HANDLES+j,
           nname,names[h->status-1],
           copys,(int)h->blockdata,h->size,h->counter,h->lockcount);
           ln++;
           total_data+=h->size;
           if(h->status==BK_PRESENT)total_mem+=h->size;
           if (ln%24==23)
              {
              char c;
              printf("Enter to continue, q-quit:");
              c=getchar();
              if (c=='q' || c=='Q')
                 {
                 while (getchar()!='\n');
                 return;
                 }
              }
           }
        }

     }
//  get_mem_info(&mem);
  printf("Data: %7d KB, Loaded: %7d KB, Largest free: %7d KB",total_data/1024,total_mem/1024,0);
  while (getchar()!='\n');
  }

void *grealloc(void *p,long size)
  {
  void *q;
  long scop;

  if (!size)
     {
     free(p);
     return NULL;
     }
  /*q=realloc(p,size);
  if (q!=NULL)
     {
     SEND_LOG("(ALLOC) **** Realloc: New %p size %d\n",q,*((long *)q-1));
     return q;
     }*/
  q=getmem(size);
  if (p==NULL) return q;
  scop=_msize(p);
  if (scop>size) scop=size;
  memmove(q,p,scop);
  free(p);
  return q;
  }

char *read_next_entry(char mode)
  {
  if (mode==MMR_FIRST) next_name_read=0;
  if (main_file_name==NULL) return NULL;
  if (next_name_read>=nmtab_size) return NULL;
  return nametable[next_name_read++].name;
  }

int read_group(int index)
  {
  return grptable[index<<1];
  }

char add_patch_file(char *filename)
	{
	long l;
	long poc;
	int i,cc=0;
	TNAMETABLE p;
	SEND_LOG("Adding patch: %s",filename,0);
	if (patch!=-1) return 2;
	if (bmf==-1) return 3;
	patch=open(filename,O_BINARY|O_RDONLY);
	if (patch==-1) return 1;
	lseek(patch,4,SEEK_SET);
	read(patch,&l,4);
	lseek(patch,l,SEEK_SET);
	read(patch,&p,sizeof(p));
	poc=(p.seek-l)/sizeof(p);
	lseek(patch,l,SEEK_SET);
	for(i=0;i<poc;i++)
		{
		int j;
		read(patch,&p,sizeof(p));
		j=find_name(read_group(0),p.name);
		if (j==-1)
			{
			nametable=grealloc(nametable,sizeof(TNAMETABLE)*(nmtab_size+1));
			j=nmtab_size++;strncpy(nametable[j].name,p.name,12);
			}
		nametable[j].seek=-p.seek,cc++;
		}
	SEND_LOG("Patch added: %s - %d entries modified",filename,cc);
	return 0;
	}

#ifdef LOGFILE

char *get_time_str()
  {
  time_t long_time;
  struct tm *newtime;
  static char text[20];


  time( &long_time );                /* Get time as long integer. */
  newtime = localtime( &long_time ); /* Convert to local time. */

  sprintf(text,"%02d:%02d:%02d",newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
  return text;
  }


#endif

long get_handle_size(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  if (h->status==BK_NOT_USED) return -1;
  return h->size;
  }

/*void *malloc(size_t size)
  {
  static char zavora=1;
  void *c;

  c=_nmalloc(size);
  if (log!=NULL && zavora)
     {
     zavora=0;
     fprintf(log,"Alloc: %p size %d\n",c,*((long *)c-1));
     zavora=1;
     }
  return c;
  }
 */

FILE *afiletemp(char *filename, int group)
  {
  long size;  
  void *p=afile(filename,group,&size);
  FILE *f;
  if (p==NULL) return NULL;
  f=tmpfile();
  if (f==NULL) {free(p);return NULL;}
  fwrite(p,size,1,f);
  fseek(f,0,SEEK_SET);
  return f;
  }
