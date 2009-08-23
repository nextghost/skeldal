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
#ifndef __EVENT_H
#define __EVENT_H

#include <inttypes.h>
#include <cstdarg>

//#define nodebug  // 0 znamena ze se nealokuje udalost pro chybu
// Tato knihovna definuje zakladni systemove konstanty
// pro system hlaseni a udalosti

#pragma pack(1)

// zakladni

#define E_INIT 1      //inicializace udalost (interni)
#define E_ADD 2       //pridani udalosti do stromu
#define E_DONE 3      //odebrani udalosti ze stromu
#define E_IDLE 4      //udalost volana v dobe necinnosti
#define E_GROUP 5     //vytvareni skupin udalosti
#define E_ADDEND 7    //pridani udalosti do stromu na konec.
#define E_MEMERROR 8  //udalost je vyvolana pri nedostatku pameti
                       // neni prirazena ZADNA standardni akce!!!
#define E_WATCH 6     //udalost majici prednost pred idle
                       //slouzi pro procedury sledujici cinnost hardware
#define E_PRGERROR 9  //udalost vyvolana pri chybe programu
                       //parametrem udalosti je ukazatel na int ktery oznamuje cislo chyb
                      //vysledek zapsat do tohoto intu (0-exit,1-ignorovat)
#define E_ADD_REPEAT 10       //pridani udalosti do stromu


#define ERR_MEMREF -1
#define ERR_ILLEGI -2
// zarizeni

#define E_KEYBOARD 10
#define E_MOUSE 11
#define E_TIMER 12

#define TASK_RUNNING 0
#define TASK_TERMINATING 1

#define TASK_EVENT_WAITING 2

#define EVENT_BUFF_SIZE 16

typedef struct event_msg {
	int msg;
	va_list data;
} EVENT_MSG;

typedef void (*EV_PROC)(EVENT_MSG *,void *) ;

#define PROC_GROUP (EV_PROC )1

//typedef struct event_list
//  {
//  long *table; //tabulka udalosti
//  EV_PROC *procs;  //co se ma pri danne udalosti stat
//  void *(*user_data); //ukazatel na uzivatelska data
//  char *zavora; //1 znamena ze udalost je povolena
// long max_events; // maximalni pocet udalosti, na ktere je system rezervova
//  long count; //aktualni pocet udalosti
//  }EVENT_LIST;

/* event procedura ma dva parametry

 1. je ukazatel na tzv. message, tato struktura je videt dole.
    Predava se cislo udalosti, ktera nastala a ukazatel na dalsi udaje
    tykajici se udalosti
 2. je ukazatel na ukazatel. Tyto 4 bajty jsou volnym mistem pro samotnou
    udalost. Pri instalaci ma obsah NULL. Udalost si muze volne alokovat
    pamet a pouzit ji pro svoji porebu. Pri deinstalaci se nestara o jeji
    dealokaci, o to se postara system
     (ukazatel na user_data)
 */

typedef struct t_event_point
  {
  EV_PROC proc;
  void *user_data;
  int8_t nezavora;
  int8_t nezavirat;
  int32_t calls;
  struct t_event_point *next;
  }T_EVENT_POINT;


typedef struct t_event_root
  {
  int32_t event_msg;
  int32_t used;
  struct t_event_root *next;
  T_EVENT_POINT *list;
  }T_EVENT_ROOT;

extern char exit_wait; // 1 - opousti aktivni cekaci event;
extern char freeze_on_exit; //1 - po opusteni udalosti cela cesta uzamcena
extern int8_t *otevri_zavoru;
//extern int curtask;
//extern char *task_info;

void init_events();
 // inicalizuje zakladni strom udalosto
void send_message(int message, ...);
 // posila zpravu do stromu
void tree_basics(T_EVENT_ROOT **ev_tree,EVENT_MSG *msg);
 // pripojuje zakladni funkce brany, jako je instalace listu a jejich deinstalace
T_EVENT_ROOT *gate_basics(EVENT_MSG *msg, void **user_data);
 // implementace brany
 /* vstupuji informace, jake dostane brana pri zavolani
    vystupuji informace s jakymi musi vstoupit do stromu.
    Je li MSG = NULL byla zavolana udalost E_DESTROY a brana se musi zlikvidovat
    vysledkem funkce je ukazatel na koren stromu brany.
  */
void enter_event(T_EVENT_ROOT **tree,EVENT_MSG *msg);
 //vstupuje do stromu s udalosti (msg)

void do_events();
void escape();

/*
void *task_sleep(void *param);
//#pragma aux task_sleep parm [eax] value [eax]

int add_task(int stack,void *name,...);
 //spusti provadeni funkce v rizenem multitaskingu (switch task)
void term_task(int id_num);
 //informuje task o ukonceni. Uloha by mela zkoncit
void shut_down_task(int id_num);
 //Nasilne ukonci ulohu
void raise_error(int error_number);

EVENT_MSG *task_wait_event(long event_number);
char is_running(int id_num);
*/
void timer(EVENT_MSG *msg);

#define EVENT_PROC(name) void name(EVENT_MSG *msg,void **user_ptr)
#define WHEN_MSG(msg_num) if (msg->msg==msg_num)
#define UNTIL_MSG(msg_num) if (msg->msg!=msg_num)
#define GET_USER(data_type) (*(data_type *)user_ptr)
#define SAVE_USER_PTR(p) (*user_ptr=p)
#define GET_USER_PTR() user_ptr
#define EVENT_RETURN(value) msg->msg=value
#define GET_MSG_VAR() msg
#define GET_MSG() msg->msg
#define TASK_GET_TERMINATE() ((task_info[cur_task] & TASK_TERMINATING)!=0)

#define EVENT_HALT -1
#define EVENT_DONE -2
#define EVENT_HALT_DONE -3

#pragma option align=reset

#endif



