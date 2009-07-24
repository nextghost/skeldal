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
//#define LOGFILE

#ifndef _MEMMAN_H_

#include <stdio.h>

#define _MEMMAN_H_
#define freemem(size) free(size);
//#define malloc(size) getmem(size)
#define New(typ) (typ *)getmem(sizeof(typ))
#define NewArr(typ,count) (typ *)getmem(sizeof(typ)*(count))

typedef struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MEMINFO;


typedef struct thandle_data
  {
  char src_file[12];  //12
  long seekpos;       //16
  void *blockdata;    //20
  char flags;        //21
  char path;        //22
  short status;
  void (*loadproc)(void **data,long *size);//28
  unsigned short lockcount;               //32
  unsigned long counter;
  unsigned long size;
  }THANDLE_DATA;

#define BK_MAJOR_HANDLES 256 // maximalni pocet skupin rukojeti
#define BK_MINOR_HANDLES 256 // pocet rukojeti v jedne skupine

typedef THANDLE_DATA handle_list[BK_MINOR_HANDLES];
typedef handle_list *handle_groups[BK_MAJOR_HANDLES];

//status
#define BK_NOT_USED 0
#define BK_NOT_LOADED 1
#define BK_PRESENT 2
#define BK_SWAPED 3
#define BK_DIRLIST 4 //znaci ze handle je docasne pouzito pro adresarovy list
           //pokud je status=4 je automaticky chapan jako 1.
#define BK_SAME_AS 5
//flags
#define BK_LOCKED 1
#define BK_SWAPABLE 2
#define BK_SHARED 4
#define BK_PRELOAD 8
#define BK_HSWAP 16

//extern char **mman_pathlist;  //tento pointer musi byt naplnen ukazatelem na tabulku cest
extern void (*mem_error)(size_t);    //pokud neni NULL je tato funkce volana vzdy kdyz dojde pamet a system si s tim nevi rady
extern void (*swap_error)();
extern int memman_handle; //cislo handle naposled zpracovavaneho prikazem ablock
extern char mman_patch;    //jednicka zapina moznost pouziti patchu
void *getmem(long size);        //alokace pameti pres memman. alokovat pomoci malloc lze ale hrozi nebezpeci ze vrati NULL
void *grealloc(void *m,long size); //realokace pameti pres memman
void *load_file(char *filename); //obycejne natahne soubor do pameti a vrati ukazatel.
void init_manager(char *filename,char *swp); //inicializuje manager. Jmeno filename i swapname nejsou povinne (musi byt NULL kdyz nejsou pouzity)
THANDLE_DATA *def_handle(int handle,char *filename,void *decompress,char path); //deklaruje rukojet. promenna decompress je ukazatel na funkci ktera upravi data pred vracenim ukazatele
void *ablock(int handle);             //vraci ukazatel bloku spojeneho s handlem
void alock(int handle);               //zamyka blok
void aunlock(int handle);             //odmyka blok
void aswap(int handle);               //zapina swapovani pro blok
void aunswap(int handle);             //vypina swapovani pro blok
void apreload(int handle);            //zapina preloading pro blok (preloading proved pomoci ablock)
//void free();                          //free
void close_manager();                 //uzavre manager a uvolni veskerou pamet
void undef_handle(int handle);        //uvolni hadle k dalsimu pouziti
THANDLE_DATA *zneplatnit_block(int handle); //zneplatni data bloku
THANDLE_DATA *get_handle(int handle); //vraci informace o rukojeti
int find_handle(char *name,void *decomp);   //hleda mezi rukojeti stejnou definici
int test_file_exist(int group,char *filename); //testuje zda soubor existuje v ramci mmanageru
void *afile(char *filename,int group,long *blocksize); //nahraje do pameti soubor registrovany v ramci mmanageru
long get_handle_size(int handle);
//void get_mem_info(MEMORYSTATUS *mem); 

void apreload_sign(int handle,int max_handle);     //pripravi preloading pro nacteni dat z CD (sekvencne)
void apreload_start(void (*percent)(int cur,int max));   //provede sekvenci apreload

char *read_next_entry(char mode);     //cte adresar DDL souboru a vraci jmeno, nebo NULL. mode udava, zda se hleda od zacatku, nebo pokracuje tam kde program skoncil
int read_group(int index);
char add_patch_file(char *filename);  //pripojuje zaplatu
FILE *afiletemp(char *filename, int group);


#define MMA_READ 1
#define MMA_SWAP 2
#define MMA_SWAP_READ 3
#define MMA_FREE 4

#define MMR_FIRST 0
#define MMR_NEXT 1

extern void (*mman_action)(int action);  //udalost volajici se pri akci mmanagera.

void display_status();    //zobrazi na display status memmanageru

#ifdef LOGFILE
char *get_time_str();
int q_current_task();
#define OPEN_LOG(log) memcpy(stderr,fopen(log,"w"),sizeof(FILE));
#define SEND_LOG(format,parm1,parm2) fprintf(stderr,"%-2d %s "format"\n",q_current_task(),get_time_str(),parm1,parm2),fflush(stderr)
#define CLOSE_LOG() fclose(logfile);
#else
#define OPEN_LOG(log)
#define SEND_LOG(format,parm1,parm2)
#define CLOSE_LOG()
#endif
#endif
