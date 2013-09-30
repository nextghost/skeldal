#ifndef SKELDAL_MEMFILE
#define SKELDAL_MEMFILE

#include <stdint.h>

///Struktura popisuje jeden soubor v pametovem souborovem systemu
typedef struct game_statefile {

	///Soubory jsou ulozeny ve spojovem seznamu - toto je odkaz na dalsi soubor
	struct game_statefile *next;
	///Toto je odkaz na predchozi soubor
	struct game_statefile *prev;
	///Jmeno souboru
	char name[24];
	///Delka souboru
	unsigned int length;
	///Kolik je naalokovano pro pripadne zvetsovani souboru
	unsigned int alloc;
	///Vlastni data souboru - (1 je vychozi, v provozu je zde minimalne alloc bytu)
	uint8_t data[1];
}TMEMFILE;

///Ukazatel na strukturu souboru - pouziva se spis nez primo struktura
typedef TMEMFILE *PMEMFILE;

///Otevre existujici soubor
/**
 @param name jmeno souboru
 @return vraci ukazatel na otevreny soubor, nebo NULL pokud neexistuje.
 */
PMEMFILE openMemFile(const char *name);

///Vytvori novy soubor
/** 
  Vytvori soubor s urcitym nazvem, ale nezaradi ho do filesystemu. 
  @param name jmeno souboru - muze byt duplicitni
  @param initialSize predalokovana velikost pro zvetsovani souboru
  @return vraci ukazatel na nove vytvoreny soubor

  @note Pro vlozeni souboru do filesystemu je treba zavolat commitMemFile.
*/
PMEMFILE createMemFile(const char *name,unsigned int initialSize);

///Prepise existujici soubor zaroven ho prejmenuje
/**
  @param name nove jmeno souboru
  @param f soubor ktery se k tomu vyuzije (pozor ukazatel na promennou)
  @param reserveSize kolik ma byt v souboru rezervovano pro rozsirovani
  Funkce vraci pripadne novy handle souboru
*/
PMEMFILE reuseMemFile(const char *name, PMEMFILE *f,unsigned int reserveSize);

///Zmensi velikost souboru na nulu
/**
  Funkce je zajimava zejmena tim, ze ponecha alokovanou velikost, cimz muze
  zrychlit rozsirovani souboru
  */
PMEMFILE truncateMemFile(PMEMFILE f);

///Prejmenuje soubor
PMEMFILE renameMemFile(PMEMFILE f, const char *name);
//PMEMFILE replaceMemFile(PMEMFILE old, PMEMFILE nw);

///Ulozi soubor do filesystemu
/**
 @param f soubor, ktery se ma vlozit do filesystemu
 @note pokud soubor s uvedenym nazvem existuje, je prepsan.
 */
void commitMemFile(PMEMFILE f);

///Zapis do souboru
/**
  @param pf ukazatel na promennou, ktera obsahuje ukazatel na soubor. Je potreba
    sem vkladat ukazatel na ukazatel, protoze pokud se soubor nahodou zvetsi pres
	predalokovanou velikost, musi se relokovat a tim dojde ke zmenen obsahu
	promenne PMEMFILE
  @param data zaspana data
  @param count pocet bajtu
*/
void writeMemFile(PMEMFILE *pf, const void *data, unsigned int count);
///Cte soubor
/**
  @param f soubor ze ktereho se bude cist
  @param seekPos ukazatel na promennou, ktera obsahuje offset v souboru
  @param data kam data zapsat
  @param count pocet bajtu
  @return vraci pocet prectenych bajtu. Muze byt precteno mene, pokud se cte
  za konec souboru. Pokud je offset mimo soubor, vraci nula.
  Funkce aktualizuje offset (seekPos) podle prectenych bajtu
  */
int readMemFile(PMEMFILE f, unsigned int *seekPos, void *data, unsigned int count);
///Vraci velikost souboru
int lengthOfMemFile(PMEMFILE f);
///Smaze cely pametovy filesystem
void deleteAllMemFiles();
///Smaze soubor z filesystemu
char eraseMemFile(const char *name);
///Vraci prvni soubor ve filesystemu
PMEMFILE getFirstMemFile();
///Vraci dalsi soubr ve filesystemu
__inline PMEMFILE getNextMemFile(PMEMFILE f) {return f->next;}
///Uzavre otevreny soubor
/** 
  Funkce rozlisuje, zda je soubor ve filesystemu nebo ne. Pokud je ve filesystemu,
  pak funkce prakticky nedela nic. Pokud neni ve filesystemu, pak je
  soubor smazan
 */
void closeMemFile(PMEMFILE f);

#endif